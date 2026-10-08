# Herkunft, Bibliotheken und Lizenzhinweise

Der eigene Projektcode und die Dokumentation von SmallTV Deutsch/VVO stehen ab v0.4.11 unter MIT. Copyright (c) 2026 du13500. Copyright- und Lizenzhinweis müssen bei Weitergabe erhalten bleiben. Eine sichtbare Namensnennung in Werbung oder auf dem Display wird dadurch nicht vorgeschrieben.

Die Hardware-/Displaygrundlage stammt aus smalltv-mod von giovi321, veröffentlicht unter WTFPL v2. Der originale mitgelieferte Lizenzhinweis bleibt in LICENSES/smalltv-mod-WTFPL.txt erhalten. Die WTFPL erlaubt Weiterlizenzierung; zuvor veröffentlichte Fassungen behalten ihre bisherigen Nutzungsrechte.

## Eigenständige Komponenten

| Bestandteil | Lizenz und Hinweise |
| --- | --- |
| ArduinoJson 7.4.3 | MIT, Benoit Blanchon; Originaltext in LICENSES/ArduinoJson-MIT.txt |
| GFX Library for Arduino 1.6.8 | Offizielles Repository: BSD mit Adafruit-Copyright; Text in LICENSES/Arduino-GFX-BSD.txt. Dateispezifische Herkunftshinweise bleiben in den unveränderten Bibliotheksquellen erhalten. Das installierte Registry-Paket enthält keinen eigenen license.txt; der mitgelieferte Text wurde aus dem offiziellen Repository übernommen, nicht aus einem verifizierten Release-Tag. |
| Arduino-ESP8266 3.1.2 | LGPL 2.1 und weitere Komponenten mit eigenen Bedingungen; Haupttext in LICENSES/ESP8266-LGPL-2.1.txt |
| BearSSL, LittleFS, umm_malloc, libb64, eboot | Mitgelieferte Originaltexte in LICENSES; zusätzliche Hinweise in den Frameworkquellen bleiben maßgeblich |
| Leaflet 1.9.4 | BSD 2-Clause, Volodymyr Agafonkin und CloudMade; LICENSES/Leaflet-BSD-2-Clause.txt; extern im Browser geladen |
| Espressif SDK, lwIP und weitere Frameworkbestandteile | Eigenständige Bedingungen in den Frameworkdateien. Die MIT-Projektlizenz ersetzt diese nicht. |
| Playwright und PlatformIO | Entwicklungswerkzeuge, nicht als solche Bestandteil der Firmware-BIN |

## Firmwareweitergabe

Der GitHub-Workflow legt Lizenztexte, den Projektquellstand, die tatsächlich verwendeten Framework- und Bibliotheksquellen sowie Buildobjekte in ein Begleitarchiv. firmware.elf wird separat bereitgestellt, damit Absturzadressen zum konkreten Build zugeordnet werden können. Hinweise zum Ändern und Neubauen stehen in RELINKING.md. Beim Weiterverteilen einer BIN auch dieses Begleitmaterial und die Lizenzhinweise bereitstellen.

Dies ist eine dokumentierte Prüfung der Hauptbestandteile, keine vollständige rechtliche Freigabe sämtlicher SDK-Dateien. Insbesondere die Herkunft des genauen GFX-Registry-Pakets und dateispezifische SDK-Hinweise bleiben zu beachten.

## Daten und Dienste

Die MIT-Lizenz gilt nicht für Wetter-, Verkehrs-, Flug- und Kartendaten, Marken oder die Dienste der Anbieter. Deren Nutzungsbedingungen, Abruflimits und Attributionspflichten gelten unabhängig. Maßgebliche Anbieter sind in THIRD_PARTY_SERVICES.md aufgeführt. Kartendaten: © OpenStreetMap-Mitwirkende. SmallTV Deutsch/VVO ist unabhängig von GeekMagic, VVO und den genannten Datenanbietern.
