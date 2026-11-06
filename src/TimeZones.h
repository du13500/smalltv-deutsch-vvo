// POSIX rules from ESP8266 Arduino Core TZ.h, IANA 2023d (IANA-derived).
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifdef ARDUINO
#include <pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#define pgm_read_ptr(p) (*(p))
#define strcmp_P(a,b) strcmp((a),(b))
#endif
struct TimeZoneEntry { const char* id; const char* rule; };
static const char timeZoneId0[] PROGMEM="Europe/Berlin";
static const char timeZoneRule0[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId1[] PROGMEM="Europe/London";
static const char timeZoneRule1[] PROGMEM="GMT0BST,M3.5.0/1,M10.5.0";
static const char timeZoneId2[] PROGMEM="Europe/Paris";
static const char timeZoneRule2[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId3[] PROGMEM="Europe/Madrid";
static const char timeZoneRule3[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId4[] PROGMEM="Europe/Rome";
static const char timeZoneRule4[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId5[] PROGMEM="Europe/Prague";
static const char timeZoneRule5[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId6[] PROGMEM="Europe/Warsaw";
static const char timeZoneRule6[] PROGMEM="CET-1CEST,M3.5.0,M10.5.0/3";
static const char timeZoneId7[] PROGMEM="Europe/Athens";
static const char timeZoneRule7[] PROGMEM="EET-2EEST,M3.5.0/3,M10.5.0/4";
static const char timeZoneId8[] PROGMEM="Europe/Helsinki";
static const char timeZoneRule8[] PROGMEM="EET-2EEST,M3.5.0/3,M10.5.0/4";
static const char timeZoneId9[] PROGMEM="Europe/Bucharest";
static const char timeZoneRule9[] PROGMEM="EET-2EEST,M3.5.0/3,M10.5.0/4";
static const char timeZoneId10[] PROGMEM="Europe/Kyiv";
static const char timeZoneRule10[] PROGMEM="EET-2EEST,M3.5.0/3,M10.5.0/4";
static const char timeZoneId11[] PROGMEM="Europe/Istanbul";
static const char timeZoneRule11[] PROGMEM="<+03>-3";
static const char timeZoneId12[] PROGMEM="Europe/Moscow";
static const char timeZoneRule12[] PROGMEM="MSK-3";
static const char timeZoneId13[] PROGMEM="Atlantic/Reykjavik";
static const char timeZoneRule13[] PROGMEM="GMT0";
static const char timeZoneId14[] PROGMEM="Atlantic/Canary";
static const char timeZoneRule14[] PROGMEM="WET0WEST,M3.5.0/1,M10.5.0";
static const char timeZoneId15[] PROGMEM="Atlantic/Azores";
static const char timeZoneRule15[] PROGMEM="<-01>1<+00>,M3.5.0/0,M10.5.0/1";
static const char timeZoneId16[] PROGMEM="Africa/Cairo";
static const char timeZoneRule16[] PROGMEM="EET-2EEST,M4.5.5/0,M10.5.4/24";
static const char timeZoneId17[] PROGMEM="Africa/Johannesburg";
static const char timeZoneRule17[] PROGMEM="SAST-2";
static const char timeZoneId18[] PROGMEM="Africa/Nairobi";
static const char timeZoneRule18[] PROGMEM="EAT-3";
static const char timeZoneId19[] PROGMEM="America/New_York";
static const char timeZoneRule19[] PROGMEM="EST5EDT,M3.2.0,M11.1.0";
static const char timeZoneId20[] PROGMEM="America/Chicago";
static const char timeZoneRule20[] PROGMEM="CST6CDT,M3.2.0,M11.1.0";
static const char timeZoneId21[] PROGMEM="America/Denver";
static const char timeZoneRule21[] PROGMEM="MST7MDT,M3.2.0,M11.1.0";
static const char timeZoneId22[] PROGMEM="America/Phoenix";
static const char timeZoneRule22[] PROGMEM="MST7";
static const char timeZoneId23[] PROGMEM="America/Los_Angeles";
static const char timeZoneRule23[] PROGMEM="PST8PDT,M3.2.0,M11.1.0";
static const char timeZoneId24[] PROGMEM="America/Anchorage";
static const char timeZoneRule24[] PROGMEM="AKST9AKDT,M3.2.0,M11.1.0";
static const char timeZoneId25[] PROGMEM="Pacific/Honolulu";
static const char timeZoneRule25[] PROGMEM="HST10";
static const char timeZoneId26[] PROGMEM="America/Toronto";
static const char timeZoneRule26[] PROGMEM="EST5EDT,M3.2.0,M11.1.0";
static const char timeZoneId27[] PROGMEM="America/Vancouver";
static const char timeZoneRule27[] PROGMEM="PST8PDT,M3.2.0,M11.1.0";
static const char timeZoneId28[] PROGMEM="America/Mexico_City";
static const char timeZoneRule28[] PROGMEM="CST6";
static const char timeZoneId29[] PROGMEM="America/Bogota";
static const char timeZoneRule29[] PROGMEM="<-05>5";
static const char timeZoneId30[] PROGMEM="America/Sao_Paulo";
static const char timeZoneRule30[] PROGMEM="<-03>3";
static const char timeZoneId31[] PROGMEM="America/Argentina/Buenos_Aires";
static const char timeZoneRule31[] PROGMEM="<-03>3";
static const char timeZoneId32[] PROGMEM="America/St_Johns";
static const char timeZoneRule32[] PROGMEM="NST3:30NDT,M3.2.0,M11.1.0";
static const char timeZoneId33[] PROGMEM="Asia/Jerusalem";
static const char timeZoneRule33[] PROGMEM="IST-2IDT,M3.4.4/26,M10.5.0";
static const char timeZoneId34[] PROGMEM="Asia/Dubai";
static const char timeZoneRule34[] PROGMEM="<+04>-4";
static const char timeZoneId35[] PROGMEM="Asia/Kolkata";
static const char timeZoneRule35[] PROGMEM="IST-5:30";
static const char timeZoneId36[] PROGMEM="Asia/Kathmandu";
static const char timeZoneRule36[] PROGMEM="<+0545>-5:45";
static const char timeZoneId37[] PROGMEM="Asia/Bangkok";
static const char timeZoneRule37[] PROGMEM="<+07>-7";
static const char timeZoneId38[] PROGMEM="Asia/Singapore";
static const char timeZoneRule38[] PROGMEM="<+08>-8";
static const char timeZoneId39[] PROGMEM="Asia/Shanghai";
static const char timeZoneRule39[] PROGMEM="CST-8";
static const char timeZoneId40[] PROGMEM="Asia/Tokyo";
static const char timeZoneRule40[] PROGMEM="JST-9";
static const char timeZoneId41[] PROGMEM="Asia/Seoul";
static const char timeZoneRule41[] PROGMEM="KST-9";
static const char timeZoneId42[] PROGMEM="Australia/Perth";
static const char timeZoneRule42[] PROGMEM="AWST-8";
static const char timeZoneId43[] PROGMEM="Australia/Darwin";
static const char timeZoneRule43[] PROGMEM="ACST-9:30";
static const char timeZoneId44[] PROGMEM="Australia/Adelaide";
static const char timeZoneRule44[] PROGMEM="ACST-9:30ACDT,M10.1.0,M4.1.0/3";
static const char timeZoneId45[] PROGMEM="Australia/Brisbane";
static const char timeZoneRule45[] PROGMEM="AEST-10";
static const char timeZoneId46[] PROGMEM="Australia/Sydney";
static const char timeZoneRule46[] PROGMEM="AEST-10AEDT,M10.1.0,M4.1.0/3";
static const char timeZoneId47[] PROGMEM="Pacific/Auckland";
static const char timeZoneRule47[] PROGMEM="NZST-12NZDT,M9.5.0,M4.1.0/3";
static const char timeZoneId48[] PROGMEM="Pacific/Fiji";
static const char timeZoneRule48[] PROGMEM="<+12>-12";
static const char timeZoneId49[] PROGMEM="UTC";
static const char timeZoneRule49[] PROGMEM="UTC0";
static const TimeZoneEntry timeZones[] PROGMEM={
  {timeZoneId0,timeZoneRule0},
  {timeZoneId1,timeZoneRule1},
  {timeZoneId2,timeZoneRule2},
  {timeZoneId3,timeZoneRule3},
  {timeZoneId4,timeZoneRule4},
  {timeZoneId5,timeZoneRule5},
  {timeZoneId6,timeZoneRule6},
  {timeZoneId7,timeZoneRule7},
  {timeZoneId8,timeZoneRule8},
  {timeZoneId9,timeZoneRule9},
  {timeZoneId10,timeZoneRule10},
  {timeZoneId11,timeZoneRule11},
  {timeZoneId12,timeZoneRule12},
  {timeZoneId13,timeZoneRule13},
  {timeZoneId14,timeZoneRule14},
  {timeZoneId15,timeZoneRule15},
  {timeZoneId16,timeZoneRule16},
  {timeZoneId17,timeZoneRule17},
  {timeZoneId18,timeZoneRule18},
  {timeZoneId19,timeZoneRule19},
  {timeZoneId20,timeZoneRule20},
  {timeZoneId21,timeZoneRule21},
  {timeZoneId22,timeZoneRule22},
  {timeZoneId23,timeZoneRule23},
  {timeZoneId24,timeZoneRule24},
  {timeZoneId25,timeZoneRule25},
  {timeZoneId26,timeZoneRule26},
  {timeZoneId27,timeZoneRule27},
  {timeZoneId28,timeZoneRule28},
  {timeZoneId29,timeZoneRule29},
  {timeZoneId30,timeZoneRule30},
  {timeZoneId31,timeZoneRule31},
  {timeZoneId32,timeZoneRule32},
  {timeZoneId33,timeZoneRule33},
  {timeZoneId34,timeZoneRule34},
  {timeZoneId35,timeZoneRule35},
  {timeZoneId36,timeZoneRule36},
  {timeZoneId37,timeZoneRule37},
  {timeZoneId38,timeZoneRule38},
  {timeZoneId39,timeZoneRule39},
  {timeZoneId40,timeZoneRule40},
  {timeZoneId41,timeZoneRule41},
  {timeZoneId42,timeZoneRule42},
  {timeZoneId43,timeZoneRule43},
  {timeZoneId44,timeZoneRule44},
  {timeZoneId45,timeZoneRule45},
  {timeZoneId46,timeZoneRule46},
  {timeZoneId47,timeZoneRule47},
  {timeZoneId48,timeZoneRule48},
  {timeZoneId49,timeZoneRule49}
};
static constexpr size_t TIME_ZONE_COUNT=sizeof(timeZones)/sizeof(timeZones[0]);
static inline int timeZoneIndex(const char* id) {
  if(!id)return -1;
  for(size_t i=0;i<TIME_ZONE_COUNT;i++)if(!strcmp_P(id,(const char*)pgm_read_ptr(&timeZones[i].id)))return (int)i;
  return -1;
}
static inline const char* timeZoneRule(int index) {
  if(index<0||(size_t)index>=TIME_ZONE_COUNT)index=0;
  return (const char*)pgm_read_ptr(&timeZones[index].rule);
}
