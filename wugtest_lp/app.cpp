/* WugTest Low Power fuer Seeed Studio XIAO ESP32S3 - (C)JoEmbedded.de

  Dieser Sketch sendet einen Temperaturwert an ein LTX/Wunderground-
  aehnliches GET-Upload-Script und nutzt zwischen den Uploads Deep-Sleep.

  Pro Wake-Zyklus wird ein Zaehler im RTC-RAM erhoeht. Der Zaehler bleibt
  ueber Deep-Sleep erhalten, wird bei Reset oder Spannungsverlust aber wieder
  initialisiert.
*/

#include "app.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_sleep.h>
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

RTC_DATA_ATTR static uint32_t eventCounter = 0;

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

static void configureWiFiForUpload()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
}

static void stopWiFi()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
}

static bool connectWiFi()
{
  tb_printf("Verbinde mit WLAN %s", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  configureWiFiForUpload();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    tb_printf(".");
  }

  tb_printf("\n");

  if (WiFi.status() != WL_CONNECTED) {
    tb_printf("WLAN-Verbindung fehlgeschlagen.\n");
    stopWiFi();
    return false;
  }

  tb_printf("WLAN verbunden, IP: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

static float readApproxTemperatureF()
{
  const float tempC = temperatureRead() + TEMP_CALIBRATION_OFFSET_C;
  return (tempC * 9.0f / 5.0f) + 32.0f;
}

static String buildUploadUrl(float tempF, int32_t rssi, uint32_t currentEvent)
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
  url += "&rssi=";
  url += rssi;
  url += "&event=";
  url += currentEvent;

  return url;
}

static void sendTemperature()
{
  if (!connectWiFi()) {
    return;
  }

  const int32_t rssi = WiFi.RSSI();
  const float tempF = readApproxTemperatureF();
  const String url = buildUploadUrl(tempF, rssi, eventCounter);
  char tempBuffer[16];
  snprintf(tempBuffer, sizeof(tempBuffer), "%.2f", tempF);

  tb_printf("Event: %lu\n", static_cast<unsigned long>(eventCounter));
  tb_printf("WLAN RSSI: %ld dBm\n", static_cast<long>(rssi));
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

  if (httpCode > 0) {
    tb_printf("Serverantwort: %s\n", http.getString().c_str());
  } else {
    tb_printf("HTTP-Fehler: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

static void enterDeepSleep()
{
  stopWiFi();

  const uint64_t sleepUs = static_cast<uint64_t>(SEND_INTERVAL_SECONDS) * 1000000ULL;
  esp_sleep_enable_timer_wakeup(sleepUs);

  tb_printf("Deep-Sleep fuer %lu Sekunden.\n", SEND_INTERVAL_SECONDS);
  Serial.flush();
  Serial1.flush();

  esp_deep_sleep_start();
}

// ---------------------------------------------------------------------------
// Arduino-Einstiegspunkte
// ---------------------------------------------------------------------------

void appSetup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // OFF

  tb_init();
  delay(1000);

  ++eventCounter;

  tb_printf("\n");
  tb_printf("WugTest Low Power startet.\n");
  tb_printf("Wakeup-Cause: %d\n", static_cast<int>(esp_sleep_get_wakeup_cause()));

  digitalWrite(LED_BUILTIN, LOW); // ON
  sendTemperature();
  digitalWrite(LED_BUILTIN, HIGH); // OFF

  enterDeepSleep();
}

void appLoop()
{
  // Wird nicht erreicht, weil appSetup() nach dem Upload in Deep-Sleep wechselt.
}
