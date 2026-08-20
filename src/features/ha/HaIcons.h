// HaIcons.h — vector icon set for the HA screens feature (features/ha).
//
// The "icon" draw primitive renders one of a fixed set of named glyphs, each
// defined in code on a normalized 24x24 grid and scaled by an integer s — no
// bitmap or font assets, so the flash cost is a few KB of drawing calls.
// The renderer resolves the anchor (a) and passes the resulting top-left in.
#pragma once
#include <Arduino.h>

// Draw icon `name` with its 24x24 logical box scaled by s at top-left (x,y).
// Rendered size is 24*s px; strokes are max(1,s) px thick. Returns false when
// the name is unknown — the caller skips it silently (forgiving-parse
// contract), so nothing is drawn in that case.
bool haDrawIcon(const char* name, int x, int y, int s, uint16_t color);
