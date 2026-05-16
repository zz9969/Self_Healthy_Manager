#ifndef EXERCISE_H
#define EXERCISE_H

#include "server.h"

class ExerciseHandler {
public:
    static HttpResponse handleTypes(const HttpRequest& req);
    static HttpResponse handleRecord(const HttpRequest& req);
    static HttpResponse handleRecords(const HttpRequest& req);
    static HttpResponse handleCalculate(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // EXERCISE_H
