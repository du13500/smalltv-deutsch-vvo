#pragma once
#include <Arduino.h>

struct WeatherSnapshot {
  bool valid;
  bool error;
  float currentC;
  float highC;
  float lowC;
  int16_t rainPct;
  int16_t code;
  bool isDay;
  char sunrise[6];
  char sunset[6];
};

void weatherReset();
bool weatherPollDue(uint16_t pollSec);
bool weatherFetch(float lat, float lon, const char* timeZone = "Europe/Berlin");
const WeatherSnapshot& weatherData();
uint32_t weatherLastOkMs();
const char* weatherLabel(int code);
