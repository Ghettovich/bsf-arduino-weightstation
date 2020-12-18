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


void hx711Setup() {
  scale.begin(HX711_dout, HX711_sck);

  scale.tare();
  scale.set_scale(110.10f);

//  Serial.print("UNITS: ");
//  Serial.println(scale.get_units(10));
//  
//    Serial.println("\nEmpty the scale, press a key to continue");
//    while (!Serial.available());
//    while (Serial.available()) Serial.read();
//  
//    scale.tare();
//  
//    Serial.print("UNITS: ");
//    Serial.println(scale.get_units(10));
//  
//    Serial.println("\nPut a 1 kg in the scale, press a key to continue");
//    while (!Serial.available());
//    while (Serial.available()) Serial.read();
//  
//    scale.callibrate_scale(1000, 5);
//    Serial.print("UNITS: ");
//    Serial.println(scale.get_units(10));
//  
//    Serial.println("\nScale is callibrated, press a key to continue");
//    Serial.println(scale.get_scale());
//  
//    while (!Serial.available());
//    while (Serial.available()) Serial.read();

  delayStartPublishRecipe = millis();
  delayRunning = true;
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

void setDelayRunning(bool _delayRunning) {
  delayRunning = _delayRunning;
}

void tareScaleHx711() {
  scale.tare();
  Serial.print("(tare) UNITS: ");
  tareWeight = scale.get_units(10);
  Serial.println(tareWeight);
}

void hx711Loop() {

  if (currentRecipe != NULL &&
    delayRunning && (millis() - delayStartPublishRecipe) >= interval) {

    scale.power_up();
    currentRecipe->updateWeight(scale.get_units(3));
    publishRecipeData();
//    Serial.print("Weight = ");
//    Serial.println(scale.get_units(3));


    delayStartPublishRecipe = millis();
    delayRunning = true;
  }

}
