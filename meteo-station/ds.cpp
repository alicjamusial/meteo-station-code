#include <OneWire.h>
#include <DallasTemperature.h>

#include "ds.hpp"

bool Ds::Initialize() {  
  _ds.begin();
  return true;
}

float Ds::GetTemperature() {
    _ds.requestTemperatures(); 
    return _ds.getTempCByIndex(0);
}
