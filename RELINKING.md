# Firmware ändern und erneut bauen

Das GitHub-Begleitarchiv enthält den Projektquellstand, Framework, Bibliotheken und .pio/build/smalltv_ultra mit Objektdateien, Archiven und Linkerinformationen des ausgelieferten Builds. Die ELF-Datei wird außerdem separat bereitgestellt.

1. PlatformIO Core 6.1.19 installieren, Versionsarchiv entpacken.
2. Im Projektverzeichnis `pio run -e smalltv_ultra` ausführen. platformio.ini fixiert Plattform und Bibliotheken; die zugehörige Arduino-ESP8266-Version ist 3.1.2.
3. Für Änderungen am Framework das mitgelieferte framework-arduinoespressif8266 in einen lokalen Ordner entpacken und in platformio.ini über `platform_packages = framework-arduinoespressif8266 @ file:///ABSOLUTER/PFAD` auswählen. Die Compiler- und Linkerwerkzeuge werden regulär von PlatformIO installiert.
4. Quellen ändern und `pio run -t clean -e smalltv_ultra`, danach `pio run -e smalltv_ultra` ausführen. Projekt und Bibliotheken sind dadurch mit dem geänderten Core neu verknüpft. Es gibt keine Projektbeschränkung gegen eigene Änderungen oder Debugging dieser Änderungen.
5. Die neue BIN liegt in .pio/build/smalltv_ultra/firmware.bin. Sie kann über den Firmware-Updater installiert werden, sofern sie zum SmallTV-Ultra passt und in die Firmwarepartition passt.

Die unveränderten Objektdateien werden zusätzlich mitgeliefert, um das erneute Verknüpfen mit geänderten LGPL-Komponenten zu ermöglichen. Welche einzelnen SDK-Dateien geändert oder weitergegeben werden dürfen, richtet sich nach deren eigenen Lizenzhinweisen. Das Begleitarchiv ersetzt diese Bedingungen nicht.
