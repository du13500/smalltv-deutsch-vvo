#!/usr/bin/env python3
"""Execute real radar guard, retry and stale-state functions with a fake clock/heap.
Network transfer and TLS handshake require the device; this tests the decisions
before a request and the state transitions independently of the data provider.
"""
from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/Flight.cpp').read_text()
def function(name):
 m=re.search(r'^(?:static )?[^\n]+\b'+name+r'\([^\n]*\) \{',s,re.M)
 assert m,name
 start=m.start();end=s.index('{',start)+1;depth=1
 while depth:
  if s[end]=='{':depth+=1
  elif s[end]=='}':depth-=1
  end+=1
 return s[start:end]
pre='''#include <cassert>
#include <cstdint>
#include <cstring>
#include "RadarPolicy.h"
enum {D_RADAR,DS_TLS};struct DiagRequest {DiagRequest(int){} void stage(int){}};
static uint32_t clockMs=10000;
uint32_t millis(){return clockMs;}
struct br_ssl_client_context { char bytes[3408]; };
struct FakeESP { uint32_t heap=25000,block=11368; uint32_t getFreeHeap(){return heap;}uint32_t getMaxFreeBlockSize(){return block;} } ESP;
const int WL_CONNECTED=3;
struct FakeWifi { int state=WL_CONNECTED;int status(){return state;} } WiFi;
uint32_t serviceWaitMs(int){return 0;}
static uint32_t minimumInterval=0;
uint32_t serviceMinIntervalMs(int){return minimumInterval;}
const int SERVICE_ADSB_LOL=0;
struct Snapshot {bool valid,error;} g_f={};
static uint32_t g_lastPollMs=0,g_lastOkMs=0,g_firstFailureMs=0;
static bool g_hasPolled=false,g_providerPaused=false;
static uint8_t g_consecutiveFailures=0;
static int g_lastHttp=0;
static const char* g_stage="bereit";
static uint16_t g_adsbLolRx=512;
static int probes=0,requests=0;
static uint16_t adsbLolRx(){++probes;return g_adsbLolRx;}
'''
funcs='\n'.join(function(n) for n in ['tlsHeapReady','flightPollDue','flightExpire','flightFailure','flightSuccess'])
# Run the original pre-request section verbatim; replace only the network body.
fetch=function('flightFetch').split('  uint16_t radiusNm',1)[0]
fetch+='  (void)homeLat;(void)homeLon;(void)radiusKm;requests++;return true;\n}\n'
tests='''int main(){
 for(auto rx:{512,1024,4096}){g_adsbLolRx=rx;ESP.heap=25248;ESP.block=11368;assert(flightFetch(51,13,25));}
 assert(requests==3&&probes==3);
 g_f.valid=true;g_f.error=false;g_lastOkMs=10000;clockMs=15000;
 ESP.block=6000;assert(!flightFetch(51,13,25));assert(requests==3&&probes==3);
 assert(g_f.valid&&g_f.error);assert(!strcmp(g_stage,"übersprungen: Heap"));
 assert(!flightPollDue(5));clockMs=20001;assert(flightPollDue(5));
 minimumInterval=15000;clockMs=29999;assert(!flightPollDue(5));clockMs=30000;assert(flightPollDue(5));minimumInterval=0;
 clockMs=39999;assert(!flightExpire());clockMs=40000;assert(flightExpire());assert(!g_f.valid&&g_f.error);assert(!flightExpire());
 ESP.heap=25768;ESP.block=13192;g_adsbLolRx=512;assert(flightFetch(51,13,25));
 g_f={true,false};flightSuccess();assert(!g_consecutiveFailures&&g_lastOkMs==40000);
 WiFi.state=0;clockMs=45000;assert(!flightFetch(51,13,25));assert(g_f.valid&&g_f.error);assert(!strcmp(g_stage,"kein WLAN"));
 clockMs=70000;assert(!flightFailure("HTTP-Fehler"));assert(!g_f.valid);
 // Confirmed provider pauses preserve data beyond normal expiry, even after
 // the cooldown timer ends, until the next actual result. Real errors expire.
 g_f={true,false};clockMs=80000;flightSuccess();clockMs=110001;
 assert(!flightFailure("Abrufpause: Anbieter",true));assert(g_f.valid&&g_f.error&&g_providerPaused);
 clockMs=590000;assert(!flightExpire()&&g_f.valid);
 assert(!flightFailure("Verbindung fehlgeschlagen"));assert(!g_f.valid&&!g_providerPaused);
 g_f={true,false};flightSuccess();assert(!g_providerPaused&&!g_consecutiveFailures);
 g_f={false,false};flightFailure("Abrufpause: Anbieter",true);assert(!g_f.valid);flightSuccess();
 // Unsigned elapsed times remain valid across the millis wrap.
 g_f={true,true};g_lastOkMs=0xfffffff0U;clockMs=29983;assert(!flightExpire());clockMs=29984;assert(flightExpire());
}
'''
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)/'radar.cpp';p.write_text('#include <initializer_list>\n'+pre+funcs+fetch+tests)
 out=Path(tmp)/'radar';subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(p),'-o',str(out)],check=True)
 subprocess.run([str(out)],check=True)
print('Real radar guard/retry: screenshot heap values admitted, failures marked, expiry/recovery/wrap OK')
