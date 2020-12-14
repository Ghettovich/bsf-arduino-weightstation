#include "Recipe.h"

Recipe::Recipe() {
}

void Recipe::setRecipeId(int _recipeId)
{
    recipeId = _recipeId;
}
void Recipe::setIndexComponentWithId(int _componentId)
{
    for (int i = 0; i < count; ++i) {
        if(componentArray->componentId == _componentId) {
            return;
        }
    }
}
void Recipe::addComponent(int _componentId, int _targetWeight)
{
    for (int i = 0; i < count; ++i) {
        if(componentArray->componentId == _componentId) {

            return;
        }
    }

    insertComponent(count, _componentId, _targetWeight);
    count++;
}
void Recipe::updateComponentCurrentWeightWithIndex(int index, int currentWeight)
{
    componentArray[index].currentWeight = currentWeight;
}
void Recipe::insertComponent(int index, int componentId, int targetWeight)
{
    componentArray[index].componentId = componentId;
    componentArray[index].targetWeight = targetWeight;
    componentArray[index].currentWeight = 0;
}
