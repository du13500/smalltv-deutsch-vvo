// HaIcons.cpp — see HaIcons.h for the contract this implements.
//
// Every icon is a small function drawing into a 24x24 grid through the scaled
// helpers below: grid coords are multiplied by the scale s, and strokes are
// s px thick so shapes don't vanish at larger scales. Static state (G/S/C/OX/
// OY) keeps the per-icon signatures empty — rendering is single-threaded, so
// this is safe and keeps the dispatch table tiny.
#include "HaIcons.h"
#include "config.h"
#if WITH_HA
#include <Arduino_GFX_Library.h>
#include "Gfx.h"

static Arduino_GFX* G;      // draw target for the current icon
static int          S;      // scale (1..8)
static uint16_t     C;      // already-tinted RGB565
static int          OX, OY; // resolved top-left of the 24*s box

static int X(int v) { return OX + v * S; }
static int Y(int v) { return OY + v * S; }

// Filled box from grid (x,y), w x h grid units.
static void box(int x, int y, int w, int h) {
  G->fillRect(X(x), Y(y), w * S, h * S, C);
}
// Horizontal / vertical strokes, thickness s, endpoints inclusive.
static void hline(int x1, int x2, int y) {
  G->fillRect(X(x1), Y(y), (x2 - x1 + 1) * S, S, C);
}
static void vline(int x, int y1, int y2) {
  G->fillRect(X(x), Y(y1), S, (y2 - y1 + 1) * S, C);
}
// Diagonal stroke, thickness s (two offset passes give a solid joint).
static void diag(int x1, int y1, int x2, int y2) {
  for (int k = 0; k < S; k++) {
    G->drawLine(X(x1) + k, Y(y1), X(x2) + k, Y(y2), C);
    if (k) G->drawLine(X(x1), Y(y1) + k, X(x2), Y(y2) + k, C);
  }
}
static void disc(int cx, int cy, int r) { G->fillCircle(X(cx), Y(cy), r * S, C); }
static void tri(int x1, int y1, int x2, int y2, int x3, int y3) {
  G->fillTriangle(X(x1), Y(y1), X(x2), Y(y2), X(x3), Y(y3), C);
}
static void rbox(int x, int y, int w, int h, int r) {
  G->fillRoundRect(X(x), Y(y), w * S, h * S, r * S, C);
}

// ---------------------------------------------------------------------------
// The icon set. Names are the fixed contract the docs are written against.
// ---------------------------------------------------------------------------
static void ic_thermometer() {
  box(10, 2, 4, 14);        // stem
  disc(12, 17, 5);          // bulb
}

static void ic_humidity() {  // droplet: pointed top over a round bottom
  tri(12, 2, 6, 12, 18, 12);
  disc(12, 14, 6);
}

static void ic_sun() {
  disc(12, 12, 5);
  hline(1, 4, 12); hline(20, 23, 12);
  vline(12, 1, 4); vline(12, 20, 23);
  diag(4, 4, 6, 6);   diag(18, 18, 20, 20);
  diag(20, 4, 18, 6); diag(4, 20, 6, 18);
}

static void cloudBody() {   // shared by cloud / rain / snow
  disc(9, 13, 4);
  disc(15, 12, 5);
  box(5, 13, 14, 6);        // flat base
}

static void ic_cloud() { cloudBody(); }

static void ic_rain() {
  cloudBody();
  diag(8, 19, 7, 22);
  diag(12, 19, 11, 22);
  diag(16, 19, 15, 22);
}

static void flake(int x, int y) {  // small plus-shaped flake
  hline(x - 1, x + 1, y);
  vline(x, y - 1, y + 1);
}

static void ic_snow() {
  cloudBody();
  flake(8, 20);
  flake(12, 21);
  flake(16, 20);
}

static void ic_wind() {     // three streaks with curled ends
  hline(2, 14, 7);  vline(14, 5, 7);
  hline(6, 21, 12); vline(21, 12, 14);
  hline(2, 14, 17); vline(14, 17, 19);
}

static void ic_home() {
  tri(12, 3, 2, 12, 22, 12);   // roof
  box(5, 12, 14, 3);           // lintel above the door gap
  box(5, 15, 5, 6);            // left wall
  box(14, 15, 5, 6);           // right wall (door gap x 10..13 shows through)
}

static void ic_door_open() {  // frame outline + swung-open panel
  hline(5, 19, 4);
  vline(5, 4, 21); vline(19, 4, 21);
  tri(7, 6, 7, 19, 15, 15);    // panel, hinged on the left jamb
  tri(7, 6, 15, 15, 15, 9);
  disc(9, 12, 1);              // knob
}

static void ic_door_closed() {  // frame outline + shut panel outline + knob
  hline(5, 19, 4);
  vline(5, 4, 21); vline(19, 4, 21);
  hline(8, 16, 7); hline(8, 16, 18);
  vline(8, 7, 18); vline(16, 7, 18);
  disc(14, 13, 1);
}

static void ic_window_open() {  // frame + tilted sash hinged on the left
  hline(3, 21, 4); hline(3, 21, 20);
  vline(3, 4, 20); vline(21, 4, 20);
  tri(6, 7, 6, 17, 17, 13);    // sash
  tri(6, 7, 17, 13, 17, 11);
}

static void ic_window_closed() {  // frame with a cross mullion
  hline(3, 21, 4); hline(3, 21, 20);
  vline(3, 4, 20); vline(21, 4, 20);
  vline(12, 4, 20);
  hline(3, 21, 12);
}

static void ic_motion() {    // running figure
  disc(15, 4, 2);            // head
  diag(14, 7, 11, 13);       // torso
  diag(13, 9, 18, 11);       // front arm
  diag(13, 9, 9, 12);        // back arm
  diag(11, 13, 16, 18);      // front leg
  diag(11, 13, 6, 18);       // back leg
}

static void ic_plug() {
  vline(10, 3, 8); vline(14, 3, 8);   // prongs
  rbox(7, 9, 10, 9, 3);               // body
  vline(12, 18, 22);                  // cord
}

static void ic_battery() {   // outline + nub + ~2/3 charge
  hline(2, 20, 8); hline(2, 20, 16);
  vline(2, 8, 16); vline(20, 8, 16);
  box(21, 10, 2, 5);         // nub
  box(4, 10, 12, 5);         // charge level
}

static void ic_alert() {     // triangle outline with a bang
  diag(12, 3, 3, 20);
  diag(12, 3, 21, 20);
  hline(3, 21, 20);
  vline(12, 9, 14);
  disc(12, 17, 1);
}

static void ic_check() {
  diag(4, 13, 10, 19);
  diag(10, 19, 20, 6);
}

static void ic_x() {
  diag(5, 5, 19, 19);
  diag(19, 5, 5, 19);
}

static void ic_arrow_up() {
  vline(12, 4, 20);
  diag(12, 3, 5, 10);
  diag(12, 3, 19, 10);
}

static void ic_arrow_down() {
  vline(12, 4, 20);
  diag(5, 14, 12, 21);
  diag(19, 14, 12, 21);
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------
struct HaIconEntry { const char* name; void (*draw)(); };

static const HaIconEntry kIcons[] = {
  { "alert",         ic_alert },
  { "arrow-down",    ic_arrow_down },
  { "arrow-up",      ic_arrow_up },
  { "battery",       ic_battery },
  { "check",         ic_check },
  { "cloud",         ic_cloud },
  { "door-closed",   ic_door_closed },
  { "door-open",     ic_door_open },
  { "home",          ic_home },
  { "humidity",      ic_humidity },
  { "motion",        ic_motion },
  { "plug",          ic_plug },
  { "rain",          ic_rain },
  { "snow",          ic_snow },
  { "sun",           ic_sun },
  { "thermometer",   ic_thermometer },
  { "wind",          ic_wind },
  { "window-closed", ic_window_closed },
  { "window-open",   ic_window_open },
  { "x",             ic_x },
};

bool haDrawIcon(const char* name, int x, int y, int s, uint16_t color) {
  if (!name || !name[0]) return false;
  for (uint8_t i = 0; i < sizeof(kIcons) / sizeof(kIcons[0]); i++) {
    if (strcmp(kIcons[i].name, name)) continue;
    Arduino_GFX* gfx = gfxDev();
    if (!gfx) return false;
    G = gfx; S = constrain(s, 1, 8); C = color; OX = x; OY = y;
    kIcons[i].draw();
    return true;
  }
  return false;   // unknown name: the renderer skips it silently
}

#endif  // WITH_HA
