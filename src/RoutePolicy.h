#pragma once
#include <math.h>
#include <stdint.h>
struct RoutePoint { float lat, lon; };
static inline bool routePointValid(RoutePoint p) {
  return isfinite(p.lat)&&isfinite(p.lon)&&p.lat>=-90&&p.lat<=90&&p.lon>=-180&&p.lon<=180;
}
static inline float routeDistanceKm(RoutePoint a,RoutePoint b) {
  const float rad=0.01745329252f;
  float dl=(b.lat-a.lat)*rad,dn=(b.lon-a.lon)*rad;
  float h=sinf(dl/2)*sinf(dl/2)+cosf(a.lat*rad)*cosf(b.lat*rad)*sinf(dn/2)*sinf(dn/2);
  return 12742.0f*asinf(sqrtf(fminf(1.0f,fmaxf(0.0f,h))));
}
// A low/ground aircraft should be near a reported airport. At cruising height
// allow detours, but reject positions far outside the finite route segment.
static inline bool routePositionPlausible(RoutePoint p,const RoutePoint* airports,uint8_t count,bool ground,bool altitudeKnown,int32_t altitudeFt) {
  if(!routePointValid(p)||count<2)return false;
  float nearest=1e9f;
  for(uint8_t i=0;i<count;i++){
    if(!routePointValid(airports[i]))return false;
    nearest=fminf(nearest,routeDistanceKm(p,airports[i]));
  }
  if(ground)return nearest<=25;
  if(altitudeKnown&&altitudeFt<10000)return nearest<=80;
  for(uint8_t i=1;i<count;i++){
    float length=routeDistanceKm(airports[i-1],airports[i]);
    float detour=routeDistanceKm(p,airports[i-1])+routeDistanceKm(p,airports[i])-length;
    if(detour<=fmaxf(100.0f,length*0.20f))return true;
  }
  return false;
}
