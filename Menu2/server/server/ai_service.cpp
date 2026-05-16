#include "ai_service.h"
#include "nutrition_calc.h"
#include "server.h"
#include <wininet.h>
#include <sstream>
#include <iostream>
#include <cstring>

#pragma comment(lib, "wininet.lib")

NutritionResult AiNutritionService::analyzeDish(const std::string& dishName, const std::string& preference) {
    NutritionResult result;
    result.calories = 0;
    result.protein_g = 0;
    result.fat_g = 0;
    result.carbs_g = 0;
    result.fiber_g = 0;
    result.fromAI = false;

    if (!hasApiKey()) {
        std::cout << "[AiService] No API key configured, using default estimation" << std::endl;
        return NutritionCalculator::defaultEstimation(dishName);
    }

    std::string prompt = "你是一个中国家常菜营养分析专家。用户输入一道菜名";
    if (!preference.empty()) {
        prompt += "和口味偏好（" + preference + "）";
    }
    prompt += "，你需要拆解这道菜的标准食材及预估克重。\n\n";
    prompt += "严格按以下JSON格式返回，不要输出任何其他文字、解释或markdown标记：\n";
    prompt += "[{\"name\":\"食材名\",\"weight_g\":克重}]\n\n";
    prompt += "示例输入：宫保鸡丁 少油\n";
    prompt += "示例输出：[{\"name\":\"鸡胸肉\",\"weight_g\":150},{\"name\":\"花生米\",\"weight_g\":30},";
    prompt += "{\"name\":\"干辣椒\",\"weight_g\":5},{\"name\":\"大葱\",\"weight_g\":20},";
    prompt += "{\"name\":\"生姜\",\"weight_g\":10},{\"name\":\"蒜\",\"weight_g\":5},";
    prompt += "{\"name\":\"酱油\",\"weight_g\":15},{\"name\":\"醋\",\"weight_g\":10},";
    prompt += "{\"name\":\"白糖\",\"weight_g\":8},{\"name\":\"淀粉\",\"weight_g\":5}]\n\n";
    prompt += "现在请分析：";
    prompt += dishName;
    if (!preference.empty()) {
        prompt += " " + preference;
    }

    std::string aiResponse = callZhipuAPI(prompt);
    if (aiResponse.empty()) {
        std::cout << "[AiService] AI call failed for: " << dishName << std::endl;
        return NutritionCalculator::defaultEstimation(dishName);
    }

    result.ingredients = parseIngredients(aiResponse);
    if (result.ingredients.empty()) {
        std::cout << "[AiService] No ingredients parsed for: " << dishName << std::endl;
        return NutritionCalculator::defaultEstimation(dishName);
    }

    NutritionResult calcResult = NutritionCalculator::calculate(result.ingredients);
    calcResult.ingredients = result.ingredients;
    calcResult.fromAI = true;

    std::cout << "[AiService] Analyzed '" << dishName << "': "
              << calcResult.ingredients.size() << " ingredients, "
              << calcResult.calories << " kcal" << std::endl;

    return calcResult;
}

std::string AiNutritionService::callZhipuAPI(const std::string& prompt) {
    try {
    std::string requestBody = "{\"model\":\"glm-5.1\",\"messages\":["
        "{\"role\":\"user\",\"content\":\"" + json::escape(prompt) + "\"}"
        "],\"temperature\":0.1,\"max_tokens\":1024}";

    HINTERNET hInternet = InternetOpenA("DailyDietServer/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hConnect = InternetConnectA(hInternet, "open.bigmodel.cn", 443, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/api/paas/v4/chat/completions",
        NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    std::string authHeader = "Authorization: Bearer " + apiKey_;
    std::string contentType = "Content-Type: application/json";
    HttpAddRequestHeadersA(hRequest, authHeader.c_str(), (DWORD)authHeader.size(), HTTP_ADDREQ_FLAG_ADD);
    HttpAddRequestHeadersA(hRequest, contentType.c_str(), (DWORD)contentType.size(), HTTP_ADDREQ_FLAG_ADD);

    std::string contentLength = "Content-Length: " + std::to_string(requestBody.size());
    HttpAddRequestHeadersA(hRequest, contentLength.c_str(), (DWORD)contentLength.size(), HTTP_ADDREQ_FLAG_ADD);

    BOOL sent = HttpSendRequestA(hRequest, NULL, 0, (LPVOID)requestBody.c_str(), (DWORD)requestBody.size());
    if (!sent) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    std::string response;
    char buffer[4096];
    DWORD bytesRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    size_t contentPos = response.find("{\"id\":");
    if (contentPos == std::string::npos) {
        contentPos = response.find("{\"choices\":");
    }
    if (contentPos == std::string::npos) {
        contentPos = response.find("{");
    }
    if (contentPos != std::string::npos && contentPos > 0) {
        response = response.substr(contentPos);
    }

    size_t msgStart = response.find("\"content\":\"");
    if (msgStart == std::string::npos) return "";

    msgStart += 11;
    size_t msgEnd = response.find("\"", msgStart);
    if (msgEnd == std::string::npos) return "";

    std::string content = response.substr(msgStart, msgEnd - msgStart);

    std::string unescaped;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\\' && i + 1 < content.size()) {
            char next = content[i + 1];
            if (next == 'n') { unescaped += '\n'; ++i; }
            else if (next == 't') { unescaped += '\t'; ++i; }
            else if (next == '"') { unescaped += '"'; ++i; }
            else if (next == '\\') { unescaped += '\\'; ++i; }
            else { unescaped += content[i]; }
        } else {
            unescaped += content[i];
        }
    }

    size_t jsonStart = unescaped.find('[');
    size_t jsonEnd = unescaped.rfind(']');
    if (jsonStart == std::string::npos || jsonEnd == std::string::npos || jsonEnd <= jsonStart) {
        return "";
    }

    return unescaped.substr(jsonStart, jsonEnd - jsonStart + 1);
    } catch (const std::exception& e) {
        std::cerr << "[AiService] Exception in API call: " << e.what() << std::endl;
        return "";
    } catch (...) {
        std::cerr << "[AiService] Unknown exception in API call" << std::endl;
        return "";
    }
}

std::vector<IngredientInfo> AiNutritionService::parseIngredients(const std::string& jsonStr) {
    std::vector<IngredientInfo> ingredients;

    size_t pos = 0;
    while ((pos = jsonStr.find("\"name\"", pos)) != std::string::npos) {
        IngredientInfo info;

        size_t nameStart = jsonStr.find("\"", pos + 6);
        if (nameStart == std::string::npos) break;
        nameStart++;
        size_t nameEnd = jsonStr.find("\"", nameStart);
        if (nameEnd == std::string::npos) break;
        info.name = jsonStr.substr(nameStart, nameEnd - nameStart);

        size_t weightPos = jsonStr.find("\"weight_g\"", nameEnd);
        if (weightPos == std::string::npos) {
            weightPos = jsonStr.find("weight_g", nameEnd);
        }
        if (weightPos != std::string::npos) {
            size_t colonPos = jsonStr.find(":", weightPos);
            if (colonPos != std::string::npos) {
                size_t valStart = colonPos + 1;
                while (valStart < jsonStr.size() && (jsonStr[valStart] == ' ' || jsonStr[valStart] == '\t'))
                    valStart++;
                size_t valEnd = valStart;
                while (valEnd < jsonStr.size() && (jsonStr[valEnd] >= '0' && jsonStr[valEnd] <= '9' || jsonStr[valEnd] == '.'))
                    valEnd++;
                std::string valStr = jsonStr.substr(valStart, valEnd - valStart);
                try { info.weight_g = std::stod(valStr); }
                catch (...) { info.weight_g = 100; }
            }
        } else {
            info.weight_g = 100;
        }

        if (!info.name.empty()) {
            ingredients.push_back(info);
        }
        pos = nameEnd + 1;
    }

    return ingredients;
}
