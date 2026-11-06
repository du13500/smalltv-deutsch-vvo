#pragma once
#include <stdint.h>
#include <stddef.h>

// ESP8266 Arduino 3.1.2: one 6,200-byte BearSSL stack, input overhead 325,
// output overhead 85. Context size comes from the compiled BearSSL headers.
// Reserve covers validator, TCP, HTTP, filtered JSON and allocation overhead.
static constexpr uint32_t NETWORK_TLS_STACK_BYTES = 6200;
struct RadarTlsBudget { uint32_t total, block; };
static inline RadarTlsBudget radarTlsBudget(uint16_t rx, size_t contextBytes) {
  uint32_t input = (uint32_t)rx + 325;
  uint32_t output = 512 + 85;
  uint32_t context = (uint32_t)contextBytes + 256;
  uint32_t largest = input > context ? input : context;
  // Stack, context and buffers are separate allocations. Keep a margin
  // for the largest allocation; free-heap reserve is accounted for below.
  if (largest < NETWORK_TLS_STACK_BYTES + 256) largest = NETWORK_TLS_STACK_BYTES + 256;
  return {NETWORK_TLS_STACK_BYTES + input + output + context + 4096, largest};
}
static inline bool radarTlsFits(uint32_t freeHeap, uint32_t maxBlock,
                                uint16_t rx, size_t contextBytes) {
  RadarTlsBudget b = radarTlsBudget(rx, contextBytes);
  return freeHeap >= b.total && maxBlock >= b.block;
}
static constexpr uint32_t RADAR_MAX_AGE_MS = 30000;
static inline bool radarExpired(uint32_t now, uint32_t lastOk, bool valid) {
  return valid && (uint32_t)(now - lastOk) >= RADAR_MAX_AGE_MS;
}
