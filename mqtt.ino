const long pingInterval = 30000;
unsigned long delayStart = 0;
bool flagPublishRecipeData = false, delayPingStart = false;
byte macMqtt[] = {0xFE, 0xA7, 0x3D, 0x80, 0xB4, 0xC2};

#define SERVER   "192.168.178.242"
#define PORT     1883

const char recipeDataTopic[] = "/recipe/data";
const char recipeConfigSub[] = "/config/recipe";

//Set up the ethernet client
EthernetClient client;
Adafruit_MQTT_Client mqtt(&client, SERVER, PORT);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// Setup a feed for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish recipeDataPublish = Adafruit_MQTT_Publish(&mqtt, recipeDataTopic, MQTT_QOS_1);

// Setup a feed for subscribing to changes.
Adafruit_MQTT_Subscribe configRecipeSubcription = Adafruit_MQTT_Subscribe(&mqtt, recipeConfigSub, MQTT_QOS_1);

void setPublishRecipeData(bool publishData) {
  flagPublishRecipeData = true;
}

void setupMqttClient() {
  Serial.begin(57600);

  Serial.println(F("Adafruit MQTT demo"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(macMqtt);
  delay(1000); //give the ethernet a second to initialize

  //delayStart = millis();
  delayPingStart = true;

  configRecipeSubcription.setCallback(configRecipeCallback);

  mqtt.subscribe(&configRecipeSubcription);
}

uint32_t x = 0;

void mqttLoop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(1000);


  if (delayPingStart &&
      (millis() - delayStart) >= pingInterval) {
    // ping the server to keep the mqtt connection alive
    if (! mqtt.ping()) {
      mqtt.disconnect();
    }

    delayStart = millis();
    delayPingStart = true;
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void configRecipeCallback(char *data, uint16_t len) {
  Serial.println("Call back triggered");
  deserializeConfigRecipe(data, len);
}

void deserializeConfigRecipe(char *data, uint16_t len) {
  DynamicJsonDocument doc(len);
  deserializeJson(doc, data);

  int recipeId = doc["recipeId"];
  int componentId = doc["componentId"]; // 1
  int targetWeight = doc["targetWeight"]; // 100


  if (recipeId && componentId) {
    recipe = new Recipe(recipeId);

    recipe->addComponent(componentId, targetWeight);
    updateState(StateCode::RECIPE_SET);
    setDelayRunning(true);
    setCurrentRecipe(*recipe);
  }
}

void publishRecipeData() {
  //const size_t capacity = JSON_OBJECT_SIZE(4);
  char payload[64];
  DynamicJsonDocument doc(64);

  doc["did"] = iodeviceId;
  doc["rid"] = recipe->recipeId;
  doc["cid"] = recipe->getCurrentComponentId();
  doc["weight"] = 1337;

  serializeJson(doc, payload);

  if (! recipeDataPublish.publish(payload)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}
