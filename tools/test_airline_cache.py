#!/usr/bin/env python3
"""Test production lookupAirline with real JSON and controlled HTTP responses."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];source=(root/'src/Flight.cpp').read_text()
a=source.index('static void lookupAirline(');b=source.index('\nstatic void enrichFlight',a)
code=r'''
#include <ArduinoJson.h>
#include <cassert>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
size_t strlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){memcpy(d,s,std::min(l,n-1));d[std::min(l,n-1)]=0;}return l;}
struct String:std::string {using std::string::string;String(const std::string&s):std::string(s){};void replace(const char*a,const char*b){size_t p;while((p=find(a))!=npos)std::string::replace(p,strlen(a),b);}};
static uint32_t now=1000,wait=0;uint32_t millis(){return now;}
static bool heapReady=true;static int requests=0,response=200;
static std::string body;static uint16_t g_adsbDbRx=4096;static int g_adsbDbSession=0;
enum{SERVICE_ADSB_DB,D_METADATA,D_AIRLINE,DS_TLS,DS_JSON};
uint32_t serviceWaitMs(int){return wait;}int networkTlsRx(const char*,int,int){return 4096;}bool tlsHeapReady(int){return heapReady;}
struct DiagRequest{DiagRequest(int){}void stage(int){}void http(int){}void finish(bool){}};
namespace BearSSL {struct WiFiClientSecure{void stop(){}};}
void makeSecure(BearSSL::WiFiClientSecure&,int*,int){}
struct HTTPClient{void useHTTP10(bool){}void setTimeout(int){}void setReuse(bool){}bool begin(BearSSL::WiFiClientSecure&,String){return true;}void collectHeaders(const char**,int){}void setUserAgent(const char*){}int GET(){requests++;return response;}String header(const char*){return "";}const char*getStream(){return body.c_str();}void end(){}};
void serviceResponse(int,int,const char*){}
struct TestAllocator:ArduinoJson::Allocator{void*allocate(size_t n)override{return malloc(n);}void deallocate(void*p)override{free(p);}void*reallocate(void*p,size_t n)override{return realloc(p,n);}};
ArduinoJson::Allocator*networkJsonAllocator(){static TestAllocator a;return &a;}
bool jsonFilterReady(const JsonDocument&d,DiagRequest&){return !d.overflowed();}void logJsonError(DiagRequest&,DeserializationError){}
struct FlightSnapshot{char callsign[12]="SKW4933",registration[12]="N710SK",hex[8]="abcdef";};struct Enrichment{char airline[64]={};};
'''
code+=source[a:b]+r'''
int main(){
 FlightSnapshot f;Enrichment out;
 response=404;lookupAirline(f,out);assert(requests==1&&!out.airline[0]);
 response=200;body=R"({"response":[{"icao":"SKW","name":"SkyWest Airlines"}]})";
 lookupAirline(f,out);assert(requests==2&&!strcmp(out.airline,"SkyWest Airlines"));
 out={};heapReady=false;wait=30000;lookupAirline(f,out);assert(requests==2&&!strcmp(out.airline,"SkyWest Airlines"));
 heapReady=true;wait=0;out={};now+=1800001;response=500;lookupAirline(f,out);assert(requests==3&&!out.airline[0]);
 response=200;lookupAirline(f,out);assert(requests==4&&out.airline[0]);
 strcpy(f.callsign,"UAL2183");out={};body="{";lookupAirline(f,out);assert(requests==5&&!out.airline[0]);
 body=R"({"response":[{"icao":"UAL","name":""}]})";lookupAirline(f,out);assert(requests==6&&!out.airline[0]);
 body=R"({"response":[{"icao":"AAL","name":"Wrong operator"}]})";lookupAirline(f,out);assert(requests==7&&!out.airline[0]);
 body=R"({"response":[{"icao":"UAL","name":"United Airlines"}]})";lookupAirline(f,out);assert(requests==8&&!strcmp(out.airline,"United Airlines"));
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(root/'.pio/libdeps/smalltv_ultra/ArduinoJson/src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Actual airline cache: errors not cached, valid names reused during pause/low heap, expiration recovers OK')
