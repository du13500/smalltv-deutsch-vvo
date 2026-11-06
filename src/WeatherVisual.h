#pragma once
#include <stdint.h>
enum WeatherMotif : uint8_t {
  WX_CLEAR, WX_PARTLY, WX_CLOUD, WX_FOG, WX_DRIZZLE, WX_RAIN,
  WX_FREEZING_DRIZZLE, WX_FREEZING_RAIN, WX_SNOW, WX_GRAINS,
  WX_RAIN_SHOWER, WX_SNOW_SHOWER, WX_THUNDER, WX_THUNDER_HAIL
};
struct WeatherVisual { WeatherMotif motif; uint8_t intensity; const char* label; };
static inline WeatherVisual weatherVisual(int code) {
  switch (code) {
    case 0: return {WX_CLEAR,0,"KLAR"};
    case 1: return {WX_CLEAR,0,"HEITER"};
    case 2: return {WX_PARTLY,0,"WOLKIG"};
    case 3: return {WX_CLOUD,0,"BEWÖLKT"};
    case 45: return {WX_FOG,0,"NEBEL"};
    case 48: return {WX_FOG,0,"REIFNEBEL"};
    case 51: return {WX_DRIZZLE,1,"LEICHTER NIESEL"};
    case 53: return {WX_DRIZZLE,2,"MÄSSIGER NIESEL"};
    case 55: return {WX_DRIZZLE,3,"DICHTER NIESEL"};
    case 56: return {WX_FREEZING_DRIZZLE,1,"GEFRIERENDER NIESEL"};
    case 57: return {WX_FREEZING_DRIZZLE,3,"GEFRIERENDER NIESEL"};
    case 61: return {WX_RAIN,1,"LEICHTER REGEN"};
    case 63: return {WX_RAIN,2,"MÄSSIGER REGEN"};
    case 65: return {WX_RAIN,3,"STARKER REGEN"};
    case 66: return {WX_FREEZING_RAIN,1,"GEFRIERENDER REGEN"};
    case 67: return {WX_FREEZING_RAIN,3,"GEFRIERENDER REGEN"};
    case 71: return {WX_SNOW,1,"LEICHTER SCHNEEFALL"};
    case 73: return {WX_SNOW,2,"MÄSSIGER SCHNEEFALL"};
    case 75: return {WX_SNOW,3,"STARKER SCHNEEFALL"};
    case 77: return {WX_GRAINS,1,"SCHNEEKÖRNER"};
    case 80: return {WX_RAIN_SHOWER,1,"LEICHTE SCHAUER"};
    case 81: return {WX_RAIN_SHOWER,2,"MÄSSIGE SCHAUER"};
    case 82: return {WX_RAIN_SHOWER,3,"HEFTIGE SCHAUER"};
    case 85: return {WX_SNOW_SHOWER,1,"LEICHTE SCHNEESCHAUER"};
    case 86: return {WX_SNOW_SHOWER,3,"STARKE SCHNEESCHAUER"};
    case 95: return {WX_THUNDER,2,"GEWITTER"};
    case 96: return {WX_THUNDER_HAIL,1,"GEWITTER MIT HAGEL"};
    case 99: return {WX_THUNDER_HAIL,3,"GEWITTER MIT HAGEL"};
    default: return {WX_CLOUD,0,"WETTER"};
  }
}
