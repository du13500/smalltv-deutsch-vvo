// HaScreens.h — the Home Assistant screen store (features/ha).
//
// Home Assistant (or any MQTT client) pushes one full 240x240 screen per slot
// as a retained JSON draw list on smalltv/<hostname>/screen/<slot>. This store
// keeps the parsed result as fixed-size structs — no JsonDocument is retained —
// packed and sorted by slot name, so iterating it IS the carousel order
// (lexicographic). Limits come from config.h (HA_MAX_SCREENS / HA_MAX_PRIMS /
// HA_TEXT_POOL), per chip family.
//
// Screens persist to LittleFS /ha_screens.json, written with a debounce after
// the last change, so they survive a reboot; the broker's retained copies
// re-arrive on the next connect anyway and simply overwrite them.
#pragma once
#include <Arduino.h>
#include "config.h"

// Draw primitive kinds (the "t" field in the screen JSON).
enum HaPrimType : uint8_t {
  HA_P_FILL,    // c
  HA_P_RECT,    // x y w h c     (w/h stored in x2/y2)
  HA_P_RRECT,   // x y w h r c   (r in aux, w/h in x2/y2)
  HA_P_CIRCLE,  // x y r c       (centre x/y, r in aux)
  HA_P_LINE,    // x y x2 y2 c
  HA_P_TEXT,    // x y s c a v   (s in aux, a in align, v in the text pool)
  HA_P_ICON,    // x y s c a v   (like text, but v is an icon name from HaIcons)
};

enum HaAlign : uint8_t { HA_A_LEFT, HA_A_CENTER, HA_A_RIGHT };

// One draw primitive, 16 bytes. Text payloads live in the owning screen's
// text pool (voff indexes it); a screen's pool is bounded by HA_TEXT_POOL and
// in practice by the MQTT payload cap, which is smaller than an all-text
// screen at the 64-char string limit.
struct HaPrim {
  uint8_t  type;        // HaPrimType
  uint8_t  aux;         // text: font scale (>=1); circle/rrect: radius
  uint8_t  align;       // text only: HaAlign
  uint8_t  _pad;
  int16_t  x, y;        // rect/rrect: top-left; circle: centre; text: anchor
  int16_t  x2, y2;      // line: end point; rect/rrect: w / h
  uint16_t color;       // RGB565
  uint16_t voff;        // text only: offset into the screen's text pool
};
static_assert(sizeof(HaPrim) == 16, "HaPrim must stay compact");

struct HaScreen {
  char     name[HA_SLOT_LEN];   // slot = topic suffix, used as-is
  uint16_t bg;                  // RGB565 background (default black)
  uint32_t expiresAt;           // millis() deadline; 0 = sticky (ttl 0)
  uint16_t textUsed;            // bytes used in text[]
  uint8_t  primCount;
  HaPrim   prims[HA_MAX_PRIMS];
  char     text[HA_TEXT_POOL];  // NUL-terminated strings referenced by voff
};

void haScreensBegin();     // load /ha_screens.json (idempotent)
void haScreensService();   // debounced persist; call every loop tick
void haScreensReload();    // re-read the persisted file (settings re-applied)

// MQTT entry point. `slot` is the topic suffix after the last '/';
// `payload` is NOT NUL-terminated. A zero-length payload deletes the slot —
// checked before any JSON parsing, per the docs contract. A payload that is
// not valid JSON leaves the slot exactly as it was; inside a valid payload,
// unknown fields are ignored and malformed primitives are skipped
// individually.
void haScreensApply(const char* slot, const uint8_t* payload, size_t len);

// Renderer view: the live (non-expired) screens in lexicographic slot order.
uint8_t haScreensLive();
const HaScreen* haScreenAt(uint8_t liveIndex);

// Set on any store change (set/delete/parse); the renderer takes it once per
// repaint so a screen pushed over MQTT shows up without waiting for rotation.
bool haScreensTakeDirty();
