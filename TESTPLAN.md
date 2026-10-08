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

# Hardwaretest v0.4.10

| Priorität | Test | Erwartung |
| --- | --- | --- |
| 1 | Mehrere schnelle Kartenpunkte wählen, Ortsnamen abwarten, speichern | Kein Reboot; letzter gewählter Ort zählt. Bei Speichermangel kontrollierter Fehler und Freigabe der Speichertaste. |
| 1 | VVO mindestens 15 Minuten allein, danach alle Programme im Wechsel | Keine Exception, automatische Erholung nach fehlgeschlagenen Abrufen. |
| 1 | Wetter allein starten, dann andere Standorte speichern | Keine Abstürze; Ladezustand vor dem ersten Abruf, Fehler nur nach fehlgeschlagenem Abruf. Bei Rot sofort Log sichern. |
| 2 | Radar direkt nach Programmstart mit 5 s, später 10 s | Neue echte Positionen; bei HTTP 429 Anbieterpause und danach höheres Mindestintervall im Status. Keine zusätzliche adsb.lol-Route während dieser Phase. Nicht als sekündliche Live-Telemetrie bewerten. |
| 2 | Radar nach mindestens zehn Minuten ohne 429 | Rückkehr zum eingestellten Intervall; erneute Drosselung bleibt möglich. |
| 2 | Neues Flugzeug mit Route | HTTP 201 führt zur JSON-Auswertung; unpassende Flughäfen bleiben ausgeblendet. |
| 3 | Display-Teilaktualisierungen | Radar, Abfahrten und Countdowns bleiben flackerarm; kürzere Werte hinterlassen keine Reste. |
| 3 | Helligkeit und Nachtmodus, Neustart | Untergrenze 1 %, gespeicherte 0 % werden auf 1 % angehoben. |
| 3 | WLAN und Konfiguration nach Programmwechsel/Neustart | Einstellungen bleiben erhalten; bei Setup-Modus vor erneuter WLAN-Eingabe möglichst den Diagnose-Log sichern. |

Bei einem Absturz: Log unmittelbar nach Neustart sichern und ELF-Datei des tatsächlich installierten GitHub-Builds aufbewahren. Gleicher Reset-Text und gleicher RTC-Marker in späteren Logs desselben Starts bedeuten keinen erneuten Absturz. Der RTC-Marker ist ein Hinweis, kein Ursachenbeweis.
