# SmallTV Deutsch/VVO

[![Build SmallTV DD v0.4.14.5](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v04145.yml/badge.svg)](https://github.com/du13500/smalltv-deutsch-vvo/actions/workflows/build-v04145.yml)

> 🟨 **v0.4.14.5 · Entwicklungsbuild**  
> Für den **GeekMagic SmallTV-Ultra** · Deutschsprachige Oberfläche · Hardwaretest dieser Version noch offen

**Ein kleiner WLAN-Würfel für Abfahrten, Wetter, Flugradar, Uhr und Countdowns.** Nach der Einrichtung benötigt das Gerät nur USB-Strom und WLAN. Ein eigener Server oder Raspberry Pi ist nicht erforderlich.

Entwickelt von **du13500**, auf Grundlage von **smalltv-mod**. Der Flug-Tracker ist eine zusätzliche, optionale Ansicht für einen bestimmten Flug.

## v0.4.14.5

- Helligkeit: Ein unveränderter PWM-Wert wird nicht erneut gesetzt. Das vermeidet unnötige Eingriffe in die Hintergrundbeleuchtung. Ob das beobachtete Aufblitzen bei 1 % damit vollständig verschwindet, muss am Gerät geprüft werden.
- Airline-Namen: Fehlgeschlagene Abfragen werden nicht mehr als leeres Ergebnis für 30 Minuten gespeichert. Erfolgreich gespeicherte Namen bleiben auch während einer Abrufpause oder bei knappem Speicher verfügbar.
- Ortssuche: Der zusätzliche dauerhafte Hinweis oberhalb der Karte entfällt. Bei erkanntem Offlinezustand erscheint beim Suchen oder Öffnen der Karte rot „Internetverbindung erforderlich“. Ein nicht erreichbarer Dienst wird weiterhin gesondert beschrieben.
- System: Ortsdienst unter eingeklappten erweiterten Einstellungen; Dienste, Datenquellen und deren Bedingungen unterhalb der Wartungsschaltflächen.

Ortssuche und Radar wurden in den vorherigen Builds ab v0.4.14.3 im Praxistest als stabil gemeldet. Die Änderungen dieser Version benötigen noch einen Hardwaretest. Der Flug-Tracker mit eigenem aviationstack-Schlüssel ist weiterhin nicht vollständig auf Hardware getestet.

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

- `smalltv-deutsch-vvo-v0.4.14.5.zip` unverändert in das Hauptverzeichnis des Repositorys hochladen.
- `build-v04145.yml` als `.github/workflows/build-v04145.yml` ablegen.
- Die separate README-ZIP entpacken und deren README sowie Lizenzunterlagen im Hauptverzeichnis ersetzen. Sie enthält auch `LICENSE`, `LICENSES`, `THIRD_PARTY_NOTICES.md` und `THIRD_PARTY_SERVICES.md`.
- Unter **Actions** den Workflow **Build SmallTV DD v0.4.14.5** öffnen und ausführen, falls er nicht bereits automatisch gestartet wurde.
- Nach erfolgreichem Build **smalltv-dd-v0.4.14.5-firmware** herunterladen und die BIN flashen. **smalltv-dd-v0.4.14.5-begleitpaket** enthält separat Buildlog, ELF, Lizenztexte sowie Quellen und Buildobjekte.

Der Workflow baut den Inhalt des Versionsarchivs. Änderungen an anderen, älteren Quelldateien im Repository werden dadurch nicht automatisch Teil dieses Builds.

## Updates

Im Webinterface unter **System → Firmware-Update** die passende `.bin` auswählen. Während des Updates die Stromversorgung aufrechterhalten. Die Einstellungen werden im Gerät gespeichert und bleiben bei einem normalen Firmwareupdate erhalten.

## Datenquellen und Grenzen

- **VVO** liefert Abfahrten, **Open-Meteo** das Wetter.
- **OpenStreetMap / Nominatim** liefert Kartendaten und Ortsnamen. Gewässernamen werden übernommen, soweit die Ortsabfrage sie liefert; eine weltweite Gewässerabdeckung wird nicht zugesichert.
- **adsb.lol** liefert Radarpositionen, **adsb.lol und adsbdb** ergänzen Routen. **adsbdb** ergänzt verfügbare Airline- und Flugzeugtypenamen; lokale Bezeichnungen dienen als Rückfall.
- **aviationstack** liefert die Flugplandaten des optionalen Flug-Trackers. Dafür ist ein eigener Schlüssel erforderlich; Tarif und Abrufkontingent bestimmen die Nutzungsmöglichkeiten.

Radar-Routen stammen aus Zusatzdatenbanken und können trotz Positionsprüfung falsch oder unvollständig sein. Unpassende oder nicht verfügbare Routen werden nicht durch vermutete Flughäfen ersetzt. `N/A` bedeutet fehlende Information, `GND` einen ausdrücklich gemeldeten Bodenstatus. Höhenangaben stammen aus den verfügbaren Flugdaten und sind keine Höhe über dem Gelände.

adsb.lol kann Abfragen vorübergehend begrenzen. Während einer bestätigten Anbieterpause bleibt das zuletzt empfangene Flugzeug stehen. Die Anzeige nennt die Zeit seit dem letzten erfolgreichen Positionsabruf. Das ausgewählte Aktualisierungsintervall verlängert sich nach HTTP 429 automatisch auf mindestens 15 Sekunden, bei wiederholter Begrenzung auf mindestens 20 Sekunden. Nach zehn Minuten ohne weitere Begrenzung gilt wieder der gespeicherte Wert. Die Wartezeit beginnt nach Abschluss des Abrufs; die Antwortdauer kommt hinzu. Zusätzliche adsb.lol-Routenabrufe pausieren während dieser Drosselung.

### Radar-Footer und Fehlermeldungen

| Anzeige | Bedeutung |
| --- | --- |
| `ADSB.LOL LIVE 4s` | Letzter erfolgreicher Positionsabruf vor vier Sekunden |
| `LETZTER STAND 16s` | Vorherige Daten nach einem echten Abruffehler, vorübergehend weiter angezeigt |
| `ABRUFPAUSE 46s` | Bestätigte Anbieterpause; Alter des letzten erfolgreichen Positionsabrufs, kein Countdown bis zum nächsten Abruf |
| `KEIN FLUGZEUG` | Erfolgreiche Abfrage hat kein geeignetes Flugzeug geliefert |
| `FLUGRADAR / keine aktuellen Daten`, `ABRUF FEHLGESCHLAGEN` | Abruf konnte keine verwendbaren Positionsdaten liefern und es gibt keinen noch gültigen bisherigen Stand |

Der Abrufcounter lässt sich im Flugradar-Reiter deaktivieren und ist standardmäßig aktiviert. Nur die betroffene Footerfläche wird bei einem Sekundenwechsel aktualisiert. Die Uhrzeit steht rechts. Während blockierender Netzwerkoperationen können Sekundenupdates kurz stehen bleiben und anschließend zum tatsächlichen Alter springen. Zusatzdaten wie Airline und Route setzen das Alter nicht zurück. „LIVE“ beschreibt den Empfangsstand des Würfels, keine zugesicherte Aktualität der ursprünglichen Messung.

Die rote Fehlermeldung ist keine Aussage, dass keine Flugzeuge fliegen. Ursachen können WLAN-/Verbindungsfehler, HTTP-Fehler, eine unvollständige oder ungültige Antwort, Speichermangel oder fehlende Koordinaten sein. HTTP 200 allein garantiert keine verwendbaren Daten. Nach einem echten Abruffehler wird ein vorhandener Stand nach 30 Sekunden ohne neue Positionsdaten entfernt. Fehler beim Laden von Zusatzdaten allein lösen diese Meldung nicht aus. Während einer bestätigten Anbieterpause bleibt ein vorhandenes Flugzeug erhalten; ohne zuvor empfangenes Flugzeug kann kein alter Stand angezeigt werden.

### Ortssuche und Internetverbindung

Freitextsuche und Ortsauflösung auf der Karte laufen über die Internetverbindung des Browsers. Der ESP8266 übernimmt die ausgewählten Koordinaten und Namen. Kartenkacheln kommen ebenfalls direkt zum Browser. Die Orte für Wetter und Radar bleiben unabhängig. Wiederholte Anfragen werden im Browser zwischengespeichert; schnelle Kartenauswahlen werden zusammengefasst und Anfragen nacheinander versendet.

Unter System → Erweiterte Einstellungen kann ein Nominatim-kompatibler Ortsdienst für diesen Browser eingestellt werden. Die Standardadresse ist Nominatim von OpenStreetMap. Die Auswahl bleibt im Browser gespeichert. Der Offlinehinweis nutzt den vom Browser gemeldeten Verbindungszustand; eine gemeldete Onlineverbindung garantiert keine Erreichbarkeit eines externen Dienstes. Gültige Koordinaten können auch ohne ermittelten Ortsnamen gespeichert werden.

Die Anbieterlinks und Nutzungsregeln stehen direkt unter System → Dienste und Datenquellen. Weitere Hinweise enthält `THIRD_PARTY_SERVICES.md`.

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
