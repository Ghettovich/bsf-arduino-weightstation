#include <EtherSia.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"

const int etherSS = 53, port = 6678;
const char *serverIP = "fe80::3d15:b791:18be:a308";

// ENC28J60 Ethernet Interface
EtherSia_ENC28J60 ether(etherSS);

// Define UDP socket with ether and port
UDPSocket udp(ether, port);

// HTTP server
HTTPServer http(ether);

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

    Serial.print("Our link-local address is: ");
    ether.linkLocalAddress().println();
    Serial.print("Our global address is: ");
    ether.globalAddress().println();
}

void receiveEtherPacket() {
    ether.receivePacket();

    // HTTP endpoints
    if (http.isGet(F("/"))) {
        replyWithFullStatePayload();
    } else if (http.isGet(F("/test"))) {
        http.printHeaders(http.typeHtml);
        http.println(F("<h1>Hello World</h1>"));
        http.sendReply();
    }

    // UDP
    if (udp.havePacket()) {
        Serial.print("Received UDP from: ");
        udp.packetSource().println();

        Serial.print("Packet length: ");
        Serial.println(udp.payloadLength(), DEC);

        deserializePayload();
    }
}

void deserializePayload() {
    Serial.println("copying payload...");
    StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
    char json[ETHERSIA_MAX_PACKET_SIZE];

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, udp.payload());

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    } else {
        if (pRecipe == nullptr) {
            pRecipe = new Recipe();
        }
        pRecipe->recipeId = doc["recipeId"];
        pRecipe->data[0] = doc["data"][0];
        pRecipe->data[1] = doc["data"][1];

        updateRecipeComponents(pRecipe->recipeId, pRecipe->data[0], pRecipe->data[1]);
    }
}

void broadcastUpdatedRecipe(struct Recipe recipe) {
    StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
    char payload[ETHERSIA_MAX_PACKET_SIZE];

    doc["arduinoId"] = recipe.arduinoId;
    doc["iodeviceId"] = recipe.deviceId;
    doc["recipeId"] = recipe.recipeId;

    // ToDo: refactor with correct component id
    doc["componentId"] = selectedComponent;
    doc["weight"] = recipe.data[0];

    serializeJson(doc, payload);

    if (!udp.remoteAddress()) {
        udp.setRemoteAddress(serverIP, port);
        udp.remoteAddress().println();
    } else {
        udp.remoteAddress().println();
    }

    udp.println(payload);
    udp.send();
}

void replyWithFullStatePayload() {
    StaticJsonDocument <ETHERSIA_MAX_PACKET_SIZE> doc;
    char payload[ETHERSIA_MAX_PACKET_SIZE];

    doc["arduinoId"] = recipe.arduinoId;
    doc["iodeviceId"] = recipe.deviceId;
    doc["recipeId"] = recipe.recipeId;

    serializeJson(doc, payload);

    http.printHeaders(http.typeJson);
    http.println(payload);
    http.sendReply();
}