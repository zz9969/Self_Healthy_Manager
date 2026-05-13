#include "diet.h"
#include "http_server.h"
#include "db.h"

void DietHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"POST",   "/api/record/diet",         handleRecord});
    server.addRoute({"GET",    "/api/record/diet",         handleToday});
    server.addRoute({"DELETE", "/api/record/diet/:id",     handleDelete});
    server.addRoute({"GET",    "/api/report/daily",        handleStats});
}

HttpResponse DietHandler::handleRecord(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string mealType = json::getString(req.body, "meal_type");
    std::string notes = json::getString(req.body, "notes");

    int recordId = DbManager::getInstance().saveDietRecord(userId, mealType, req.body, notes);
    if (recordId <= 0) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(5002, "记录失败，请提供有效的recipe_id", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = json::buildDataNum({
        {"record_id", std::to_string(recordId)},
        {"meal_type", "\"" + mealType + "\""}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "记录成功", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietHandler::handleToday(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string recordsJson = DbManager::getInstance().getTodayRecords(userId);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", recordsJson);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietHandler::handleDelete(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto it = req.pathParams.find("id");
    std::string recordIdStr = (it != req.pathParams.end()) ? it->second : "0";
    int recordId = 0;
    try { recordId = std::stoi(recordIdStr); } catch (...) {}

    bool ok = DbManager::getInstance().deleteDietRecord(recordId, userId);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(ok ? 0 : 5002, ok ? "删除成功" : "删除失败", "{}");
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse DietHandler::handleStats(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string nutritionJson = DbManager::getInstance().getTodayNutrition(userId);
    std::string recJson = DbManager::getInstance().getNutritionRecommendation(userId);

    std::string data = std::string("{\"intake\":") + nutritionJson
                     + ",\"target\":" + recJson + "}";

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}
