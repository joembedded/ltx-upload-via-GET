# WugTest - (C)JoEmbedded - Stand 29.04.2026

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
- [docu/0950_get_upload_DE.md](docu/0950_get_upload_DE.md): aufbereitete Dokumentation zum GET-Upload.

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
#define SEND_INTERVAL_SECONDS 60UL
```

`secret/config.h` ist absichtlich in `.gitignore` eingetragen. Nur die Platzhalterdatei soll ins Repository.
