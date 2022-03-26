#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "esp_adc_cal.h"

#include "data.h"
#include "bme.hpp"
#include "bh.hpp"
#include "ds.hpp"

const int TimeToSleep = TimeToSleepSeconds * 1000000; /* sleep time converted */
const int TimeToSleep2 = 120 * 1000000; /* sleep time converted */


const int ledPin = 2;

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
  WiFi.setSleep(true);
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

  setCpuFrequencyMhz(80);

  delay(1000);
  setupAdc();
  uint32_t voltage;
  esp_adc_cal_get_voltage(VoltageChannel, &VoltageCharacteristics, &voltage);
  
  Serial.begin(115200);
  Serial.println("AAAA");
  Serial.println(voltage / 1000.0 * 2);
   Serial.println("Freq: " + String(getCpuFrequencyMhz()));
    
//  if (voltage / 1000.0 * 2 > 2.6)
//  {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
  
    if (DebugPrints) {
      Serial.println("Boot number: " + String(bootCount));
    }
//  
bool wifiStatus = connectToWifi();
    if (DebugPrints) {
      Serial.println("Wifi status " + String(wifiStatus));
    }
  
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

//      BmeData bmeData;
//      bmeData.temperature = 12.5;
//      bmeData.pressure = 1015.1;
//      bmeData.altitude = 200.0;
//      bmeData.humidity = 33.1;
//      float lux = 1111;
//      float tempDs = 15.4;
   
      postToInflux(bmeData, lux, tempDs);
    }
//
    adc_power_off();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_enable_timer_wakeup(TimeToSleep);
  
    if (DebugPrints) {
      Serial.println("ESP32 is going to sleep for " + String(TimeToSleepSeconds) + " seconds");
      Serial.flush();
    }
  
    esp_deep_sleep_start();
//  }
//  else {
//   
//    esp_sleep_enable_timer_wakeup(TimeToSleep2);
//    Serial.flush();
//    esp_deep_sleep_start();
//  }
  
}

void loop() {
  // not used
}
