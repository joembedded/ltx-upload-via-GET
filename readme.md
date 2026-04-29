# WugTest - (C)JoEmbedded

### **Stand:** 30.04.2026 

Arduino-Sketch für einen Uploader auf [LTX_SERVER](https://github.com/joembedded/LTX_server) via GET-UPLOAD über einen XIAO ESP32S3.

Der Sketch verbindet sich mit WLAN und sendet in einem per Makro einstellbaren Intervall einen ungefaehren Temperaturwert an einen LTX/Wunderground-kompatiblen Upload-Endpunkt.

Die Doku [docu/0950_get_upload_DE.md](docu/0950_get_upload_DE.md) beschreibt das GET-Upload-Format:

```text
http://server.example/ltx/sw/lxu_wug_v1.php?ID=0000000000000000&PASSWORD=CHANGE_ME&tempf=61.70
```

`ID` und `PASSWORD` sind Zugangsdaten. Der Temperaturwert wird als Fahrenheit-Wert im Parameter `tempf` uebertragen.

## Dateien

- [wugtest.ino](wugtest.ino): Arduino-Sketch fuer WLAN-Verbindung und HTTP-GET-Upload.
- [secret/_placeholder_config.h](secret/_placeholder_config.h): Vorlage mit ungefaehrlichen Beispielwerten.
- `secret/config.h`: lokale private Konfiguration mit WLAN- und Upload-Zugangsdaten.
- [docu/0950_get_upload_DE.md](docu/0950_get_upload_DE.md): Dokumentation zum GET-Upload.

## Konfiguration

Vor dem Kompilieren muss eine lokale Konfiguration existieren:

```text
secret/config.h
```

Als Startpunkt kann [secret/_placeholder_config.h](secret/_placeholder_config.h) verwendet werden. Die privaten Makros liegen dort zwischen:

```cpp
// Private START
// ...
// Private END
```

Wichtige Makros:

```cpp
#define WIFI_SSID "YourWifiName"
#define WIFI_PASSWORD "YourWifiPassword"
#define LTX_ENDPOINT "http://example.org/ltx/sw/lxu_wug_v1.php"
#define LTX_DEVICE_ID "0000000000000000"
#define LTX_DEVICE_PASSWORD "CHANGE_ME"
```

`secret/config.h` ist absichtlich in `.gitignore` eingetragen. Nur die Platzhalterdatei soll ins Repository.

## Arduino CLI

Der Sketch kann mit `arduino-cli` fuer den XIAO ESP32S3 kompiliert und
hochgeladen werden. Die passende Board-ID ist:

```text
esp32:esp32:XIAO_ESP32S3
```

Aus dem Projektverzeichnis kompilieren:

```powershell
cd C:\c\arduino\wugtest
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 .
```

Kompilieren und direkt auf das Board hochladen, hier beispielhaft auf `COM33`:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --port COM33 --upload .
```

Den aktuellen Port zeigt:

```powershell
arduino-cli board list
```

Wenn die erzeugten Binaerdateien im Sketch-Verzeichnis abgelegt werden sollen:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --export-binaries .
```

Falls der Upload nicht startet, den XIAO ESP32S3 in den Bootloader-Modus
bringen: `BOOT` gedrueckt halten, kurz `RESET` druecken, dann `BOOT` loslassen
und den Upload erneut starten.
