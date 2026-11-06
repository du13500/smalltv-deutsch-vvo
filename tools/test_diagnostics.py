#!/usr/bin/env python3
"""Compile the real diagnostic ring/RTC and poll policy with a fake ESP and clock."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
stub=r'''#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#define F(x) x
#define PROGMEM
#define strncpy_P strncpy
static uint32_t clockMs=0;
static uint32_t millis(){return clockMs;}
static size_t fakeStrlcpy(char* d,const char* s,size_t n){size_t len=strlen(s);if(n){memcpy(d,s,std::min(len,n-1));d[std::min(len,n-1)]=0;}return len;}
#define strlcpy fakeStrlcpy
struct Print {std::string text;void print(const char* s){text+=s;}void println(const char* s){text+=s;text+='\n';}};
struct FakeESP {
 uint8_t rtc[512]={};uint32_t heap=25000,block=12000;
 uint32_t getFreeHeap(){return heap;}uint32_t getMaxFreeBlockSize(){return block;}
 std::string getResetReason(){return "Exception";}std::string getResetInfo(){return "test reset";}
 bool rtcUserMemoryRead(uint32_t offset,uint32_t* data,size_t len){if(offset*4+len>512)return false;memcpy(data,rtc+offset*4,len);return true;}
 bool rtcUserMemoryWrite(uint32_t offset,uint32_t* data,size_t len){if(offset*4+len>512)return false;memcpy(rtc+offset*4,data,len);return true;}
} ESP;
'''
test=r'''
#include <cassert>
#include "PollPolicy.h"
int main(){
 PollPolicy p;assert(p.due(0,600000));p.finish(0,false);assert(!p.due(14999,600000));assert(p.due(15000,600000));
 p.finish(15000,false);assert(!p.due(44999,600000));assert(p.due(45000,600000));
 p.finish(45000,false);assert(!p.due(104999,600000));assert(p.due(105000,600000));
 p.finish(105000,false);assert(p.retryMs()==120000);p.finish(105001,false);assert(p.retryMs()==120000);
 p.finish(225001,true);assert(!p.due(240001,600000));assert(p.due(825001,600000));
 p.finish(0xfffffff0,false);assert(!p.due(14983,600000));assert(p.due(14984,600000));p.reset();assert(p.due(0,600000));
 memset(ESP.rtc,0xA5,128);diagInit();assert(!previousValid);
 for(int stage=DS_GEO_PREPARE;stage<=DS_GEO_RELEASE;++stage){diagEvent(D_GEO_SEARCH,(DiagStage)stage);assert(!hasLastError);Print printed;diagPrint(printed);assert(printed.text.find("Geo:")!=std::string::npos);diagInit();assert(previousValid);}
 memset(ESP.rtc+128,0,sizeof(ESP.rtc)-128);diagInit();
 for(int i=0;i<30;i++){clockMs=i+1;diagEvent(D_RADAR,DS_HTTP,200,i);}
 diagLookup("a44962","UAL2823");diagEvent(D_METADATA,DS_FAILED,404,200);for(int i=0;i<25;i++)diagEvent(D_RADAR,DS_DONE,200,i);Print retained;diagPrint(retained);assert(retained.text.find("Letzter Fehler dieses Starts")!=std::string::npos);assert(retained.text.find("HTTP 404")!=std::string::npos);assert(retained.text.find("UAL2823")!=std::string::npos);
 for(int i=0;i<30;i++){clockMs=i+1;diagEvent(D_RADAR,DS_HTTP,200,i);}
 Print out;diagPrint(out);assert(count==24);assert(out.text.find("Wert 5\n")==std::string::npos);assert(out.text.find("Wert 6\n")!=std::string::npos);assert(out.text.find("Wert 29\n")!=std::string::npos);
 for(int i=0;i<128;i++)assert(ESP.rtc[i]==0xA5); // protect OTA RTC words
 diagInit();assert(previousValid&&previous.code==200&&previous.value==29);assert(marker.boot==2);
 ESP.rtc[128+8]^=1;diagInit();assert(!previousValid); // checksum detects corrupted RTC
 clockMs=0xfffffff0;deferBackgroundNetwork(750);clockMs=733;assert(!backgroundNetworkReady());clockMs=734;assert(backgroundNetworkReady());
 clockMs=1000;{DiagRequest request(D_WEATHER);clockMs=1200;request.http(200);request.finish(true);}assert(marker.op==D_WEATHER&&marker.stage==DS_DONE&&marker.value==200);
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'Arduino.h').write_text(stub)
 (p/'test.cpp').write_text('#include "'+str(root/'src/Diagnostics.cpp')+'"\n'+test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(p),'-I'+str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Diagnostic ring, RTC checksum/OTA area, durations and poll retry/recovery/wrap: OK')
