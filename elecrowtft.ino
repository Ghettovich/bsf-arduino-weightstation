#include <stdint.h>
#include <SeeedTouchScreen.h>
#include <TFTv2.h>
#include <SPI.h>
#include "src/Recipe.h"

const int tftWidth = 240, tftHeight = 320;
const int SD_CS_PIN = 4, TFT_CS_PIN = 5;
const int plastifierId = 1, waterId = 2, sandId = 3;

enum DISPLAY_STATUS { START, SELECT, UPDATE };
enum COMPONENTS { NONE, PLASTIFIER, WATER, SAND };

int ColorPaletteHigh = 60;
int color = WHITE;  //Paint brush color
unsigned int colors[4] = {RED, BLUE, YELLOW, GRAY1};

struct Recipe recipe;
COMPONENTS selectedComponent = COMPONENTS::NONE;
DISPLAY_STATUS status = DISPLAY_STATUS::START;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM); //init TouchScreen port pins

// END GLOBAL VAR
void updateRecipeComponents(int recipeId, int plastifierTarget, int waterTarget) {
    recipe.recipeId = recipeId;
    recipe.data[0] = plastifierTarget;
    recipe.data[1] = waterTarget;
}

/** Define Pins, set pins low let library handle SS/CS */
void setTFTPinDefinitions() {
    // TF_CS SD card select input
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    // TFT_CS TFT chip select
    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);
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
    if (selectedComponent == COMPONENTS ::PLASTIFIER ||
        selectedComponent == COMPONENTS ::WATER ||
        selectedComponent == COMPONENTS ::SAND) {

        Tft.fillRectangle(0, 115, 235, 165, BLACK);
        Tft.drawString("COMPONENT", 20, 120, 3, color);
        Tft.drawString("TARGET =", 20, 160, 2, color);
        Tft.drawString("HUIDIG =", 20, 230, 2, color);
        // update with actual value
        switch (selectedComponent) {
            case COMPONENTS::PLASTIFIER :
                Tft.drawNumber(recipe.data[0], 140, 160, 2, color);
                Tft.drawNumber(recipe.data[0], 130, 220, 3, color);
                break;
            case COMPONENTS::WATER :
                Tft.drawNumber(recipe.data[1], 140, 160, 2, color);
                Tft.drawNumber(recipe.data[1], 130, 220, 3, color);
                break;
        }
    }
}

/** Update displayed weight with selected component */
void updateRecipeWeightInfo() {
    Tft.fillRectangle(130, 220, 110, 70, BLACK);
    switch (selectedComponent) {
        case COMPONENTS::PLASTIFIER :
            Tft.drawNumber(recipe.data[0], 120, 220, 3, BLUE);
            break;
        case COMPONENTS::WATER :
            Tft.drawNumber(recipe.data[1], 120, 220, 3, YELLOW);
            break;
    }
}

/** Update display based on state, always called in loop when currenState != prevState */
void updateDisplay() {
    switch (status) {
        case START:
            drawRecipeInfo();
            break;
        case SELECT:
            drawSelectComponent();
            break;
        case UPDATE:
            // update recipe component

            break;
    }
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
            if (recipe.recipeId > 0) {
                drawSelectComponent();
            }
            else if (recipe.recipeId == 0) {
                Tft.drawString("Kies receptuur!", 10, 260, 2, RED);
            }
        }
        else if (x > 90 && x < 140) {
            selectedComponent = COMPONENTS::PLASTIFIER;
            drawSelectedComponentInfo(BLUE);
        }
        else if (x > 140 && x < 190) {
            selectedComponent = COMPONENTS::WATER;
            drawSelectedComponentInfo(YELLOW);
        }
        else if (x > 190 && x < tftWidth) {
            selectedComponent = COMPONENTS::SAND;
            drawSelectedComponentInfo(GRAY1);
        }
    }
    else if (y >= 280) {
        // touch plus '+' sign
        if (x >= 25 && x <= 70) {
            if(selectedComponent == COMPONENTS::PLASTIFIER) {
                recipe.data[0]++;
            }
            else if(selectedComponent == COMPONENTS::WATER) {
                recipe.data[1]++;
            }

            Serial.println("got touch");
            updateRecipeWeightInfo();
        }
            // touch minus '-' sign
        else if (x >= 75 && x <= 110) {
            // ToDo: implement for all necessary components
            if(selectedComponent == COMPONENTS::PLASTIFIER) {
                recipe.data[0]--;
            }
            else if(selectedComponent == COMPONENTS::WATER) {
                recipe.data[1]--;
            }

            pRecipe = &recipe;
            Serial.println("got touch");
            updateRecipeWeightInfo();
        }
    }
}
