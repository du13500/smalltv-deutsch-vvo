#include "NotifyMode.h"
#include <Arduino_GFX_Library.h>
#include "Gfx.h"
#include "notify_frames.h"

NotifyMode g_notifyMode;

#define NOTIFY_LABEL_BAND 30
#define NOTIFY_LABEL_SIZE 2

static const char* animLabel(uint8_t anim) {
  return anim == NOTIFY_ANIM_WAITING ? "NEEDS YOU" : "TASK DONE";
}

void NotifyMode::draw(bool full) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;

  const NotifyAnim& a = notify_anims[anim_];
  const int cell = min(TFT_WIDTH / a.w, (TFT_HEIGHT - NOTIFY_LABEL_BAND) / a.h);
  const int x0 = (TFT_WIDTH - a.w * cell) / 2;
  const int y0 = (TFT_HEIGHT - NOTIFY_LABEL_BAND - a.h * cell) / 2;
  const uint8_t* cells = a.frames + (uint32_t)frame_ * a.w * a.h;

  if (full) {
    gfx->fillScreen(C_BLACK);
    gfxDrawCentered(animLabel(anim_), TFT_HEIGHT - NOTIFY_LABEL_BAND + 7,
                    NOTIFY_LABEL_SIZE, a.palette[1]);
  }

  for (int gy = 0; gy < a.h; gy++) {
    const uint8_t* row = cells + gy * a.w;
    int runStart = 0;
    uint8_t runCode = pgm_read_byte(row);
    for (int gx = 1; gx <= a.w; gx++) {
      uint8_t code = (gx < a.w) ? pgm_read_byte(row + gx) : (uint8_t)0xFF;
      if (code == runCode) continue;
      gfx->fillRect(x0 + runStart * cell, y0 + gy * cell, (gx - runStart) * cell, cell,
                    runCode < NOTIFY_PALETTE_SIZE ? a.palette[runCode] : C_BLACK);
      runStart = gx;
      runCode  = code;
    }
  }
}

bool NotifyMode::request(const char* state, uint32_t ttlSec) {
  if      (!strcmp(state, "done"))    anim_ = NOTIFY_ANIM_DONE;
  else if (!strcmp(state, "waiting")) anim_ = NOTIFY_ANIM_WAITING;
  else return false;

  if (ttlSec < NOTIFY_TTL_MIN_SEC) ttlSec = NOTIFY_TTL_MIN_SEC;
  if (ttlSec > NOTIFY_TTL_MAX_SEC) ttlSec = NOTIFY_TTL_MAX_SEC;

  startedMs_    = millis();
  untilMs_      = startedMs_ + ttlSec * 1000UL;
  frame_        = 0;
  frameStartMs_ = startedMs_;
  primed_       = false;
  armed_        = true;
  return true;
}

bool NotifyMode::active() const {
  return armed_ && (int32_t)(millis() - untilMs_) < 0;
}

void NotifyMode::service(const Settings& s) {
  const NotifyAnim& a = notify_anims[anim_];
  if (!primed_) {
    primed_ = true;
    draw(true);
    return;
  }
  if (millis() - frameStartMs_ >= a.holds[frame_]) {
    frame_ = (frame_ + 1) % a.frame_count;
    frameStartMs_ = millis();
    draw(false);
  }
}
