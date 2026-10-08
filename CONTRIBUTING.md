# Mitmachen bei SmallTV Deutsch/VVO

Danke für Fehlerberichte und Verbesserungsvorschläge. Dieses Projekt ist eine
eigenständige deutschsprachige Firmware auf Grundlage von smalltv-mod.

## Unterstützte Hardware und Projektstruktur

Getestetes Gerät: **GeekMagic SmallTV-Ultra mit ESP8266 / ESP-12F, 4 MB Flash
und 240 × 240 Pixel ST7789**. Andere Geräte und Displayvarianten sind nicht
getestet; Kompatibilität wird nicht zugesichert.

Der aktuelle Quellstand liegt im Versionsarchiv
`smalltv-deutsch-vvo-v0.4.11.zip`. Vor Änderungen dieses Archiv entpacken.
Die noch vorhandenen Dateien des Ursprungsprojekts im Repository-Hauptverzeichnis
sind nicht der Quellstand dieses Versionsbuilds. Kein automatischer Fork-Sync.

Im entpackten Versionsordner:

```sh
python3 tools/check.py
python3 tools/test_radar.py
python3 tools/test_display.py
g++ -std=c++11 -Wall -Wextra -Werror tests/test_core.cpp -o /tmp/smalltv-test-core
/tmp/smalltv-test-core
pio run -e smalltv_ultra
```

Der GitHub-Workflow `.github/workflows/build-v0411.yml` baut ausschließlich dieses
Versionsarchiv. Änderungen am Quellcode müssen auch im Archiv enthalten sein.
Die README ist die vollständige Benutzerdokumentation ohne Unterseiten.
Weitere Dateien im Quellarchiv dienen Entwicklung und Validierung.
Die separate README-ZIP bleibt Teil jeder Auslieferung.

## Fehler melden

Bitte Firmwareversion, Gerät, betroffenen Programmtyp, Schritte zur Reproduktion
und erwartetes/tatsächliches Verhalten nennen. Bei Layoutfehlern Browser und
Bildschirmgröße ergänzen. Fotos und bereinigte Buildlogs helfen.

Keine WLAN-Passwörter, API-Keys oder vollständigen privaten Konfigurationsdateien
veröffentlichen. Für öffentliche Credits ausschließlich `du13500` verwenden.

## Änderungen beitragen

- Pixelstil und funktionierende Standardansichten erhalten.
- Neue Optionen mit kompatiblen Standardwerten versehen.
- Speicherbedarf, Watchdog, API-Abfragehäufigkeit und OTA-Größe beachten.
- Keine Flugrouten, Zeiten oder Statuswerte erfinden, wenn eine Quelle sie nicht liefert.
- Geeignete Prüfungen ausführen und Hardwaretests ehrlich kennzeichnen.
- README, Änderungsprotokoll und Testplan zum Verhalten aktualisieren.
- Den generierten Font über `tools/generate_font.py` ändern und neu erzeugen.

WireGuard, MQTT/Home Assistant und zusätzliche Hardwaretargets gehören nicht zum
zugesagten Funktionsumfang. Neue größere Ideen zunächst diskutieren.

## Lizenz und Datenquellen

Projektcode: [MIT](LICENSE). Für externe Daten, Bibliotheken und Dienste gelten
ihre jeweiligen Bedingungen; siehe [Datenquellen](THIRD_PARTY_SERVICES.md).
