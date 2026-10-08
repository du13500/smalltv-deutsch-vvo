# Externe Datenquellen und Dienste

Die Projektlizenz MIT gilt für den eigenen Projektcode und die Dokumentation. Sie überträgt keine
Rechte an fremden Daten, Marken, Bibliotheken oder Diensten. Deren jeweilige
Bedingungen, Limits und Attributionspflichten gelten unabhängig davon.

| Verwendung | Dienst und maßgebliche Informationen |
| --- | --- |
| Haltestellen und Abfahrten | [VVO](https://www.vvo-online.de/), [Impressum](https://www.vvo-online.de/de/impressum/index.cshtml) |
| Wetter | [Open-Meteo](https://open-meteo.com/), [Bedingungen](https://open-meteo.com/en/terms) |
| Ortssuche und Reverse-Geocoding | [Nominatim](https://nominatim.org/release-docs/latest/api/Overview/), [Nutzungsregeln](https://operations.osmfoundation.org/policies/nominatim/) |
| Flugradar, Live-Telemetrie und Routen | [adsb.lol API](https://api.adsb.lol/docs) |
| Routen-Fallback und einzelne Zwischenstopps | [adsbdb](https://www.adsbdb.com/), [Projektdokumentation](https://github.com/mrjackwills/adsbdb) |
| Flugplandaten und Status im Tracker | [aviationstack](https://aviationstack.com/), [API-Dokumentation](https://docs.apilayer.com/aviationstack/docs/api-documentation) |
| Karten und Attribution | [OpenStreetMap Copyright](https://www.openstreetmap.org/copyright), [Kachelserver-Regeln](https://operations.osmfoundation.org/policies/tiles/) |
| Kartenbibliothek im Browser | [Leaflet](https://leafletjs.com/), ausgeliefert über unpkg.com |
| Uhrzeit | pool.ntp.org, time.cloudflare.com |

Ein kostenloser Zugriff ist keine pauschale Freigabe für jede Weiterverwendung.
Vor kommerzieller Nutzung oder Weiterverteilung der Daten die aktuellen
Anbieterbedingungen prüfen. Insbesondere nennt adsbdb in seiner Dokumentation
zusätzliche Bedingungen für die übernommenen Routendaten.

## Flugtracker: Ergebnis der Quellenprüfung für v0.4.6

adsbdb dokumentiert IATA-/ICAO-Rufzeichen, Start und Ziel sowie für manche Routen
einen `midpoint`. Diese Angaben beschreiben eine zugeordnete Route, nicht
zuverlässig den vollständigen aktuellen Flugplan oder Verspätungsstatus.
adsb.lol liefert empfangene Live-Telemetrie und die Routenfolge des Rufzeichens.
Fehlender ADS-B-Empfang bedeutet weder „gelandet“ noch „annulliert“.

Ein gleichwertiger kostenloser Ersatz ohne API-Key für alle bestehenden
Tracker-Felder ist in dieser Prüfung nicht belegt. **v0.4.6 behält aviationstack
für Plan-/Ist-Zeiten und Status bei.** Ohne eigenen Key bleiben diese Angaben
unverfügbar. Der Tracker ergänzt belegte Zwischenstopps über die vorhandenen
Routenquellen, wenn deren Route den gemeldeten Flugabschnitt enthält. Angezeigte
Start-/Zielpaare sind keine Nonstop-Zusage.

Prüfstand: 6. Oktober 2026. Anbieter können APIs und Bedingungen ändern.

## Abrufverhalten ab v0.4.10

Radarpositionen und Zusatzdaten werden getrennt geladen. Mehrteilige Routen bleiben erhalten, wenn die bestehende Positionsprüfung sie akzeptiert. Ein Start- oder Zielflughafen wird nicht allein aus der Nähe zu einem Flughafen abgeleitet. Die Quellen können weiterhin veraltete, unvollständige oder falsche Routen liefern.
