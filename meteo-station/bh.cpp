#include <Wire.h>
#include <BH1750.h>

#include "bh.hpp"

bool Bh::Initialize() {  
  Wire.begin();
  return _lightMeter.begin();
}

float Bh::GetLux() {
    return _lightMeter.readLightLevel();
}
