#ifndef CONSTITUTION_H
#define CONSTITUTION_H

#include "server.h"

class ConstitutionHandler {
public:
    static HttpResponse handleQuestions(const HttpRequest& req);
    static HttpResponse handleAnalyze(const HttpRequest& req);
    static HttpResponse handleResult(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // CONSTITUTION_H
