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

# Themenspeicher

- Radar-Zuverlässigkeit mit v0.4.10 auf Hardware prüfen; bei Aussetzern den Diagnose-Log auswerten.
- Wetter-Ortswechsel und VVO-Erstabruf testen; Neustart-Hinweise sind noch kein Ursachenbeweis.
- Wetter-Icons in einem zukünftigen Build gestalterisch überarbeiten, zunächst Motive und Varianten gemeinsam vergleichen.
- Flug-Tracker mit gültigem aviationstack-Schlüssel vollständig prüfen.
- Erneuter Dauerbetrieb mit allen Programmen nach den Speicheränderungen; ein kurzer Rotationstest in v0.4.9 lieferte erfolgreiche Abrufe, aber keinen Langzeitnachweis.
- MIT für eigene Quellen ist festgelegt. Herkunft des konkreten GFX-Pakets und dateispezifische Framework-/SDK-Hinweise weiter prüfen.
- Nicht darstellbare Ortsnamen weiterhin über das optionale Freitextfeld lösen.
