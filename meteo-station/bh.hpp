#pragma once
#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

class Bh {
  public:
    bool Initialize();
    float GetLux();

  private:
    BH1750 _lightMeter;
};
