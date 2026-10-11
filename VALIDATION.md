# Validierung v0.4.14.5

## Lokal geprüfte Funktionen

- Versionsangaben, JavaScript-Syntax, eindeutige HTML-IDs, Zeitzonen und reproduzierbarer Zeichensatz.
- Produktionsfunktionen für Helligkeit: Erstaufruf, unveränderte Wiederholungen, Helligkeitsänderung, Nachtmodusgrenzen und Begrenzung auf 100 %.
- Produktions-Airline-Cache mit ArduinoJson 7.4.3: HTTP-Fehler, ungültiges JSON, leere Namen und abweichendes ICAO werden nicht als gültiger Name gespeichert; Erholung, Ablauf, Cache während Pause und knappen Speichers geprüft.
- Bestehende Tests für Radar, Display, Diagnose, Zusatzdaten-Scheduler, Anbieterpausen, JSON-Speicherreserve und Geo-Helfer bestanden. Browser-Ortssuche nutzt die Geo-Helfer im ESP8266 nicht mehr für reguläre Anfragen.
- ESP8266-Build mit PlatformIO 6.1.19 und den gepinnten Bibliotheken bestanden: statischer RAM 52.776 Bytes von 81.920 Bytes; Flash 708.959 Bytes von 1.044.464 Bytes.

- WebKit 26.5 / Playwright 1.62.1: Layout auf 320, 375, 390, 430, 560, 561, 768, 820 und 1024 Pixeln; Einstellungen laden/speichern, Counter-Option, Geo-Warteschlange, Cache, Namensauswahl, Offlinefeedback, Anbieterpause, Systemblöcke und Diagnosebedienung bestanden.

## Grenzen

Kein physischer Würfel verfügbar. Das gemeldete Aufblitzen bei 1 % kann durch wiederholte PWM-Aufrufe mitverursacht werden; die vollständige Beseitigung ist nicht bewiesen. Unveränderte Werte werden nun nicht erneut gesetzt. Hardwaretest bei Counterwechseln, neuen Flugzeugen und Nachtmodus erforderlich.

Die im Gespräch gemeldete Stabilität betrifft vorherige Builds. Anbieterqualität, verfügbare Routen und Netzwerkantwortzeiten sind nicht durch lokale Tests garantiert. Die Offlineerkennung kann nur den vom Browser gemeldeten Zustand und Anfragefehler auswerten.
