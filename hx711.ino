#include "HX711.h"

HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 250;
unsigned long delayStart = 0;

bool delayRunning = false;

Recipe *currentRecipe;// = new Recipe;
int currentComponent = -1;

float *currentWeight;

bool readScale = false;

void setReadScale(bool _readScale) {
  readScale = _readScale;
}

void hx711Setup() {
  delayStart = millis();
  delayRunning = true;

  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);

  scale.begin(HX711_dout, HX711_sck);
  scale.set_scale(110.5);
  //scale.power_down();
//  scale.callibrate_scale(1000, 5);
}

void setCurrentRecipe(Recipe &recipe) {
  currentRecipe = &recipe;
}

void setTareWeight(float &_currentWeight) {
  currentWeight = &_currentWeight;
}

void setCurrentComponent(int selectedComponent) {
  currentComponent = selectedComponent;
}

void tareScaleHx711() {
  scale.tare();
  Serial.print("(tare) UNITS: ");
  tareWeight = scale.get_units(10);
  Serial.println(tareWeight);
}


void calibrateScale() {
  scale.callibrate_scale(1000, 5);
  Serial.print("(calibrate) UNITS: ");
  Serial.println(scale.get_units(3));

  readScale = false;
  currentWeight = NULL;
}

void hx711Loop() {

  if (readScale && currentWeight != NULL
      && (millis() - delayStart) >= interval) {
        
    scale.power_up();
    *currentWeight = scale.get_units(3);

    delayStart = millis();
    delayRunning = true;
  }
  else if (currentComponent != -1) {

    if (delayRunning && (millis() - delayStart) >= interval) {

      scale.power_up();

      currentRecipe->components[currentComponent].currentWeight = (int)scale.get_units(3);

      delayStart = millis();
      delayRunning = true;
    }
  }
}
