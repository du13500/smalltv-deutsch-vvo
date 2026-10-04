# SmallTV Deutsch/VVO

[![Build SmallTV DD v0.4.1](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v041.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v041.yml)

Deutschsprachige Custom-Firmware für den **GeekMagic SmallTV-Ultra** mit ESP8266 und 240 × 240 Pixel ST7789-Display.

Aus dem kleinen WLAN-Display wird ein autonomer Informationswürfel für **VVO-Abfahrten, Wetter, Flugradar, Uhr, Flug-Tracking und vier unabhängige Countdowns**. Nach der Einrichtung benötigt der SmallTV nur **USB-Strom und WLAN**. Ein Raspberry Pi, Home Assistant oder anderer eigener Server ist nicht erforderlich.

> **Aktueller Stand:** v0.4.1 wurde erfolgreich gebaut und auf echter SmallTV-Ultra-Hardware getestet. **v0.4.2** ist die aktuelle Entwicklungsfassung mit UX- und Display-Politur. Build und Hardwaretest von v0.4.2 stehen noch aus.

## Unterstützte Hardware

Der aktuelle Build ist für den **GeekMagic SmallTV-Ultra** ausgelegt:

- ESP8266 / ESP-12F
- 4 MB Flash
- 240 × 240 Pixel ST7789
- SPI Mode 3
- LittleFS
- OTA-Firmwareupdates über das Webinterface

Andere SmallTV-Varianten sind derzeit nicht getestet.

## Programme

### Abfahrtsmonitor

Live-Abfahrten des Verkehrsverbunds Oberelbe direkt auf dem Würfel.

- Haltestellensuche per Freitext im Webinterface
- Echtzeit-Abfahrten und Ausfälle
- Anzahl der angezeigten Abfahrten konfigurierbar
- Verkehrsmittel einzeln auswählbar: Zug, S-Bahn, Straßenbahn, Bus, Seil-/Schwebebahn, Fähre und AST/Rufbus
- adaptive Darstellung längerer Linienbezeichnungen wie `RE50`, `IC 2045` oder `ICE 1650`
- farbliche Statusanzeige für reguläre, verspätete, unmittelbar anstehende und ausfallende Fahrten

### Wetter

Aktuelles Wetter über Open-Meteo.

- Ortssuche statt manueller Koordinatensuche
- ab v0.4.2 zusätzlicher Karten-Picker für die exakte Position
- Koordinaten bleiben weiterhin manuell editierbar
- optionaler eigener Anzeigename, z. B. `ZUHAUSE`
- aktuelle Temperatur
- Wetterzustand
- Tageshöchst- und Tiefsttemperatur
- Regenwahrscheinlichkeit
- Sonnenaufgang und Sonnenuntergang mit eigenen Pixel-Symbolen
- optional farbige Darstellung der großen aktuellen Temperatur
- deutsche Umlaute und `ß` im Pixel-Font

### Flugradar

Zeigt das nächstgelegene empfangene Flugzeug zu einem frei wählbaren Beobachtungspunkt.

- eigener Standort unabhängig vom Wetterprogramm
- Ortssuche im Webinterface
- ab v0.4.2 Karten-Picker zum exakten Setzen des Radar-Mittelpunkts
- Koordinaten weiterhin manuell editierbar
- frei einstellbarer Suchradius
- Callsign
- Airline, soweit auflösbar
- Route, soweit über die verfügbaren Datenquellen auflösbar
- Flugzeugtyp
- Registrierung
- Entfernung
- Höhe
- `N/A`, wenn eine Information tatsächlich nicht verfügbar ist

Für die Live-Flugdaten wird **adsb.lol** verwendet. Für die Routenauflösung wird zunächst adsb.lol und bei Bedarf zusätzlich **adsbdb** abgefragt.

### Uhr

- NTP-Zeitsynchronisation
- deutsche Zeitzone mit Sommer- und Winterzeit
- Uhrzeit als eigene Ansicht
- Footer-Uhr in den Informationsansichten
- ab v0.4.2 ohne zusätzliche `UHR`-Überschrift
- minütliche Aktualisierung ohne vollständiges Löschen und Neuzeichnen des Displays

### Flug-Tracker

Detailansicht für einen gezielt eingetragenen Flug.

- Eingabe einer Flugnummer im Webinterface
- Flugstatus und Flugplandaten optional über aviationstack
- zusätzliche Live-Telemetrie über ADS-B, soweit verfügbar
- Route
- geplante und tatsächliche Zeiten
- Flugdauer
- Höhe und Geschwindigkeit

Für aviationstack ist ein eigener API-Key erforderlich. Der Flug-Tracker ist optional; ohne Key bleiben die übrigen Programme vollständig nutzbar.

### Countdown 1 bis 4

Vier voneinander unabhängige und dauerhaft gespeicherte Countdown-Slots.

- optionaler Titel
- eigene Farbe für Titel und Countdown
- 7-Segment-Darstellung im Retro-Digitalstil
- Kalendertage, Stunden:Minuten:Sekunden oder Minuten:Sekunden
- `-` vor dem Zielzeitpunkt, kein sichtbares Vorzeichen bei Null, `+` danach
- feste Vorzeichenspalte gegen horizontales Springen
- absolute Zielzeitpunkte bleiben auch nach Stromverlust erhalten
- ab v0.4.2 wird ein Tages-Countdown nur noch neu gezeichnet, wenn sich sein sichtbarer Wert tatsächlich ändert

## Anzeigerotation

Mehrere Ansichten können gleichzeitig aktiviert und mit einer individuellen Anzeigedauer versehen werden. Bei nur einer aktivierten Ansicht bleibt diese dauerhaft stehen.

Die **Anzeigedauer ist von der Datenaktualisierung getrennt**. Wetter- oder VVO-Daten werden nicht bei jedem Einblenden neu geladen.

## Webinterface

Die Konfiguration erfolgt lokal im Browser. Bereiche gibt es für Anzeige/Rotation, Abfahrten, Wetter, Flugradar, Flug-Tracker, Countdowns, WLAN und Systemstatus.

### Neu in v0.4.2

- Branding als **SmallTV Deutsch/VVO**
- dezenter Footer mit Autor und GitHub-Link
- störende Browser-`alert()`-Fenster durch automatische Toast-Meldungen ersetzt
- Karten-Picker für Wetter und Flugradar
- Browser-GPS-Schaltfläche entfernt, da Geolocation auf einer lokalen HTTP-Seite von modernen Browsern regelmäßig blockiert wird
- manuelle Eingabe von Breitengrad und Längengrad bleibt erhalten
- Uhr ohne gelbe `UHR`-Überschrift und mit teilweisem statt vollständigem Redraw
- Tages-Countdown ohne unnötiges sekündliches Fullscreen-Redraw

Der Karten-Picker wird erst beim Öffnen im Browser geladen. Dadurch bleibt das normale Webinterface auch im Setup-Hotspot ohne Internet benutzbar. Die Karte verwendet **Leaflet** und Kartenkacheln von **OpenStreetMap**. Ein Klick auf die Karte oder das Verschieben des Markers übernimmt die exakten Koordinaten.

## Ersteinrichtung

Ohne gespeicherte WLAN-Daten öffnet die Firmware den Access Point:

```text
SmallTV-Setup
```

Danach im Browser öffnen:

```text
http://192.168.4.1/
```

WLAN und gewünschte Programme konfigurieren, speichern und den Neustart abwarten.

> Die Ortssuche und der Karten-Picker benötigen Internetzugang. Im reinen Setup-Hotspot ohne Internet bleiben die manuellen Koordinatenfelder als Fallback verfügbar.

## Installation und Updates

### Update von v0.4/v0.4.1 auf eine neuere Version

Wer bereits diese Custom-Firmware verwendet, benötigt den Ultra-Loader **nicht erneut**.

1. Erfolgreichen GitHub-Actions-Build der gewünschten Version öffnen.
2. Build-Artefakt herunterladen und entpacken.
3. Im SmallTV-Webinterface `Firmware-Update` öffnen oder `http://<IP-des-SmallTV>/update` aufrufen.
4. Nur die kompilierte `*-firmware.bin` hochladen.
5. Während des Updates die Stromversorgung nicht trennen.

Die gespeicherte Konfiguration wird weiterverwendet; neue Einstellungen erhalten kompatible Standardwerte.

### Erstinstallation auf originaler SmallTV-Ultra-Firmware

Die originale Ultra-Firmware verwendet ein Partitionslayout, bei dem die vollständige Custom-Firmware nicht direkt in den verfügbaren OTA-Slot passt. Die Erstinstallation erfolgt deshalb zweistufig über den kleinen **SmallTV-Ultra-Loader**.

> **Wichtig:** Die vollständige SmallTV-DD-Firmware nicht direkt über den Updater der originalen Ultra-Firmware hochladen.

1. Loader-BIN über den Loader-Workflow bauen bzw. das Artefakt herunterladen.
2. In der originalen SmallTV-Firmware `/update` öffnen.
3. Nur `smalltv-ultra-loader.bin` hochladen.
4. Nach dem Neustart mit `SmallTV-Loader` verbinden.
5. `http://192.168.4.1/update` öffnen.
6. Dort die aktuelle Custom-Firmware-BIN hochladen.
7. Anschließend über `SmallTV-Setup` einrichten.

## Build

Das Projekt verwendet PlatformIO. Im entpackten Versionsordner:

```bash
pio run -e smalltv_ultra
```

Die Firmware liegt anschließend unter:

```text
.pio/build/smalltv_ultra/firmware.bin
```

Verwendete Hauptbibliotheken:

- ArduinoJson
- GFX Library for Arduino
- ESP8266 Arduino Core

## Datenquellen und externe Dienste

Die Firmware besitzt keinen eigenen Cloud-Backenddienst, greift für Live-Daten aber direkt auf externe Quellen zu:

| Funktion | Quelle |
| --- | --- |
| VVO-Abfahrten | VVO-WebAPI |
| Wetter | Open-Meteo |
| Ortssuche | Open-Meteo Geocoding |
| Flugradar | adsb.lol |
| Flugrouten-Fallback | adsbdb |
| Flug-Tracker | optional aviationstack + adsb.lol |
| Karten-Picker | Leaflet + OpenStreetMap |
| Uhrzeit | NTP |

Verfügbarkeit, Rate-Limits, Nutzungsbedingungen und Datenlizenzen dieser Dienste können sich unabhängig von diesem Projekt ändern.

## Lizenz und kommerzielle Nutzung

Der **Quellcode dieses Projekts** wird grundsätzlich frei unter der [WTFPL Version 2](LICENSE) bereitgestellt. Er darf damit grundsätzlich verwendet, verändert und weitergegeben werden, auch in kommerziellen Projekten.

**Diese Projektlizenz umfasst jedoch nicht automatisch die externen Daten und APIs, die die Firmware verwendet.** Für sie gelten die jeweiligen Bedingungen der Anbieter. Das ist insbesondere bei kommerzieller Nutzung wichtig:

- Die kostenlose Open-Meteo-API ist aktuell für **nicht-kommerzielle Nutzung** vorgesehen; kommerzielle Nutzung erfordert einen geeigneten Dienst bzw. eine andere Bereitstellung. Siehe <https://open-meteo.com/en/terms>.
- Der VVO weist für Inhalte seines Internetangebotes auf Beschränkungen für öffentliche bzw. kommerzielle Verwertung ohne vorherige Zustimmung hin. Siehe <https://www.vvo-online.de/de/impressum/index.cshtml>.
- adsb.lol stellt seine öffentliche API und öffentlich bereitgestellte Daten unter der **ODbL** bereit. Siehe <https://api.adsb.lol/docs>.
- adsbdb weist für Teile der verwendeten Flugroutendaten auf eigene Weiterverwendungsbeschränkungen hin. Siehe <https://www.adsbdb.com/>.
- Der kostenlose aviationstack-Tarif ist aktuell für **nicht-kommerzielle Nutzung** vorgesehen. Siehe <https://aviationstack.com/pricing>.
- OpenStreetMap-Daten stehen unter der ODbL; für die Standard-Kachelserver gelten zusätzlich eigene Nutzungsregeln und Attributionspflichten. Siehe <https://www.openstreetmap.org/copyright> und <https://operations.osmfoundation.org/policies/tiles/>.

Wer SmallTV Deutsch/VVO kommerziell einsetzt oder vertreibt, muss daher selbst prüfen, ob die jeweils aktivierten externen Quellen dafür genutzt werden dürfen, und gegebenenfalls eigene Lizenzen, eigene Infrastruktur oder alternative Datenquellen verwenden.

## Datenschutz

Die Konfiguration liegt lokal auf dem SmallTV. WLAN-Passwort, ausgewählte Orte, Countdowns und gegebenenfalls ein aviationstack-Key werden auf dem Gerät gespeichert.

Der Karten-Picker läuft im Browser. Beim Laden der Karte werden Leaflet-Ressourcen von `unpkg.com` sowie Kartenkacheln von OpenStreetMap abgerufen. Die Firmware selbst betreibt keinen eigenen Cloud-Dienst.

## Bekannte Grenzen

- Flugrouten lassen sich nicht für jedes ADS-B-Callsign zuverlässig auflösen; bei fehlenden Daten wird bewusst `N/A` angezeigt.
- Ortssuche und Karten-Picker benötigen Internetzugang.
- Die Zuverlässigkeit der Live-Funktionen hängt von den jeweiligen externen APIs und der WLAN-Verbindung ab.
- v0.4.2 ist bis zum erfolgreichen GitHub-Actions-Build und Hardwaretest als Entwicklungsfassung zu betrachten.

## Vorgemerkt für v0.5

- DWD-Wetterwarnungen als zusätzliche Wetterseite, optional per Checkbox und mit Hinweis `nur Deutschland`
- Bevölkerungsschutzwarnungen als zusätzliche Wetterseite, separat schaltbar und ebenfalls `nur Deutschland`
- mehrere gleichzeitig aktive Warnungen nacheinander anzeigen
- mögliche neue Flughafen-Ankunfts-/Abflugtafel mit möglichst freier, registrierungsfreier Datenquelle

## Projektgeschichte

- **v0.1:** deutsches Webinterface, WLAN, NTP, OTA und erste Ansichten
- **v0.2:** echte VVO-Haltestellensuche und Live-Abfahrten
- **v0.3:** Open-Meteo, adsb.lol und live arbeitende Hauptansichten
- **v0.4:** Flug-Tracker und vier persistente Countdowns; Hardwaretest auf echtem SmallTV-Ultra erfolgreich
- **v0.4.1:** Anzeigerotation, WLAN-Scan, Ortssuchen, deutsche Glyphen, VVO-Verkehrsmittelfilter, Wetter-Politur und robusteres Flugradar; erfolgreich auf echter Hardware getestet
- **v0.4.2:** UX-/Redraw-Politur, Karten-Picker, Toast-Meldungen und Projekt-Branding; aktuell in Entwicklung

## Basis

Das Projekt entstand auf Basis von [`giovi321/smalltv-mod`](https://github.com/giovi321/smalltv-mod), insbesondere dessen Hardware- und Displayansteuerung für den GeekMagic SmallTV. Das Ursprungsprojekt steht ebenfalls unter der **WTFPL**.

## Hinweis

Dies ist ein privates Community-/Experimentierprojekt und keine offizielle Firmware von GeekMagic, VVO, DVB, Open-Meteo, adsb.lol, adsbdb, aviationstack, Leaflet oder OpenStreetMap. Marken, Daten und Dienste gehören ihren jeweiligen Rechteinhabern.
