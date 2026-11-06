#pragma once
#include <Arduino.h>
#include "FlightRoute.h"

struct FlightSnapshot {
  bool valid;
  bool error;
  char hex[8]; // ICAO address identifies the aircraft even when callsign changes.
  char callsign[12];
  char airline[64];
  char registration[16];
  char typeCode[8];
  char category[4];
  char typeName[80];
  char origin[4];
  char destination[4];
  FlightRoute route;
  float distanceKm;
  int32_t altitudeFt;
  bool altitudeKnown;
  bool onGround;
};

void flightReset();
// visibleShare: fraction of the time the radar is on screen (for the daily budget).
bool flightPollDue(uint16_t pollSec, float visibleShare = 1.0f);
bool flightFetch(float homeLat, float homeLon, uint16_t radiusKm);
const FlightSnapshot& flightData();
uint32_t flightLastOkMs();
bool flightProviderPaused();
const char* flightStageName();
int flightLastHttp();
uint16_t flightTlsRx();

bool flightExpire();
uint32_t flightTlsMinBlock();

uint32_t flightWaitSec();
const char* flightRouteStatus();
const char* flightRouteSource();
int flightRouteHttp();

bool flightEnrichmentDue();
void flightEnrich();
