# SmallTV Deutsch/VVO

Eigene Firmware für den **GeekMagic SmallTV-Ultra** mit ESP8266 und 240×240-Display.

Aktueller Stand: **v0.1 läuft als eigener ESP8266-Build.**  
Die Oberfläche und Gerätefunktionen sind bereits angelegt, die Live-Datenquellen werden jetzt schrittweise ergänzt.

## Geplante Ansichten

- **Abfahrten** über VVO
- **Wetter** über Open-Meteo
- **Flugradar** über ADS-B
- **Uhr**

## Ziel

Eine schlanke, deutschsprachige Firmware ohne die ursprünglichen Stock-/Crypto-/Claude-/Home-Assistant-Funktionen.

Der SmallTV soll nach der Einrichtung vollständig selbstständig laufen: USB-Strom + WLAN, ohne zusätzlichen Server oder Raspberry Pi.

## Status

**v0.1**
- deutsches Webinterface
- WLAN + Fallback-Hotspot
- Helligkeit und Nachtmodus
- LittleFS-Konfiguration
- OTA-Update
- NTP-Uhr
- Testlayouts für Abfahrten, Wetter und Flugradar

**Als Nächstes**
- echte VVO-Haltestellensuche
- Live-Abfahrten mit Echtzeitdaten und Ausfällen

> Noch nicht für produktives Flashen freigegeben. Der SmallTV-Ultra wird erst nach abgeschlossenem Loader- und Recovery-Test geflasht.

## Basis

Entstanden auf Basis von [`giovi321/smalltv-mod`](https://github.com/giovi321/smalltv-mod).  
Die ursprüngliche Firmwarebasis steht unter WTFPL.
