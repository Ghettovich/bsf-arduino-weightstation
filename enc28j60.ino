#include <EtherSia.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"

const int intervalBroadcastRecipe = 1000;
bool broadcastRecipe = false;
unsigned long delayStartRecipe = 0;

const int etherSS = 53, port = 6677;
const char *serverIP = "2a02:a213:9f81:4e80:2aab:51a2:d551:1c33";
                              

// ENC28J60 Ethernet Interface
EtherSia_ENC28J60 ether(etherSS);

// Define UDP socket with ether and port
UDPSocket udp(ether, port);

// HTTP server
HTTPServer http(ether);

void setBroadcastRecipe(bool _broadcastRecipe) {
  broadcastRecipe = _broadcastRecipe;
}

void setEthernetPinDefinitions() {
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);
}


// init adapter with auto config
void initEthernetAdapter() {
  // Ethernet adapter
  MACAddress macAddress("2e:7d:fd:96:c3:98");
  Serial.println("[BSF-WeightStation]");
  macAddress.println();

  //  // Start Ethernet
  if (ether.begin(macAddress) == false) {
    Serial.println("Failed to configure Ethernet");
  }

  if (udp.setRemoteAddress(serverIP, port)) {
    Serial.print("Remote address: ");
    udp.remoteAddress().println();
  }

  Serial.print("Our link-local address is: ");
  ether.linkLocalAddress().println();
  Serial.print("Our global address is: ");
  ether.globalAddress().println();

  // Start timer, unsigned long should last around 50 days.
  delayStartRecipe = millis();
}

void etherLoop() {
  if (ether.receivePacket()) {

    // HTTP endpoints
    if (http.isGet(F("/"))) {
      replyWithFullStatePayload();
    }

    // UDP
    if (udp.havePacket()) {
      Serial.print("Received UDP from: ");
      udp.packetSource().println();

      Serial.print("Packet length: ");
      Serial.println(udp.payloadLength(), DEC);

      deserializePayload();
      udpReplyWithFullState();      
    }

    if (broadcastRecipe && (millis() - delayStartRecipe) >= intervalBroadcastRecipe) {
      Serial.println("sendiing reciipe payload.");

      broadcastUpdatedRecipe();

      delayStartRecipe = millis();
      broadcastRecipe = true;
    }
    else {
      ether.rejectPacket();
    }
  }
}


void deserializePayload() {
  Serial.println("copying payload...");
  StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
  DeserializationError error = deserializeJson(doc, udp.payload());

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {
    int arduinoId = doc["arduinoid"];
    int recipeId = doc["recipeId"];
    int componentSize = doc["componentSize"];

    if (recipeId) {
      setRecipeId(recipeId);
    }

    JsonArray components = doc["components"];

    for (int i = 0; i < componentSize; i++) {
      int componentId = components[i]["id"]; // 1
      int targetWeight = components[i]["weight"]; // 100

      if (componentId && targetWeight) {
        insertComponentWithIdAndWeight(componentId, targetWeight);
      }
    }

    updateState(StateCode::RECIPE_SET);
    updateDisplayStatus(displayRecipeStates::START_WITH_RECIPE);    

    // Set loop to broadcast udp payload to host
    setBroadcastRecipe(true);
  }
}

void createFullStatePayload(JsonObject info, JsonArray items) {
  info["arduinoId"] = recipe->arduinoId;
  info["state"] = getCurrentState();
  info["iodeviceId"] = recipe->iodeviceId;
  info["typeId"] = recipe->typeId;
  info["recipeId"] = recipe->recipeId;
  info["low"] = 1;
  info["selecComp"] = getSelectedComponent();

  addRecipeComponentsToJsonArray(items);
}

void udpReplyWithFullState() {
  StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
  char payload[ETHERSIA_MAX_PACKET_SIZE];

  JsonObject info = doc.to<JsonObject>();
  JsonArray componentArray = doc.createNestedArray("components");

  createFullStatePayload(info, componentArray);
  serializeJson(doc, payload);

  udp.sendReply(payload);
}

void broadcastUpdatedRecipe() {
  StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
  char payload[ETHERSIA_MAX_PACKET_SIZE];

  JsonObject info = doc.to<JsonObject>();
  JsonArray componentArray = doc.createNestedArray("components");

  createFullStatePayload(info, componentArray);
  serializeJson(doc, payload);

  if (!udp.remoteAddress()) {
    udp.setRemoteAddress(serverIP, port);
    udp.remoteAddress().println();
  } else {
    udp.remoteAddress().println();
  }

  Serial.println("Broadcasting payload to host");
  Serial.println(payload);

  udp.println(payload);
  udp.send();
}

void replyWithFullStatePayload() {
  StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
  char payload[ETHERSIA_MAX_PACKET_SIZE];

  JsonObject info = doc.to<JsonObject>();
  JsonArray componentArray = doc.createNestedArray("components");

  createFullStatePayload(info, componentArray);
  serializeJson(doc, payload);


  Serial.println("printing payload on TCP reply");
  Serial.println(payload);

  http.printHeaders(http.typeJson);
  http.println(payload);
  http.sendReply();
}
