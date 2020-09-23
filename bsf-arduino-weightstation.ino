#include <stdint.h>
#include "src/Recipe.h"

Recipe * pRecipe = nullptr;

void setup() {
  Serial.begin(115200);  
    
  // set pins manually
  setTFTPinDefinitions();
  setEthernetPinDefinitions();

  // initialize load cell 
  hx711SetupUp();  

  // initialize hardware
  initTFTouchScreen();
  initEthernetAdapter();

  updateDisplay();
  
  Serial.println("Ready.");
}

void loop() {
  // check if load has new data
  hx711Loop();
  // check if ether has a pending packet
  receiveEtherPacket();
  // a point object holds x y and z coordinates.
  processTouch();

}
