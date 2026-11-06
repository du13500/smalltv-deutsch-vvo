# Änderungen

## v0.5.1-dev

- **Ortsname bei Fahrtzielen:** Der Ort einer Haltestelle kommt jetzt aus der VVO-Suche und wird gespeichert. Bisher galt das erste Wort des Haltestellennamens als Ort; bei „S-Bahnhof Reick“ wurde so „S-Bahnhof“ als Ort angenommen, „Dresden Flughafen“ blieb ungekürzt und der Titel lautete nur „Reick“. Ohne gespeicherten Ort gilt weiter Dresden. Wer die Haltestelle vor v0.5.1 gewählt hat, sucht sie einmal neu aus, damit auch Haltestellen außerhalb Dresdens den richtigen Ort bekommen.
- **3-Tage-Vorhersage:** Die Wetteransicht wechselt alle 8 Sekunden zwischen heute und den nächsten drei Tagen (Wochentag, Symbol, Höchst-/Tiefstwert, Regen). Abschaltbar im Wetter-Reiter.
- **Webinterface aufgeräumt:** Reiter in einer wischbaren Zeile, Rotation einzeilig mit Knopf ▶ „Jetzt anzeigen“, roter Punkt am Speichern-Knopf bei ungespeicherten Änderungen, Koordinaten und lange Hinweise eingeklappt, inaktive Countdowns eingeklappt, Links im Akzentgelb.
- **Arbeitsspeicher:** etwa 3 KB fester Texte und Tabellen (Webserver-Antworten, Airline- und Flugzeugtypnamen, deutsche Wetterbezeichnungen) liegen jetzt im Flash.

## v0.5.0-dev

- **Neuer Name: PixelStop.** Repository, Firmware-Name, Setup-WLAN (`PixelStop-Setup`), Standard-Hostname (`pixelstop`), User-Agent und Dateinamen. Gespeicherte Hostnamen bleiben erhalten.
- **Deutsch/Englisch:** umschaltbar unter System → Sprache / Language, für Display und Webinterface. Display-Texte liegen beidsprachig im Flash; die englischen Webtexte werden nur bei Bedarf geladen.
- **Fahrtziele:** optional abkürzen (Hbf, Bf, Str., Pl., St.) und Ortsnamen der Haltestelle weglassen, beides standardmäßig an. Gekürzte Ziele enden nicht mehr mit Leerzeichen vor dem Punkt.
- Das bisher fest eingebaute Entfernen von „Dresden “ aus allen Anzeigetexten entfällt; Wetter-, Radar- und Countdown-Titel bleiben unverändert.
- `src/Version.h` als einzige Stelle für Name und Version.
- `tools/render_screens.py` erzeugt die Screenshots der README aus dem echten Displaycode.
- Neue README auf Deutsch und Englisch.


## v0.4.14.8

- Radar- und Routenabfragen nur, solange das Radar sichtbar ist, plus Vorabruf 10 s vor dem Wechsel.
- Tagesbudget (450 Anfragen/UTC-Tag) ohne Schlüssel; Intervall wird anhand des sichtbaren Radaranteils gestreckt, Routen bei knappem Budget übersprungen.
- Anfragezähler und Budget-Intervall im Systemstatus.

## v0.4.14.7

- adsb.lol-Abfragen über HTTP/1.1 statt HTTP/1.0 (Server antwortete mit 426); Chunked-Antworten werden beim Lesen entpackt.
- 401/403/426 lösen eine lange Pause (10/20/40 min) statt schneller Wiederholungen aus; eigene Displaymeldung.
- Optionaler adsb.lol-Schlüssel (`x-api-key`); Radar-Intervalle 1 und 3 min.

## v0.4.14.6

- Hintergrundbeleuchtung: PWM 300 Hz statt 1 kHz und 1023 statt 255 Stufen; längere Einschaltpulse bei 1 % gegen Aufblitzen.
- Quellcode direkt im Repository; Releases über Versions-Tags.

## v0.4.14.5.1

- Exakte VVO-Antwort ServiceError / no serving lines found neutral darstellen; übrige Fehler unverändert.
- Phasensynchrone PWM-Ansteuerung des vorhandenen Frameworks zum Hardwaretest gegen Aufblitzen.
- Gemeinsamer Build-Workflow mit gezielten Auslösern und Abbruch überholter Builds.
- README-Versionsgeschichte ergänzt; README einzeln liefern.

## v0.4.14.5

- Unveränderte Helligkeitswerte setzen die PWM nicht erneut.
- Airline-Cache speichert nur erfolgreich geladene Namen; Cache vor Speicher- und Pausenprüfung.
- Doppelten Kartenhinweis entfernt, Offlinefeedback ergänzt.
- Ortsdienst eingeklappt unter erweiterten Einstellungen; Dienste und Bedingungen unter Wartung.
- Dokumentation auf Browser-Ortssuche, Anbieterpausen, Counter und Fehleranzeigen aktualisiert.

## v0.4.14.4.2

- Einheitlicher optionaler Abrufcounter für Live, letzten Stand und Anbieterpause, standardmäßig aktiv.

## v0.4.14.4

- Letztes Flugzeug bleibt während bestätigter Anbieterpause mit Altersanzeige erhalten.

## v0.4.14.3

- Ortssuche und Kartenauflösung in den Browser verlagert.

## v0.4.13

- Geo prüft den freien Heap und den größten Speicherblock erneut unmittelbar vor dem HTTP-Abruf, nach Vorbereitung von LittleFS-Datei, URL, HTTP-Client und Headern. Eine nicht ausreichende Reserve führt zum kontrollierten Abbruch vor dem Verbindungsaufbau.
- Lokale URL-Kopien und die interne Suchtextkopie werden nach Übernahme durch HTTPClient freigegeben. Die Suche fordert keine ungenutzten Addressdetails mehr an; acht Treffer und deutsche Namensvarianten bleiben erhalten. Die Kartenauflösung fordert ihre benötigten Adressdaten weiterhin an.
- Feste Diagnosebezeichnungen liegen im Flash. Der statische RAM-Bedarf sinkt gegenüber v0.4.12 um 484 Bytes. Das ist ein begrenzter Gewinn, keine Garantie für erfolgreiche Ortsabfragen in jedem Betriebszustand.
- Neue Geo-Marker: Geo:Vorbereitung, Geo:Verbinden, Geo:Empfang und Geo:TLS-frei. Normale Fortschrittsmarker werden nicht als letzter Fehler gespeichert; sie bleiben als RTC-Marker auswertbar.
- Der Empfangsschutz aus v0.4.12 bleibt bestehen: mindestens 3 KiB Rest-Heap, höchstens 48 KiB Antwort, begrenzte Schreibschritte und JSON erst nach TLS-Freigabe.

Hardwaretest offen. Der v0.4.12-Absturz ist durch die passende ELF als ungültiger Schreibzugriff innerhalb der ROM-Speicherkopierfunktion eingegrenzt, aber nicht vollständig erklärt. Wetter-/Radar-Abrufe, Anbieterpausen, Typanzeige und Programme bleiben erhalten.

## v0.4.12

- Geo-Empfang gezielt repariert: Der LittleFS-Zwischenpuffer meldet nun seine Schreibkapazität. Ohne diese Angabe übertrug die ESP8266-Routine in v0.4.11 keine Bytes und wartete bis zum Timeout.
- Empfang in höchstens 256-Byte-Schritten. Unter 3 KiB freiem Heap, bei Schreibfehlern oder Antworten über 48 KiB wird kontrolliert abgebrochen. Der Dateicache wird vor der TLS-Verbindung vorbereitet; JSON wird weiterhin erst nach TLS-Freigabe gelesen.
- Diagnose unterscheidet Empfangsfehler, Dateifehler und zu große Antworten von JSON-Parserfehlern.
- Regressionstest verwendet den tatsächlichen Übertragungscode des installierten ESP8266-Frameworks statt eines vereinfachten Transfers.
- Bekannte Airbus-, Boeing- und Embraer-E-Jet-Typen erhalten stabile lesbare Displaynamen, auch bevor Metadaten eintreffen. E75L und E75S werden als Embraer E175 angezeigt.
- Abfahrten-Hinweis nennt jetzt den Verkehrsverbund Oberelbe (VVO) ausgeschrieben.

Gezielte Weiterentwicklung von v0.4.11. Radar-Abrufe, Anbieterpausen, Routenprüfung, Wetterabrufe und Konfiguration bleiben erhalten. Kein vollständiger Rückbau auf v0.4.9. Hardwaretest dieser Version offen.

## v0.4.11

- Ortssuche und Karten-Ortsnamen: TLS zuerst beenden, anschließend JSON lesen. Die Antwort wird vorübergehend in einer größenbegrenzten LittleFS-Datei gespeichert und danach gelöscht. Das vermeidet gleichzeitigen TLS-/JSON-Speicherbedarf, verursacht aber zusätzliche Flash-Schreibvorgänge bei Ortsabfragen. Keine Schreibvorgänge bei normalen Wetter- oder Radar-Aktualisierungen.
- Flugzeugtyp nur übernehmen, wenn Live-Typ oder unterstützte Live-Kategorie dazu passen. Bei unbekannter Kategorie bleiben zusätzliche Typangaben leer. Testfall N54561: gemeldete Kategorie A3 darf keinen ST75-Doppeldecker übernehmen.
- Streckendaten bleiben eine externe Datenbankzuordnung, kein belegter aktueller Flugplan. Falsche Herkunft mit gleichem Ziel kann die Positionsprüfung weiterhin bestehen.
- Leere HTTP-201-Antwort bei Radar.Route im Log als leer kenntlich machen.
- Log kopieren mit Auswahl-Fallback auf lokalem HTTP; gleiche Schrift für Schaltflächen. Letzten Fehler dieses Starts separat behalten, auch wenn die 24 Ereignisse überschrieben sind.
- Programmauswahl: Abfahrten, Wetter, Flugradar, Flug-Tracker, Uhr, Countdowns. Bestehende Programm-IDs bleiben erhalten.
- Standortname: Kein Wechsel (Wert 0), Wunschtext bleibt stehen.
- Workflow: Firmware und Diagnose-/Lizenz-/Quellenpaket als getrennte Downloads.

Hardwaretest für v0.4.11 offen. v0.4.10 lief im Nutzer-Belastungstest über 83 Minuten ohne Neustart, inklusive Programmwechseln und selbstständiger Erholung nach HTTP 429. Ortssuche in v0.4.10 blieb durch JSON:NoMemory defekt.

# Versionsgeschichte

### v0.4.10

- **Speicher bei Netzwerkabfragen:** JSON-Speicher wächst in kleineren Blöcken. Ein eigener Allocator hält 3 KiB freien Heap zurück; bei fehlendem Speicher wird die Antwort verworfen. Nicht mehr benötigte Anfrage-, Filter- und TLS-Daten werden vor der weiteren Auswertung freigegeben. Auch ein unvollständig aufgebauter JSON-Filter wird erkannt. Das soll die in v0.4.9 beobachteten Abstürze bei Karten- und VVO-Abfragen vermeiden; der Hardwaretest steht aus.
- **TLS-Puffer:** Wetter, VVO, Ortsabfragen und Radar-Metadaten prüfen einmal pro Start, ob der jeweilige Server kleinere TLS-Empfangsblöcke unterstützt. Nur bei bestätigter Unterstützung wird ein 512-Byte-Puffer verwendet. Andernfalls bleibt der bisherige größere Puffer.
- **Radar-Drosselung:** Ein erfolgreicher Abruf setzt nach HTTP 429 nicht mehr sofort alle Drosselungshinweise zurück. Positionsabrufe laufen vorübergehend mit mindestens 15 beziehungsweise 20 Sekunden Abstand; zusätzliche adsb.lol-Routenabfragen pausieren. Retry-After und Anbieterpausen bleiben maßgeblich. Kürzere HTTP-Antworttimeouts verkürzen einige Wartephasen, sind aber keine garantierte Obergrenze für die gesamte Verbindung.
- **Routen:** HTTP 201 wird beim adsb.lol-Routenendpunkt als mögliche erfolgreiche Antwort verarbeitet. Callsign, Antwortstruktur und Plausibilität der Flughäfen werden weiterhin geprüft.
- **Diagnose:** Der Log unterscheidet JSON-Speichermangel, unvollständige, leere, zu tief verschachtelte und ungültige Antworten. Der Systemstatus zeigt das vorübergehende Radar-Mindestintervall. Der Build enthält die passende ELF-Datei zur späteren Zuordnung von Absturzadressen.
- **Helligkeit:** Normal- und Nachtmodus erlauben mindestens 1 %. Zuvor gespeicherte 0 % werden beim Laden auf 1 % angehoben.
- **Lizenz:** Eigener Projektcode und Dokumentation stehen unter MIT. Copyright- und Lizenzhinweis bleiben bei Weitergabe verpflichtend. Herkunft aus smalltv-mod, Bibliothekslizenzen und externe Datenrechte werden getrennt aufgeführt. Dem Firmware-Build liegen Quellen, Lizenzhinweise und Buildobjekte bei.

### v0.4.9

Diagnose-Log, RTC-Marker, getrennte Radar-Abfragen, Fehlererholung für Wetter/VVO und Speichersperre während Ortsabfragen. Hardwaretests zeigten weiterhin HTTP-429-Pausen sowie Abstürze bei speicherintensiven JSON-Abfragen.

Ältere Versionen sind in der README zusammengefasst.

# Übersicht der Versionen bis v0.4.14.8

Aus der früheren README übernommen.

| Version | Änderungen |
| --- | --- |
| **v0.4.14.8**   | Radar-Abfragen nur bei sichtbarem Radar plus Vorabruf; Tagesbudget mit automatischer Intervallstreckung ohne Schlüssel |
| **v0.4.14.7**   | adsb.lol über HTTP/1.1 mit Chunk-Entpacker; lange Pause bei 401/403/426 statt Reboot-Schleife; optionaler adsb.lol-Schlüssel; Intervalle 1 und 3 min |
| **v0.4.14.6**   | Hintergrundbeleuchtung mit 300 Hz und 1023 Stufen gegen Aufblitzen bei 1 %; Quellcode direkt im Repository, Releases über Tags |
| **v0.4.14.5.1** | Neutrale VVO-Meldung für nicht bediente Haltestellen; Test einer phasensynchronen Hintergrundbeleuchtung; gemeinsamer Build-Workflow; Versionsgeschichte ergänzt |
| **v0.4.14.5** | Unveränderte PWM-Werte nicht erneut setzen; fehlerhafte leere Airline-Cacheeinträge vermeiden; Offlinehinweise; Ortsdienst einklappen und Anbieterinformationen unter System |
| **v0.4.14.4.2** | Einheitlicher optionaler Abrufcounter für LIVE, LETZTER STAND und ABRUFPAUSE; standardmäßig aktiviert |
| **v0.4.14.4.1** | Zeitangabe bei Anbieterpause links zum Statustext verschoben |
| **v0.4.14.4** | Letztes Flugzeug während bestätigter Anbieterpause erhalten; Alter seit letztem erfolgreichen Positionsabruf anzeigen |
| **v0.4.14.3** | Freitextsuche und Karten-Ortsauflösung in den Browser verlagert; Warteschlange, Cache und austauschbarer Ortsdienst |
| **v0.4.14.2** | Geo-Empfang und Speicherbudget getrennt geprüft; TLS-Objekte vor JSON-Auswertung vollständig freigegeben; Ortsabfragen auf Hardware noch nicht zuverlässig |
| **v0.4.14.1** | TLS-Speicherprüfung an tatsächliche Stackgröße und kleinere zusammenhängende Blockanforderung angepasst |
| **v0.4.14** | Weitere gezielte Korrekturen am Geo-Empfang auf Basis von v0.4.13; Karten- und Freitextsuche blieben in den Hardwaretests problematisch |
| **v0.4.13** | Zusätzliche Speicherprüfung vor Geo-Verbindung; Diagnosemarker erweitert; feste Diagnosebezeichnungen in Flash verlagert |
| **v0.4.12** | Schreibkapazität und Fehler des Geo-Zwischenpuffers korrekt melden; Empfang begrenzen; stabile lesbare Flugzeugtypen einschließlich Embraer; VVO-Hinweis ausgeschrieben |
| **v0.4.11** | Geo-Antwort zwischenspeichern und erst nach TLS-Freigabe auswerten; Flugzeugtyp plausibilisieren; letzter Fehler separat behalten; Programmauswahl neu geordnet; Standortname ohne Wechsel möglich |

Die Geo-Korrekturen v0.4.11 bis v0.4.14.2 beseitigten nicht alle auf Hardware beobachteten Probleme. Erst die Verlagerung der Ortssuche in den Browser ab v0.4.14.3 wurde im Gespräch als stabil bestätigt. Die Tabelle beschreibt Änderungen, keine zugesicherte Fehlerfreiheit aller historischen Builds.


## v0.4.10

- **Speicher bei Netzwerkabfragen:** JSON-Speicher wächst in kleineren Blöcken. Ein eigener Allocator hält 3 KiB freien Heap zurück; bei fehlendem Speicher wird die Antwort verworfen. Nicht mehr benötigte Anfrage-, Filter- und TLS-Daten werden vor der weiteren Auswertung freigegeben. Auch ein unvollständig aufgebauter JSON-Filter wird erkannt. Das soll die in v0.4.9 beobachteten Abstürze bei Karten- und VVO-Abfragen vermeiden; der Hardwaretest steht aus.
- **TLS-Puffer:** Wetter, VVO, Ortsabfragen und Radar-Metadaten prüfen einmal pro Start, ob der jeweilige Server kleinere TLS-Empfangsblöcke unterstützt. Nur bei bestätigter Unterstützung wird ein 512-Byte-Puffer verwendet. Andernfalls bleibt der bisherige größere Puffer.
- **Radar-Drosselung:** Ein erfolgreicher Abruf setzt nach HTTP 429 nicht mehr sofort alle Drosselungshinweise zurück. Positionsabrufe laufen vorübergehend mit mindestens 15 beziehungsweise 20 Sekunden Abstand; zusätzliche adsb.lol-Routenabfragen pausieren. Retry-After und Anbieterpausen bleiben maßgeblich. Kürzere HTTP-Antworttimeouts verkürzen einige Wartephasen, sind aber keine garantierte Obergrenze für die gesamte Verbindung.
- **Routen:** HTTP 201 wird beim adsb.lol-Routenendpunkt als mögliche erfolgreiche Antwort verarbeitet. Callsign, Antwortstruktur und Plausibilität der Flughäfen werden weiterhin geprüft.
- **Diagnose:** Der Log unterscheidet JSON-Speichermangel, unvollständige, leere, zu tief verschachtelte und ungültige Antworten. Der Systemstatus zeigt das vorübergehende Radar-Mindestintervall. Der Build enthält die passende ELF-Datei zur späteren Zuordnung von Absturzadressen.
- **Helligkeit:** Normal- und Nachtmodus erlauben mindestens 1 %. Zuvor gespeicherte 0 % werden beim Laden auf 1 % angehoben.
- **Lizenz:** Eigener Projektcode und Dokumentation stehen unter MIT. Copyright- und Lizenzhinweis bleiben bei Weitergabe verpflichtend. Herkunft aus smalltv-mod, Bibliothekslizenzen und externe Datenrechte werden getrennt aufgeführt. Dem Firmware-Build liegen Quellen, Lizenzhinweise und Buildobjekte bei.

## Ältere Versionen

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
