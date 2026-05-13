#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <string>
#include <map>
#include <mutex>
#include <chrono>

struct WeatherInfo {
    std::string city;
    std::string text;
    std::string temperature;
    std::string tcmLabel;
    std::string fetchTime;
    bool valid;
};

class WeatherService {
public:
    static WeatherService& getInstance() {
        static WeatherService instance;
        return instance;
    }

    void setApiKey(const std::string& key) { apiKey_ = key; }

    WeatherInfo getWeather(const std::string& city);
    std::string mapWeatherLabel(const std::string& temperature, const std::string& text);

private:
    WeatherService() = default;

    std::string callSeniverseApi(const std::string& location);
    WeatherInfo parseWeatherResponse(const std::string& response, const std::string& city);
    std::string httpGet(const std::string& url);

    std::string apiKey_;
    std::map<std::string, WeatherInfo> cache_;
    std::mutex cacheMutex_;
    static constexpr int CACHE_TTL_SECONDS = 3600;
};

#endif // WEATHER_SERVICE_H
