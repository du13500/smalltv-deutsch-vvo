# Lizenzprüfung für v0.4.13

Stand: 7. Oktober 2026. Eigener Projektcode und Dokumentation stehen ab v0.4.13 unter MIT. Herkunft und fremde Bedingungen werden getrennt dokumentiert. Die Entscheidung folgt dem Wunsch nach freier Nutzung unter Erhalt des Copyright- und Lizenzhinweises.

## CC0 und MIT

| Punkt | CC0 1.0 | MIT |
| --- | --- | --- |
| Ziel | Eigene Urheber- und verwandte Rechte soweit rechtlich möglich aufgeben; Auffanglizenz bei unwirksamem Verzicht | Sehr weitgehende Nutzungserlaubnis für Software |
| Kommerzielle Nutzung und Änderungen | Erlaubt | Erlaubt |
| Namensnennung | Keine verpflichtende Zuschreibung für die unter CC0 freigegebenen eigenen Rechte | Copyright- und Lizenzhinweis müssen in Kopien oder wesentlichen Teilen erhalten bleiben; keine Werbenennung vorgeschrieben |
| Gewährleistung | Ausschluss im Rechtstext | Gewährleistungs- und Haftungsausschluss im Rechtstext |
| Patente und Marken | Ausdrücklich nicht betroffen | Keine ausdrückliche Patentlizenz im Standardtext; Markenrechte werden nicht pauschal freigegeben |
| Fremde Bestandteile | Ihre bisherigen Bedingungen bleiben bestehen | Ihre bisherigen Bedingungen bleiben bestehen |

CC0 ist laut Creative Commons auch für Software möglich. Wenn das Ziel die freie Nutzung der eigenen Beiträge ohne verpflichtenden Copyright-Hinweis ist, passt CC0 dazu. MIT bietet eine verbreitete Softwarelizenz, verlangt aber das Beibehalten des Hinweises. Keine der beiden Optionen beseitigt fremde Lizenzpflichten.

Die WTFPL-FAQ erlaubt eine Weiterlizenzierung. Ein Wechsel sollte trotzdem die Herkunft aus smalltv-mod dokumentieren und die eigenen Beiträge klar von Bibliotheken abgrenzen. Bisherige unter WTFPL veröffentlichte Kopien werden dadurch nicht rückwirkend entzogen.

## Im Projekt verwendete Bestandteile

| Bestandteil | Festgestellter Stand | Konsequenz |
| --- | --- | --- |
| Eigener Projektcode / smalltv-mod-Herkunft | MIT für eigene Quellen; ursprüngliche WTFPL v2 separat erhalten | Herkunftshinweis bewahren. Keine fremden Copyright-Zeilen als eigene ausgeben. |
| ArduinoJson 7.4.3 | Lokale LICENSE.txt: MIT, Benoit Blanchon | Originalen Copyright- und Lizenztext bei Weitergabe berücksichtigen. |
| GFX Library for Arduino 1.6.8 | Installiertes Paket nennt diese Version, enthält jedoch keinen vollständigen Lizenztext. Offizielles Repository führt license.txt mit BSD-Bedingungen und Adafruit-Copyright; Quelltext nennt weitere Herkunftsprojekte. | Vor einer umfassenden Lizenzumstellung und BIN-Verteilung den exakten Stand sowie dateispezifische Hinweise sichern. Keine pauschale CC0/MIT-Angabe für die Bibliothek. |
| Arduino-ESP8266 3.1.2 | Lokale Framework-LICENSE: LGPL 2.1; Paket enthält weitere Drittkomponenten | Bedingungen und Quellenbereitstellung der jeweiligen Komponenten getrennt beachten. Dieses Dokument ersetzt keine vollständige SDK-/Bibliotheksprüfung. |
| Leaflet im Browser | Extern geladen, eigenständige Bibliothek | Lizenz und OSM-Attribution bleiben unabhängig von der Projektlizenz. |
| Playwright | Entwicklungswerkzeug, nicht Teil der Firmware-BIN | Eigenständige Bedingungen; kein Anlass für eine Umstellung der Projektquellen. |
| Wetter-, Verkehrs-, Karten- und Flugdaten | Externe Dienste mit eigenen Bedingungen | Datenrechte werden durch die Code-Lizenz nicht mitfreigegeben. |

## Ergebnis

MIT wurde für den eigenen Projektcode gewählt. LICENSES enthält die Originaltexte der lokal geprüften Hauptkomponenten und die Herkunftslizenz von smalltv-mod. THIRD_PARTY_NOTICES.md grenzt diese Bestandteile ab. Der Workflow erstellt ein Begleitarchiv aus Quellen, Lizenzen und Buildobjekten. Eine pauschale MIT-Freigabe der gesamten Firmware einschließlich Bibliotheken und Daten erfolgt nicht.

## Geprüfte Primärquellen

- Creative Commons: CC0 1.0 Legal Code, insbesondere Abschnitte 2 bis 4; https://creativecommons.org/publicdomain/zero/1.0/legalcode.en
- Creative Commons: CC0 FAQ, Software und Attribution; https://wiki.creativecommons.org/wiki/CC0_FAQ
- Open Source Initiative: MIT-Standardtext; https://opensource.org/license/mit
- WTFPL: FAQ zur Weiterlizenzierung; https://www.wtfpl.net/faq/
- GFX: offizieller license.txt-Stand im Repository; https://github.com/moononournation/Arduino_GFX/blob/master/license.txt
- Lokal geprüfte Lizenztexte der installierten ArduinoJson- und ESP8266-Frameworkpakete.

Die Prüfung erfasst die genannten Hauptbestandteile, keine vollständige rechtliche Freigabe aller SDK-Bestandteile. Originaltexte bleiben maßgeblich.
