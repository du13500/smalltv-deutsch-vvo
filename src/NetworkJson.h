#pragma once
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include "Diagnostics.h"

// Keep a reserve for the SDK, TCP and temporary Strings while TLS is alive.
// Returning nullptr lets ArduinoJson report NoMemory instead of exhausting RAM.
class NetworkJsonAllocator : public ArduinoJson::Allocator {
  struct alignas(max_align_t) Header { size_t size; };
public:
  static constexpr size_t reserveBytes = 3072;
  void* allocate(size_t size) override {
    if (size > SIZE_MAX-sizeof(Header)) return nullptr;
    size_t total=size+sizeof(Header);
    if (total > ESP.getMaxFreeBlockSize() || ESP.getFreeHeap() < reserveBytes || total > ESP.getFreeHeap()-reserveBytes) return nullptr;
    auto* h=static_cast<Header*>(malloc(total));
    if (!h) return nullptr;
    h->size=size; return h+1;
  }
  void deallocate(void* ptr) override { if(ptr) free(static_cast<Header*>(ptr)-1); }
  void* reallocate(void* ptr, size_t size) override {
    if (!ptr) return allocate(size);
    auto* h=static_cast<Header*>(ptr)-1;
    // ArduinoJson assumes shrinking cannot fail. Keeping the allocation is safe.
    if (size <= h->size) return ptr;
    if (size > SIZE_MAX-sizeof(Header) || ESP.getFreeHeap() < reserveBytes || size-h->size > ESP.getFreeHeap()-reserveBytes) return nullptr;
    auto* next=static_cast<Header*>(realloc(h,size+sizeof(Header)));
    if(!next) return nullptr;
    next->size=size;return next+1;
  }
};

inline ArduinoJson::Allocator* networkJsonAllocator() {
  static NetworkJsonAllocator allocator;
  return &allocator;
}
inline bool jsonFilterReady(const JsonDocument& filter, DiagRequest& trace) {
  if (!filter.overflowed()) return true;
  trace.stage(DS_JSON_MEMORY); return false;
}
inline void logJsonError(DiagRequest& trace, DeserializationError err) {
  if (!err) return;
  switch (err.code()) {
    case DeserializationError::NoMemory: trace.stage(DS_JSON_MEMORY); break;
    case DeserializationError::IncompleteInput: trace.stage(DS_JSON_INCOMPLETE); break;
    case DeserializationError::EmptyInput: trace.stage(DS_JSON_EMPTY); break;
    case DeserializationError::TooDeep: trace.stage(DS_JSON_DEPTH); break;
    default: trace.stage(DS_JSON_INVALID); break;
  }
}
