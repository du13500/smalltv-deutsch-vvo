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
