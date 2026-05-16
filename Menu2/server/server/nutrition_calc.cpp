#include "nutrition_calc.h"
#include <iostream>
#include <algorithm>

std::map<std::string, FoodNutrition> NutritionCalculator::nutritionDB_;
bool NutritionCalculator::initialized_ = false;

void NutritionCalculator::initNutritionDB() {
    if (initialized_) return;
    initialized_ = true;

    struct Entry { const char* name; double cal; double pro; double fat; double carb; double fib; };
    Entry entries[] = {
        {"鸡胸肉", 133, 31.0, 3.6, 0, 0},
        {"鸡腿肉", 181, 26.0, 8.0, 0, 0},
        {"鸡翅", 194, 17.4, 11.8, 0.4, 0},
        {"鸡", 167, 19.3, 9.4, 1.3, 0},
        {"猪肉", 143, 20.3, 6.2, 1.5, 0},
        {"猪里脊", 155, 20.2, 7.9, 1.5, 0},
        {"五花肉", 349, 14.5, 30.8, 3.2, 0},
        {"排骨", 264, 18.0, 20.4, 1.7, 0},
        {"牛肉", 125, 19.9, 4.2, 2.0, 0},
        {"牛腩", 332, 17.1, 29.3, 0, 0},
        {"羊肉", 203, 19.0, 14.1, 0, 0},
        {"鸭肉", 240, 15.5, 19.7, 0.1, 0},
        {"鱼肉", 104, 17.6, 3.3, 0, 0},
        {"草鱼", 113, 16.6, 5.2, 0, 0},
        {"鲤鱼", 109, 17.6, 4.1, 0.5, 0},
        {"鲫鱼", 108, 17.1, 2.7, 3.8, 0},
        {"鲈鱼", 105, 18.6, 3.1, 0, 0},
        {"带鱼", 127, 17.7, 4.9, 3.1, 0},
        {"虾仁", 87, 18.6, 0.8, 1.0, 0},
        {"虾", 93, 18.6, 0.8, 2.8, 0},
        {"鸡蛋", 144, 13.3, 8.8, 2.8, 0},
        {"鸭蛋", 180, 12.6, 13.0, 3.1, 0},
        {"豆腐", 81, 8.1, 3.7, 4.2, 0.4},
        {"豆腐干", 140, 15.8, 5.9, 8.2, 0.6},
        {"豆腐皮", 409, 44.6, 17.4, 18.8, 0.2},
        {"豆浆", 31, 3.0, 1.6, 1.2, 1.1},
        {"牛奶", 54, 3.0, 3.2, 3.4, 0},
        {"大米", 346, 7.4, 0.8, 77.9, 0.7},
        {"米饭", 116, 2.6, 0.3, 25.9, 0.3},
        {"面条", 110, 3.5, 0.5, 23.0, 0.3},
        {"面粉", 349, 11.2, 1.5, 73.6, 2.1},
        {"馒头", 221, 7.0, 1.1, 47.0, 1.3},
        {"包子", 227, 7.5, 2.8, 44.2, 1.0},
        {"饺子", 196, 8.5, 5.2, 31.5, 0.8},
        {"小米", 358, 9.0, 3.1, 75.1, 1.6},
        {"玉米", 112, 4.0, 1.2, 22.8, 2.9},
        {"红薯", 99, 1.1, 0.2, 23.1, 1.6},
        {"土豆", 76, 2.0, 0.2, 16.5, 0.7},
        {"山药", 56, 1.9, 0.2, 11.6, 0.8},
        {"芋头", 79, 2.2, 0.2, 17.1, 1.0},
        {"燕麦", 367, 15.0, 6.7, 61.6, 5.3},
        {"花生米", 563, 24.8, 44.3, 21.7, 5.5},
        {"花生", 563, 24.8, 44.3, 21.7, 5.5},
        {"核桃", 627, 14.9, 58.8, 19.1, 9.5},
        {"芝麻", 531, 19.1, 46.1, 10.0, 5.9},
        {"白菜", 18, 1.5, 0.1, 2.9, 0.8},
        {"菠菜", 24, 2.6, 0.3, 2.8, 1.7},
        {"芹菜", 14, 0.8, 0.1, 2.6, 1.4},
        {"西兰花", 33, 4.1, 0.6, 3.0, 1.6},
        {"西红柿", 19, 0.9, 0.2, 3.5, 0.5},
        {"番茄", 19, 0.9, 0.2, 3.5, 0.5},
        {"黄瓜", 15, 0.8, 0.2, 2.9, 0.5},
        {"茄子", 21, 1.1, 0.2, 3.6, 1.3},
        {"青椒", 22, 1.0, 0.2, 4.2, 1.4},
        {"辣椒", 23, 1.4, 0.3, 3.7, 1.9},
        {"干辣椒", 212, 15.0, 8.0, 40.0, 25.0},
        {"胡萝卜", 37, 1.0, 0.2, 7.7, 1.1},
        {"白萝卜", 21, 0.9, 0.1, 4.0, 1.0},
        {"冬瓜", 11, 0.4, 0.2, 1.9, 0.7},
        {"南瓜", 22, 0.7, 0.1, 4.5, 0.8},
        {"丝瓜", 20, 1.0, 0.2, 3.6, 0.6},
        {"苦瓜", 19, 1.0, 0.1, 3.5, 1.4},
        {"莲藕", 70, 1.9, 0.2, 15.2, 1.2},
        {"豆角", 34, 2.5, 0.2, 5.7, 1.5},
        {"四季豆", 30, 2.4, 0.1, 5.7, 1.6},
        {"洋葱", 39, 1.1, 0.2, 8.1, 0.9},
        {"大蒜", 126, 4.5, 0.2, 27.6, 1.1},
        {"蒜", 126, 4.5, 0.2, 27.6, 1.1},
        {"生姜", 41, 1.3, 0.6, 7.6, 2.7},
        {"大葱", 30, 1.7, 0.3, 5.2, 1.3},
        {"葱", 30, 1.7, 0.3, 5.2, 1.3},
        {"韭菜", 26, 2.4, 0.4, 3.2, 1.4},
        {"木耳", 21, 1.5, 0.2, 5.4, 2.6},
        {"银耳", 200, 10.0, 1.4, 36.9, 30.4},
        {"香菇", 19, 2.2, 0.3, 2.4, 3.3},
        {"蘑菇", 20, 2.7, 0.1, 2.4, 2.1},
        {"苹果", 53, 0.2, 0.2, 12.3, 1.2},
        {"香蕉", 93, 1.4, 0.2, 20.8, 1.2},
        {"橙子", 48, 0.8, 0.2, 10.5, 0.6},
        {"梨", 44, 0.4, 0.2, 10.6, 1.4},
        {"酱油", 63, 5.6, 0.1, 9.9, 0},
        {"醋", 31, 2.1, 0.3, 4.9, 0},
        {"白糖", 400, 0, 0, 99.5, 0},
        {"食盐", 0, 0, 0, 0, 0},
        {"淀粉", 346, 1.5, 0, 85.8, 0.8},
        {"食用油", 899, 0, 99.9, 0, 0},
        {"花生油", 899, 0, 99.9, 0, 0},
        {"菜籽油", 899, 0, 99.9, 0, 0},
        {"猪油", 897, 0.2, 99.6, 0, 0},
        {"料酒", 15, 0.3, 0, 2.2, 0},
        {"蚝油", 130, 5.3, 0.3, 23.5, 0},
        {"豆瓣酱", 178, 5.4, 5.2, 26.2, 2.2},
        {"味精", 268, 40.1, 0.2, 26.5, 0},
        {"花椒", 258, 6.7, 8.9, 36.2, 28.7},
        {"八角", 281, 5.3, 5.6, 55.2, 18.4},
        {"桂皮", 199, 3.5, 1.5, 44.3, 16.5},
    };

    for (auto& e : entries) {
        FoodNutrition fn;
        fn.calories = e.cal;
        fn.protein_g = e.pro;
        fn.fat_g = e.fat;
        fn.carbs_g = e.carb;
        fn.fiber_g = e.fib;
        nutritionDB_[e.name] = fn;
    }

    std::cout << "[NutritionCalc] Initialized with " << nutritionDB_.size() << " food items" << std::endl;
}

FoodNutrition NutritionCalculator::lookupFood(const std::string& name) {
    initNutritionDB();

    auto it = nutritionDB_.find(name);
    if (it != nutritionDB_.end()) return it->second;

    for (auto& kv : nutritionDB_) {
        if (name.find(kv.first) != std::string::npos || kv.first.find(name) != std::string::npos) {
            return kv.second;
        }
    }

    FoodNutrition fallback;
    fallback.calories = 150;
    fallback.protein_g = 5;
    fallback.fat_g = 5;
    fallback.carbs_g = 20;
    fallback.fiber_g = 1;
    return fallback;
}

NutritionResult NutritionCalculator::calculate(const std::vector<IngredientInfo>& ingredients) {
    initNutritionDB();

    NutritionResult result;
    result.calories = 0;
    result.protein_g = 0;
    result.fat_g = 0;
    result.carbs_g = 0;
    result.fiber_g = 0;
    result.fromAI = true;
    result.ingredients = ingredients;

    for (const auto& ing : ingredients) {
        FoodNutrition fn = lookupFood(ing.name);
        double ratio = ing.weight_g / 100.0;
        result.calories += fn.calories * ratio;
        result.protein_g += fn.protein_g * ratio;
        result.fat_g += fn.fat_g * ratio;
        result.carbs_g += fn.carbs_g * ratio;
        result.fiber_g += fn.fiber_g * ratio;
    }

    return result;
}

NutritionResult NutritionCalculator::defaultEstimation(const std::string& dishName) {
    initNutritionDB();

    NutritionResult result;
    result.fromAI = false;
    result.ingredients.clear();

    double baseCal = 300;
    double basePro = 15;
    double baseFat = 12;
    double baseCarb = 25;
    double baseFib = 2;

    if (dishName.find("汤") != std::string::npos) {
        baseCal = 120; basePro = 8; baseFat = 5; baseCarb = 10;
    } else if (dishName.find("粥") != std::string::npos) {
        baseCal = 150; basePro = 4; baseFat = 1; baseCarb = 30;
    } else if (dishName.find("面") != std::string::npos) {
        baseCal = 400; basePro = 12; baseFat = 8; baseCarb = 65;
    } else if (dishName.find("饭") != std::string::npos) {
        baseCal = 450; basePro = 15; baseFat = 10; baseCarb = 70;
    } else if (dishName.find("鸡") != std::string::npos || dishName.find("鸭") != std::string::npos) {
        baseCal = 350; basePro = 25; baseFat = 18; baseCarb = 10;
    } else if (dishName.find("鱼") != std::string::npos || dishName.find("虾") != std::string::npos) {
        baseCal = 250; basePro = 22; baseFat = 8; baseCarb = 8;
    } else if (dishName.find("肉") != std::string::npos) {
        baseCal = 380; basePro = 20; baseFat = 22; baseCarb = 12;
    } else if (dishName.find("菜") != std::string::npos || dishName.find("蔬") != std::string::npos) {
        baseCal = 120; basePro = 4; baseFat = 5; baseCarb = 12;
    } else if (dishName.find("豆") != std::string::npos) {
        baseCal = 200; basePro = 12; baseFat = 8; baseCarb = 15;
    } else if (dishName.find("蛋") != std::string::npos) {
        baseCal = 200; basePro = 14; baseFat = 10; baseCarb = 8;
    }

    result.calories = baseCal;
    result.protein_g = basePro;
    result.fat_g = baseFat;
    result.carbs_g = baseCarb;
    result.fiber_g = baseFib;

    return result;
}
