
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include "Vvo.h"
#include "Weather.h"
#include "AdsbLol.h"
#include "Flight.h"
#include "TrackFlight.h"
#include "Geo.h"
#include "RequestPolicy.h"
#include "Diagnostics.h"
#include "PixelFont.h"
#include "TimeZones.h"
#include "WeatherVisual.h"
#include "AircraftLabel.h"
#include "DestinationFormat.h"
#include "webui_en.h"

// -----------------------------------------------------------------------------
// SmallTV-Ultra hardware
// ESP8266 HW SPI: SCLK GPIO14, MOSI GPIO13
// Pin map based on the open-source smalltv-mod hardware definition.
// -----------------------------------------------------------------------------
#define TFT_DC   0
#define TFT_RST  2
#define TFT_CS   15
#define TFT_BL   5
#define TFT_W    240
#define TFT_H    240

#include "Version.h"
// Backlight PWM (since v0.4.14.6, against flashes at 1 % brightness)
#define BL_PWM_FREQ 300
#define BL_PWM_RANGE 1023

#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_YELLOW  0xFFE0
#define C_RED     0xF800
#define C_GREEN   0x07E0
#define C_GRAY    0x8410
#define C_DGRAY   0x4208
#define C_BLUE    0x041F
#define C_CYAN    0x07FF

enum AppMode : uint8_t {
  MODE_DEPARTURES = 0,
  MODE_WEATHER    = 1,
  MODE_PLANE      = 2,
  MODE_CLOCK      = 3,
  MODE_TRACKED_FLIGHT = 4,
  MODE_COUNTDOWN1 = 5,
  MODE_COUNTDOWN2 = 6,
  MODE_COUNTDOWN3 = 7,
  MODE_COUNTDOWN4 = 8
};

enum Language : uint8_t { LANG_DE = 0, LANG_EN = 1 };

enum CountdownType : uint8_t {
  COUNTDOWN_DAYS = 0,
  COUNTDOWN_HMS  = 1,
  COUNTDOWN_MS   = 2
};

struct CountdownSlot {
  String title = "";
  String titleColor = "#FFFFFF";
  String countColor = "#FF5D56";
  uint8_t type = COUNTDOWN_DAYS;
  String targetDate = "";        // YYYY-MM-DD, intentionally date-only
  uint32_t durationSec = 600;     // used for HMS / MS start/restart
  uint32_t targetEpoch = 0;       // absolute target for running duration timers
  bool active = false;
  uint8_t dayDigits = 3;          // keep leading zeroes stable once activated
};

struct Settings {
  String ssid;
  String pass;
  String hostname = "pixelstop";
  uint8_t lang = LANG_DE;          // display and web interface language

  uint8_t mode = MODE_DEPARTURES; // current runtime view
  uint16_t rotationMask = (1U << MODE_DEPARTURES);
  uint16_t rotationSec[9] = {30,20,20,15,20,20,20,20,20};
  uint8_t brightness = 70;
  String timeZone = "Europe/Berlin";

  bool nightEnabled = true;
  uint16_t nightStart = 22 * 60;
  uint16_t nightEnd = 7 * 60;
  uint8_t nightBrightness = 12;

  String stopName = "Pohlandplatz";
  String stopId = "";
  uint8_t departureCount = 4;
  uint16_t departurePoll = 30;
  uint8_t vvoMotMask = VVO_MOT_ALL;
  bool departureCompactDestinations = false;
  bool departureAbbreviate = true;   // Hbf, Str., Pl. ...
  bool departureDropTown = true;     // "Dresden Bühlau" -> "Bühlau"

  String weatherCity = "Dresden"; // selected real place / fallback title
  String weatherDisplayName = ""; // optional display-only title
  float weatherLat = 51.0504f;
  float weatherLon = 13.7373f;
  uint16_t weatherPoll = 600;
  bool weatherTempColor = false;
  bool weatherFahrenheit = false;
  bool geoGermanNames = false;

  String planePlace = "Dresden";
  String planeDisplayName = "";
  float planeLat = 51.0504f;
  float planeLon = 13.7373f;
  uint16_t planeRadiusKm = 25;
  uint16_t planePoll = 10;
  bool planeShowPlace = false;
  bool planeShowAge = true;
  uint16_t planePlaceToggleSec = 10;

  String trackedFlightNo = "";
  String aviationstackKey = "";
  String adsbLolKey = "";
  uint16_t trackerStatusPollMin = 15;

  CountdownSlot countdown[4];
};

static Settings cfg;
static ESP8266WebServer server(80);
static ESP8266HTTPUpdateServer updater;
static DNSServer dns;
static bool apMode = false;
static uint32_t lastSecond = 0;
static uint8_t lastMinute = 255;
static uint32_t lastModeSwitchMs = 0;
static String lastCountdownRendered[4];
static uint8_t departurePage = 0;
static bool departurePagingReady = false;
static uint32_t lastDeparturePageSwitchMs = 0;
static bool planePlacePhase = false;
static uint32_t lastPlanePlaceSwitchMs = 0;

// ---- language ---------------------------------------------------------------
// Display texts live in flash (PSTR), both languages; only the selected one
// is copied into a String when drawn.
static String tr(PGM_P de, PGM_P en) { return String(FPSTR(cfg.lang==LANG_EN ? en : de)); }
#define TR(de,en) tr(PSTR(de),PSTR(en))

// ---- ST7789 quirk -----------------------------------------------------------
class SmallTV_ST7789 : public Arduino_ST7789 {
 public:
  using Arduino_ST7789::Arduino_ST7789;
  bool begin(int32_t speed = GFX_NOT_DEFINED) override {
    _override_datamode = SPI_MODE3;
    return Arduino_TFT::begin(speed);
  }

  void setRotation(uint8_t r) override {
    Arduino_TFT::setRotation(r);
    uint8_t madctl;
    switch (_rotation) {
      case 1:  madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MV; break;
      case 2:  madctl = ST7789_MADCTL_MX | ST7789_MADCTL_MY; break;
      case 3:  madctl = ST7789_MADCTL_MY | ST7789_MADCTL_MV; break;
      default: madctl = 0; break;
    }
    _bus->beginWrite();
    _bus->writeC8D8(ST7789_MADCTL, madctl); // RGB order
    _bus->endWrite();
  }
};

static Arduino_DataBus* bus = nullptr;
static Arduino_GFX* gfx = nullptr;

static const char WEBUI[] PROGMEM = R"HTML(
<!doctype html><html lang="de"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>PixelStop</title>
<style>
:root{--bg:#17191c;--card:#24272b;--card2:#2e3237;--text:#f4f4f4;--muted:#aeb4bb;--accent:#f3ce00;--red:#ff5d56;--green:#5dd78b;--border:#3b4047}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}.wrap{max-width:760px;margin:auto;padding:18px}.head{display:flex;justify-content:space-between;align-items:center;margin-bottom:16px}h1{font-size:24px;margin:0}.chip{font-size:12px;color:#111;background:var(--accent);padding:6px 9px;border-radius:99px;font-weight:700}
nav{display:grid;grid-template-columns:repeat(4,1fr);gap:6px;margin-bottom:14px}.tab{border:0;background:var(--card);color:var(--muted);padding:10px 5px;border-radius:10px;font-size:12px}.tab.on{background:var(--accent);color:#111;font-weight:700}.page{display:none}.page.on{display:block}.card{background:var(--card);border:1px solid var(--border);border-radius:14px;padding:16px;margin:10px 0}h2{font-size:18px;margin:0 0 14px}.hint{font-size:12px;color:var(--muted);line-height:1.45}.mini{font-size:11px;color:var(--muted);margin-top:8px}label{display:block;font-size:13px;color:var(--muted);margin:11px 0 5px}input,select{width:100%;background:#16181b;color:var(--text);border:1px solid #4a5058;border-radius:9px;padding:11px;font-size:16px}.row{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}.row>div,.colorrow>div,.triple>div{min-width:0}input,select{min-width:0;max-width:100%}input[type=time],input[type=date]{display:block;appearance:none;-webkit-appearance:none;width:100%;min-width:0;max-width:100%;min-height:44px;height:44px;padding:9px 11px}input::-webkit-date-and-time-value{min-width:0;text-align:left}input::-webkit-datetime-edit{min-width:0;padding:0}.triple{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}.colorrow{display:grid;grid-template-columns:1fr 76px;gap:10px;align-items:end}
button.action,a.action{display:inline-block;border:0;border-radius:10px;background:var(--accent);color:#111;padding:11px 15px;font-family:inherit;font-size:14px;line-height:1.3;font-weight:750;text-decoration:none;min-height:42px}button.sec,a.sec{background:var(--card2);color:var(--text)}button.danger{background:var(--red);color:#111}.actions{display:flex;gap:8px;flex-wrap:wrap;margin-top:12px}.serviceList{padding-left:20px}.serviceList li{margin:12px 0;overflow-wrap:anywhere}summary{cursor:pointer;font-weight:750}details h3{margin-top:16px}.sticky{position:sticky;bottom:10px;padding-top:12px}.sticky button{width:100%;font-size:16px}#saveBtn:disabled{background:#45494f;color:#aeb3ba;cursor:wait}.status{font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:12px;white-space:pre-wrap;color:#cdd2d8}.pickres{display:block;width:100%;text-align:left;margin:6px 0;padding:10px;border:1px solid var(--border);border-radius:9px;background:var(--card2);color:var(--text)}input[type=range]{padding:4px}input[type=color]{padding:3px;height:44px}.val{color:var(--accent);font-weight:700}.hidden{display:none}.checks{display:grid;grid-template-columns:repeat(2,1fr);gap:8px;margin-top:8px}.check{display:flex;align-items:center;gap:8px;background:#1d2024;border:1px solid var(--border);border-radius:9px;padding:9px;color:var(--text);margin:0}.check input{width:auto;margin:0}.modeRow{display:grid;grid-template-columns:1fr 110px;gap:10px;align-items:center;border-bottom:1px solid var(--border);padding:8px 0}.modeRow:last-child{border-bottom:0}.modeLeft{display:flex;align-items:center;gap:9px}.modeLeft input{width:auto}.wifirow{display:grid;grid-template-columns:1fr auto;gap:10px;align-items:center}.sig{font-family:ui-monospace,SFMono-Regular,Menlo,monospace;color:var(--muted);float:right}.ok{color:var(--green)}.err{color:var(--red)}.mapwrap{margin-top:10px;border:1px solid var(--border);border-radius:12px;overflow:hidden;background:#16181b}.map{height:300px;width:100%}.mapnote{padding:8px 10px;font-size:11px;color:var(--muted);border-top:1px solid var(--border)}.footer{text-align:center;color:var(--muted);font-size:11px;line-height:1.5;padding:18px 6px 6px}.footer a{color:var(--muted)}.toast{position:fixed;z-index:9999;top:18px;left:50%;transform:translate(-50%,-18px);opacity:0;pointer-events:none;min-width:220px;max-width:min(90vw,520px);padding:14px 18px;border-radius:12px;font-weight:800;text-align:center;box-shadow:0 8px 28px #0008;transition:opacity .18s ease,transform .18s ease}.toast.show{opacity:1;transform:translate(-50%,0)}.toast.ok{background:var(--green);color:#0b1a10}.toast.err{background:var(--red);color:#200}.toast.warn{background:var(--accent);color:#111}.subhead{font-size:13px;font-weight:400;color:var(--muted);margin:18px 0 5px}.depblock{transition:opacity .15s ease}.depblock.disabled{opacity:.42}.depblock.disabled label{color:var(--muted)}select:disabled,input:disabled{cursor:not-allowed;opacity:.58}
@media(max-width:560px){nav{grid-template-columns:repeat(3,1fr)}.row,.colorrow,.modeRow,.wifirow{grid-template-columns:1fr}.triple{grid-template-columns:repeat(3,1fr)}.checks{grid-template-columns:1fr}}
</style></head><body><div class="wrap">
<div class="head"><h1>PixelStop</h1><span class="chip">v)HTML" FW_VERSION R"HTML(</span></div>
<nav><button class="tab on" data-p="anzeige">Anzeige</button><button class="tab" data-p="abfahrten">Abfahrten</button><button class="tab" data-p="wetter">Wetter</button><button class="tab" data-p="flugradar">Flugradar</button><button class="tab" data-p="flugtracker">Flug-Tracker</button><button class="tab" data-p="countdown">Countdown</button><button class="tab" data-p="system">System</button></nav>

<section id="anzeige" class="page on">
<div class="card"><h2>Anzeigerotation</h2><p class="hint">Programme anhaken und je Ansicht die Anzeigedauer festlegen. Mit nur einem Haken bleibt die Ansicht dauerhaft stehen.</p><div id="modeRows"></div>
<label>Helligkeit: <span class="val" id="brv"></span>%</label><input id="brightness" type="range" min="1" max="100"></div>
<div class="card"><h2>Zeitzone</h2><label for="timeZone">Lokale Zeit</label><select id="timeZone"><option value="Europe/Berlin">Europe/Berlin</option><option value="Europe/London">Europe/London</option><option value="Europe/Paris">Europe/Paris</option><option value="Europe/Madrid">Europe/Madrid</option><option value="Europe/Rome">Europe/Rome</option><option value="Europe/Prague">Europe/Prague</option><option value="Europe/Warsaw">Europe/Warsaw</option><option value="Europe/Athens">Europe/Athens</option><option value="Europe/Helsinki">Europe/Helsinki</option><option value="Europe/Bucharest">Europe/Bucharest</option><option value="Europe/Kyiv">Europe/Kyiv</option><option value="Europe/Istanbul">Europe/Istanbul</option><option value="Europe/Moscow">Europe/Moscow</option><option value="Atlantic/Reykjavik">Atlantic/Reykjavik</option><option value="Atlantic/Canary">Atlantic/Canary</option><option value="Atlantic/Azores">Atlantic/Azores</option><option value="Africa/Cairo">Africa/Cairo</option><option value="Africa/Johannesburg">Africa/Johannesburg</option><option value="Africa/Nairobi">Africa/Nairobi</option><option value="America/New_York">America/New_York</option><option value="America/Chicago">America/Chicago</option><option value="America/Denver">America/Denver</option><option value="America/Phoenix">America/Phoenix</option><option value="America/Los_Angeles">America/Los_Angeles</option><option value="America/Anchorage">America/Anchorage</option><option value="Pacific/Honolulu">Pacific/Honolulu</option><option value="America/Toronto">America/Toronto</option><option value="America/Vancouver">America/Vancouver</option><option value="America/Mexico_City">America/Mexico_City</option><option value="America/Bogota">America/Bogota</option><option value="America/Sao_Paulo">America/Sao_Paulo</option><option value="America/Argentina/Buenos_Aires">America/Argentina/Buenos_Aires</option><option value="America/St_Johns">America/St_Johns</option><option value="Asia/Jerusalem">Asia/Jerusalem</option><option value="Asia/Dubai">Asia/Dubai</option><option value="Asia/Kolkata">Asia/Kolkata</option><option value="Asia/Kathmandu">Asia/Kathmandu</option><option value="Asia/Bangkok">Asia/Bangkok</option><option value="Asia/Singapore">Asia/Singapore</option><option value="Asia/Shanghai">Asia/Shanghai</option><option value="Asia/Tokyo">Asia/Tokyo</option><option value="Asia/Seoul">Asia/Seoul</option><option value="Australia/Perth">Australia/Perth</option><option value="Australia/Darwin">Australia/Darwin</option><option value="Australia/Adelaide">Australia/Adelaide</option><option value="Australia/Brisbane">Australia/Brisbane</option><option value="Australia/Sydney">Australia/Sydney</option><option value="Pacific/Auckland">Pacific/Auckland</option><option value="Pacific/Fiji">Pacific/Fiji</option><option value="UTC">UTC</option></select><p class="hint">Gilt für Uhr, Footer, Nachtmodus, Wetter-Zeitangaben und Tages-Countdowns. Sommer- und Winterzeit werden nach den Regeln der ausgewählten Zone umgestellt.</p></div>
<div class="card"><h2>Nachtmodus</h2><label class="check"><input id="nightEnabled" type="checkbox"> nachts dimmen</label><div class="row"><div><label for="nightStart">Beginn</label><input id="nightStart" type="time"></div><div><label for="nightEnd">Ende</label><input id="nightEnd" type="time"></div></div><label>Nacht-Helligkeit: <span class="val" id="nbv"></span>%</label><input id="nightBrightness" type="range" min="1" max="100"></div>
</section>

<section id="abfahrten" class="page"><div class="card"><h2>Abfahrtsmonitor</h2><p class="hint">Live vom Verkehrsverbund Oberelbe (VVO). Haltestelle suchen, Treffer wählen und Verkehrsmittel auswählen.</p><label>Haltestelle</label><input id="stopName" placeholder="z. B. Pohlandplatz"><div class="actions"><button class="action sec" type="button" id="searchStopBtn">Haltestelle suchen</button></div><div id="stopResults"></div><label>VVO-Haltestellen-ID</label><input id="stopId" readonly placeholder="noch keine Haltestelle gewählt"><label>Verkehrsmittel</label><div class="checks"><label class="check"><input type="checkbox" id="motTrain"> Zug</label><label class="check"><input type="checkbox" id="motSuburban"> S-Bahn</label><label class="check"><input type="checkbox" id="motTram"> Straßenbahn</label><label class="check"><input type="checkbox" id="motBus"> Bus</label><label class="check"><input type="checkbox" id="motCable"> Seil-/Schwebebahn</label><label class="check"><input type="checkbox" id="motFerry"> Fähre</label><label class="check"><input type="checkbox" id="motTaxi"> AST / Rufbus</label></div><div class="row"><div><label>Anzahl Abfahrten</label><select id="departureCount"><option>3</option><option>4</option><option>5</option><option>10</option></select></div><div><label>Aktualisierung</label><select id="departurePoll"><option value="15">15 s</option><option value="30">30 s</option><option value="60">60 s</option></select></div></div><div class="subhead">Darstellung Fahrziel</div><label class="check"><input id="departureCompactDestinations" type="checkbox"> Lange Ziele kompakt darstellen</label><label class="check"><input id="departureAbbreviate" type="checkbox" checked> Ziele abkürzen (Hbf, Bf, Str., Pl., St.)</label><label class="check"><input id="departureDropTown" type="checkbox" checked> Ortsnamen der Haltestelle weglassen</label><p class="mini">Beispiel: „Dresden Bühlau“ wird zu „Bühlau“, wenn die Haltestelle in Dresden liegt.</p></div></section>

<section id="wetter" class="page"><div class="card"><h2>Wetter</h2><p class="hint">Live von Open-Meteo. Ort suchen und den Beobachtungspunkt auf der Karte exakt setzen. Standort gilt nur für dieses Programm.</p><label>Ort suchen</label><input id="weatherSearch" placeholder="z. B. Dresden oder 01237"><div class="actions"><button class="action sec" type="button" id="weatherSearchBtn">Suchen</button><button class="action sec" type="button" id="weatherMapBtn">Auf Karte auswählen</button></div><div id="weatherResults"></div><div id="weatherMapWrap" class="mapwrap hidden"><div id="weatherMap" class="map"></div><div class="mapnote">Kartenmaterial © OpenStreetMap-Mitwirkende. Auf die gewünschte Position tippen, um den Standort exakt zu setzen.</div></div><label>Ausgewählter Standort</label><input id="weatherCity" readonly><label>Anzeigename <span class="mini">optional, leer = gefundener Ortsname</span></label><input id="weatherDisplayName" placeholder="z. B. ZUHAUSE"><div class="subhead">Ortsnamen</div><label class="check"><input id="weatherGeoGerman" type="checkbox"> Auf Deutsch anzeigen, wenn verfügbar</label><div class="row"><div><label>Breitengrad</label><input id="weatherLat" inputmode="decimal"></div><div><label>Längengrad</label><input id="weatherLon" inputmode="decimal"></div></div><div class="row"><div><label>Temperatureinheit</label><select id="weatherUnit"><option value="C">Celsius (°C)</option><option value="F">Fahrenheit (°F)</option></select></div><div><label>Aktualisierung</label><select id="weatherPoll"><option value="300">5 min</option><option value="600">10 min</option><option value="900">15 min</option></select></div></div><div class="subhead">Farbige Darstellung</div><label class="check"><input id="weatherTempColor" type="checkbox"> Aktuelle Temperatur farbig darstellen</label></div></section>

<section id="flugradar" class="page"><div class="card"><h2>Flugradar</h2><p class="hint">Live von adsb.lol. Ort suchen und den Beobachtungspunkt auf der Karte exakt setzen. Standort gilt nur für dieses Programm.</p><label>Ort suchen</label><input id="planeSearch" placeholder="z. B. Schönefeld"><div class="actions"><button class="action sec" type="button" id="planeSearchBtn">Suchen</button><button class="action sec" type="button" id="planeMapBtn">Auf Karte auswählen</button></div><div id="planeResults"></div><div id="planeMapWrap" class="mapwrap hidden"><div id="planeMap" class="map"></div><div class="mapnote">Kartenmaterial © OpenStreetMap-Mitwirkende. Auf die gewünschte Position tippen, um den Standort exakt zu setzen.</div></div><label>Ausgewählter Standort</label><input id="planePlace" readonly><label>Anzeigename <span class="mini">optional, leer = gefundener Ortsname</span></label><input id="planeDisplayName" placeholder="z. B. ZUHAUSE"><div class="subhead">Ortsnamen</div><label class="check"><input id="planeGeoGerman" type="checkbox"> Auf Deutsch anzeigen, wenn verfügbar</label><div class="subhead">Standortname Darstellung</div><label class="check"><input id="planeShowPlace" type="checkbox"> Auf dem Display anzeigen</label><div id="planePlaceToggleBlock" class="depblock"><label>Standortname Wechselintervall</label><select id="planePlaceToggleSec"><option value="0">Kein Wechsel</option><option value="5">5 s</option><option value="10">10 s</option><option value="15">15 s</option><option value="30">30 s</option></select></div><div class="row"><div><label>Breitengrad</label><input id="planeLat" inputmode="decimal"></div><div><label>Längengrad</label><input id="planeLon" inputmode="decimal"></div></div><div class="row"><div><label>Radius</label><select id="planeRadiusKm"><option value="10">10 km</option><option value="25">25 km</option><option value="50">50 km</option><option value="100">100 km</option></select></div><div><label>Aktualisierung</label><select id="planePoll"><option value="5">5 s</option><option value="10">10 s</option><option value="15">15 s</option><option value="30">30 s</option><option value="60">1 min</option><option value="180">3 min</option></select></div></div><label class="check"><input id="planeShowAge" type="checkbox" checked> Zeit seit letztem Abruf anzeigen</label><p class="mini">Zeigt die Sekunden seit dem letzten erfolgreichen Positionsabruf.</p><label>adsb.lol API-Key <span class="mini">optional</span></label><input id="adsbLolKey" type="password" autocomplete="off" placeholder="API-Key eintragen"><p class="mini">Ohne Schlüssel erlaubt adsb.lol nur eine begrenzte Zahl von Anfragen pro Tag und Internetanschluss; für den Dauerbetrieb ohne Schlüssel 3 min wählen. Schlüssel gibt es über die Anfrageseite von adsb.lol. „-“ eintragen und speichern entfernt einen gespeicherten Schlüssel.</p><p class="mini">adsb.lol kann Abfragen je nach Serverauslastung vorübergehend begrenzen. Während einer Anbieterpause bleibt das zuletzt empfangene Flugzeug stehen. Nach einer Anbieterbegrenzung verlängert sich das Aktualisierungsintervall automatisch. Nach 10 Minuten ohne erneute Begrenzung gilt wieder das ausgewählte Intervall.</p></div></section>

<section id="flugtracker" class="page"><div class="card"><h2>Flug-Tracker</h2><p class="hint">Verfolgt genau einen Flug. Plan-/Ist-Zeiten kommen von aviationstack; Höhe und Geschwindigkeit während des Fluges zusätzlich über adsb.lol.</p><label>Flugnummer</label><input id="trackedFlightNo" placeholder="z. B. AF1234" autocapitalize="characters"><label>aviationstack API-Key</label><input id="aviationstackKey" type="password" autocomplete="off" placeholder="API-Key eintragen"><label>Status-Aktualisierung</label><select id="trackerStatusPollMin"><option value="10">10 min</option><option value="15">15 min</option><option value="30">30 min</option><option value="60">60 min</option></select><p class="mini">Statusabfragen laufen sparsam und nur, wenn der Tracker tatsächlich angezeigt wird.</p></div></section>

<section id="countdown" class="page"><p class="hint">Vier dauerhaft gespeicherte Slots. Tages-Countdowns sind reine Kalendertage; Zeit-Countdowns laufen nach Null automatisch mit Plus weiter.</p><div id="countdownSlots"></div></section>

<section id="system" class="page"><div class="card"><h2>Sprache / Language</h2><select id="lang"><option value="0">Deutsch</option><option value="1">English</option></select></div><div class="card"><h2>WLAN</h2><div class="actions"><button class="action sec" type="button" id="wifiScanBtn">Netzwerke suchen</button></div><div id="wifiResults"></div><label>SSID</label><input id="ssid" autocomplete="off"><label>Passwort</label><input id="pass" type="password" placeholder="leer = gespeichertes Passwort behalten"><label>Hostname</label><input id="hostname"></div><div class="card"><h2>Status</h2><div id="status" class="status">lade…</div></div><div class="card"><h2>Diagnose</h2><div class="actions"><button class="action sec" type="button" id="logBtn">Log laden</button><button class="action sec" type="button" id="logCopyBtn">Log kopieren</button></div><pre id="diagnosticLog" class="status" style="overflow-wrap:anywhere">Noch nicht geladen.</pre></div><div class="card"><details id="geoAdvanced"><summary>Erweiterte Einstellungen</summary><h3>Ortsdienst</h3><p class="mini">Ortsauswahl über die Internetverbindung dieses Browsers. © <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">OpenStreetMap-Mitwirkende</a>. <a href="https://operations.osmfoundation.org/policies/nominatim/" target="_blank" rel="noopener">Nutzungsregeln</a>: nur manuelle Auswahl, höchstens eine Anfrage pro Sekunde für die Anwendung.</p><label>Adresse des Ortsdienstes</label><input id="geoProviderUrl" type="url"><p class="mini">Nominatim-kompatibler Dienst. Gilt für diesen Browser.</p><button class="action sec" type="button" id="geoProviderBtn">Ortsdienst übernehmen</button></details></div><div class="card"><h2>Wartung</h2><a class="action sec" href="/update">Firmware-Update</a> <button class="action sec" type="button" id="rebootBtn">Neustart</button></div><div class="card" id="serviceInfo"><h2>Dienste und Datenquellen</h2><p class="mini">Für die verwendeten Dienste gelten die Bedingungen der jeweiligen Anbieter. Daten und Verfügbarkeit können eingeschränkt sein.</p><ul class="serviceList"><li><strong>Abfahrten</strong><br>Verkehrsverbund Oberelbe (VVO) · <a href="https://www.vvo-online.de/de/impressum/index.cshtml" target="_blank" rel="noopener">Anbieterinformationen</a></li><li><strong>Wetter</strong><br>Open-Meteo · <a href="https://open-meteo.com/en/terms" target="_blank" rel="noopener">Nutzungsbedingungen</a></li><li><strong>Radarpositionen und Routen</strong><br>adsb.lol · <a href="https://www.adsb.lol/docs/open-data/api/" target="_blank" rel="noopener">API und Datenlizenz</a></li><li><strong>Zusatzdaten und Routen</strong><br>adsbdb · <a href="https://github.com/mrjackwills/adsbdb#readme" target="_blank" rel="noopener">Dokumentation und Datenbedingungen</a></li><li><strong>Flug-Tracker</strong><br>aviationstack · <a href="https://aviationstack.com/terms" target="_blank" rel="noopener">Nutzungsbedingungen</a></li><li><strong>Ortsnamen</strong><br>Nominatim / OpenStreetMap · <a href="https://operations.osmfoundation.org/policies/nominatim/" target="_blank" rel="noopener">Nutzungsregeln</a></li><li><strong>Karten</strong><br>OpenStreetMap · <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">Copyright und Datenlizenz</a></li><li><strong>Kartenkacheln</strong><br>OpenStreetMap-Kachelserver · <a href="https://operations.osmfoundation.org/policies/tiles/" target="_blank" rel="noopener">Nutzungsregeln</a></li><li><strong>Kartenbibliothek</strong><br>Leaflet / unpkg · <a href="https://leafletjs.com/" target="_blank" rel="noopener">Projektinformationen</a></li><li><strong>Uhrzeit</strong><br>NTP Pool / Cloudflare · <a href="https://www.ntppool.org/en/use.html" target="_blank" rel="noopener">Nutzungshinweise</a></li></ul><p class="mini">Nominatim: nur manuelle Ortssuche, höchstens eine Anfrage pro Sekunde für die Anwendung; Suchergebnisse werden zwischengespeichert. Open-Meteo: die kostenlose API ist für nichtkommerzielle Nutzung vorgesehen. aviationstack benötigt einen eigenen API-Schlüssel und unterliegt dessen Tarifgrenzen.</p><p class="mini">adsbdb nennt gesonderte Weiterverwendungsbedingungen für Routendaten. Links und Bedingungen zuletzt geprüft am 11.10.2026.</p></div></section>
<div class="sticky"><button class="action" type="button" id="saveBtn">Einstellungen speichern</button></div><footer class="footer">PixelStop · entwickelt von du13500 · <a href="https://github.com/du13500/pixelstop" target="_blank" rel="noopener">GitHub</a></footer></div><div id="toast" class="toast" role="status" aria-live="polite"></div>
<script>
let I18N=null,LANG='de';
function trText(t){if(!I18N||!t)return t;const k=t.trim();if(!k)return t;const e=I18N.exact[k];if(e!==undefined)return t.replace(k,e);let o=t;for(const p in I18N.prefix)if(k.startsWith(p)){o=t.replace(p,I18N.prefix[p]);break}for(const f in I18N.frag)o=o.split(f).join(I18N.frag[f]);if(o.includes('\n'))for(const l in I18N.lines)o=o.split(l).join(I18N.lines[l]);return o}
function trNode(n){if(!n)return;if(n.nodeType===3){const v=trText(n.nodeValue);if(v!==n.nodeValue)n.nodeValue=v;return}if(n.nodeType!==1||n.tagName==='SCRIPT'||n.tagName==='STYLE'||n.closest('#timeZone,#lang'))return;for(const a of['placeholder','title','aria-label']){const o=n.getAttribute(a);if(o){const v=trText(o);if(v!==o)n.setAttribute(a,v)}}for(const c of n.childNodes)trNode(c)}
async function useLang(l){if(l===LANG)return;if(l!=='en'){try{localStorage.setItem('pixelstop.lang','de')}catch(e){}location.reload();return}try{const r=await fetch('/i18n/en.json');if(!r.ok)return;I18N=await r.json();LANG='en';document.documentElement.lang='en';try{localStorage.setItem('pixelstop.lang','en')}catch(e){}trNode(document.body);new MutationObserver(ms=>{for(const m of ms){if(m.type==='childList')m.addedNodes.forEach(trNode);else trNode(m.target)}}).observe(document.body,{subtree:true,childList:true,characterData:true,attributes:true,attributeFilter:['placeholder','title']})}catch(e){}}

const $=id=>document.getElementById(id),clamp=(n,a,b)=>Math.max(a,Math.min(b,Number(n)||0));
const modeNames=['Abfahrten','Wetter','Flugradar','Uhr','Flug-Tracker','Countdown 1','Countdown 2','Countdown 3','Countdown 4'];
const modeDefaults=[30,20,20,15,20,20,20,20,20];
function friendly(e){if(e&&e.userMessage)return e.userMessage;if(e instanceof TypeError||String(e).includes('fetch'))return 'Verbindung zum Würfel fehlgeschlagen. Ist der Würfel eingeschaltet und mit dem WLAN verbunden?';return e?.message||'Unbekannter Fehler.'}
async function fetchJson(url,opt){let r;const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),25000);try{r=await fetch(url,{...opt,signal:controller.signal})}catch(e){let x=new Error();x.userMessage='Verbindung zum Würfel fehlgeschlagen. Ist der Würfel eingeschaltet und mit dem WLAN verbunden?';throw x}finally{clearTimeout(timer)}let d={};try{d=await r.json()}catch(e){}if(!r.ok||d.ok===false){let x=new Error();x.userMessage=d.error||('Anfrage fehlgeschlagen ('+r.status+').');throw x}return d}
function showBox(id,msg,err=false){const span=document.createElement('span');span.className='hint '+(err?'err':'');span.textContent=msg;$(id).replaceChildren(span)}
let toastTimer;function toast(msg,type='ok',ms=2200){const t=$('toast');clearTimeout(toastTimer);t.textContent=msg;t.className='toast '+type+' show';toastTimer=setTimeout(()=>t.classList.remove('show'),ms)}
const maps={weather:null,plane:null},markers={weather:null,plane:null};let leafletPromise=null;
function loadLeaflet(){if(window.L)return Promise.resolve();if(leafletPromise)return leafletPromise;leafletPromise=new Promise((resolve,reject)=>{if(!document.querySelector('link[data-leaflet]')){let l=document.createElement('link');l.rel='stylesheet';l.href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css';l.dataset.leaflet='1';document.head.appendChild(l)}let sc=document.createElement('script');sc.src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js';sc.onload=resolve;sc.onerror=()=>{leafletPromise=null;reject(new Error('Leaflet konnte nicht geladen werden'))};document.head.appendChild(sc)});return leafletPromise}
function parseCoord(v){const s=String(v??'').trim().replace(',','.');return s?Number(s):NaN}function wrapLongitude(lon){return ((lon+180)%360+360)%360-180}function coords(kind){return[parseCoord($(kind+'Lat').value),parseCoord($(kind+'Lon').value)]}
function setMapPoint(kind,lat,lon,center=true){if(!Number.isFinite(lat)||!Number.isFinite(lon)||lat<-90||lat>90)return;lon=wrapLongitude(lon);$(kind+'Lat').value=lat.toFixed(5);$(kind+'Lon').value=lon.toFixed(5);if(maps[kind]){markers[kind].setLatLng([lat,lon]);if(center)maps[kind].setView([lat,lon],Math.max(maps[kind].getZoom(),14))}}
function geoGermanFor(kind){const e=$(kind==='weather'?'weatherGeoGerman':'planeGeoGerman');return !!(e&&e.checked)}
const geoPending={weather:null,plane:null},geoGeneration={weather:0,plane:0};let geoTimer=null,geoBusy=false,geoLastStart=0,saving=false;
function updateSaveGate(){const waiting=geoBusy||geoPending.weather!==null||geoPending.plane!==null;const b=$('saveBtn');b.disabled=waiting||saving;b.textContent=saving?'Wird gespeichert…':waiting?'Ortsname wird ermittelt…':'Einstellungen speichern'}
function reversePlace(kind,lat,lon){if(saving)return;if(navigator.onLine===false){++geoGeneration[kind];geoPending[kind]=null;showBox(kind+'Results','Internetverbindung erforderlich',true);updateSaveGate();return}lon=wrapLongitude(lon);const generation=++geoGeneration[kind];geoPending[kind]={lat,lon,generation};const field=kind==='weather'?'weatherCity':'planePlace';$(field).value='';$(kind+'Search').value='';showBox(kind+'Results','Kartenposition übernommen: '+lat.toFixed(5)+', '+lon.toFixed(5)+' · Ortsname wird ermittelt…');updateSaveGate();scheduleGeo()}
// Geo HTTPS and JSON belong to the browser, not the ESP8266 heap.
// Provider can be changed here without a firmware update. No autocomplete.
let geoProvider='https://nominatim.openstreetmap.org/';
try{geoProvider=localStorage.getItem('smalltv.geoProvider')||geoProvider}catch(e){}
let geoPauseUntil=0;const geoCache=new Map();
try{for(const [k,v] of JSON.parse(localStorage.getItem('smalltv.geoCache')||'[]').slice(-32))if(v&&Date.now()-v.time<86400000)geoCache.set(k,v)}catch(e){}
$('geoProviderUrl').value=geoProvider;
$('geoProviderBtn').addEventListener('click',()=>{try{const u=new URL($('geoProviderUrl').value.trim());if(u.protocol!=='https:'||u.search||u.hash||u.username||u.password)throw new Error();if(geoBusy||geoPending.weather||geoPending.plane){toast('Bitte die laufende Ortsauswahl abwarten.','warn');return}geoProvider=u.href.endsWith('/')?u.href:u.href+'/';localStorage.setItem('smalltv.geoProvider',geoProvider);geoCache.clear();localStorage.removeItem('smalltv.geoCache');toast('Ortsdienst für diesen Browser gespeichert')}catch(e){toast('Bitte eine gültige HTTPS-Adresse eingeben.','warn')}});
function geoName(d,german){const names=d.namedetails||{},a=d.address||{};return String((german&&names['name:de'])||names.name||d.name||['neighbourhood','quarter','suburb','city_district','borough','hamlet','village','town','city','municipality','island','islet','sea','ocean','water','county','state_district','state','region','country'].map(k=>a[k]).find(Boolean)||(d.display_name||'').split(',')[0]||'').trim()}
function geoOfflineError(){const e=new Error('Internetverbindung erforderlich');e.offline=true;return e}
async function browserGeo(point,kind){
 if(navigator.onLine===false)throw geoOfflineError();
 const german=geoGermanFor(kind),search=!!point.q,u=new URL(search?'search':'reverse',geoProvider);
 u.search=new URLSearchParams(search?{format:'jsonv2',namedetails:'1',limit:'8',q:point.q}:{format:'jsonv2',addressdetails:'1',namedetails:'1',zoom:'13',layer:'address,natural',lat:point.lat.toFixed(6),lon:point.lon.toFixed(6)}).toString();
 // Empty language selects local names instead of the browser's preferred language.
 u.searchParams.set('accept-language',german?'de':'');
 const key=u.href,cached=geoCache.get(key);if(cached&&Date.now()-cached.time<86400000)return cached.data;
 const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),15000);let r,d;
 try{
  r=await fetch(u.href,{mode:'cors',credentials:'omit',signal:controller.signal,referrer:new URL('/pixelstop/',location.href).href,referrerPolicy:'unsafe-url'});
  if(r.status===429||r.status===503){const retry=r.headers.get('Retry-After'),seconds=Number(retry),date=Date.parse(retry);geoPauseUntil=Date.now()+Math.max(60000,Number.isFinite(seconds)?seconds*1000:Number.isFinite(date)?date-Date.now():0);throw new Error('Der Ortsdienst bittet um eine Pause. Bitte später erneut versuchen.')}
  if(r.status===404&&!search)d={};else{if(!r.ok)throw new Error('Ortsdienst antwortet mit HTTP '+r.status+'.');d=await r.json();if(d.error)throw new Error(d.error)}
 }catch(e){if(navigator.onLine===false)throw geoOfflineError();if(!r||e.name==='AbortError')throw new Error('Ortsdienst nicht erreichbar. Internetverbindung des Browsers prüfen.');throw e}finally{clearTimeout(timer)}
 const data=search?{places:(Array.isArray(d)?d:[]).map(x=>({name:geoName(x,german),label:String(x.display_name||geoName(x,german)),lat:Number(x.lat),lon:Number(x.lon)})).filter(x=>x.name&&Number.isFinite(x.lat)&&Math.abs(x.lat)<=90&&Number.isFinite(x.lon)&&Math.abs(x.lon)<=180)}:{name:geoName(d,german)};
 if(geoCache.size>=32)geoCache.delete(geoCache.keys().next().value);geoCache.set(key,{time:Date.now(),data});try{localStorage.setItem('smalltv.geoCache',JSON.stringify([...geoCache]))}catch(e){}return data;
}
function scheduleGeo(){if(geoBusy)return;clearTimeout(geoTimer);geoTimer=setTimeout(runGeo,Math.max(350,1100-(Date.now()-geoLastStart)))}
async function runGeo(){const kind=geoPending.weather?'weather':(geoPending.plane?'plane':null);if(!kind)return;const point=geoPending[kind];geoPending[kind]=null;geoBusy=true;geoLastStart=Date.now();const field=kind==='weather'?'weatherCity':'planePlace';try{if(Date.now()<geoPauseUntil)throw new Error('Der Ortsdienst bittet um eine Pause. Bitte später erneut versuchen.');const d=await browserGeo(point,kind);if(point.generation===geoGeneration[kind]){if(point.q)renderPlaceResults(kind,d.places);else if(d.name){$(field).value=d.name;$(kind+'Search').value=d.name;showBox(kind+'Results','Ausgewählt: '+d.name+' · '+point.lat.toFixed(5)+', '+point.lon.toFixed(5))}else showBox(kind+'Results','Position ausgewählt · kein Ortsname verfügbar. Optional einen Anzeigenamen eingeben.')}}catch(e){if(point.generation===geoGeneration[kind])showBox(kind+'Results',e.offline?e.message:(point.q?'':'Position ausgewählt · ')+(e.message||friendly(e))+(point.q?'':' Optional einen Anzeigenamen eingeben.'),true)}finally{geoBusy=false;updateSaveGate();if(geoPending.weather||geoPending.plane)scheduleGeo()}}
async function ensureMap(kind){await loadLeaflet();let [lat,lon]=coords(kind);if(!Number.isFinite(lat)||!Number.isFinite(lon)){lat=51.0504;lon=13.7373}if(!maps[kind]){const m=L.map(kind+'Map').setView([lat,lon],14);L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png',{maxZoom:19,attribution:'&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank">OpenStreetMap-Mitwirkende</a>'}).addTo(m);const marker=L.marker([lat,lon],{draggable:true}).addTo(m);const picked=p=>{const lon=wrapLongitude(p.lng);setMapPoint(kind,p.lat,lon,false);reversePlace(kind,p.lat,lon)};m.on('click',e=>picked(e.latlng));marker.on('dragend',()=>picked(marker.getLatLng()));maps[kind]=m;markers[kind]=marker}else{maps[kind].setView([lat,lon],maps[kind].getZoom());markers[kind].setLatLng([lat,lon])}setTimeout(()=>maps[kind].invalidateSize(),50)}
async function toggleMap(kind){const w=$(kind+'MapWrap'),opening=w.classList.contains('hidden');if(opening&&navigator.onLine===false){showBox(kind+'Results','Internetverbindung erforderlich',true);return}w.classList.toggle('hidden',!opening);if(!opening)return;showBox(kind+'Results','Karte wird geladen…');try{await ensureMap(kind);showBox(kind+'Results','Auf die gewünschte Position in der Karte tippen.')}catch(e){w.classList.add('hidden');showBox(kind+'Results',navigator.onLine===false?'Internetverbindung erforderlich':'Karte konnte nicht geladen werden. Bitte Internetverbindung prüfen oder Koordinaten manuell eingeben.',true)}}
function syncMapFromInputs(kind){let [lat,lon]=coords(kind);if(Number.isFinite(lat)&&Number.isFinite(lon)&&maps[kind]){markers[kind].setLatLng([lat,lon]);maps[kind].setView([lat,lon],maps[kind].getZoom())}}
document.querySelectorAll('.tab').forEach(b=>b.addEventListener('click',()=>{document.querySelectorAll('.tab,.page').forEach(x=>x.classList.remove('on'));b.classList.add('on');$(b.dataset.p).classList.add('on')}));
function hhmm(m){let h=Math.floor(m/60)%24,mm=m%60;return String(h).padStart(2,'0')+':'+String(mm).padStart(2,'0')}function mins(s){let p=(s||'00:00').split(':');return (+p[0])*60+(+p[1])}
function buildModeRows(){let h='';[0,1,2,4,3,5,6,7,8].forEach(i=>h+=`<div class="modeRow"><label class="modeLeft"><input type="checkbox" id="modeOn${i}"><span>${modeNames[i]}</span></label><select id="modeSec${i}"><option value="10">10 s</option><option value="15">15 s</option><option value="20">20 s</option><option value="30">30 s</option><option value="45">45 s</option><option value="60">60 s</option></select></div>`);$('modeRows').innerHTML=h}
function buildCountdownCards(){const root=$('countdownSlots');let html='';for(let i=0;i<4;i++)html+=`<div class="card"><h2>Countdown ${i+1}</h2><div class="colorrow"><div><label>Optionaler Titel</label><input id="cdTitle${i}" maxlength="24" placeholder="z. B. URLAUB"></div><div><label>Titelfarbe</label><input id="cdTitleColor${i}" type="color" value="#ffffff"></div></div><label>Timer-Art</label><select id="cdType${i}"><option value="0">Tage</option><option value="1">Stunden : Minuten : Sekunden</option><option value="2">Minuten : Sekunden</option></select><div id="cdDays${i}"><label>Zieltag</label><input id="cdDate${i}" type="date"></div><div id="cdHms${i}" class="triple hidden"><div><label>Stunden</label><input id="cdH${i}" type="number" min="0" max="999" value="0"></div><div><label>Minuten</label><input id="cdM${i}" type="number" min="0" max="59" value="10"></div><div><label>Sekunden</label><input id="cdS${i}" type="number" min="0" max="59" value="0"></div></div><div id="cdMs${i}" class="row hidden"><div><label>Minuten</label><input id="cdOnlyM${i}" type="number" min="0" max="9999" value="10"></div><div><label>Sekunden</label><input id="cdOnlyS${i}" type="number" min="0" max="59" value="0"></div></div><div class="colorrow"><div><label>Countdown-Farbe</label><span class="mini" id="cdStatus${i}">inaktiv</span></div><div><label>Farbe</label><input id="cdCountColor${i}" type="color" value="#ff5d56"></div></div><div class="actions"><button type="button" class="action" id="cdStart${i}">Start / aktivieren</button><button type="button" class="action sec" id="cdStop${i}">Stoppen</button></div></div>`;root.innerHTML=html;for(let i=0;i<4;i++){$(`cdType${i}`).addEventListener('change',()=>updateCdFields(i));$(`cdStart${i}`).addEventListener('click',()=>countdownAction(i,'start'));$(`cdStop${i}`).addEventListener('click',()=>countdownAction(i,'stop'))}}
function updateCdFields(i){let t=+$(`cdType${i}`).value;$(`cdDays${i}`).classList.toggle('hidden',t!==0);$(`cdHms${i}`).classList.toggle('hidden',t!==1);$(`cdMs${i}`).classList.toggle('hidden',t!==2)}
function countdownPayload(i){let t=+$(`cdType${i}`).value,d=0;if(t===1)d=clamp($(`cdH${i}`).value,0,999)*3600+clamp($(`cdM${i}`).value,0,59)*60+clamp($(`cdS${i}`).value,0,59);if(t===2)d=clamp($(`cdOnlyM${i}`).value,0,9999)*60+clamp($(`cdOnlyS${i}`).value,0,59);return{title:$(`cdTitle${i}`).value,titleColor:$(`cdTitleColor${i}`).value,countColor:$(`cdCountColor${i}`).value,type:t,targetDate:$(`cdDate${i}`).value,durationSec:d}}
function loadCountdown(i,c){$(`cdTitle${i}`).value=c.title||'';$(`cdTitleColor${i}`).value=c.titleColor||'#ffffff';$(`cdCountColor${i}`).value=c.countColor||'#ff5d56';$(`cdType${i}`).value=c.type??0;$(`cdDate${i}`).value=c.targetDate||'';let d=Number(c.durationSec||0),h=Math.floor(d/3600),m=Math.floor((d%3600)/60),sec=d%60;$(`cdH${i}`).value=h;$(`cdM${i}`).value=m;$(`cdS${i}`).value=sec;$(`cdOnlyM${i}`).value=Math.floor(d/60);$(`cdOnlyS${i}`).value=sec;$(`cdStatus${i}`).textContent=c.active?'aktiv':'inaktiv';updateCdFields(i)}
async function countdownAction(i,action){let body=countdownPayload(i);body.slot=i;body.action=action;try{await fetchJson('/api/countdown',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});await load();toast(action==='start'?'Countdown gestartet':'Countdown gestoppt')}catch(e){try{let c=await load();let wanted=action==='start';if(c?.countdowns?.[i]?.active===wanted){toast(action==='start'?'Countdown gestartet':'Countdown gestoppt');return}}catch(_){}toast(friendly(e),'err',4200)}}
$('brightness').addEventListener('input',()=> $('brv').textContent=$('brightness').value);$('nightBrightness').addEventListener('input',()=> $('nbv').textContent=$('nightBrightness').value);
async function load(){try{let c=await fetchJson('/api/config');$('lang').value=String(+c.lang===1?1:0);useLang(+c.lang===1?'en':'de');['ssid','hostname','timeZone','stopName','stopId','weatherCity','weatherDisplayName','planePlace','planeDisplayName','trackedFlightNo'].forEach(k=>{if($(k))$(k).value=c[k]??''});['brightness','departureCount','departurePoll','weatherPoll','planeRadiusKm','planePoll','planePlaceToggleSec','trackerStatusPollMin'].forEach(k=>$(k).value=c[k]);['weatherLat','weatherLon','planeLat','planeLon'].forEach(k=>$(k).value=Number(c[k]).toFixed(4));$('weatherSearch').value=c.weatherCity||'';$('planeSearch').value=c.planePlace||'';$('nightEnabled').checked=!!c.nightEnabled;$('nightStart').value=hhmm(c.nightStart);$('nightEnd').value=hhmm(c.nightEnd);$('nightBrightness').value=c.nightBrightness;$('weatherTempColor').checked=!!c.weatherTempColor;$('weatherUnit').value=c.weatherFahrenheit?'F':'C';$('departureCompactDestinations').checked=!!c.departureCompactDestinations;$('departureAbbreviate').checked=c.departureAbbreviate!==false;$('departureDropTown').checked=c.departureDropTown!==false;$('weatherGeoGerman').checked=!!c.geoGermanNames;$('planeGeoGerman').checked=!!c.geoGermanNames;$('planeShowAge').checked=c.planeShowAge!==false;$('planeShowPlace').checked=!!c.planeShowPlace;updatePlanePlaceControls();$('brv').textContent=c.brightness;$('nbv').textContent=c.nightBrightness;let mask=Number(c.rotationMask||0),secs=c.rotationSec||modeDefaults;for(let i=0;i<9;i++){$(`modeOn${i}`).checked=!!(mask&(1<<i));$(`modeSec${i}`).value=secs[i]||modeDefaults[i]}let mm=Number(c.vvoMotMask??127);$('motTrain').checked=!!(mm&1);$('motSuburban').checked=!!(mm&2);$('motTram').checked=!!(mm&4);$('motBus').checked=!!(mm&8);$('motCable').checked=!!(mm&16);$('motFerry').checked=!!(mm&32);$('motTaxi').checked=!!(mm&64);$('aviationstackKey').value='';$('aviationstackKey').placeholder=c.aviationstackKeySet?'API-Key gespeichert, leer = behalten':'API-Key eintragen';$('adsbLolKey').value='';$('adsbLolKey').placeholder=c.adsbLolKeySet?'API-Key gespeichert, leer = behalten':'API-Key eintragen';(c.countdowns||[]).forEach((x,i)=>{if(i<4)loadCountdown(i,x)});statusNow();return c}catch(e){toast(friendly(e),'err',4200);throw e}}
async function searchStop(){const box=$('stopResults'),q=$('stopName').value.trim();if(q.length<2){showBox('stopResults','Bitte mindestens zwei Zeichen eingeben.');return}showBox('stopResults','suche beim VVO…');try{const d=await fetchJson('/api/vvo/search?q='+encodeURIComponent(q));if(!d.stops.length){showBox('stopResults','Keine Haltestelle gefunden.');return}box.innerHTML=d.stops.map((x,i)=>`<button class="pickres" type="button" data-i="${i}">${x.label}</button>`).join('');box.querySelectorAll('button').forEach((b,i)=>b.addEventListener('click',()=>{const x=d.stops[i];$('stopId').value=x.id;$('stopName').value=x.name;showBox('stopResults','Ausgewählt: '+x.label)}))}catch(e){showBox('stopResults',friendly(e),true)}}
function searchPlace(kind){if(saving)return;if(navigator.onLine===false){++geoGeneration[kind];geoPending[kind]=null;showBox(kind+'Results','Internetverbindung erforderlich',true);updateSaveGate();return}const q=$(kind+'Search').value.trim();if(q.length<2){showBox(kind+'Results','Bitte mindestens zwei Zeichen eingeben.');return}geoPending[kind]={q,generation:++geoGeneration[kind]};showBox(kind+'Results','suche Ort…');updateSaveGate();scheduleGeo()}
function renderPlaceResults(kind,places){const box=$(kind+'Results');if(!places.length){showBox(kind+'Results','Kein Ort gefunden.');return}box.replaceChildren();places.forEach(x=>{const b=document.createElement('button'),s=document.createElement('span');b.type='button';b.className='pickres';b.textContent=x.label;s.className='sig';s.textContent=x.lat.toFixed(4)+', '+x.lon.toFixed(4);b.append(s);b.addEventListener('click',()=>{++geoGeneration[kind];geoPending[kind]=null;$(kind==='weather'?'weatherCity':'planePlace').value=x.name;setMapPoint(kind,x.lat,x.lon,true);showBox(kind+'Results','Ausgewählt: '+x.label+(maps[kind]?' · Marker auf Ortsmitte gesetzt':''));updateSaveGate()});box.append(b)});const note=document.createElement('p'),link=document.createElement('a');note.className='mini';note.textContent='Ortsdaten © ';link.href='https://www.openstreetmap.org/copyright';link.target='_blank';link.rel='noopener';link.textContent='OpenStreetMap-Mitwirkende';note.append(link);box.append(note)}
function updatePlanePlaceControls(){const off=!$('planeShowPlace').checked;$('planePlaceToggleSec').disabled=off;$('planePlaceToggleBlock').classList.toggle('disabled',off)}
function syncGeoLanguage(source){const v=$(source).checked;$('weatherGeoGerman').checked=v;$('planeGeoGerman').checked=v}
function signalBars(r){return r>=-50?'▁▂▄▆█':r>=-60?'▁▂▄▆':r>=-70?'▁▂▄':r>=-80?'▁▂':'▁'}
async function scanWifi(){showBox('wifiResults','suche WLAN-Netze…');try{let d=await fetchJson('/api/wifi/scan'),best=new Map();(d.networks||[]).forEach(n=>{if(!n.ssid)return;let o=best.get(n.ssid);if(!o||n.rssi>o.rssi)best.set(n.ssid,n)});let a=[...best.values()].sort((x,y)=>y.rssi-x.rssi);if(!a.length){showBox('wifiResults','Keine WLAN-Netze gefunden.');return}$('wifiResults').innerHTML=a.map((n,i)=>`<button class="pickres" type="button" data-i="${i}">${n.open?'○':'●'} ${n.ssid}<span class="sig">${signalBars(n.rssi)} ${n.rssi} dBm</span></button>`).join('');$('wifiResults').querySelectorAll('button').forEach((b,i)=>b.addEventListener('click',()=>{$('ssid').value=a[i].ssid;showBox('wifiResults','Ausgewählt: '+a[i].ssid)}))}catch(e){showBox('wifiResults',friendly(e),true)}}
async function save(){if(saving||geoBusy||geoPending.weather||geoPending.plane)return;let mask=0,rotationSec=[];for(let i=0;i<9;i++){if($(`modeOn${i}`).checked)mask|=1<<i;rotationSec.push(+$(`modeSec${i}`).value)}if(!mask){toast('Bitte mindestens ein Programm für die Anzeige auswählen.','warn',3500);return}let mot=($('motTrain').checked?1:0)|($('motSuburban').checked?2:0)|($('motTram').checked?4:0)|($('motBus').checked?8:0)|($('motCable').checked?16:0)|($('motFerry').checked?32:0)|($('motTaxi').checked?64:0);if(!mot){toast('Bitte mindestens ein Verkehrsmittel auswählen.','warn',3500);return}let [wlat,wlon]=coords('weather'),[plat,plon]=coords('plane');if(!Number.isFinite(wlat)||wlat<-90||wlat>90||!Number.isFinite(wlon)||wlon<-180||wlon>180){toast('Bitte gültige Wetter-Koordinaten eingeben.','warn',3500);return}if(!Number.isFinite(plat)||plat<-90||plat>90||!Number.isFinite(plon)||plon<-180||plon>180){toast('Bitte gültige Flugradar-Koordinaten eingeben.','warn',3500);return}let o={lang:+$('lang').value,ssid:$('ssid').value,pass:$('pass').value,hostname:$('hostname').value,rotationMask:mask,rotationSec,brightness:+$('brightness').value,timeZone:$('timeZone').value,nightEnabled:$('nightEnabled').checked,nightStart:mins($('nightStart').value),nightEnd:mins($('nightEnd').value),nightBrightness:+$('nightBrightness').value,stopName:$('stopName').value,stopId:$('stopId').value,departureCount:+$('departureCount').value,departurePoll:+$('departurePoll').value,vvoMotMask:mot,departureCompactDestinations:$('departureCompactDestinations').checked,departureAbbreviate:$('departureAbbreviate').checked,departureDropTown:$('departureDropTown').checked,weatherCity:$('weatherCity').value,weatherDisplayName:$('weatherDisplayName').value,weatherLat:wlat,weatherLon:wlon,weatherPoll:+$('weatherPoll').value,weatherTempColor:$('weatherTempColor').checked,weatherFahrenheit:$('weatherUnit').value==='F',geoGermanNames:$('weatherGeoGerman').checked,planePlace:$('planePlace').value,planeDisplayName:$('planeDisplayName').value,planeLat:plat,planeLon:plon,planeRadiusKm:+$('planeRadiusKm').value,planePoll:+$('planePoll').value,planeShowAge:$('planeShowAge').checked,planeShowPlace:$('planeShowPlace').checked,planePlaceToggleSec:+$('planePlaceToggleSec').value,trackedFlightNo:$('trackedFlightNo').value,aviationstackKey:$('aviationstackKey').value,adsbLolKey:$('adsbLolKey').value,trackerStatusPollMin:+$('trackerStatusPollMin').value,countdowns:[0,1,2,3].map(countdownPayload)};saving=true;updateSaveGate();try{let r=await fetchJson('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(o)});toast(r.reboot?'Gespeichert · PixelStop startet neu':'✓ Einstellungen gespeichert','ok',r.reboot?5500:2200);if(r.reboot)setTimeout(()=>location.reload(),7000);else await load()}catch(e){toast(friendly(e),'err',4200)}finally{saving=false;updateSaveGate()}}
async function statusNow(){try{let s=await fetchJson('/api/status');$('status').textContent=`Firmware: ${s.fw} ${s.version}\nNetz: ${s.ssid||'-'}\nIP: ${s.ip}\nRSSI: ${s.rssi} dBm\nFreier Heap: ${s.heap} B (${s.maxBlock} B größter Block)\nFlugradar: ${s.flightStage||'-'}${s.flightHttp?` · HTTP ${s.flightHttp}`:''}${s.flightTlsRx?` · TLS RX ${s.flightTlsRx} B`:''}${s.flightTlsMinBlock?` · Blockbedarf ${s.flightTlsMinBlock} B`:''}${s.flightWaitSec?` · Pause ${s.flightWaitSec} s`:''}${s.flightMinIntervalSec?` · Mindestintervall ${s.flightMinIntervalSec} s`:''}\nadsb.lol heute: ${s.adsbToday} Anfragen${s.adsbKey?' · mit Schlüssel':` von 450 · Tagesbudget-Intervall ${s.adsbBudgetSec} s`}\nRadar-Route: ${s.routeStatus||'-'} · ${s.routeSource||'-'}${s.routeHttp?` · HTTP ${s.routeHttp}`:''}\nUptime: ${s.uptime}s\nTracker: ${s.trackerValid?'Daten vorhanden':(s.trackerError?'Fehler':'bereit')}`}catch(e){$('status').textContent='PixelStop derzeit nicht erreichbar.'}}
async function reboot(){toast('PixelStop startet neu…','ok',5000);try{await fetch('/api/reboot',{method:'POST'})}catch(e){}setTimeout(()=>location.reload(),6000)}
async function copyLog(){
try{const r=await fetch('/api/log');if(!r.ok)throw new Error();const text=await r.text();$('diagnosticLog').textContent=text;
if(window.isSecureContext&&navigator.clipboard){try{await navigator.clipboard.writeText(text);toast('Log kopiert','ok',2200);return}catch(e){}}
const range=document.createRange();range.selectNodeContents($('diagnosticLog'));const selection=window.getSelection();selection.removeAllRanges();selection.addRange(range);$('diagnosticLog').scrollIntoView({block:'center'});
let copied=false;try{copied=document.execCommand('copy')}catch(e){}toast(copied?'Log kopiert':'Log markiert · bitte Kopieren wählen',copied?'ok':'warn',4500);
}catch(e){toast('Log derzeit nicht erreichbar.','err',3500)}}
$('logCopyBtn').addEventListener('click',copyLog);
buildModeRows();buildCountdownCards();$('searchStopBtn').addEventListener('click',searchStop);$('weatherSearchBtn').addEventListener('click',()=>searchPlace('weather'));$('planeSearchBtn').addEventListener('click',()=>searchPlace('plane'));$('weatherMapBtn').addEventListener('click',()=>toggleMap('weather'));$('planeMapBtn').addEventListener('click',()=>toggleMap('plane'));$('planeShowPlace').addEventListener('change',updatePlanePlaceControls);$('weatherGeoGerman').addEventListener('change',()=>syncGeoLanguage('weatherGeoGerman'));$('planeGeoGerman').addEventListener('change',()=>syncGeoLanguage('planeGeoGerman'));['weather','plane'].forEach(k=>{ $(k+'Lat').addEventListener('change',()=>syncMapFromInputs(k));$(k+'Lon').addEventListener('change',()=>syncMapFromInputs(k))});$('wifiScanBtn').addEventListener('click',scanWifi);$('saveBtn').addEventListener('click',save);$('logBtn').addEventListener('click',async()=>{try{let r=await fetch('/api/log');if(!r.ok)throw new Error();$('diagnosticLog').textContent=await r.text()}catch(e){$('diagnosticLog').textContent='Log derzeit nicht erreichbar.'}});$('rebootBtn').addEventListener('click',reboot);try{if(localStorage.getItem('pixelstop.lang')==='en')useLang('en')}catch(e){}load();setInterval(statusNow,5000);
</script></body></html>
)HTML";


// ---- helpers ----------------------------------------------------------------
static bool nextCodepoint(const String& s, size_t& i, uint32_t& cp) {
  return nextDisplayCodepoint(s.c_str(), s.length(), i, cp);
}

static uint16_t glyphCount(const String& s) {
  uint16_t n=0; uint32_t cp=0; size_t i=0;
  while(nextCodepoint(s,i,cp)) n++;
  return n;
}

static void drawSpecialGlyph(int x,int y,uint8_t size,const uint8_t rows[7],uint16_t color) {
  for(uint8_t ry=0;ry<7;ry++) for(uint8_t rx=0;rx<5;rx++) if(pixelRow(rows+ry)&(1U<<(4-rx))) gfx->fillRect(x+rx*size,y+ry*size,size,size,color);
}

static void printDisplayTextAt(int x,int y,const String& s,uint8_t size,uint16_t color) {
  gfx->setTextSize(size); gfx->setTextColor(color);
  size_t i=0; uint32_t cp=0;
  while(nextCodepoint(s,i,cp)) {
    const uint8_t* g=specialGlyph(cp);
    if(g) drawSpecialGlyph(x,y,size,g,color);
    else if(cp>=32 && cp<127) { gfx->setCursor(x,y); gfx->write((uint8_t)cp); }
    else drawSpecialGlyph(x,y,size,missingGlyph,color);
    x += 6*size;
  }
}

static int textW(const String& s, uint8_t size) { return glyphCount(s) * 6 * size; }

static void centerText(const String& s, int y, uint8_t size, uint16_t color) {
  int x = (TFT_W - textW(s, size)) / 2;
  if (x < 0) x = 0;
  printDisplayTextAt(x,y,s,size,color);
}

static String crop(const String& s, uint8_t maxChars) {
  if (glyphCount(s) <= maxChars) return s;
  if (maxChars < 2) return "";
  size_t i=0,end=0; uint32_t cp=0; uint8_t n=0;
  while(i<s.length() && n<maxChars-1){size_t before=i;if(!nextCodepoint(s,i,cp))break;end=i;n++;(void)before;}
  end=destCropEnd(s.c_str(),end); // no "Leipzig ." with a blank before the dot
  return s.substring(0,end)+".";
}

// Cache only the active view. Compare formatted visible data, not API timestamps.
struct VisibleField { String text; int16_t x=0,y=0,w=0,h=0,tx=0,ty=0; uint16_t color=0; uint8_t size=0; bool valid=false; };
static VisibleField visibleFields[32];
static bool viewFresh=true;
static void beginView(bool force) {
  viewFresh=force;
  if(force){for(auto& f:visibleFields){f.text="";f.valid=false;}gfx->fillScreen(C_BLACK);}
}
static bool visibleText(uint8_t id,const String& text,int x,int y,uint8_t size,uint16_t color,int bx,int by,int bw,int bh,bool force=false) {
  VisibleField& old=visibleFields[id];
  if(!force&&old.valid&&old.text==text&&old.tx==x&&old.ty==y&&old.w==bw&&old.h==bh&&old.size==size&&old.color==color)return false;
  if(old.valid||!viewFresh){
    if(old.valid)gfx->fillRect(old.x,old.y,old.w,old.h,C_BLACK);
    gfx->fillRect(bx,by,bw,bh,C_BLACK);
  }
  printDisplayTextAt(x,y,text,size,color);
  old.text=text;old.tx=x;old.ty=y;old.x=bx;old.y=by;old.w=bw;old.h=bh;old.size=size;old.color=color;old.valid=true;
  // Store bounds separately from actual text alignment in the signature below.
  return true;
}
static bool visibleCenter(uint8_t id,const String& text,int y,uint8_t size,uint16_t color,int top,int height) {
  return visibleText(id,text,(TFT_W-textW(text,size))/2,y,size,color,8,top,224,height);
}

static String timeHHMM() {
  time_t now = time(nullptr);
  if (now < 1609459200) return "--:--";
  struct tm t;
  localtime_r(&now, &t);
  char b[6];
  snprintf(b, sizeof(b), "%02d:%02d", t.tm_hour, t.tm_min);
  return String(b);
}

static void applyTimeZone() {
  String rule=String(FPSTR(timeZoneRule(timeZoneIndex(cfg.timeZone.c_str()))));
  setenv("TZ",rule.c_str(),1);tzset();lastMinute=255;
}

static bool nightWindow(uint16_t now, uint16_t start, uint16_t end) {
  if (start == end) return false;
  if (start < end) return now >= start && now < end;
  return now >= start || now < end;
}

static void applyBrightness() {
  uint8_t b = cfg.brightness;
  time_t now = time(nullptr);
  if (cfg.nightEnabled && now > 1609459200) {
    struct tm t; localtime_r(&now, &t);
    uint16_t m = t.tm_hour * 60 + t.tm_min;
    if (nightWindow(m, cfg.nightStart, cfg.nightEnd)) b = cfg.nightBrightness;
  }
  if (b > 100) b = 100;
  // SmallTV backlight is active-low.
  int duty = BL_PWM_RANGE - ((int)b * BL_PWM_RANGE / 100);
  // Reapplying PWM can interrupt very dim backlighting. Update only on change.
  static int lastDuty=-1;
  if(duty==lastDuty)return;
  analogWrite(TFT_BL, duty);
  lastDuty=duty;
}

static bool modeEnabled(uint8_t mode) {
  return mode < 9 && (cfg.rotationMask & (1U << mode));
}

static uint8_t enabledModeCount() {
  uint8_t n=0; for(uint8_t i=0;i<9;i++) if(modeEnabled(i)) n++; return n;
}

static uint8_t firstEnabledMode() {
  for(uint8_t i=0;i<9;i++) if(modeEnabled(i)) return i;
  return MODE_DEPARTURES;
}

static uint8_t nextEnabledMode(uint8_t current) {
  for(uint8_t step=1;step<=9;step++) { uint8_t m=(current+step)%9; if(modeEnabled(m)) return m; }
  return current;
}

// Radar requests run only while the radar is on screen, plus a short
// prefetch before the rotation switches to it, so a fresh aircraft is ready.
static const uint32_t RADAR_PREFETCH_MS = 10000UL;
static bool radarActive() {
  if(!modeEnabled(MODE_PLANE)) return false;
  if(cfg.mode==MODE_PLANE) return true;
  if(enabledModeCount()<=1||nextEnabledMode(cfg.mode)!=MODE_PLANE) return false;
  uint32_t stayMs=(uint32_t)max((uint16_t)5,cfg.rotationSec[cfg.mode])*1000UL;
  uint32_t shown=(uint32_t)(millis()-lastModeSwitchMs);
  return shown+RADAR_PREFETCH_MS>=stayMs;
}
// Share of the rotation the radar is visible; used to spread the daily budget.
static float radarVisibleShare() {
  if(enabledModeCount()<=1) return 1.0f;
  uint32_t total=0;
  for(uint8_t i=0;i<9;i++) if(modeEnabled(i)) total+=max((uint16_t)5,cfg.rotationSec[i]);
  uint32_t radar=max((uint16_t)5,cfg.rotationSec[MODE_PLANE])+RADAR_PREFETCH_MS/1000UL;
  return total?min(1.0f,(float)radar/(float)total):1.0f;
}

// ---- local config ------------------------------------------------------------
static const char* CFG_PATH = "/config.json";

static bool validHexColor(const String& s) {
  if (s.length() != 7 || s[0] != '#') return false;
  for (uint8_t i=1;i<7;i++) if (!isxdigit((unsigned char)s[i])) return false;
  return true;
}

static bool saveConfig() {
  JsonDocument d;
  d["ssid"] = cfg.ssid; d["pass"] = cfg.pass; d["hostname"] = cfg.hostname;
  d["timeZone"] = cfg.timeZone;
  d["mode"] = cfg.mode; d["rotationMask"] = cfg.rotationMask; d["brightness"] = cfg.brightness;
  JsonArray rs = d["rotationSec"].to<JsonArray>(); for(uint8_t i=0;i<9;i++) rs.add(cfg.rotationSec[i]);
  d["nightEnabled"] = cfg.nightEnabled; d["nightStart"] = cfg.nightStart; d["nightEnd"] = cfg.nightEnd; d["nightBrightness"] = cfg.nightBrightness;
  d["stopName"] = cfg.stopName; d["stopId"] = cfg.stopId; d["departureCount"] = cfg.departureCount; d["departurePoll"] = cfg.departurePoll; d["vvoMotMask"] = cfg.vvoMotMask; d["lang"] = cfg.lang; d["departureCompactDestinations"] = cfg.departureCompactDestinations; d["departureAbbreviate"] = cfg.departureAbbreviate; d["departureDropTown"] = cfg.departureDropTown;
  d["weatherCity"] = cfg.weatherCity; d["weatherDisplayName"] = cfg.weatherDisplayName; d["weatherLat"] = cfg.weatherLat; d["weatherLon"] = cfg.weatherLon; d["weatherPoll"] = cfg.weatherPoll; d["weatherTempColor"] = cfg.weatherTempColor; d["weatherFahrenheit"] = cfg.weatherFahrenheit; d["geoGermanNames"] = cfg.geoGermanNames;
  d["planePlace"] = cfg.planePlace; d["planeDisplayName"] = cfg.planeDisplayName; d["planeLat"] = cfg.planeLat; d["planeLon"] = cfg.planeLon; d["planeRadiusKm"] = cfg.planeRadiusKm; d["planePoll"] = cfg.planePoll; d["planeShowAge"] = cfg.planeShowAge; d["planeShowPlace"] = cfg.planeShowPlace; d["planePlaceToggleSec"] = cfg.planePlaceToggleSec;
  d["trackedFlightNo"] = cfg.trackedFlightNo; d["aviationstackKey"] = cfg.aviationstackKey; d["trackerStatusPollMin"] = cfg.trackerStatusPollMin; d["adsbLolKey"] = cfg.adsbLolKey;
  JsonArray cds = d["countdowns"].to<JsonArray>();
  for (uint8_t i=0;i<4;i++) {
    JsonObject c = cds.add<JsonObject>();
    c["title"] = cfg.countdown[i].title; c["titleColor"] = cfg.countdown[i].titleColor; c["countColor"] = cfg.countdown[i].countColor;
    c["type"] = cfg.countdown[i].type; c["targetDate"] = cfg.countdown[i].targetDate; c["durationSec"] = cfg.countdown[i].durationSec;
    c["targetEpoch"] = cfg.countdown[i].targetEpoch; c["active"] = cfg.countdown[i].active; c["dayDigits"] = cfg.countdown[i].dayDigits;
  }
  if(d.overflowed()) return false;
  static const char* TMP_PATH = "/config.json.tmp";
  File f = LittleFS.open(TMP_PATH, "w");
  if (!f) return false;
  const size_t expected = measureJson(d);
  const size_t written = serializeJson(d, f);
  f.flush();
  const bool complete = written == expected && !f.getWriteError() && f.size() == expected;
  f.close();
  if (!complete) { LittleFS.remove(TMP_PATH); return false; }
  if (!LittleFS.rename(TMP_PATH, CFG_PATH)) { LittleFS.remove(TMP_PATH); return false; }
  return true;
}

static void loadConfig() {
  if (!LittleFS.begin()) LittleFS.format(), LittleFS.begin();
  File f = LittleFS.open(CFG_PATH, "r"); if (!f) return;
  JsonDocument d; if (deserializeJson(d, f)) { f.close(); return; } f.close();
  cfg.ssid = d["ssid"] | cfg.ssid; cfg.pass = d["pass"] | cfg.pass; cfg.hostname = d["hostname"] | cfg.hostname;
  if(d["timeZone"].is<const char*>() && timeZoneIndex(d["timeZone"].as<const char*>())>=0)cfg.timeZone=d["timeZone"].as<String>();
  cfg.mode = constrain((int)(d["mode"] | cfg.mode), 0, 8);
  if(d["rotationMask"].is<uint16_t>() || d["rotationMask"].is<int>()) cfg.rotationMask = ((uint16_t)(d["rotationMask"] | cfg.rotationMask)) & 0x01FFU;
  else cfg.rotationMask = (1U << cfg.mode); // migrate v0.4 single-view configuration
  if(!cfg.rotationMask) cfg.rotationMask = (1U << cfg.mode);
  JsonArrayConst rs=d["rotationSec"].as<JsonArrayConst>(); if(!rs.isNull()) for(uint8_t i=0;i<9&&i<rs.size();i++) cfg.rotationSec[i]=constrain((int)(rs[i]|cfg.rotationSec[i]),5,3600);
  cfg.brightness = constrain((int)(d["brightness"] | cfg.brightness), 1, 100);
  cfg.nightEnabled = d["nightEnabled"] | cfg.nightEnabled; cfg.nightStart = constrain((int)(d["nightStart"] | cfg.nightStart), 0, 1439); cfg.nightEnd = constrain((int)(d["nightEnd"] | cfg.nightEnd), 0, 1439); cfg.nightBrightness = constrain((int)(d["nightBrightness"] | cfg.nightBrightness), 1, 100);
  cfg.stopName = d["stopName"] | cfg.stopName; cfg.stopId = d["stopId"] | cfg.stopId; {int dc=(int)(d["departureCount"] | cfg.departureCount); cfg.departureCount=(dc==10)?10:constrain(dc,3,5);} cfg.departurePoll = constrain((int)(d["departurePoll"] | cfg.departurePoll), 10, 300); cfg.vvoMotMask = ((uint8_t)(d["vvoMotMask"] | cfg.vvoMotMask)) & VVO_MOT_ALL; if(!cfg.vvoMotMask)cfg.vvoMotMask=VVO_MOT_ALL; cfg.lang = (uint8_t)(d["lang"] | cfg.lang) == LANG_EN ? LANG_EN : LANG_DE; cfg.departureCompactDestinations = d["departureCompactDestinations"] | cfg.departureCompactDestinations; cfg.departureAbbreviate = d["departureAbbreviate"] | cfg.departureAbbreviate; cfg.departureDropTown = d["departureDropTown"] | cfg.departureDropTown;
  cfg.weatherCity = d["weatherCity"] | cfg.weatherCity; cfg.weatherDisplayName = d["weatherDisplayName"] | cfg.weatherDisplayName; cfg.weatherLat = d["weatherLat"] | cfg.weatherLat; cfg.weatherLon = d["weatherLon"] | cfg.weatherLon; cfg.weatherPoll = constrain((int)(d["weatherPoll"] | cfg.weatherPoll), 60, 3600); cfg.weatherTempColor = d["weatherTempColor"] | cfg.weatherTempColor; cfg.weatherFahrenheit = d["weatherFahrenheit"] | cfg.weatherFahrenheit; cfg.geoGermanNames = d["geoGermanNames"] | cfg.geoGermanNames;
  cfg.planePlace = d["planePlace"] | cfg.planePlace; cfg.planeDisplayName = d["planeDisplayName"] | cfg.planeDisplayName; cfg.planeLat = d["planeLat"] | cfg.planeLat; cfg.planeLon = d["planeLon"] | cfg.planeLon; cfg.planeRadiusKm = constrain((int)(d["planeRadiusKm"] | cfg.planeRadiusKm), 1, 250); cfg.planePoll = constrain((int)(d["planePoll"] | cfg.planePoll), 3, 300); cfg.planeShowAge = d["planeShowAge"] | cfg.planeShowAge; cfg.planeShowPlace = d["planeShowPlace"] | cfg.planeShowPlace; cfg.planePlaceToggleSec = (d["planePlaceToggleSec"] | cfg.planePlaceToggleSec)==0 ? 0 : constrain((int)(d["planePlaceToggleSec"] | cfg.planePlaceToggleSec), 5, 30);
  cfg.trackedFlightNo = d["trackedFlightNo"] | cfg.trackedFlightNo; cfg.aviationstackKey = d["aviationstackKey"] | cfg.aviationstackKey; cfg.adsbLolKey = d["adsbLolKey"] | cfg.adsbLolKey; cfg.trackerStatusPollMin = constrain((int)(d["trackerStatusPollMin"] | cfg.trackerStatusPollMin), 5, 120);
  JsonArrayConst cds = d["countdowns"].as<JsonArrayConst>();
  if (!cds.isNull()) for (uint8_t i=0;i<4 && i<cds.size();i++) {
    JsonObjectConst c=cds[i].as<JsonObjectConst>(); CountdownSlot& x=cfg.countdown[i];
    x.title = c["title"] | x.title; String tc=c["titleColor"] | x.titleColor; if(validHexColor(tc))x.titleColor=tc; String cc=c["countColor"] | x.countColor; if(validHexColor(cc))x.countColor=cc;
    x.type=constrain((int)(c["type"]|x.type),0,2); x.targetDate=c["targetDate"]|x.targetDate; x.durationSec=c["durationSec"]|x.durationSec; x.targetEpoch=c["targetEpoch"]|x.targetEpoch; x.active=c["active"]|x.active; x.dayDigits=constrain((int)(c["dayDigits"]|x.dayDigits),3,6);
  }
  if(!modeEnabled(cfg.mode)) cfg.mode=firstEnabledMode();
}

// ---- display views -----------------------------------------------------------
static void drawFooterClockOnly() {
  String tm=timeHHMM();visibleText(31,tm,232-textW(tm,1),226,1,C_GRAY,196,224,36,12);
}

static void drawFooter(const String& label, bool withClock=true) {
  if(viewFresh)gfx->drawFastHLine(8,218,224,C_DGRAY);
  visibleText(30,label,8,226,1,C_GRAY,8,223,186,14);
  if(withClock)drawFooterClockOnly();
}



static void drawFooterPageArrow(bool down) {
  const int cx=120, cy=230;
  uint16_t c=C_GRAY;
  if(down){gfx->drawLine(cx,cy-4,cx,cy+3,c);gfx->drawLine(cx,cy+3,cx-4,cy-1,c);gfx->drawLine(cx,cy+3,cx+4,cy-1,c);}
  else{gfx->drawLine(cx,cy+4,cx,cy-3,c);gfx->drawLine(cx,cy-3,cx-4,cy+1,c);gfx->drawLine(cx,cy-3,cx+4,cy+1,c);}
}

static String displayAscii(String s) {
  // Name kept for compatibility with the v0.4 rendering code. UTF-8 European
  // glyphs are rendered natively by printDisplayTextAt()/centerText().
  // Town names are no longer stripped here; see departureText().
  return s;
}

// Town of the configured stop: "Dresden, Postplatz" / "Dresden Postplatz"
// -> "Dresden". A one-word stop name ("Postplatz") falls back to Dresden,
// the VVO home town, until other providers deliver their own town.
static String stopTown() {
  String n=cfg.stopName;int comma=n.indexOf(',');
  if(comma>0)return n.substring(0,comma);
  int space=n.indexOf(' ');
  return space>0?n.substring(0,space):String("Dresden");
}
static String departureText(const char* raw,bool abbreviate) {
  char out[96];String town=stopTown();
  formatDestination(raw,town.c_str(),cfg.departureDropTown,abbreviate,out,sizeof(out));
  return String(out);
}


static uint16_t color565(const String& hex) {
  if (!validHexColor(hex)) return C_WHITE;
  uint32_t rgb = strtoul(hex.substring(1).c_str(), nullptr, 16);
  uint8_t r=(rgb>>16)&0xFF, g=(rgb>>8)&0xFF, b=rgb&0xFF;
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static int64_t daysFromCivilDate(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

static bool parseDateOnly(const String& iso, int& y, int& m, int& d) {
  if (iso.length() != 10 || iso[4] != '-' || iso[7] != '-') return false;
  y=iso.substring(0,4).toInt(); m=iso.substring(5,7).toInt(); d=iso.substring(8,10).toInt();
  return y>=1970 && m>=1 && m<=12 && d>=1 && d<=31;
}

static bool countdownDayDifference(const String& target, int32_t& outDays) {
  time_t now=time(nullptr); if(now<1609459200) return false;
  struct tm t; localtime_r(&now,&t);
  int y,m,d; if(!parseDateOnly(target,y,m,d)) return false;
  int64_t today=daysFromCivilDate(t.tm_year+1900,(unsigned)(t.tm_mon+1),(unsigned)t.tm_mday);
  int64_t goal=daysFromCivilDate(y,(unsigned)m,(unsigned)d);
  int64_t diff=goal-today;
  if(diff>2147483647LL || diff<-2147483647LL) return false;
  outDays=(int32_t)diff; return true;
}

static uint8_t decimalDigits(uint32_t v) {
  uint8_t n=1; while(v>=10){v/=10;n++;} return n;
}

static String padUnsigned(uint32_t v, uint8_t digits) {
  String s=String(v); while(s.length()<digits)s="0"+s; return s;
}

static uint8_t segMask(uint8_t d) {
  static const uint8_t masks[10]={
    0b00111111, // 0: a b c d e f
    0b00000110, // 1: b c
    0b01011011, // 2: a b d e g
    0b01001111, // 3: a b c d g
    0b01100110, // 4: b c f g
    0b01101101, // 5: a c d f g
    0b01111101, // 6: a c d e f g
    0b00000111, // 7: a b c
    0b01111111, // 8
    0b01101111  // 9
  };
  return d<10?masks[d]:0;
}

static void drawSevenDigit(int x,int y,int w,int h,int t,uint8_t d,uint16_t color) {
  uint8_t m=segMask(d); int half=h/2;
  if(m&0x01)gfx->fillRect(x+t,y,w-2*t,t,color);
  if(m&0x02)gfx->fillRect(x+w-t,y+t,t,half-t,color);
  if(m&0x04)gfx->fillRect(x+w-t,y+half,t,half-t,color);
  if(m&0x08)gfx->fillRect(x+t,y+h-t,w-2*t,t,color);
  if(m&0x10)gfx->fillRect(x,y+half,t,half-t,color);
  if(m&0x20)gfx->fillRect(x,y+t,t,half-t,color);
  if(m&0x40)gfx->fillRect(x+t,y+half-t/2,w-2*t,t,color);
}

static int sevenUnit(char c) {
  if(c>='0'&&c<='9')return 100;
  if(c==':')return 34;
  if(c=='+'||c=='-'||c==' ')return 62;
  return 45;
}

static int sevenCharWidth(char c,int digitW) {
  return max(4,(digitW*sevenUnit(c))/100);
}

static void drawSevenString(const String& value,int centerY,uint16_t color,bool partial=false,const String& old="") {
  const int gap=3;
  int units=0; for(size_t i=0;i<value.length();i++)units+=sevenUnit(value[i]);
  int gaps=value.length()>0?(int)(value.length()-1)*gap:0;
  int digitW=(224-gaps)*100/max(100,units);
  digitW=constrain(digitW,18,42);
  int h=(digitW*19)/10;
  int total=gaps; for(size_t i=0;i<value.length();i++)total+=sevenCharWidth(value[i],digitW);
  int x=(TFT_W-total)/2, y=centerY-h/2, t=max(3,digitW/6);
  for(size_t i=0;i<value.length();i++){
    char c=value[i]; int w=sevenCharWidth(c,digitW);
    if(partial&&i<old.length()&&old[i]==c){x+=w+gap;continue;}
    if(partial)gfx->fillRect(x,y,w,h,C_BLACK);
    if(c>='0'&&c<='9') drawSevenDigit(x,y,digitW,h,t,(uint8_t)(c-'0'),color);
    else if(c==':') { int s=max(3,t-1); gfx->fillRect(x+(w-s)/2,y+h/3-s/2,s,s,color); gfx->fillRect(x+(w-s)/2,y+(2*h)/3-s/2,s,s,color); }
    else if(c=='-'||c=='+') { int cy=y+h/2; gfx->fillRect(x,cy-t/2,w,t,color); if(c=='+')gfx->fillRect(x+w/2-t/2,cy-w/2,t,w,color); }
    // Space deliberately draws nothing but keeps the sign column width stable.
    x+=w+gap;
  }
}

static int8_t countdownSlotFromMode(uint8_t mode) {
  if(mode<MODE_COUNTDOWN1||mode>MODE_COUNTDOWN4)return -1;
  return (int8_t)(mode-MODE_COUNTDOWN1);
}

static String countdownValue(const CountdownSlot& c) {
  if(!c.active)return "---";
  if(c.type==COUNTDOWN_DAYS){
    int32_t days=0;if(!countdownDayDifference(c.targetDate,days))return "---";
    uint32_t a=days<0?(uint32_t)(-(int64_t)days):(uint32_t)days;
    String sign=days>0?"-":(days<0?"+":" ");
    return sign+padUnsigned(a,c.dayDigits);
  }
  time_t now=time(nullptr);if(now<1609459200||!c.targetEpoch)return "---";
  int64_t diff=(int64_t)c.targetEpoch-(int64_t)now;
  uint64_t a=diff<0?(uint64_t)(-diff):(uint64_t)diff;
  String sign=diff>0?"-":(diff<0?"+":" ");
  if(c.type==COUNTDOWN_MS){
    uint32_t mins=(uint32_t)(a/60ULL), sec=(uint32_t)(a%60ULL);
    return sign+padUnsigned(mins,2)+":"+padUnsigned(sec,2);
  }
  uint32_t hours=(uint32_t)(a/3600ULL), mins=(uint32_t)((a%3600ULL)/60ULL), sec=(uint32_t)(a%60ULL);
  return sign+padUnsigned(hours,2)+":"+padUnsigned(mins,2)+":"+padUnsigned(sec,2);
}

static String flightStatusGerman(const char* s) {
  if(!s||!*s)return "N/A";
  if(!strcmp(s,"active"))return TR("IN DER LUFT","IN THE AIR");
  if(!strcmp(s,"landed"))return TR("GELANDET","LANDED");
  if(!strcmp(s,"scheduled"))return TR("GEPLANT","SCHEDULED");
  if(!strcmp(s,"cancelled"))return TR("ANNULLIERT","CANCELLED");
  if(!strcmp(s,"diverted"))return TR("UMGELEITET","DIVERTED");
  if(!strcmp(s,"incident"))return TR("STÖRUNG","INCIDENT");
  return String(s);
}

static String trackedFlightDuration(const TrackedFlightSnapshot& f) {
  int64_t start=f.depActualEpoch?f.depActualEpoch:f.depScheduledEpoch;
  if(!start)return "N/A";
  int64_t end=0;
  if(f.arrActualEpoch)end=f.arrActualEpoch;
  else if(f.depActualEpoch){time_t now=time(nullptr);if(now>1609459200)end=now;}
  else end=f.arrScheduledEpoch;
  if(!end||end<start)return "N/A";
  uint64_t sec=(uint64_t)(end-start);uint32_t h=sec/3600ULL,m=(sec%3600ULL)/60ULL;
  return padUnsigned(h,2)+":"+padUnsigned(m,2);
}

static void drawDepartures(bool force=true) {
  static uint8_t lastVisible=255,lastPage=255;static bool lastError=false,lastUnserved=false;
  uint8_t count=min(vvoCount(),cfg.departureCount);
  bool pagedNow=cfg.departureCount==10&&count>5;
  uint8_t startNow=pagedNow&&departurePage?5:0;
  uint8_t shown=count>startNow?min((uint8_t)5,(uint8_t)(count-startNow)):0;
  force=force||shown!=lastVisible||(pagedNow&&lastPage!=departurePage)||(!shown&&(lastError!=vvoError()||lastUnserved!=vvoNotServed()));
  beginView(force);lastVisible=shown;lastPage=departurePage;lastError=vvoError();lastUnserved=vvoNotServed();
  String title = crop(cfg.stopName.length() ? departureText(cfg.stopName.c_str(),false) : String(TR("HALTESTELLE","STOP")), 18);
  visibleText(0,title,8,7,2,C_YELLOW,0,0,240,29);
  if(force)gfx->drawFastHLine(8,29,224,C_DGRAY);

  if (!cfg.stopId.length()) {
    if(force)centerText(TR("HALTESTELLE","STOP"), 78, 2, C_WHITE);
    if(force)centerText(TR("IM WEBINTERFACE","IN THE WEB"), 108, 2, C_GRAY);
    if(force)centerText(TR("AUSWÄHLEN","INTERFACE"), 136, 2, C_GRAY);
    drawFooter(TR("VVO NICHT KONFIGURIERT","VVO NOT CONFIGURED"));
    return;
  }

  uint8_t n = vvoCount();
  if (!n) {
    if (vvoNotServed()) {
      if(force){centerText(TR("Haltestelle wird","Stop is"),106,2,C_GRAY);centerText(TR("nicht bedient","not served"),128,2,C_GRAY);}
      drawFooter("VVO");
    } else if (vvoError()) {
      if(force)centerText("VVO", 82, 4, C_RED);
      if(force)centerText(TR("nicht erreichbar","not reachable"), 128, 2, C_GRAY);
      drawFooter(TR("VVO FEHLER","VVO ERROR"));
    } else {
      if(force)centerText("VVO", 82, 4, C_YELLOW);
      if(force)centerText(TR("lade Abfahrten...","loading departures..."), 128, 2, C_GRAY);
      drawFooter("LIVE");
    }
    return;
  }

  if (n > cfg.departureCount) n = cfg.departureCount;
  bool paged = cfg.departureCount == 10 && n > 5;
  uint8_t start = paged && departurePage ? 5 : 0;
  uint8_t visible = n > start ? min((uint8_t)5, (uint8_t)(n - start)) : 0;
  int y0 = 41, step = paged ? 34 : (visible >= 5 ? 34 : (visible == 4 ? 42 : 52));
  for (uint8_t row = 0; row < visible; row++) {
    const VvoDeparture& d = vvoAt(start + row);
    int y = y0 + row * step;
    String line = displayAscii(String(d.line));
    String rawDest = departureText(d.direction,cfg.departureAbbreviate);
    String right;
    uint16_t rightColor = C_WHITE;
    if (d.cancelled) { right = "X"; rightColor = C_RED; }
    else if (d.minutes < 0) { right = "?"; rightColor = C_GRAY; }
    else if (d.minutes <= 0) { right = TR("JETZT","NOW"); rightColor = C_GREEN; }
    else { right = String(d.minutes); rightColor = d.delayed ? C_YELLOW : C_WHITE; }

    uint16_t lc = glyphCount(line);
    uint8_t lineSize = lc <= 2 ? 3 : (lc <= 4 ? 2 : 1);
    int lineY = y + (lineSize == 3 ? 0 : (lineSize == 2 ? 6 : 12));
    visibleText(1+row*3,line,8,lineY,lineSize,C_YELLOW,8,y,50,step);

    uint8_t rs = (d.minutes == 0 && !d.cancelled) ? 1 : 2;
    int rightX = 232 - textW(right, rs);
    uint8_t destSize = 2;
    String dest = crop(rawDest, 9);
    if (cfg.departureCompactDestinations && glyphCount(rawDest) > 9) {
      destSize = 1;
      int maxChars = max(2, (rightX - 70) / 6);
      dest = crop(rawDest, (uint8_t)min(maxChars, 31));
    }
    bool destChanged=visibleText(2+row*3,dest,62,y+(destSize==2?6:12),destSize,C_WHITE,62,y,rightX-64,step);
    visibleText(3+row*3,right,rightX,y+(rs==1?10:6),rs,rightColor,rightX,y,232-rightX,step,destChanged);
  }
  drawFooter(vvoError() ? TR("LIVE / LETZTER STAND","LIVE / LAST DATA") : "VVO LIVE");
  if(paged&&force)drawFooterPageArrow(!departurePage);
}

static void drawCloud(int cx, int cy, uint16_t color) {
  gfx->fillCircle(cx-18, cy+4, 11, color);
  gfx->fillCircle(cx, cy-5, 17, color);
  gfx->fillCircle(cx+20, cy+4, 12, color);
  gfx->fillRect(cx-27, cy+4, 56, 15, color);
}

static void drawSun(int cx, int cy, uint16_t color, int reach=30) {
  gfx->fillCircle(cx, cy, reach/2, color);
  for (int a=0; a<360; a+=45) {
    float r=a*(float)PI/180.0f;
    int x1=cx+(int)((reach*0.73f)*cosf(r)), y1=cy+(int)((reach*0.73f)*sinf(r));
    int x2=cx+(int)(reach*cosf(r)), y2=cy+(int)(reach*sinf(r));
    gfx->drawLine(x1,y1,x2,y2,color);
  }
}

static void drawMoon(int cx,int cy,int radius=18) {
  gfx->fillCircle(cx,cy,radius,C_WHITE);gfx->fillCircle(cx+radius/2,cy-radius*2/5,radius,C_BLACK);
}

static void drawSnowflake(int x,int y,int radius,uint16_t color) {
  gfx->drawLine(x-radius,y,x+radius,y,color);
  gfx->drawLine(x,y-radius,x,y+radius,color);
  gfx->drawLine(x-radius+1,y-radius+1,x+radius-1,y+radius-1,color);
  gfx->drawLine(x-radius+1,y+radius-1,x+radius-1,y-radius+1,color);
}

static void drawWeatherIcon(int code, bool day) {
  WeatherVisual w=weatherVisual(code);
  if(w.motif==WX_CLEAR){if(day)drawSun(120,55,C_YELLOW);else drawMoon(120,55);return;}
  if(w.motif==WX_PARTLY){if(day)drawSun(103,47,C_YELLOW,20);else drawMoon(103,47);drawCloud(125,58,C_WHITE);return;}
  if(w.motif==WX_FOG){
    for(int row=0;row<4;row++)gfx->fillRect(row%2?95:87,37+row*12,row%2?60:76,4,C_GRAY);
    if(code==48)drawSnowflake(164,71,4,C_CYAN);
    return;
  }
  drawCloud(120,w.motif==WX_CLOUD?55:44,C_WHITE);
  if(!w.intensity)return;
  int count=w.intensity==1?2:(w.intensity==2?4:6);
  bool drizzle=w.motif==WX_DRIZZLE||w.motif==WX_FREEZING_DRIZZLE;
  bool snow=w.motif==WX_SNOW||w.motif==WX_SNOW_SHOWER;
  bool grains=w.motif==WX_GRAINS||w.motif==WX_THUNDER_HAIL;
  for(int i=0;i<count;i++){
    int x=91+i*58/(count-1),y=72+(i%2)*4;
    if(snow)drawSnowflake(x,y+4,3,C_CYAN);
    else if(grains)gfx->fillCircle(x,y+4,w.motif==WX_GRAINS?1:2,C_CYAN);
    else gfx->drawLine(x,y,x-(drizzle?2:5),y+(drizzle?4:10),C_CYAN);
  }
  if(w.motif==WX_FREEZING_DRIZZLE||w.motif==WX_FREEZING_RAIN)drawSnowflake(166,78,5,C_CYAN);
  if(w.motif==WX_RAIN_SHOWER||w.motif==WX_SNOW_SHOWER){if(day)drawSun(91,40,C_YELLOW,16);else drawMoon(91,40,12);drawCloud(120,44,C_WHITE);}
  if(w.motif==WX_THUNDER||w.motif==WX_THUNDER_HAIL){gfx->drawLine(124,59,113,78,C_YELLOW);gfx->drawLine(113,78,125,75,C_YELLOW);gfx->drawLine(125,75,116,86,C_YELLOW);}
}

static uint16_t weatherTempColor565(float c);
static void drawSunEventIcon(int x,int y,bool rise);
static float displayTempValue(float c);
static String weatherUnit();

static void drawWeather(bool force=true) {
  const WeatherSnapshot& w=weatherData();static bool valid=false,error=false;
  force=force||valid!=w.valid||(!w.valid&&error!=w.error);beginView(force);valid=w.valid;error=w.error;
  String title=cfg.weatherDisplayName.length()?cfg.weatherDisplayName:(cfg.weatherCity.length()?cfg.weatherCity:String(TR("WETTER","WEATHER")));
  visibleText(0,crop(displayAscii(title),18),8,7,2,C_YELLOW,0,0,240,22);
  if(!w.valid){if(force){centerText(w.error?TR("WETTER FEHLER","WEATHER ERROR"):TR("WETTER LÄDT","WEATHER LOADING"),88,2,w.error?C_RED:C_YELLOW);centerText(w.error?TR("Open-Meteo nicht erreichbar","Open-Meteo not reachable"):TR("Daten werden geladen...","Loading data..."),124,1,C_GRAY);}drawFooter("OPEN-METEO");return;}
  String icon=String(w.code)+(w.isDay?"D":"N");
  if(force||visibleFields[1].text!=icon){if(!force)gfx->fillRect(8,22,224,66,C_BLACK);drawWeatherIcon(w.code,w.isDay);visibleFields[1].text=icon;}
  String temp=String((int)lroundf(displayTempValue(w.currentC)));
  uint16_t color=cfg.weatherTempColor?weatherTempColor565(w.currentC):C_WHITE;
  bool tempChanged=visibleCenter(2,temp,88,6,color,88,48);
  visibleText(3,weatherUnit(),176,103,2,cfg.weatherTempColor?color:C_GRAY,176,103,36,16,tempChanged);
  String label=cfg.lang==LANG_EN?String(FPSTR(weatherLabelEn(w.code))):String(weatherLabel(w.code));visibleCenter(4,label,142,textW(label,2)<=224?2:1,C_WHITE,140,19);
  String hi="H "+String((int)lroundf(displayTempValue(w.highC)))+weatherUnit();
  String lo=TR("T ","L ")+String((int)lroundf(displayTempValue(w.lowC)))+weatherUnit();
  visibleText(5,hi,18,169,2,C_GRAY,8,166,112,24);
  visibleText(6,lo,222-textW(lo,2),169,2,C_GRAY,120,166,112,24);
  String rain=w.rainPct>=0?String(TR("REGEN ","RAIN "))+String(w.rainPct)+"%":String(TR("REGEN --","RAIN --"));
  visibleText(7,rain,18,197,1,C_CYAN,8,194,106,16);
  if(force){drawSunEventIcon(118,196,true);drawSunEventIcon(174,196,false);}
  visibleText(8,String(w.sunrise),133,197,1,C_GRAY,133,194,32,16);
  visibleText(9,String(w.sunset),189,197,1,C_GRAY,189,194,36,16);
  drawFooter(w.error?TR("LIVE / LETZTER STAND","LIVE / LAST DATA"):"OPEN-METEO LIVE");
}

static uint16_t weatherTempColor565(float c) {
  if(c <= -10.0f) return 0x001F; // deep blue
  if(c <=   0.0f) return C_BLUE;
  if(c <=  10.0f) return C_CYAN;
  if(c <=  17.0f) return C_WHITE;
  if(c <=  23.0f) return C_GREEN;
  if(c <=  28.0f) return C_YELLOW;
  if(c <=  34.0f) return 0xFD20; // orange
  return C_RED;
}

static void drawSunEventIcon(int x,int y,bool rise) {
  // 13x9 tiny pixel-style sunrise/sunset mark. No emoji/font dependency.
  uint16_t c=C_GRAY;
  gfx->drawFastHLine(x,y+7,13,c);
  gfx->drawPixel(x+6,y+1,c);
  gfx->drawPixel(x+3,y+3,c); gfx->drawPixel(x+9,y+3,c);
  gfx->drawLine(x+4,y+6,x+4,y+5,c); gfx->drawLine(x+8,y+6,x+8,y+5,c);
  gfx->drawFastHLine(x+5,y+4,3,c);
  gfx->drawLine(x+6,y+8,x+6,y+8,c);
  if(rise){gfx->drawLine(x+6,y+8,x+4,y+6,c);gfx->drawLine(x+6,y+8,x+8,y+6,c);}
  else{gfx->drawLine(x+6,y+6,x+4,y+8,c);gfx->drawLine(x+6,y+6,x+8,y+8,c);}
}

static float displayTempValue(float c) { return cfg.weatherFahrenheit ? (c * 9.0f / 5.0f + 32.0f) : c; }
static String weatherUnit() { return cfg.weatherFahrenheit ? String("°F") : String("°C"); }



static String planePlaceTitle() {
  String s=cfg.planeDisplayName.length()?cfg.planeDisplayName:cfg.planePlace;
  return crop(displayAscii(s),18);
}

static void drawPlaneHeader() {
  gfx->fillRect(0,0,TFT_W,30,C_BLACK);
  String title=(cfg.planeShowPlace&&(cfg.planePlaceToggleSec==0||planePlacePhase)&&planePlaceTitle().length())?planePlaceTitle():String(TR("IN DER NÄHE","NEARBY"));
  printDisplayTextAt(8,7,title,2,C_YELLOW);
  gfx->drawFastHLine(8,29,224,C_DGRAY);
}

static String routePart(const FlightRoute& route, uint8_t begin, uint8_t end) {
  String out;
  for(uint8_t i=begin;i<end;i++){if(i>begin)out+=" > ";out+=route.airports[i];}
  if(end==route.count && route.truncated)out+=" > ...";
  return out;
}

static void drawFlightRoute(const FlightRoute& route, const char* origin, const char* destination, int y) {
  if(route.count<=2){
    centerText(String(origin[0]?origin:"N/A")+" > "+String(destination[0]?destination:"N/A"),y,3,C_YELLOW);
    return;
  }
  String text=routePart(route,0,route.count);
  if(textW(text,2)<=224)centerText(text,y,2,C_YELLOW);
  else if(textW(text,1)<=224)centerText(text,y,1,C_YELLOW);
  else { // Long chains fit in the existing route area as two compact rows.
    uint8_t split=route.count/2;
    centerText(routePart(route,0,split)+" >",y,1,C_YELLOW);
    centerText(routePart(route,split,route.count),y+10,1,C_YELLOW);
  }
}

static FlightSnapshot lastPlaneRendered = {};
static bool planeRendered = false;

static bool samePlane(const FlightSnapshot& a,const FlightSnapshot& b) {
  if(a.hex[0]&&b.hex[0])return !strcmp(a.hex,b.hex);
  if(a.registration[0]&&b.registration[0])return !strcmp(a.registration,b.registration);
  return a.callsign[0]&&b.callsign[0]&&!strcmp(a.callsign,b.callsign);
}

static String planeDistance(const FlightSnapshot& f) { return String(f.distanceKm,1)+" km"; }
static String planeAltitude(const FlightSnapshot& f) { return f.onGround?String("GND"):(f.altitudeKnown?String((int)lroundf(f.altitudeFt*0.3048f))+" m":String("N/A")); }

static String compactType(const FlightSnapshot& f) {
  const char* stable=aircraftDisplayLabel(f.typeCode);
  if(stable&&textW(stable,2)<=224)return String(stable);
  String full=f.typeName[0]?String(f.typeName):String(f.typeCode);
  if(!full.length())return "N/A";
  if(textW(full,2)<=224)return full;
  String maker;
  if(full.indexOf("Cessna")>=0)maker="Cessna";
  else if(full.indexOf("Airbus")>=0)maker="Airbus";
  else if(full.indexOf("Boeing")>=0)maker="Boeing";
  else if(full.indexOf("Embraer")>=0)maker="Embraer";
  if(maker.length()){
    int start=full.indexOf(maker)+maker.length();while(start<(int)full.length()&&full[start]==' ')start++;
    int end=full.indexOf(' ',start);if(end<0)end=full.length();String model=full.substring(start,end);
    bool digit=false;for(size_t i=0;i<model.length();i++)if(model[i]>='0'&&model[i]<='9')digit=true;
    String shortName=maker+" "+model;if(digit&&textW(shortName,2)<=224)return shortName;
  }
  return f.typeCode[0]?String(f.typeCode):crop(full,18);
}
// Reception age follows the current status. Only this number/unit field
// changes each second; the status label and clock keep their own areas.
static bool lastPlanePauseRendered=false,lastPlaneAgeRendered=true;
static String planeFooterLabel() {
  return flightProviderPaused()?TR("ABRUFPAUSE","PAUSED"):(flightData().error?TR("LETZTER STAND","LAST DATA"):"ADSB.LOL LIVE");
}
static void drawPlaneAge() {
  if(!cfg.planeShowAge||!flightData().valid)return;
  const int x=8+textW(String(planeFooterLabel())+" ",1);
  String seconds=String((uint32_t)(millis()-flightLastOkMs())/1000UL)+"s";
  VisibleField& field=visibleFields[28];
  if(field.valid&&field.text==seconds&&field.tx==x)return;
  gfx->fillRect(x,223,48,14,C_BLACK);
  printDisplayTextAt(x,226,seconds,1,C_GRAY);
  field.text=seconds;field.tx=x;field.valid=true;
}
static void drawPlaneFooter() {
  drawFooter(planeFooterLabel());
  drawPlaneAge();
}
static void drawPlane(bool force=true) {
  const FlightSnapshot& f=flightData();
  const bool paused=f.valid&&flightProviderPaused();
  bool full=force||!planeRendered||f.valid!=lastPlaneRendered.valid||
    (f.valid&&!samePlane(f,lastPlaneRendered))||(!f.valid&&f.error!=lastPlaneRendered.error);
  beginView(full);if(full)drawPlaneHeader();
  if(!f.valid){
    if(full){
      if(f.error&&adsbLolAccessDenied(flightLastHttp())){centerText("ADSB.LOL",80,3,C_RED);centerText(TR("Zugriff abgelehnt","Access denied"),118,2,C_WHITE);centerText(String("HTTP ")+String(flightLastHttp())+(flightLastHttp()==426?"":TR(" - Schlüssel prüfen"," - check key")),146,1,C_GRAY);centerText(TR("längere Pause vor neuem Versuch","pausing before next try"),162,1,C_GRAY);}
      else if(f.error){centerText(TR("FLUGRADAR","FLIGHT RADAR"),80,3,C_RED);centerText(TR("keine aktuellen Daten","no current data"),125,1,C_GRAY);}
      else{centerText(TR("KEIN FLUGZEUG","NO AIRCRAFT"),80,2,C_WHITE);centerText(TR("im gewählten Radius","in selected radius"),116,1,C_GRAY);}
      drawFooter(f.error?TR("ABRUF FEHLGESCHLAGEN","REQUEST FAILED"):"ADSB.LOL");
    }
    lastPlaneRendered=f;planeRendered=true;return;
  }
  if(full||strcmp(f.callsign,lastPlaneRendered.callsign)){
    if(!full)gfx->fillRect(8,35,224,40,C_BLACK);
    String cs=f.callsign[0]?String(f.callsign):String("N/A");centerText(cs,42,cs.length()<=6?4:3,C_WHITE);
  }
  if(full||strcmp(f.airline,lastPlaneRendered.airline)){
    if(!full)gfx->fillRect(8,78,224,20,C_BLACK);
    String airline=f.airline[0]?String(f.airline):String("N/A");centerText(crop(airline,36),80,textW(airline,2)<=224?2:1,C_GRAY);
  }
  if(full||strcmp(f.origin,lastPlaneRendered.origin)||strcmp(f.destination,lastPlaneRendered.destination)||
    f.route.count!=lastPlaneRendered.route.count||f.route.truncated!=lastPlaneRendered.route.truncated||
    memcmp(f.route.airports,lastPlaneRendered.route.airports,sizeof(f.route.airports))){
    if(!full)gfx->fillRect(8,102,224,29,C_BLACK);
    drawFlightRoute(f.route,f.origin,f.destination,105);
  }
  if(full||strcmp(f.typeName,lastPlaneRendered.typeName)||strcmp(f.typeCode,lastPlaneRendered.typeCode)){
    if(!full)gfx->fillRect(8,136,224,23,C_BLACK);
    centerText(compactType(f),139,2,C_WHITE);
  }
  if(full||strcmp(f.registration,lastPlaneRendered.registration)){
    if(!full)gfx->fillRect(8,161,224,24,C_BLACK);
    centerText(f.registration[0]?String(f.registration):String("N/A"),164,2,C_GRAY);
  }
  String dist=planeDistance(f),alt=planeAltitude(f);
  if(full||dist!=planeDistance(lastPlaneRendered)){
    if(!full)gfx->fillRect(8,190,112,24,C_BLACK);
    gfx->setTextSize(2);gfx->setTextColor(C_CYAN);gfx->setCursor(10,194);gfx->print(dist);
  }
  if(full||alt!=planeAltitude(lastPlaneRendered)){
    if(!full)gfx->fillRect(120,190,112,24,C_BLACK);
    gfx->setTextSize(2);gfx->setTextColor(C_CYAN);gfx->setCursor(230-textW(alt,2),194);gfx->print(alt);
  }
  if(full||f.error!=lastPlaneRendered.error||paused!=lastPlanePauseRendered||cfg.planeShowAge!=lastPlaneAgeRendered){
    if(!full)gfx->fillRect(8,223,186,14,C_BLACK);
    visibleFields[28].valid=false;visibleFields[29].valid=false;visibleFields[30].valid=false;
    drawPlaneFooter();
  }
  drawPlaneAge();
  lastPlanePauseRendered=paused;lastPlaneAgeRendered=cfg.planeShowAge;
  lastPlaneRendered=f;planeRendered=true;
}

static void drawTrackedFlight(bool force=true) {
  const TrackedFlightSnapshot& snapshot=trackedFlightData();
  static uint8_t lastState=255;static String oldRoute;
  uint8_t state=!cfg.trackedFlightNo.length()?0:(!cfg.aviationstackKey.length()?1:(!snapshot.valid?(snapshot.error?2:(snapshot.notFound?3:4)):5));
  String routeText=String(snapshot.origin)+">"+snapshot.destination+routePart(snapshot.route,0,snapshot.route.count);
  force=force||state!=lastState||routeText!=oldRoute;beginView(force);lastState=state;oldRoute=routeText;
  if(state!=5&&!force)return;
  if(!cfg.trackedFlightNo.length()){
    centerText("N/A",72,5,C_WHITE);centerText("N/A > N/A",126,3,C_YELLOW);centerText(TR("Flugnummer im","Enter flight number"),172,1,C_GRAY);centerText(TR("Webinterface eingeben","in the web interface"),184,1,C_GRAY);drawFooter("");return;
  }
  if(!cfg.aviationstackKey.length()){
    centerText(displayAscii(cfg.trackedFlightNo),62,4,C_WHITE);centerText("N/A > N/A",116,3,C_YELLOW);centerText(TR("API-KEY FEHLT","API KEY MISSING"),166,2,C_RED);drawFooter("");return;
  }
  const TrackedFlightSnapshot& f=trackedFlightData();
  if(!f.valid){
    centerText(displayAscii(cfg.trackedFlightNo),56,4,C_WHITE);
    centerText("N/A > N/A",108,3,C_YELLOW);
    if(f.error)centerText(TR("TRACKER FEHLER","TRACKER ERROR"),158,2,C_RED);
    else if(f.notFound)centerText(TR("FLUG N/A","FLIGHT N/A"),158,2,C_GRAY);
    else centerText(TR("lade Flugdaten...","loading flight data..."),158,2,C_GRAY);
    drawFooter(f.error?TR("TRACKER FEHLER","TRACKER ERROR"):(f.notFound?TR("FLUG NICHT GEFUNDEN","FLIGHT NOT FOUND"):TR("LADE FLUGDATEN","LOADING FLIGHT DATA")));
    return;
  }
  String no=f.flightIata[0]?String(f.flightIata):cfg.trackedFlightNo;
  visibleCenter(0,no,5,no.length()<=7?3:2,C_WHITE,0,29);
  if(force)drawFlightRoute(f.route,f.origin,f.destination,36);
  visibleCenter(1,flightStatusGerman(f.status),64,1,C_GRAY,62,12);

  auto row=[](uint8_t id,const String& label,const char* value,int y,uint16_t col){
    String v=(value&&*value)?String(value):String("N/A");
    visibleText(id+10,label,8,y+5,1,C_GRAY,8,y,88,20);
    visibleText(id,v,232-textW(v,2),y,2,col,100,y,132,20);
  };
  row(2,TR("AB PLAN","DEP SCHED"),f.depScheduled,82,C_WHITE);
  row(3,TR("AB IST","DEP ACTUAL"),f.depActual,105,C_WHITE);
  row(4,TR("AN PLAN","ARR SCHED"),f.arrScheduled,128,C_WHITE);
  if(strcmp(f.arrActual,"N/A"))row(5,TR("AN IST","ARR ACTUAL"),f.arrActual,151,C_GREEN);else row(5,TR("AN ETA","ARR ETA"),f.arrEstimated,151,C_CYAN);
  String dur=trackedFlightDuration(f);row(6,TR("FLUGZEIT","FLIGHT TIME"),dur.c_str(),174,C_WHITE);
  String alt=f.altitudeM>0?String(f.altitudeM)+" m":String("N/A");
  String spd=f.speedKmh>1?String((int)lroundf(f.speedKmh))+" km/h":String("N/A");
  visibleText(7,alt,8,204,1,C_CYAN,8,200,110,14);visibleText(8,spd,232-textW(spd,1),204,1,C_CYAN,120,200,112,14);
  drawFooter(f.error ? TR("LETZTER STAND","LAST DATA") : "AVIATIONSTACK + ADSB.LOL");
}

static void drawCountdown(uint8_t slot,bool force=true) {
  if(slot>3)return;
  const CountdownSlot& c=cfg.countdown[slot];
  String title=crop(displayAscii(c.title),24),value=countdownValue(c),old=lastCountdownRendered[slot];
  static int lastSlot=-1;static String oldTitle,oldTitleColor,oldColor;static uint8_t oldType=255;
  bool layout=force||lastSlot!=slot||oldTitle!=title||oldTitleColor!=c.titleColor||oldColor!=c.countColor||oldType!=c.type||old.length()!=value.length();
  if(!layout)for(size_t i=0;i<value.length();i++)if(sevenUnit(value[i])!=sevenUnit(old[i]))layout=true;
  beginView(layout);bool hasTitle=glyphCount(title)>0;uint16_t color=color565(c.countColor);
  if(layout&&hasTitle)centerText(title,28,glyphCount(title)<=12?3:2,color565(c.titleColor));
  int cy=c.type==COUNTDOWN_DAYS?(hasTitle?127:111):(hasTitle?135:120);
  if(layout||value!=old)drawSevenString(value,cy,color,!layout,old);
  if(layout&&c.type==COUNTDOWN_DAYS)centerText(TR("TAGE","DAYS"),hasTitle?183:168,2,color);
  lastCountdownRendered[slot]=value;lastSlot=slot;oldTitle=title;oldTitleColor=c.titleColor;oldColor=c.countColor;oldType=c.type;
}

static void drawClock(bool fullRedraw=true) {
  beginView(fullRedraw);if(fullRedraw)drawFooter(crop(cfg.timeZone,36).c_str(),false);
  String t=timeHHMM(),old=visibleFields[1].text;int x=(TFT_W-textW(t,6))/2;
  if(fullRedraw||old.length()!=t.length()){if(!fullRedraw)gfx->fillRect(0,72,TFT_W,66,C_BLACK);centerText(t,84,6,C_WHITE);}
  else for(size_t i=0;i<t.length();i++)if(t[i]!=old[i]){gfx->fillRect(x+i*36,84,36,48,C_BLACK);printDisplayTextAt(x+i*36,84,t.substring(i,i+1),6,C_WHITE);}
  visibleFields[1].text=t;
  time_t now=time(nullptr);String date=TR("warte auf NTP","waiting for NTP");
  if(now>1609459200){struct tm tmv;localtime_r(&now,&tmv);static const char* wdDe[]={"SO","MO","DI","MI","DO","FR","SA"};static const char* wdEn[]={"SU","MO","TU","WE","TH","FR","SA"};char b[32];if(cfg.lang==LANG_EN)snprintf(b,sizeof(b),"%s  %04d-%02d-%02d",wdEn[tmv.tm_wday],tmv.tm_year+1900,tmv.tm_mon+1,tmv.tm_mday);else snprintf(b,sizeof(b),"%s  %02d.%02d.%04d",wdDe[tmv.tm_wday],tmv.tm_mday,tmv.tm_mon+1,tmv.tm_year+1900);date=b;}
  visibleCenter(2,date,145,2,C_GRAY,140,28);
}


static void drawActive() {
  applyBrightness();
  int8_t slot=countdownSlotFromMode(cfg.mode);
  if(slot>=0){drawCountdown((uint8_t)slot);return;}
  switch (cfg.mode) {
    case MODE_WEATHER: drawWeather(); break;
    case MODE_PLANE: planePlacePhase=false; lastPlanePlaceSwitchMs=millis(); drawPlane(); break;
    case MODE_CLOCK: drawClock(); break;
    case MODE_TRACKED_FLIGHT: drawTrackedFlight(); break;
    default: drawDepartures(); break;
  }
}

// ---- network ----------------------------------------------------------------
static void startAP() {
  apMode = true;
  // Keep STA enabled while hosting setup AP so /api/wifi/scan can scan nearby networks.
  WiFi.mode(WIFI_AP_STA);
  IPAddress ip(192,168,4,1);
  WiFi.softAPConfig(ip,ip,IPAddress(255,255,255,0));
  WiFi.softAP("PixelStop-Setup");
  dns.start(53,"*",ip);

  gfx->fillScreen(C_BLACK);
  centerText("SETUP", 55, 4, C_YELLOW);
  centerText(TR("WLAN:","WI-FI:"), 105, 2, C_GRAY);
  centerText("PixelStop-Setup", 128, 2, C_WHITE);
  centerText("192.168.4.1", 174, 2, C_GREEN);
}

static void connectWiFi() {
  if (!cfg.ssid.length()) { startAP(); return; }
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.hostname(cfg.hostname.c_str());
  WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
  gfx->fillScreen(C_BLACK);
  centerText("WLAN", 80, 4, C_YELLOW);
  centerText("verbinden...", 126, 2, C_GRAY);

  uint32_t start=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<18000){delay(200);yield();}
  if(WiFi.status()!=WL_CONNECTED){startAP();return;}

  apMode=false;
  if(MDNS.begin(cfg.hostname.c_str())) MDNS.addService("http","tcp",80);
  String rule=String(FPSTR(timeZoneRule(timeZoneIndex(cfg.timeZone.c_str()))));
  configTime(rule.c_str(),"pool.ntp.org","time.cloudflare.com");
}

// ---- web API ----------------------------------------------------------------
static void sendJson(JsonDocument& d) {
  String out; serializeJson(d,out); server.send(200,"application/json",out);
}

static void countdownToJson(JsonObject out,const CountdownSlot& c) {
  out["title"]=c.title;out["titleColor"]=c.titleColor;out["countColor"]=c.countColor;out["type"]=c.type;out["targetDate"]=c.targetDate;
  out["durationSec"]=c.durationSec;out["targetEpoch"]=c.targetEpoch;out["active"]=c.active;out["dayDigits"]=c.dayDigits;
}

static void updateCountdownSettings(JsonObjectConst in,CountdownSlot& c) {
  if(in["title"].is<const char*>())c.title=in["title"].as<String>();
  c.title=crop(c.title,24);
  if(in["titleColor"].is<const char*>()){String x=in["titleColor"].as<String>();if(validHexColor(x))c.titleColor=x;}
  if(in["countColor"].is<const char*>()){String x=in["countColor"].as<String>();if(validHexColor(x))c.countColor=x;}
  uint8_t oldType=c.type;c.type=constrain((int)(in["type"]|c.type),0,2);
  if(oldType!=c.type){c.active=false;c.targetEpoch=0;c.dayDigits=3;}
  if(in["targetDate"].is<const char*>())c.targetDate=in["targetDate"].as<String>();
  uint32_t dur=in["durationSec"]|c.durationSec;if(dur>31536000UL)dur=31536000UL;c.durationSec=dur;
}

static void apiGetConfig() {
  JsonDocument d;
  d["ssid"]=cfg.ssid;d["hostname"]=cfg.hostname;d["mode"]=cfg.mode;d["rotationMask"]=cfg.rotationMask;d["brightness"]=cfg.brightness;d["timeZone"]=cfg.timeZone;
  JsonArray rs=d["rotationSec"].to<JsonArray>();for(uint8_t i=0;i<9;i++)rs.add(cfg.rotationSec[i]);
  d["nightEnabled"]=cfg.nightEnabled;d["nightStart"]=cfg.nightStart;d["nightEnd"]=cfg.nightEnd;d["nightBrightness"]=cfg.nightBrightness;
  d["stopName"]=cfg.stopName;d["stopId"]=cfg.stopId;d["departureCount"]=cfg.departureCount;d["departurePoll"]=cfg.departurePoll;d["vvoMotMask"]=cfg.vvoMotMask;d["lang"]=cfg.lang;d["departureCompactDestinations"]=cfg.departureCompactDestinations;d["departureAbbreviate"]=cfg.departureAbbreviate;d["departureDropTown"]=cfg.departureDropTown;
  d["weatherCity"]=cfg.weatherCity;d["weatherDisplayName"]=cfg.weatherDisplayName;d["weatherLat"]=cfg.weatherLat;d["weatherLon"]=cfg.weatherLon;d["weatherPoll"]=cfg.weatherPoll;d["weatherTempColor"]=cfg.weatherTempColor;d["weatherFahrenheit"]=cfg.weatherFahrenheit;d["geoGermanNames"]=cfg.geoGermanNames;
  d["planePlace"]=cfg.planePlace;d["planeDisplayName"]=cfg.planeDisplayName;d["planeLat"]=cfg.planeLat;d["planeLon"]=cfg.planeLon;d["planeRadiusKm"]=cfg.planeRadiusKm;d["planePoll"]=cfg.planePoll;d["planeShowAge"]=cfg.planeShowAge;d["planeShowPlace"]=cfg.planeShowPlace;d["planePlaceToggleSec"]=cfg.planePlaceToggleSec;
  d["trackedFlightNo"]=cfg.trackedFlightNo;d["trackerStatusPollMin"]=cfg.trackerStatusPollMin;d["aviationstackKeySet"]=cfg.aviationstackKey.length()>0;d["adsbLolKeySet"]=cfg.adsbLolKey.length()>0;
  JsonArray a=d["countdowns"].to<JsonArray>();for(uint8_t i=0;i<4;i++)countdownToJson(a.add<JsonObject>(),cfg.countdown[i]);
  sendJson(d);
}

static void apiPostConfig() {
  if(!server.hasArg("plain")){server.send(400,"application/json","{\"ok\":false,\"error\":\"Keine Daten empfangen.\"}");return;}
  JsonDocument d;if(deserializeJson(d,server.arg("plain"))){server.send(400,"application/json","{\"ok\":false,\"error\":\"Einstellungen konnten nicht gelesen werden.\"}");return;}
  if(d["timeZone"].is<const char*>() && timeZoneIndex(d["timeZone"].as<const char*>())<0){server.send(400,"application/json","{\"ok\":false,\"error\":\"Unbekannte Zeitzone.\"}");return;}
  Settings previous=cfg; String oldSsid=cfg.ssid,oldHost=cfg.hostname,oldTracked=cfg.trackedFlightNo,oldKey=cfg.aviationstackKey,oldAdsbKey=cfg.adsbLolKey;
  if(d["ssid"].is<const char*>())cfg.ssid=d["ssid"].as<String>();
  if(d["pass"].is<const char*>()){String x=d["pass"].as<String>();if(x.length())cfg.pass=x;}
  if(d["hostname"].is<const char*>())cfg.hostname=d["hostname"].as<String>();
  uint16_t mask=((uint16_t)(d["rotationMask"]|cfg.rotationMask))&0x01FFU;if(mask)cfg.rotationMask=mask;
  JsonArrayConst rs=d["rotationSec"].as<JsonArrayConst>();if(!rs.isNull())for(uint8_t i=0;i<9&&i<rs.size();i++)cfg.rotationSec[i]=constrain((int)(rs[i]|cfg.rotationSec[i]),5,3600);
  if(!modeEnabled(cfg.mode))cfg.mode=firstEnabledMode();
  cfg.brightness=constrain((int)(d["brightness"]|cfg.brightness),1,100);
  if(d["timeZone"].is<const char*>())cfg.timeZone=d["timeZone"].as<String>();
  cfg.nightEnabled=d["nightEnabled"]|cfg.nightEnabled;cfg.nightStart=constrain((int)(d["nightStart"]|cfg.nightStart),0,1439);cfg.nightEnd=constrain((int)(d["nightEnd"]|cfg.nightEnd),0,1439);cfg.nightBrightness=constrain((int)(d["nightBrightness"]|cfg.nightBrightness),1,100);
  if(d["stopName"].is<const char*>()) cfg.stopName=d["stopName"].as<String>();
  if(d["stopId"].is<const char*>()) cfg.stopId=d["stopId"].as<String>();
  {int dc=(int)(d["departureCount"]|cfg.departureCount);cfg.departureCount=(dc==10)?10:constrain(dc,3,5);}
  cfg.departurePoll=constrain((int)(d["departurePoll"]|cfg.departurePoll),10,300);
  cfg.vvoMotMask=((uint8_t)(d["vvoMotMask"]|cfg.vvoMotMask))&VVO_MOT_ALL;
  if(!cfg.vvoMotMask) cfg.vvoMotMask=VVO_MOT_ALL;
  cfg.lang=(uint8_t)(d["lang"]|cfg.lang)==LANG_EN?LANG_EN:LANG_DE;
  cfg.departureCompactDestinations=d["departureCompactDestinations"]|cfg.departureCompactDestinations;
  cfg.departureAbbreviate=d["departureAbbreviate"]|cfg.departureAbbreviate;
  cfg.departureDropTown=d["departureDropTown"]|cfg.departureDropTown;
  if(d["weatherCity"].is<const char*>()) cfg.weatherCity=d["weatherCity"].as<String>();
  if(d["weatherDisplayName"].is<const char*>()) cfg.weatherDisplayName=d["weatherDisplayName"].as<String>();
  cfg.weatherLat=d["weatherLat"]|cfg.weatherLat; cfg.weatherLon=d["weatherLon"]|cfg.weatherLon;
  cfg.weatherPoll=constrain((int)(d["weatherPoll"]|cfg.weatherPoll),60,3600);
  cfg.weatherTempColor=d["weatherTempColor"]|cfg.weatherTempColor;
  cfg.weatherFahrenheit=d["weatherFahrenheit"]|cfg.weatherFahrenheit;
  cfg.geoGermanNames=d["geoGermanNames"]|cfg.geoGermanNames;
  if(d["planePlace"].is<const char*>()) cfg.planePlace=d["planePlace"].as<String>();
  if(d["planeDisplayName"].is<const char*>()) cfg.planeDisplayName=d["planeDisplayName"].as<String>();
  cfg.planeLat=d["planeLat"]|cfg.planeLat; cfg.planeLon=d["planeLon"]|cfg.planeLon;
  cfg.planeRadiusKm=constrain((int)(d["planeRadiusKm"]|cfg.planeRadiusKm),1,250);
  cfg.planePoll=constrain((int)(d["planePoll"]|cfg.planePoll),3,300);
  cfg.planeShowAge=d["planeShowAge"]|cfg.planeShowAge;cfg.planeShowPlace=d["planeShowPlace"]|cfg.planeShowPlace;
  cfg.planePlaceToggleSec=(d["planePlaceToggleSec"]|cfg.planePlaceToggleSec)==0?0:constrain((int)(d["planePlaceToggleSec"]|cfg.planePlaceToggleSec),5,30);
  if(d["trackedFlightNo"].is<const char*>()){cfg.trackedFlightNo=d["trackedFlightNo"].as<String>();cfg.trackedFlightNo.trim();cfg.trackedFlightNo.toUpperCase();}
  if(d["aviationstackKey"].is<const char*>()){String x=d["aviationstackKey"].as<String>();x.trim();if(x.length())cfg.aviationstackKey=x;}
  // adsb.lol key: empty keeps the stored key, "-" removes it.
  if(d["adsbLolKey"].is<const char*>()){String x=d["adsbLolKey"].as<String>();x.trim();if(x=="-")cfg.adsbLolKey="";else if(x.length()&&x.length()<72)cfg.adsbLolKey=x;}
  cfg.trackerStatusPollMin=constrain((int)(d["trackerStatusPollMin"]|cfg.trackerStatusPollMin),5,120);
  JsonArrayConst a=d["countdowns"].as<JsonArrayConst>();if(!a.isNull())for(uint8_t i=0;i<4&&i<a.size();i++){updateCountdownSettings(a[i].as<JsonObjectConst>(),cfg.countdown[i]);if(cfg.countdown[i].active&&cfg.countdown[i].type==COUNTDOWN_DAYS){int32_t dd=0;if(countdownDayDifference(cfg.countdown[i].targetDate,dd)){uint32_t av=dd<0?(uint32_t)(-(int64_t)dd):(uint32_t)dd;cfg.countdown[i].dayDigits=max(cfg.countdown[i].dayDigits,max((uint8_t)3,decimalDigits(av)));}}}
  d.clear();
  diagEvent(D_CONFIG,DS_BEGIN);
  if(!saveConfig()){diagEvent(D_CONFIG,DS_FAILED);cfg=previous;server.send(500,"application/json","{\"ok\":false,\"error\":\"Einstellungen konnten nicht gespeichert werden.\"}");return;}diagEvent(D_CONFIG,DS_DONE);applyTimeZone();applyBrightness();if(oldAdsbKey!=cfg.adsbLolKey){adsbLolSetKey(cfg.adsbLolKey);serviceClear(SERVICE_ADSB_LOL);flightReset();}
  uint16_t activated=cfg.rotationMask & ~previous.rotationMask;
  if(previous.stopId!=cfg.stopId||previous.departureCount!=cfg.departureCount||previous.vvoMotMask!=cfg.vvoMotMask||(activated&(1U<<MODE_DEPARTURES)))vvoReset();
  if(previous.weatherLat!=cfg.weatherLat||previous.weatherLon!=cfg.weatherLon||previous.timeZone!=cfg.timeZone||(activated&(1U<<MODE_WEATHER)))weatherReset();
  if(previous.planeLat!=cfg.planeLat||previous.planeLon!=cfg.planeLon||previous.planeRadiusKm!=cfg.planeRadiusKm||(activated&(1U<<MODE_PLANE)))flightReset();
  deferBackgroundNetwork(1000);departurePage=0;departurePagingReady=false;lastDeparturePageSwitchMs=millis();planePlacePhase=false;lastPlanePlaceSwitchMs=millis();if(oldTracked!=cfg.trackedFlightNo||oldKey!=cfg.aviationstackKey)trackedFlightReset();lastModeSwitchMs=millis();drawActive();
  bool reboot=(oldSsid!=cfg.ssid)||(oldHost!=cfg.hostname);server.send(200,"application/json",reboot?"{\"ok\":true,\"reboot\":true}":"{\"ok\":true,\"reboot\":false}");if(reboot){diagEvent(D_REBOOT,DS_BEGIN);delay(500);ESP.restart();}
}

static void apiCountdown() {
  if(!server.hasArg("plain")){server.send(400,"application/json","{\"ok\":false,\"error\":\"Keine Daten empfangen.\"}");return;}
  JsonDocument d;if(deserializeJson(d,server.arg("plain"))){server.send(400,"application/json","{\"ok\":false,\"error\":\"Ungültige Daten.\"}");return;}
  int slot=d["slot"]|-1;if(slot<0||slot>3){server.send(400,"application/json","{\"ok\":false,\"error\":\"Ungültiger Countdown-Slot.\"}");return;}
  Settings previous=cfg; CountdownSlot& c=cfg.countdown[slot];updateCountdownSettings(d.as<JsonObjectConst>(),c);String action=d["action"]|"";
  if(action=="stop"){c.active=false;}
  else if(action=="start"){
    if(c.type==COUNTDOWN_DAYS){
      int32_t diff=0;if(!countdownDayDifference(c.targetDate,diff)){server.send(409,"application/json","{\"ok\":false,\"error\":\"Zieltag fehlt oder Uhrzeit ist noch nicht synchronisiert\"}");return;}
      uint32_t a=diff<0?(uint32_t)(-(int64_t)diff):(uint32_t)diff;c.dayDigits=max((uint8_t)3,decimalDigits(a));c.active=true;c.targetEpoch=0;
    } else {
      time_t now=time(nullptr);if(now<1609459200){server.send(409,"application/json","{\"ok\":false,\"error\":\"Uhrzeit ist noch nicht synchronisiert\"}");return;}
      if(c.durationSec==0){server.send(400,"application/json","{\"ok\":false,\"error\":\"Dauer muss größer als 0 sein.\"}");return;}
      c.targetEpoch=(uint32_t)now+c.durationSec;c.active=true;
    }
  } else {server.send(400,"application/json","{\"ok\":false,\"error\":\"Unbekannte Aktion.\"}");return;}
  if(!saveConfig()){cfg=previous;server.send(500,"application/json","{\"ok\":false,\"error\":\"Countdown konnte nicht gespeichert werden.\"}");return;}if(countdownSlotFromMode(cfg.mode)==slot)drawCountdown((uint8_t)slot);server.send(200,"application/json","{\"ok\":true}");
}

static void apiStatus() {
  JsonDocument d;d["fw"]=FW_NAME;d["version"]=FW_VERSION;d["ssid"]=apMode?"PixelStop-Setup":WiFi.SSID();d["ip"]=apMode?WiFi.softAPIP().toString():WiFi.localIP().toString();d["rssi"]=apMode?0:WiFi.RSSI();d["heap"]=ESP.getFreeHeap();d["uptime"]=millis()/1000;
  d["maxBlock"]=ESP.getMaxFreeBlockSize();d["vvoCount"]=vvoCount();d["vvoError"]=vvoError();d["weatherValid"]=weatherData().valid;d["weatherError"]=weatherData().error;d["flightValid"]=flightData().valid;d["flightError"]=flightData().error;d["flightStage"]=flightStageName();d["flightHttp"]=flightLastHttp();d["flightTlsRx"]=flightTlsRx();d["flightTlsMinBlock"]=flightTlsMinBlock();d["flightWaitSec"]=flightWaitSec();d["flightMinIntervalSec"]=serviceMinIntervalMs(SERVICE_ADSB_LOL)/1000;d["adsbToday"]=adsbLolRequestsToday();d["adsbKey"]=adsbLolHasKey();d["adsbBudgetSec"]=adsbLolBudgetIntervalMs(radarVisibleShare())/1000;d["routeStatus"]=flightRouteStatus();d["routeSource"]=flightRouteSource();d["routeHttp"]=flightRouteHttp();d["flightAgeSec"]=flightLastOkMs()?(millis()-flightLastOkMs())/1000:0;d["trackerValid"]=trackedFlightData().valid;d["trackerError"]=trackedFlightData().error;sendJson(d);
}

static void apiVvoSearch() {
  if(apMode||WiFi.status()!=WL_CONNECTED){server.send(503,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Kein Internetzugang.\"}");return;}
  String q=server.arg("q");q.trim();if(q.length()<2){server.send(400,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Suchbegriff zu kurz.\"}");return;}
  String out;if(!vvoSearchStops(q,out)){server.send(502,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"VVO-Suche fehlgeschlagen.\"}");return;}server.send(200,"application/json; charset=utf-8",out);
}

static void apiGeoSearch() {
  if(apMode||WiFi.status()!=WL_CONNECTED){server.send(503,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Kein Internetzugang.\"}");return;}
  String q=server.arg("q");q.trim();
  if(q.length()<2){server.send(400,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Suchbegriff zu kurz.\"}");return;}
  if(serviceWaitMs(SERVICE_GEO)){server.send(429,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Ortsdienst pausiert nach zu vielen Anfragen. Bitte später erneut versuchen.\"}");return;}
  String out;
  bool german=server.arg("german")=="1";
  if(!geoSearchPlaces(q,german,out)){server.send(502,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Ortssuche fehlgeschlagen.\"}");return;}
  server.send(200,"application/json; charset=utf-8",out);
}

static void apiGeoReverse() {
  if(apMode||WiFi.status()!=WL_CONNECTED){server.send(503,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Kein Internetzugang.\"}");return;}
  float lat=server.arg("lat").toFloat(),lon=server.arg("lon").toFloat();
  if(lat<-90||lat>90||lon<-180||lon>180){server.send(400,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Ungültige Koordinaten.\"}");return;}
  if(serviceWaitMs(SERVICE_GEO)){server.send(429,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Ortsdienst pausiert nach zu vielen Anfragen. Bitte später erneut versuchen.\"}");return;}
  String out;
  bool german=server.arg("german")=="1";
  if(!geoReversePlace(lat,lon,german,out)){server.send(502,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"Ortsname konnte nicht ermittelt werden.\"}");return;}
  server.send(200,"application/json; charset=utf-8",out);
}

static void apiWifiScan() {
  int n=WiFi.scanNetworks(false,false);
  if(n<0){server.send(500,"application/json; charset=utf-8","{\"ok\":false,\"error\":\"WLAN-Suche fehlgeschlagen.\"}");return;}
  JsonDocument d; d["ok"]=true; JsonArray a=d["networks"].to<JsonArray>();
  for(int i=0;i<n;i++){
    String ssid=WiFi.SSID(i); if(!ssid.length())continue;
    JsonObject x=a.add<JsonObject>(); x["ssid"]=ssid; x["rssi"]=WiFi.RSSI(i); x["open"]=(WiFi.encryptionType(i)==ENC_TYPE_NONE);
  }
  WiFi.scanDelete(); sendJson(d);
}

class LogResponse : public Print {
 public: size_t write(uint8_t c) override {return write(&c,1);}
 size_t write(const uint8_t* data,size_t len) override {server.sendContent((const char*)data,len);yield();return len;}
};
static void apiLog(){
 server.sendHeader("Cache-Control","no-store");
 if(server.hasArg("download"))server.sendHeader("Content-Disposition","attachment; filename=pixelstop-log.txt");
 server.setContentLength(CONTENT_LENGTH_UNKNOWN);server.send(200,"text/plain; charset=utf-8","");
 LogResponse response;diagPrint(response);server.sendContent("");deferBackgroundNetwork();
}
static void setupWeb() {
  server.on("/api/log",HTTP_GET,apiLog);
  server.on("/",HTTP_GET,[]{server.send_P(200,"text/html; charset=utf-8",WEBUI);});
  server.on("/i18n/en.json",HTTP_GET,[]{server.sendHeader("Cache-Control","max-age=86400");server.send_P(200,"application/json; charset=utf-8",WEBUI_EN);});
  server.on("/api/config",HTTP_GET,apiGetConfig);server.on("/api/config",HTTP_POST,apiPostConfig);server.on("/api/countdown",HTTP_POST,apiCountdown);server.on("/api/status",HTTP_GET,apiStatus);server.on("/api/vvo/search",HTTP_GET,apiVvoSearch);server.on("/api/geo/search",HTTP_GET,apiGeoSearch);server.on("/api/geo/reverse",HTTP_GET,apiGeoReverse);server.on("/api/wifi/scan",HTTP_GET,apiWifiScan);
  server.on("/api/reboot",HTTP_POST,[]{server.send(200,"application/json","{\"ok\":true}");diagEvent(D_REBOOT,DS_BEGIN);delay(300);ESP.restart();});updater.setup(&server,"/update");
  server.onNotFound([](){if(apMode){server.sendHeader("Location","http://192.168.4.1/",true);server.send(302,"text/plain","");}else server.send(404,"text/plain","Nicht gefunden");});server.begin();
}

// ---- setup / loop ------------------------------------------------------------
void setup() {
  diagInit();
  Serial.begin(115200);
  // Select the framework phase-locked generator for the single backlight PWM.
  enablePhaseLockedWaveform();
  // Lower PWM frequency: at 1 % the on-pulse becomes ~33 us instead of ~8 us,
  // so interrupt jitter from WiFi/TLS/flash access is far less visible.
  analogWriteFreq(BL_PWM_FREQ);
  analogWriteRange(BL_PWM_RANGE);
  pinMode(TFT_BL,OUTPUT);

  bus = new Arduino_HWSPI(TFT_DC,TFT_CS);
  gfx = new SmallTV_ST7789(bus,TFT_RST,0,true,TFT_W,TFT_H,0,0,0,80);
  gfx->begin();
  gfx->setRotation(0);
  gfx->setTextWrap(false);
  gfx->fillScreen(C_BLACK);
  centerText(FW_NAME,82,3,C_YELLOW);
  centerText(FW_VERSION,118,2,C_GRAY);

  loadConfig();
  adsbLolSetKey(cfg.adsbLolKey);
  applyTimeZone();
  applyBrightness();
  connectWiFi();
  setupWeb();

  if(!apMode){
    gfx->fillScreen(C_BLACK);
    centerText(TR("VERBUNDEN","CONNECTED"),50,3,C_GREEN);
    centerText(WiFi.localIP().toString(),105,2,C_WHITE);
    centerText(cfg.hostname+".local",137,2,C_GRAY);
    delay(2200);
    drawActive();
    lastModeSwitchMs=millis();
  }
}

void loop() {
  server.handleClient();
  if(apMode)dns.processNextRequest();else MDNS.update();

  uint32_t nowMs=millis();
  if(nowMs-lastSecond>=1000){
    lastSecond=nowMs; applyBrightness();
    if(flightExpire()&&cfg.mode==MODE_PLANE)drawPlane(false);
    if(cfg.mode==MODE_PLANE)drawPlaneAge();
    time_t ep=time(nullptr);
    if(ep>1609459200){
      struct tm t; localtime_r(&ep,&t);
      if(t.tm_min!=lastMinute){
        lastMinute=t.tm_min;
        if(cfg.mode==MODE_CLOCK) drawClock(false);
        else if(cfg.mode==MODE_DEPARTURES||cfg.mode==MODE_WEATHER||cfg.mode==MODE_PLANE||cfg.mode==MODE_TRACKED_FLIGHT) drawFooterClockOnly();
      }
    }
    int8_t slot=countdownSlotFromMode(cfg.mode);
    if(slot>=0){
      String visible=countdownValue(cfg.countdown[(uint8_t)slot]);
      if(visible!=lastCountdownRendered[(uint8_t)slot]) drawCountdown((uint8_t)slot,false);
    }
  }

  // View rotation is separate from data refreshes. API data can stay fresh while hidden.
  if(!apMode&&enabledModeCount()>1){
    uint32_t stayMs=(uint32_t)max((uint16_t)5,cfg.rotationSec[cfg.mode])*1000UL;
    if(nowMs-lastModeSwitchMs>=stayMs){
      cfg.mode=nextEnabledMode(cfg.mode); lastModeSwitchMs=nowMs; drawActive();
    }
  }

  if(cfg.departureCount==10&&vvoCount()>5){
    if(!departurePagingReady){departurePagingReady=true;departurePage=0;lastDeparturePageSwitchMs=nowMs;}
    else if((uint32_t)(nowMs-lastDeparturePageSwitchMs)>=5000UL){departurePage^=1;lastDeparturePageSwitchMs=nowMs;if(cfg.mode==MODE_DEPARTURES)drawDepartures(false);}
  }else{departurePagingReady=false;departurePage=0;lastDeparturePageSwitchMs=nowMs;}

  if(cfg.mode==MODE_PLANE&&cfg.planeShowPlace&&cfg.planePlaceToggleSec>0&&planePlaceTitle().length()){
    if((uint32_t)(nowMs-lastPlanePlaceSwitchMs)>=(uint32_t)cfg.planePlaceToggleSec*1000UL){planePlacePhase=!planePlacePhase;lastPlanePlaceSwitchMs=nowMs;drawPlaneHeader();}
  }else if(!cfg.planeShowPlace){planePlacePhase=false;lastPlanePlaceSwitchMs=nowMs;}

  if(!apMode&&backgroundNetworkReady()){
    static uint8_t cursor=0;
    for(uint8_t n=0;n<6;n++){
      uint8_t task=cursor;cursor=(cursor+1)%6;bool ran=false;
      if(task==0&&modeEnabled(MODE_DEPARTURES)&&cfg.stopId.length()&&vvoPollDue(cfg.departurePoll)){
        vvoFetchDepartures(cfg.stopId,cfg.departureCount,cfg.vvoMotMask);if(cfg.mode==MODE_DEPARTURES)drawDepartures(false);ran=true;
      }else if(task==1&&modeEnabled(MODE_WEATHER)&&weatherPollDue(cfg.weatherPoll)){
        weatherFetch(cfg.weatherLat,cfg.weatherLon,cfg.timeZone.c_str());if(cfg.mode==MODE_WEATHER)drawWeather(false);ran=true;
      }else if(task==2&&radarActive()&&flightPollDue(cfg.planePoll,radarVisibleShare())){
        flightFetch(cfg.planeLat,cfg.planeLon,cfg.planeRadiusKm);if(cfg.mode==MODE_PLANE)drawPlane(false);ran=true;
      }else if(task==3&&radarActive()&&flightEnrichmentDue()){
        flightEnrich();if(cfg.mode==MODE_PLANE)drawPlane(false);ran=true;
      }else if(cfg.mode==MODE_TRACKED_FLIGHT&&cfg.trackedFlightNo.length()&&cfg.aviationstackKey.length()){
        if(task==4&&trackedFlightStatusPollDue(cfg.trackerStatusPollMin)){trackedFlightFetchStatus(cfg.aviationstackKey,cfg.trackedFlightNo);trackedFlightFetchRoute();drawTrackedFlight(false);ran=true;}
        else if(task==5&&trackedFlightLivePollDue(15)){trackedFlightFetchLive();trackedFlightFetchRoute();drawTrackedFlight(false);ran=true;}
      }
      if(ran){deferBackgroundNetwork();break;}
    }
  }
  yield();
}
