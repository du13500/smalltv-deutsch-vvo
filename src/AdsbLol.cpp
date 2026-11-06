#include "AdsbLol.h"
#include "RadarBudget.h"
#include <time.h>

// Optional key from adsb.lol; lifts the per-IP daily request limit.
static char g_key[72] = "";
// Requests sent today (UTC day), kept in RAM only.
static uint16_t g_requestsToday = 0;
static int32_t g_day = -1;

static bool clockValid(time_t now) { return now > 1609459200; }

static void rollDay() {
  time_t now = time(nullptr);
  int32_t day = clockValid(now) ? (int32_t)(now / 86400) : -1;
  if (day != g_day) { g_day = day; g_requestsToday = 0; }
}

void adsbLolSetKey(const String& key) {
  strlcpy(g_key, key.c_str(), sizeof(g_key));
}

bool adsbLolHasKey() { return g_key[0] != 0; }

void adsbLolPrepare(HTTPClient& http, const char* userAgent) {
  rollDay();
  if (g_requestsToday < 0xFFFF) g_requestsToday++;
  http.useHTTP10(false);
  http.setUserAgent(userAgent);
  if (g_key[0]) http.addHeader("x-api-key", g_key);
  static const char* headers[] = {"Retry-After", "Transfer-Encoding"};
  http.collectHeaders(headers, 2);
}

uint16_t adsbLolRequestsToday() {
  rollDay();
  return g_requestsToday;
}

uint32_t adsbLolBudgetIntervalMs(float visibleShare) {
  if (g_key[0]) return 0;
  rollDay();
  time_t now = time(nullptr);
  if (!clockValid(now)) return 0;
  return radarBudgetIntervalMs(g_requestsToday, ADSB_LOL_DAILY_BUDGET,
                               86400UL - (uint32_t)(now % 86400), visibleShare);
}

bool adsbLolBudgetLow() {
  if (g_key[0]) return false;
  rollDay();
  return g_requestsToday + ADSB_LOL_ROUTE_RESERVE >= ADSB_LOL_DAILY_BUDGET;
}
