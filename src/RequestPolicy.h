#pragma once
#include <stdint.h>
#include <stddef.h>

// Provider-wide cooldowns survive configuration changes, not power cycles.
enum RequestService { SERVICE_ADSB_LOL, SERVICE_ADSB_DB, SERVICE_GEO, SERVICE_WEATHER, SERVICE_VVO, SERVICE_COUNT };
uint32_t serviceWaitMs(RequestService service);
uint32_t serviceMinIntervalMs(RequestService service);
void serviceResponse(RequestService service, int code, const char* retryAfter);
// Forget a cooldown, e.g. after the user entered a new API key.
void serviceClear(RequestService service);
uint32_t retryAfterMs(const char* value, int64_t nowEpoch);

// 10, 20, then 40 minutes after repeated 401/403/426.
static inline uint32_t deniedBackoffMs(uint8_t failures) {
  return 600000UL << (failures > 2 ? 2 : failures);
}

static inline uint32_t retryBackoffMs(uint8_t failures) {
  return 30000UL << (failures > 4 ? 4 : failures);
}
