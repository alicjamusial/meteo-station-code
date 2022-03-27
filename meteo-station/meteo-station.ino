#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "esp_adc_cal.h"

#include "data.h"
#include "bme.hpp"
#include "bh.hpp"
#include "ds.hpp"

// Converted time from configuration
const int TimeToSleep = TimeToSleepSeconds * 1000000;
const int TimeToSleepIfLowBattery = TimeToSleepSecondsIfBatteryLow * 1000000;

const int LedPin = 2;

// ADC constants to be able to measure voltage
const uint32_t ReferenceVoltage = 1100;
const int VoltageMeasurementPin = 34;
const adc_channel_t VoltageChannel = ADC_CHANNEL_6;
const adc_unit_t VoltageUnit = ADC_UNIT_1;
const adc_atten_t VoltageAtten = ADC_ATTEN_DB_11;
const adc_bits_width_t VoltageWidth = ADC_WIDTH_BIT_12;
esp_adc_cal_characteristics_t VoltageCharacteristics;

void setupAdc() {
    adc1_config_width(VoltageWidth);
    adc1_config_channel_atten((adc1_channel_t) VoltageChannel, VoltageAtten);
    esp_adc_cal_characterize(VoltageUnit, VoltageAtten, VoltageWidth, ReferenceVoltage, &VoltageCharacteristics);
}

float convertVoltage(uint32_t voltage) {
    return voltage / 1000.0 * 2;
}

bool connectToWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID.c_str(), Password.c_str());
    delay(1000); // ugly hack but works

    int i = 0;

    while (WiFi.status() != WL_CONNECTED) {
        i++;
        if (IsDebug) {
            Serial.println("Trying to connect to WiFi... " + String(WiFi.status()));
        }
        delay(1000);
        if (i > 8) { // try to connect 8 times, there is no sense in more (usually 1-4 is enough)
            return false;
        }
    }
    WiFi.setSleep(true);
    return true;
}

void postToInflux(BmeData bmeData, float lux, float tempDs, uint32_t voltage) {

    String metrics = "meteo temperature=" + String(bmeData.temperature, 1) + "," +
                     "pressure=" + String(bmeData.pressure, 1) + "," +
                     "altitude=" + String(bmeData.altitude, 1) + "," +
                     "humidity=" + String(bmeData.humidity, 1) + "," +
                     "lux=" + String(lux, 1) + "," +
                     "temperature_ds=" + String(tempDs, 1) + "," +
                     "voltage=" + String(convertVoltage(voltage));

    if (IsDebug) {
        Serial.println("Data: " + metrics);
    }

    HTTPClient http;
    http.begin(Influx);

    int i = 0;
    int status = 0;

    while (status != 204) {
        i++;
        status = http.POST(metrics);
        if (IsDebug) {
            Serial.println("Posting to influx, status: " + String(status));
        }
        if (i > 8) {
            break;
        }
    }
}

void setup() {

    setCpuFrequencyMhz(80); // low, 'cause we want long lifetime :)
    delay(1000); // just another one ugly hack

    setupAdc();

    uint32_t voltage;
    esp_adc_cal_get_voltage(VoltageChannel, &VoltageCharacteristics, &voltage);

    if (convertVoltage(voltage) > MinimumBatteryVoltage || IsDebug) {

        if (IsDebug) {
            Serial.begin(115200);
            pinMode(LedPin, OUTPUT);
            digitalWrite(LedPin, HIGH);
        }

        bool wifiStatus = connectToWifi();
        if (IsDebug) {
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

            postToInflux(bmeData, lux, tempDs, voltage);
        }

        adc_power_off();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);

        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
        esp_sleep_enable_timer_wakeup(TimeToSleep);

        if (IsDebug) {
            Serial.println("ESP32 is going to sleep for " + String(TimeToSleepSeconds) + " seconds");
            Serial.flush();
        }

        esp_deep_sleep_start(); // bye bye sweet angel

    } else {

        // very basic kind of hysteresis to try to avoid boot loop if there's low battery level
        // (theoretically TP4056 should do that but ESP32 likes to consume a lot of power on start,
        // so there's a peak, and there are problems)

        esp_sleep_enable_timer_wakeup(TimeToSleepIfLowBattery);
        if (IsDebug) {
            Serial.flush();
        }
        esp_deep_sleep_start();

    }

}

void loop() {
    // not used
}
