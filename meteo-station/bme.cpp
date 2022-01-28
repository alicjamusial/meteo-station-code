#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "bme.hpp"

bool Bme::Initialize() {
    return _bme.begin(0x76);
}

BmeData Bme::GetData() {
    BmeData data{};
    data.temperature = _bme.readTemperature();
    data.pressure = _bme.readPressure() / 100.0F;
    data.altitude = _bme.readAltitude(SEALEVELPRESSURE_HPA);
    data.humidity = _bme.readHumidity();
    return data;
}
