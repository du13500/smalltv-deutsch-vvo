#pragma once
#include <stdint.h>
#include <stddef.h>

// Provider-wide cooldowns survive configuration changes, not power cycles.
enum RequestService { SERVICE_ADSB_LOL, SERVICE_ADSB_DB, SERVICE_GEO, SERVICE_WEATHER, SERVICE_VVO, SERVICE_COUNT };
uint32_t serviceWaitMs(RequestService service);
uint32_t serviceMinIntervalMs(RequestService service);
void serviceResponse(RequestService service, int code, const char* retryAfter);
uint32_t retryAfterMs(const char* value, int64_t nowEpoch);

static inline uint32_t retryBackoffMs(uint8_t failures) {
  return 30000UL << (failures > 4 ? 4 : failures);
}
