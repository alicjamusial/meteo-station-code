#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define USE_SERIAL Serial


#define SSID ""
#define PASSWORD ""
#define INFLUX ""


WiFiMulti wifiMulti;
RTC_DATA_ATTR int bootCount = 0;


void setup(){
  Serial.begin(115200);

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  wifiMulti.addAP(SSID, PASSWORD);

  for(int i = 0; i < 10 && wifiMulti.run() != WL_CONNECTED; i++) {
    Serial.println("Trying to connect to WiFi... " + String(i));
    delay(1000);
  }

  String metrics = "meteo temperature=" + String(23.666, 1) + "," +
                   "pressure=" + String(999.2, 1) + "," +
                   "boot_nr=" + String(bootCount);

  String dburl = INFLUX;
  Serial.println("Posting to influx");
  
  HTTPClient http;
  http.begin(dburl);
  http.POST(metrics);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop(){
  // not needed
}
