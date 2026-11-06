#pragma once
#include <stdint.h>
// Completion timestamps cannot point one millisecond into the future.
struct PollPolicy {
  uint32_t completed=0; uint8_t failures=0; bool attempted=false;
  void reset(){completed=0;failures=0;attempted=false;}
  uint32_t retryMs() const {return 15000UL << (failures>3?3:(failures?failures-1:0));}
  bool due(uint32_t now,uint32_t normalMs) const {
    return !attempted || (uint32_t)(now-completed)>=(failures?retryMs():normalMs);
  }
  void finish(uint32_t now,bool ok){completed=now;attempted=true;if(ok)failures=0;else if(failures<4)++failures;}
};
