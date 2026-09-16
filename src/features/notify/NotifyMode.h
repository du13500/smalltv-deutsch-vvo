// NotifyMode.h — transient full-screen attention overlay. Armed over HTTP, kept
// out of the main.cpp mode registry, never persisted.
#pragma once
#include "Mode.h"
#include "Gfx.h"
#include "config.h"

#define NOTIFY_LABEL_SIZE  2
#define NOTIFY_LABEL_MAX   (TFT_WIDTH / (GFX_FONT_W * NOTIFY_LABEL_SIZE))

class NotifyMode : public DisplayMode {
 public:
  const char* id() const override { return "notify"; }
  uint8_t     modeConst() const override { return MODE_NOTIFY; }

  void service(const Settings& s) override;

  bool     request(const char* state, uint32_t ttlSec, const char* label);
  bool     active() const;
  // How long the overlay has covered the panel, from the request that put it up
  // rather than the last one that refreshed it. Capped at the expiry so the few
  // milliseconds before main.cpp notices it has ended are not credited too.
  uint32_t heldMs() const {
    const uint32_t now = millis();
    return (((int32_t)(now - untilMs_) >= 0) ? untilMs_ : now) - runStartedMs_;
  }

 private:
  void draw(bool full);
  void setLabel(const char* label);

  char     label_[NOTIFY_LABEL_MAX + 1] = {0};
  uint8_t  anim_ = 0;
  uint32_t startedMs_ = 0;
  uint32_t runStartedMs_ = 0;   // first request of an unbroken run
  uint32_t untilMs_ = 0;
  uint16_t frame_ = 0;
  uint32_t frameStartMs_ = 0;
  bool     armed_ = false;
  bool     primed_ = false;
};

extern NotifyMode g_notifyMode;
