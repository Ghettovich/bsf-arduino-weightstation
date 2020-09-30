#include "HX711.h"

HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 250;
unsigned long delayStart = 0;

bool delayRunning = false;

Recipe *currentRecipe;// = new Recipe;
int currentComponent = -1;

void hx711Setup() {
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);

  delayStart = millis();
  delayRunning = true;

  scale.begin(HX711_dout, HX711_sck);
  scale.set_scale(127.15);
}

void setCurrentRecipe(Recipe &recipe) {
  currentRecipe = &recipe;
}

void setCurrentComponent(int selectedComponent) {
  currentComponent = selectedComponent;
}


void hx711Loop() {

  if (currentComponent != -1) {
    
    if (delayRunning && (millis() - delayStart) >= interval) {

      scale.power_up();

      currentRecipe->components[currentComponent].currentWeight = (int)scale.get_units(3);      

      delayStart = millis();
      delayRunning = true;
    }
  }
}
