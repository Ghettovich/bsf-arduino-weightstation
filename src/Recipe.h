#ifndef BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
#define BSF_ARDUINO_WEIGHTSTATION_RECIPE_H

const int maxComponentSize = 3;

enum components { WATER,  SAND, PLASTIFIER };

struct Component {
    int componentId = 0;
    int currentWeight = 0;
    int targetWeight = 0;
};

struct Recipe {
    int arduinoId = 2;
    int iodeviceId = 1;
    int typeId = 1;    
    int recipeId = 0;
    int componentSize = 0;
    Component components[maxComponentSize];
};

#endif //BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
