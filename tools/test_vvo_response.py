#!/usr/bin/env python3
"""Run the production departure request with real JSON and controlled replies."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];source=(root/'src/Vvo.cpp').read_text()
def fn(name):
 a=source.index(name+'(');a=source.rfind('\n',0,a)+1;i=source.index('{',a)+1;depth=1
 while depth:depth+=(source[i]=='{')-(source[i]=='}');i+=1
 return source[a:i]
code=r'''
#include <ArduinoJson.h>
#include <cstring>
#include <cstdlib>
#define FW_USER_AGENT "PixelStop/test"
#include <cassert>
#include <cstdint>
#include <ctime>
#include <string>
#include <algorithm>
#include "PollPolicy.h"
#include "VvoResponse.h"
struct String:std::string {using std::string::string;String(const std::string&s):std::string(s){};};
void serializeJson(const JsonDocument&d,String&s){std::string tmp;ArduinoJson::serializeJson(d,tmp);s=String(tmp);}
size_t strlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){memcpy(d,s,std::min(l,n-1));d[std::min(l,n-1)]=0;}return l;}
static uint32_t nowMs=1000;uint32_t millis(){return nowMs;}
enum {SERVICE_VVO,D_VVO,DS_TLS,DS_WAIT,DS_JSON_MEMORY,DS_JSON,DS_SCHEMA,DS_EMPTY,WL_CONNECTED=3,HTTP_CODE_OK=200};
struct{int status(){return WL_CONNECTED;}}WiFi;
struct DiagRequest{DiagRequest(int){}void stage(int){}void http(int){}void finish(bool,int=0){}};
void diagEvent(int,int,int,uint32_t){}uint32_t serviceWaitMs(int){return 0;}void serviceResponse(int,int,const char*){}
bool networkHeapReady(int,int){return true;}int networkTlsRx(const char*,int,int){return 4096;}
namespace BearSSL{struct WiFiClientSecure{void stop(){}};}
void makeSecure(BearSSL::WiFiClientSecure&){}
static int responseCode=200;static std::string reply;
struct HTTPClient{void useHTTP10(bool){}void setTimeout(int){}bool begin(BearSSL::WiFiClientSecure&,String){return true;}void addHeader(const char*,const char*){}void setUserAgent(const char*){}void collectHeaders(const char**,int){}int POST(String){return responseCode;}String header(const char*){return "";}void end(){}const char*getStream(){return reply.c_str();}};
struct TestAllocator:ArduinoJson::Allocator{void*allocate(size_t n)override{return malloc(n);}void deallocate(void*p)override{free(p);}void*reallocate(void*p,size_t n)override{return realloc(p,n);}};
ArduinoJson::Allocator* networkJsonAllocator(){static TestAllocator a;return &a;}
bool jsonFilterReady(const JsonDocument&d,DiagRequest&){return !d.overflowed();}void logJsonError(DiagRequest&,DeserializationError){}
struct VvoDeparture{char line[10],direction[56];int16_t minutes;bool cancelled,delayed;};
static const char*VVO_HOST="webapi.vvo-online.de";static const uint8_t MAX_DEP=10;static VvoDeparture g_dep[MAX_DEP];static uint8_t g_count=0;static bool g_error=false,g_notServed=false;static uint32_t g_lastOkMs=0;static PollPolicy g_poll;static uint16_t g_tlsRx=4096;
enum{VVO_MOT_TRAIN=1,VVO_MOT_SUBURBAN=2,VVO_MOT_TRAM=4,VVO_MOT_BUS=8,VVO_MOT_CABLEWAY=16,VVO_MOT_FERRY=32,VVO_MOT_TAXI=64};
'''+fn('vvoEpochMs')+'\n'+fn('vvoFetchDepartures')+r'''
int main(){
 reply=R"JSON({"Name":"Campingplatz","Status":{"Code":"ServiceError","Message":"no serving lines found"},"Place":"Paulsdorf (Malter)"})JSON";
 assert(vvoFetchDepartures("33001604",10,127));assert(g_notServed&&!g_error&&g_count==0&&g_poll.failures==0);
 reply=R"JSON({"Status":{"Code":"ServiceError","Message":"stop invalid"}})JSON";
 assert(!vvoFetchDepartures("00000000",10,127));assert(!g_notServed&&g_error);
 reply=R"JSON({"Status":{"Code":"Ok"},"Departures":[{"LineName":"1","Direction":"Prohlis","ScheduledTime":"/Date(1791756300000)/"}]})JSON";
 assert(vvoFetchDepartures("33000037",10,127));assert(!g_notServed&&!g_error&&g_count==1&&!strcmp(g_dep[0].line,"1"));
 reply=R"JSON({"Status":{"Code":"Ok"},"Departures":[]})JSON";
 assert(vvoFetchDepartures("33000037",10,127));assert(!g_notServed&&!g_error&&g_count==0);
 reply=R"JSON({"Status":{"Code":"Ok","Message":"no serving lines found"}})JSON";
 assert(!vvoFetchDepartures("33001604",10,127));assert(!g_notServed&&g_error);
 responseCode=503;assert(!vvoFetchDepartures("33001604",10,127));assert(!g_notServed&&g_error);
 responseCode=200;reply="{";assert(!vvoFetchDepartures("33001604",10,127));assert(!g_notServed&&g_error);
 assert(!vvoResponseNotServed(nullptr,"no serving lines found"));assert(!vvoResponseNotServed("ServiceError",nullptr));
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(root/'src'),'-I'+str(root/'.pio/libdeps/smalltv_ultra/ArduinoJson/src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Production VVO: exact unserved response, real JSON filter, recovery, empty list and other errors OK')
