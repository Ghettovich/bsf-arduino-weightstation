#ifndef BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
#define BSF_ARDUINO_WEIGHTSTATION_RECIPE_H

struct Component {
    int componentId;
    int weight;
};

struct Recipe {
    int arduinoId = 2;
    int iodeviceId = 1;
    int typeId = 1;    
    int recipeId = 0;
    Component components[3] = {
            {1, 0},
            {2, 0},
            {3, 0}};
};

#endif //BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
