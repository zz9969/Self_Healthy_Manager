#include "user.h"
#include "http_server.h"
#include "db.h"

void UserHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"POST", "/api/user/register", handleRegister});
    server.addRoute({"POST", "/api/user/login", handleLogin});
    server.addRoute({"POST", "/api/auth/tourist", handleTourist});
    server.addRoute({"GET",  "/api/user/profile", handleUserInfo});
    server.addRoute({"PUT",  "/api/user/profile", handleUpdateUserInfo});
    server.addRoute({"POST", "/api/auth/logout", handleLogout});
}

HttpResponse UserHandler::handleRegister(const HttpRequest& req) {
    std::string phone = json::getString(req.body, "phone");
    std::string password = json::getString(req.body, "password");

    std::cout << "[User] Register attempt: phone=" << phone << std::endl;
    double height = json::getDouble(req.body, "height");
    double weight = json::getDouble(req.body, "weight");
    std::string genderStr = json::getString(req.body, "gender");
    int age = json::getInt(req.body, "age");

    int gender = 1;
    if (genderStr == "female" || genderStr == "0" || genderStr == "女") gender = 0;

    if (phone.empty() || password.empty()) {
        HttpResponse res;
        res.statusCode = 400;
        res.body = json::jsonResponse(1001, "手机号和密码不能为空", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    int userId = DbManager::getInstance().createUser(phone, password, height, weight, std::to_string(gender), age);
    if (userId <= 0) {
        auto userOpt = DbManager::getInstance().verifyLogin(phone, password);
        if (userOpt.has_value()) {
            auto& user = userOpt.value();
            std::string sessionId = DbManager::getInstance().createSession(user.userId);
            std::string data = json::buildDataNum({
                {"user_id", std::to_string(user.userId)},
                {"token",   std::string("\"") + sessionId + "\""},
                {"phone",   std::string("\"") + json::escape(phone) + "\""}
            });
            HttpResponse res;
            res.statusCode = 200;
            res.body = json::jsonResponse(0, "该手机号已注册，已自动登录", data);
            res.headers["Content-Type"] = "application/json";
            return res;
        }

        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(1001, "该手机号已注册，请直接登录", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string sessionId = DbManager::getInstance().createSession(userId);

    std::string data = json::buildDataNum({
        {"user_id", std::to_string(userId)},
        {"token",   std::string("\"") + sessionId + "\""},
        {"phone",   std::string("\"") + json::escape(phone) + "\""}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "注册成功", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse UserHandler::handleLogin(const HttpRequest& req) {
    std::string phone = json::getString(req.body, "phone");
    std::string password = json::getString(req.body, "password");

    std::cout << "[User] Login attempt: phone=" << phone << std::endl;

    if (phone.empty() || password.empty()) {
        HttpResponse res;
        res.statusCode = 400;
        res.body = json::jsonResponse(1001, "手机号和密码不能为空", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto userOpt = DbManager::getInstance().verifyLogin(phone, password);
    if (!userOpt.has_value()) {
        std::cout << "[User] Login failed: invalid credentials for " << phone << std::endl;
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(1003, "手机号或密码错误", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto& user = userOpt.value();
    std::string sessionId = DbManager::getInstance().createSession(user.userId);

    std::cout << "[User] Login success: userId=" << user.userId << " sessionId=" << sessionId << std::endl;

    std::string data = json::buildDataNum({
        {"user_id",  std::to_string(user.userId)},
        {"token",    std::string("\"") + sessionId + "\""},
        {"phone",    std::string("\"") + json::escape(phone) + "\""},
        {"nickname", std::string("\"") + json::escape(user.nickname) + "\""}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "登录成功", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse UserHandler::handleTourist(const HttpRequest& req) {
    std::string deviceInfo = json::getString(req.body, "device_info");

    std::string touristPhone = "tourist_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    int userId = DbManager::getInstance().createUser(touristPhone, "tourist", 170, 65, "1", 25);

    std::string sessionId;
    if (userId > 0) {
        sessionId = DbManager::getInstance().createSession(userId);
    } else {
        sessionId = DbManager::getInstance().createSession(1);
    }

    std::string data = json::buildDataNum({
        {"user_id",    std::to_string(userId > 0 ? userId : 1)},
        {"token",      std::string("\"") + sessionId + "\""},
        {"is_tourist", "true"}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "游客模式已开启", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse UserHandler::handleUserInfo(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto userOpt = DbManager::getInstance().getUserById(userId);
    if (!userOpt.has_value()) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(1002, "用户不存在", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    auto& user = userOpt.value();
    std::string data = json::buildDataNum({
        {"user_id",  std::to_string(user.userId)},
        {"phone",    std::string("\"") + json::escape(user.phone) + "\""},
        {"nickname", std::string("\"") + json::escape(user.nickname) + "\""},
        {"height",   std::to_string(static_cast<int>(user.height))},
        {"weight",   std::to_string(static_cast<int>(user.weight))},
        {"gender",   user.gender == "0" ? std::string("\"女\"") : std::string("\"男\"")},
        {"age",      std::to_string(user.age)},
        {"activity_level", std::to_string(user.activityLevel)}
    });

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse UserHandler::handleUpdateUserInfo(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    double height = json::getDouble(req.body, "height");
    double weight = json::getDouble(req.body, "weight");
    int age = json::getInt(req.body, "age");
    int activityLevel = json::getInt(req.body, "activity_level", 1);

    bool ok = DbManager::getInstance().updateUser(userId, height, weight, age, activityLevel);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(ok ? 0 : 9005, ok ? "更新成功" : "更新失败", "{}");
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse UserHandler::handleLogout(const HttpRequest& req) {
    std::string token = AuthMiddleware::extractToken(req);
    if (!token.empty()) {
        DbManager::getInstance().deleteSession(token);
    }

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "登出成功", "{}");
    res.headers["Content-Type"] = "application/json";
    return res;
}
