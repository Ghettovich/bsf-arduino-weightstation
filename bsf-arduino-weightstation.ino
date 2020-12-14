#include "HX711.h"

#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <stdint.h>
#include <EthernetENC.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <ArduinoJson.h>
#include "src/Recipe/Recipe.h"

Recipe *recipe = new Recipe;

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
  Serial.println("[BSF Scale 1]");

  setupMqttClient();
  delay(100);
  
  // initialize load cell   
  hx711Setup(); 
  
   
  state = StateCode::READY;
  Serial.println("Ready.");
}

void loop() {
  // check if load has new data
  hx711Loop();
  // check if ether has a pending packet
  mqttLoop();

  prevState = state;
}
