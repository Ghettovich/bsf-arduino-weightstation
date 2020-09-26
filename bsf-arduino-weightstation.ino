#include <stdint.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"


int state = 0, prevState = 0;
enum StateCode {
  READY = 0, RECIPE_SET, RECIPE_TARGET_UNDERFLOW, RECIPE_TARGET_OVERFLOW
};

void updateState(int newState) {
  int oldState = state;
  state = newState;
  prevState = oldState;
}

int getCurrentState() {
  return state;
}

void setup() {
  Serial.begin(115200);  
    
  // set pins manually
  setTFTPinDefinitions();
  setEthernetPinDefinitions();

  // initialize load cell 
  hx711SetupUp();  

  // initialize hardware
  initTFTouchScreen();
  delay(500);
  initEthernetAdapter();
  delay(500);
  updateDisplay();

  state = StateCode::READY;
  Serial.println("Ready.");
}

void loop() {
  // check if load has new data
  hx711Loop();
  // check if ether has a pending packet
  receiveEtherPacket();
  // a point object holds x y and z coordinates.
  displayLoop();

  prevState = state;
}
