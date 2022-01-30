#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "esp_adc_cal.h"

#include "data.h"
#include "bme.hpp"
#include "bh.hpp"
#include "ds.hpp"

const int TimeToSleep = TimeToSleepSeconds * 1000000; /* sleep time converted */

const uint32_t ReferenceVoltage = 1100;
const int VoltageMeasurementPin = 34;
const adc_channel_t VoltageChannel = ADC_CHANNEL_6;
const adc_unit_t VoltageUnit = ADC_UNIT_1;
const adc_atten_t VoltageAtten = ADC_ATTEN_DB_11;
const adc_bits_width_t VoltageWidth = ADC_WIDTH_BIT_12;
esp_adc_cal_characteristics_t VoltageCharacteristics;

RTC_DATA_ATTR int bootCount = 0; /* is saved between reboots */

void setupAdc() {
  adc1_config_width(VoltageWidth);
  adc1_config_channel_atten((adc1_channel_t)VoltageChannel, VoltageAtten);
  esp_adc_cal_characterize(VoltageUnit, VoltageAtten, VoltageWidth, ReferenceVoltage, &VoltageCharacteristics);
}

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

void postToInflux(BmeData bmeData, float lux, float tempDs) {
  uint32_t voltage;
  esp_adc_cal_get_voltage(VoltageChannel, &VoltageCharacteristics, &voltage);
  
  String metrics = "meteo temperature=" + String(bmeData.temperature, 1) + "," +
                   "pressure=" + String(bmeData.pressure, 1) + "," +
                   "altitude=" + String(bmeData.altitude, 1) + "," +
                   "humidity=" + String(bmeData.humidity, 1) + "," +
                   "lux=" + String(lux, 1) + "," +
                   "temperature_ds=" + String(tempDs, 1) + "," +
                   "voltage=" + String(voltage / 1000.0 * 2) + "," +
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
  
  setupAdc();

  if (wifiStatus) {
    Bme bme{};
    bool bmeStatus = bme.Initialize();

    Bh bh{};
    bool bhStatus = bh.Initialize();

    Ds ds{};
    bool dsStatus = ds.Initialize();
  
    BmeData bmeData = bme.GetData();
    float lux = bh.GetLux();
    float tempDs = ds.GetTemperature();
 
    postToInflux(bmeData, lux, tempDs);
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
