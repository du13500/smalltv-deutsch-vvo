#include "Weather.h"
#include "WeatherVisual.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "NetworkJson.h"
#include "PollPolicy.h"
#include "RequestPolicy.h"
#include "NetworkGuard.h"

static WeatherSnapshot g_w = {};
static PollPolicy g_poll;
static uint32_t g_lastOkMs = 0;
static BearSSL::Session g_session;

static uint16_t g_tlsRx=0;
static void makeSecure(BearSSL::WiFiClientSecure& client) {
  client.setInsecure();
  client.setBufferSizes(g_tlsRx?g_tlsRx:4096, 512);
  client.setSession(&g_session);
  client.setTimeout(6000);
}

static void hhmm(const char* iso, char out[6]) {
  strlcpy(out, "--:--", 6);
  if (!iso) return;
  const char* t = strchr(iso, 'T');
  if (!t || strlen(t) < 6) return;
  memcpy(out, t + 1, 5);
  out[5] = 0;
}

void weatherReset() {
  memset(&g_w, 0, sizeof(g_w));
  g_w.rainPct = -1;
  strlcpy(g_w.sunrise, "--:--", sizeof(g_w.sunrise));
  strlcpy(g_w.sunset, "--:--", sizeof(g_w.sunset));
  g_poll.reset();
  g_lastOkMs = 0;
}

bool weatherPollDue(uint16_t pollSec) {
  if(serviceWaitMs(SERVICE_WEATHER))return false;
  return g_poll.due(millis(), (uint32_t)pollSec * 1000UL);
}

const WeatherSnapshot& weatherData() { return g_w; }
uint32_t weatherLastOkMs() { return g_lastOkMs; }

const char* weatherLabel(int code) { return weatherVisual(code).label; }

bool weatherFetch(float lat, float lon, const char* timeZone) {
  if(serviceWaitMs(SERVICE_WEATHER))return false;
  DiagRequest trace(D_WEATHER);
  struct Completion { ~Completion(){g_poll.finish(millis(),!g_w.error);} } completion;
  g_w.error=true;
  if(!networkHeapReady(D_WEATHER,networkTlsRx("api.open-meteo.com",g_tlsRx,D_WEATHER)))return false;
  trace.stage(DS_TLS);
  if (WiFi.status() != WL_CONNECTED) { g_w.error = true; return false; }

  BearSSL::WiFiClientSecure client;
  makeSecure(client);
  HTTPClient http;
  http.useHTTP10(true);
  http.setTimeout(6000);
  http.setReuse(false);

  String url = F("https://api.open-meteo.com/v1/forecast?latitude=");
  url += String(lat, 4);
  url += F("&longitude=");
  url += String(lon, 4);
  url += F("&current=temperature_2m,weather_code,is_day");
  url += F("&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,sunrise,sunset");
  String zone=timeZone;zone.replace("/","%2F");
  url += F("&timezone=");url += zone;url += F("&forecast_days=1");

  if (!http.begin(client, url)) { g_w.error = true; return false; }
  http.addHeader("Accept", "application/json");
  http.setUserAgent("SmallTV-DD/0.4.14.7");

  const char* headers[]={"Retry-After"};http.collectHeaders(headers,1);
  int code = http.GET();
  trace.http(code);
  serviceResponse(SERVICE_WEATHER,code,http.header("Retry-After").c_str());
  if(serviceWaitMs(SERVICE_WEATHER))diagEvent(D_WEATHER,DS_WAIT,code,serviceWaitMs(SERVICE_WEATHER));
  if (code != HTTP_CODE_OK) { http.end(); g_w.error = true; return false; }

  JsonDocument filter(networkJsonAllocator());
  filter["current"]["temperature_2m"] = true;
  filter["current"]["weather_code"] = true;
  filter["current"]["is_day"] = true;
  filter["daily"]["temperature_2m_max"][0] = true;
  filter["daily"]["temperature_2m_min"][0] = true;
  filter["daily"]["precipitation_probability_max"][0] = true;
  filter["daily"]["sunrise"][0] = true;
  filter["daily"]["sunset"][0] = true;

  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);
  if(!jsonFilterReady(filter,trace)){http.end();client.stop();return false;}
  auto err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear(); http.end(); client.stop();
  if (err) { g_w.error = true; return false; }

  if (!doc["current"]["temperature_2m"].is<float>() && !doc["current"]["temperature_2m"].is<int>()) {
    g_w.error = true;
    return false;
  }

  WeatherSnapshot next = {};
  next.valid = true;
  next.error = false;
  next.currentC = doc["current"]["temperature_2m"].as<float>();
  next.code = doc["current"]["weather_code"] | 0;
  next.isDay = (doc["current"]["is_day"] | 1) != 0;
  next.highC = doc["daily"]["temperature_2m_max"][0] | next.currentC;
  next.lowC = doc["daily"]["temperature_2m_min"][0] | next.currentC;
  next.rainPct = doc["daily"]["precipitation_probability_max"][0] | -1;
  hhmm(doc["daily"]["sunrise"][0] | (const char*)nullptr, next.sunrise);
  hhmm(doc["daily"]["sunset"][0] | (const char*)nullptr, next.sunset);
  g_w = next;
  g_lastOkMs = millis();
  trace.finish(true);
  return true;
}
