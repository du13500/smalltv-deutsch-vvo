#include "NotifyAnims.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <string.h>
#include <strings.h>   // strcasecmp
#include "Gfx.h"

// ---- shape predicates -----------------------------------------------------
// Everything below is integer-only and squared-distance based: these run once
// per pixel per frame (~30k calls at 24x24 units on a 168 px box), and the
// ESP8266 has no FPU.

static uint16_t dim565(uint16_t c, int num, int den) {
  if (num <= 0) return 0;
  const uint16_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return (uint16_t)((r * num / den) << 11 | (g * num / den) << 5 | (b * num / den));
}

static inline bool inDisc(int dx, int dy, int r) {
  return dx * dx + dy * dy <= r * r;
}

static inline bool inRing(int dx, int dy, int rIn, int rOut) {
  const int d = dx * dx + dy * dy;
  return d >= rIn * rIn && d <= rOut * rOut;
}

static inline bool inRect(int x, int y, int rx, int ry, int rw, int rh) {
  return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

// The two glyphs the badges need, as rectangles rather than the scaled 6x8 font:
// at this size the font's stair-stepping is visible and a bar reads cleaner.
// Both are 2u wide and 11u tall, so callers centre them the same way.
#define GLYPH_H_UNITS 11

static inline bool inGlyphBang(int x, int y, int cx, int top, int u) {
  return inRect(x, y, cx - u, top, 2 * u, 7 * u) ||
         inRect(x, y, cx - u, top + 9 * u, 2 * u, 2 * u);
}

static inline bool inGlyphInfo(int x, int y, int cx, int top, int u) {
  return inRect(x, y, cx - u, top, 2 * u, 2 * u) ||
         inRect(x, y, cx - u, top + 4 * u, 2 * u, 7 * u);
}

// Stroke width for rings, never below one pixel on a small box.
static inline int stroke(int u) { return u / 2 > 0 ? u / 2 : 1; }

// The warning triangle, grown outward by `g` so the same test serves the body
// (g = 0) and the outline emanating from it.
static inline bool inTri(int x, int y, int cx, int cy, int u, int g) {
  const int apexY = cy - 8 * u - g, baseY = cy + 8 * u + g;
  if (y < apexY || y > baseY) return false;
  const int half = (10 * u + g) * (y - apexY) / (baseY - apexY);
  return x >= cx - half && x <= cx + half;
}

// ---- procedural animations ------------------------------------------------
// Each works on a 24x24 unit box. To add one: write the function, then add a row
// to notify_anim_table below and an id next to the others in NotifyAnims.h.

// INFO — a steady badge with a halo that swells outward and restarts. Calm on
// purpose: this is the one that fires for things nobody has to act on.
static uint16_t pxInfo(int x, int y, const NotifyPixelCtx& c) {
  const int u = c.u, cx = c.w / 2, cy = c.h / 2;
  const int dx = x - cx, dy = y - cy;

  if (inDisc(dx, dy, 7 * u)) {
    return inGlyphInfo(x, y, cx, cy - GLYPH_H_UNITS * u / 2, u) ? c.bg : c.accent;
  }
  const int halo = 8 * u + (int)c.frame * u / 2;
  if (inRing(dx, dy, halo, halo + stroke(u))) {
    return dim565(c.accent, 7 - (int)(c.frame % 8), 12);
  }
  return c.bg;
}

// WARNING — a still road sign with an outline emanating from its edge.
//
// The body is a constant colour and never moves. Two earlier versions animated
// it and both read as a fault rather than an animation: first a two-state blink,
// then a gentler brightness ramp. Amplitude was never the problem — changing a
// large filled area at all is. Only the thin outline animates.
static uint16_t pxWarn(int x, int y, const NotifyPixelCtx& c) {
  const int u = c.u, cx = c.w / 2, cy = c.h / 2;

  if (inTri(x, y, cx, cy, u, 0)) {
    // A triangle narrows towards its apex and carries most of its area low, so
    // the glyph is neither centred (it looks adrift high) nor full height (its
    // foot lands on the base). Shrunk to 7/8 and hung 6u above the base: ~2u of
    // clearance underneath, and still well inside the edges at its top.
    const int ug = (u * 7) / 8 > 0 ? (u * 7) / 8 : 1;
    if (inGlyphBang(x, y, cx, cy + 6 * u - GLYPH_H_UNITS * ug, ug)) return c.bg;
    return c.accent;
  }

  // Growth is capped so the widest outline still lands inside the box: the base
  // is 10u of half-width, the box 12u, and the stroke takes the last half unit.
  const int phase = c.frame % 4;
  const int g = phase * u / 2;
  if (inTri(x, y, cx, cy, u, g + stroke(u)) && !inTri(x, y, cx, cy, u, g)) {
    return dim565(c.accent, 8 - phase * 2, 12);
  }
  return c.bg;
}

// ALERT — a still badge throwing off rings, faster than the warning sign.
//
// Same shape as warn, same rule, different tempo: that is the whole difference
// between "look at this" and "act now". Two earlier versions flashed the disc and
// then shook it sideways; the flash strobed, and the shake tore — translating a
// 168 px disc while the panel scans out and we write row by row with no
// framebuffer shows the new position up top and the old one below it. Nothing
// large may change colour OR move.
static uint16_t pxBang(int x, int y, const NotifyPixelCtx& c) {
  const int u = c.u, cx = c.w / 2, cy = c.h / 2;
  const int dx = x - cx, dy = y - cy;

  if (inDisc(dx, dy, 9 * u)) {
    return inGlyphBang(x, y, cx, cy - GLYPH_H_UNITS * u / 2, u) ? c.bg : c.accent;
  }
  const int phase = c.frame % 4;
  const int r = 10 * u + phase * u / 2;
  if (inRing(dx, dy, r, r + stroke(u))) return dim565(c.accent, 8 - phase * 2, 12);
  return c.bg;
}

// PULSE — shape-free rings, for a type that wants attention without saying what
// kind. The generic fallback when a new preset has no art of its own yet.
static uint16_t pxPulse(int x, int y, const NotifyPixelCtx& c) {
  const int u = c.u, cx = c.w / 2, cy = c.h / 2;
  const int dx = x - cx, dy = y - cy;

  if (inDisc(dx, dy, 3 * u)) return c.accent;
  for (int k = 0; k < 3; k++) {
    const int phase = (c.frame + k * 3) % 8;
    const int r = 4 * u + phase * u;
    if (inRing(dx, dy, r, r + stroke(u))) return dim565(c.accent, 8 - phase, 12);
  }
  return c.bg;
}

// ---- the registry ---------------------------------------------------------
// Pixel rows point into the generated table; procedural rows carry a function.
// The renderer reads both through the accessors below and never branches on kind
// beyond "is `pixels` NULL".

const NotifyAnimEntry notify_anim_table[NOTIFY_ANIM_ID_COUNT] = {
  //  name       pixel frames                        procedural   w   h  fr   ms
  {"jumping", &notify_anims[NOTIFY_ANIM_DONE],    nullptr,  0,  0, 0,   0},
  {"waving",  &notify_anims[NOTIFY_ANIM_WAITING], nullptr,  0,  0, 0,   0},
  {"info",    nullptr,                            pxInfo,  24, 24, 8, 110},
  {"warn",    nullptr,                            pxWarn,  24, 24, 4, 220},
  {"bang",    nullptr,                            pxBang,  24, 24, 4,  80},
  {"pulse",   nullptr,                            pxPulse, 24, 24, 8,  90},
};

uint8_t notifyAnimW(const NotifyAnimEntry& e) {
  return e.pixels ? e.pixels->w : e.procW;
}
uint8_t notifyAnimH(const NotifyAnimEntry& e) {
  return e.pixels ? e.pixels->h : e.procH;
}
uint16_t notifyAnimFrames(const NotifyAnimEntry& e) {
  return e.pixels ? e.pixels->frame_count : e.procFrames;
}
uint16_t notifyAnimHold(const NotifyAnimEntry& e, uint16_t frame) {
  if (!e.pixels) return e.procPeriodMs;
  return e.pixels->holds[frame % e.pixels->frame_count];
}

void notifyAnimRender(Arduino_GFX* gfx, const NotifyAnimEntry& e, int x0, int y0,
                      int cell, uint16_t frame, uint16_t accent) {
  NotifyPixelCtx c;
  c.w      = e.procW * cell;
  c.h      = e.procH * cell;
  c.u      = cell;           // the box is procW units wide, one unit per cell
  c.frame  = frame;
  c.accent = accent;
  c.bg     = C_BLACK;        // hoisted: gfxTint() must not run once per pixel

  // One row at a time out of the stack. A whole-box buffer would be 56 KB, which
  // the ESP8266 does not have; a row is 480 B at the widest the panel can be.
  uint16_t row[TFT_WIDTH];
  const int w = c.w > TFT_WIDTH ? TFT_WIDTH : c.w;
  for (int y = 0; y < c.h; y++) {
    for (int x = 0; x < w; x++) row[x] = e.pixel(x, y, c);
    gfx->draw16bitRGBBitmap(x0, y0 + y, row, w, 1);
  }
}

int notifyAnimFind(const char* name) {
  if (!name || !*name) return -1;
  for (int i = 0; i < NOTIFY_ANIM_ID_COUNT; i++) {
    if (!strcasecmp(name, notify_anim_table[i].name)) return i;
  }
  // The endpoint shipped with the animations named after the two states it had.
  // Scripts written against that still work.
  if (!strcasecmp(name, "done"))    return NOTIFY_ANIM_ID_JUMPING;
  if (!strcasecmp(name, "waiting")) return NOTIFY_ANIM_ID_WAVING;
  return -1;
}
