# Nächste Prüfungen

- Flugradar v0.4.14.7 am Gerät prüfen: Positionen, Routen und Flug-Tracker über HTTP/1.1, freier Heap während der Abfrage, mit und ohne adsb.lol-Schlüssel.

- Helligkeit v0.4.14.6 (300 Hz, 1023 Stufen) am Gerät bei 1 % prüfen; bei hörbarem Fiepen oder sichtbarem Flimmern Frequenz anpassen (`BL_PWM_FREQ` in `src/main.cpp`).
- Langzeittest mit Wetter und Radar sowie Anbieterpausen fortsetzen.
- Airline-Erholung nach fehlgeschlagenem Zusatzabruf prüfen; Routenqualität bleibt quellenabhängig.
- Flug-Tracker mit gültigem aviationstack-Schlüssel und verschiedene Wetterbilder vollständig auf Hardware prüfen.

