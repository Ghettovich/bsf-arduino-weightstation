#include "Recipe.h"

Recipe::Recipe(int _recipeId) :
    recipeId(_recipeId) {
}
void Recipe::addComponent(int _componentId, int _targetWeight)
{
    for (int i = 0; i < count; ++i) {
        if(componentArray[i].componentId == _componentId) {
            currentComponentIndex = i;
            return;
        }
    }

    insertComponent(count, _componentId, _targetWeight);
    count++;
}
void Recipe::updateWeight(int weight)
{
    componentArray[currentComponentIndex].currentWeight = weight;
}
void Recipe::insertComponent(int index, int componentId, int targetWeight)
{
    componentArray[index].componentId = componentId;
    componentArray[index].targetWeight = targetWeight;
    componentArray[index].currentWeight = 0;
    currentComponentIndex = index;
}
int Recipe::getCurrentComponentId()
{
    return componentArray[currentComponentIndex].componentId;
}
int Recipe::getCurrentWeight()
{
    return componentArray[currentComponentIndex].currentWeight;
}
int Recipe::getRecipeId()
{
    return recipeId;
}
int Recipe::getCurrentIndex()
{
    return currentComponentIndex;
}
