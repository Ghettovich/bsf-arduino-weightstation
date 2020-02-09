#ifndef BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
#define BSF_ARDUINO_WEIGHTSTATION_RECIPE_H

struct Recipe {
    int arduinoId = 2;
    int deviceId = 1;
    int recipeId = 0;
    int data[2] = {0, 0};
};

#endif //BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
