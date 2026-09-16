// NotifyTypes.h — the presets behind the API's "type" field.
//
// A type is nothing but a set of defaults: the word above the label, the accent
// colour, which animation runs, how long it holds, and how hard it pushes past
// whatever else is queued. Every one of those can be overridden per request, so
// the table decides what a caller gets when it says nothing, never what it is
// allowed to ask for.
//
// Adding a type is one row. Nothing else in the feature indexes into this table.
//
// Included from NotifyMode.cpp only — the table is `static`, so a second
// includer would get a second copy of it in RAM for no reason.
#pragma once
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "NotifyAnims.h"

typedef struct {
  const char* name;     // what the API's "type" field matches on
  const char* word;     // the line above the label
  uint16_t    color;    // RGB565, UNTINTED — gfxTint() is applied at draw time
  uint8_t     anim;     // index into notify_anim_table
  uint16_t    ttlSec;
  uint8_t     prio;
} NotifyType;

// Colours are raw literals rather than the C_* macros from Gfx.h on purpose:
// those are gfxTint() calls, which cannot initialise a static table and would
// bake in the panel correction that the Display tab is allowed to change while
// the device runs.
//
// 0xDBAA is the cream the Clawdmeter art uses for its own text; done/waiting
// keep it so a device upgrading from the two-state endpoint looks unchanged.
static const NotifyType notify_types[] = {
  //  name       word         colour  animation             ttl  priority
  {"info",    "INFO",      0x041F, NOTIFY_ANIM_ID_INFO,     20, NOTIFY_PRIO_LOW},
  {"warning", "WARNING",   0xFFE0, NOTIFY_ANIM_ID_WARN,     30, NOTIFY_PRIO_MED},
  {"alert",   "ALERT",     0xF800, NOTIFY_ANIM_ID_BANG,     45, NOTIFY_PRIO_HIGH},
  {"done",    "TASK DONE", 0xDBAA, NOTIFY_ANIM_ID_JUMPING,  20, NOTIFY_PRIO_LOW},
  {"waiting", "NEEDS YOU", 0xDBAA, NOTIFY_ANIM_ID_WAVING,   20, NOTIFY_PRIO_MED},
};
#define NOTIFY_TYPE_COUNT (sizeof(notify_types) / sizeof(notify_types[0]))
#define NOTIFY_TYPE_DEFAULT 0   // "info": what a request with no type at all gets

static inline int notifyTypeFind(const char* name) {
  if (!name || !*name) return -1;
  for (size_t i = 0; i < NOTIFY_TYPE_COUNT; i++) {
    if (!strcasecmp(name, notify_types[i].name)) return (int)i;
  }
  return -1;
}
