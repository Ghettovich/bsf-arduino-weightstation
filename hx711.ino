#include "HX711.h"

HX711 scale;

const int HX711_dout = 18; //mcu > HX711 dout pin, must be external interrupt capable!
const int HX711_sck = 49; //mcu > HX711 sck pin

const int interval = 1000;
unsigned long currentMillis = 0, prevMillis = 0;

void hx711SetupUp() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  scale.begin(HX711_dout, HX711_sck);

  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nEmpty the scale, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  scale.tare();
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));


  Serial.println("\nPut a 1 kg in the scale, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  scale.callibrate_scale(1000, 5);
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nScale is callibrated, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  scale.set_unit_price(0.031415);  // we only have one price

  digitalWrite(HX711_sck, HIGH);
}


void hx711Loop() {
  currentMillis = millis();
  
  if (currentMillis + interval > prevMillis) {
    digitalWrite(HX711_sck, LOW);
    digitalWrite(HX711_dout, LOW);
    Serial.print("UNITS: ");
    Serial.print(scale.get_units(3));
    Serial.print("\t\tPRICE: ");
    Serial.println(scale.get_price(3));
    digitalWrite(HX711_sck, HIGH);
    digitalWrite(HX711_dout, HIGH);

    prevMillis = currentMillis;
  }
}
