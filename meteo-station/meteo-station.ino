#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "data.h"
#include "bme.hpp"
#include "bh.hpp"

const int TimeToSleep = TimeToSleepSeconds * 1000000; /* sleep time converted */

RTC_DATA_ATTR int bootCount = 0; /* is saved between reboots */

bool connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID.c_str(), Password.c_str());
  delay(1000);
  
  int i = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    if (DebugPrints) {
      Serial.println("Trying to connect to WiFi... " + String(WiFi.status()));
    }
    delay(1000);
    if (i > 8) {
      return false;
    }
  }
  return true;
}

void postToInflux(BmeData bmeData, float lux) {
  String metrics = "meteo temperature=" + String(bmeData.temperature, 1) + "," +
                   "pressure=" + String(bmeData.pressure, 1) + "," +
                   "altitude=" + String(bmeData.altitude, 1) + "," +
                   "humidity=" + String(bmeData.humidity, 1) + "," +
                   "lux=" + String(lux, 1) + "," +
                   "boot_nr=" + String(bootCount);

  if (DebugPrints) {
    Serial.println("Data: " + metrics);
  }
  
  HTTPClient http;
  http.begin(Influx);

  int i = 0;
  int status = 0;
  
  while (status != 204) {
    i++;
    status = http.POST(metrics);
    if (DebugPrints) {
      Serial.println("Posting to influx, status: " + String(status));
    }
    if (i > 8) {
      break;
    }
  }
}

void setup() {

  ++bootCount;

  if (DebugPrints) {
    Serial.begin(115200);
    Serial.println("Boot number: " + String(bootCount));
  }

  bool wifiStatus = connectToWifi();
  if (DebugPrints) {
    Serial.println("Wifi status " + String(wifiStatus));
  }

  if (wifiStatus) {
    Bme bme{};
    bool bmeStatus = bme.Initialize();

    Bh bh{};
    bool bhStatus = bh.Initialize();
  
    BmeData bmeData = bme.GetData();
    float lux = bh.GetLux();
 
    postToInflux(bmeData, lux);
  }
 
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
