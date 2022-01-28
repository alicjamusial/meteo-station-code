#pragma once
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

struct BmeData {
  float temperature;
  float pressure;
  float altitude;
  float humidity;
};

class Bme {
  public:
    bool Initialize();
    BmeData GetData();

  private:
    Adafruit_BME280 _bme; // I2C
};
