#!/usr/bin/env python3
"""Render the real display views from src/main.cpp into PNG screenshots.

The drawing code (helpers and all views) is taken verbatim from main.cpp and
compiled for the host against a 240x240 RGB565 framebuffer. Text uses the
same two fonts as the device: Arduino_GFX's classic 5x7 font for ASCII and
PixelFont.h for everything else. Data comes from the sample fixtures below.

Usage:
  python3 tools/render_screens.py [OUTPUT_DIR] [--font PATH/TO/glcdfont.h] [--scale N]

The font is part of the GFX Library for Arduino; after `pio pkg install` it is
found in .pio/libdeps automatically.
"""
from pathlib import Path
import argparse, re, subprocess, sys, tempfile

root = Path(__file__).resolve().parents[1]
src = (root / 'src/main.cpp').read_text()

ap = argparse.ArgumentParser()
ap.add_argument('out', nargs='?', default=str(root / 'docs/screens'))
ap.add_argument('--font', default=None)
ap.add_argument('--scale', type=int, default=2)
ap.add_argument('--frame', action='store_true', help='add a dark bezel around each screen')
ap.add_argument('--sheet', action='store_true', help='also write uebersicht.png with all views')
args = ap.parse_args()

font = Path(args.font) if args.font else next(
    (p for p in (root / '.pio/libdeps').glob('*/GFX Library for Arduino/src/font/glcdfont.h')), None)
if not font or not font.is_file():
    sys.exit('glcdfont.h not found; run "pio pkg install -e smalltv_ultra" or pass --font')
glyphs = re.search(r'font\[\]\s*PROGMEM\s*=\s*\{(.*?)\};', font.read_text(), re.S).group(1)

def between(start, end):
    a = src.index(start); b = src.index(end, a)
    return src[a:b]

# Declarations (pins, colours, modes, settings) and the complete drawing block.
decl = between('#define TFT_DC', '// ---- ST7789 quirk')
decl = re.sub(r'static ESP8266WebServer[^\n]*\n|static ESP8266HTTPUpdateServer[^\n]*\n|static DNSServer[^\n]*\n', '', decl)
views_raw = between('// ---- helpers', '// ---- network')

def top_level_items(code):
    """Split code into top-level chunks (definitions incl. their bodies)."""
    items, depth, start, i, n = [], 0, 0, 0, len(code)
    while i < n:
        c = code[i]
        if code.startswith('//', i):
            j = code.find('\n', i); i = n if j < 0 else j; continue
        if c == '"' or c == "'":
            q = c; i += 1
            while i < n and code[i] != q: i += 2 if code[i] == '\\' else 1
        elif c == '{': depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                k = i + 1
                if code[k:k+1] == ';': k += 1
                items.append(code[start:k]); start = k
        elif c == ';' and depth == 0:
            items.append(code[start:i+1]); start = i + 1
        i += 1
    items.append(code[start:])
    return items

# Keep everything that only draws; drop config storage, JSON and network code.
SKIP = re.compile(r'JsonDocument|JsonObject|JsonArray|LittleFS|\bFile\b|server\.|WiFi|ESP\.|deserializeJson|serializeJson')
views = ''.join(x for x in top_level_items(views_raw) if not SKIP.search(x))

ARDUINO_H = r'''
#pragma once
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <ctime>
#include <algorithm>
#define PROGMEM
#define F(x) x
#define FPSTR(x) (x)
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define pgm_read_dword(p) (*(const uint32_t*)(p))
#define PI 3.14159265358979323846
using std::min; using std::max;
template<class T,class A,class B> T constrain(T x,A a,B b){return x<(T)a?(T)a:(x>(T)b?(T)b:x);}
uint32_t millis();
inline void analogWrite(int,int){}
inline long lround_(double v){return lround(v);}
inline size_t strlcpy(char*d,const char*s,size_t n){size_t l=strlen(s);if(n){size_t c=l<n-1?l:n-1;memcpy(d,s,c);d[c]=0;}return l;}
struct String : std::string {
  String() {}
  String(const char* s) : std::string(s ? s : "") {}
  String(const std::string& s) : std::string(s) {}
  String(char c) : std::string(1, c) {}
  String(int v) : std::string(std::to_string(v)) {}
  String(unsigned v) : std::string(std::to_string(v)) {}
  String(long v) : std::string(std::to_string(v)) {}
  String(unsigned long v) : std::string(std::to_string(v)) {}
  String(long long v) : std::string(std::to_string(v)) {}
  String(unsigned long long v) : std::string(std::to_string(v)) {}
  String(double v, int digits = 2) { char b[48]; snprintf(b, sizeof b, "%.*f", digits, v); assign(b); }
  unsigned length() const { return (unsigned)size(); }
  String substring(unsigned a) const { return a < size() ? String(substr(a)) : String(); }
  String substring(unsigned a, unsigned b) const { if (a > b) std::swap(a, b); return a < size() ? String(substr(a, b - a)) : String(); }
  int indexOf(char c, unsigned from = 0) const { auto p = find(c, from); return p == npos ? -1 : (int)p; }
  int indexOf(const String& q, unsigned from = 0) const { auto p = find(q, from); return p == npos ? -1 : (int)p; }
  int lastIndexOf(char c) const { auto p = rfind(c); return p == npos ? -1 : (int)p; }
  bool startsWith(const String& p) const { return compare(0, p.size(), p) == 0; }
  bool endsWith(const String& p) const { return size() >= p.size() && compare(size() - p.size(), p.size(), p) == 0; }
  void toUpperCase() { for (auto& c : *this) if (c >= 'a' && c <= 'z') c -= 32; }
  void toLowerCase() { for (auto& c : *this) if (c >= 'A' && c <= 'Z') c += 32; }
  void trim() { size_t a = find_first_not_of(" \t\r\n"); if (a == npos) { clear(); return; } size_t b = find_last_not_of(" \t\r\n"); *this = substr(a, b - a + 1); }
  void replace(const String& a, const String& b) { if (a.empty()) return; size_t p = 0; while ((p = find(a, p)) != npos) { std::string::replace(p, a.size(), b); p += b.size(); } }
  long toInt() const { return atol(c_str()); }
  float toFloat() const { return (float)atof(c_str()); }
  char charAt(unsigned i) const { return i < size() ? (*this)[i] : 0; }
  bool equalsIgnoreCase(const String& o) const { if (o.size() != size()) return false; for (size_t i = 0; i < size(); i++) if (tolower((*this)[i]) != tolower(o[i])) return false; return true; }
  bool concat(const String& s) { append(s); return true; }
};
inline String operator+(const String& a, const String& b) { return String(static_cast<const std::string&>(a) + b); }
inline String operator+(const String& a, const char* b) { return String(static_cast<const std::string&>(a) + b); }
inline String operator+(const char* a, const String& b) { return String(a + static_cast<const std::string&>(b)); }
inline String operator+(const String& a, char b) { return String(static_cast<const std::string&>(a) + b); }
inline String operator+(const String& a, int b) { return a + String(b); }
inline String operator+(const String& a, unsigned b) { return a + String(b); }
inline String operator+(const String& a, long b) { return a + String(b); }
inline String operator+(const String& a, unsigned long b) { return a + String(b); }
'''

GFX = r'''
static const unsigned char glcd[] = {''' + glyphs + r'''};
struct Framebuffer {
  uint16_t px[240 * 240] = {};
  int cx = 0, cy = 0, size = 1; uint16_t color = 0xFFFF; uint8_t rot = 0;
  void setRotation(uint8_t r) { rot = r; }
  void setTextWrap(bool) {}
  void fillScreen(uint16_t c) { for (auto& p : px) p = c; }
  void drawPixel(int x, int y, uint16_t c) { if (x >= 0 && y >= 0 && x < 240 && y < 240) px[y * 240 + x] = c; }
  void fillRect(int x, int y, int w, int h, uint16_t c) { for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) drawPixel(x + i, y + j, c); }
  void drawFastHLine(int x, int y, int w, uint16_t c) { fillRect(x, y, w, 1, c); }
  void drawFastVLine(int x, int y, int h, uint16_t c) { fillRect(x, y, 1, h, c); }
  void drawRect(int x, int y, int w, int h, uint16_t c) { drawFastHLine(x, y, w, c); drawFastHLine(x, y + h - 1, w, c); drawFastVLine(x, y, h, c); drawFastVLine(x + w - 1, y, h, c); }
  void drawLine(int x0, int y0, int x1, int y1, uint16_t c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, e = dx + dy;
    for (;;) { drawPixel(x0, y0, c); if (x0 == x1 && y0 == y1) break; int e2 = 2 * e; if (e2 >= dy) { e += dy; x0 += sx; } if (e2 <= dx) { e += dx; y0 += sy; } }
  }
  void fillCircle(int x0, int y0, int r, uint16_t c) { for (int y = -r; y <= r; y++) for (int x = -r; x <= r; x++) if (x * x + y * y <= r * r + r) drawPixel(x0 + x, y0 + y, c); }
  void drawCircle(int x0, int y0, int r, uint16_t c) {
    int f = 1 - r, ddx = 1, ddy = -2 * r, x = 0, y = r;
    drawPixel(x0, y0 + r, c); drawPixel(x0, y0 - r, c); drawPixel(x0 + r, y0, c); drawPixel(x0 - r, y0, c);
    while (x < y) { if (f >= 0) { y--; ddy += 2; f += ddy; } x++; ddx += 2; f += ddx;
      int pts[8][2] = {{x,y},{-x,y},{x,-y},{-x,-y},{y,x},{-y,x},{y,-x},{-y,-x}}; for (auto& p : pts) drawPixel(x0 + p[0], y0 + p[1], c); }
  }
  void fillRoundRect(int x, int y, int w, int h, int r, uint16_t c) { fillRect(x + r, y, w - 2 * r, h, c); fillRect(x, y + r, w, h - 2 * r, c); fillCircle(x + r, y + r, r, c); fillCircle(x + w - r - 1, y + r, r, c); fillCircle(x + r, y + h - r - 1, r, c); fillCircle(x + w - r - 1, y + h - r - 1, r, c); }
  void drawRoundRect(int x, int y, int w, int h, int, uint16_t c) { drawRect(x, y, w, h, c); }
  void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) {
    int minx = std::min({x0, x1, x2}), maxx = std::max({x0, x1, x2}), miny = std::min({y0, y1, y2}), maxy = std::max({y0, y1, y2});
    auto edge = [](int ax, int ay, int bx, int by, int px, int py) { return (bx - ax) * (py - ay) - (by - ay) * (px - ax); };
    for (int y = miny; y <= maxy; y++) for (int x = minx; x <= maxx; x++) {
      int a = edge(x0, y0, x1, y1, x, y), b = edge(x1, y1, x2, y2, x, y), d = edge(x2, y2, x0, y0, x, y);
      if ((a >= 0 && b >= 0 && d >= 0) || (a <= 0 && b <= 0 && d <= 0)) drawPixel(x, y, c); }
  }
  void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) { drawLine(x0, y0, x1, y1, c); drawLine(x1, y1, x2, y2, c); drawLine(x2, y2, x0, y0, c); }
  void setTextSize(int s) { size = s; }
  void setTextColor(uint16_t c) { color = c; }
  void setTextColor(uint16_t c, uint16_t) { color = c; }
  void setCursor(int x, int y) { cx = x; cy = y; }
  size_t write(uint8_t ch) {
    // Arduino_GFX classic font: 5 columns, LSB at the top, 6x8 cell.
    for (int i = 0; i < 5; i++) { uint8_t line = glcd[ch * 5 + i];
      for (int j = 0; j < 8; j++, line >>= 1) if (line & 1) fillRect(cx + i * size, cy + j * size, size, size, color); }
    cx += 6 * size; return 1;
  }
  void print(const String& s) { for (unsigned char c : s) write(c); }
  void print(const char* s) { print(String(s)); }
  void print(int v) { print(String(v)); }
};
static Framebuffer framebuffer;
static Framebuffer* gfx = &framebuffer;
'''

FIXTURES = r'''
static uint32_t clockMs = 1000000;
uint32_t millis() { return clockMs; }
static time_t fakeNow = 1793956212; // Fri 2026-11-06 10:10 CET
static time_t fakeTime(time_t* t) { if (t) *t = fakeNow; return fakeNow; }
#define time(x) fakeTime(x)

// ---- data accessors with sample values ----
static VvoDeparture deps[10]; static uint8_t depN = 0;
uint8_t vvoCount() { return depN; }
const VvoDeparture& vvoAt(uint8_t i) { return deps[i]; }
bool vvoError() { return false; }
bool vvoNotServed() { return false; }
static WeatherSnapshot wx = {};
const WeatherSnapshot& weatherData() { return wx; }
const char* weatherLabel(int c) { return weatherVisual(c).label; }
static FlightSnapshot fl = {};
const FlightSnapshot& flightData() { return fl; }
uint32_t flightLastOkMs() { return clockMs - 4000; }
bool flightProviderPaused() { return false; }
int flightLastHttp() { return 200; }
uint32_t flightWaitSec() { return 0; }
static TrackedFlightSnapshot tf = {};
const TrackedFlightSnapshot& trackedFlightData() { return tf; }
'''

def stub_scan():
    # Functions used by the views but implemented elsewhere: keep fixture list explicit.
    return ''

MAIN = r'''
#include "Arduino.h"
#include "Vvo.h"
#include "Weather.h"
#include "Flight.h"
#include "TrackFlight.h"
#include "RequestPolicy.h"
#include "PixelFont.h"
#include "TimeZones.h"
#include "WeatherVisual.h"
#include "AircraftLabel.h"
static inline bool adsbLolAccessDenied(int c){return c==401||c==403||c==426;}
''' + GFX + '\n' + decl + '\n' + FIXTURES + '\n' + views + r'''
static void dump(const char* path) {
  FILE* f = fopen(path, "wb"); fprintf(f, "P6\n240 240\n255\n");
  for (uint16_t p : framebuffer.px) { uint8_t rgb[3] = {(uint8_t)(((p >> 11) & 31) * 255 / 31), (uint8_t)(((p >> 5) & 63) * 255 / 63), (uint8_t)((p & 31) * 255 / 31)}; fwrite(rgb, 1, 3, f); }
  fclose(f);
}
static void dep(const char* line, const char* dir, int min, bool delayed = false, bool cancelled = false) {
  VvoDeparture& d = deps[depN++]; strlcpy(d.line, line, sizeof d.line); strlcpy(d.direction, dir, sizeof d.direction);
  d.minutes = min; d.delayed = delayed; d.cancelled = cancelled;
}
int main(int argc, char** argv) {
  std::string out = argc > 1 ? argv[1] : ".";
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); tzset();
  cfg.timeZone = "Europe/Berlin";
  cfg.rotationMask = 0x1FF;

  // Abfahrten
  cfg.mode = MODE_DEPARTURES; cfg.stopName = "Postplatz"; cfg.stopId = "33000037"; cfg.departureCount = 5;
  dep("1", "Prohlis", 2); dep("2", "Kleinzschachwitz", 4, true); dep("4", "Radebeul West", 7);
  dep("12", "Striesen", 11); dep("62", "Johannstadt", 14, false, true);
  drawDepartures(); dump((out + "/abfahrten.ppm").c_str());

  // Wetter
  cfg.mode = MODE_WEATHER; cfg.weatherCity = "Dresden"; cfg.weatherDisplayName = "";
  wx.valid = true; wx.currentC = 11.6f; wx.highC = 12.1f; wx.lowC = 4.3f; wx.rainPct = 30; wx.code = 2; wx.isDay = true;
  strlcpy(wx.sunrise, "07:12", sizeof wx.sunrise); strlcpy(wx.sunset, "16:28", sizeof wx.sunset);
  drawWeather(); dump((out + "/wetter.ppm").c_str());

  // Flugradar
  cfg.mode = MODE_PLANE; cfg.planePlace = "Dresden-Klotzsche"; cfg.planeShowAge = true; cfg.planeShowPlace = false;
  fl.valid = true; strlcpy(fl.hex, "3c4b2a", 8); strlcpy(fl.callsign, "DLH6CM", 12); strlcpy(fl.airline, "Lufthansa", 64);
  strlcpy(fl.registration, "D-AIZW", 16); strlcpy(fl.typeCode, "A320", 8); strlcpy(fl.typeName, "Airbus A320", 80);
  strlcpy(fl.origin, "FRA", 4); strlcpy(fl.destination, "DRS", 4); parseFlightRoute("FRA-DRS", fl.route);
  fl.distanceKm = 7.8f; fl.altitudeFt = 6200; fl.altitudeKnown = true;
  drawPlane(); dump((out + "/flugradar.ppm").c_str());

  // Uhr
  cfg.mode = MODE_CLOCK; drawClock(); dump((out + "/uhr.ppm").c_str());

  // Flug-Tracker
  cfg.mode = MODE_TRACKED_FLIGHT; cfg.trackedFlightNo = "LH1178"; cfg.aviationstackKey = "sample";
  tf.valid = true; strlcpy(tf.flightIata, "LH1178", 12); strlcpy(tf.flightIcao, "DLH1178", 12); strlcpy(tf.airline, "Lufthansa", 28);
  strlcpy(tf.origin, "DRS", 4); strlcpy(tf.destination, "FRA", 4); parseFlightRoute("DRS-FRA", tf.route);
  strlcpy(tf.status, "active", 16); strlcpy(tf.depScheduled, "09:40", 6); strlcpy(tf.depActual, "09:52", 6);
  strlcpy(tf.arrScheduled, "10:50", 6); strlcpy(tf.arrEstimated, "10:58", 6); strlcpy(tf.registration, "D-AIBH", 16);
  strlcpy(tf.typeCode, "A319", 8); tf.altitudeM = 10970; tf.speedKmh = 812;
  tf.depScheduledEpoch = fakeNow - 1800; tf.depActualEpoch = fakeNow - 1080; tf.arrScheduledEpoch = fakeNow + 2400; tf.arrEstimatedEpoch = fakeNow + 2880;
  drawTrackedFlight(); dump((out + "/flugtracker.ppm").c_str());

  // Countdowns
  CountdownSlot& c1 = cfg.countdown[0]; c1.title = "Urlaub"; c1.type = COUNTDOWN_DAYS; c1.targetDate = "2026-12-23"; c1.active = true; c1.dayDigits = 3;
  cfg.mode = MODE_COUNTDOWN1; drawCountdown(0); dump((out + "/countdown-tage.ppm").c_str());
  CountdownSlot& c2 = cfg.countdown[1]; c2.title = "Pizza"; c2.type = COUNTDOWN_MS; c2.titleColor = "#F3CE00"; c2.countColor = "#5DD78B";
  c2.durationSec = 720; c2.targetEpoch = (uint32_t)fakeNow + 534; c2.active = true;
  cfg.mode = MODE_COUNTDOWN2; drawCountdown(1); dump((out + "/countdown-timer.ppm").c_str());
  return 0;
}
'''

out = Path(args.out); out.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory() as tmp:
    t = Path(tmp)
    (t / 'Arduino.h').write_text(ARDUINO_H)
    for name in ['ESP8266HTTPClient.h']:
        (t / name).write_text('#pragma once\nstruct HTTPClient{};\n')
    (t / 'render.cpp').write_text(MAIN)
    exe = t / 'render'
    r = subprocess.run(['g++', '-std=c++17', '-w', '-fpermissive', '-I', str(t), '-I', str(root / 'src'),
                        str(t / 'render.cpp'),
                        '-o', str(exe)], capture_output=True, text=True)
    if r.returncode:
        sys.stderr.write(r.stderr[-6000:]); sys.exit(1)
    subprocess.run([str(exe), str(t)], check=True)
    from PIL import Image
    for ppm in sorted(t.glob('*.ppm')):
        img = Image.open(ppm).convert('RGB')
        if args.scale > 1:
            img = img.resize((240 * args.scale, 240 * args.scale), Image.NEAREST)
        if args.frame:
            pad = 12 * args.scale
            framed = Image.new('RGB', (img.width + 2 * pad, img.height + 2 * pad), (24, 26, 29))
            framed.paste(img, (pad, pad)); img = framed
        img.save(out / (ppm.stem + '.png'))
        print(out / (ppm.stem + '.png'))
    if args.sheet:
        order = ['abfahrten', 'wetter', 'flugradar', 'uhr', 'flugtracker', 'countdown-tage', 'countdown-timer']
        tiles = [Image.open(out / (n + '.png')) for n in order]
        w, h, gap = tiles[0].width, tiles[0].height, 8 * args.scale
        cols = 4
        sheet = Image.new('RGB', (cols * w + (cols + 1) * gap, 2 * h + 3 * gap), (24, 26, 29))
        for i, tile in enumerate(tiles):
            sheet.paste(tile, (gap + (i % cols) * (w + gap), gap + (i // cols) * (h + gap)))
        sheet = sheet.crop((0, 0, sheet.width, sheet.height))
        sheet.save(out / 'uebersicht.png'); print(out / 'uebersicht.png')
