#pragma once

// Private START
#define WIFI_SSID "YourWifiName"
#define WIFI_PASSWORD "YourWifiPassword"

#define LTX_ENDPOINT "http://example.org/ltx/sw/lxu_wug_v1.php"
#define LTX_DEVICE_ID "0000000000000000"
#define LTX_DEVICE_PASSWORD "CHANGE_ME"

// Laut Doku ist "tempf" der GET-Parameter fuer Temperaturwerte in Fahrenheit.
#define LTX_TEMP_PARAMETER "tempf"

// Upload-Intervall in Sekunden. Default: 60 Sekunden.
#define SEND_INTERVAL_SECONDS 60UL

// Der interne ESP32-S3-Sensor misst die Chiptemperatur, nicht die Raumluft.
// Mit diesem Offset kann der Wert grob angepasst werden.
#define TEMP_CALIBRATION_OFFSET_C 0.0f

// Timeouts halten den Sketch handhabbar, falls WLAN oder Server nicht reagieren.
#define WIFI_CONNECT_TIMEOUT_MS 20000UL
#define HTTP_REQUEST_TIMEOUT_MS 10000UL
// Private END
