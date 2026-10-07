# SmallTV Deutsch/VVO

> 🟨 **v0.4.8 · Entwicklungsbuild**  
> Für den **GeekMagic SmallTV-Ultra** · Deutschsprachige Oberfläche · Hardwaretest dieser Version noch offen

**Ein kleiner WLAN-Würfel für Abfahrten, Wetter, Flugradar, Uhr und Countdowns.** Nach der Einrichtung benötigt das Gerät nur USB-Strom und WLAN. Ein eigener Server oder Raspberry Pi ist nicht erforderlich.

Entwickelt von **du13500**, auf Grundlage von **smalltv-mod**. Der Flug-Tracker ist eine zusätzliche, optionale Ansicht für einen bestimmten Flug.

## Entwicklungsstand

🟩 = im Praxistest bestätigt · 🟨 = neue Änderungen oder Tests offen · ⬜ = noch nicht vollständig getestet

| Bereich | Stand |
| --- | --- |
| Einrichtung, Zeitzone, Nachtmodus und Zeichensatz | 🟩 In v0.4.7 auf echter Hardware bestätigt |
| Abfahrten, Wetter, Uhr und Countdowns | 🟨 Gezieltes Neuzeichnen in v0.4.8 erweitert; erneuter Hardwaretest nötig |
| Flugradar | 🟨 Abrufpausen, Zusatzdaten und Routenprüfung in v0.4.8 überarbeitet |
| Wetterbilder | 🟨 Verschiedene Wetterlagen getestet; vollständiger Praxistest offen |
| Flug-Tracker mit aviationstack | ⬜ Vollständiger Hardwaretest mit gültigem API-Schlüssel offen |

Die Ampel beschreibt den Entwicklungsstand. Sie ist keine Anzeige eines automatisch ermittelten GitHub-Buildstatus.

## Unterstütztes Gerät

**GeekMagic SmallTV-Ultra mit ESP8266 / ESP-12F, 4 MB Flash und ST7789-Display mit 240 × 240 Pixeln.** Andere SmallTV-Modelle sind nicht getestet. Die Firmware ist nicht für ESP32-Geräte bestimmt.

## Programme

| Programm | Funktionen |
| --- | --- |
| **Abfahrten** | VVO-Haltestellensuche, Abfahrtsminuten, Verspätungen und Ausfälle; Verkehrsmittelfilter; bis zu zehn Einträge auf zwei Seiten |
| **Wetter** | Aktuelle Temperatur, Wetterbild, Tageshöchst- und Tiefsttemperatur, Regenwahrscheinlichkeit sowie Sonnenauf- und -untergang; Celsius oder Fahrenheit |
| **Flugradar** | Nächstgelegenes gemeldetes Flugzeug im gewählten Radius: Callsign, Airline, Route, Typ, Registrierung, Entfernung und Höhe |
| **Uhr** | Uhrzeit und Datum mit wählbarer Zeitzone und automatischer Zeitabfrage |
| **Flug-Tracker** | Status, Flughäfen, Flugzeiten und verfügbare Live-Daten eines eingetragenen Fluges; aviationstack-Schlüssel erforderlich |
| **Countdown 1 bis 4** | Vier unabhängige Timer mit eigenem Titel und Farben; Kalendertage, Stunden/Minuten/Sekunden oder Minuten/Sekunden |

Programme können einzeln oder im automatischen Wechsel angezeigt werden. Die Anzeigedauer lässt sich je Programm einstellen. Ein Nachtmodus reduziert die Helligkeit in einem frei wählbaren Zeitfenster.

Wetterbilder unterscheiden Tag und Nacht, Bewölkung, Nebel, Niederschlagsart und Intensität. Unterstützt werden erweiterte lateinische Zeichen sowie griechische und kyrillische Buchstaben. Für nicht darstellbare Ortsnamen kann ein eigener Anzeigename eingetragen werden.

## Einrichtung

1. Die zum SmallTV-Ultra passende Firmware installieren. Der Erstflashweg hängt von der vorhandenen Firmware ab; der USB-Stromanschluss allein ist kein zugesicherter Programmierzugang.
2. Beim ersten Start ohne gespeicherte WLAN-Verbindung mit dem WLAN **SmallTV-Setup** verbinden und im Browser **192.168.4.1** öffnen.
3. Unter **System** das WLAN auswählen und das Passwort eintragen. Einstellungen speichern.
4. Nach dem Neustart das Webinterface über die auf dem Display angezeigte IP-Adresse öffnen. Je nach Netzwerk funktioniert auch der Hostname mit der Endung `.local`.
5. Unter **Anzeige** Programme, Wechselzeiten, Zeitzone, Helligkeit und Nachtmodus einstellen.
6. In den jeweiligen Programmreitern Haltestelle, Standorte oder Timer konfigurieren und speichern.

Wetter und Flugradar haben unabhängige Standorte. Sie lassen sich über die Ortssuche, die Karte oder manuell eingegebene Koordinaten festlegen. Ein Kartenpunkt bleibt auch dann verwendbar, wenn kein Ortsname verfügbar ist. Ein optionaler Anzeigename ersetzt den Namen auf dem Display.

Der **Hostname** ist der Gerätename im Netzwerk. Ein leeres WLAN-Passwortfeld behält das bereits gespeicherte Passwort.

## Firmware über GitHub bauen

Dieser Entwicklungsstand wird als Versionsarchiv geliefert:

- `smalltv-deutsch-vvo-v0.4.8.zip` unverändert in das Hauptverzeichnis des Repositorys hochladen.
- `build-v048.yml` als `.github/workflows/build-v048.yml` ablegen.
- Die README aus der separaten README-ZIP bei Bedarf als `README.md` im Hauptverzeichnis ersetzen.
- Unter **Actions** den Workflow **Build SmallTV DD v0.4.8** öffnen und ausführen, falls er nicht bereits automatisch gestartet wurde.
- Nach erfolgreichem Build das Artefakt **smalltv-dd-v0.4.8** herunterladen und entpacken. Es enthält die Firmware-BIN und den Buildlog.

Der Workflow baut den Inhalt des Versionsarchivs. Änderungen an anderen, älteren Quelldateien im Repository werden dadurch nicht automatisch Teil dieses Builds.

## Updates

Im Webinterface unter **System → Firmware-Update** die passende `.bin` auswählen. Während des Updates die Stromversorgung aufrechterhalten. Die Einstellungen werden im Gerät gespeichert und bleiben bei einem normalen Firmwareupdate erhalten.

## Datenquellen und Grenzen

- **VVO** liefert Abfahrten, **Open-Meteo** das Wetter.
- **OpenStreetMap / Nominatim** liefert Kartendaten und Ortsnamen. Gewässernamen werden übernommen, soweit die Ortsabfrage sie liefert; eine weltweite Gewässerabdeckung wird nicht zugesichert.
- **adsb.lol** liefert Radarpositionen, **adsb.lol und adsbdb** ergänzen Routen. **adsbdb** ergänzt verfügbare Airline- und Flugzeugtypenamen; lokale Bezeichnungen dienen als Rückfall.
- **aviationstack** liefert die Flugplandaten des optionalen Flug-Trackers. Dafür ist ein eigener Schlüssel erforderlich; Tarif und Abrufkontingent bestimmen die Nutzungsmöglichkeiten.

Radar-Routen stammen aus Zusatzdatenbanken und können trotz Positionsprüfung falsch oder unvollständig sein. Unpassende oder nicht verfügbare Routen werden nicht durch vermutete Flughäfen ersetzt. `N/A` bedeutet fehlende Information, `GND` einen ausdrücklich gemeldeten Bodenstatus. Höhenangaben stammen aus den verfügbaren Flugdaten und sind keine Höhe über dem Gelände.

Externe Dienste können Abrufe begrenzen. Bei HTTP 429 pausieren die betroffenen ADS-B-Abfragen automatisch. Während einer Fehlerphase zeigt das Radar kurzzeitig **LETZTER STAND**; nach 30 Sekunden ohne neue Positionsdaten wird ein als veraltet markiertes Flugzeug entfernt. Ein eingestellter Aktualisierungstakt garantiert keine neuen Daten vom Anbieter.

## Neu in v0.4.8

- Gemeinsame Abrufpausen bei Rate-Limits für Radar und Tracker.
- Routenprüfung gegen die aktuelle Position und zeitlich begrenzter Zusatzdaten-Cache.
- Online-Ergänzung von Airline- und Typnamen mit kompakter Typdarstellung.
- `GND` für ausdrücklich gemeldete Flugzeuge am Boden.
- Kartenkoordinaten korrekt über den Kartenrand hinweg; verspätete Ortsantworten überschreiben keinen neueren Kartenpunkt.
- Gewässernamen aus der Ortsabfrage statt grober lokaler Gebietsrechtecke.
- Gezieltes Aktualisieren geänderter Anzeigeelemente in weiteren Programmen.

## Fehlerberichte und Lizenz

Hilfreich sind Firmwareversion, Gerät, betroffene Ansicht, Schritte zur Reproduktion sowie ein Foto oder der bereinigte Systemstatus. Keine WLAN-Passwörter oder API-Schlüssel veröffentlichen.

Projektcode: **WTFPL**, entsprechend der mitgelieferten Lizenzdatei. Für Bibliotheken und externe Daten gelten die jeweiligen Bedingungen. Kartendaten: © OpenStreetMap-Mitwirkende. Dieses Projekt ist unabhängig von GeekMagic, VVO und den genannten Datenanbietern.
