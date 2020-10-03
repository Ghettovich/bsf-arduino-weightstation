#include "HX711.h"

HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 250;
unsigned long delayStart = 0;

bool delayRunning = false;

Recipe *currentRecipe;// = new Recipe;
int currentComponent = -1;

int tareWeight = 0;
bool tareScale = false;

void setTareScale(bool _tareScale) {
  tareScale = _tareScale;
}

int getTargeWeight() {
  return tareWeight;
}

void hx711Setup() {
  delayStart = millis();
  delayRunning = true;

  scale.begin(HX711_dout, HX711_sck);
  scale.set_scale(127.15);
  scale.callibrate_scale(1000, 5);
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
