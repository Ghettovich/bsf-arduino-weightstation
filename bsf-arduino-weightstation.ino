// Paint application - Demonstate both TFT and Touch Screen
#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include <EtherSia.h>
#include <ArduinoJson.h>

#define PAYLOAD_SIZE = 600;

int etherSS = 53;
char * payload;

/** W5100 Ethernet Interface */
EtherSia_ENC28J60 ether(etherSS);

/** Define UDP socket with ether and port */
UDPSocket udp(ether, 6678);
const char * serverIP = "fd54:d174:8676:1:653f:56d7:bd7d:c238";

struct Recipe {
  int recipeId = 0;
  int data[2] = {0, 0};
};

int currentState = 0, prevState = 0;
Recipe recipe = Recipe();
int currentWeight = 0, prevWeight = 0;

int ColorPaletteHigh = 60;
int color = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

static void setPinDefinitions() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);
}

static void initTFTAndDrawButtons() {
  // TFT
  Tft.TFTinit();  //init TFT library
  Serial.begin(115200);
  Tft.fillRectangle(0, 60, 240, 320, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.fillRectangle(90, 0, 50, ColorPaletteHigh, BLUE);
  Tft.fillRectangle(140, 0, 50, ColorPaletteHigh, YELLOW);
  Tft.fillRectangle(190, 0, 50, ColorPaletteHigh, GRAY1);
}

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
    Tft.drawString("Geen recept bekend", 10, 230, 2, RED);
  }
  else {
    Tft.drawString("Druk op start.", 10, 230, 2, GREEN);
  }
}

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

static void copyPayload() {
  Serial.println("copying payload...");

  StaticJsonDocument<200> doc;
  char json[80];
  memcpy(json, udp.payload(), udp.payloadLength());

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  else {
    udp.sendReply("ok"); // reply to host that recipe is received and parsed succesfully
  }

  recipe.recipeId = doc["recipeId"];
  recipe.data[0] = doc["data"][0];
  recipe.data[1] = doc["data"][1];

  // if true component info is displayed, update state and let main loop update the ui
  if (currentState == 1) {
    currentState = 0;
  }
  // recipe info is being displayed, update values retrieved from payload
  else {
    updateDisplay();
  }
}

void setup()
{
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

    copyPayload();
  }

  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates

  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > __PRESURE) {
    // Detect component select change
    if (p.y < ColorPaletteHigh + 2)
    {      
      // Start button pressed update UI and request user to select component
      if (p.x > 0 && p.x < 90) { 
        if(currentState == 0) {
           currentState = 1;         
        }
        else if(currentState == 1) {
          currentState = 0;
        }
      }
      else if (p.x > 90 && p.x < 140) {
        Tft.fillRectangle(0, 115, 240, 30, BLACK);
        Tft.drawString("COMPONENT 1", 20, 120, 3, BLUE);
        Tft.drawString("TARGET =", 20, 160, 2, BLUE);
        Tft.drawNumber(recipe.data[0], 180, 160, 2, BLUE);
        Tft.drawString("HUIDIG =", 20, 230, 2, BLUE);
        // update with actual value
        Tft.drawNumber(0, 130, 220, 5, BLUE);
      }
      else if (p.x > 140 && p.x < 190) {
        Tft.fillRectangle(0, 115, 240, 30, BLACK);
        Tft.drawString("COMPONENT 2", 20, 120, 3, YELLOW);
      }
      else if (p.x > 190 && p.x < 240) {
        Tft.fillRectangle(0, 115, 240, 30, BLACK);
        Tft.drawString("COMPONENT 3", 20, 120, 3, GRAY1);
      }
    }
    else if(p.y >= 280) {
      // touch plus '+' sign
      if(p.x >= 25 && p.x <= 70) {
        currentWeight++;
        Tft.fillRectangle(130, 220, 120, 70, BLACK);
        Tft.drawNumber(currentWeight, 130, 220, 5, BLUE);
      }
      // touch minus '-' sign
      else if(p.x >= 75 && p.x <= 110) {
        currentWeight--;
        Tft.fillRectangle(130, 220, 120, 70, BLACK);
        Tft.drawNumber(currentWeight, 130, 220, 5, BLUE);
      }
    }
    
  }

  if(currentWeight != prevWeight {
    currentWeight = prevWeight;
    // send packet to app
  }

  if (currentState != prevState) {
    updateDisplay();
  }

  prevState = currentState;
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
