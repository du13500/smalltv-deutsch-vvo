# Validierung v0.4.13

## Lokal erfolgreich geprüft

- Vollständiger Firmwarebuild: PlatformIO 6.1.19, espressif8266 4.2.1, Arduino-ESP8266 3.1.2, ArduinoJson 7.4.3, GFX Library for Arduino 1.6.8.
- Statischer RAM: 52.752 / 81.920 Bytes (64,4 %). Flash: 700.131 / 1.044.464 Bytes (67,0 %). Das sind keine Laufzeit-Heap-Messungen.
- Geo: Originalfehler mit Standard-Schreibkapazität 0 reproduziert. Die echte Framework-Übertragungsroutine wird mit dem Produktionspuffer geprüft: erfolgreicher Empfang, 256-Byte-Schritte, 48-KiB-Grenze, Kurzschreiben, Transportfehler, Öffnungs-/Truncate-Fehler, Heap-Ablehnung vor und während des Empfangs und Dateilöschung. JSON wird erst nach TLS-Freigabe gelesen.
- Die beiden Produktions-Geo-Filter bestehen repräsentative Such-/Reverse-Antworten mit ArduinoJson 7.4.3. Der JSON-Allocator hält weiterhin 3072 Bytes zurück.
- Bestehende Tests für Radar-Speicherschutz, Alterung/Erholung, Routenparser, Retry-After, progressive Anbieterpausen, Enrichment, Diagnose/RTC und Unicode bestehen.
- Displaytest: stabile bekannte Typnamen vor und nach Metadaten, einschließlich E75L/E75S und E2-Modelle; Breite höchstens 224 Pixel. Bestehende Displayzustände bestehen.
- WebKit: alle neun Breiten 320, 375, 390, 430, 560, 561, 768, 820 und 1024 Pixel bestehen.

Das offizielle Framework- und SCons-Archiv aus dem vorherigen Build wurde lokal wiederverwendet, weil Paketmirror teilweise falsche Downloads lieferten. Die offiziellen Versionsstände bleiben unverändert. Ausgelieferte platformio.ini und Workflow verwenden reguläre festgelegte Pakete.

- Geo-Integration: Der zweite Speichercheck folgt nach temporärer Datei, URL-Freigabe und HTTP-Header-Vorbereitung und vor GET. Die Reihenfolge der neuen Diagnosemarker wird geprüft.
- Diagnose: Flash-Tabellen werden im ESP8266-Build kompiliert; Hosttest prüft Ausgabe, Fortschrittsmarker ohne falschen letzten Fehler und RTC-Wiedererkennung.
- Statischer RAM sinkt um 484 Bytes gegenüber v0.4.12. Laufzeit-Ersparnis durch Freigabe der URL-Kopien ist nicht auf Hardware vermessen.

## Grenzen

Kein physischer Würfel verfügbar. Reales LittleFS, TLS-Empfang und langandauernde Ortsabfragen sind auf Hardware zu prüfen. Unter 3 KiB Rest-Heap wird bewusst abgebrochen; erfolgreiche Ortsauflösung in jedem Betriebszustand ist nicht zugesagt. Temporäre Geo-Dateien verursachen Flash-Schreibvorgänge ausschließlich bei Ortsabfragen und werden nach dem Versuch gelöscht.

Die hochgeladene ELF des tatsächlich installierten v0.4.12-Builds stimmt bytegenau mit dem lokalen ELF überein. Exception 29 und excvaddr 0 zeigen einen ungültigen Schreibzugriff; epc1 0x4000df64 liegt hinter dem ROM-Symbol memcpy bei 0x4000df48. Die zusätzliche epc3-Adresse liefert ohne vollständigen Stacktrace keinen belastbaren Aufrufpfad. Speicherknappheit ist plausibel, aber als alleinige Absturzursache nicht bewiesen. v0.4.13 behebt die belegte Lücke der zu frühen Speicherprüfung und schafft begrenzt RAM; die reale Ortsauflösung und Reboot-Vermeidung bleiben auf Hardware zu testen.

Radar-Datenbankzuordnungen können weiterhin falsch oder unvollständig sein. Bekannte Typcodes erhalten konsistente Displaynamen; unbekannte Typen behalten den bisherigen Fallback. Anbieterlimits und die unveränderte Datenalterung können weiterhin rote Radar-Anzeigen verursachen. Keine Änderungen an Radar-/Wetter-Abrufen und keine vollständige Rückkehr zu v0.4.9.
