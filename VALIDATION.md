# Validierung v0.4.14.5.1

- Firmwarebuild bestanden: PlatformIO 6.1.19, ESP8266-Framework 3.1.2, gepinnte Bibliotheken.
- Statischer RAM: 53.288 von 81.920 Bytes. Flash: 709.007 von 1.044.464 Bytes.
- ELF enthält die aktiv ausgewählte phasensynchrone Waveform-Implementierung. Rund 512 Byte zusätzlicher statischer RAM gegenüber v0.4.14.5.
- VVO-Test führt die Produktions-Abfragefunktion mit ArduinoJson aus: exakte unbediente Haltestelle, andere ServiceError-Meldungen, HTTP-Fehler, ungültiges JSON, erfolgreiche Abfahrten, leere Liste und Erholung. Nur die vereinbarte Antwort erhält den neuen Zustand.
- Displaytest prüft die graue zweiteilige Meldung, den neutralen Footer, ausbleibende Wiederholungs-Neuzeichnungen sowie den Wechsel zur bisherigen Fehleranzeige.
- Bestehende Tests für Helligkeit, Radar, Anbieterpausen, Airline-Cache, Diagnose, Display, JSON und Geo-Speicherschutz bleiben enthalten.

## Hardwaregrenze

Die Hardwareursache des Aufblitzens ist nicht nachgewiesen. Die alternative, im vorhandenen Framework enthaltene PWM ist ein begrenzter Versuch. Frequenz, Helligkeitswerte und die Prüfung unveränderter Werte bleiben erhalten. Bei 1 % Helligkeit Counterübergänge und neue Flugzeuge sowie den Nachtmodus prüfen. Radar- und Geo-Abruflogik wurden nicht geändert.
