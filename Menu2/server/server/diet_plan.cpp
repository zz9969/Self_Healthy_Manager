#include "diet_plan.h"
#include "http_server.h"
#include "db.h"
#include "weather_service.h"

void DietPlanHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"POST", "/api/diet/plan/generate",   handleGenerate});
    server.addRoute({"GET",  "/api/diet/plan",            handleGetPlan});
    server.addRoute({"PUT",  "/api/diet/plan/substitute", handleSubstitute});
    server.addRoute({"GET",  "/api/diet/substitutes",     handleSubstitutes});
    server.addRoute({"POST", "/api/diet/feedback",        handleFeedback});
    server.addRoute({"GET",  "/api/diet/recipes/search",  handleRecipesSearch});
}

static std::string getWeatherTips(const std::string& tcmLabel) {
    if (tcmLabel == "严寒") return "天气严寒，推荐温补食材：羊肉、生姜、红枣、桂圆，忌生冷";
    if (tcmLabel == "酷暑") return "天气酷暑，推荐清热解暑：绿豆、西瓜、苦瓜、莲子，忌辛辣油腻";
    if (tcmLabel == "潮湿") return "天气潮湿，推荐健脾祛湿：薏米、山药、冬瓜、陈皮，忌甜腻厚味";
    if (tcmLabel == "干燥") return "天气干燥，推荐滋阴润燥：银耳、百合、梨、蜂蜜，忌煎炸辛辣";
    return "天气适宜，推荐均衡饮食，荤素搭配";
}

static std::string getWeatherMealPlan(const std::string& tcmLabel) {
    if (tcmLabel == "严寒") {
        return std::string("{")
            + "\"breakfast\":{"
              "\"name\":\"温补早餐\","
              "\"foods\":["
              "{\"name\":\"红枣小米粥\",\"amount\":\"250g\",\"calories\":150},"
              "{\"name\":\"煮鸡蛋\",\"amount\":\"1个(约50g)\",\"calories\":78},"
              "{\"name\":\"红糖馒头\",\"amount\":\"1个(约100g)\",\"calories\":240}"
              "],\"calories\":468},"
            + "\"lunch\":{"
              "\"name\":\"暖身午餐\","
              "\"foods\":["
              "{\"name\":\"当归羊肉汤\",\"amount\":\"200g\",\"calories\":280},"
              "{\"name\":\"姜汁菠菜\",\"amount\":\"150g\",\"calories\":50},"
              "{\"name\":\"米饭\",\"amount\":\"1碗(约150g)\",\"calories\":170},"
              "{\"name\":\"桂圆红枣茶\",\"amount\":\"1杯(约200ml)\",\"calories\":60}"
              "],\"calories\":560},"
            + "\"dinner\":{"
              "\"name\":\"温养晚餐\","
              "\"foods\":["
              "{\"name\":\"生姜炖鸡\",\"amount\":\"180g\",\"calories\":250},"
              "{\"name\":\"炒山药\",\"amount\":\"150g\",\"calories\":80},"
              "{\"name\":\"杂粮饭\",\"amount\":\"1碗(约130g)\",\"calories\":150}"
              "],\"calories\":480},"
            + "\"total_calories\":1508";
    }
    if (tcmLabel == "酷暑") {
        return std::string("{")
            + "\"breakfast\":{"
              "\"name\":\"消暑早餐\","
              "\"foods\":["
              "{\"name\":\"绿豆粥\",\"amount\":\"250g\",\"calories\":100},"
              "{\"name\":\"凉拌黄瓜\",\"amount\":\"100g\",\"calories\":20},"
              "{\"name\":\"花卷\",\"amount\":\"1个(约80g)\",\"calories\":170}"
              "],\"calories\":290},"
            + "\"lunch\":{"
              "\"name\":\"清凉午餐\","
              "\"foods\":["
              "{\"name\":\"清蒸鲈鱼\",\"amount\":\"150g\",\"calories\":160},"
              "{\"name\":\"冬瓜排骨汤\",\"amount\":\"200g\",\"calories\":120},"
              "{\"name\":\"凉拌苦瓜\",\"amount\":\"100g\",\"calories\":25},"
              "{\"name\":\"米饭\",\"amount\":\"1碗(约150g)\",\"calories\":170}"
              "],\"calories\":475},"
            + "\"dinner\":{"
              "\"name\":\"解暑晚餐\","
              "\"foods\":["
              "{\"name\":\"莲子百合粥\",\"amount\":\"250g\",\"calories\":130},"
              "{\"name\":\"白灼虾\",\"amount\":\"100g\",\"calories\":90},"
              "{\"name\":\"蒜蓉丝瓜\",\"amount\":\"150g\",\"calories\":40}"
              "],\"calories\":260},"
            + "\"total_calories\":1025";
    }
    if (tcmLabel == "潮湿") {
        return std::string("{")
            + "\"breakfast\":{"
              "\"name\":\"祛湿早餐\","
              "\"foods\":["
              "{\"name\":\"薏米红豆粥\",\"amount\":\"250g\",\"calories\":130},"
              "{\"name\":\"煮鸡蛋\",\"amount\":\"1个(约50g)\",\"calories\":78},"
              "{\"name\":\"全麦面包\",\"amount\":\"2片(约60g)\",\"calories\":150}"
              "],\"calories\":358},"
            + "\"lunch\":{"
              "\"name\":\"健脾午餐\","
              "\"foods\":["
              "{\"name\":\"山药炖排骨\",\"amount\":\"200g\",\"calories\":280},"
              "{\"name\":\"陈皮蒸鱼\",\"amount\":\"150g\",\"calories\":160},"
              "{\"name\":\"炒冬瓜\",\"amount\":\"150g\",\"calories\":35},"
              "{\"name\":\"米饭\",\"amount\":\"1碗(约150g)\",\"calories\":170}"
              "],\"calories\":645},"
            + "\"dinner\":{"
              "\"name\":\"利湿晚餐\","
              "\"foods\":["
              "{\"name\":\"薏米莲子汤\",\"amount\":\"200g\",\"calories\":100},"
              "{\"name\":\"清炒山药\",\"amount\":\"150g\",\"calories\":80},"
              "{\"name\":\"杂粮饭\",\"amount\":\"1碗(约130g)\",\"calories\":150}"
              "],\"calories\":330},"
            + "\"total_calories\":1333";
    }
    if (tcmLabel == "干燥") {
        return std::string("{")
            + "\"breakfast\":{"
              "\"name\":\"润燥早餐\","
              "\"foods\":["
              "{\"name\":\"银耳百合粥\",\"amount\":\"250g\",\"calories\":120},"
              "{\"name\":\"蜂蜜水\",\"amount\":\"1杯(约200ml)\",\"calories\":60},"
              "{\"name\":\"蒸南瓜\",\"amount\":\"100g\",\"calories\":50}"
              "],\"calories\":230},"
            + "\"lunch\":{"
              "\"name\":\"滋阴午餐\","
              "\"foods\":["
              "{\"name\":\"雪梨炖排骨\",\"amount\":\"200g\",\"calories\":260},"
              "{\"name\":\"百合炒西芹\",\"amount\":\"150g\",\"calories\":55},"
              "{\"name\":\"莲藕汤\",\"amount\":\"1碗(约250ml)\",\"calories\":80},"
              "{\"name\":\"米饭\",\"amount\":\"1碗(约150g)\",\"calories\":170}"
              "],\"calories\":565},"
            + "\"dinner\":{"
              "\"name\":\"润肺晚餐\","
              "\"foods\":["
              "{\"name\":\"银耳木瓜汤\",\"amount\":\"200g\",\"calories\":90},"
              "{\"name\":\"清蒸鲈鱼\",\"amount\":\"150g\",\"calories\":160},"
              "{\"name\":\"杂粮饭\",\"amount\":\"1碗(约130g)\",\"calories\":150}"
              "],\"calories\":400},"
            + "\"total_calories\":1195";
    }

    return std::string("{")
        + "\"breakfast\":{"
          "\"name\":\"营养早餐\","
          "\"foods\":["
          "{\"name\":\"小米粥\",\"amount\":\"200g\",\"calories\":120},"
          "{\"name\":\"鸡蛋\",\"amount\":\"1个(约50g)\",\"calories\":78},"
          "{\"name\":\"馒头\",\"amount\":\"1个(约100g)\",\"calories\":221}"
          "],\"calories\":419},"
        + "\"lunch\":{"
          "\"name\":\"健康午餐\","
          "\"foods\":["
          "{\"name\":\"清蒸鲈鱼\",\"amount\":\"150g\",\"calories\":160},"
          "{\"name\":\"蒜蓉西兰花\",\"amount\":\"200g\",\"calories\":68},"
          "{\"name\":\"糙米饭\",\"amount\":\"1碗(约150g)\",\"calories\":170},"
          "{\"name\":\"紫菜蛋花汤\",\"amount\":\"1碗(约250ml)\",\"calories\":60}"
          "],\"calories\":458},"
        + "\"dinner\":{"
          "\"name\":\"清淡晚餐\","
          "\"foods\":["
          "{\"name\":\"番茄牛腩\",\"amount\":\"180g\",\"calories\":230},"
          "{\"name\":\"凉拌黄瓜\",\"amount\":\"150g\",\"calories\":30},"
          "{\"name\":\"杂粮饭\",\"amount\":\"1碗(约130g)\",\"calories\":150}"
          "],\"calories\":410},"
        + "\"total_calories\":1287";
}

HttpResponse DietPlanHandler::handleGenerate(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string mealType = json::getString(req.body, "meal_type", "lunch");
    std::string city = json::getString(req.body, "city", "北京");

    WeatherInfo weather = WeatherService::getInstance().getWeather(city);
    std::string tcmLabel = weather.tcmLabel;
    std::string weatherTips = getWeatherTips(tcmLabel);

    std::string mealPlan = getWeatherMealPlan(tcmLabel);

    std::string data = std::string("{")
        + "\"plan_id\":1,"
        + mealPlan + ","
        + "\"weather\":{"
          "\"city\":\"" + json::escape(city) + "\","
          "\"temperature\":\"" + json::escape(weather.temperature) + "\","
          "\"text\":\"" + json::escape(weather.text) + "\","
          "\"tcm_label\":\"" + json::escape(tcmLabel) + "\""
        "},"
        + "\"nutrition_tips\":\"" + json::escape(weatherTips) + "\""
        + "}";

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietPlanHandler::handleGetPlan(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = "{}";

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(4001, "暂无推荐计划，请先生成", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietPlanHandler::handleSubstitute(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    int planId = json::getInt(req.body, "plan_id", 0);
    int slotIndex = json::getInt(req.body, "slot_index", -1);
    int newRecipeId = json::getInt(req.body, "new_recipe_id", 0);

    if (planId <= 0 || slotIndex < 0 || newRecipeId <= 0) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(4002, "请提供有效的替换参数", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = json::buildDataNum({
        {"plan_id",      std::to_string(planId)},
        {"slot_index",   std::to_string(slotIndex)},
        {"new_recipe_id", std::to_string(newRecipeId)}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "替换成功", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietPlanHandler::handleSubstitutes(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string item1 = std::string("{\"name\":\"香煎鸡胸肉\",\"amount\":\"120g\",\"calories\":165,\"protein_g\":31,\"reason\":\"高蛋白低脂替代\"}");
    std::string item2 = std::string("{\"name\":\"清蒸虾仁\",\"amount\":\"100g\",\"calories\":98,\"protein_g\":20,\"reason\":\"海鲜优质蛋白\"}");
    std::string item3 = std::string("{\"name\":\"豆腐炖鱼\",\"amount\":\"150g\",\"calories\":150,\"protein_g\":18,\"reason\":\"植物蛋白组合\"}");
    std::string altItems = std::string("[") + item1 + "," + item2 + "," + item3 + "]";

    std::string data = std::string("{\"substitutes\":") + altItems + "}";

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietPlanHandler::handleFeedback(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    int planId = json::getInt(req.body, "plan_id", 0);
    int rating = json::getInt(req.body, "rating", 0);

    if (planId <= 0 || rating < 1 || rating > 5) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(4003, "请提供有效的评分(1-5)", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "反馈提交成功", "{}");
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietPlanHandler::handleRecipesSearch(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getFoodList();

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}
