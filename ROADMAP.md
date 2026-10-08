## Aktuell offen

- v0.4.13 auf Hardware: Ortsuche und Reverse-Geocoding unter knappen Speicherbedingungen prüfen. Der Übertragungsfehler ist lokal reproduziert und korrigiert; die Reboots aus v0.4.11/v0.4.12 ist damit nicht abschließend erklärt.
- Externe Flugzeug- und Routendaten können veraltet oder falsch zugeordnet sein. Die vorhandene Plausibilitätsprüfung ersetzt keinen aktuellen Flugplan.

# Themenspeicher

- Radar-Zuverlässigkeit mit v0.4.10 auf Hardware prüfen; bei Aussetzern den Diagnose-Log auswerten.
- Wetter-Ortswechsel und VVO-Erstabruf testen; Neustart-Hinweise sind noch kein Ursachenbeweis.
- Wetter-Icons in einem zukünftigen Build gestalterisch überarbeiten, zunächst Motive und Varianten gemeinsam vergleichen.
- Flug-Tracker mit gültigem aviationstack-Schlüssel vollständig prüfen.
- Erneuter Dauerbetrieb mit allen Programmen nach den Speicheränderungen; ein kurzer Rotationstest in v0.4.9 lieferte erfolgreiche Abrufe, aber keinen Langzeitnachweis.
- MIT für eigene Quellen ist festgelegt. Herkunft des konkreten GFX-Pakets und dateispezifische Framework-/SDK-Hinweise weiter prüfen.
- Nicht darstellbare Ortsnamen weiterhin über das optionale Freitextfeld lösen.
