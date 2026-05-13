#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "server.h"
#include "pool.h"
#include <vector>
#include <atomic>

class HttpServer {
public:
    explicit HttpServer(int port = 8888);
    ~HttpServer();

    void addRoute(const Route& route);
    void start();
    void stop();

private:
    void acceptLoop();
    void handleClient(SOCKET clientSocket);
    HttpResponse dispatch(const HttpRequest& req);
    HttpRequest parseRequest(const std::string& rawRequest);
    void sendResponse(SOCKET sock, const HttpResponse& res);

    int port_;
    SOCKET listenSocket_{INVALID_SOCKET};
    std::vector<Route> routes_;
    ThreadPool pool_;
    std::atomic<bool> running_{false};
};

#endif // HTTP_SERVER_H
