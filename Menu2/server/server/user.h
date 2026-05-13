#ifndef USER_H
#define USER_H

#include "server.h"

class UserHandler {
public:
    static HttpResponse handleRegister(const HttpRequest& req);
    static HttpResponse handleLogin(const HttpRequest& req);
    static HttpResponse handleTourist(const HttpRequest& req);
    static HttpResponse handleUserInfo(const HttpRequest& req);
    static HttpResponse handleUpdateUserInfo(const HttpRequest& req);
    static HttpResponse handleLogout(const HttpRequest& req);

    static void registerRoutes(class HttpServer& server);
};

#endif // USER_H
