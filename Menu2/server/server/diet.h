#ifndef DIET_H
#define DIET_H

#include "server.h"

class DietHandler {
public:
    static HttpResponse handleRecord(const HttpRequest& req);
    static HttpResponse handleToday(const HttpRequest& req);
    static HttpResponse handleDelete(const HttpRequest& req);
    static HttpResponse handleStats(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // DIET_H
