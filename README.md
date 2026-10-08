# SmallTV Deutsch/VVO

[![Build SmallTV DD v0.4.13](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v0413.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v0413.yml)

> 🟨 **v0.4.13 · Entwicklungsbuild**  
> Für den **GeekMagic SmallTV-Ultra** · Deutschsprachige Oberfläche · Hardwaretest dieser Version noch offen

**Ein kleiner WLAN-Würfel für Abfahrten, Wetter, Flugradar, Uhr und Countdowns.** Nach der Einrichtung benötigt das Gerät nur USB-Strom und WLAN. Ein eigener Server oder Raspberry Pi ist nicht erforderlich.

Entwickelt von **du13500**, auf Grundlage von **smalltv-mod**. Der Flug-Tracker ist eine zusätzliche, optionale Ansicht für einen bestimmten Flug.

## v0.4.13

- Geo prüft den freien Heap und den größten Speicherblock erneut unmittelbar vor dem HTTP-Abruf, nach Vorbereitung von LittleFS-Datei, URL, HTTP-Client und Headern. Eine nicht ausreichende Reserve führt zum kontrollierten Abbruch vor dem Verbindungsaufbau.
- Lokale URL-Kopien und die interne Suchtextkopie werden nach Übernahme durch HTTPClient freigegeben. Die Suche fordert keine ungenutzten Addressdetails mehr an; acht Treffer und deutsche Namensvarianten bleiben erhalten. Die Kartenauflösung fordert ihre benötigten Adressdaten weiterhin an.
- Feste Diagnosebezeichnungen liegen im Flash. Der statische RAM-Bedarf sinkt gegenüber v0.4.12 um 484 Bytes. Das ist ein begrenzter Gewinn, keine Garantie für erfolgreiche Ortsabfragen in jedem Betriebszustand.
- Neue Geo-Marker: Geo:Vorbereitung, Geo:Verbinden, Geo:Empfang und Geo:TLS-frei. Normale Fortschrittsmarker werden nicht als letzter Fehler gespeichert; sie bleiben als RTC-Marker auswertbar.
- Der Empfangsschutz aus v0.4.12 bleibt bestehen: mindestens 3 KiB Rest-Heap, höchstens 48 KiB Antwort, begrenzte Schreibschritte und JSON erst nach TLS-Freigabe.

Hardwaretest offen. Der v0.4.12-Absturz ist durch die passende ELF als ungültiger Schreibzugriff innerhalb der ROM-Speicherkopierfunktion eingegrenzt, aber nicht vollständig erklärt. Wetter-/Radar-Abrufe, Anbieterpausen, Typanzeige und Programme bleiben erhalten.

## Entwicklungsstand

🟩 = im Praxistest bestätigt · 🟨 = neue Änderungen oder Tests offen · ⬜ = noch nicht vollständig getestet

| Bereich | Stand |
| --- | --- |
| Einrichtung, Zeitzone, Nachtmodus und Zeichensatz | 🟩 In v0.4.7 auf echter Hardware bestätigt |
| Teilaktualisierung bei Radar, Abfahrten und Countdowns | 🟩 In v0.4.8 auf echter Hardware bestätigt |
| Wetter und VVO: Abrufe und Fehlererholung | 🟩 VVO einzeln und Programmwechsel in v0.4.10 stabil getestet; Wetter-Ortssuche v0.4.13 noch offen |
| Flugradar | 🟩 v0.4.10 über 2 Stunden 49 Minuten ohne Neustart; 🟨 externe Routenqualität und neue Typprüfung weiter offen |
| Diagnose-Log und Speichersperre | 🟩 Schutz in v0.4.10 praktisch beobachtet; 🟨 Kopieren und Geo-Puffer in v0.4.13 noch auf Hardware zu testen |
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

Programme können einzeln oder im automatischen Wechsel angezeigt werden. Die Anzeigedauer lässt sich je Programm einstellen. Ein Nachtmodus reduziert die Helligkeit in einem frei wählbaren Zeitfenster. Die Mindesthelligkeit beträgt tagsüber und nachts 1 %.

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

- `smalltv-deutsch-vvo-v0.4.13.zip` unverändert in das Hauptverzeichnis des Repositorys hochladen.
- `build-v0413.yml` als `.github/workflows/build-v0413.yml` ablegen.
- Die separate README-ZIP entpacken und deren README sowie Lizenzunterlagen im Hauptverzeichnis ersetzen. Sie enthält auch `LICENSE`, `LICENSES`, `THIRD_PARTY_NOTICES.md` und `THIRD_PARTY_SERVICES.md`.
- Unter **Actions** den Workflow **Build SmallTV DD v0.4.13** öffnen und ausführen, falls er nicht bereits automatisch gestartet wurde.
- Nach erfolgreichem Build **smalltv-dd-v0.4.13-firmware** herunterladen und die BIN flashen. **smalltv-dd-v0.4.13-begleitpaket** enthält separat Buildlog, ELF, Lizenztexte sowie Quellen und Buildobjekte.

Der Workflow baut den Inhalt des Versionsarchivs. Änderungen an anderen, älteren Quelldateien im Repository werden dadurch nicht automatisch Teil dieses Builds.

## Updates

Im Webinterface unter **System → Firmware-Update** die passende `.bin` auswählen. Während des Updates die Stromversorgung aufrechterhalten. Die Einstellungen werden im Gerät gespeichert und bleiben bei einem normalen Firmwareupdate erhalten.

## Datenquellen und Grenzen

- **VVO** liefert Abfahrten, **Open-Meteo** das Wetter.
- **OpenStreetMap / Nominatim** liefert Kartendaten und Ortsnamen. Gewässernamen werden übernommen, soweit die Ortsabfrage sie liefert; eine weltweite Gewässerabdeckung wird nicht zugesichert.
- **adsb.lol** liefert Radarpositionen, **adsb.lol und adsbdb** ergänzen Routen. **adsbdb** ergänzt verfügbare Airline- und Flugzeugtypenamen; lokale Bezeichnungen dienen als Rückfall.
- **aviationstack** liefert die Flugplandaten des optionalen Flug-Trackers. Dafür ist ein eigener Schlüssel erforderlich; Tarif und Abrufkontingent bestimmen die Nutzungsmöglichkeiten.

Radar-Routen stammen aus Zusatzdatenbanken und können trotz Positionsprüfung falsch oder unvollständig sein. Unpassende oder nicht verfügbare Routen werden nicht durch vermutete Flughäfen ersetzt. `N/A` bedeutet fehlende Information, `GND` einen ausdrücklich gemeldeten Bodenstatus. Höhenangaben stammen aus den verfügbaren Flugdaten und sind keine Höhe über dem Gelände.

Externe Dienste können Abrufe begrenzen. Bei HTTP 429 pausieren die betroffenen ADS-B-Abfragen automatisch. Nach einer Drosselung gilt mindestens 15 Sekunden Abstand zwischen Positionsabrufen, bei wiederholter Drosselung mindestens 20 Sekunden. Der eingestellte Wert bleibt gespeichert; das höhere Mindestintervall gilt bis zehn Minuten ohne weitere Drosselung vergangen sind. Währenddessen entfallen zusätzliche adsb.lol-Routenabrufe. Das Webinterface zeigt das aktive Mindestintervall. Während einer Fehlerphase zeigt das Radar kurzzeitig **LETZTER STAND**; nach 30 Sekunden ohne neue Positionsdaten wird ein als veraltet markiertes Flugzeug entfernt. Ein eingestellter Aktualisierungstakt garantiert keine neuen Daten vom Anbieter.

## Versionsgeschichte

### v0.4.10

- **Speicher bei Netzwerkabfragen:** JSON-Speicher wächst in kleineren Blöcken. Ein eigener Allocator hält 3 KiB freien Heap zurück; bei fehlendem Speicher wird die Antwort verworfen. Nicht mehr benötigte Anfrage-, Filter- und TLS-Daten werden vor der weiteren Auswertung freigegeben. Auch ein unvollständig aufgebauter JSON-Filter wird erkannt. Das soll die in v0.4.9 beobachteten Abstürze bei Karten- und VVO-Abfragen vermeiden; der Hardwaretest steht aus.
- **TLS-Puffer:** Wetter, VVO, Ortsabfragen und Radar-Metadaten prüfen einmal pro Start, ob der jeweilige Server kleinere TLS-Empfangsblöcke unterstützt. Nur bei bestätigter Unterstützung wird ein 512-Byte-Puffer verwendet. Andernfalls bleibt der bisherige größere Puffer.
- **Radar-Drosselung:** Ein erfolgreicher Abruf setzt nach HTTP 429 nicht mehr sofort alle Drosselungshinweise zurück. Positionsabrufe laufen vorübergehend mit mindestens 15 beziehungsweise 20 Sekunden Abstand; zusätzliche adsb.lol-Routenabfragen pausieren. Retry-After und Anbieterpausen bleiben maßgeblich. Kürzere HTTP-Antworttimeouts verkürzen einige Wartephasen, sind aber keine garantierte Obergrenze für die gesamte Verbindung.
- **Routen:** HTTP 201 wird beim adsb.lol-Routenendpunkt als mögliche erfolgreiche Antwort verarbeitet. Callsign, Antwortstruktur und Plausibilität der Flughäfen werden weiterhin geprüft.
- **Diagnose:** Der Log unterscheidet JSON-Speichermangel, unvollständige, leere, zu tief verschachtelte und ungültige Antworten. Der Systemstatus zeigt das vorübergehende Radar-Mindestintervall. Der Build enthält die passende ELF-Datei zur späteren Zuordnung von Absturzadressen.
- **Helligkeit:** Normal- und Nachtmodus erlauben mindestens 1 %. Zuvor gespeicherte 0 % werden beim Laden auf 1 % angehoben.
- **Lizenz:** Eigener Projektcode und Dokumentation stehen unter MIT. Copyright- und Lizenzhinweis bleiben bei Weitergabe verpflichtend. Herkunft aus smalltv-mod, Bibliothekslizenzen und externe Datenrechte werden getrennt aufgeführt. Dem Firmware-Build liegen Quellen, Lizenzhinweise und Buildobjekte bei.

### Ältere Versionen

| Version | Wichtigste Neuerungen |
| --- | --- |
| **v0.4.9** | Diagnose-Log mit Neustart-Hinweisen, getrennte Radar-Zusatzabfragen, gestufte Wetter-/VVO-Wiederholungen und Speichersperre während der Ortsnamensabfrage |
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

Hilfreich sind Firmwareversion, Gerät, betroffene Ansicht, Schritte zur Reproduktion sowie ein Foto oder der bereinigte Systemstatus. Keine WLAN-Passwörter oder API-Schlüssel veröffentlichen.

## Lizenz

Eigener Projektcode und Dokumentation: **MIT**, Copyright (c) 2026 du13500. Der Copyright- und Lizenzhinweis muss bei Weitergabe erhalten bleiben. Die Grundlage aus smalltv-mod von giovi321 wurde unter WTFPL veröffentlicht; der ursprüngliche Hinweis bleibt mitgeliefert. Für Bibliotheken und externe Daten gelten die jeweiligen Bedingungen. Kartendaten: © OpenStreetMap-Mitwirkende. Dieses Projekt ist unabhängig von GeekMagic, VVO und den genannten Datenanbietern.
