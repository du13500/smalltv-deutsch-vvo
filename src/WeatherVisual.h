#pragma once
#include <stdint.h>
// Labels are kept in flash on the device (PSTR): read them with FPSTR()/
// String(FPSTR(...)), never as plain RAM strings.
#ifndef PSTR
#define PSTR(x) (x)
#endif
#ifndef PGM_P
#define PGM_P const char*
#endif
enum WeatherMotif : uint8_t {
  WX_CLEAR, WX_PARTLY, WX_CLOUD, WX_FOG, WX_DRIZZLE, WX_RAIN,
  WX_FREEZING_DRIZZLE, WX_FREEZING_RAIN, WX_SNOW, WX_GRAINS,
  WX_RAIN_SHOWER, WX_SNOW_SHOWER, WX_THUNDER, WX_THUNDER_HAIL
};
struct WeatherVisual { WeatherMotif motif; uint8_t intensity; const char* label; };
static inline WeatherVisual weatherVisual(int code) {
  switch (code) {
    case 0: return {WX_CLEAR,0,PSTR("KLAR")};
    case 1: return {WX_CLEAR,0,PSTR("HEITER")};
    case 2: return {WX_PARTLY,0,PSTR("WOLKIG")};
    case 3: return {WX_CLOUD,0,PSTR("BEWÖLKT")};
    case 45: return {WX_FOG,0,PSTR("NEBEL")};
    case 48: return {WX_FOG,0,PSTR("REIFNEBEL")};
    case 51: return {WX_DRIZZLE,1,PSTR("LEICHTER NIESEL")};
    case 53: return {WX_DRIZZLE,2,PSTR("MÄSSIGER NIESEL")};
    case 55: return {WX_DRIZZLE,3,PSTR("DICHTER NIESEL")};
    case 56: return {WX_FREEZING_DRIZZLE,1,PSTR("GEFRIERENDER NIESEL")};
    case 57: return {WX_FREEZING_DRIZZLE,3,PSTR("GEFRIERENDER NIESEL")};
    case 61: return {WX_RAIN,1,PSTR("LEICHTER REGEN")};
    case 63: return {WX_RAIN,2,PSTR("MÄSSIGER REGEN")};
    case 65: return {WX_RAIN,3,PSTR("STARKER REGEN")};
    case 66: return {WX_FREEZING_RAIN,1,PSTR("GEFRIERENDER REGEN")};
    case 67: return {WX_FREEZING_RAIN,3,PSTR("GEFRIERENDER REGEN")};
    case 71: return {WX_SNOW,1,PSTR("LEICHTER SCHNEEFALL")};
    case 73: return {WX_SNOW,2,PSTR("MÄSSIGER SCHNEEFALL")};
    case 75: return {WX_SNOW,3,PSTR("STARKER SCHNEEFALL")};
    case 77: return {WX_GRAINS,1,PSTR("SCHNEEKÖRNER")};
    case 80: return {WX_RAIN_SHOWER,1,PSTR("LEICHTE SCHAUER")};
    case 81: return {WX_RAIN_SHOWER,2,PSTR("MÄSSIGE SCHAUER")};
    case 82: return {WX_RAIN_SHOWER,3,PSTR("HEFTIGE SCHAUER")};
    case 85: return {WX_SNOW_SHOWER,1,PSTR("LEICHTE SCHNEESCHAUER")};
    case 86: return {WX_SNOW_SHOWER,3,PSTR("STARKE SCHNEESCHAUER")};
    case 95: return {WX_THUNDER,2,PSTR("GEWITTER")};
    case 96: return {WX_THUNDER_HAIL,1,PSTR("GEWITTER MIT HAGEL")};
    case 99: return {WX_THUNDER_HAIL,3,PSTR("GEWITTER MIT HAGEL")};
    default: return {WX_CLOUD,0,PSTR("WETTER")};
  }
}

// English labels, also in flash.
static inline PGM_P weatherLabelEn(int code) {
  switch (code) {
    case 0: return PSTR("CLEAR");
    case 1: return PSTR("MAINLY CLEAR");
    case 2: return PSTR("PARTLY CLOUDY");
    case 3: return PSTR("OVERCAST");
    case 45: return PSTR("FOG");
    case 48: return PSTR("RIME FOG");
    case 51: return PSTR("LIGHT DRIZZLE");
    case 53: return PSTR("DRIZZLE");
    case 55: return PSTR("DENSE DRIZZLE");
    case 56: case 57: return PSTR("FREEZING DRIZZLE");
    case 61: return PSTR("LIGHT RAIN");
    case 63: return PSTR("RAIN");
    case 65: return PSTR("HEAVY RAIN");
    case 66: case 67: return PSTR("FREEZING RAIN");
    case 71: return PSTR("LIGHT SNOW");
    case 73: return PSTR("SNOW");
    case 75: return PSTR("HEAVY SNOW");
    case 77: return PSTR("SNOW GRAINS");
    case 80: return PSTR("LIGHT SHOWERS");
    case 81: return PSTR("SHOWERS");
    case 82: return PSTR("HEAVY SHOWERS");
    case 85: return PSTR("LIGHT SNOW SHOWERS");
    case 86: return PSTR("HEAVY SNOW SHOWERS");
    case 95: return PSTR("THUNDERSTORM");
    case 96: case 99: return PSTR("THUNDERSTORM, HAIL");
    default: return PSTR("WEATHER");
  }
}
