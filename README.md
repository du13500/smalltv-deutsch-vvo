# SmallTV Deutsch/VVO · v0.4.7

Deutschsprachige Firmware für den GeekMagic SmallTV-Ultra mit ESP8266 und
240 × 240 Pixel ST7789-Display. Der Würfel zeigt Abfahrten, Wetter, Flugradar,
Uhr, einen Flug-Tracker und vier unabhängige Countdowns. Nach der Einrichtung
reichen USB-Strom und WLAN. Ein eigener Server ist nicht erforderlich.

Diese README enthält Einrichtung, Funktionen, Änderungen und Teststand in
einer Datei. Es gibt keine Unterseiten oder Verlinkungen.

## Neu in v0.4.7

- Flugradar: Der bisher pauschale Schutz bei weniger als 16.000 Byte größtem
  Speicherblock wird durch ein Budget passend zur TLS-Puffergröße ersetzt.
  Freier Gesamtspeicher wird ebenfalls geprüft. Die Speicherwerte aus den
  gemeldeten v0.4.6-Screenshots verhindern den Abruf damit nicht mehr.
- Der Radar-Timer wurde korrigiert: Ein Zeitstempel wird nicht mehr künstlich
  um ein Bit verändert. Das konnte bei geraden Millisekundenwerten einen
  sofortigen weiteren Abruf auslösen.
- Nach einem fehlgeschlagenen Abruf heißen die noch sichtbaren Radardaten
  „LETZTER STAND“. Nach 30 Sekunden ohne erfolgreichen Positionsabruf
  verschwindet das alte Flugzeug. Alte Daten bleiben nicht als „LIVE“ stehen.
- Beim gleichen Flugzeug werden nur geänderte sichtbare Felder neu gezeichnet.
  Das gilt für Entfernung, Höhe und Änderungen an Flugnummer, Airline, Route,
  Typ oder Registrierung. Ein neues Flugzeug oder ein anderer Anzeigezustand
  wird vollständig gezeichnet. Zwischenstopps aus v0.4.6 bleiben erhalten.
- Im Reiter Anzeige steht zwischen Programmauswahl und Nachtmodus die
  Zeitzone. Es gibt 50 Auswahlmöglichkeiten; Standard ist Europe/Berlin.
- Der Pixelzeichensatz enthält jetzt auch griechische und kyrillische
  Buchstaben sowie weitere lateinische Sonderzeichen. Unbekannte Zeichen
  bleiben Kästchen. Der eigene Anzeigename bleibt die einfache Alternative.
- Wetter: Auch bei teilweiser Bewölkung erscheint nachts der Mond. Niesel,
  Regen, gefrierender Niederschlag, Schnee, Schneekörner, Schauer und Gewitter
  mit Hagel erhalten unterscheidbare Motive und Intensitäten. Niederschlag
  fällt jeweils aus einer Wolke. Nebel hat horizontale Bänder ohne Wolke;
  Reifnebel erhält ein zusätzliches Eissymbol.
- Im Flug-Tracker enthält der Footer nur die Uhrzeit, wenn Flugnummer oder
  API-Key fehlt. Der Hinweis im Hauptbereich bleibt erhalten.

Der Radarfix ist anhand der Statuswerte und des Quellcodes entwickelt und
mit simuliertem Speicher und Zeitablauf geprüft. Die echte Verbindung und
Dauerstabilität auf dem Würfel müssen nach dem Update getestet werden.

## Dateien und Build über GitHub

Das Lieferpaket enthält:

- `smalltv-deutsch-vvo-v0.4.7.zip`: vollständiger Versionsquellcode.
- `build-v047.yml`: Workflow für GitHub Actions.
- `smalltv-dd-v0.4.7-firmware.zip`: hier erfolgreich gebaute BIN-Datei mit
  Prüfsumme und Buildprotokoll für das direkte Firmware-Update.
- `smalltv-v0.4.7-readme.zip`: ausschließlich diese README, separat für den
  Download und das Entpacken auf iPad und iPhone.

Für das bestehende Repository mit Versionsarchiven:

1. `smalltv-deutsch-vvo-v0.4.7.zip` unverändert ins Repository-Hauptverzeichnis
   hochladen. Der Workflow entpackt das Archiv selbst.
2. Die Workflow-Datei unter `.github/workflows/build-v047.yml` ablegen.
   In GitHubs Dateieditor kann der vollständige Pfad als Dateiname eingegeben
   werden. Die Datei gehört nicht ins Hauptverzeichnis.
3. Die README aus der separaten ZIP entpacken und als `README.md` im
   Hauptverzeichnis ersetzen.
4. Unter Actions den Workflow „Build SmallTV DD v0.4.7“ starten. Ein Push der
   Versions-ZIP oder des Workflows auf main startet ihn ebenfalls.
5. Nach erfolgreichem Lauf das Artefakt `smalltv-dd-v0.4.7` herunterladen und
   entpacken. Darin liegt `smalltv-dd-v0.4.7-firmware.bin` zum Update sowie die
   SHA-256-Prüfsumme und das Buildprotokoll.

Der Workflow baut ausschließlich den entpackten Versionsordner. Historische
Quelldateien im Repository-Hauptverzeichnis werden nicht mitgebaut.
Eine abweichende alte README im Hauptverzeichnis blockiert den Build nicht.

## Firmware installieren

### Vorhandene Custom-Firmware aktualisieren

Die BIN kann aus dem GitHub-Artefakt oder aus der zusätzlich mitgelieferten
Firmware-ZIP entpackt werden. Für die fertige BIN ist kein eigener Build nötig.

1. Die lokale Weboberfläche des Würfels im WLAN öffnen.
2. Im Reiter System „Firmware-Update“ wählen.
3. Die entpackte BIN-Datei auswählen und das Update durchführen.
4. Während des Updates die Stromversorgung erhalten. Nach dem Neustart
   muss die Versionsanzeige v0.4.7 zeigen.

Gespeicherte Einstellungen werden weiter eingelesen. Fehlt die neue Zeitzone
in einer älteren Konfiguration, wird Europe/Berlin verwendet.

### Ersteinrichtung

Der Build ist für SmallTV-Ultra mit ESP-12F/ESP8266, 4 MB Flash, ST7789,
SPI Mode 3 und LittleFS ausgelegt. Andere SmallTV-Varianten sind nicht
getestet. Der geeignete Erstflashweg hängt von der vorhandenen Firmware ab;
die BIN-Datei allein legt diesen Weg nicht fest.

Ohne erreichbares gespeichertes WLAN startet die Firmware einen
Einrichtungszugang. Mit dem angezeigten WLAN verbinden und die lokale
Einrichtungsseite öffnen. Anschließend WLAN auswählen, Passwort eingeben
und speichern. Im normalen Betrieb erreicht man die Seite über die IP des
Würfels, beispielsweise aus der Geräteliste des Routers.

Der Hostname ist der Gerätename im WLAN, standardmäßig `smalltv-dd`.
Ein leer gelassenes WLAN-Passwortfeld behält das gespeicherte Passwort.
Änderungen an WLAN oder Hostname lösen einen Neustart aus.

## Anzeige und Uhrzeit

Mehrere Programme können aktiviert werden. Sie wechseln entsprechend ihrer
eigenen Anzeigedauer. Ist nur ein Programm aktiv, bleibt es stehen.
Anzeigedauer und Datenabruf sind getrennte Einstellungen. Ein Wechsel lädt
Wetter oder Abfahrten nicht jedes Mal neu.

NTP liefert die absolute Uhrzeit. Die ausgewählte Zeitzone bestimmt daraus
die lokale Anzeige und gegebenenfalls Sommer- oder Winterzeit. Es ist kein
festes tägliches Zeitfenster für Sonne und Mond eingestellt: Beim Wetter
liefert Open-Meteo die Information Tag oder Nacht.

Die Zeitzone gilt für Uhr, Footer-Uhren, Nachtmodus und Kalendertage der
Tages-Countdowns. Sonnenaufgang und Sonnenuntergang werden von Open-Meteo
in dieser Zeitzone angefragt. Die Wetterposition selbst bleibt unverändert.
Flugplanzeiten im Tracker bleiben die von der Flug-API gelieferten
Flughafenzeiten. Laufende Stunden- oder Sekunden-Countdowns behalten ihre
absolute Zielzeit.

Die Sommerzeitregeln sind Teil der Firmware. Politische Änderungen an
Zeitzonenregeln erfordern bei Bedarf ein Firmwareupdate.

Der Nachtmodus dimmt nach lokalem Beginn und Ende, auch über Mitternacht.
Die Helligkeit kann für Tag und Nacht getrennt gewählt werden.

## Programme

### Abfahrten

Der VVO-Abfahrtsmonitor bietet Haltestellensuche, Echtzeit-Abfahrten,
Verspätungen und Ausfälle. Verkehrsmittel können einzeln ausgewählt werden:
Zug, S-Bahn, Straßenbahn, Bus, Seil-/Schwebebahn, Fähre und AST/Rufbus.
Die Anzeige unterstützt drei bis fünf Abfahrten oder zehn auf zwei
5er-Seiten. Diese Seiten wechseln im festen 5-Sekunden-Takt.
Lange Linienbezeichnungen werden angepasst; für lange Ziele gibt es eine
kompakte Schriftoption.

### Wetter

Open-Meteo liefert aktuelle Temperatur, Wettercode, Tag-/Nachtinformation,
Tageshöchst- und Tiefstwert, Regenwahrscheinlichkeit sowie Sonnenaufgang
und Sonnenuntergang. Celsius und Fahrenheit sowie farbige Temperaturanzeige
sind wählbar. Höchst- und Tiefsttemperatur bleiben kompakt, etwa `H 21°C`
und `T 7°C`, ohne Leerzeichen zwischen Zahl und Einheit.

Ortssuche, Karten-Picker und manuelle Koordinaten stehen zur Verfügung.
Es gibt die bereits vorhandene Auswahl deutscher Ortsnamen, soweit die
Quelle diese liefert, sowie ein eigenes Feld für den Anzeigenamen.
Es wird keine neue automatische Sprachersetzung oder Transliteration
aufgrund fehlender Zeichen durchgeführt.

| Wettercode | Motiv |
| --- | --- |
| 0, 1 | Sonne oder Mond |
| 2 | Sonne oder Mond hinter einer Wolke |
| 3 | Wolke |
| 45, 48 | Nebelbänder; bei Reifnebel zusätzlich Eis |
| 51, 53, 55 | Niesel aus einer Wolke, drei Intensitäten |
| 56, 57 | Gefrierender Niesel mit Eiszeichen, zwei Intensitäten |
| 61, 63, 65 | Regen aus einer Wolke, drei Intensitäten |
| 66, 67 | Gefrierender Regen mit Eiszeichen, zwei Intensitäten |
| 71, 73, 75 | Schneeflocken aus einer Wolke, drei Intensitäten |
| 77 | Kleine Schneekörner aus einer Wolke |
| 80, 81, 82 | Regenschauer, drei Intensitäten, Sonne/Mond hinter Wolke |
| 85, 86 | Schneeschauer, zwei Intensitäten, Sonne/Mond hinter Wolke |
| 95 | Gewitterwolke mit Regen und Blitz |
| 96, 99 | Gewitterwolke mit Hagel und Blitz, zwei Intensitäten |

Intensitäten werden mit Anzahl und Art der Niederschlagselemente sowie
passendem Text unterschieden. Die kleine Pixelanzeige bildet keine
meteorologische Detailkarte ab.

### Flugradar

Das Radar zeigt das nächstgelegene von adsb.lol gemeldete Flugzeug innerhalb
des eingestellten Radius. Standort und Anzeigename sind unabhängig vom
Wetter konfigurierbar. Der Header kann zwischen „IN DER NÄHE“ und dem
Standort wechseln.

Angezeigt werden Callsign, Airline soweit bekannt, Route mit verfügbaren
Zwischenstopps, Flugzeugtyp, Registrierung, Entfernung und Höhe. Für
Routen werden adsb.lol und gegebenenfalls adsbdb verwendet. Fehlende
Informationen heißen `N/A`. Eine Route ist keine Nonstop-Zusage.

Der Abruf-Takt ist einstellbar. Auch bei fünf Sekunden müssen Zahlen nicht
zwingend bei jedem Abruf anders aussehen: Die Quelle kann dieselbe Position
liefern, und die Entfernung wird auf eine Nachkommastelle gerundet.
Entscheidend sind erfolgreiche neue Abrufe. Das Display zeichnet beim
selben Flugzeug nur tatsächlich geänderte sichtbare Werte neu.

Der Status im Reiter System zeigt freien Heap, größten freien Block,
Abrufstatus, HTTP-Status bei Bedarf, TLS-Empfangspuffer und benötigte
Blockgröße. Ein übersprungener oder fehlgeschlagener Abruf wird als Fehler
behandelt; alte Werte werden höchstens 30 Sekunden seit dem letzten
erfolgreichen Positionsabruf behalten. Danach erscheinen keine alten
Flugzeugdaten mehr. Eine erfolgreiche leere Antwort bedeutet dagegen
„kein Flugzeug“, nicht einen Verbindungsfehler.

### Uhr

Die Uhr synchronisiert sich über NTP und zeigt die gewählte lokale Zeit.
Ihre eigene Ansicht hat keine redundante zweite Footer-Uhr. Andere
Informationsansichten aktualisieren die Footer-Uhr minütlich.

### Flug-Tracker

Der Tracker verfolgt eine eingetragene Flugnummer. Flugstatus und
Flugplanzeiten kommen über aviationstack mit eigenem API-Key. Der
Abrufabstand ist einstellbar und beeinflusst den Verbrauch des API-Kontingents.
ADS-B-Daten ergänzen Route und Telemetrie, soweit verfügbar. Eine kostenlose
ADS-B-Quelle ersetzt Flugplanzeiten und Status nicht vollständig.

Ohne Flugnummer oder API-Key bleibt der Footer bis auf die Uhrzeit leer.
Der Hauptbereich erklärt die fehlende Konfiguration. Der Tracker ist optional;
die übrigen Programme benötigen keinen aviationstack-Key.

### Countdown 1 bis 4

Vier unabhängig gespeicherte Countdown-Slots unterstützen Titel, Farben,
Pixelherzen, Kalendertage sowie Stunden:Minuten:Sekunden und Minuten:Sekunden.
Vor dem Ziel erscheint ein Minus, bei Null kein Vorzeichen und danach ein
Plus. Absolute Zielzeitpunkte bleiben nach Stromverlust erhalten. Ein
Tages-Countdown wird nur neu gezeichnet, wenn sich sein sichtbarer Wert ändert.

## Zeichen und Speicher

Der zusätzliche 5 × 7-Pixelzeichensatz umfasst 545 Sonderzeichen und
346 Kombinationen aus Grundzeichen und Akzent. Dazu gehören erweiterte
lateinische Buchstaben, modernes Griechisch und die kyrillischen Zeichen
U+0400 bis U+045F sowie zusätzliche häufige Varianten. Vorkomponierte und
unterstützte getrennt geschriebene Akzente erscheinen als ein Zeichen.
Countdown-Titel werden nach sichtbaren Zeichen begrenzt, nicht mitten in
einem mehrbyteigen Buchstaben abgeschnitten. Die Tabellen liegen im Flash. Chinesisch, Japanisch und arabische
Schriftformung sind nicht enthalten. Unbekannte Zeichen werden als Kasten
angezeigt. Ein eigener kurzer Anzeigename ist dafür weiterhin vorgesehen.

Im erfolgreich gebauten v0.4.7:

| Ressource | Belegt | Verfügbar im Buildziel |
| --- | ---: | ---: |
| Statischer RAM | 48.052 Byte, 58,7 % | 81.920 Byte |
| Programm-Flash | 672.839 Byte, 64,4 % | 1.044.464 Byte |

Zum Vergleich meldete der erfolgreiche v0.4.6-Build 47.504 Byte RAM und
652.879 Byte Flash. Der genaue freie Heap im Betrieb ist etwas anderes:
TLS, Netzwerk, JSON-Antworten und die Weboberfläche brauchen zusätzlichen
zeitweiligen Speicher. Besonders Flugradar und Tracker sind dabei
anspruchsvoller als Uhr und Countdown. Eine größere Zahl aktivierter
Programme ist mit dieser Version noch kein gemessener Dauertest.
Die vereinbarten Tests mit mehr als zwei Programmen folgen nach dem
Radar-Praxistest.

## Lokal bauen und prüfen

Versions-ZIP entpacken und im Ordner `smalltv-deutsch-vvo-v0.4.7` arbeiten.
Der Build verwendet PlatformIO 6.1.19, ESP8266-Plattform 4.2.1,
ArduinoJson 7.4.3 und GFX Library for Arduino 1.6.8.

```sh
python3 -m pip install platformio==6.1.19
python3 tools/check.py
python3 tools/test_radar.py
python3 tools/test_display.py
g++ -std=c++11 -Wall -Wextra -Werror tests/test_core.cpp -o /tmp/smalltv-core-test
/tmp/smalltv-core-test
npm install --ignore-scripts
npx playwright install --with-deps webkit
npm run test:webui
python3 -m platformio run -e smalltv_ultra
```

Die BIN liegt anschließend in `.pio/build/smalltv_ultra/firmware.bin`.
Der GitHub-Workflow führt diese Prüfungen ebenfalls aus.

## Teststand und Prüfung auf dem Würfel

Der vollständige ESP8266-Build und die automatisierten Prüfungen für
Zeichensatz, Zeitzonen, Wetterzuordnung, Radar-Speicherentscheidungen,
Ablauf alter Daten, Tracker-Footer und selektive Displayupdates bestehen.
Auch die lokale WebKit-Prüfung besteht an allen neun Breiten von 320 bis
1024 Pixeln, einschließlich Laden und Speichern der Zeitzone, Zeitfelder,
Abstände und Kontrolle auf horizontalen Seitenüberlauf.

Nach dem Flashen zuerst mit einem, höchstens zwei Programmen testen:

1. Radar auf fünf Sekunden stellen. Im Status prüfen, dass bei den bisherigen
   Heap-Werten wieder Abrufe erfolgen. Entfernung und Höhe bei einem
   vorbeifliegenden Flugzeug über mehrere Abrufe beobachten.
2. Gleiches Flugzeug: nur geänderte Felder zeichnen sich neu. Neues Flugzeug:
   vollständige neue Ansicht. Änderungen der Route ebenfalls beobachten.
3. WLAN kurz unterbrechen: alter Stand wird markiert und verschwindet
   spätestens 30 Sekunden nach dem letzten Erfolg. Nach Wiederherstellung
   müssen gültige Daten wieder erscheinen.
4. Europe/Berlin und eine deutlich abweichende Zeitzone ausprobieren,
   anschließend speichern und neu starten. Nachtmodus und Tages-Countdown
   müssen dieselbe Zeitzone verwenden.
5. Kyrillische und griechische Titel sowie nachts teilbewölktes Wetter prüfen.
6. Tracker ohne Flugnummer und ohne Key: keine Fehlermeldung im Footer.
7. Mehrstündigen Betrieb beobachten, insbesondere Heap, Radar und Neustarts.

Die automatische Browserprüfung ersetzt keine Prüfung in echtem iPhone-
oder iPad-Safari. Eine stabile TLS-Verbindung und sichere Speicherreserven
auf dem realen ESP8266 lassen sich nicht aus dem Build allein bestätigen.

## Datenquellen und Lizenz

VVO liefert Abfahrten, Open-Meteo Wetter und Ortssuche, die vorhandenen
Karten- und Ortsdienste unterstützen die Standortwahl. Flugpositionen und
Routen kommen von adsb.lol und adsbdb, optionale Flugplandaten von
aviationstack. Diese Dienste können Daten verzögert, unvollständig oder
zeitweise gar nicht liefern. Ihre Kontingente und Nutzungsbedingungen gelten
unabhängig von dieser Firmware.

Die Hardware- und Displaygrundlage stammt aus smalltv-mod unter WTFPL.
Die Projektlizenz steht auch im Versionsarchiv. Die verwendeten Bibliotheken
behalten ihre eigenen Lizenzen. SmallTV Deutsch/VVO wird von du13500 gepflegt.
