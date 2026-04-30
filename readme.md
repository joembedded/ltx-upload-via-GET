# WugTest - (C)JoEmbedded

**Stand:** 30.04.2026

Arduino-Sketch für einen Uploader auf [LTX_SERVER](https://github.com/joembedded/LTX_server) via GET-UPLOAD über einen XIAO ESP32S3.

Die Sketches verbinden sich mit WLAN und senden in einem per Makro einstellbaren Intervall einen ungefaehren Temperaturwert an einen LTX/Wunderground-kompatiblen Upload-Endpunkt.

Neben der normalen Dauerlauf-Version unter `wugtest/` gibt es mit `wugtest_lp/` eine Low-Power-Version. Sie wacht nur zum Upload auf, verbindet sich dann mit dem WLAN, uebertraegt Temperatur, RSSI und einen Deep-Sleep-Eventzaehler und meldet sich danach wieder vom WLAN ab, bevor der ESP32 in Deep-Sleep geht.

In beiden Sketch-Ordnern ist die jeweilige `NAME.ino` nur der Arduino-Einstieg mit `setup()` und `loop()`. Die eigentliche Logik liegt in `app.cpp`/`app.h`, damit Formatierung und "Gehe zu Definition/Deklaration" wie bei normalen C++-Dateien funktionieren.

Die Doku [docu/0950_get_upload_DE.md](docu/0950_get_upload_DE.md) beschreibt das GET-Upload-Format:

```text
http://server.example/ltx/sw/lxu_wug_v1.php?ID=0000000000000000&PASSWORD=CHANGE_ME&tempf=61.70
```

`ID` und `PASSWORD` sind Zugangsdaten. Der Temperaturwert wird als Fahrenheit-Wert im Parameter `tempf` uebertragen.

Die Low-Power-Version haengt zusaetzlich `rssi` und `event` an die URL an:

```text
http://server.example/ltx/sw/lxu_wug_v1.php?ID=0000000000000000&PASSWORD=CHANGE_ME&tempf=61.70&rssi=-67&event=42
```

Der Eventzaehler liegt im RTC-RAM des ESP32 und bleibt ueber Deep-Sleep-Zyklen erhalten. Bei Reset oder Spannungsverlust startet er wieder neu.

## Dateien

- [wugtest/wugtest.ino](wugtest/wugtest.ino): Einstieg fuer die Dauerlauf-Version.
- [wugtest/app.cpp](wugtest/app.cpp): Logik fuer WLAN-Verbindung und HTTP-GET-Upload.
- [wugtest_lp/wugtest_lp.ino](wugtest_lp/wugtest_lp.ino): Einstieg fuer die Low-Power-Version.
- [wugtest_lp/app.cpp](wugtest_lp/app.cpp): Logik fuer Deep-Sleep, WLAN-Upload, RSSI und Eventzaehler.
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

## Arduino CLI - Schnellstart

Der Sketch kann mit `arduino-cli` fuer den XIAO ESP32S3 kompiliert und
hochgeladen werden. Die passende Board-ID ist:

```text
esp32:esp32:XIAO_ESP32S3
```

Die Dauerlauf-Version ist ein eigener Sketch-Ordner:

```powershell
cd C:\c\arduino\wugtest
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 .\wugtest
```

Kompilieren und direkt auf das Board hochladen, hier beispielhaft auf `COM33`:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --port COM33 --upload .\wugtest
```

Den aktuellen Port zeigt:

```powershell
arduino-cli board list
```

Wenn die erzeugten Binaerdateien im Sketch-Verzeichnis abgelegt werden sollen:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --export-binaries .\wugtest
```

Die Low-Power-Version liegt ebenfalls in einem eigenen Sketch-Ordner:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 .\wugtest_lp
```

Upload der Low-Power-Version:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32S3 --port COM33 --upload .\wugtest_lp
```

Falls der Upload nicht startet, den XIAO ESP32S3 in den Bootloader-Modus
bringen: `BOOT` gedrueckt halten, kurz `RESET` druecken, dann `BOOT` loslassen
und den Upload erneut starten.
