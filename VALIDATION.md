# Validierung v0.4.11

## Erfolgreich lokal geprüft

- Vollständiger Firmwarebuild: PlatformIO 6.1.19, espressif8266 4.2.1, Arduino-ESP8266 3.1.2, ArduinoJson 7.4.3 und GFX Library for Arduino 1.6.8.
- RAM statisch: 52.964 / 81.920 Bytes (64,7 %). Flash: 698.711 / 1.044.464 Bytes (66,9 %). Keine Messung des Laufzeit-Heaps.
- Produktionscode: Radar-Speicherschutz, Ablauf/Erholung, Routenparser, Retry-After und progressive Abrufpausen, Enrichment-Scheduler, Teilaktualisierung, Unicode und 50 Zeitzonen.
- Geo-Puffer: erfolgreicher Transfer, Größenlimit, Kurzschreiben, Transportfehler, Öffnungsfehler, Löschen; JSON erst nach TLS-Freigabe. Beide echten Geo-Filter mit ArduinoJson 7.4.3 und repräsentativen Such-/Reverse-Antworten nach Freigabe erfolgreich. Speicherschutzgrenze bleibt 3072 Bytes.
- Typprüfung: A3 ohne Live-Typ verwirft ST75, akzeptiert unterstützten A21N; bekannte Live-Typen müssen übereinstimmen, unbekannte Kategorien werden nicht als Bestätigung gewertet. Auch Cache-Anwendung prüft erneut.
- Diagnose: letzter Fehler überlebt Überschreiben des 24-Ereignisrings; RTC-Prüfsumme, OTA-Bereich, Dauer und Uhrüberlauf. Fehler bei Radar.Route/Metadata enthalten im separat behaltenen Fehler die Abfragekennung und das Callsign.
- WebKit: neun Breiten 320 bis 1024 Pixel. Neue Reihenfolge, ortsabhängige Speichersperre, Auswahl-Fallback beim Log-Kopieren und einheitliche Schaltflächenschrift. Dauerhafter Standortname im echten Displaycode getestet.

Lokal wurden die identischen offiziellen Compiler-/Frameworkarchive aus dem vorherigen Build genutzt, da der Paketmirror fehlerhafte Downloads lieferte. Nur lokal wurde das alte, nicht benötigte Uploadwerkzeug tool-esptool als optional markiert. Ausgelieferte platformio.ini und Workflow verwenden die regulären festgelegten Pakete.

## Grenzen und Hardwaretest

Kein physischer Würfel in dieser Umgebung. Schnelle Kartenwechsel, LittleFS-Schreiben, TLS und Laufzeit-Heap nach vielen Ortsabfragen sind auf Hardware zu prüfen. Temporäre Geo-Antworten verursachen Flash-Schreibvorgänge ausschließlich bei Ortsabfragen; maximal 48 KiB pro Antwort. Sie enthalten keine WLAN-Konfiguration und werden nach dem Versuch gelöscht.

Die Datenquellen können unpassende oder alte Routen liefern. Gleicher Zielflughafen kann trotz falscher Herkunft die Positionsprüfung bestehen. Typ-/Kategorieprüfung ist ein zusätzlicher Filter, keine vollständige Verifikation des Flugzeugs. Anbieterlimits und unveränderte 30-Sekunden-Datenalterung können weiterhin rote Radar-Anzeigen verursachen.

Log-Kopieren auf unverschlüsseltem lokalen HTTP verwendet einen Auswahl-Fallback; tatsächliche iPhone-Safari-Bedienung prüfen. Wetter-Icons bleiben unverändert. Aviationstack-Tracker weiterhin ohne vollständigen Hardwaretest mit API-Schlüssel.
