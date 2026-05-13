#ifndef SERVER_H
#define SERVER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> pathParams;
};

struct HttpResponse {
    int statusCode = 200;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct Route {
    std::string method;
    std::string pathPattern;
    std::function<HttpResponse(const HttpRequest&)> handler;
};

namespace json {

inline std::string escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;       break;
        }
    }
    return out;
}

inline std::string jsonResponse(int code, const std::string& msg, const std::string& data) {
    std::ostringstream ss;
    ss << "{\"code\":" << code << ","
       << "\"msg\":\"" << escape(msg) << "\","
       << "\"data\":" << data << "}";
    return ss.str();
}

inline std::string buildData(std::initializer_list<std::pair<const char*, const char*>> items) {
    std::ostringstream ss;
    ss << "{";
    bool first = true;
    for (auto& [k, v] : items) {
        if (!first) ss << ",";
        first = false;
        ss << "\"" << k << "\":\"" << escape(v) << "\"";
    }
    ss << "}";
    return ss.str();
}

inline std::string buildDataNum(std::initializer_list<std::pair<const char*, std::string>> items) {
    std::ostringstream ss;
    ss << "{";
    bool first = true;
    for (auto& [k, v] : items) {
        if (!first) ss << ",";
        first = false;
        ss << "\"" << k << "\":" << v;
    }
    ss << "}";
    return ss.str();
}

inline std::string jsonArray(std::initializer_list<std::string> items) {
    std::ostringstream ss;
    ss << "[";
    bool first = true;
    for (auto& item : items) {
        if (!first) ss << ",";
        first = false;
        ss << item;
    }
    ss << "]";
    return ss.str();
}

inline size_t findKey(const std::string& body, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = body.find(search);
    if (pos == std::string::npos) return std::string::npos;
    size_t colonPos = body.find(':', pos + search.size());
    if (colonPos == std::string::npos) return std::string::npos;
    size_t valStart = colonPos + 1;
    while (valStart < body.size() && (body[valStart] == ' ' || body[valStart] == '\t'))
        valStart++;
    return valStart;
}

inline std::string getString(const std::string& body, const std::string& key, const std::string& defaultVal = "") {
    size_t pos = findKey(body, key);
    if (pos == std::string::npos) return defaultVal;
    if (body[pos] != '"') return defaultVal;
    size_t start = pos + 1;
    size_t end = start;
    while (end < body.size()) {
        if (body[end] == '\\' && end + 1 < body.size()) { end += 2; continue; }
        if (body[end] == '"') break;
        end++;
    }
    std::string raw = body.substr(start, end - start);
    std::string result;
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            char c = raw[++i];
            if (c == 'n') result += '\n';
            else if (c == 'r') result += '\r';
            else if (c == 't') result += '\t';
            else if (c == '"') result += '"';
            else if (c == '\\') result += '\\';
            else { result += '\\'; result += c; }
        } else {
            result += raw[i];
        }
    }
    return result;
}

inline double getDouble(const std::string& body, const std::string& key, double defaultVal = 0.0) {
    size_t pos = findKey(body, key);
    if (pos == std::string::npos) return defaultVal;
    try { return std::stod(body.substr(pos)); }
    catch (...) { return defaultVal; }
}

inline int getInt(const std::string& body, const std::string& key, int defaultVal = 0) {
    size_t pos = findKey(body, key);
    if (pos == std::string::npos) return defaultVal;
    try { return std::stoi(body.substr(pos)); }
    catch (...) { return defaultVal; }
}

inline bool getBool(const std::string& body, const std::string& key, bool defaultVal = false) {
    size_t pos = findKey(body, key);
    if (pos == std::string::npos) return defaultVal;
    std::string sub = body.substr(pos, 5);
    if (sub == "true") return true;
    if (sub == "fals") return false;
    return defaultVal;
}

inline std::string getArrayStr(const std::string& body, const std::string& key) {
    size_t pos = findKey(body, key);
    if (pos == std::string::npos) return "[]";
    if (body[pos] != '[') return "[]";
    int depth = 1;
    size_t start = pos;
    size_t end = pos + 1;
    while (end < body.size() && depth > 0) {
        if (body[end] == '[') depth++;
        else if (body[end] == ']') depth--;
        end++;
    }
    return body.substr(start, end - start);
}

} // namespace json

inline std::map<std::string, std::string> extractPathParams(
    const std::string& actualPath,
    const std::string& pattern)
{
    std::map<std::string, std::string> params;

    auto split = [](const std::string& s) -> std::vector<std::string> {
        std::vector<std::string> parts;
        std::string current;
        for (char c : s) {
            if (c == '/') { if (!current.empty()) { parts.push_back(current); current.clear(); } }
            else current += c;
        }
        if (!current.empty()) parts.push_back(current);
        return parts;
    };

    auto actualParts = split(actualPath);
    auto patternParts = split(pattern);

    if (actualParts.size() != patternParts.size()) return params;

    for (size_t i = 0; i < patternParts.size(); ++i) {
        if (patternParts[i].size() > 1 && patternParts[i][0] == ':') {
            std::string paramName = patternParts[i].substr(1);
            params[paramName] = actualParts[i];
        } else if (patternParts[i] != actualParts[i]) {
            return {};
        }
    }

    return params;
}

#endif // SERVER_H
