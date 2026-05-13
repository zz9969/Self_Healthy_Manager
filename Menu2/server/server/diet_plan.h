#ifndef DIET_PLAN_H
#define DIET_PLAN_H

#include "server.h"

class DietPlanHandler {
public:
    static HttpResponse handleGenerate(const HttpRequest& req);
    static HttpResponse handleGetPlan(const HttpRequest& req);
    static HttpResponse handleSubstitute(const HttpRequest& req);
    static HttpResponse handleSubstitutes(const HttpRequest& req);
    static HttpResponse handleFeedback(const HttpRequest& req);
    static HttpResponse handleRecipesSearch(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // DIET_PLAN_H
