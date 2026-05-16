#include "weather_service.h"
#include "server.h"
#include <wininet.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "wininet.lib")

WeatherInfo WeatherService::getWeather(const std::string& city) {
    if (city.empty()) {
        WeatherInfo info;
        info.valid = false;
        info.tcmLabel = "常温";
        return info;
    }

    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = cache_.find(city);
        if (it != cache_.end()) {
            auto now = std::chrono::steady_clock::now();
            auto cachedTime = std::chrono::steady_clock::time_point(
                std::chrono::seconds(std::stoll(it->second.fetchTime)));
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - cachedTime).count();
            if (elapsed < CACHE_TTL_SECONDS && it->second.valid) {
                return it->second;
            }
        }
    }

    if (apiKey_.empty()) {
        char* envKey = nullptr;
        size_t len = 0;
        if (_dupenv_s(&envKey, &len, "SENIVERSE_API_KEY") == 0 && envKey != nullptr && strlen(envKey) > 0) {
            apiKey_ = envKey;
            free(envKey);
        }
    }

    if (apiKey_.empty()) {
        std::ifstream cfgFile("server_config.json");
        if (cfgFile.is_open()) {
            std::string content((std::istreambuf_iterator<char>(cfgFile)),
                                 std::istreambuf_iterator<char>());
            apiKey_ = json::getString(content, "seniverse_api_key", "");
            cfgFile.close();
        }
    }

    if (apiKey_.empty()) {
        std::cerr << "[WeatherService] No API key configured" << std::endl;
        WeatherInfo info;
        info.city = city;
        info.valid = false;
        info.tcmLabel = "常温";
        return info;
    }

    std::string url = "https://api.seniverse.com/v3/weather/now.json?key="
                      + apiKey_ + "&location=" + city + "&language=zh-Hans";

    std::string response = httpGet(url);

    if (response.empty()) {
        std::cerr << "[WeatherService] API call failed for city: " << city << std::endl;

        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = cache_.find(city);
        if (it != cache_.end()) {
            return it->second;
        }

        WeatherInfo info;
        info.city = city;
        info.valid = false;
        info.tcmLabel = "常温";
        return info;
    }

    WeatherInfo info = parseWeatherResponse(response, city);

    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto now = std::chrono::steady_clock::now();
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        info.fetchTime = std::to_string(secs);
        cache_[city] = info;
    }

    std::cout << "[WeatherService] " << city << ": " << info.temperature
              << "C " << info.text << " -> " << info.tcmLabel << std::endl;

    return info;
}

std::string WeatherService::mapWeatherLabel(const std::string& temperature, const std::string& text) {
    int temp = 20;
    try { temp = std::stoi(temperature); }
    catch (...) { temp = 20; }

    if (temp <= 0) return "严寒";
    if (temp >= 35) return "酷暑";

    if (text.find("雪") != std::string::npos || text.find("冰雹") != std::string::npos) return "严寒";
    if (text.find("暴雨") != std::string::npos || text.find("大雨") != std::string::npos ||
        text.find("雷阵雨") != std::string::npos) return "潮湿";

    if (temp > 20) {
        if (text.find("雨") != std::string::npos || text.find("阴") != std::string::npos ||
            text.find("雾") != std::string::npos || text.find("霾") != std::string::npos) return "潮湿";
    }

    if (temp > 15 && temp < 30) {
        if (text.find("晴") != std::string::npos && temp > 25) return "干燥";
    }

    return "常温";
}

std::string WeatherService::callSeniverseApi(const std::string& location) {
    if (apiKey_.empty()) return "";
    std::string url = "https://api.seniverse.com/v3/weather/now.json?key="
                      + apiKey_ + "&location=" + location + "&language=zh-Hans";
    return httpGet(url);
}

WeatherInfo WeatherService::parseWeatherResponse(const std::string& response, const std::string& city) {
    WeatherInfo info;
    info.city = city;

    std::string resultsStr = json::getArrayStr(response, "results");
    if (resultsStr == "[]" || resultsStr.empty()) {
        info.valid = false;
        info.tcmLabel = "常温";
        return info;
    }

    info.text = json::getString(resultsStr, "text", "晴");
    info.temperature = json::getString(resultsStr, "temperature", "20");
    info.tcmLabel = mapWeatherLabel(info.temperature, info.text);
    info.valid = true;

    return info;
}

std::string WeatherService::httpGet(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("DailyDietServer/1.0",
                                         INTERNET_OPEN_TYPE_DIRECT,
                                         NULL, NULL, 0);
    if (!hInternet) return "";

    DWORD flags = INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE;
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, flags, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return "";
    }

    std::string result;
    char buffer[4096];
    DWORD bytesRead = 0;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return result;
}
