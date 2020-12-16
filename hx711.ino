HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 1000;
unsigned long delayStartPublishRecipe = 0;

bool delayRunning = false;

Recipe *currentRecipe;
int currentComponentIndex = -1;

float *currentWeight;
float tareWeight = -1;

bool readScale = false;

void hx711Setup() {
  scale.begin(HX711_dout, HX711_sck);

  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nEmpty the scale, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  scale.tare();
  
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nPut a 1 kg in the scale, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  scale.callibrate_scale(1000, 5);
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nScale is callibrated, press a key to continue");
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  delayStartPublishRecipe = millis();
  delayRunning = true;
}

void setReadScale(bool _readScale) {
  readScale = _readScale;
}

void setCurrentRecipe(Recipe &recipe) {
  currentRecipe = &recipe;
}

void setTareWeight(float &_currentWeight) {
  currentWeight = &_currentWeight;
}

void setCurrentComponent(int selectedComponent) {
  currentComponentIndex = currentComponentIndex;
}

void tareScaleHx711() {
  scale.tare();
  Serial.print("(tare) UNITS: ");
  tareWeight = scale.get_units(10);
  Serial.println(tareWeight);
}


void calibrateScale() {
  scale.callibrate_scale(1000, 5);
  Serial.print("(calibrate) UNITS: ");
  Serial.println(scale.get_units(3));

  readScale = false;
  currentWeight = NULL;
}

void hx711Loop() {

    if (delayRunning && (millis() - delayStartPublishRecipe) >= interval) {

      scale.power_up();
      Serial.print("Weight = ");
      Serial.println(scale.get_units(3));

      delayStartPublishRecipe = millis();
      delayRunning = true;
    }
  
}
