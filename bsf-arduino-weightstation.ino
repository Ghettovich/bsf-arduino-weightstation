#include <stdint.h>
#include <EthernetENC.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"
#include "src/Display.h"
#include "src/ReplyEnum.h"



int state = 0, prevState = 0;
enum StateCode {
  READY = 0, RECIPE_SET = 20, RECIPE_TARGETS_MET = 21
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
  Serial.begin(57600);  
    
  // set pins manually
  setEthernetPinDefinitions();
  setTFTPinDefinitions();
  
  // initialize load cell 
  hx711Setup(); 
  delay(100);
  
  // initialize hardware
  initTFTouchScreen();  
  updateDisplay(); 

  delay(100);
  initEthernetAdapter(); 
  

  state = StateCode::READY;
  Serial.println("Ready.");
}

void loop() {
  // check if load has new data
  hx711Loop();
  // check if ether has a pending packet
  receiveEthernetPacketLoop();
  // a point object holds x y and z coordinates.
  displayLoop();

  prevState = state;
}
