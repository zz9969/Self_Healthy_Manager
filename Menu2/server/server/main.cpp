#include "server.h"
#include "http_server.h"
#include "user.h"
#include "diet.h"
#include "diet_plan.h"
#include "constitution.h"
#include "exercise.h"
#include "config.h"
#include "db.h"
#include "weather_service.h"
#include "ai_service.h"
#include <iostream>
#include <fstream>

BOOL WINAPI ConsoleHandler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT ||
        fdwCtrlType == CTRL_CLOSE_EVENT ||
        fdwCtrlType == CTRL_LOGOFF_EVENT ||
        fdwCtrlType == CTRL_SHUTDOWN_EVENT) {
        std::cout << "\n[Main] Shutting down..." << std::endl;
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "[Main] Failed to install console handler" << std::endl;
        return 1;
    }

    std::cout << "=== Daily Diet Lightweight Server ===" << std::endl;
    std::cout << "[Main] Initializing..." << std::endl;

    if (!DbManager::getInstance().initialize()) {
        std::cerr << "[Main] Database initialization failed, running in mock mode" << std::endl;
    }

    char* envKey = nullptr;
    size_t len = 0;
    if (_dupenv_s(&envKey, &len, "SENIVERSE_API_KEY") == 0 && envKey != nullptr && strlen(envKey) > 0) {
        WeatherService::getInstance().setApiKey(envKey);
        std::cout << "[Main] Weather API key loaded from env" << std::endl;
        free(envKey);
    } else {
        std::ifstream cfgFile("server_config.json");
        if (cfgFile.is_open()) {
            std::string content((std::istreambuf_iterator<char>(cfgFile)),
                                 std::istreambuf_iterator<char>());
            std::string key = json::getString(content, "seniverse_api_key", "");
            if (!key.empty()) {
                WeatherService::getInstance().setApiKey(key);
                std::cout << "[Main] Weather API key loaded from config" << std::endl;
            }
            std::string zhipuKey = json::getString(content, "zhipu_api_key", "");
            if (!zhipuKey.empty()) {
                AiNutritionService::getInstance().setApiKey(zhipuKey);
                std::cout << "[Main] Zhipu AI API key loaded from config" << std::endl;
            } else {
                std::cout << "[Main] Zhipu AI API key not configured, AI nutrition disabled" << std::endl;
            }
            cfgFile.close();
        }
    }

    try {
        DbManager::getInstance().fixZeroNutritionRecipes();
    } catch (const std::exception& e) {
        std::cerr << "[Main] fixZeroNutritionRecipes failed: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[Main] fixZeroNutritionRecipes failed with unknown error" << std::endl;
    }

    HttpServer server(8888);

    UserHandler::registerRoutes(server);
    ConstitutionHandler::registerRoutes(server);
    ExerciseHandler::registerRoutes(server);
    DietPlanHandler::registerRoutes(server);
    DietHandler::registerRoutes(server);
    ConfigHandler::registerRoutes(server);

    std::cout << "[Main] Registered 24 endpoints on port 8888" << std::endl;
    std::cout << "[Main] Press Ctrl+C to stop" << std::endl;

    server.start();

    std::cout << "[Main] Server exited cleanly." << std::endl;
    return 0;
}
