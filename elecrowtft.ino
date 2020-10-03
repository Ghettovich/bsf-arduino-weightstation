#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "src/Recipe.h"

const int tftWidth = 240, tftHeight = 320;
const int SD_CS_PIN = 4, TFT_CS_PIN = 5, TFT_DC = 6;

const int intervalUpdate = 250;
unsigned long delayUpdateStart = 0;
bool delayUpdateRunning = false;

displayRecipeStates displayStatus, prevDisplayStatus;

const char *componentsTextDisplay[] = { "Water", "Zand", "Plastificeerder" };

int ColorPaletteHigh = 60;
int color = WHITE, currentColor = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

Recipe *recipe = new Recipe;
int selectedComponent = -1;

const int touchIntervalMS = 250;
bool touchInterval = true;
unsigned long delayTouch = 0;

float tareWeight = -1;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

void updateDisplayStatus(displayRecipeStates _displayStatus) {
  prevDisplayStatus = displayStatus;
  displayStatus = _displayStatus;
}

void setDelayUpdateRunning(bool _delayUpdateRunning) {
  delayUpdateRunning = _delayUpdateRunning;
}

void setRecipeId(int _recipeId) {
  recipe->recipeId = _recipeId;
}

void setTareWeight(int _tareWeight) {
  tareWeight = _tareWeight;
}

int getSelectedComponent() {
  if (selectedComponent == components::WATER) {
    return 1;
  } else if (selectedComponent == components::SAND)  {
    return 2;
  } else if (selectedComponent == components::PLASTIFIER) {
    return 3;
  }
  return selectedComponent;
}

void setRecipeForScale() {
  setCurrentRecipe(*recipe);
}

/** Insert a component with its id and weight. */
void insertComponentWithIdAndWeight(int id, int weight) {
  if (recipe->componentSize == maxComponentSize) {
    recipe->componentSize = 0;
  }
  recipe->components[recipe->componentSize].componentId = id;
  recipe->components[recipe->componentSize].targetWeight = weight;
  recipe->componentSize++;
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

  // Start timer for touch
  delayTouch = millis();
  delayUpdateStart = millis();
}

/** Draw recipe info with targets from received recipe (0 if none) */
void drawRecipeInfo() {
  Serial.println("drawing start...");
  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.fillRectangle(90, 0, 50, ColorPaletteHigh, BLUE);
  Tft.fillRectangle(140, 0, 50, ColorPaletteHigh, YELLOW);
  Tft.fillRectangle(190, 0, 50, ColorPaletteHigh, GRAY1);

  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
  Tft.drawString("Start", 5, 20, 2, WHITE);
  Tft.drawString("Recept info", 30, 80, 3, WHITE);

  // ToDo make more room for 3rd components or something smart.

  if (recipe->recipeId > 0) {
    Tft.drawString("Kies component.", 10, 230, 2, GREEN);

    // Pass pointer of recipe to hx711 tab
    setRecipeForScale();
  }

  Tft.drawChar('T', 200, 280, 5, WHITE);
  Tft.drawRectangle(200, 280, 35, 35, WHITE);

  setDelayUpdateRunning(false);  
}

/** Draw select component info with temporary plus and minus buttons */
void drawSelectComponent() {
  if (recipe->recipeId != 0) {
    Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
    Tft.fillRectangle(0, 0, 90, ColorPaletteHigh, RED);
    Tft.drawString("Recept", 5, 20, 2, WHITE);
    Tft.drawString("KIES COMPONENT", 40, 80, 2, WHITE);

    drawSelectedComponentInfo();
  } else {
    Tft.drawString("Kies receptuur!", 10, 260, 2, RED);
  }
}


/** Draw selected component with corresponding colors */
void drawSelectedComponentInfo() {
  Tft.fillRectangle(0, 115, 240, 205, BLACK);
  Tft.drawString(componentsTextDisplay[selectedComponent], 20, 120, 2, currentColor);
  Tft.drawString("TARGET =", 20, 160, 2, currentColor);
  Tft.drawString("HUIDIG =", 20, 210, 2, currentColor);

  Tft.drawNumber(recipe->components[selectedComponent].targetWeight, 130, 160, 2, currentColor);
  Tft.drawNumber(recipe->components[selectedComponent].currentWeight, 140, 210, 3, currentColor);

  // Set bool in tft loop for updateing display
  setDelayUpdateRunning(true);
  // Set selected component in hx711
  setCurrentComponent(selectedComponent);
}

/** Update displayed weight with selected component */
void updateRecipeWeightInfo() {
  int x = 140, y = 210;
  Tft.fillRectangle(120, 200, 110, 70, BLACK);
  Tft.drawNumber(recipe->components[selectedComponent].currentWeight, x, y, 3, currentColor);
}


void drawTareScreen() {
  Tft.fillRectangle(0, 60, tftWidth, tftHeight, BLACK);
  Tft.drawString("Leeg weegschaal en", 10, 80, 2, WHITE);
  Tft.drawString("druk OK", 10, 120, 2, WHITE);

  Tft.fillRectangle(40, 160, 160, 80, GREEN);
  Tft.drawString("OK", 100, 185, 4, WHITE);
}

void drawRequestObjectForTare() {
  Tft.fillRectangle(0, 0, tftWidth, tftHeight, BLACK);
  Tft.drawString("Plaats een gewicht", 10, 20, 2, WHITE);
  Tft.drawString("van exact 1 kg.", 10, 60, 2, WHITE);

  Tft.fillRectangle(40, 160, 160, 80, GREEN);
  Tft.drawString("CALIBREER", 55, 190, 2, WHITE);

  Tft.fillRectangle(110, 265, 90, 90, BLACK);
  Tft.drawNumber(0, 110, 265, 5, WHITE);
}

void drawUpdateTareWeight() {
  int weight = round(tareWeight);
  Tft.fillRectangle(60, 265, 180, 90, BLACK);
  Tft.drawNumber(weight, 60, 265, 5, WHITE);
}

void drawFinishCalibrating() {
  const int dotsToDrawn = 5;
  int x = 60;
  Tft.fillRectangle(0, 0, tftWidth, tftHeight, BLACK);
  Tft.drawString("Calibratie succes", 10, 20, 2, GREEN);

  for (int i = 0; i < dotsToDrawn; i++) {
    Tft.drawString(".", x, 70, 4, WHITE);
    x += 15;
    delay(300);
  }

  setTareWeight(-1);
  // for some reason I when setting state here to START it wont actually draw the screen.
  drawRecipeInfo();
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
      updateRecipeWeightInfo();
      break;
    case TARE:
      drawTareScreen();
      break;
    case CALIBRATE_START:
      drawRequestObjectForTare();
      break;
    case CALIBRATE_FINISH:
      drawFinishCalibrating();
      break;
  }
}

void displayLoop() {
  if (touchInterval && (millis() - delayTouch) >= touchIntervalMS) {
    processTouch();

    delayTouch = millis();
    touchInterval = true;
  }


  if (delayUpdateRunning && (millis() - delayUpdateStart) >= intervalUpdate) {

    delayUpdateStart = millis();

    // this check should be moved to update recipe weight info (test!)
    // but doing disables updateing the current weight displayed :/.
    if (selectedComponent != -1 && recipe->recipeId) {
      updateRecipeWeightInfo();
    } else if (tareWeight != -1) {
      drawUpdateTareWeight();
    }
  }

  if (displayStatus != prevDisplayStatus) {
    Serial.println("Display status changed");
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
      selectedComponent = -1;
      setCurrentComponent(selectedComponent);
      setBroadcastRecipe(false);
      setDelayUpdateRunning(false);
      setReadScale(false);
      drawRecipeInfo();
    }
    else if (x > 90 && x < 140) {
      setBroadcastRecipe(true);
      currentColor = BLUE;
      selectedComponent = components::WATER;
      drawSelectedComponentInfo();
      //updateDisplayStatus(displayRecipeStates::SELECT_COMP);
    }
    else if (x > 140 && x < 190) {
      setBroadcastRecipe(true);
      currentColor = YELLOW;
      selectedComponent = components::SAND;
      drawSelectedComponentInfo();
      //updateDisplayStatus(displayRecipeStates::SELECT_COMP);
    }
    else if (x > 190 && x < tftWidth) {
      setBroadcastRecipe(true);
      currentColor = GRAY1;
      selectedComponent = components::PLASTIFIER;
      drawSelectedComponentInfo();
      //updateDisplayStatus(displayRecipeStates::SELECT_COMP);
    }
  }
  else if (y >= 160 && y < 240) {
    if (x > 40 && x < 200 && displayStatus == displayRecipeStates::TARE) {
      Serial.println("OK pressed, start taring...");      
      setReadScale(true);
      setDelayUpdateRunning(true);

      tareWeight = 0;
      setTareWeight(tareWeight);
      tareScaleHx711();
      updateDisplayStatus(displayRecipeStates::CALIBRATE_START);
    }
    else if (x > 40 && x < 200 && displayStatus == CALIBRATE_START) {
      Serial.println("Calibrate start pressed...");            
      setDelayUpdateRunning(false);
      
      calibrateScale();
      updateDisplayStatus(displayRecipeStates::CALIBRATE_FINISH);
    }
  }
  else if (y >= 280) {
    // touch tare button
    if (x >= 200 && x <= tftWidth) {
      Serial.println("Tare pressed");
      updateDisplayStatus(displayRecipeStates::TARE);
      //drawTareScreen();
    }
  }
}
