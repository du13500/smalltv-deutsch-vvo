#include "Vvo.h"
#include "VvoResponse.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "NetworkJson.h"
#include "PollPolicy.h"
#include "RequestPolicy.h"
#include "NetworkGuard.h"
#include <time.h>

static const char* VVO_HOST = "webapi.vvo-online.de";
static const uint8_t MAX_DEP = 10;
static VvoDeparture g_dep[MAX_DEP];
static uint8_t g_count = 0;
static bool g_error = false;
static bool g_notServed = false;
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

static String urlEncode(const String& in) {
  static const char* hex = "0123456789ABCDEF";
  String out;
  out.reserve(in.length() * 3);
  for (size_t i = 0; i < in.length(); i++) {
    uint8_t c = (uint8_t)in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else if (c == ' ') {
      out += "%20";
    } else {
      out += '%'; out += hex[c >> 4]; out += hex[c & 15];
    }
  }
  return out;
}

static uint64_t vvoEpochMs(const char* s) {
  if (!s || !*s) return 0;
  const char* p = strstr(s, "/Date(");
  if (!p) return 0;
  p += 6;
  uint64_t n = 0;
  bool any = false;
  while (*p >= '0' && *p <= '9') {
    any = true;
    n = n * 10ULL + (uint64_t)(*p - '0');
    p++;
  }
  return any ? n : 0;
}

static String field(const String& s, uint8_t index) {
  int start = 0;
  for (uint8_t i = 0; i < index; i++) {
    int p = s.indexOf('|', start);
    if (p < 0) return String();
    start = p + 1;
  }
  int end = s.indexOf('|', start);
  if (end < 0) end = s.length();
  return s.substring(start, end);
}

void vvoReset() {
  g_count = 0;
  g_error = false;
  g_notServed = false;
  g_poll.reset();
  g_lastOkMs = 0;
}

bool vvoPollDue(uint16_t pollSec) {
  if(serviceWaitMs(SERVICE_VVO))return false;
  return g_poll.due(millis(), (uint32_t)pollSec * 1000UL);
}

uint8_t vvoCount() { return g_count; }
const VvoDeparture& vvoAt(uint8_t i) { return g_dep[i < g_count ? i : 0]; }
bool vvoError() { return g_error; }
bool vvoNotServed() { return g_notServed; }
uint32_t vvoLastOkMs() { return g_lastOkMs; }

bool vvoSearchStops(const String& query, String& outJson) {
  if(serviceWaitMs(SERVICE_VVO))return false;
  DiagRequest trace(D_STOP_SEARCH);
  if(!networkHeapReady(D_STOP_SEARCH,networkTlsRx("webapi.vvo-online.de",g_tlsRx,D_STOP_SEARCH)))return false;
  trace.stage(DS_TLS);
  BearSSL::WiFiClientSecure client;
  makeSecure(client);
  HTTPClient http;
  http.useHTTP10(true);
  http.setTimeout(6000);

  String url = String("https://") + VVO_HOST + "/tr/pointfinder?query=" + urlEncode(query) +
               "&stopsOnly=true&regionalOnly=true&limit=8&format=json";
  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");
  http.setUserAgent("SmallTV-DD/0.4.14.7");

  const char* headers[]={"Retry-After"};http.collectHeaders(headers,1);
  int code = http.GET();
  trace.http(code);
  serviceResponse(SERVICE_VVO,code,http.header("Retry-After").c_str());
  if(serviceWaitMs(SERVICE_VVO))diagEvent(D_VVO,DS_WAIT,code,serviceWaitMs(SERVICE_VVO));
  if (code != HTTP_CODE_OK) { http.end(); return false; }

  JsonDocument filter(networkJsonAllocator());
  filter["Points"][0] = true;
  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);
  if(!jsonFilterReady(filter,trace)){http.end();client.stop();return false;}
  auto err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear(); http.end(); client.stop();
  if (err) return false;

  JsonDocument out(networkJsonAllocator());
  out["ok"] = true;
  JsonArray arr = out["stops"].to<JsonArray>();
  for (JsonVariantConst v : doc["Points"].as<JsonArrayConst>()) {
    String raw = v.as<String>();
    String id = field(raw, 0);
    if (!id.length() || id.indexOf(':') >= 0) continue;
    bool numeric = true;
    for (size_t i=0;i<id.length();i++) if (id[i]<'0'||id[i]>'9') numeric=false;
    if (!numeric) continue;
    String place = field(raw, 2);
    String name = field(raw, 3);
    if (!name.length()) continue;
    JsonObject o = arr.add<JsonObject>();
    o["id"] = id;
    o["name"] = name;
    o["place"] = place;
    o["label"] = place.length() ? place + ", " + name : name;
  }
  if(out.overflowed()){trace.stage(DS_JSON_MEMORY);return false;}
  serializeJson(out, outJson);
  trace.finish(true);
  return true;
}

bool vvoFetchDepartures(const String& stopId, uint8_t limit, uint8_t motMask) {
  if(serviceWaitMs(SERVICE_VVO))return false;
  DiagRequest trace(D_VVO);
  struct Completion { ~Completion(){g_poll.finish(millis(),!g_error);} } completion;
  g_error=true;
  g_notServed=false;
  if(!networkHeapReady(D_VVO,networkTlsRx("webapi.vvo-online.de",g_tlsRx,D_VVO)))return false;
  trace.stage(DS_TLS);
  if (!stopId.length() || WiFi.status() != WL_CONNECTED) {
    g_error = true;
    return false;
  }
  if (limit < 1) limit = 1;
  if (limit > MAX_DEP) limit = MAX_DEP;

  BearSSL::WiFiClientSecure client;
  makeSecure(client);
  HTTPClient http;
  http.useHTTP10(true);
  http.setTimeout(6000);
  String url = String("https://") + VVO_HOST + "/dm";
  if (!http.begin(client, url)) { g_error = true; return false; }
  http.addHeader("Content-Type", "application/json;charset=UTF-8");
  http.addHeader("Accept", "application/json");
  http.setUserAgent("SmallTV-DD/0.4.14.7");

  JsonDocument req(networkJsonAllocator());
  req["stopid"] = stopId;
  req["limit"] = limit;
  req["isarrival"] = false;
  req["shorttermchanges"] = true;
  req["format"] = "json";
  JsonArray mot = req["mot"].to<JsonArray>();
  if (motMask & VVO_MOT_TRAM) mot.add("Tram");
  if (motMask & VVO_MOT_BUS) { mot.add("CityBus"); mot.add("IntercityBus"); }
  if (motMask & VVO_MOT_SUBURBAN) mot.add("SuburbanRailway");
  if (motMask & VVO_MOT_TRAIN) mot.add("Train");
  if (motMask & VVO_MOT_CABLEWAY) mot.add("Cableway");
  if (motMask & VVO_MOT_FERRY) mot.add("Ferry");
  if (motMask & VVO_MOT_TAXI) mot.add("HailedSharedTaxi");
  if (mot.size() == 0) { http.end(); g_error = false; g_count = 0; trace.finish(true,DS_EMPTY); return true; }
  if(req.overflowed()){trace.stage(DS_JSON_MEMORY);http.end();return false;}
  String body; serializeJson(req, body);

  const char* headers[]={"Retry-After"};http.collectHeaders(headers,1);
  int code = http.POST(body);
  req.clear(); body = String();
  trace.http(code);
  serviceResponse(SERVICE_VVO,code,http.header("Retry-After").c_str());
  if(serviceWaitMs(SERVICE_VVO))diagEvent(D_VVO,DS_WAIT,code,serviceWaitMs(SERVICE_VVO));
  if (code != HTTP_CODE_OK) { http.end(); g_error = true; return false; }

  JsonDocument filter(networkJsonAllocator());
  filter["Status"]["Code"] = true;
  filter["Status"]["Message"] = true;
  JsonObject f = filter["Departures"][0].to<JsonObject>();
  f["LineName"] = true;
  f["Direction"] = true;
  f["ScheduledTime"] = true;
  f["RealTime"] = true;
  f["State"] = true;
  f["CancelReasons"][0]["Reason"] = true;

  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);
  if(!jsonFilterReady(filter,trace)){http.end();client.stop();return false;}
  auto err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear(); http.end(); client.stop();
  if (err) { g_error = true; return false; }

  if(vvoResponseNotServed(doc["Status"]["Code"]|"",doc["Status"]["Message"]|"")) {
    g_count=0;g_error=false;g_notServed=true;g_lastOkMs=millis();
    trace.finish(true,DS_EMPTY);return true;
  }
  JsonArrayConst deps = doc["Departures"].as<JsonArrayConst>();
  if (deps.isNull()) { trace.stage(DS_SCHEMA);g_error = true; return false; }

  g_count = 0;
  time_t now = time(nullptr);
  for (JsonObjectConst a : deps) {
    if (g_count >= MAX_DEP || g_count >= limit) break;
    VvoDeparture& d = g_dep[g_count];
    memset(&d, 0, sizeof(d));
    strlcpy(d.line, a["LineName"] | "?", sizeof(d.line));
    strlcpy(d.direction, a["Direction"] | "?", sizeof(d.direction));
    const char* state = a["State"] | "";
    d.delayed = !strcasecmp(state, "Delayed");
    d.cancelled = !a["CancelReasons"].as<JsonArrayConst>().isNull() && a["CancelReasons"].as<JsonArrayConst>().size() > 0;
    if (!strcasecmp(state, "Cancelled") || !strcasecmp(state, "Canceled")) d.cancelled = true;

    const char* ts = a["RealTime"] | (const char*)nullptr;
    if (!ts || !*ts) ts = a["ScheduledTime"] | "";
    uint64_t ms = vvoEpochMs(ts);
    d.minutes = -1;
    if (ms && now > 1609459200) {
      int64_t delta = (int64_t)(ms / 1000ULL) - (int64_t)now;
      d.minutes = delta <= 0 ? 0 : (int16_t)((delta + 59) / 60);
    }
    g_count++;
  }

  g_error = false;
  g_lastOkMs = millis();
  trace.finish(true);
  return true;
}
