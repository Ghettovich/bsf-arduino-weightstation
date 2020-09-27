#include "HX711.h"

HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 1000;
unsigned long delayStart = 0;

bool delayRunning = false;

void hx711SetupUp() {
  delayStart = millis();
  delayRunning = true;

  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(HX711_dout, HX711_sck);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(5));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  delay(100);

//  Serial.print("get units: \t\t");
//  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)
//
//  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
//  scale.tare();               // reset the scale to 0
//
//  Serial.println("After setting up the scale:");
//
//  Serial.print("read: \t\t");
//  Serial.println(scale.read());                 // print a raw reading from the ADC
//
//  Serial.print("read average: \t\t");
//  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC
//
//  Serial.print("get value: \t\t");
//  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()
//
//  Serial.print("get units: \t\t");
//  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale
}


void hx711Loop() {

  if (delayRunning && (millis() - delayStart) >= 2000) {

    scale.power_up();    
    
    Serial.print("one reading:\t");
    Serial.println(scale.get_units(), 1);
    
    delayStart = millis();
    delayRunning = true;
  }
  
}
