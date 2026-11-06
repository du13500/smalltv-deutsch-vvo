# Nächste Prüfungen

- v0.5.0-dev am Gerät: Sprachumschaltung (Display und Webinterface), abgekürzte Fahrtziele, Startbildschirm und Setup-WLAN „PixelStop-Setup“.

- v0.4.14.8: Anfragezähler über einen Tag beobachten; Budgetwerte anpassen, sobald adsb.lol genaue Zahlen nennt (Limit, Reset-Zeitpunkt, Zählung von Fehlern und Routen).

- Flugradar v0.4.14.7 am Gerät prüfen: Positionen, Routen und Flug-Tracker über HTTP/1.1, freier Heap während der Abfrage, mit und ohne adsb.lol-Schlüssel.

- Helligkeit v0.4.14.6 (300 Hz, 1023 Stufen) am Gerät bei 1 % prüfen; bei hörbarem Fiepen oder sichtbarem Flimmern Frequenz anpassen (`BL_PWM_FREQ` in `src/main.cpp`).
- Langzeittest mit Wetter und Radar sowie Anbieterpausen fortsetzen.
- Airline-Erholung nach fehlgeschlagenem Zusatzabruf prüfen; Routenqualität bleibt quellenabhängig.
- Flug-Tracker mit gültigem aviationstack-Schlüssel und verschiedene Wetterbilder vollständig auf Hardware prüfen.

