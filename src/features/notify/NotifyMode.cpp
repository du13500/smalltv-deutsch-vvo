#include "NotifyMode.h"
#include <Arduino_GFX_Library.h>
#include <stdlib.h>
#include <string.h>
#include "Gfx.h"
#include "NotifyAnims.h"
#include "NotifyTypes.h"

NotifyMode g_notifyMode;

#define NOTIFY_TEXT_PAD   7
#define NOTIFY_LINE_H     (GFX_FONT_H * NOTIFY_LABEL_SIZE)
#define NOTIFY_LINE_GAP   4
#define NOTIFY_MASCOT_GAP NOTIFY_LINE_H
#define NOTIFY_BAND_TITLE (NOTIFY_TEXT_PAD + NOTIFY_LINE_H + NOTIFY_TEXT_PAD)
#define NOTIFY_BAND_LABEL (NOTIFY_MASCOT_GAP + NOTIFY_LINE_H + NOTIFY_LINE_GAP + \
                           NOTIFY_LINE_H + NOTIFY_TEXT_PAD)
#define NOTIFY_LABEL_Y    (TFT_HEIGHT - NOTIFY_TEXT_PAD - NOTIFY_LINE_H)
#define NOTIFY_LABEL_AVAIL (TFT_WIDTH - 2 * NOTIFY_TEXT_PAD)

// gfxDrawCentered is the only text helper the shared layer exposes, and the
// marquee needs to place a string at an arbitrary x — including off-screen on
// both sides. Arduino_GFX clips per character, so this is safe to call with a
// negative x or one past the right edge.
//
// `bg` is what keeps the marquee from flickering: with a background colour set,
// each glyph paints its own cell opaque, so a moving string overwrites its old
// position in place. Clearing the band first and redrawing, which is the obvious
// way to do it, puts a blank line on screen 25 times a second.
static void drawAt(Arduino_GFX* gfx, const char* s, int x, int y, uint8_t size,
                   uint16_t color, uint16_t bg) {
  gfx->setTextSize(size);
  gfx->setTextColor(color, bg);
  gfx->setCursor(x, y);
  gfx->print(s);
}

// Black out a horizontal run of the label band, clipped to the panel. Used only
// for the runs no glyph covers, which are already black — so nothing visibly
// blinks — but which must be cleared as the text slides out of them.
static void clearBandRun(Arduino_GFX* gfx, int x, int w, int y, int h) {
  if (x < 0) { w += x; x = 0; }
  if (w <= 0 || x >= TFT_WIDTH) return;
  if (x + w > TFT_WIDTH) w = TFT_WIDTH - x;
  gfx->fillRect(x, y, w, h, C_BLACK);
}

// "#rrggbb", "rrggbb", or one of the names below, to RGB565. Names exist so a
// shell one-liner does not have to know the 565 packing; hex exists so anything
// else is reachable.
static bool parseColor(const char* s, uint16_t* out) {
  static const struct { const char* name; uint16_t rgb; } kNamed[] = {
    {"white", 0xFFFF}, {"black", 0x0000}, {"red",     0xF800}, {"green", 0x07E0},
    {"blue",  0x041F}, {"yellow", 0xFFE0}, {"cyan",   0x07FF}, {"magenta", 0xF81F},
    {"orange", 0xFC00}, {"gray",  0x8410}, {"grey",   0x8410},
  };
  for (size_t i = 0; i < sizeof(kNamed) / sizeof(kNamed[0]); i++) {
    if (!strcasecmp(s, kNamed[i].name)) { *out = kNamed[i].rgb; return true; }
  }
  if (*s == '#') s++;

  // Six hex digits, checked one by one. strtoul is not a validator: it skips
  // leading whitespace and accepts a sign, so " 12345" and "-00001" both clear a
  // length test and then parse into a colour nobody asked for.
  unsigned long v = 0;
  for (int i = 0; i < 6; i++) {
    const char c = s[i];
    int d;
    if      (c >= '0' && c <= '9') d = c - '0';
    else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
    else return false;
    v = (v << 4) | (unsigned long)d;
  }
  if (s[6] != '\0') return false;

  *out = (uint16_t)((((v >> 19) & 0x1F) << 11) | (((v >> 10) & 0x3F) << 5) |
                    ((v >> 3) & 0x1F));
  return true;
}

// ---- rendering ------------------------------------------------------------

void NotifyMode::drawLabelBand(bool full) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx || !cur_.label[0]) return;
  const uint16_t accent = gfxTint(cur_.color);

  if (!scrollW_) {   // fits on the line: static, and only ever on a full repaint
    if (full) gfxDrawCentered(cur_.label, NOTIFY_LABEL_Y, NOTIFY_LABEL_SIZE, accent);
    return;
  }

  // Scrolling: draw the string twice, one span apart, so the tail of the run and
  // the head of the next are on screen together and the wrap has no visible
  // seam. Both are drawn with an opaque background, which overwrites the
  // previous position rather than needing the band cleared first.
  const int span = scrollW_ + NOTIFY_SCROLL_GAP_PX;
  const int x = NOTIFY_TEXT_PAD - scrollX_;
  drawAt(gfx, cur_.label, x, NOTIFY_LABEL_Y, NOTIFY_LABEL_SIZE, accent, C_BLACK);
  drawAt(gfx, cur_.label, x + span, NOTIFY_LABEL_Y, NOTIFY_LABEL_SIZE, accent, C_BLACK);

  // The glyphs cover their own cells and nothing else, so the runs between and
  // around the two copies are cleared separately as the text slides out of them.
  const int tail = x + span + scrollW_;
  clearBandRun(gfx, 0, x, NOTIFY_LABEL_Y, NOTIFY_LINE_H);
  clearBandRun(gfx, x + scrollW_, NOTIFY_SCROLL_GAP_PX, NOTIFY_LABEL_Y, NOTIFY_LINE_H);
  clearBandRun(gfx, tail, TFT_WIDTH - tail, NOTIFY_LABEL_Y, NOTIFY_LINE_H);
}

void NotifyMode::draw(bool full) {
  Arduino_GFX* gfx = gfxDev();
  if (!gfx) return;

  const NotifyAnimEntry& a = notify_anim_table[cur_.anim];
  const int aw = notifyAnimW(a), ah = notifyAnimH(a);
  const bool labelled = cur_.label[0] != '\0';
  const bool titled   = cur_.title[0] != '\0';

  // Reserve only the lines there is text for, so dropping the title gives the
  // animation the room back instead of leaving a gap.
  const int lines = (labelled ? 1 : 0) + (titled ? 1 : 0);
  const int band = (lines >= 2) ? NOTIFY_BAND_LABEL
                 : (lines == 1) ? NOTIFY_BAND_TITLE
                                : NOTIFY_TEXT_PAD;
  const int cell = min(TFT_WIDTH / aw, (TFT_HEIGHT - band) / ah);
  const int x0 = (TFT_WIDTH - aw * cell) / 2;
  const int y0 = (TFT_HEIGHT - band - ah * cell) / 2;
  const uint16_t accent = gfxTint(cur_.color);

  if (full) {
    gfx->fillScreen(C_BLACK);
    if (titled) {
      gfxDrawCentered(cur_.title,
                      labelled ? NOTIFY_LABEL_Y - NOTIFY_LINE_H - NOTIFY_LINE_GAP
                               : NOTIFY_LABEL_Y,
                      NOTIFY_LABEL_SIZE, accent);
    }
    drawLabelBand(true);
  }

  if (!a.pixels) {
    notifyAnimRender(gfx, a, x0, y0, cell, frame_, accent);
    return;
  }

  // Pixel animation: run-length blit straight out of PROGMEM, one row at a time.
  const NotifyAnim& p = *a.pixels;
  const uint8_t* cells = p.frames + (uint32_t)frame_ * p.w * p.h;
  for (int gy = 0; gy < p.h; gy++) {
    const uint8_t* row = cells + gy * p.w;
    int runStart = 0;
    uint8_t runCode = pgm_read_byte(row);
    for (int gx = 1; gx <= p.w; gx++) {
      uint8_t code = (gx < p.w) ? pgm_read_byte(row + gx) : (uint8_t)0xFF;
      if (code == runCode) continue;
      gfx->fillRect(x0 + runStart * cell, y0 + gy * cell, (gx - runStart) * cell, cell,
                    runCode < NOTIFY_PALETTE_SIZE ? p.palette[runCode] : C_BLACK);
      runStart = gx;
      runCode  = code;
    }
  }
}

// ---- queue ----------------------------------------------------------------

void NotifyMode::copyText(char* dst, size_t cap, const char* src) {
  size_t n = 0;
  for (; src && *src && n + 1 < cap; src++) {
    if (*src >= 0x20 && *src <= 0x7E) dst[n++] = *src;
  }
  dst[n] = '\0';
}

void NotifyMode::startItem(const NotifyItem& it) {
  cur_ = it;

  // Decide once per item whether the label scrolls: it depends only on the text,
  // and gfxTextW on every frame would be wasted work.
  const int w = gfxTextW(cur_.label, NOTIFY_LABEL_SIZE);
  scrollW_ = (w > NOTIFY_LABEL_AVAIL) ? w : 0;
  scrollX_ = 0;

  startedMs_    = millis();
  untilMs_      = startedMs_ + (uint32_t)cur_.ttlSec * 1000UL;
  frame_        = 0;
  frameStartMs_ = startedMs_;
  scrollStepMs_ = startedMs_ + NOTIFY_SCROLL_PAUSE_MS;
  primed_       = false;

  if (!armed_) {            // the panel was not ours: this starts the run
    overlayStartedMs_ = startedMs_;
    armed_ = true;
  }
}

// Sorted highest-priority-first, FIFO among equals, so inserting is a search for
// the first entry this one outranks.
bool NotifyMode::enqueue(const NotifyItem& it) {
  if (qCount_ >= NOTIFY_QUEUE_DEPTH) {
    // Full. Evict the last entry this one outranks or ties — the queue is
    // sorted, so that is the least important thing waiting. If everything queued
    // is strictly more important, the arrival is what gives way.
    int victim = -1;
    for (int i = (int)qCount_ - 1; i >= 0; i--) {
      if (queue_[i].prio <= it.prio) { victim = i; break; }
    }
    if (victim < 0) return false;
    for (uint8_t i = (uint8_t)victim + 1; i < qCount_; i++) queue_[i - 1] = queue_[i];
    qCount_--;
  }

  uint8_t at = qCount_;
  for (uint8_t i = 0; i < qCount_; i++) {
    if (queue_[i].prio < it.prio) { at = i; break; }
  }
  for (uint8_t i = qCount_; i > at; i--) queue_[i] = queue_[i - 1];
  queue_[at] = it;
  qCount_++;
  return true;
}

void NotifyMode::advance() {
  if (qCount_ == 0) { retire(); return; }
  const NotifyItem next = queue_[0];
  for (uint8_t i = 1; i < qCount_; i++) queue_[i - 1] = queue_[i];
  qCount_--;
  startItem(next);
}

bool NotifyMode::owns() const {
  return armed_ && (qCount_ > 0 || (int32_t)(millis() - untilMs_) < 0);
}

uint32_t NotifyMode::heldMs() const {
  if (!armed_) return lastHeldMs_;
  // Cap at the current item's own expiry. When service() runs normally the two
  // are the same; when it was starved, wall clock kept moving while the panel
  // showed something else entirely, and crediting that would hand the carousel
  // back time the overlay never took.
  const uint32_t now = millis();
  const uint32_t end = ((int32_t)(now - untilMs_) >= 0) ? untilMs_ : now;
  return end - overlayStartedMs_;
}

void NotifyMode::retire() {
  if (!armed_) return;
  lastHeldMs_ = heldMs();   // freeze before clearing armed_
  armed_ = false;
}

// Two requests describe the same event when they name the same preset and carry
// the same label. That is the heartbeat case: a script re-firing every few
// seconds to say "still waiting" means refresh, not another overlay.
bool NotifyMode::sameEvent(const NotifyItem& a, const NotifyItem& b) {
  return a.type == b.type && !strcmp(a.label, b.label);
}

NotifyResult NotifyMode::request(const NotifyRequest& r) {
  int t = notifyTypeFind(r.type);
  if (t < 0) {
    if (r.type && *r.type) return NOTIFY_REJECTED;   // a type that does not exist
    t = NOTIFY_TYPE_DEFAULT;
  }
  const NotifyType& preset = notify_types[t];

  NotifyItem it;
  it.type   = (uint8_t)t;
  it.anim   = preset.anim;
  it.color  = preset.color;
  it.prio   = preset.prio;
  it.ttlSec = preset.ttlSec;

  // Each override is independent: a request can take the preset's animation and
  // its own colour, or keep everything and only stretch the hold time.
  if (r.anim && *r.anim) {
    const int a = notifyAnimFind(r.anim);
    if (a < 0) return NOTIFY_REJECTED;
    it.anim = (uint8_t)a;
  }
  if (r.color && *r.color) {
    uint16_t c;
    if (!parseColor(r.color, &c)) return NOTIFY_REJECTED;
    it.color = c;
  }
  if (r.prio >= 0) it.prio = (uint8_t)(r.prio > NOTIFY_PRIO_MAX ? NOTIFY_PRIO_MAX : r.prio);
  if (r.ttlSec) it.ttlSec = (uint16_t)(r.ttlSec > NOTIFY_TTL_MAX_SEC ? NOTIFY_TTL_MAX_SEC
                                                                     : r.ttlSec);
  if (it.ttlSec < NOTIFY_TTL_MIN_SEC) it.ttlSec = NOTIFY_TTL_MIN_SEC;

  copyText(it.label, sizeof(it.label), r.label);

  // The preset's word is a fallback, not a header. Printing "INFO" above a label
  // says nothing the colour and the animation have not already said, and costs a
  // line the message could use. It appears only when there is no label at all,
  // which is also what keeps a bare {"state":"done"} showing TASK DONE.
  if (r.title && *r.title)      copyText(it.title, sizeof(it.title), r.title);
  else if (it.label[0] == '\0') copyText(it.title, sizeof(it.title), preset.word);

  // A run that has finished but whose service() never got to clear it (safe
  // mode, setup mode) must not make this look like an overlay is up.
  if (!owns()) retire();

  // A repeat of what is on screen refreshes it in place. This is what the
  // endpoint is mostly used for: a session heartbeat re-firing the same waiting
  // notice every few seconds. Queueing those would hold the panel for the sum of
  // their hold times and then replay events the session has long moved past.
  if (armed_ && sameEvent(it, cur_)) { startItem(it); return NOTIFY_ACCEPTED; }

  // Same for a repeat of something already waiting: refresh it where it sits
  // rather than letting a heartbeat fill the queue behind a long alert.
  for (uint8_t i = 0; i < qCount_; i++) {
    if (sameEvent(it, queue_[i])) { queue_[i] = it; return NOTIFY_ACCEPTED; }
  }

  // Nothing on the panel, or something less important than this: show it now.
  // The pre-empted overlay is dropped rather than requeued — it has already had
  // its time on screen, and what arrived outranks it.
  if (!armed_ || it.prio > cur_.prio) { startItem(it); return NOTIFY_ACCEPTED; }
  return enqueue(it) ? NOTIFY_ACCEPTED : NOTIFY_QUEUE_FULL;
}

// ---- service --------------------------------------------------------------

void NotifyMode::service(const Settings& s) {
  (void)s;
  if (!armed_) return;
  if (!owns()) { retire(); return; }   // ran out while we were not being called

  if ((int32_t)(millis() - untilMs_) >= 0) {
    advance();
    if (!armed_) return;   // queue drained: main.cpp restores the feature below
  }

  const NotifyAnimEntry& a = notify_anim_table[cur_.anim];
  if (!primed_) {
    primed_ = true;
    draw(true);
    return;
  }

  const uint32_t now = millis();
  if (now - frameStartMs_ >= notifyAnimHold(a, frame_)) {
    frame_ = (uint16_t)((frame_ + 1) % notifyAnimFrames(a));
    frameStartMs_ = now;
    draw(false);
  }
  if (scrollW_ && (int32_t)(now - scrollStepMs_) >= 0) {
    scrollX_ += NOTIFY_SCROLL_STEP_PX;
    if (scrollX_ >= scrollW_ + NOTIFY_SCROLL_GAP_PX) scrollX_ = 0;
    scrollStepMs_ = now + NOTIFY_SCROLL_MS;
    drawLabelBand(false);
  }
}
