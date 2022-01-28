#include "data.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

const int TimeToSleep = TimeToSleepSeconds * 1000000; /* sleep time converted */

WiFiMulti wifiMulti;
RTC_DATA_ATTR int bootCount = 0; /* is saved between reboots */

void connectToWifi() {
  wifiMulti.addAP(SSID.c_str(), Password.c_str());

  for (int i = 0; i < 10 && wifiMulti.run() != WL_CONNECTED; i++) {
    if (DebugPrints) {
      Serial.println("Trying to connect to WiFi... " + String(i));
    }
    delay(200);
  }
}

void postToInflux() {
  String metrics = "meteo temperature=" + String(23.666, 1) + "," +
                   "pressure=" + String(999.2, 1) + "," +
                   "boot_nr=" + String(bootCount);

  HTTPClient http;
  http.begin(Influx);
  int status = http.POST(metrics);

  if (DebugPrints) {
    Serial.println("Posting to influx, status: " + String(status));
  }
}

void setup() {

  ++bootCount;

  if (DebugPrints) {
    Serial.begin(115200);
    Serial.println("Boot number: " + String(bootCount));
  }

  connectToWifi();
  postToInflux();

  esp_sleep_enable_timer_wakeup(TimeToSleep);

  if (DebugPrints) {
    Serial.println("ESP32 is going to sleep for " + String(TimeToSleepSeconds) + " seconds");
    Serial.flush();
  }

  esp_deep_sleep_start();
}

void loop() {
  // not used
}
