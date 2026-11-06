#include "AdsbLol.h"

// Optional key from adsb.lol; lifts the per-IP daily request limit.
static char g_key[72] = "";

void adsbLolSetKey(const String& key) {
  strlcpy(g_key, key.c_str(), sizeof(g_key));
}

bool adsbLolHasKey() { return g_key[0] != 0; }

void adsbLolPrepare(HTTPClient& http, const char* userAgent) {
  http.useHTTP10(false);
  http.setUserAgent(userAgent);
  if (g_key[0]) http.addHeader("x-api-key", g_key);
  static const char* headers[] = {"Retry-After", "Transfer-Encoding"};
  http.collectHeaders(headers, 2);
}
