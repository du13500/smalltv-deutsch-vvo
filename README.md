# SmallTV Deutsch/VVO

[![Build SmallTV DD v0.4.9](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v049.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v049.yml)

> 🟨 **v0.4.9 · Entwicklungsbuild**  
> Für den **GeekMagic SmallTV-Ultra** · Deutschsprachige Oberfläche · Hardwaretest dieser Version noch offen

**Ein kleiner WLAN-Würfel für Abfahrten, Wetter, Flugradar, Uhr und Countdowns.** Nach der Einrichtung benötigt das Gerät nur USB-Strom und WLAN. Ein eigener Server oder Raspberry Pi ist nicht erforderlich.

Entwickelt von **du13500**, auf Grundlage von **smalltv-mod**. Der Flug-Tracker ist eine zusätzliche, optionale Ansicht für einen bestimmten Flug.

## Entwicklungsstand

🟩 = im Praxistest bestätigt · 🟨 = neue Änderungen oder Tests offen · ⬜ = noch nicht vollständig getestet

| Bereich | Stand |
| --- | --- |
| Einrichtung, Zeitzone, Nachtmodus und Zeichensatz | 🟩 In v0.4.7 auf echter Hardware bestätigt |
| Teilaktualisierung bei Radar, Abfahrten und Countdowns | 🟩 In v0.4.8 auf echter Hardware bestätigt |
| Wetter und VVO: Abrufe und Fehlererholung | 🟨 Änderungen in v0.4.9; Hardwaretest offen |
| Flugradar | 🟨 Getrennte Abrufe und Diagnose in v0.4.9; zuverlässiger Dauerbetrieb noch offen |
| Diagnose-Log und Speichersperre | 🟨 Automatisiert geprüft; Hardwaretest offen |
| Wetterbilder | 🟨 Verschiedene Wetterlagen getestet; vollständiger Praxistest offen |
| Flug-Tracker mit aviationstack | ⬜ Vollständiger Hardwaretest mit gültigem API-Schlüssel offen |

Die Textampel beschreibt den Praxistest der Funktionen. Das Build-Badge oben zeigt separat das Ergebnis des GitHub-Workflows.

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

Solange nach einer Kartenauswahl der Ortsname ermittelt wird, bleibt die Speichertaste deaktiviert. Ein Fehler bei der Namensabfrage verhindert anschließend nicht das Speichern gültiger Koordinaten.

Der **Hostname** ist der Gerätename im Netzwerk. Ein leeres WLAN-Passwortfeld behält das bereits gespeicherte Passwort.

## Firmware über GitHub bauen

Dieser Entwicklungsstand wird als Versionsarchiv geliefert:

- `smalltv-deutsch-vvo-v0.4.9.zip` unverändert in das Hauptverzeichnis des Repositorys hochladen.
- `build-v049.yml` als `.github/workflows/build-v049.yml` ablegen.
- Die README aus der separaten README-ZIP bei Bedarf als `README.md` im Hauptverzeichnis ersetzen.
- Unter **Actions** den Workflow **Build SmallTV DD v0.4.9** öffnen und ausführen, falls er nicht bereits automatisch gestartet wurde.
- Nach erfolgreichem Build das Artefakt **smalltv-dd-v0.4.9** herunterladen und entpacken. Es enthält die Firmware-BIN und den Buildlog.

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

## Versionsgeschichte

### v0.4.9

- **Diagnose im Webinterface:** Im Reiter System lassen sich die letzten 24 Ereignisse anzeigen und als Textdatei herunterladen. Abrufe werden nach Position, Route, Metadaten, Airline, Wetter, VVO und Ortsabfrage unterschieden. Aufgeführt werden HTTP-Ergebnis, Zeit seit Start, Dauer, freier Heap und größter Speicherblock. Passwörter, API-Schlüssel, Suchtexte und vollständige URLs werden nicht protokolliert.
- **Neustart-Hinweise:** Reset-Grund und Reset-Information werden angezeigt. Ein kleiner RTC-Marker hält den letzten protokollierten Arbeitsschritt des vorherigen Starts fest, soweit er einen Neustart übersteht. Er liefert einen Hinweis, keinen Beweis für die Absturzursache. Der normale Ereignislog beginnt nach jedem Neustart neu.
- **Radar-Abrufe:** Positionsdaten werden vor den Zusatzabfragen übernommen. Route, Metadaten und Airline werden getrennt und nacheinander ergänzt. Der Positionspfad verwendet nur zwischengespeicherte Zusatzdaten. Bei wechselnden Flugzeugen liegen mindestens 60 Sekunden zwischen neuen Ergänzungszyklen; fehlende Routen werden frühestens nach zwei Minuten erneut geprüft. Gültige Routen bleiben höchstens fünf Minuten verwendbar und werden weiterhin gegen die Position geprüft.
- **Hintergrundabfragen:** Abfahrten, Wetter, Radar und Tracker werden im Wechsel bedient. Zwischen Hintergrundaufgaben liegt eine kurze Pause; das Webinterface kann dazwischen Anfragen bearbeiten. Eine einzelne HTTPS-Abfrage kann weiterhin einige Sekunden blockieren.
- **Erholung bei Fehlern:** Wetter und VVO wiederholen fehlgeschlagene Abrufe nach 15, 30, 60 und höchstens 120 Sekunden. Nach Erfolg gilt wieder das eingestellte Intervall. Bei HTTP 429/503 werden zusätzlich Anbieterpausen berücksichtigt. TLS-Abfragen für Wetter, VVO, Ortsnamen und Tracker prüfen den verfügbaren Speicher vor dem Verbindungsaufbau.
- **Einstellungen speichern:** Unveränderte Programme behalten ihre Daten und Abrufzeiten. Neue Standorte, Haltestellen, Datenfilter oder neu aktivierte Programme lösen den jeweiligen Neuabruf aus. Während eine Karten-Ortsabfrage wartet oder läuft, ist die Speichertaste sichtbar deaktiviert. Nach Erfolg, fehlendem Ortsnamen oder einem Fehler wird sie wieder freigegeben.
- **Antwortprüfung:** Eine Radar-Antwort ohne Flugzeugliste oder mit ungültigen Positionskoordinaten gilt als Abruffehler. Eine ausdrücklich leere Liste bleibt ein erfolgreicher Abruf ohne Flugzeuge.

### Ältere Versionen

| Version | Wichtigste Neuerungen |
| --- | --- |
| **v0.4.8** | Anbieterpausen, positionsgeprüfte Radar-Routen, Online-Ergänzung von Airline und Typ, Bodenstatus GND, Kartenkorrekturen und Teilaktualisierungen in weiteren Programmen |
| **v0.4.7** | Einstellbare Zeitzonen, griechische und kyrillische Zeichen, differenziertere Wetterbilder sowie Verbesserungen bei Radar-Abrufen und der Anzeige veralteter Daten |
| **v0.4.6** | Erweiterter lateinischer Zeichensatz, mehrteilige Flugrouten, Tracker-Footer und Verbesserungen am Webinterface |
| **v0.4.5** | Kompakte Abfahrtsziele, Verbesserungen an Karten- und Standortauswahl, Speichern der Einstellungen und Radar-Verbindungen |
| **v0.4.4** | Bis zu zehn Abfahrten mit Seitenwechsel, Temperatureinheiten, Uhr im Footer und robustere Radarfehlerbehandlung |
| **v0.4.3** | Überarbeitung der öffentlichen Projektangaben |
| **v0.4.2** | Karten-Picker, Rückmeldungen im Webinterface und ruhigere Uhr- und Countdown-Anzeige |
| **v0.4.1** | Programmrotation, WLAN-Scan, Ortssuche, deutsche Sonderzeichen und Verkehrsmittelfilter |
| **v0.4** | Flug-Tracker und vier speicherbare Countdowns |
| **v0.3** | Live-Wetter, Flugradar und weitere Hauptansichten |
| **v0.2** | VVO-Haltestellensuche und Abfahrtsmonitor |
| **v0.1** | WLAN-Einrichtung, Webinterface, automatische Zeitabfrage und Firmware-Updates |

## Diagnose und Fehlerberichte

Unter **System → Diagnose** zeigt **Log laden** die letzten Ereignisse; **Log herunterladen** speichert sie als Textdatei. Nach einem Radar-Aussetzer oder einer fehlgeschlagenen Wetter-/VVO-Abfrage den Log möglichst direkt laden. Nach einem Reboot ist der vorherige vollständige Log verloren; Reset-Information und ein gültiger RTC-Marker können Hinweise erhalten. Der Log wird im RAM geführt und nicht fortlaufend in den Flash geschrieben.

## Lizenz

Hilfreich sind Firmwareversion, Gerät, betroffene Ansicht, Schritte zur Reproduktion sowie ein Foto oder der bereinigte Systemstatus. Keine WLAN-Passwörter oder API-Schlüssel veröffentlichen.

Projektcode: **WTFPL**, entsprechend der mitgelieferten Lizenzdatei. Für Bibliotheken und externe Daten gelten die jeweiligen Bedingungen. Kartendaten: © OpenStreetMap-Mitwirkende. Dieses Projekt ist unabhängig von GeekMagic, VVO und den genannten Datenanbietern.
