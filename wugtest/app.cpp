/* WugTest fuer Seeed Studio XIAO ESP32S3 - (C)JoEmbedded.de

  Dieser Sketch sendet einen Temperaturwert an ein LTX/Wunderground-
  aehnliches GET-Upload-Script.

  Bezug zur Doku docu/0950_get_upload_DE.md:
  - lxu_wug_v1.php nimmt Messwerte per HTTP-GET entgegen.
  - ID und PASSWORD sind die Zugangsdaten in der URL.
  - Der Parameter tempf ist in der LTX-Parameterliste als Temperaturwert
    im Rohformat Fahrenheit definiert.
  - Ein Upload-Intervall von 1 Minute ist ein typischer Wert.
  - der XIAO ESP32S3 benoetigt hier ca. 35 mA @ 5V
*/

#include "app.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <stdarg.h>

// ---------------------------------------------------------------------------
// Konfiguration
// ---------------------------------------------------------------------------

// Die privaten Werte liegen in ../secret/config.h.
// Als Vorlage gibt es ../secret/_placeholder_config.h.
#include "../secret/config.h"

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

// ---------------------------------------------------------------------------
// Laufzeitstatus
// ---------------------------------------------------------------------------

#define UART_TX 43  // D6
#define UART_RX 44  // D7
#define WIFI_RECONNECT_CHECK_MS 5000UL

static unsigned long lastSendMs = 0;

// ---------------------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------------------

static void tb_printf(const char *fmt, ...)
{
  static char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  Serial1.print(buf);
  Serial.print(buf);
}

static void tb_init()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  tb_printf("\n\nBoard initialisiert.\n");
}

static void configureWiFiPowerSave()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
}

static void waitLowPower(unsigned long waitMs)
{
  if (waitMs == 0) {
    return;
  }

  delay(waitMs);
}

static bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  tb_printf("Verbinde mit WLAN %s", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  configureWiFiPowerSave();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    waitLowPower(500);
    tb_printf(".");
  }

  tb_printf("\n");

  if (WiFi.status() != WL_CONNECTED) {
    tb_printf("WLAN-Verbindung fehlgeschlagen.\n");
    return false;
  }

  tb_printf("WLAN verbunden, IP: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

static unsigned long sendIntervalMs()
{
  return SEND_INTERVAL_SECONDS * 1000UL;
}

static void waitUntilNextSend()
{
  const unsigned long intervalMs = sendIntervalMs();

  while (millis() - lastSendMs < intervalMs) {
    const unsigned long elapsedMs = millis() - lastSendMs;
    const unsigned long remainingMs = intervalMs - elapsedMs;
    const unsigned long sleepMs = min(remainingMs, WIFI_RECONNECT_CHECK_MS);

    waitLowPower(sleepMs);

    if (WiFi.status() != WL_CONNECTED) {
      tb_printf("WLAN-Verbindung verloren.\n");
      connectWiFi();
    }
  }
}

static float readApproxTemperatureF()
{
  const float tempC = temperatureRead() + TEMP_CALIBRATION_OFFSET_C;
  return (tempC * 9.0f / 5.0f) + 32.0f;
}

static String buildUploadUrl(float tempF)
{
  char tempBuffer[16];
  snprintf(tempBuffer, sizeof(tempBuffer), "%.2f", tempF);

  // Format laut Doku:
  // .../lxu_wug_v1.php?ID=<id>&PASSWORD=<key>&tempf=<fahrenheit>
  String url = String(LTX_ENDPOINT);
  url += "?ID=";
  url += LTX_DEVICE_ID;
  url += "&PASSWORD=";
  url += LTX_DEVICE_PASSWORD;
  url += "&";
  url += LTX_TEMP_PARAMETER;
  url += "=";
  url += tempBuffer;

  return url;
}

static void sendTemperature()
{
  if (!connectWiFi()) {
    return;
  }

  tb_printf("WLAN RSSI: %d dBm\n", WiFi.RSSI());

  const float tempF = readApproxTemperatureF();
  const String url = buildUploadUrl(tempF);
  char tempBuffer[16];
  snprintf(tempBuffer, sizeof(tempBuffer), "%.2f", tempF);

  tb_printf("Temperatur ungefaehr: %s F\n", tempBuffer);
  tb_printf("Upload: %s\n", url.c_str());

  HTTPClient http;
  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  if (!http.begin(url)) {
    tb_printf("HTTP-Client konnte URL nicht oeffnen.\n");
    return;
  }

  const int httpCode = http.GET();

  tb_printf("HTTP-Status: %d\n", httpCode);

  // Die Doku beschreibt, dass einfache Sender die Serverantwort oft ignorieren.
  // Fuer die Entwicklung geben wir sie trotzdem seriell aus.
  if (httpCode > 0) {
    tb_printf("Serverantwort: %s\n", http.getString().c_str());
  } else {
    tb_printf("HTTP-Fehler: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

// ---------------------------------------------------------------------------
// Arduino-Einstiegspunkte
// ---------------------------------------------------------------------------

void appSetup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  tb_init();
  delay(1000);

  tb_printf("\n");
  tb_printf("WugTest startet.\n");

  WiFi.mode(WIFI_STA);
  configureWiFiPowerSave();
}

void appLoop()
{
  lastSendMs = millis();
  digitalWrite(LED_BUILTIN, LOW); // ON
  sendTemperature();
  digitalWrite(LED_BUILTIN, HIGH); // OFF
  waitUntilNextSend();
}
