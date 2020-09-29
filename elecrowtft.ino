#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"

const int tftWidth = 240, tftHeight = 320;
const int SD_CS_PIN = 4, TFT_CS_PIN = 5, TFT_DC = 6;

int displayStatus = 0, prevDisplayStatus = 0;
enum displayRecipeStates { START = 0, START_WITH_RECIPE = 1, SELECT_COMP = 2, UPDATE = 3 };
const char *componentsTextDisplay[] = { "Water", "Zand", "Plastificeerder" };

int ColorPaletteHigh = 60;
int color = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

Recipe *recipe = new Recipe;
int selectedComponent = -1;


// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

void updateDisplayStatus(int _displayStatus) {
  displayStatus = _displayStatus;
}

void setRecipeId(int _recipeId) {
  recipe->recipeId = _recipeId;
}

int getSelectedComponent() {
  if(selectedComponent == components::WATER) {
      return 1;
  } else if (selectedComponent == components::SAND)  {
    return 2;
  } else if (selectedComponent == components::PLASTIFIER) {
    return 3;
  } 
  return selectedComponent;
}

/** Insert a component with its id and weight. */
void insertComponentWithIdAndWeight(int id, int weight) {   
  recipe->components[recipe->componentSize].componentId = id;
  recipe->components[recipe->componentSize].targetWeight = weight;
  recipe->componentSize++;
  Serial.println("added component!");

}

void addRecipeComponentsToJsonArray(JsonArray items) {

  for (int i = 0; i < maxComponentSize; i++) {
    JsonObject obj = items.createNestedObject();
    obj["id"] = recipe->components[i].componentId;
    obj["weight"] = recipe->components[i].currentWeight;
  }
}

/** Define Pins, set pins low let library handle SS/CS */
void setTFTPinDefinitions() {
  // TF_CS SD card select input
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  // TFT_CS TFT chip select
  pinMode(TFT_CS_PIN, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  // TFT_DC TFT chip select
  pinMode(TFT_DC, OUTPUT);
  digitalWrite(TFT_DC, HIGH);
}

/** Initialize TFT and draw start screen */
void initTFTouchScreen() {
  //init TFT library
  Tft.TFTinit();
  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.fillRectangle(90, 0, 50, ColorPaletteHigh, BLUE);
  Tft.fillRectangle(140, 0, 50, ColorPaletteHigh, YELLOW);
  Tft.fillRectangle(190, 0, 50, ColorPaletteHigh, GRAY1);
}

/** Draw recipe info with targets from received recipe (0 if none) */
void drawRecipeInfo() {
  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.drawString("Recept info", 30, 80, 3, WHITE);

  // ToDo make more room for 3rd components or something smart.

  if (recipe->recipeId == 0) {
    Tft.drawString("Geen recept bekend", 10, 230, 2, WHITE);

    Serial.println("recipeId niet bekend.");
  }
  else {
    Tft.drawString("Druk op start.", 10, 230, 2, GREEN);
  }
}

/** Draw select component info with temporary plus and minus buttons */
void drawSelectComponent() {
  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Recept", 5, 20, 2, WHITE);
  Tft.drawString("KIES COMPONENT", 40, 80, 2, WHITE);
  Tft.drawChar('+', 25, 280, 5, WHITE);
  Tft.drawRectangle(25, 280, 35, 35, WHITE);
  Tft.drawChar('-', 75, 280, 5, WHITE);
  Tft.drawRectangle(75, 280, 35, 35, WHITE);
}

/** Draw selected component with corresponding colors */
void drawSelectedComponentInfo(int color) {

  if (selectedComponent == components ::PLASTIFIER ||
      selectedComponent == components ::WATER ||
      selectedComponent == components ::SAND) {

    Tft.fillRectangle(0, 115, 240, 160, BLACK);
    Tft.drawString(componentsTextDisplay[selectedComponent], 20, 120, 2, color);
    Tft.drawString("TARGET =", 20, 160, 2, color);
    Tft.drawString("HUIDIG =", 20, 210, 2, color);

    Tft.drawNumber(recipe->components[selectedComponent].targetWeight, 130, 160, 2, color);
    Tft.drawNumber(recipe->components[selectedComponent].currentWeight, 140, 210, 3, color);
  }
}

/** Update displayed weight with selected component */
void updateRecipeWeightInfo() {
  int x = 140, y = 210;
  Tft.fillRectangle(120, 200, 110, 70, BLACK);

  switch (selectedComponent) {
    case components::WATER :
      Tft.drawNumber(recipe->components[selectedComponent].currentWeight, x, y, 3, BLUE);
      break;
    case components::PLASTIFIER :
      Tft.drawNumber(recipe->components[selectedComponent].currentWeight, x, y, 3, GRAY1);
      break;
    case components::SAND :
      Tft.drawNumber(recipe->components[selectedComponent].currentWeight, x, y, 3, YELLOW);
      break;
  }
}

/** Update display based on state, always called in loop when currenState != prevState */
void updateDisplay() {
  switch (displayStatus) {
    case START:
    case START_WITH_RECIPE:
      drawRecipeInfo();
      break;
    case SELECT_COMP:
      drawSelectComponent();
      break;
    case UPDATE:
      // update recipe component

      break;
  }
}

void displayLoop() {
  processTouch();

  if (prevDisplayStatus != displayStatus) {
    updateDisplay();
  }

  prevDisplayStatus = displayStatus;
}

/** Process touch input if valid pressure detected */
void processTouch() {
  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();
  //map the ADC value read to into pixel co-ordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tftWidth);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tftHeight);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > __PRESURE) {
    readTouchInput(p.x, p.y);
  }
}

void readTouchInput(int x, int y) {
  // Detect component select change
  if (y < ColorPaletteHigh + 2)
  {
    // Start button pressed, update UI and request user to select component
    if (x > 0 && x < 90) {
      if (recipe->recipeId > 0) {
        drawSelectComponent();
      }
      else if (recipe->recipeId == 0) {
        Tft.drawString("Kies receptuur!", 10, 260, 2, RED);
      }
    }
    else if (x > 90 && x < 140) {
      selectedComponent = components::WATER;
      drawSelectedComponentInfo(BLUE);
    }
    else if (x > 140 && x < 190) {
      selectedComponent = components::SAND;
      drawSelectedComponentInfo(YELLOW);
    }
    else if (x > 190 && x < tftWidth) {
      selectedComponent = components::PLASTIFIER;
      drawSelectedComponentInfo(GRAY1);
    }
  }
  else if (y >= 280) {
    // touch plus '+' sign
    if (x >= 25 && x <= 70) {
      recipe->components[selectedComponent].currentWeight++;
      Serial.println("got touch");
      updateRecipeWeightInfo();
    }
    // touch minus '-' sign
    else if (x >= 75 && x <= 110) {
      recipe->components[selectedComponent].currentWeight--;

      Serial.println("got touch");
      updateRecipeWeightInfo();
    }
  }
}
