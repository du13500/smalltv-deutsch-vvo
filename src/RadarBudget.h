#pragma once
#include <stdint.h>

// Without a key adsb.lol allows about 500 requests per day and connection.
// The firmware keeps a margin because the exact counting rules (reset time,
// whether errors and route lookups count) are not documented.
static const uint16_t ADSB_LOL_DAILY_BUDGET = 450;
// Route lookups are skipped once fewer requests than this remain.
static const uint16_t ADSB_LOL_ROUTE_RESERVE = 40;

// Smallest poll interval that spreads the remaining requests over the rest
// of the day. `visibleShare` (0..1] is the fraction of that time the radar
// is on screen, because polls only run while it is visible.
// Returns 0 when no stretching is needed.
static inline uint32_t radarBudgetIntervalMs(uint16_t used, uint16_t budget,
                                             uint32_t secondsLeftToday,
                                             float visibleShare) {
  if (visibleShare <= 0.0f || visibleShare > 1.0f) visibleShare = 1.0f;
  if (secondsLeftToday > 86400UL) secondsLeftToday = 86400UL;
  if (used >= budget) return (secondsLeftToday + 60UL) * 1000UL;  // wait for the new day
  const uint32_t remaining = budget - used;
  const float visibleSeconds = (float)secondsLeftToday * visibleShare;
  return (uint32_t)(visibleSeconds * 1000.0f / (float)remaining);
}
