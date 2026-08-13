// NotifyMode.h — transient full-screen attention overlay. Armed over HTTP, kept
// out of the main.cpp mode registry, never persisted.
#pragma once
#include "Mode.h"
#include "config.h"

class NotifyMode : public DisplayMode {
 public:
  const char* id() const override { return "notify"; }
  uint8_t     modeConst() const override { return MODE_NOTIFY; }

  void service(const Settings& s) override;

  bool     request(const char* state, uint32_t ttlSec);
  bool     active() const;
  uint32_t heldMs() const { return millis() - startedMs_; }

 private:
  void draw(bool full);

  uint8_t  anim_ = 0;
  uint32_t startedMs_ = 0;
  uint32_t untilMs_ = 0;
  uint16_t frame_ = 0;
  uint32_t frameStartMs_ = 0;
  bool     armed_ = false;
  bool     primed_ = false;
};

extern NotifyMode g_notifyMode;
