#!/usr/bin/env python3
"""Exercise production guards before and after TLS stack allocation."""
from pathlib import Path
import subprocess, tempfile
root=Path(__file__).resolve().parents[1]
# Check the installed framework's separate TLS allocations.
framework=Path.home()/'.platformio/packages/framework-arduinoespressif8266'
stack=(framework/'cores/esp8266/StackThunk.cpp').read_text()
tls=(framework/'libraries/ESP8266WiFi/src/WiFiClientSecureBearSSL.cpp').read_text()
assert '#define _stackSize (6200/4)' in stack
assert '_sc = std::make_shared<br_ssl_client_context>();' in tls
assert '_iobuf_in = _alloc_iobuf(_iobuf_in_size);' in tls
assert '_iobuf_out = _alloc_iobuf(_iobuf_out_size);' in tls
# First-fit replay in a fragmented 20,208-byte heap with allocation overhead.
blocks=[9584,9584,1040]
for size in [6200,3408,4096+325,512+85]:
 needed=(size+31)//8*8
 for i,block in enumerate(blocks):
  if block>=needed:
   blocks[i]-=needed
   break
 else:
  raise AssertionError('TLS allocation does not fit replay heap')
assert sum(blocks)>=4096

with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)
 (p/'Arduino.h').write_text('''#pragma once
#include <cstdint>
struct Print;
''')
 (p/'ESP8266WiFi.h').write_text('''#pragma once
#include <cstddef>
#include <cstdint>
struct FakeESP {uint32_t heap=22040,block=20576;
 uint32_t getFreeHeap(){return heap;}uint32_t getMaxFreeBlockSize(){return block;}};
static FakeESP ESP;
''')
 (p/'WiFiClientSecureBearSSL.h').write_text('''#pragma once
// Context size matching the reported 18,922-byte budget at rx=4096.
struct br_ssl_client_context {char bytes[3408];};
namespace BearSSL {struct WiFiClientSecure {
 static bool probeMaxFragmentLength(const char*,int,int){return false;}};}
''')
 (p/'test.cpp').write_text('''#include "NetworkGuard.h"
#include "GeoMemory.h"
#include <cassert>
static uint32_t recorded=0;
void diagEvent(DiagOp,DiagStage stage,int code,uint32_t value){assert(stage==DS_HEAP&&code==0);recorded=value;}
int main(){
 const auto initial=geoReceiveBudget(4096,3408,false);
 const auto prepared=geoReceiveBudget(4096,3408,true);
 assert(initial.total==17954&&initial.block==6456);
 assert(prepared.total==11754&&prepared.block==4677);
 assert(geoReceiveBudget(512,3408,true).total<prepared.total);
 // Hardware reports: former 34/130-byte shortfalls now fit the receive-only budget.
 assert(12648>=prepared.total&&6712>=prepared.block);
 assert(12744>=prepared.total&&6712>=prepared.block);
 assert(18056>=initial.total&&6752>=initial.block);
 // Reserve never disappears, even for a negotiated small record.
 assert(prepared.total>=GEO_RECEIVE_RESERVE+4096+325+512+85+3408);
 assert(networkHeapReady(D_GEO_SEARCH,4096));
 // v0.4.14 hardware report: enough total heap, fragmented into 9,584-byte blocks.
 ESP.heap=20208;ESP.block=9584;
 assert(networkHeapReady(D_GEO_SEARCH,4096));
 assert(networkHeapReady(D_GEO_REVERSE,4096));
 assert(networkHeapReady(D_WEATHER,4096));
 ESP.heap=18978;ESP.block=6456;assert(networkHeapReady(D_GEO_SEARCH,4096));
 --ESP.heap;assert(!networkHeapReady(D_GEO_SEARCH,4096));
 ESP.heap=20208;ESP.block=6455;assert(!networkHeapReady(D_GEO_SEARCH,4096));
 ESP.heap=15016;ESP.block=14368;
 assert(!networkHeapReady(D_GEO_SEARCH,4096)&&recorded==18978);
 assert(networkPreparedHeapReady(D_GEO_SEARCH,4096));
 ESP.heap=13424;ESP.block=12952;
 assert(networkPreparedHeapReady(D_GEO_REVERSE,4096));
 ESP.heap=12778;ESP.block=4677;
 assert(networkPreparedHeapReady(D_GEO_SEARCH,4096));
 --ESP.heap;assert(!networkPreparedHeapReady(D_GEO_SEARCH,4096)&&recorded==12778);
 ESP.heap=20000;ESP.block=4676;
 assert(!networkPreparedHeapReady(D_GEO_REVERSE,4096));
 // A smaller negotiated TLS buffer still requires context and reserve.
 ESP.heap=15000;ESP.block=10000;assert(networkPreparedHeapReady(D_GEO_SEARCH,512));
 ESP.heap=5000;assert(!networkPreparedHeapReady(D_GEO_SEARCH,512));
}
''')
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(p),'-I'+str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('TLS guard: fragmented hardware heaps admitted; total reserve and actual allocation boundaries retained')
