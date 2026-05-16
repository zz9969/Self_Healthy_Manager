#include "config.h"
#include "http_server.h"
#include "db.h"

void ConfigHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"GET", "/api/config/cities", handleCities});
}

HttpResponse ConfigHandler::handleCities(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getCityList();

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}
