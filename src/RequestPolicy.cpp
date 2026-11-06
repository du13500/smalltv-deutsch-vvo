#include "RequestPolicy.h"
#include <Arduino.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Cooldown { uint32_t started=0, duration=0; uint8_t failures=0; uint32_t limitedAt=0,minInterval=0; };
static Cooldown cooldowns[SERVICE_COUNT];

uint32_t retryAfterMs(const char* value, int64_t nowEpoch) {
  if (!value || !*value) return 0;
  char* end=nullptr;
  unsigned long seconds=strtoul(value,&end,10);
  if(end!=value && !*end) return (uint32_t)(seconds>86400UL?86400000UL:seconds*1000UL);
  char weekday[4]={},month[4]={},zone[4]={};int d=0,y=0,h=0,m=0,s=0;
  if(sscanf(value,"%3s, %d %3s %d %d:%d:%d %3s",weekday,&d,month,&y,&h,&m,&s,zone)!=8 || strcmp(zone,"GMT") || nowEpoch<1609459200) return 0;
  const char* months="JanFebMarAprMayJunJulAugSepOctNovDec";int mo=0;
  for(int i=0;i<12;i++)if(!strncmp(month,months+i*3,3))mo=i+1;
  if(!mo||d<1||d>31||y<2021||y>2100||h<0||h>23||m<0||m>59||s<0||s>59)return 0;
  // Gregorian civil date to Unix days, independent of the selected time zone.
  y-=mo<=2;int era=y/400;unsigned yoe=(unsigned)(y-era*400);
  unsigned doy=(153*(mo+(mo>2?-3:9))+2)/5+d-1;
  unsigned doe=yoe*365+yoe/4-yoe/100+doy;
  int64_t epoch=((int64_t)era*146097+doe-719468)*86400+h*3600+m*60+s;
  int64_t delta=epoch-nowEpoch;
  return delta<=0?0:(uint32_t)(delta>86400?86400000:delta*1000);
}

uint32_t serviceWaitMs(RequestService service) {
  const Cooldown& c=cooldowns[service];uint32_t elapsed=millis()-c.started;
  return elapsed<c.duration?c.duration-elapsed:0;
}

uint32_t serviceMinIntervalMs(RequestService service) {
  Cooldown& c=cooldowns[service];
  if(c.minInterval && (uint32_t)(millis()-c.limitedAt)>=600000UL){c.minInterval=0;c.failures=0;}
  return c.minInterval;
}

void serviceResponse(RequestService service,int code,const char* retryAfter) {
  Cooldown& c=cooldowns[service];
  if(code==429||code==503){
    uint32_t requested=retryAfterMs(retryAfter,(int64_t)time(nullptr));
    c.started=millis();c.duration=requested?requested:retryBackoffMs(c.failures);
    if(service==SERVICE_ADSB_LOL && code==429){
      c.limitedAt=millis(); c.minInterval=c.failures?20000UL:15000UL;
    }
    if(c.failures<5)c.failures++;
  } else if(code==401||code==403||code==426){
    // Rejected request (missing/invalid key, outdated protocol): asking again
    // every few seconds cannot help and only burns memory and quota.
    c.started=millis();c.duration=deniedBackoffMs(c.failures);
    if(c.failures<5)c.failures++;
  } else if(code>=200&&code<300){c.duration=0;if(!serviceMinIntervalMs(service))c.failures=0;}
}

void serviceClear(RequestService service) { cooldowns[service] = Cooldown(); }
