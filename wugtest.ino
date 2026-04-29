#include <WiFi.h>
#include <HTTPClient.h>

/*
  WugTest fuer Seeed Studio XIAO ESP32S3 - (C)JoEmbedded.de

  Dieser Sketch sendet einen Temperaturwert an ein LTX/Wunderground-
  aehnliches GET-Upload-Script.

  Bezug zur Doku docu/0950_get_upload_DE.md:
  - lxu_wug_v1.php nimmt Messwerte per HTTP-GET entgegen.
  - ID und PASSWORD sind die Zugangsdaten in der URL.
  - Der Parameter tempf ist in der LTX-Parameterliste als Temperaturwert
    im Rohformat Fahrenheit definiert.
  - Ein Upload-Intervall von 1 Minute ist ein typischer Wert.
*/

// ---------------------------------------------------------------------------
// Konfiguration
// ---------------------------------------------------------------------------

// Die privaten Werte liegen in secret/config.h.
// Als Vorlage gibt es secret/_placeholder_config.h.

#include "secret/config.h"

// ---------------------------------------------------------------------------
// Laufzeitstatus
// ---------------------------------------------------------------------------

static unsigned long lastSendMs = 0;

// ---------------------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------------------

static bool connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.print("Verbinde mit WLAN ");
  Serial.print(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WLAN-Verbindung fehlgeschlagen.");
    return false;
  }

  Serial.print("WLAN verbunden, IP: ");
  Serial.println(WiFi.localIP());
  return true;
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

  const float tempF = readApproxTemperatureF();
  const String url = buildUploadUrl(tempF);

  Serial.print("Temperatur ungefaehr: ");
  Serial.print(tempF, 2);
  Serial.println(" F");

  Serial.print("Upload: ");
  Serial.println(url);

  HTTPClient http;
  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  if (!http.begin(url)) {
    Serial.println("HTTP-Client konnte URL nicht oeffnen.");
    return;
  }

  const int httpCode = http.GET();

  Serial.print("HTTP-Status: ");
  Serial.println(httpCode);

  // Die Doku beschreibt, dass einfache Sender die Serverantwort oft ignorieren.
  // Fuer die Entwicklung geben wir sie trotzdem seriell aus.
  if (httpCode > 0) {
    Serial.print("Serverantwort: ");
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP-Fehler: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}

// ---------------------------------------------------------------------------
// Arduino-Einstiegspunkte
// ---------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("WugTest startet.");

  sendTemperature();
  lastSendMs = millis();
}

void loop()
{
  const unsigned long intervalMs = SEND_INTERVAL_SECONDS * 1000UL;

  if (millis() - lastSendMs >= intervalMs) {
    lastSendMs = millis();
    sendTemperature();
  }
}
