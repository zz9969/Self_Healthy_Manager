#ifndef NUTRITION_CALC_H
#define NUTRITION_CALC_H

#include <string>
#include <vector>
#include <map>
#include "ai_service.h"

struct FoodNutrition {
    double calories;
    double protein_g;
    double fat_g;
    double carbs_g;
    double fiber_g;
};

class NutritionCalculator {
public:
    static NutritionResult calculate(const std::vector<IngredientInfo>& ingredients);
    static NutritionResult defaultEstimation(const std::string& dishName);
    static FoodNutrition lookupFood(const std::string& name);

private:
    static void initNutritionDB();
    static std::map<std::string, FoodNutrition> nutritionDB_;
    static bool initialized_;
};

#endif // NUTRITION_CALC_H
