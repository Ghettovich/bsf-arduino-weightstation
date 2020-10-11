#include <ArduinoJson.h>
#include "src/Recipe.h"

const int etherSS = 53;
const int intervalBroadcastRecipe = 1000;
bool broadcastRecipe = false;
unsigned long delayStartRecipe = 0;

const int serverPort = 5001;
const int maxPayloadSize = 1500;
char jsonPayload[maxPayloadSize];

ReplyWithCode replyCode;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xFE, 0xA7, 0x3D, 0x80, 0xB4, 0xC2
};

IPAddress ip(192, 168, 178, 22);
byte serverIP[] = {192, 168, 178, 242};

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(serverPort);
EthernetClient client;

void setBroadcastRecipe(bool _broadcastRecipe) {
  broadcastRecipe = _broadcastRecipe;
}

void setEthernetPinDefinitions() {
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);
}


// init adapter with auto config
void initEthernetAdapter() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  Serial.println("[BSF Weightstation]");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // Start timer, unsigned long should last around 50 days.
  delayStartRecipe = millis();
}

void receiveEthernetPacketLoop() {

  // listen for incoming clients
  client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '{') {
          int index = 0;

          // Here is where the payload data is.
          while (client.available()) {
            jsonPayload[index] = c;
            c = client.read();
            index++;
          }

          deserializeJsonPayload();
          sendReply();
          break;
        }
      }

    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }


  if (broadcastRecipe && (millis() - delayStartRecipe) >= intervalBroadcastRecipe) {
    sendUpdatedRecipeInfo();

    delayStartRecipe = millis();
    broadcastRecipe = true;
    Serial.println("sendiing recipe..");
  }
}

void sendReply() {

  switch (replyCode) {
    case FULL_STATE_RPLY:
      sendFullStatePayloadPacket();
      break;
    case EMPTY:
      sendEmptyReply();
      break;
    default:
      sendFullStatePayloadPacket();
      break;
  }
}

void deserializeJsonPayload() {
  Serial.println("copying payload...");
  StaticJsonDocument <maxPayloadSize> doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {

    int replyCodeJson = doc["replyCode"];

    if (replyCodeJson) {
      replyCode = identifyReplyCode(replyCodeJson);
    }

    int recipeId = doc["recipeId"];

    if (recipeId) {
      int arduinoId = doc["arduinoid"];
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
    }
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

void sendUpdatedRecipeInfo() {
  char payload[maxPayloadSize];
  StaticJsonDocument <maxPayloadSize> doc;

  JsonObject info = doc.to<JsonObject>();
  JsonArray componentArray = doc.createNestedArray("components");

  // if you get a connection, report back via serial:
  if (client.connect(serverIP, serverPort)) {

    createFullStatePayload(info, componentArray);
    serializeJson(doc, payload);

    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Write to host socket
    client.println(payload);
    client.println();
    client.stop();

  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void sendFullStatePayloadPacket() {
  char payload[maxPayloadSize];
  StaticJsonDocument <maxPayloadSize> doc;

  JsonObject info = doc.to<JsonObject>();
  JsonArray componentArray = doc.createNestedArray("components");

  createFullStatePayload(info, componentArray);
  serializeJson(doc, payload);

  client.println(payload);
}

void sendEmptyReply() {
  client.stop();
}
