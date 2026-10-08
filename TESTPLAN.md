# Testplan v0.4.13

## Vorrangiger Hardwaretest

1. Wetter: Dresden und New York direkt suchen, Treffer auswählen und speichern. Anschließend dasselbe im Flugradar testen.
2. Kartenpunkte bei ORD sowie am bisherigen Wetterstandort wählen. Ortsname und Koordinaten prüfen; mehrere Punkte nacheinander setzen.
3. Bei kontrollierter Ablehnung müssen Einstellungen weiterhin bedienbar sein. Manuelle Koordinaten und Anzeigename bleiben als Ausweichmöglichkeit verfügbar. Es darf kein Neustart erfolgen.
4. Ortsabfragen nach längerem Radar-Betrieb und nach Programmwechseln wiederholen. Bei jedem Fehler sofort Diagnose kopieren: Geo:Vorbereitung, Geo:Verbinden, Geo:Empfang und Geo:TLS-frei zeigen den erreichten Abschnitt. Heap-knapp vor Geo:Verbinden bedeutet Ablehnung vor dem Verbindungsaufbau; nach HTTP 200 bedeutet es Ablehnung während des Empfangs. Empfang:Fehler, Datei:Fehler und Antwort:ZuGross sind getrennte Fehler.
5. Bekannte Embraer E170/E175/E190/E195 und E2-Modelle prüfen. Der lesbare Name soll bei später eintreffenden Metadaten erhalten bleiben. Auch B38M und A21N prüfen.
6. VVO-Hinweis prüfen; Abfahrten, Radar, Wetter und Programmwechsel als kurzen Regressionstest laufen lassen.

HTTP 429 mit Pause und anschließender Erholung ist kein Neustart. Bei einer Exception den Log direkt nach dem Boot und die ELF-Datei des tatsächlich installierten Builds sichern. Gleicher Reset-Text und RTC-Marker in späteren Logs desselben Starts bedeuten keinen erneuten Absturz. Der RTC-Marker ist ein Hinweis, kein Ursachenbeweis.

Die Ortsuche auch direkt nach erfolgreicher Kartenauflösung und im Wechselbetrieb Wetter/Radar prüfen. Ein kontrollierter Speicherabbruch ist kein erfolgreicher Funktionstest. Bei Reboot sofort Log und ELF sichern.

## Lokale Prüfungen

Im entpackten Versionsordner zunächst `pio pkg install -e smalltv_ultra` ausführen. Danach:

```sh
python3 tools/check.py
python3 tools/test_geo_transfer.py
python3 tools/test_network_json.py
python3 tools/test_radar.py
python3 tools/test_policy.py
python3 tools/test_display.py
python3 tools/test_diagnostics.py
python3 tools/test_enrichment.py
g++ -std=c++11 -Wall -Wextra -Werror tests/test_core.cpp -o /tmp/smalltv-test-core
/tmp/smalltv-test-core
npm ci --ignore-scripts
npx playwright install --with-deps webkit
npm run test:webui
pio run -e smalltv_ultra
```

`test_geo_transfer.py` liest die echte `Stream::SendGenericPeekBuffer`-Routine aus dem installierten Arduino-ESP8266-Framework. Geprüft werden der ursprüngliche Null-Kapazitäts-Timeout, erfolgreicher Transfer, Größenlimit, Kurzschreiben, Transport-/Öffnungsfehler, Speichermangel vor und während des Empfangs sowie Aufräumen der temporären Datei. Netzwerk und Dateisystem werden dabei simuliert; das ersetzt keinen Hardwaretest.
