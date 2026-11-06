#pragma once
#include <Arduino.h>
enum DiagOp : uint8_t { D_BOOT,D_RADAR,D_RADAR_ROUTE,D_METADATA,D_AIRLINE,D_WEATHER,D_VVO,D_STOP_SEARCH,D_GEO_SEARCH,D_GEO_REVERSE,D_TRACK_STATUS,D_TRACK_LIVE,D_TRACK_ROUTE,D_CONFIG,D_REBOOT,D_COUNTDOWN,D_WIFI,D_COUNT };
enum DiagStage : uint8_t { DS_BEGIN,DS_TLS,DS_HTTP,DS_JSON,DS_DONE,DS_FAILED,DS_HEAP,DS_WAIT,DS_EMPTY,DS_SCHEMA,DS_JSON_MEMORY,DS_JSON_INCOMPLETE,DS_JSON_EMPTY,DS_JSON_DEPTH,DS_JSON_INVALID,DS_TRANSFER_ERROR,DS_FILE_ERROR,DS_RESPONSE_LIMIT,DS_GEO_PREPARE,DS_GEO_CONNECT,DS_GEO_RECEIVE,DS_GEO_RELEASE };
void diagInit();
void diagEvent(DiagOp op,DiagStage stage,int code=0,uint32_t value=0);
void diagLookup(const char* aircraft,const char* callsign);
void diagPrint(Print& out);
const char* diagResetReason();
bool backgroundNetworkReady();
void deferBackgroundNetwork(uint32_t ms=750);
class DiagRequest {
  DiagOp op_; uint32_t started_; int code_=0; DiagStage result_=DS_FAILED;
public:
  explicit DiagRequest(DiagOp op);
  ~DiagRequest();
  void stage(DiagStage stage);
  void http(int code);
  void finish(bool ok,DiagStage result=DS_DONE){result_=ok?result:DS_FAILED;}
};
