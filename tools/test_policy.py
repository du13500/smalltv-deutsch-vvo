#!/usr/bin/env python3
"""Provider cooldowns and route regressions with an independent fake clock."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
test=r'''
#include "RequestPolicy.h"
#include "RoutePolicy.h"
#include <cassert>
#include <cmath>
#include <cstdio>
static uint32_t clockMs=100;
uint32_t millis(){return clockMs;}
int main(){
 assert(retryAfterMs("120",0)==120000);
 assert(retryAfterMs("garbage",0)==0);
 assert(retryAfterMs("Wed, 07 Oct 2026 12:02:00 GMT",1791374400)==120000);
 assert(retryAfterMs("Wed, 07 Oct 2026 12:00:00 GMT",1791374400)==0);
 serviceResponse(SERVICE_ADSB_LOL,429,"");assert(serviceWaitMs(SERVICE_ADSB_LOL)==30000);
 clockMs+=29999;assert(serviceWaitMs(SERVICE_ADSB_LOL)==1);
 clockMs++;assert(!serviceWaitMs(SERVICE_ADSB_LOL));
 serviceResponse(SERVICE_ADSB_LOL,429,"");assert(serviceWaitMs(SERVICE_ADSB_LOL)==60000);
 serviceResponse(SERVICE_ADSB_LOL,429,"180");assert(serviceWaitMs(SERVICE_ADSB_LOL)==180000);
 assert(!serviceWaitMs(SERVICE_ADSB_DB));
 serviceResponse(SERVICE_ADSB_LOL,201,"");assert(!serviceWaitMs(SERVICE_ADSB_LOL));
 assert(serviceMinIntervalMs(SERVICE_ADSB_LOL)==20000);
 serviceResponse(SERVICE_ADSB_LOL,200,"");assert(serviceMinIntervalMs(SERVICE_ADSB_LOL)==20000);
 clockMs+=599999;assert(serviceMinIntervalMs(SERVICE_ADSB_LOL)==20000);
 clockMs++;assert(!serviceMinIntervalMs(SERVICE_ADSB_LOL));
 serviceResponse(SERVICE_ADSB_LOL,429,"");assert(serviceWaitMs(SERVICE_ADSB_LOL)==30000);assert(serviceMinIntervalMs(SERVICE_ADSB_LOL)==15000);
 assert(!serviceMinIntervalMs(SERVICE_GEO));
 clockMs=0xfffffff0U;serviceResponse(SERVICE_GEO,429,"1");clockMs=983;assert(serviceWaitMs(SERVICE_GEO)==1);clockMs=984;assert(!serviceWaitMs(SERVICE_GEO));
 RoutePoint drs={51.134,13.768},fra={50.038,8.562},fmo={52.134,7.685},wro={51.103,16.886},ayt={36.899,30.801},dus={51.29,6.767},muc={48.354,11.786};
 RoutePoint wrong1[]={wro,fra},wrong2[]={fra,fmo},wrong3[]={ayt,dus};
 assert(!routePositionPlausible(drs,wrong1,2,true,false,0));
 assert(!routePositionPlausible(drs,wrong2,2,false,true,2450));
 assert(!routePositionPlausible(drs,wrong3,2,true,false,0));
 RoutePoint valid[]={drs,muc};assert(routePositionPlausible(drs,valid,2,true,false,0));
 RoutePoint peg[]={ayt,drs};assert(routePositionPlausible({51.04,13.65},peg,2,false,true,2900));
 assert(!routePositionPlausible({0,0},valid,2,false,true,33000));
 RoutePoint dateline[]={{0,170},{0,-170}};assert(routePositionPlausible({0,180},dateline,2,false,true,33000));
 RoutePoint invalid[]={{NAN,0},drs};assert(!routePositionPlausible(drs,invalid,2,false,false,0));
 puts("Retry-After, progressive/shared cooldowns, clock wrap and DRS route regressions: OK");
}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'Arduino.h').write_text('#pragma once\n#include <stdint.h>\nuint32_t millis();\n')
 (p/'policy.cpp').write_text(test)
 subprocess.run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-I'+str(p),'-I'+str(root/'src'),str(p/'policy.cpp'),str(root/'src/RequestPolicy.cpp'),'-o',str(p/'policy')],check=True)
 subprocess.run([str(p/'policy')],check=True)
