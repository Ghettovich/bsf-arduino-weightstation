#ifndef BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
#define BSF_ARDUINO_WEIGHTSTATION_RECIPE_H

#include <stddef.h>

const int columnSize = 3;
const int maxComponentSize = 10;

enum components { WATER,  SAND, PLASTIFIER };
enum columns { ID = 0, TARGET_WEIGHT = 1, CURRENT_WEIGHT = 2 };

struct Component {
    int componentId = 0;
    int currentWeight = 0;
    int targetWeight = 0;
};

class Recipe {

public:
    Recipe();

    int recipeId = 0;
    int count = 0;
    int componentsSize = 0;
    int currentComponentIndex = 0;
    const int iodeviceId = 1;
    Component componentArray[maxComponentSize];

    void setRecipeId(int recipeId);
    void setIndexComponentWithId(int componentId);
    void addComponent(int componentId, int targetWeight);
    void updateComponentCurrentWeightWithIndex(int index, int currentWeight);

private:
    void insertComponent(int index, int componentId, int targetWeight);

};

#endif //BSF_ARDUINO_WEIGHTSTATION_RECIPE_H
