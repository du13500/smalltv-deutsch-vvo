#pragma once
#include <string.h>

// Stable display labels for known ICAO types, independent of metadata wording.
static inline const char* aircraftDisplayLabel(const char* code) {
  if(!code||!*code)return nullptr;
  struct Label {const char* code;const char* text;};
  static const Label labels[]={
    {"E170","Embraer E170"},{"E175","Embraer E175"},
    {"E75L","Embraer E175"},{"E75S","Embraer E175"},
    {"E190","Embraer E190"},{"E195","Embraer E195"},
    {"E290","Embraer E190-E2"},{"E295","Embraer E195-E2"},
    {"A318","Airbus A318"},{"A319","Airbus A319"},{"A320","Airbus A320"},
    {"A321","Airbus A321"},{"A20N","Airbus A320neo"},{"A21N","Airbus A321neo"},
    {"B737","Boeing 737-700"},{"B738","Boeing 737-800"},{"B739","Boeing 737-900"},
    {"B38M","Boeing 737 MAX 8"},{"B39M","Boeing 737 MAX 9"}
  };
  for(const auto& label:labels)if(!strcmp(code,label.code))return label.text;
  return nullptr;
}
