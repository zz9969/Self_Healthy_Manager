#ifndef AI_SERVICE_H
#define AI_SERVICE_H

#include <string>
#include <vector>

struct IngredientInfo {
    std::string name;
    double weight_g;
};

struct NutritionResult {
    double calories;
    double protein_g;
    double fat_g;
    double carbs_g;
    double fiber_g;
    std::vector<IngredientInfo> ingredients;
    bool fromAI;
};

class AiNutritionService {
public:
    static AiNutritionService& getInstance() {
        static AiNutritionService instance;
        return instance;
    }

    void setApiKey(const std::string& key) { apiKey_ = key; }
    bool hasApiKey() const { return !apiKey_.empty(); }

    NutritionResult analyzeDish(const std::string& dishName, const std::string& preference = "");

private:
    AiNutritionService() = default;
    std::string apiKey_;

    std::string callZhipuAPI(const std::string& prompt);
    std::vector<IngredientInfo> parseIngredients(const std::string& jsonStr);
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif // AI_SERVICE_H
