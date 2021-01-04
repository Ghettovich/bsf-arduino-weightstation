HX711 scale;

const int HX711_dout = 18;
const int HX711_sck = 49;

const int interval = 750, timeOutInterval = 30000;
unsigned long delayStartPublishRecipe = 0, startTimeOut = 0;

bool timeOutOccured = false, delayTimeOut = false;
bool delayRunning = false, isTareActive = false;

Recipe *currentRecipe;
int currentComponentIndex = -1;

int prevWeight = 0;
float currentWeight = -1.00f, tareWeight = -1.00f;
float scaleFactorAddress = 10;
const int maxLoad = 20000;


void hx711Setup() {
  scale.begin(HX711_dout, HX711_sck);
  scale.set_scale(scaleFactorAddress);
  
  delayStartPublishRecipe = millis();
  delayRunning = false;
}
void setIsTareActive(bool active) {
  isTareActive = active;
}

void setCurrentRecipe(Recipe &recipe) {
  currentRecipe = &recipe;
}

void setTareWeight(float _tareWeight) {
  tareWeight = _tareWeight;
}

void setCurrentComponent(int selectedComponent) {
  currentComponentIndex = currentComponentIndex;
}

void setDelayRunning(bool _delayRunning) {
  delayRunning = _delayRunning;
}

void setDelayTimeOut(bool _delayTimeOut) {
  delayTimeOut = _delayTimeOut;
}

int getMaxLoadCellWeight() {
  return maxLoad;
}

bool getTimeOutOccured() {
  return timeOutOccured;
}

void setTimeOutOccured(bool _timeOutOccured) {
  timeOutOccured = _timeOutOccured;
}

void tareScaleHx711() {
  scale.tare();
}

void calibrateScale() {
  int tareReadings = 5;
  scale.callibrate_scale(tareWeight, tareReadings);
  EEPROM.updateFloat(scaleFactorAddress, scale.get_scale());  
}

void startTimeOutTimer() {
  delayTimeOut = true;
  timeOutOccured = false;
  startTimeOut = millis();
}

void hx711Loop() {

  if (currentRecipe != NULL &&
      delayRunning && (millis() - delayStartPublishRecipe) >= interval) {

    scale.power_up();

    if (isTareActive) {
      scale.callibrate_scale(1000, 5);
    }
    
    currentWeight = scale.get_units(3);
    currentRecipe->updateWeight((int)currentWeight);
    
    delayStartPublishRecipe = millis();    
  }

  if(delayTimeOut && (millis() - startTimeOut) >= timeOutInterval) {
    timeOutOccured = true;    
    delayRunning = false;
    delayTimeOut = false;    
  }
}
