// UsageMode.h — Claude usage meter feature.
//
// Shows 5h/7d usage bars + a small mascot when data is flowing, and an animated
// pixel-art mascot when the daemon goes quiet. Owns its fetch (UsageClient), its
// mascot animation (Mascot) and its render/dirty state.
#pragma once
#include "Mode.h"
#include "config.h"
#include "UsageData.h"

class UsageMode : public DisplayMode {
 public:
  const char* id() const override { return "usage"; }
  uint8_t     modeConst() const override { return MODE_USAGE; }

  void begin(const Settings& s) override;
  void service(const Settings& s) override;
  void invalidate(const Settings& s) override;
  // repaint only (no refetch) — another mode may have drawn over the screen since,
  // so force the full layout back in, not just a value diff
  void wake(const Settings& s) override { needRender_ = true; layoutPrimed_ = false; }

 private:
  bool contentChanged(const UsageData& u) const;
  void rememberContent(const UsageData& u);

  uint32_t usageSampled_ = 0;              // lastOkMs already fed to the mascot tracker
  uint32_t usageRenderedOk_ = 0xFFFFFFFF;
  bool     showingMascot_ = false;
  bool     needRender_ = true;

  // Last content actually painted to the screen. The daemon re-POSTs on a fixed
  // interval even when nothing changed (it doesn't know the device's redraw
  // cost), so a full-screen redraw is only warranted when one of these differs
  // from what's currently on screen — not on every fresh lastOkMs.
  bool     contentPrimed_ = false;

  // Whether the static screen layout (black bg + header) is currently painted, so
  // a routine value update can skip the full-screen clear that causes the flash.
  bool     layoutPrimed_ = false;
  float    lastSessionPct_ = -1;
  float    lastWeeklyPct_ = -1;
  int      lastSessionResetMin_ = -1;
  int      lastWeeklyResetMin_ = -1;
  char     lastStatus_[16] = {0};
  bool     lastValid_ = false;
  bool     lastError_ = false;
};

extern UsageMode g_usageMode;
