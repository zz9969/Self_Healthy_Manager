#include "exercise.h"
#include "http_server.h"
#include "db.h"

void ExerciseHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"GET",  "/api/exercise/types",     handleTypes});
    server.addRoute({"POST", "/api/exercise/record",    handleRecord});
    server.addRoute({"GET",  "/api/exercise/records",   handleRecords});
    server.addRoute({"POST", "/api/exercise/calculate", handleCalculate});
}

HttpResponse ExerciseHandler::handleTypes(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getExerciseTypes();

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse ExerciseHandler::handleRecord(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    int typeId = json::getInt(req.body, "type_id", 0);
    int duration = json::getInt(req.body, "duration", 0);
    int intensity = json::getInt(req.body, "intensity", 2);

    if (typeId <= 0 || duration <= 0) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(3001, "请提供有效的运动类型和时长", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto userOpt = DbManager::getInstance().getUserById(userId);
    double weight = 65.0;
    if (userOpt.has_value()) {
        weight = userOpt.value().weight;
    }

    double metBase = 3.0;
    double intensityFactor[] = {0.7, 1.0, 1.3, 1.7};
    int idx = intensity - 1;
    if (idx < 0 || idx > 3) idx = 1;
    double calories = metBase * intensityFactor[idx] * weight * (duration / 60.0);

    int recordId = DbManager::getInstance().addExerciseRecord(userId, typeId, duration, intensity, calories);

    std::string data = json::buildDataNum({
        {"record_id", std::to_string(recordId)},
        {"type_id",   std::to_string(typeId)},
        {"duration",  std::to_string(duration)},
        {"intensity", std::to_string(intensity)},
        {"calories",  std::to_string(static_cast<int>(calories))}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "运动记录添加成功", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse ExerciseHandler::handleRecords(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getTodayExercise(userId);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse ExerciseHandler::handleCalculate(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    int typeId = json::getInt(req.body, "type_id", 0);
    int duration = json::getInt(req.body, "duration", 0);
    int intensity = json::getInt(req.body, "intensity", 2);

    if (typeId <= 0 || duration <= 0) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(3001, "请提供有效的运动类型和时长", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto userOpt = DbManager::getInstance().getUserById(userId);
    double weight = 65.0;
    if (userOpt.has_value()) {
        weight = userOpt.value().weight;
    }

    double metBase = 3.0;
    double intensityFactor[] = {0.7, 1.0, 1.3, 1.7};
    int idx = intensity - 1;
    if (idx < 0 || idx > 3) idx = 1;
    double calories = metBase * intensityFactor[idx] * weight * (duration / 60.0);

    std::string data = json::buildDataNum({
        {"type_id",    std::to_string(typeId)},
        {"duration",   std::to_string(duration)},
        {"intensity",  std::to_string(intensity)},
        {"calories",   std::to_string(static_cast<int>(calories))},
        {"weight_kg",  std::to_string(static_cast<int>(weight))}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}
