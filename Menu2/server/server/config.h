#ifndef CONFIG_H
#define CONFIG_H

#include "server.h"

class ConfigHandler {
public:
    static HttpResponse handleCities(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // CONFIG_H
