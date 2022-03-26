#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const int OneWireBus = 4;

class Ds {
public:
    bool Initialize();

    float GetTemperature();

private:
    OneWire _oneWire{OneWireBus};
    DallasTemperature _ds{&_oneWire};
};
