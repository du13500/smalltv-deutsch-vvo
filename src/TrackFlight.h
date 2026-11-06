#pragma once
#include <Arduino.h>
#include "FlightRoute.h"

struct TrackedFlightSnapshot {
  bool valid;
  bool error;
  bool notFound;
  bool terminal;
  char requested[12];
  char flightIata[12];
  char flightIcao[12];
  char airline[28];
  char origin[4];
  char destination[4];
  FlightRoute route;
  char status[16];
  char depScheduled[6];
  char depEstimated[6];
  char depActual[6];
  char arrScheduled[6];
  char arrEstimated[6];
  char arrActual[6];
  char registration[16];
  char typeCode[8];
  int32_t altitudeM;
  float speedKmh;
  int64_t depScheduledEpoch;
  int64_t depActualEpoch;
  int64_t arrScheduledEpoch;
  int64_t arrEstimatedEpoch;
  int64_t arrActualEpoch;
};

void trackedFlightReset();
bool trackedFlightStatusPollDue(uint16_t pollMinutes);
bool trackedFlightLivePollDue(uint16_t pollSeconds = 15);
bool trackedFlightFetchStatus(const String& apiKey, const String& flightNumber);
bool trackedFlightFetchLive();
const TrackedFlightSnapshot& trackedFlightData();
uint32_t trackedFlightLastStatusOkMs();
uint32_t trackedFlightLastLiveOkMs();

bool trackedFlightFetchRoute();
