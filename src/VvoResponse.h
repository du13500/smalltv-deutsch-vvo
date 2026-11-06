#pragma once
#include <cstring>
// Only this exact provider response represents a known, unserved stop.
inline bool vvoResponseNotServed(const char* code,const char* message) {
  return code && message && !strcmp(code,"ServiceError") && !strcmp(message,"no serving lines found");
}
