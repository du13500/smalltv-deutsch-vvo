#include "Version.h"
#include "RequestPolicy.h"
#include "TrackFlight.h"
#include "TrackerRoute.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "NetworkJson.h"
#include "AdsbLol.h"
#include "NetworkGuard.h"
#include <ctype.h>
#include <math.h>
#include <time.h>

static TrackedFlightSnapshot g_t = {};
static uint32_t g_lastStatusPollMs = 0;
static uint32_t g_lastLivePollMs = 0;
static uint32_t g_lastStatusOkMs = 0;
static uint32_t g_lastLiveOkMs = 0;
static float g_routeLat=NAN,g_routeLon=NAN;
static bool g_routeInitialDone=false,g_routePositionDone=false;
static BearSSL::Session g_statusSession;
static BearSSL::Session g_liveSession;

static void makeSecure(BearSSL::WiFiClientSecure& client, BearSSL::Session* session) {
  client.setInsecure();
  client.setBufferSizes(4096, 512);
  client.setSession(session);
  client.setTimeout(10000);
}

static String urlEncode(const String& in) {
  String out;
  out.reserve(in.length() * 3);
  static const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < in.length(); i++) {
    uint8_t c = (uint8_t)in[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out += (char)c;
    else { out += '%'; out += hex[c >> 4]; out += hex[c & 15]; }
  }
  return out;
}

static void trimCopyUpper(const String& in, char* out, size_t n) {
  String s = in;
  s.trim();
  s.toUpperCase();
  strlcpy(out, s.c_str(), n);
}

static void extractHHMM(const char* iso, char out[6]) {
  strlcpy(out, "N/A", 6);
  if (!iso) return;
  const char* t = strchr(iso, 'T');
  if (!t || strlen(t) < 6) return;
  if (!isdigit((unsigned char)t[1]) || !isdigit((unsigned char)t[2]) || t[3] != ':' ||
      !isdigit((unsigned char)t[4]) || !isdigit((unsigned char)t[5])) return;
  memcpy(out, t + 1, 5);
  out[5] = 0;
}

// Days since Unix epoch, based on Howard Hinnant's civil calendar algorithm.
static int64_t daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

static int64_t parseIsoEpoch(const char* s) {
  if (!s || strlen(s) < 19) return 0;
  int y=0,mo=0,d=0,h=0,mi=0,se=0;
  if (sscanf(s, "%4d-%2d-%2dT%2d:%2d:%2d", &y,&mo,&d,&h,&mi,&se) != 6) return 0;
  int tzSign = 0, tzh = 0, tzm = 0;
  const char* p = s + 19;
  if (*p == 'Z' || *p == 'z' || *p == 0) {
    tzSign = 0;
  } else if (*p == '+' || *p == '-') {
    tzSign = (*p == '+') ? 1 : -1;
    p++;
    if (sscanf(p, "%2d:%2d", &tzh, &tzm) < 1) { tzh = 0; tzm = 0; }
  }
  int64_t epoch = daysFromCivil(y, (unsigned)mo, (unsigned)d) * 86400LL + h * 3600LL + mi * 60LL + se;
  if (tzSign) epoch -= (int64_t)tzSign * (tzh * 3600LL + tzm * 60LL);
  return epoch;
}

static bool isTerminalStatus(const char* s) {
  if (!s) return false;
  return !strcmp(s, "landed") || !strcmp(s, "cancelled") || !strcmp(s, "incident") || !strcmp(s, "diverted");
}

void trackedFlightReset() {
  memset(&g_t, 0, sizeof(g_t));
  g_lastStatusPollMs = 0;
  g_lastLivePollMs = 0;
  g_lastStatusOkMs = 0;
  g_lastLiveOkMs = 0;
  g_routeLat=NAN;g_routeLon=NAN;g_routeInitialDone=false;g_routePositionDone=false;
}

bool trackedFlightStatusPollDue(uint16_t pollMinutes) {
  if (g_t.terminal && g_t.valid) return false;
  if (g_lastStatusPollMs == 0) return true;
  return (uint32_t)(millis() - g_lastStatusPollMs) >= (uint32_t)pollMinutes * 60000UL;
}

bool trackedFlightLivePollDue(uint16_t pollSeconds) {
  if (serviceWaitMs(SERVICE_ADSB_LOL)||!g_t.valid || g_t.terminal || !g_t.flightIcao[0]) return false;
  if (g_lastLivePollMs == 0) return true;
  return (uint32_t)(millis() - g_lastLivePollMs) >= (uint32_t)pollSeconds * 1000UL;
}

const TrackedFlightSnapshot& trackedFlightData() { return g_t; }
uint32_t trackedFlightLastStatusOkMs() { return g_lastStatusOkMs; }
uint32_t trackedFlightLastLiveOkMs() { return g_lastLiveOkMs; }

bool trackedFlightFetchStatus(const String& apiKey, const String& flightNumber) {
  DiagRequest trace(D_TRACK_STATUS);
  if(!networkHeapReady(D_TRACK_STATUS)){g_t.error=true;return false;}
  trace.stage(DS_TLS);
  g_lastStatusPollMs = millis() | 1;
  if (WiFi.status() != WL_CONNECTED || !apiKey.length() || !flightNumber.length()) {
    g_t.error = false;
    return false;
  }

  BearSSL::WiFiClientSecure client;
  makeSecure(client, &g_statusSession);
  HTTPClient http;
  http.useHTTP10(true);
  http.setReuse(false);
  http.setTimeout(10000);

  String url = F("https://api.aviationstack.com/v1/flights?access_key=");
  url += urlEncode(apiKey);
  url += F("&flight_iata=");
  url += urlEncode(flightNumber);
  url += F("&limit=3");

  if (!http.begin(client, url)) { g_t.error = true; return false; }
  http.addHeader("Accept", "application/json");
  http.setUserAgent(FW_USER_AGENT);
  int code = http.GET();trace.http(code);
  if (code != HTTP_CODE_OK) { http.end(); g_t.error = true; return false; }

  JsonDocument filter(networkJsonAllocator());
  JsonObject item = filter["data"][0].to<JsonObject>();
  item["flight_status"] = true;
  item["flight_date"] = true;
  item["airline"]["name"] = true;
  item["flight"]["iata"] = true;
  item["flight"]["icao"] = true;
  item["departure"]["iata"] = true;
  item["departure"]["scheduled"] = true;
  item["departure"]["estimated"] = true;
  item["departure"]["actual"] = true;
  item["arrival"]["iata"] = true;
  item["arrival"]["scheduled"] = true;
  item["arrival"]["estimated"] = true;
  item["arrival"]["actual"] = true;
  item["aircraft"]["registration"] = true;
  item["aircraft"]["icao"] = true;
  item["live"]["altitude"] = true;
  item["live"]["speed_horizontal"] = true;

  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);if(!jsonFilterReady(filter,trace)){http.end();client.stop();return false;}auto err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear(); http.end(); client.stop();
  if (err) { g_t.error = true; return false; }

  JsonArrayConst data = doc["data"].as<JsonArrayConst>();
  if (data.isNull() || data.size() == 0) {
    TrackedFlightSnapshot next = {};
    trimCopyUpper(flightNumber, next.requested, sizeof(next.requested));
    next.notFound = true;
    next.error = false;
    g_t = next;
    g_lastStatusOkMs = millis();
    trace.finish(true);return true;
  }

  size_t bestIndex=0; int bestScore=-999;
  time_t now=time(nullptr); char today[11]="";
  if(now>1609459200){struct tm tmv;localtime_r(&now,&tmv);strftime(today,sizeof(today),"%Y-%m-%d",&tmv);}
  for(size_t i=0;i<data.size();i++){
    JsonObjectConst cand=data[i].as<JsonObjectConst>();
    const char* st=cand["flight_status"]|"";
    const char* fd=cand["flight_date"]|"";
    int score=0;
    if(today[0]&&!strcmp(fd,today))score+=30;
    if(!strcmp(st,"active"))score+=50;
    else if(!strcmp(st,"scheduled"))score+=35;
    else if(!strcmp(st,"landed"))score+=15;
    else if(!strcmp(st,"cancelled"))score+=5;
    if(score>bestScore){bestScore=score;bestIndex=i;}
  }
  JsonObjectConst x = data[bestIndex].as<JsonObjectConst>();
  TrackedFlightSnapshot next = {};
  trimCopyUpper(flightNumber, next.requested, sizeof(next.requested));
  strlcpy(next.flightIata, x["flight"]["iata"] | next.requested, sizeof(next.flightIata));
  strlcpy(next.flightIcao, x["flight"]["icao"] | "", sizeof(next.flightIcao));
  strlcpy(next.airline, x["airline"]["name"] | "", sizeof(next.airline));
  strlcpy(next.origin, x["departure"]["iata"] | "N/A", sizeof(next.origin));
  strlcpy(next.destination, x["arrival"]["iata"] | "N/A", sizeof(next.destination));
  strlcpy(next.status, x["flight_status"] | "unknown", sizeof(next.status));
  strlcpy(next.registration, x["aircraft"]["registration"] | "", sizeof(next.registration));
  strlcpy(next.typeCode, x["aircraft"]["icao"] | "", sizeof(next.typeCode));

  const char* depSch = x["departure"]["scheduled"].as<const char*>();
  const char* depEst = x["departure"]["estimated"].as<const char*>();
  const char* depAct = x["departure"]["actual"].as<const char*>();
  const char* arrSch = x["arrival"]["scheduled"].as<const char*>();
  const char* arrEst = x["arrival"]["estimated"].as<const char*>();
  const char* arrAct = x["arrival"]["actual"].as<const char*>();
  extractHHMM(depSch, next.depScheduled);
  extractHHMM(depEst, next.depEstimated);
  extractHHMM(depAct, next.depActual);
  extractHHMM(arrSch, next.arrScheduled);
  extractHHMM(arrEst, next.arrEstimated);
  extractHHMM(arrAct, next.arrActual);
  next.depScheduledEpoch = parseIsoEpoch(depSch);
  next.depActualEpoch = parseIsoEpoch(depAct);
  next.arrScheduledEpoch = parseIsoEpoch(arrSch);
  next.arrEstimatedEpoch = parseIsoEpoch(arrEst);
  next.arrActualEpoch = parseIsoEpoch(arrAct);

  if (x["live"]["altitude"].is<float>() || x["live"]["altitude"].is<int>())
    next.altitudeM = (int32_t)lroundf(x["live"]["altitude"].as<float>());
  if (x["live"]["speed_horizontal"].is<float>() || x["live"]["speed_horizontal"].is<int>())
    next.speedKmh = x["live"]["speed_horizontal"].as<float>();

  next.valid = true;
  next.error = false;
  next.notFound = false;
  next.terminal = isTerminalStatus(next.status);
  // Keep route enrichment only for the same flight instance and reported leg.
  bool same=g_t.valid&&!strcmp(g_t.requested,next.requested)&&
    g_t.depScheduledEpoch==next.depScheduledEpoch&&!strcmp(g_t.origin,next.origin)&&
    !strcmp(g_t.destination,next.destination);
  appendRouteAirport(next.route,next.origin);appendRouteAirport(next.route,next.destination);
  if(same)next.route=g_t.route;
  else {g_routeLat=NAN;g_routeLon=NAN;g_routeInitialDone=false;g_routePositionDone=false;}
  g_t = next;
  g_lastStatusOkMs = millis();
  g_lastLivePollMs = 0;
  trace.finish(true);return true;
}

bool trackedFlightFetchLive() {
  DiagRequest trace(D_TRACK_LIVE);
  if(!networkHeapReady(D_TRACK_LIVE)){g_t.error=true;return false;}
  trace.stage(DS_TLS);
  if(serviceWaitMs(SERVICE_ADSB_LOL))return false;
  g_lastLivePollMs = millis();
  if (WiFi.status() != WL_CONNECTED || !g_t.valid || g_t.terminal || !g_t.flightIcao[0]) return false;

  BearSSL::WiFiClientSecure client;
  makeSecure(client, &g_liveSession);
  HTTPClient http;
  http.setReuse(false);
  http.setTimeout(9000);
  String url = F("https://api.adsb.lol/v2/callsign/");
  url += urlEncode(String(g_t.flightIcao));
  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");
  adsbLolPrepare(http, FW_USER_AGENT);
  int code = http.GET();trace.http(code);
  serviceResponse(SERVICE_ADSB_LOL,code,http.header("Retry-After").c_str());
  if (code != HTTP_CODE_OK) { http.end(); return false; }

  JsonDocument filter(networkJsonAllocator());
  JsonObject f = filter["ac"][0].to<JsonObject>();
  f["flight"] = true;
  f["alt_baro"] = true;
  f["gs"] = true;
  f["r"] = true;
  f["t"] = true;
  f["lat"] = true;f["lon"] = true;
  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);if(!jsonFilterReady(filter,trace)){http.end();client.stop();return false;}AdsbLolBody liveBody(http);auto err = deserializeJson(doc, liveBody, DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear(); http.end(); client.stop();
  if (err) return false;

  JsonArrayConst ac = doc["ac"].as<JsonArrayConst>();
  if (ac.isNull() || ac.size() == 0) return false;
  JsonObjectConst a = ac[0].as<JsonObjectConst>();
  if((a["lat"].is<float>()||a["lat"].is<int>())&&
     (a["lon"].is<float>()||a["lon"].is<int>())){
    g_routeLat=a["lat"].as<float>();g_routeLon=a["lon"].as<float>();
  }
  if (a["alt_baro"].is<int>() || a["alt_baro"].is<float>())
    g_t.altitudeM = (int32_t)lroundf(a["alt_baro"].as<float>() * 0.3048f);
  if (a["gs"].is<int>() || a["gs"].is<float>())
    g_t.speedKmh = a["gs"].as<float>() * 1.852f;
  if (!g_t.registration[0] && a["r"].is<const char*>()) strlcpy(g_t.registration, a["r"], sizeof(g_t.registration));
  if (!g_t.typeCode[0] && a["t"].is<const char*>()) strlcpy(g_t.typeCode, a["t"], sizeof(g_t.typeCode));
  g_lastLiveOkMs = millis();
  trace.finish(true);return true;
}

// One DB lookup, then at most one position-based lookup per flight instance.
// Missing intermediate data never implies that a flight is nonstop.
bool trackedFlightFetchRoute() {
  if(!g_t.valid||WiFi.status()!=WL_CONNECTED||ESP.getMaxFreeBlockSize()<16000)return false;
  bool positioned=!isnan(g_routeLat)&&!isnan(g_routeLon);
  if(g_t.route.count>2||(positioned?g_routePositionDone:g_routeInitialDone))return false;
  if(positioned)g_routePositionDone=true;
  g_routeInitialDone=true;
  FlightRoute route={};char origin[4]={},destination[4]={};
  const char* call=g_t.flightIcao[0]?g_t.flightIcao:g_t.flightIata;
  if(!trackerRouteLookup(call,g_routeLat,g_routeLon,route,origin,destination))return false;
  // A static route may be stale. Accept it only if it contains this reported leg.
  int from=-1,to=-1;
  for(uint8_t i=0;i<route.count;i++){
    if(from<0&&!strcmp(route.airports[i],g_t.origin))from=i;
    else if(from>=0&&!strcmp(route.airports[i],g_t.destination)){to=i;break;}
  }
  if(from<0||to<=from)return false;
  if(route.count>g_t.route.count){g_t.route=route;return true;}
  return false;
}
