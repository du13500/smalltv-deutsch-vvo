#pragma once
#include <Arduino.h>

struct VvoDeparture {
  char line[10];
  char direction[56];
  int16_t minutes;
  bool cancelled;
  bool delayed;
};

void vvoReset();
bool vvoPollDue(uint16_t pollSec);
enum VvoMotMask : uint8_t {
  VVO_MOT_TRAIN = 1 << 0,
  VVO_MOT_SUBURBAN = 1 << 1,
  VVO_MOT_TRAM = 1 << 2,
  VVO_MOT_BUS = 1 << 3,
  VVO_MOT_CABLEWAY = 1 << 4,
  VVO_MOT_FERRY = 1 << 5,
  VVO_MOT_TAXI = 1 << 6,
  VVO_MOT_ALL = 0x7F
};

bool vvoFetchDepartures(const String& stopId, uint8_t limit, uint8_t motMask = VVO_MOT_ALL);
bool vvoSearchStops(const String& query, String& outJson);
uint8_t vvoCount();
const VvoDeparture& vvoAt(uint8_t i);
bool vvoError();
bool vvoNotServed();
uint32_t vvoLastOkMs();
