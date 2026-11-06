#pragma once
#include <stddef.h>
#include <string.h>

// Shortens departure destinations for the 240 px display. Works on UTF-8
// byte strings, so it is host-testable and independent of Arduino String.
//
// 1. dropTown: "Dresden Bühlau" -> "Bühlau" and "Dresden, Bühlau" -> "Bühlau"
//    when `town` is the town of the configured stop. A destination that is
//    only the town ("Dresden") is kept.
// 2. abbreviate: common German terms, as whole words or word endings:
//    Hauptbahnhof -> Hbf, Bahnhof -> Bf, ...straße/...strasse -> ...str.,
//    Platz -> Pl., Sankt -> St.

static inline bool destWordEnd(char c) { return c == 0 || c == ' ' || c == ',' || c == '-' || c == '/' || c == ')'; }
static inline bool destWordStart(const char* s, const char* p) { return p == s || p[-1] == ' ' || p[-1] == '-' || p[-1] == '(' || p[-1] == '/'; }

// Replace every match of `from` (whole word, or word ending if `suffix`).
static inline void destReplace(char* s, size_t cap, const char* from, const char* to, bool suffix) {
  const size_t fl = strlen(from), tl = strlen(to);
  char* p = s;
  while ((p = strstr(p, from)) != nullptr) {
    const bool startOk = suffix || destWordStart(s, p);
    if (!startOk || !destWordEnd(p[fl])) { p += fl; continue; }
    const size_t rest = strlen(p + fl);
    if ((size_t)(p - s) + tl + rest + 1 > cap) return;
    memmove(p + tl, p + fl, rest + 1);
    memcpy(p, to, tl);
    p += tl;
  }
}

static inline void formatDestination(const char* in, const char* town, bool dropTown, bool abbreviate,
                                     char* out, size_t cap) {
  if (!cap) return;
  if (!in) in = "";
  const char* src = in;
  if (dropTown && town && *town) {
    const size_t tl = strlen(town);
    if (!strncmp(src, town, tl) && (src[tl] == ' ' || src[tl] == ',')) {
      const char* rest = src + tl;
      while (*rest == ' ' || *rest == ',') rest++;
      if (*rest) src = rest;
    }
  }
  strncpy(out, src, cap - 1);
  out[cap - 1] = 0;
  if (!abbreviate) return;
  destReplace(out, cap, "Hauptbahnhof", "Hbf", false);
  destReplace(out, cap, "Bahnhof", "Bf", false);
  destReplace(out, cap, "Stra\xC3\x9F" "e", "Str.", false);
  destReplace(out, cap, "Strasse", "Str.", false);
  destReplace(out, cap, "stra\xC3\x9F" "e", "str.", true);
  destReplace(out, cap, "strasse", "str.", true);
  destReplace(out, cap, "Platz", "Pl.", false);
  destReplace(out, cap, "Sankt", "St.", false);
}

// Bytes to keep so that a cropped text never ends in a space or separator
// before the abbreviation dot ("Leipzig ." -> "Leipzig."). `end` is the byte
// length of the first maxChars-1 glyphs.
static inline size_t destCropEnd(const char* s, size_t end) {
  while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == ',' || s[end - 1] == '-' || s[end - 1] == '.' || s[end - 1] == '/')) end--;
  return end;
}
