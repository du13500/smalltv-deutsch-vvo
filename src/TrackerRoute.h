#pragma once
#include <Arduino.h>
#include "FlightRoute.h"

// Tracker enrichment is independent of the restored radar implementation.
bool trackerRouteLookup(const char* callsign, float lat, float lon, FlightRoute& route,
                        char origin[4], char destination[4]);
