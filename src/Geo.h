#pragma once
#include <Arduino.h>

bool geoSearchPlaces(const String& query, bool germanNames, String& outJson);
bool geoReversePlace(float lat, float lon, bool germanNames, String& outJson);
