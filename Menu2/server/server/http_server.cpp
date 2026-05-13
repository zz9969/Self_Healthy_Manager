#include "http_server.h"
#include <iostream>

HttpServer::HttpServer(int port) : port_(port), pool_(4) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::addRoute(const Route& route) {
    routes_.push_back(route);
}

void HttpServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Server] WSAStartup failed" << std::endl;
        return;
    }

    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET) {
        std::cerr << "[Server] socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    int opt = 1;
    setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port_));

    if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Server] bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket_);
        WSACleanup();
        return;
    }

    if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[Server] listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket_);
        WSACleanup();
        return;
    }

    running_.store(true);
    std::cout << "[Server] Listening on port " << port_ << std::endl;

    acceptLoop();

    pool_.shutdown();
    closesocket(listenSocket_);
    WSACleanup();
    std::cout << "[Server] Stopped" << std::endl;
}

void HttpServer::stop() {
    running_.store(false);
    if (listenSocket_ != INVALID_SOCKET) {
        shutdown(listenSocket_, SD_BOTH);
    }
}

void HttpServer::acceptLoop() {
    while (running_.load()) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET clientSock = accept(listenSocket_, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            if (running_.load()) {
                std::cerr << "[Server] accept error: " << WSAGetLastError() << std::endl;
            }
            continue;
        }

        SOCKET sock = clientSock;
        pool_.submit([this, sock]() { handleClient(sock); });
    }
}

void HttpServer::handleClient(SOCKET clientSocket) {
    char buffer[8192];
    std::string rawData;
    int contentLength = 0;
    bool gotContentLength = false;
    bool headersComplete = false;

    while (running_.load()) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) break;

        rawData.append(buffer, bytesReceived);

        if (!headersComplete) {
            size_t headerEnd = rawData.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersComplete = true;
                std::string headerSection = rawData.substr(0, headerEnd);

                size_t clPos = headerSection.find("Content-Length:");
                if (clPos == std::string::npos)
                    clPos = headerSection.find("content-length:");
                if (clPos != std::string::npos) {
                    clPos = headerSection.find_first_not_of(" \t", clPos + 15);
                    size_t clEnd = headerSection.find("\r\n", clPos);
                    contentLength = std::stoi(headerSection.substr(clPos, clEnd - clPos));
                    gotContentLength = true;
                }
            }
        }

        if (headersComplete) {
            size_t headerEnd = rawData.find("\r\n\r\n");
            size_t bodyStart = headerEnd + 4;
            size_t bodySize = rawData.size() - bodyStart;

            if (!gotContentLength || bodySize >= static_cast<size_t>(contentLength)) {
                break;
            }
        }
    }

    if (rawData.empty()) {
        closesocket(clientSocket);
        return;
    }

    HttpRequest req = parseRequest(rawData);
    HttpResponse res = dispatch(req);
    sendResponse(clientSocket, res);

    closesocket(clientSocket);
}

HttpRequest HttpServer::parseRequest(const std::string& rawRequest) {
    HttpRequest req;

    size_t reqLineEnd = rawRequest.find("\r\n");
    if (reqLineEnd == std::string::npos) return req;

    std::string requestLine = rawRequest.substr(0, reqLineEnd);
    size_t space1 = requestLine.find(' ');
    size_t space2 = requestLine.rfind(' ');
    if (space1 != std::string::npos && space2 != std::string::npos && space1 < space2) {
        req.method = requestLine.substr(0, space1);
        req.path = requestLine.substr(space1 + 1, space2 - space1 - 1);
    }

    size_t headerStart = reqLineEnd + 2;
    size_t headerEnd = rawRequest.find("\r\n\r\n", headerStart);
    if (headerEnd == std::string::npos) return req;

    std::string headerSection = rawRequest.substr(headerStart, headerEnd - headerStart);
    size_t pos = 0;
    while (pos < headerSection.size()) {
        size_t lineEnd = headerSection.find("\r\n", pos);
        if (lineEnd == std::string::npos) lineEnd = headerSection.size();
        std::string line = headerSection.substr(pos, lineEnd - pos);
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                value.erase(value.begin());
            req.headers[key] = value;
        }
        pos = (lineEnd < headerSection.size()) ? lineEnd + 2 : headerSection.size();
    }

    size_t bodyStart = headerEnd + 4;
    if (bodyStart < rawRequest.size()) {
        req.body = rawRequest.substr(bodyStart);
    }

    return req;
}

HttpResponse HttpServer::dispatch(const HttpRequest& req) {
    for (auto& route : routes_) {
        if (route.method != req.method) continue;

        auto params = extractPathParams(req.path, route.pathPattern);
        if (params.empty() && route.pathPattern != req.path) {
            continue;
        }

        HttpRequest matchedReq = req;
        matchedReq.pathParams = std::move(params);

        try {
            return route.handler(matchedReq);
        } catch (...) {
            HttpResponse errRes;
            errRes.statusCode = 500;
            errRes.body = json::jsonResponse(500, "Internal Server Error", "{}");
            errRes.headers["Content-Type"] = "application/json";
            errRes.headers["Access-Control-Allow-Origin"] = "*";
            return errRes;
        }
    }

    HttpResponse notFound;
    notFound.statusCode = 404;
    notFound.body = json::jsonResponse(404, "Not Found", "{}");
    notFound.headers["Content-Type"] = "application/json";
    notFound.headers["Access-Control-Allow-Origin"] = "*";
    return notFound;
}

void HttpServer::sendResponse(SOCKET sock, const HttpResponse& res) {
    std::string statusText;
    switch (res.statusCode) {
        case 200: statusText = "OK"; break;
        case 201: statusText = "Created"; break;
        case 400: statusText = "Bad Request"; break;
        case 401: statusText = "Unauthorized"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        default:  statusText = "Unknown"; break;
    }

    std::ostringstream responseLine;
    responseLine << "HTTP/1.1 " << res.statusCode << " " << statusText << "\r\n";

    std::string response = responseLine.str();
    for (auto& [key, val] : res.headers) {
        response += key + ": " + val + "\r\n";
    }

    bool hasContentType = (res.headers.find("Content-Type") != res.headers.end());
    if (!hasContentType) {
        response += "Content-Type: application/json\r\n";
    }

    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Request-ID\r\n";

    response += "Content-Length: " + std::to_string(res.body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += res.body;

    int totalSent = 0;
    while (totalSent < static_cast<int>(response.size())) {
        int sent = send(sock, response.data() + totalSent,
                        static_cast<int>(response.size()) - totalSent, 0);
        if (sent <= 0) break;
        totalSent += sent;
    }
}
