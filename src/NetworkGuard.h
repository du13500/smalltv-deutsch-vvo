#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include "RadarPolicy.h"
#include "Diagnostics.h"
static inline bool networkHeapReady(DiagOp op,uint16_t rx=4096){
 bool ok=radarTlsFits(ESP.getFreeHeap(),ESP.getMaxFreeBlockSize(),rx,sizeof(br_ssl_client_context));
 if(!ok)diagEvent(op,DS_HEAP,0,radarTlsBudget(rx,sizeof(br_ssl_client_context)).total);
 return ok;
}

// Use only after WiFiClientSecure construction: its shared 6,200-byte stack
// is already allocated. Buffers, context and reserve are still required.
static inline bool networkPreparedHeapReady(DiagOp op,uint16_t rx=4096){
 RadarTlsBudget budget=radarTlsBudget(rx,sizeof(br_ssl_client_context));
 budget.total-=NETWORK_TLS_STACK_BYTES;
 const uint32_t input=(uint32_t)rx+325;
 const uint32_t context=sizeof(br_ssl_client_context)+256;
 budget.block=(input>context?input:context)+256;
 bool ok=ESP.getFreeHeap()>=budget.total&&ESP.getMaxFreeBlockSize()>=budget.block;
 if(!ok)diagEvent(op,DS_HEAP,0,budget.total);
 return ok;
}

// Only use a small receive buffer when the server explicitly supports MFLN.
static inline uint16_t networkTlsRx(const char* host,uint16_t& cached,DiagOp op) {
 if(cached)return cached;
 if(!networkHeapReady(op,4096))return 4096;
 cached=BearSSL::WiFiClientSecure::probeMaxFragmentLength(host,443,512)?512:4096;
 return cached;
}
