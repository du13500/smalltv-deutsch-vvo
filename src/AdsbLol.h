#pragma once
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include "ChunkDecoder.h"

// adsb.lol rejects HTTP/1.0 with 426 "Upgrade Required" since late 2026.
// All adsb.lol requests therefore use HTTP/1.1 and read the body through
// AdsbLolBody, which removes chunked transfer framing on the fly.

void adsbLolSetKey(const String& key);
bool adsbLolHasKey();

// Daily request budget without a key (see RadarBudget.h). Every prepared
// request counts, including failed ones.
uint16_t adsbLolRequestsToday();
uint32_t adsbLolBudgetIntervalMs(float visibleShare);
bool adsbLolBudgetLow();

// Call after http.begin(): HTTP/1.1, optional x-api-key, collected headers.
void adsbLolPrepare(HTTPClient& http, const char* userAgent);

// HTTP status codes that will not go away by simply asking again.
static inline bool adsbLolAccessDenied(int code) {
  return code == 401 || code == 403 || code == 426;
}

class AdsbLolBody : public Stream {
public:
  explicit AdsbLolBody(HTTPClient& http)
    : _src{&http.getStream()},
      _decoder(_src, http.header("Transfer-Encoding").equalsIgnoreCase("chunked")) {}

  int read() override {
    if (_peeked >= 0) { int c = _peeked; _peeked = -1; return c; }
    return _decoder.read();
  }
  int peek() override {
    if (_peeked < 0) _peeked = _decoder.read();
    return _peeked;
  }
  int available() override { return _peeked >= 0 ? 1 : _src.stream->available(); }
  size_t write(uint8_t) override { return 0; }
  bool malformed() const { return _decoder.failed(); }

private:
  struct Source {
    Stream* stream;
    int nextByte() {
      // readBytes honours the client's timeout, like HTTPClient itself.
      char c;
      return stream->readBytes(&c, 1) == 1 ? (uint8_t)c : -1;
    }
  };
  Source _src;
  ChunkDecoder<Source> _decoder;
  int _peeked = -1;
};
