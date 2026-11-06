#pragma once
#include "RadarPolicy.h"

// Geo receives to an already warmed file, never to a JsonDocument while TLS
// is alive. Keep the same 3-KiB SDK/TCP reserve enforced on every file write.
// Other services still need the generic 4-KiB budget for concurrent JSON.
static constexpr uint32_t GEO_RECEIVE_RESERVE = 3072;
static inline RadarTlsBudget geoReceiveBudget(uint16_t rx, size_t contextBytes, bool prepared) {
  RadarTlsBudget b = radarTlsBudget(rx, contextBytes);
  b.total = b.total - 4096 + GEO_RECEIVE_RESERVE;
  if (prepared) {
    b.total -= NETWORK_TLS_STACK_BYTES;
    uint32_t input = uint32_t(rx) + 325;
    uint32_t context = uint32_t(contextBytes) + 256;
    b.block = (input > context ? input : context) + 256;
  }
  return b;
}
