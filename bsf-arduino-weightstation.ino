#include “includes.h”
#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include <EtherSia.h>
#include <ArduinoJson.h>

const int etherSS = 53, port = 6678;
const int plastifierId = 1, waterId = 2, sandId = 3;

/** W5100 Ethernet Interface */
EtherSia_ENC28J60 ether(etherSS);

/** Define UDP socket with ether and port */
UDPSocket udp(ether, port);
const char * serverIP = "fe80::3d15:b791:18be:a308";

/** JSON info */
const int capacity = JSON_ARRAY_SIZE(4) + 4 * JSON_OBJECT_SIZE(4);

struct Recipe {
  int arduinoId = 2;
  int deviceId = 1;
  int recipeId = 0;
  int data[2] = {0, 0};
};

enum RecipeComponent {PLASTIFIER, WATER, SAND};

Recipe recipe = Recipe();
int selectedComponent = 0;
int currentState = 0, prevState = 0;
int prevWeight = 0;

int ColorPaletteHigh = 60;
int color = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

/** Define Pins, set pins low let library handle SS/CS */
static void setPinDefinitions() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);
}

/** Initialize TFT and draw start screen */
static void initTFTAndDrawButtons() {
  // TFT
  Tft.TFTinit();  //init TFT library  
  Tft.fillRectangle(0, 60, 240, 320, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.fillRectangle(90, 0, 50, ColorPaletteHigh, BLUE);
  Tft.fillRectangle(140, 0, 50, ColorPaletteHigh, YELLOW);
  Tft.fillRectangle(190, 0, 50, ColorPaletteHigh, GRAY1);
}

/** Draw recipe info with targets from received recipe (0 if none) */
static void drawRecipeInfo() {
  Tft.fillRectangle(0, 60, 240, 320, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.drawString("Recept info", 30, 80, 3, WHITE);
  Tft.drawString("Component 1:", 10, 130, 2, WHITE);
  Tft.drawNumber(recipe.data[0], 160, 130, 2, RED);
  Tft.drawString("Component 2:", 10, 180, 2, WHITE);
  Tft.drawNumber(recipe.data[1], 160, 180, 2, RED);
  if (recipe.recipeId == 0) {
    Tft.drawString("Geen recept bekend", 10, 230, 2, WHITE);
  }
  else {
    Tft.drawString("Druk op start.", 10, 230, 2, GREEN);
  }
}

/** Draw select component info with temporary plus and minus buttons */
static void drawSelectComponent() {
  Tft.fillRectangle(0, 60, 240, 320, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Recept", 5, 20, 2, WHITE);
  Tft.drawString("KIES COMPONENT", 40, 80, 2, WHITE);
  Tft.drawChar('+', 25, 280, 5, WHITE);
  Tft.drawRectangle(25, 280, 35, 35, WHITE);
  Tft.drawChar('-', 75, 280, 5, WHITE);
  Tft.drawRectangle(75, 280, 35, 35, WHITE);
}

/** Draw selected component with corresponding colors */
static void drawSelectedComponentInfo(int color) {
  if (selectedComponent == 1 || selectedComponent == 2 || selectedComponent == 3) {
    Tft.fillRectangle(0, 115, 235, 165, BLACK);
    Tft.drawString("COMPONENT", 20, 120, 3, color);
    Tft.drawNumber(selectedComponent, 180, 120, 3, color);
    Tft.drawString("TARGET =", 20, 160, 2, color);
    Tft.drawString("HUIDIG =", 20, 230, 2, color);
    // update with actual value
    switch (selectedComponent) {
      case 1 :
        Tft.drawNumber(recipe.data[0], 140, 160, 2, color);
        Tft.drawNumber(recipe.data[0], 130, 220, 4, color);
        break;
      case 2 :
        Tft.drawNumber(recipe.data[1], 140, 160, 2, color);
        Tft.drawNumber(recipe.data[1], 130, 220, 4, color);
        break;
    }
  }
}

/** Update displayed weight with selected component */
static void updateRecipeWeightInfo() {
  Tft.fillRectangle(130, 220, 120, 70, BLACK);
  switch (selectedComponent) {
    case 1 :      
      Tft.drawNumber(recipe.data[0], 120, 220, 5, BLUE);
      break;
    case 2 :
      Tft.drawNumber(recipe.data[1], 120, 220, 5, YELLOW);
      break;
  }
}

/** Update display based on state, always called in loop when currenState != prevState */
static void updateDisplay() {
  switch (currentState) {
    case 0:
      drawRecipeInfo();
      break;
    case 1:
      drawSelectComponent();
      break;
  }
}

/** Initialize the ethernet adapter  */
static void initEthernetAdapter() {
  // Ethernet adapter
  MACAddress macAddress("9e:b3:19:c7:1b:10");
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

static void deserializePayload() {
  Serial.println("copying payload...");
  StaticJsonDocument<200> doc;
  char json[ETHERSIA_MAX_PACKET_SIZE];
  //memcpy(json, udp.payload(), udp.payloadLength()); // remove this line?? test!!

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, udp.payload());

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    udp.sendReply("{\"stateReply\":1}"); // reply to host that an error occured
    return;
  }
  else {
    recipe.recipeId = doc["recipeId"];
    recipe.data[0] = doc["data"][0];
    recipe.data[1] = doc["data"][1];
    // parsed json successfuly send reply back, 1 means parsed successfully
    udp.sendReply("{\"arduinoId\":2, \"stateReply\":1}");
  }

  // if true component info is displayed, update state and let main loop update the ui
  if (currentState == 1) {
    setState(0);
  }
  // recipe info is being displayed, update values retrieved from payload
  else {
    updateDisplay();
  }
}

static void onChangeWeightPayload() {
  StaticJsonDocument<250> doc;
  char payload[ETHERSIA_MAX_PACKET_SIZE];

  doc["arduinoId"] = recipe.arduinoId;
  doc["deviceId"] = recipe.deviceId;
  doc["recipeId"] = recipe.recipeId;
  doc["componentId"] = selectedComponent;
  doc["weight"] = recipe.data[0];

  serializeJson(doc, payload);

  if(!udp.remoteAddress()) {
    udp.setRemoteAddress(serverIP, port);
    udp.remoteAddress().println();
  }
  else {
    udp.remoteAddress().println();
  }

  udp.println(payload);
  udp.send();    
}

static void setState(int state) {
  prevState = currentState;
  currentState = state;
}

static void readTouchInput(int x, int y) {
  // Detect component select change
  if (y < ColorPaletteHigh + 2)
  {
    // Start button pressed, update UI and request user to select component
    if (x > 0 && x < 90) {
      if (currentState == 0 && recipe.recipeId > 0) {
        setState(1);
      }
      else if (recipe.recipeId == 0) {
        Tft.drawString("Kies receptuur!", 10, 260, 2, RED);
      }
      else if (currentState == 1) {
        setState(0);
      }
    }
    else if (x > 90 && x < 140 && currentState == 1) {
      selectedComponent = RecipeComponent::PLASTIFIER;
      drawSelectedComponentInfo(BLUE);
    }
    else if (x > 140 && x < 190 && currentState == 1) {
      selectedComponent = RecipeComponent::WATER;
      drawSelectedComponentInfo(YELLOW);
    }
    else if (x > 190 && x < 240 && currentState == 1) {
      selectedComponent = RecipeComponent::SAND;
      drawSelectedComponentInfo(GRAY1);
    }
  }
  else if (y >= 280) {
    // touch plus '+' sign
    if (x >= 25 && x <= 70) {
      if(selectedComponent == 1) {
        recipe.data[0]++;
      }
      else {
        recipe.data[1]++;
      }      
      updateRecipeWeightInfo();
    }
    // touch minus '-' sign
    else if (x >= 75 && x <= 110) {
      if(selectedComponent == 1) {
        recipe.data[0]--;
      }
      else {
        recipe.data[1]--;
      }      
      updateRecipeWeightInfo();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setPinDefinitions();
  initTFTAndDrawButtons();
  initEthernetAdapter();
  updateDisplay();

  Serial.println("Ready.");
}

void loop()
{
  ether.receivePacket();

  if (udp.havePacket()) {
    Serial.print("Received UDP from: ");
    udp.packetSource().println();

    Serial.print("Packet length: ");
    Serial.println(udp.payloadLength(), DEC);

    deserializePayload();
  }

  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > __PRESURE) {
    readTouchInput(p.x, p.y);
  }

  if (recipe.data[0] != prevWeight) {
    prevWeight = recipe.data[0];
    // send packet to app
    Serial.println("weight changed, transmit");
    onChangeWeightPayload();
  }

  if (currentState != prevState) {
    updateDisplay();
  }

  prevState = currentState;
}
