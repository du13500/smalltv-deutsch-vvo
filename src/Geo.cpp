#include "Geo.h"
#include "RequestPolicy.h"
#include "NetworkGuard.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "NetworkJson.h"
#include "GeoMemory.h"


// Keep TLS and JSON allocations separate. This scratch response never touches config.
// Bound storage, remove after each attempt, and fail closed on incomplete writes.
#include <LittleFS.h>
class GeoResponse : public Stream {
  File file_; size_t bytes_=0; bool overflow_=false,writeFailed_=false,lowHeap_=false;
public:
  static constexpr size_t limit=49152;
  static constexpr size_t reserve=GEO_RECEIVE_RESERVE;
  GeoResponse(){
    file_=LittleFS.open("/geo-response.tmp","w+");
    // Allocate the filesystem write cache before TLS occupies the heap.
    const uint8_t warm=0;
    if(file_&&(file_.write(&warm,1)!=1||!file_.truncate(0)||!file_.seek(0,SeekSet)))file_.close();
  }
  ~GeoResponse(){file_.close();LittleFS.remove("/geo-response.tmp");}
  bool ready() const {return (bool)file_;}
  void close(){file_.close();LittleFS.remove("/geo-response.tmp");}
  int availableForWrite() override {
    if(!file_||writeFailed_||overflow_||lowHeap_)return 0;
    if(ESP.getFreeHeap()<reserve){lowHeap_=true;return 0;}
    if(bytes_==limit){overflow_=true;return 0;}
    return (int)std::min(size_t(256),limit-bytes_);
  }
  // Zero capacity is a permanent failure for this response, not backpressure.
  bool outputCanTimeout() override {return false;}
  size_t write(uint8_t b) override {return write(&b,1);}
  size_t write(const uint8_t* p,size_t n) override {
    if(!file_||writeFailed_||lowHeap_)return 0;
    if(ESP.getFreeHeap()<reserve){lowHeap_=true;return 0;}
    if(n>limit-bytes_){overflow_=true;return 0;}
    size_t written=file_.write(p,n);bytes_+=written;
    if(written!=n)writeFailed_=true;
    return written;
  }
  int available() override {return file_.available();}
  int read() override {return file_.read();}
  int peek() override {return file_.peek();}
  void flush() override {file_.flush();}
  bool receive(HTTPClient& http,BearSSL::WiFiClientSecure& client,DiagRequest& trace){
    int transferred=-1;
    const int expected=http.getSize();
    if(expected>(int)limit)overflow_=true;
    else if(availableForWrite())transferred=http.writeToStream(this);
    bool complete=transferred>0&&!overflow_&&!writeFailed_&&!lowHeap_&&(size_t)transferred==bytes_;
    if(!complete)trace.stage(lowHeap_?DS_HEAP:overflow_?DS_RESPONSE_LIMIT:writeFailed_?DS_FILE_ERROR:DS_TRANSFER_ERROR);
    http.end();client.stop();flush();
    if(complete&&!file_.seek(0,SeekSet)){trace.stage(DS_FILE_ERROR);complete=false;}
    return complete;
  }
};

static BearSSL::Session g_geoSession;
static uint32_t g_geoLastRequest=0;static bool g_geoRequested=false;
static bool geoReady(){
  if(serviceWaitMs(SERVICE_GEO))return false;
  if(g_geoRequested&&(uint32_t)(millis()-g_geoLastRequest)<1100UL)return false;
  g_geoRequested=true;g_geoLastRequest=millis();return true;
}

static void makeSecure(BearSSL::WiFiClientSecure& client) {
  client.setInsecure();
  client.setBufferSizes(4096, 512);
  client.setSession(&g_geoSession);
  client.setTimeout(6000);
}

static bool geoHeapReady(DiagOp op,bool prepared=false) {
  const auto b=geoReceiveBudget(4096,sizeof(br_ssl_client_context),prepared);
  bool ok=ESP.getFreeHeap()>=b.total&&ESP.getMaxFreeBlockSize()>=b.block;
  if(!ok)diagEvent(op,DS_HEAP,0,b.total);
  return ok;
}

// All HTTP/TLS ownership ends when this helper returns, including the shared
// BearSSL stack. No JsonDocument may be constructed inside this function.
static int receiveGeo(String& url,GeoResponse& response,DiagRequest& trace,DiagOp op) {
  BearSSL::WiFiClientSecure client;
  makeSecure(client);
  HTTPClient http;
  http.useHTTP10(true);http.setTimeout(6000);http.setReuse(false);
  if(!geoReady()||!http.begin(client,url))return 0;
  url=String();
  http.addHeader("Accept","application/json");
  http.setUserAgent("SmallTV-Deutsch-VVO/0.4.14.8 (github.com/du13500/smalltv-deutsch-vvo)");
  const char* headers[]={"Retry-After"};http.collectHeaders(headers,1);
  if(!geoHeapReady(op,true))return 0;
  trace.stage(DS_GEO_CONNECT);
  int code=http.GET();trace.http(code);
  serviceResponse(SERVICE_GEO,code,http.header("Retry-After").c_str());
  if(code!=HTTP_CODE_OK){http.end();client.stop();return code;}
  trace.stage(DS_GEO_RECEIVE);
  return response.receive(http,client,trace)?HTTP_CODE_OK:0;
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

bool geoSearchPlaces(const String& query, bool germanNames, String& outJson) {
  DiagRequest trace(D_GEO_SEARCH);
  if(!geoHeapReady(D_GEO_SEARCH))return false;
  trace.stage(DS_GEO_PREPARE);
  if (WiFi.status() != WL_CONNECTED || serviceWaitMs(SERVICE_GEO)) return false;
  String q = query; q.trim();
  if (q.length() < 2) return false;

  GeoResponse response;
  if(!response.ready()){trace.stage(DS_FILE_ERROR);return false;}
  String url = F("https://nominatim.openstreetmap.org/search?format=jsonv2&namedetails=1&limit=8&q=");
  url += urlEncode(q);
  if (germanNames) url += F("&accept-language=de");
  q=String();
  if(receiveGeo(url,response,trace,D_GEO_SEARCH)!=HTTP_CODE_OK)return false;
  trace.stage(DS_GEO_RELEASE);
  JsonDocument filter(networkJsonAllocator());
  JsonObject f = filter[0].to<JsonObject>();
  f["name"] = true; f["display_name"] = true; f["lat"] = true; f["lon"] = true;
  f["namedetails"]["name"] = true; f["namedetails"]["name:de"] = true;

  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);
  if(!jsonFilterReady(filter,trace))return false;
  auto err = deserializeJson(doc, response, DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear();response.close();
  if (err) return false;

  JsonDocument out(networkJsonAllocator());
  out["ok"] = true;
  JsonArray places = out["places"].to<JsonArray>();
  for (JsonObjectConst r : doc.as<JsonArrayConst>()) {
    String name;
    if (germanNames) name = r["namedetails"]["name:de"] | "";
    if (!name.length()) name = r["namedetails"]["name"] | "";
    if (!name.length()) name = r["name"] | "";
    name.trim();
    if (!name.length()) continue;

    JsonObject p = places.add<JsonObject>();
    p["name"] = name;
    p["lat"] = String(r["lat"] | "0").toFloat();
    p["lon"] = String(r["lon"] | "0").toFloat();

    String label = r["display_name"] | name;
    p["label"] = label;
  }
  if(out.overflowed()){trace.stage(DS_JSON_MEMORY);return false;}
  doc.clear();
  serializeJson(out, outJson);
  trace.finish(true);return true;
}


static String firstSettlementName(JsonObjectConst a) {
  static const char* keys[] = {
    "neighbourhood", "quarter", "suburb", "city_district", "borough",
    "hamlet", "village", "town", "city", "municipality", "island", "islet"
  };
  for (const char* key : keys) {
    const char* v = a[key] | "";
    if (v && *v) return String(v);
  }
  return String();
}

static String broaderAreaName(JsonObjectConst a) {
  static const char* keys[] = {"county", "state_district", "state", "region", "country"};
  for (const char* key : keys) {
    const char* v = a[key] | "";
    if (v && *v) return String(v);
  }
  return String();
}

bool geoReversePlace(float lat, float lon, bool germanNames, String& outJson) {
  DiagRequest trace(D_GEO_REVERSE);
  if(!geoHeapReady(D_GEO_REVERSE))return false;
  trace.stage(DS_GEO_PREPARE);
  if (WiFi.status() != WL_CONNECTED || serviceWaitMs(SERVICE_GEO)) return false;
  if (lat < -90.0f || lat > 90.0f || lon < -180.0f || lon > 180.0f) return false;

  GeoResponse response;
  if(!response.ready()){trace.stage(DS_FILE_ERROR);return false;}
  String url = F("https://nominatim.openstreetmap.org/reverse?format=jsonv2&addressdetails=1&namedetails=1&zoom=13&layer=address,natural&lat=");
  url += String(lat, 6);
  url += F("&lon=");
  url += String(lon, 6);
  if (germanNames) url += F("&accept-language=de");
  int code=receiveGeo(url,response,trace,D_GEO_REVERSE);
  if (code != HTTP_CODE_OK) {
    if (code != HTTP_CODE_NOT_FOUND) return false;
    response.close();
    JsonDocument out(networkJsonAllocator());out["ok"]=true;out["name"]="";out["lat"]=lat;out["lon"]=lon;
    serializeJson(out,outJson);trace.finish(true);return true;
  }

  trace.stage(DS_GEO_RELEASE);
  JsonDocument filter(networkJsonAllocator());
  filter["name"] = true;
  filter["display_name"] = true;
  filter["namedetails"]["name"] = true;
  filter["namedetails"]["name:de"] = true;
  JsonObject af = filter["address"].to<JsonObject>();
  static const char* keys[] = {
    "city", "town", "village", "municipality", "hamlet", "island", "islet",
    "neighbourhood", "quarter", "city_district", "suburb", "borough", "county", "state_district", "state",
    "region", "sea", "ocean", "water", "country"
  };
  for (const char* key : keys) af[key] = true;

  JsonDocument doc(networkJsonAllocator());
  trace.stage(DS_JSON);
  if(!jsonFilterReady(filter,trace))return false;
  auto err = deserializeJson(doc, response, DeserializationOption::Filter(filter));
  logJsonError(trace,err); filter.clear();response.close();
  if (err) return false;

  JsonObjectConst addr = doc["address"].as<JsonObjectConst>();
  // zoom=13 resolves a locality/district rather than a street or building.
  // Search selections use geoSearchPlaces and keep their selected name.
  String name;
  if (germanNames) name = doc["namedetails"]["name:de"] | "";
  if (!name.length()) name = doc["namedetails"]["name"] | "";
  if (!name.length()) name = doc["name"] | "";
  name.trim();
  if (!name.length()) name = firstSettlementName(addr);
  if (!name.length()) {
    const char* water = addr["sea"] | (addr["ocean"] | (addr["water"] | ""));
    name = water;
  }


  if (!name.length()) name = broaderAreaName(addr);

  if (!name.length()) {
    String display = doc["display_name"] | "";
    int comma = display.indexOf(',');
    name = comma >= 0 ? display.substring(0, comma) : display;
    name.trim();
  }


  doc.clear();
  JsonDocument out(networkJsonAllocator());
  out["ok"] = true;
  out["name"] = name;
  out["lat"] = lat;
  out["lon"] = lon;
  if(out.overflowed()){trace.stage(DS_JSON_MEMORY);return false;}
  serializeJson(out, outJson);
  trace.finish(true);return true;
}
