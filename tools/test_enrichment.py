#!/usr/bin/env python3
"""Test the actual enrichment scheduler with stubbed provider transfers."""
from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[1];source=(root/'src/Flight.cpp').read_text()
def fn(name):
 m=re.search(r'^(?:static )?[^\n]+\b'+name+r'\([^\n]*\)\s*\{',source,re.M);assert m,name
 a=m.start();i=source.index('{',a)+1;depth=1
 while depth:
  depth+=(source[i]=='{')-(source[i]=='}');i+=1
 return source[a:i]
code=r'''
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
static uint32_t now=1000;uint32_t millis(){return now;}
struct FlightSnapshot {bool valid=true,error=false,onGround=false,altitudeKnown=false;int altitudeFt=0;char hex[8]="abc",callsign[12]="DLH062",airline[64]={},typeName[80]={},typeCode[8]={},category[4]={},origin[4]={},destination[4]={};FlightRoute route={};};
static FlightSnapshot g_f;static bool g_providerPaused=false;
static RoutePoint g_position={51,13};
static Enrichment g_cache[3]={};static uint8_t g_cacheNext=0;static uint32_t g_lastEnrichmentMs=0;
static const char *g_routeStatus="",*g_routeSource="",*g_stage="";
enum {SERVICE_ADSB_LOL,SERVICE_ADSB_DB};uint32_t waits[2]={};uint32_t serviceWaitMs(int i){return waits[i];}
static int calls[3]={};
static bool pauseRoute=false;
static bool lookupRouteAdsbLol(const FlightSnapshot&,RoutePoint,Enrichment&){calls[0]++;if(pauseRoute)waits[SERVICE_ADSB_LOL]=30000;return false;}
static bool lookupMetadata(const FlightSnapshot&,RoutePoint,Enrichment&){calls[1]++;return false;}
static void lookupAirline(const FlightSnapshot&,Enrichment&){calls[2]++;}
'''
struct=source[source.index('struct Enrichment {'):source.index('\nstatic Enrichment g_cache')]
# include actual route policy plus only minimal Arduino shim for route-header types
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'Arduino.h').write_text('#pragma once\n#include <cstdint>\n#include <cstring>\n#include <cctype>\n#include <algorithm>\n#define strlcpy testStrlcpy\ninline size_t testStrlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){memcpy(d,s,std::min(l,n-1));d[std::min(l,n-1)]=0;}return l;}\n')
 text='#include "Arduino.h"\n#include "RoutePolicy.h"\n#include "AircraftPolicy.h"\n#include "FlightRoute.h"\n'+struct+'\n'+code+'\n'+'\n'.join(fn(n) for n in ['enrichFlight','currentEnrichment','flightEnrichmentDue','flightEnrich'])+r'''
int main(){
 assert(flightEnrichmentDue());flightEnrich();assert(calls[0]==1&&calls[1]==0&&calls[2]==0);
 // Rendering/position updates only apply cached fields, without provider requests.
 for(int i=0;i<20;i++){enrichFlight(g_f,g_position);}assert(calls[0]==1&&calls[1]==0&&calls[2]==0);
 auto* cached=currentEnrichment();assert(cached);strcpy(cached->type,"Stearman");strcpy(cached->typeCode,"ST75");strcpy(g_f.category,"A3");enrichFlight(g_f,g_position);assert(!g_f.typeName[0]);
 strcpy(cached->type,"Airbus A321neo");strcpy(cached->typeCode,"A21N");enrichFlight(g_f,g_position);assert(!strcmp(g_f.typeName,"Airbus A321neo"));
 strcpy(g_f.typeCode,"B39M");strcpy(g_f.typeName,"Boeing 737 MAX 9");enrichFlight(g_f,g_position);assert(!strcmp(g_f.typeName,"Boeing 737 MAX 9"));
 flightEnrich();assert(calls[0]==1&&calls[1]==1&&calls[2]==0);
 waits[SERVICE_ADSB_DB]=30000;assert(!flightEnrichmentDue());waits[SERVICE_ADSB_DB]=0;
 flightEnrich();assert(calls[2]==1);assert(!flightEnrichmentDue());now=120999;assert(!flightEnrichmentDue());now=121000;assert(flightEnrichmentDue());
 g_f.error=true;assert(!flightEnrichmentDue());g_f.error=false;
 g_f.callsign[0]='A';now=2000;assert(!flightEnrichmentDue());now=61000;assert(flightEnrichmentDue());
 g_position.lat=NAN;assert(!flightEnrichmentDue());
 // A route request can impose the shared provider pause as well.
 g_position={51,13};g_f.error=false;g_cache[0]={};g_cache[1]={};g_cache[2]={};g_lastEnrichmentMs=0;pauseRoute=true;
 flightEnrich();assert(g_f.valid&&g_f.error&&g_providerPaused);assert(!flightEnrichmentDue());
}
'''
 (p/'test.cpp').write_text(text)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(p),'-I'+str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],check=True)
print('Actual enrichment: one provider per stage, cached position path, cooldown and retry spacing OK')
