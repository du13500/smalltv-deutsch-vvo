# Roadmap

Stand: 06.11.2026. Erledigtes steht in [`CHANGELOG.md`](CHANGELOG.md).

## Nächste Prüfungen auf dem Gerät

- **v0.5.0-dev:** Sprachumschaltung (Display und Webinterface), abgekürzte Fahrtziele und Ortsname weglassen, Startbildschirm, Setup-WLAN „PixelStop-Setup“; Flash- und RAM-Belegung aus `build.log` notieren.
- **Flugradar:** Langzeittest über mehrere Tage mit Rotation; Anfragezähler „adsb.lol heute“ beobachten. Budgetwerte (`src/RadarBudget.h`) anpassen, sobald adsb.lol genaue Zahlen nennt (Limit, Reset-Zeitpunkt, Zählung von Fehlern und Routen). Freien Speicher während der Abfrage im Blick behalten (ca. 5 KB bei 4-KB-TLS-Puffer).
- **Wetter:** Exception 29 beim Programmwechsel ist seit den Speicherkorrekturen nicht wieder aufgetreten; im Langzeittest weiter beobachten.
- **Zeitzone, Sommerzeit, Nachtmodus:** gezielt prüfen, spätestens bei der Zeitumstellung Ende März.
- **Abfahrten:** zehn Abfahrten mit Seitenwechsel 5+5, Verhalten bei Verbindungsproblemen.
- **Helligkeit:** 300 Hz / 1023 Stufen bei 1 %; bei Fiepen oder Flimmern `BL_PWM_FREQ` in `src/main.cpp` anpassen.
- **Flug-Tracker** mit gültigem aviationstack-Schlüssel vollständig testen; Airline-Erholung nach fehlgeschlagenem Zusatzabruf prüfen.

## Geplante Versionen

| Version | Inhalt |
| --- | --- |
| **v0.5.0** | PixelStop, Deutsch/Englisch, Fahrtziele, neue README – nach dem Hardwaretest taggen |
| **v0.5.1** | Webinterface aufräumen: Rotation einzeilig, Reiterleiste in einer Zeile, Hinweis auf ungespeicherte Änderungen, Erklärtexte einklappbar, Countdowns kompakter, Koordinaten unter „Erweitert“; Knopf „Jetzt anzeigen“ je Programm; 3-Tage-Vorhersage |
| **v0.5.2** | Warnungen: Bevölkerungsschutz (NINA) und DWD-Wetterwarnungen, einzeln aktivierbar, regional gefiltert, mehrere nacheinander; optional Unterbrechung der Rotation |
| **v0.6.0** | Abfahrten über Anbieter: VVO, Transitous (deutschlandweit inkl. Bahn), danach weitere Verbünde; Haltestellensuche im Browser |
| **v0.7.0** | Optionaler moderner Anzeigestil mit echten Schriften und neuem Layout, vorher als Entwurf mit `tools/render_screens.py` |

## Ideen für später

- **Freitext und To-do-/Einkaufsliste** mit Erledigt-Funktion und automatischem Seitenwechsel (wenig Aufwand, kann jederzeit dazwischen).
- **Fotos** (einzelne JPGs aus dem Dateisystem), eventuell eigene Wetterbilder.
- **Eigene Ersetzungen für Fahrtziele** im Webinterface.
- **Englische Diagnosetexte.**
- **ESP32-Umbau** des SmallTV mit vorhandenem Display als Experiment (Pinbelegung, Stromversorgung, Platz, Portierung).

## Bewusst verworfen

- **Flughafen-Ankunfts- und Abflugtafel:** keine passende freie Datenquelle; aviationstack ist im kostenlosen Tarif zu knapp.
- **Animiertes Regenradar:** Bilddaten sind für Speicher und Netzwerk des ESP8266 zu groß.
- **Eigenes DB-Programm:** geht in den Anbietern von v0.6.0 auf.
- **Zusätzliche Zugangssperre** fürs Webinterface.
- **Zweites, privates Repository:** nicht nötig, solange alles unter MIT auf freien Diensten aufbaut.
