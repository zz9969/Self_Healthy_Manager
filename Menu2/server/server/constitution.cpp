#include "constitution.h"
#include "http_server.h"
#include "db.h"
#include <iostream>

void ConstitutionHandler::registerRoutes(HttpServer& server) {
    server.addRoute({"GET",  "/api/constitution/questions", handleQuestions});
    server.addRoute({"POST", "/api/constitution/analyze", handleAnalyze});
    server.addRoute({"GET",  "/api/constitution/result", handleResult});
}

HttpResponse ConstitutionHandler::handleQuestions(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    std::cout << "[Constitution] handleQuestions userId=" << userId << std::endl;

    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getQuestionnaire();

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse ConstitutionHandler::handleAnalyze(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string answersStr = json::getArrayStr(req.body, "answers");
    if (answersStr.empty() || answersStr == "[]") {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(2001, "请提交问卷答案", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::map<int, std::string> answers;

    std::string remaining = answersStr;
    while (remaining.find('{') != std::string::npos) {
        size_t start = remaining.find('{');
        size_t end = remaining.find('}', start);
        if (end == std::string::npos) break;

        std::string item = remaining.substr(start + 1, end - start - 1);
        remaining = remaining.substr(end + 1);

        int qid = json::getInt(item, "question_id", 0);
        std::string optKey = json::getString(item, "option_key", "");

        if (qid > 0 && !optKey.empty()) {
            answers[qid] = optKey;
        }
    }

    if (answers.empty()) {
        HttpResponse res;
        res.statusCode = 200;
        res.body = json::jsonResponse(2002, "答案格式错误", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().analyzeConstitution(userId, answers);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse ConstitutionHandler::handleResult(const HttpRequest& req) {
    int userId = AuthMiddleware::getUserId(req);
    if (userId <= 0) {
        HttpResponse res;
        res.statusCode = 401;
        res.body = json::jsonResponse(9001, "未认证", "{}");
        res.headers["Content-Type"] = "application/json";
        return res;
    }

    std::string data = DbManager::getInstance().getConstitutionResult(userId);

    HttpResponse res;
    res.statusCode = 200;
    res.body = json::jsonResponse(0, "", data);
    res.headers["Content-Type"] = "application/json";
    return res;
}
