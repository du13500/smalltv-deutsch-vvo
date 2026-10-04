# SmallTV Deutsch / VVO

[![Build SmallTV DD v0.4.1](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v041.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v041.yml)

Eigene deutschsprachige Firmware für den **GeekMagic SmallTV-Ultra** mit ESP8266 und 240 x 240 Pixel ST7789-Display.

Aus dem kleinen WLAN-Display wird ein autonomer Informationswürfel für **VVO-Abfahrten, Wetter, Flugradar, Uhr, Flug-Tracking und Countdowns**. Nach der Einrichtung benötigt der SmallTV nur **USB-Strom und WLAN**. Ein Raspberry Pi, Home Assistant oder anderer Server ist nicht erforderlich.

> **Aktueller Stand:** v0.4 wurde erfolgreich auf echter SmallTV-Ultra-Hardware getestet. v0.4.1 ist die aktuelle Entwicklungsfassung und erweitert vor allem Bedienung und Alltagstauglichkeit. Der GitHub-Actions-Build und der anschließende Hardwaretest von v0.4.1 stehen zum Zeitpunkt dieses README noch aus.

## Unterstützte Hardware

Der aktuelle Build ist für den **GeekMagic SmallTV-Ultra** ausgelegt:

- ESP8266 / ESP-12F
- 4 MB Flash
- 240 x 240 Pixel ST7789
- SPI Mode 3
- LittleFS
- OTA-Firmwareupdates über das Webinterface

Andere SmallTV-Varianten sind derzeit **nicht getestet**.

## Programme

### Abfahrtsmonitor

Live-Abfahrten des Verkehrsverbunds Oberelbe direkt auf dem Würfel.

- Haltestellensuche per Freitext im Webinterface
- Auswahl des gewünschten Suchtreffers
- Echtzeit-Abfahrten und Ausfälle
- Anzahl der angezeigten Abfahrten konfigurierbar
- Verkehrsmittel einzeln auswählbar:
  - Zug
  - S-Bahn
  - Straßenbahn
  - Bus
  - Seil- und Schwebebahn
  - Fähre
  - AST / Rufbus
- adaptive Darstellung längerer Linienbezeichnungen wie `RE50`, `IC 2045` oder `ICE 1650`

### Wetter

Aktuelles Wetter über Open-Meteo.

- Ortssuche statt manueller Eingabe von Koordinaten
- optionaler eigener Anzeigename, zum Beispiel `ZUHAUSE`
- Browser-Standort als zusätzliche Eingabemöglichkeit, sofern der verwendete Browser dies auf der lokalen Seite zulässt
- aktuelle Temperatur
- Wetterzustand
- Tageshöchst- und Tiefsttemperatur
- Regenwahrscheinlichkeit
- Sonnenaufgang und Sonnenuntergang mit eigenen Pixel-Symbolen
- optional farbige Darstellung der großen aktuellen Temperatur
- echte deutsche Umlaute und `ß` im Pixel-Font

### Flugradar

Zeigt das nächstgelegene empfangene Flugzeug zu einem frei wählbaren Beobachtungspunkt.

- eigener Standort unabhängig vom Wetterprogramm
- Ortssuche im Webinterface
- frei einstellbarer Suchradius
- Callsign
- Airline, soweit aus dem Callsign ableitbar
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
- in v0.4.1 unabhängig vom Datenabruf minütlich aktualisiert

### Flug-Tracker

Detailansicht für einen gezielt eingetragenen Flug.

- Eingabe einer Flugnummer im Webinterface
- Flugstatus und Flugplandaten über Aviationstack
- zusätzliche Live-Telemetrie über ADS-B, soweit verfügbar
- Route
- geplante und tatsächliche Zeiten
- Flugdauer
- Höhe und Geschwindigkeit

Für Aviationstack ist ein **eigener API-Key** erforderlich. Kontingente und Nutzungsbedingungen des gewählten Tarifs sind zu beachten. Die Firmware fragt den Dienst bewusst sparsam und nur bei aktivem Flug-Tracker ab.

### Countdown 1 bis 4

Vier voneinander unabhängige und dauerhaft gespeicherte Countdown-Slots.

- optionaler Titel
- eigene Farbe für Titel und Countdown
- 7-Segment-Darstellung im Retro-Digitalstil
- drei manuell wählbare Modi:
  - Kalendertage
  - Stunden : Minuten : Sekunden
  - Minuten : Sekunden
- `-` vor dem Zielzeitpunkt
- kein sichtbares Vorzeichen bei exakt Null
- `+` nach dem Zielzeitpunkt
- feste Vorzeichenspalte gegen horizontales Springen
- absolute Zielzeitpunkte bleiben auch nach Stromverlust erhalten

Die Countdowns laufen unabhängig voneinander weiter, auch wenn gerade ein anderes Programm angezeigt wird.

## Anzeigerotation

Ab v0.4.1 muss nicht mehr genau ein Programm dauerhaft ausgewählt werden.

Im Webinterface können mehrere Ansichten per Checkbox aktiviert und jeweils mit einer eigenen Anzeigedauer versehen werden. Die Firmware rotiert anschließend automatisch durch die gewählten Programme.

Die **Anzeigedauer ist von der Datenaktualisierung getrennt**. Wetterdaten werden beispielsweise nicht bei jedem erneuten Einblenden der Wetteransicht neu vom Server geladen.

## Webinterface

Die komplette Konfiguration erfolgt lokal im Browser. Aktuell gibt es Bereiche für:

- Anzeige und Rotation
- Abfahrten
- Wetter
- Flugradar
- Flug-Tracker
- Countdown
- System und WLAN

v0.4.1 ergänzt unter anderem:

- WLAN-Scan mit anklickbaren SSIDs
- Sortierung nach Signalstärke
- Signalbalken und RSSI in dBm
- deutsche und verständlichere Netzwerkfehlermeldungen
- Ortssuche für Wetter und Flugradar
- bequemere Konfiguration ohne manuelles Heraussuchen von Längen- und Breitengraden

## Ersteinrichtung

Nach einem frischen Start ohne gespeicherte WLAN-Daten öffnet die Firmware den Access Point:

```text
SmallTV-Setup
```

Anschließend im Browser öffnen:

```text
http://192.168.4.1/
```

Dort WLAN und die gewünschten Programme konfigurieren. Nach dem Speichern startet der SmallTV neu und verbindet sich mit dem Heimnetz.

## Installation

### Update von v0.4 auf v0.4.1

Wer bereits v0.4 oder eine neuere Version dieser Firmware verwendet, benötigt den Ultra-Loader **nicht erneut**.

1. Den erfolgreichen GitHub-Actions-Build `Build SmallTV DD v0.4.1` öffnen.
2. Das Artefakt `smalltv-dd-v0.4.1` herunterladen und entpacken.
3. Im Webinterface des SmallTV `Firmware-Update` öffnen oder direkt `http://<IP-des-SmallTV>/update` aufrufen.
4. **Nur** `smalltv-dd-v0.4.1-firmware.bin` als Firmware hochladen.
5. Den SmallTV während des Updates nicht vom Strom trennen.

Die vorhandene `/config.json` wird weiterverwendet. Neue v0.4.1-Einstellungen erhalten kompatible Standardwerte.

### Erstinstallation auf originaler SmallTV-Ultra-Firmware

Die originale Ultra-Firmware verwendet ein Partitionslayout, bei dem die vollständige Custom-Firmware nicht direkt in den verfügbaren OTA-Slot passt. Deshalb erfolgt die Erstinstallation zweistufig über einen kleinen Loader.

> **Wichtig:** Die vollständige SmallTV-DD-Firmware nicht direkt über den Updater der originalen Ultra-Firmware hochladen.

1. Den Loader über den Workflow `Build SmallTV Ultra Loader` bauen bzw. dessen Artefakt herunterladen.
2. In der originalen SmallTV-Firmware `/update` öffnen.
3. Dort **nur** `smalltv-ultra-loader.bin` als Firmware hochladen.
4. Nach dem Neustart mit dem offenen WLAN `SmallTV-Loader` verbinden.
5. `http://192.168.4.1/update` öffnen.
6. Dort die aktuelle `smalltv-dd-v0.4.1-firmware.bin` hochladen.
7. Nach dem Neustart mit `SmallTV-Setup` verbinden und `http://192.168.4.1/` öffnen.

Der Loader besitzt absichtlich keine eigene Displayausgabe. Ein schwarzes oder unverändertes Display während dieser Zwischenstufe ist daher nicht automatisch ein Fehler.

## Build

Der v0.4.1-Quellstand liegt im Release-Quellpaket:

```text
smalltv-deutsch-vvo-v0.4.1.zip
```

Nach dem Entpacken im enthaltenen Projektordner:

```bash
pio run -e smalltv_ultra
```

Die erzeugte Firmware liegt anschließend unter:

```text
.pio/build/smalltv_ultra/firmware.bin
```

Verwendete Hauptbibliotheken:

- ArduinoJson
- GFX Library for Arduino
- ESP8266 Arduino Core

Der GitHub-Actions-Workflow `.github/workflows/build-v041.yml` führt denselben Build automatisiert aus und stellt bei Erfolg `smalltv-dd-v0.4.1-firmware.bin`, Build-Log und SHA-256-Prüfsumme als Artefakt bereit.

## Datenquellen und externe Dienste

Die Firmware arbeitet ohne eigenen Backend-Server, greift für Live-Daten aber auf externe Dienste zu:

| Funktion | Quelle |
| --- | --- |
| VVO-Abfahrten | VVO-WebAPI |
| Wetter | Open-Meteo |
| Ortssuche | Open-Meteo Geocoding |
| Flugradar | adsb.lol |
| Flugrouten-Fallback | adsbdb |
| Flug-Tracker | Aviationstack + adsb.lol |
| Uhrzeit | NTP |

Verfügbarkeit, Limits und Nutzungsbedingungen externer Dienste können sich unabhängig von diesem Projekt ändern.

## Datenschutz

Die Konfiguration liegt lokal auf dem SmallTV. WLAN-Passwort, ausgewählte Orte, Countdowns und gegebenenfalls der Aviationstack-Key werden auf dem Gerät gespeichert.

Das Webinterface ist für das lokale Netzwerk gedacht. Die Firmware betreibt keinen eigenen Cloud-Dienst.

## Bekannte Grenzen

- Flugrouten lassen sich nicht für jedes ADS-B-Callsign zuverlässig auflösen. Bei fehlenden Daten wird bewusst `N/A` angezeigt.
- Die Browserfunktion `Aktuelle Position` ist vom Browser und dessen Sicherheitsregeln abhängig und kann auf einer lokalen HTTP-Seite blockiert werden.
- Die Zuverlässigkeit der Live-Funktionen hängt von den jeweiligen externen APIs und der WLAN-Verbindung ab.
- v0.4.1 muss nach erfolgreichem Build noch vollständig auf echter Hardware regressionsgetestet werden.

## Späterer Ideentopf

Nicht Teil von v0.4.1, aber als mögliche spätere Erweiterungen vorgemerkt:

- grafisches Regenradar für einen festen Ort
- freier Textmodus
- Listenmodus für Einkauf, To-dos oder Packlisten
- eigener Bahnhofsmonitor mit Zug, Ziel, Zeit, Verspätung und Gleis

## Projektgeschichte

- **v0.1:** deutsches Webinterface, WLAN, NTP, OTA und erste Ansichten
- **v0.2:** echte VVO-Haltestellensuche und Live-Abfahrten
- **v0.3:** Open-Meteo, adsb.lol und vollständig live arbeitende Hauptansichten
- **v0.4:** Flug-Tracker und vier persistente Countdowns; erster umfassender Hardwaretest auf einem echten SmallTV-Ultra erfolgreich
- **v0.4.1:** UX-Politur, Anzeigerotation, WLAN-Scan, Ortssuchen, deutsche Glyphen, VVO-Verkehrsmittelfilter und robusteres Flugradar

## Basis und Lizenz

Das Projekt entstand auf Basis von [`giovi321/smalltv-mod`](https://github.com/giovi321/smalltv-mod), insbesondere dessen Hardware- und Displayansteuerung für den GeekMagic SmallTV. Das Ursprungsprojekt steht unter der **WTFPL**.

Die Lizenz dieses Repositories befindet sich in [`LICENSE`](LICENSE).

## Hinweis

Dies ist ein privates Community-/Experimentierprojekt und keine offizielle Firmware von GeekMagic, VVO, DVB, Open-Meteo, adsb.lol, adsbdb oder Aviationstack. Die genannten Marken und Dienste gehören ihren jeweiligen Rechteinhabern.
