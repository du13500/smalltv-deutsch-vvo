# SmallTV Deutsch/VVO

[![Build SmallTV DD v0.4.6](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v046.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v046.yml)

Deutschsprachige Custom-Firmware für den **GeekMagic SmallTV-Ultra** mit ESP8266 und 240 × 240 Pixel ST7789-Display.

Aus dem kleinen WLAN-Display wird ein autonomer Informationswürfel für **VVO-Abfahrten, Wetter, Flugradar, Uhr, Flug-Tracking und vier unabhängige Countdowns**. Nach der Einrichtung benötigt der SmallTV nur **USB-Strom und WLAN**. Ein Raspberry Pi, Home Assistant oder anderer eigener Server ist nicht erforderlich.

> **Aktueller Stand: v0.4.6, bereit für den Hardwaretest.** v0.4.5 wurde auf
> echter SmallTV-Ultra-Hardware getestet; v0.4.6 behebt die dabei gemeldeten Punkte.
> Prüfungen und verbleibende Hardwaretests stehen in [TESTPLAN.md](TESTPLAN.md).

**Quellcode:** `smalltv-deutsch-vvo-v0.4.6.zip` entpacken und im Versionsordner
bauen. Der Workflow `build-v046.yml` verwendet dieses Archiv. Die übrigen
historischen Dateien des Ursprungsprojekts im Repository-Hauptverzeichnis
werden für diesen Versionsbuild nicht verwendet.
Der Workflow gehört nach `.github/workflows/build-v046.yml`.
Die Dokumentationsdateien aus dem Lieferpaket ersetzen die Dateien im
Repository-Hauptverzeichnis.

[Änderungen](CHANGELOG.md) · [Themenspeicher](ROADMAP.md) ·
[Mitmachen](CONTRIBUTING.md) · [Datenquellen](THIRD_PARTY_SERVICES.md)

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
- optional 10 Abfahrten auf zwei automatisch im festen 5-Sekunden-Takt wechselnden 5er-Seiten
- Verkehrsmittel einzeln auswählbar: Zug, S-Bahn, Straßenbahn, Bus, Seil-/Schwebebahn, Fähre und AST/Rufbus
- adaptive Darstellung längerer Linienbezeichnungen wie `RE50`, `IC 2045` oder `ICE 1650`
- optional kompakte Schrift für lange Fahrziele, die sonst gekürzt würden
- farbliche Statusanzeige für reguläre, verspätete, unmittelbar anstehende und ausfallende Fahrten

### Wetter

Aktuelles Wetter über Open-Meteo.

- Ortssuche statt manueller Koordinatensuche
- ab v0.4.2 zusätzlicher Karten-Picker für die exakte Position
- Reverse-Geocoding des gesetzten Kartenpunkts mit detaillierten Orts-/Stadtteilnamen; eine Suchauswahl behält ihren ausgewählten Ortsnamen
- lokale Ortsnamen oder optional deutsche Namen, wenn verfügbar
- Gewässer-Fallbacks für Punkte auf Nordsee, Ostsee, Mittelmeer und Atlantik
- Koordinaten bleiben weiterhin manuell editierbar
- optionaler eigener Anzeigename, z. B. `ZUHAUSE`
- aktuelle Temperatur in Celsius oder Fahrenheit
- Wetterzustand
- Tageshöchst- und Tiefsttemperatur
- Regenwahrscheinlichkeit
- Sonnenaufgang und Sonnenuntergang mit eigenen Pixel-Symbolen
- optional farbige Darstellung der großen aktuellen Temperatur
- europäische Sonderzeichen im Pixel-Font, einschließlich französischer, tschechischer und polnischer Buchstaben
- kompakte H/T-Werte wie `H 21°C` und `T 7°C`

### Flugradar

Zeigt das nächstgelegene empfangene Flugzeug zu einem frei wählbaren Beobachtungspunkt.

- eigener Standort unabhängig vom Wetterprogramm
- Ortssuche im Webinterface
- ab v0.4.2 Karten-Picker zum exakten Setzen des Radar-Mittelpunkts
- Reverse-Geocoding nach derselben Ortslogik wie beim Wetter
- optionaler Anzeigename und optionaler Wechsel zwischen `IN DER NÄHE` und Standortname
- Koordinaten weiterhin manuell editierbar
- frei einstellbarer Suchradius
- Callsign
- Airline, soweit auflösbar
- Route samt verfügbaren Zwischenstopps; das normale Zwei-Flughäfen-Layout bleibt unverändert
- bis zu acht Flughafencodes, bei längeren Folgen sichtbarer Fortsetzungshinweis
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
- Footer-Uhr in den Informationsansichten, in der Uhr-Ansicht selbst ohne redundante zweite Uhrzeit
- ab v0.4.2 ohne zusätzliche `UHR`-Überschrift
- minütliche Aktualisierung ohne vollständiges Löschen und Neuzeichnen des Displays

### Flug-Tracker

Detailansicht für einen gezielt eingetragenen Flug.

- Eingabe einer Flugnummer im Webinterface
- Flugstatus und Flugplandaten optional über aviationstack
- zusätzliche Live-Telemetrie über ADS-B, soweit verfügbar
- Route mit belegten Zwischenstopps, soweit zur gemeldeten Strecke passend
- durchgehender Footer mit Status/Datenquelle und minütlich aktualisierter Uhr
- geplante und tatsächliche Zeiten
- Flugdauer
- Höhe und Geschwindigkeit

Für aviationstack ist weiterhin ein eigener API-Key erforderlich. Der Flug-Tracker
ist optional; ohne Key bleiben die übrigen Programme vollständig nutzbar. Eine
angezeigte Strecke ist keine Nonstop-Zusage. Kostenlose Routen-/ADS-B-Quellen
ersetzen die Planzeiten und den Flugstatus nicht vollständig, siehe
[Ergebnis der Quellenprüfung](THIRD_PARTY_SERVICES.md#flugtracker-ergebnis-der-quellenprüfung-für-v046).

### Countdown 1 bis 4

Vier voneinander unabhängige und dauerhaft gespeicherte Countdown-Slots.

- optionaler Titel mit europäischen Sonderzeichen und Pixelherzen
- Herzzeichen und farbige Herz-Emojis erscheinen einheitlich in der gewählten Titelfarbe
- unbekannte Zeichen erscheinen als Kasten `□`, echte Fragezeichen bleiben erhalten
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

### Neu in v0.4.6

- Zeitfelder passen auf schmalen Displays innerhalb der Karte; nebeneinander
  erhalten sie gleich breite Spalten mit Abstand.
- Checkbox-Zwischenüberschriften sind grau und normal gewichtet.
- Bestehende Option „Lange Ziele kompakt darstellen“ bleibt unverändert.
- Erweiterter Pixelzeichensatz, Herzen und Kasten für unbekannte Zeichen.
- Detaillierte Ortsnamen bei Kartenpunkten und kompakte H/T-Werte.
- Vollständiger Flugtracker-Hinweis, Footer in allen Zuständen und Zwischenstopps.

Alle Änderungen einschließlich früherer Versionen stehen im [Changelog](CHANGELOG.md).

Der Karten-Picker wird erst beim Öffnen im Browser geladen. Dadurch bleibt das normale Webinterface auch im Setup-Hotspot ohne Internet benutzbar. Die Karte verwendet **Leaflet** und Kartenkacheln von **OpenStreetMap**. Ein Klick auf die Karte oder das Verschieben des Markers übernimmt die exakten Koordinaten. Ab v0.4.4 wird der gesetzte Kartenpunkt zusätzlich über **OpenStreetMap Nominatim** in einen passenden geografischen Namen aufgelöst.

Die Flugroute wird beim ersten Auftauchen eines Callsigns über adsb.lol aufgelöst, mit ADSBDB als Fallback. Ein positiver Treffer bleibt für diesen Callsign im Cache. Falls beide Quellen zunächst keine Route liefern, erfolgt nach 60 Sekunden genau ein weiterer Versuch. Danach werden für denselben Callsign keine weiteren Routenanfragen gesendet. Die Routendaten sind keine Live-Flugplandaten und können quellenbedingt fehlen oder veraltet sein. In diesem Fall zeigt die Firmware bewusst `N/A > N/A` statt eine Route zu erfinden.

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
| Ortssuche / Reverse-Geocoding | OpenStreetMap Nominatim |
| Flugradar | adsb.lol |
| Flugrouten-Fallback | adsbdb |
| Flug-Tracker | optional aviationstack + adsb.lol |
| Karten-Picker | Leaflet + OpenStreetMap |
| Uhrzeit | NTP |

Verfügbarkeit, Rate-Limits, Nutzungsbedingungen und Datenlizenzen dieser Dienste können sich unabhängig von diesem Projekt ändern.

## Lizenz und externe Daten

Projektcode und eigene Änderungen stehen unter der [WTFPL Version 2](LICENSE).
Die ursprüngliche Lizenz bleibt unverändert. Datenquellen, Bibliotheken und
Dienste haben eigene Bedingungen: [THIRD_PARTY_SERVICES.md](THIRD_PARTY_SERVICES.md).

## Datenschutz

Die Konfiguration liegt lokal auf dem SmallTV. WLAN-Passwort, ausgewählte Orte, Countdowns und gegebenenfalls ein aviationstack-Key werden auf dem Gerät gespeichert.

Der Karten-Picker läuft im Browser. Beim Laden der Karte werden Leaflet-Ressourcen von `unpkg.com` sowie Kartenkacheln von OpenStreetMap abgerufen. Die Firmware selbst betreibt keinen eigenen Cloud-Dienst.

## Bekannte Grenzen

- Flugrouten lassen sich nicht für jedes ADS-B-Callsign zuverlässig auflösen; bei fehlenden Daten wird bewusst `N/A` angezeigt.
- Ortssuche und Karten-Picker benötigen Internetzugang.
- Die Zuverlässigkeit der Live-Funktionen hängt von den jeweiligen externen APIs und der WLAN-Verbindung ab.
- v0.4.6 benötigt noch den Hardwaretest, insbesondere native Safari-Zeitfelder,
  Glyphenlesbarkeit und Tracker mit eigenem API-Key.
- Zwei Flughafencodes garantieren keinen Nonstop-Flug. Nicht gelieferte Stopps
  werden nicht geraten.
- HTTPS verwendet wie die Vorversion `setInsecure()` ohne Zertifikatsprüfung.
  Das lokale Webinterface und OTA sind ohne Anmeldung erreichbar; das Gerät
  gehört in ein vertrauenswürdiges WLAN und sollte nicht öffentlich erreichbar sein.

## Kommende Versionen

Für v0.5 sind optionale DWD-/Bevölkerungsschutzwarnungen sowie die Prüfung einer
Flughafen-Ankunfts-/Abflugtafel vorgemerkt. Text-/Listenprogramme, Regenradar,
Bahnhofsmonitor und AIS bleiben spätere Ideen. Der vollständige Stand mit
v1.0-Veröffentlichungsideen liegt in [ROADMAP.md](ROADMAP.md).

## Projektgeschichte

- **v0.1:** deutsches Webinterface, WLAN, NTP, OTA und erste Ansichten
- **v0.2:** echte VVO-Haltestellensuche und Live-Abfahrten
- **v0.3:** Open-Meteo, adsb.lol und live arbeitende Hauptansichten
- **v0.4:** Flug-Tracker und vier persistente Countdowns; Hardwaretest auf echtem SmallTV-Ultra erfolgreich
- **v0.4.1:** Anzeigerotation, WLAN-Scan, Ortssuchen, deutsche Glyphen, VVO-Verkehrsmittelfilter, Wetter-Politur und robusteres Flugradar; erfolgreich auf echter Hardware getestet
- **v0.4.2:** UX-/Redraw-Politur, Karten-Picker, Toast-Meldungen und Projekt-Branding; GitHub-Actions-Build erfolgreich
- **v0.4.3:** Privacy-Hotfix
- **v0.4.4:** Karten-/Standort-Politur, Wettereinheiten, VVO-10er-Paging und robustere Flugradar-Fehlerbehandlung; auf echter Hardware getestet
- **v0.4.5:** Geo-/UI-Politur, kompakte VVO-Ziele, festes 5-s-Paging, atomare Config-Saves und ESP8266-Radar-TLS-Stabilisierung

- **v0.4.6:** europäischer Pixelzeichensatz, Herzen, Zwischenstopps, Tracker-Footer, responsive Zeitfelder und Dokumentation

## Basis

Das Projekt entstand auf Basis von [`giovi321/smalltv-mod`](https://github.com/giovi321/smalltv-mod), insbesondere dessen Hardware- und Displayansteuerung für den GeekMagic SmallTV. Das Ursprungsprojekt steht ebenfalls unter der **WTFPL**.

## Hinweis

Dies ist ein privates Community-/Experimentierprojekt und keine offizielle Firmware von GeekMagic, VVO, DVB, Open-Meteo, adsb.lol, adsbdb, aviationstack, Leaflet, Nominatim oder OpenStreetMap. Marken, Daten und Dienste gehören ihren jeweiligen Rechteinhabern.
