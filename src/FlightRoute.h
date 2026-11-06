#pragma once
#include <stdint.h>
#include <string.h>

// Fixed storage keeps API-controlled data bounded on the ESP8266.
struct FlightRoute {
  static constexpr uint8_t capacity=8;
  char airports[capacity][4];
  uint8_t count;
  bool truncated;
};
static inline bool routeCode(const char* code) {
  if(!code||strlen(code)!=3)return false;
  for(uint8_t i=0;i<3;i++)if(code[i]<'A'||code[i]>'Z')return false;
  return true;
}
static inline bool appendRouteAirport(FlightRoute& route,const char* code) {
  if(!routeCode(code))return false;
  if(route.count<FlightRoute::capacity)memcpy(route.airports[route.count++],code,4);
  else route.truncated=true;
  return true;
}
static inline bool parseFlightRoute(const char* text,FlightRoute& out) {
  out={};if(!text)return false;
  FlightRoute next={};const char* p=text;
  while(*p){
    while(*p==' '||*p=='\t')p++;
    char code[4]={};uint8_t len=0;
    while(*p&&*p!='-'&&*p!=' '&&*p!='\t'){
      char c=*p++;if(c>='a'&&c<='z')c-=32;
      if(len>=3)return false;
      code[len++]=c;
    }
    if(!appendRouteAirport(next,code))return false;
    while(*p==' '||*p=='\t')p++;
    if(!*p)break;
    if(*p++!='-'||!*p)return false;
  }
  if(next.count<2)return false;
  out=next;return true;
}
