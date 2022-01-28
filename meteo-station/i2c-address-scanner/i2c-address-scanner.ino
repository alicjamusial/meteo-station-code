#include <Wire.h>

void setup()
{
  Serial.begin(115200);  
  Wire.begin(21, 22);  // SDA - GPIO_21, SCL - GPIO_22
}

void Scan()
{
  Serial.println("Scanning I2C addresses...");
  int count = 0;
  Wire.begin();
  for (int i = 8; i < 120; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0
    {
      Serial.print("Found: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println (")");
      count++;
    }
  }
  Serial.print("Found ");      
  Serial.print(count, DEC);
  Serial.println(" devices.");
}

void loop()
{
  Scan();
  delay(10000);
}
