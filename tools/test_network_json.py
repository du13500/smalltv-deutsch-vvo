#!/usr/bin/env python3
"""Exercise the production allocator and parser under a simulated TLS heap budget."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
lib=root/'.pio/libdeps/smalltv_ultra/ArduinoJson/src'
if not lib.is_dir():
 import sys
 if len(sys.argv)>1:lib=Path(sys.argv[1])
 else:raise SystemExit('Install pinned ArduinoJson with pio pkg install before this test')
stub='''#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
static size_t availableHeap=12000;
static std::unordered_map<void*,size_t> allocations;
struct FakeESP {size_t getFreeHeap(){return availableHeap;}size_t getMaxFreeBlockSize(){return availableHeap;}};
static FakeESP ESP;
inline void* countedMalloc(size_t size){if(size>availableHeap)return nullptr;void*p=std::malloc(size);if(p){availableHeap-=size;allocations[p]=size;}return p;}
inline void countedFree(void*p){if(p){availableHeap+=allocations.at(p);allocations.erase(p);std::free(p);}}
inline void* countedRealloc(void*p,size_t size){size_t old=allocations.at(p);if(size>old+availableHeap)return nullptr;void*n=std::realloc(p,size);if(n){allocations.erase(p);allocations[n]=size;availableHeap+=old;availableHeap-=size;}return n;}
'''
test=r'''
#include <ArduinoJson.h>
#include <cassert>
#include <string>
#include "ESP8266WiFi.h"
#define malloc countedMalloc
#define free countedFree
#define realloc countedRealloc
#include "NetworkJson.h"
#undef malloc
#undef free
#undef realloc
static DiagStage observed=DS_BEGIN;
void DiagRequest::stage(DiagStage s){observed=s;}
DiagRequest::DiagRequest(DiagOp op):op_(op),started_(0){}
DiagRequest::~DiagRequest(){}
int main(){
 auto* a=networkJsonAllocator();
 availableHeap=5000;void* p=a->allocate(1000);assert(p&&availableHeap>=3072);
 memset(p,0x55,1000);availableHeap=3072;assert(a->reallocate(p,1100)==nullptr);
 assert(a->reallocate(p,5)==p);assert(((unsigned char*)p)[0]==0x55);a->deallocate(p);assert(allocations.empty());
 for(int i=0;i<200;i++){
  availableHeap=9000;
  {
   JsonDocument filter(a);filter["items"][0]["name"]=true;
   JsonDocument doc(a);auto err=deserializeJson(doc,"{\"items\":[{\"name\":\"Dresden\",\"unused\":123}]}",DeserializationOption::Filter(filter));
   assert(!err&&!doc.overflowed());assert(doc["items"][0]["name"]=="Dresden");assert(availableHeap>=3072);
  }
  assert(availableHeap==9000&&allocations.empty());
 }
 availableHeap=5000;
 {JsonDocument doc(a);std::string huge="[";for(int i=0;i<200;i++){if(i)huge+=",";huge+="{\"name\":\""+std::string(i+50,'x')+"\"}";}huge+="]";
  auto err=deserializeJson(doc,huge);assert(err==DeserializationError::NoMemory);assert(availableHeap>=3072);
  DiagRequest trace(D_GEO_REVERSE);logJsonError(trace,err);assert(observed==DS_JSON_MEMORY);
 }
 assert(allocations.empty()&&availableHeap==5000);
 availableHeap=3072;
 {JsonDocument filter(a);filter["name"]=true;DiagRequest trace(D_GEO_REVERSE);assert(!jsonFilterReady(filter,trace));assert(observed==DS_JSON_MEMORY);}
 assert(allocations.empty());
 for(auto c:{DeserializationError::IncompleteInput,DeserializationError::EmptyInput,DeserializationError::TooDeep,DeserializationError::InvalidInput}){
  DiagRequest trace(D_WEATHER);logJsonError(trace,DeserializationError(c));assert(observed!=DS_JSON_MEMORY&&observed!=DS_BEGIN);
 }
}
'''
# Run both actual Geo filters after the TLS allocation has been released.
geo=(root/'src/Geo.cpp').read_text()
blocks=[]
for part in geo.split('  JsonDocument filter(networkJsonAllocator());')[1:]:
 blocks.append('JsonDocument filter(networkJsonAllocator());'+part.split('  JsonDocument doc(networkJsonAllocator());',1)[0])
import json
search=[dict(name='New York',display_name='New York, '+('Unicode Dresden München '*12),lat='40.7127',lon='-74.006',namedetails={'name':'New York','name:de':'New York'}) for _ in range(8)]
reverse=dict(name='Rocca Cigliè',display_name='Rocca Cigliè, Piemonte, Italia',address={'village':'Rocca Cigliè','state':'Piemonte','country':'Italia'},namedetails={'name':'Rocca Cigliè','name:de':'Rocca Cigliè'})
extra=''
for block,payload in zip(blocks,[search,reverse]):
 extra+='\n{availableHeap=19000;\n'+block+'\nJsonDocument geoDoc(networkJsonAllocator());auto geoErr=deserializeJson(geoDoc,R"JSON('+json.dumps(payload,ensure_ascii=False)+')JSON",DeserializationOption::Filter(filter));assert(!geoErr);assert(availableHeap>=3072); }\nassert(allocations.empty()&&availableHeap==19000);\n'
pos=test.rfind('\n}')
test=test[:pos]+extra+test[pos:]

with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'Arduino.h').write_text('#pragma once\n#include <cstdint>\nstruct Print;\n')
 (p/'ESP8266WiFi.h').write_text(stub);(p/'test.cpp').write_text(test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-DARDUINOJSON_POOL_CAPACITY=32','-DARDUINOJSON_AUTO_SHRINK=0','-I'+str(p),'-I'+str(root/'src'),'-I'+str(lib),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Network JSON: reserved heap, failed growth, safe shrink, 200 cycles, NoMemory and error diagnosis OK')
