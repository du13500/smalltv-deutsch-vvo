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
