#ifndef DB_H
#define DB_H

#include "server.h"
#include <string>
#include <optional>
#include <mutex>
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mysql.h>

struct UserInfo {
    int userId;
    std::string phone;
    std::string nickname;
    std::string passwordHash;
    std::string gender;
    int age;
    double height;
    double weight;
    int activityLevel;
};

class DbManager;

class AuthMiddleware {
public:
    static std::string extractToken(const HttpRequest& req) {
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end()) return "";
        std::string val = it->second;
        const std::string prefix = "Bearer ";
        if (val.size() > prefix.size() && val.substr(0, prefix.size()) == prefix) {
            return val.substr(prefix.size());
        }
        return val;
    }

    static int getUserId(const HttpRequest& req);
};

class DbManager {
public:
    static DbManager& getInstance() {
        static DbManager instance;
        return instance;
    }

    bool initialize() {
        return mysqlInit();
    }

    bool isConnected() { return connected_; }

    int createUser(const std::string& phone, const std::string& password,
                   double height, double weight, const std::string& gender, int age) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string hash = hashPassword(password);
        std::string query = "INSERT INTO users (username, password_hash, gender, age, height, weight, activity_level) "
                            "VALUES (?, ?, ?, ?, ?, ?, 1)";
        auto result = executeInsert(query, {phone, hash, gender, std::to_string(age),
                                            std::to_string(height), std::to_string(weight)});
        if (result <= 0) {
            auto existing = executeQuery("SELECT user_id FROM users WHERE username = ?", {phone});
            if (!existing.empty()) return -1;
        }

        int uid = result > 0 ? result : ++mockUserIdCounter_;
        UserInfo user;
        user.userId = uid;
        user.phone = phone;
        user.nickname = phone;
        user.passwordHash = hash;
        user.gender = gender;
        user.age = age;
        user.height = height;
        user.weight = weight;
        user.activityLevel = 1;
        mockUsers_[uid] = user;
        return uid;
    }

    std::optional<UserInfo> verifyLogin(const std::string& phone, const std::string& password) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT user_id, username, password_hash, gender, age, height, weight, activity_level "
                            "FROM users WHERE username = ?";
        auto rows = executeQuery(query, {phone});

        if (!rows.empty()) {
            auto& row = rows[0];
            if (!verifyPassword(password, row[2])) return std::nullopt;
            UserInfo user;
            user.userId = std::stoi(row[0]);
            user.phone = row[1];
            user.passwordHash = row[2];
            user.gender = row[3];
            user.age = std::stoi(row[4]);
            user.height = std::stod(row[5]);
            user.weight = std::stod(row[6]);
            user.activityLevel = std::stoi(row[7]);
            user.nickname = user.phone;
            return user;
        }

        for (auto& [uid, user] : mockUsers_) {
            if (user.phone == phone && verifyPassword(password, user.passwordHash)) {
                return user;
            }
        }
        return std::nullopt;
    }

    std::optional<UserInfo> getUserById(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT user_id, username, password_hash, gender, age, height, weight, activity_level "
                            "FROM users WHERE user_id = ?";
        auto rows = executeQuery(query, {std::to_string(userId)});

        if (!rows.empty()) {
            auto& row = rows[0];
            UserInfo user;
            user.userId = std::stoi(row[0]);
            user.phone = row[1];
            user.passwordHash = row[2];
            user.gender = row[3];
            user.age = std::stoi(row[4]);
            user.height = std::stod(row[5]);
            user.weight = std::stod(row[6]);
            user.activityLevel = std::stoi(row[7]);
            user.nickname = user.phone;
            return user;
        }

        auto it = mockUsers_.find(userId);
        if (it != mockUsers_.end()) return it->second;
        return std::nullopt;
    }

    bool updateUser(int userId, double height, double weight, int age, int activityLevel) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "UPDATE users SET height = ?, weight = ?, age = ?, activity_level = ? "
                            "WHERE user_id = ?";
        return executeUpdate(query, {std::to_string(height), std::to_string(weight),
                                     std::to_string(age), std::to_string(activityLevel),
                                     std::to_string(userId)}) > 0;
    }

    std::string createSession(int userId) {
        std::string sessionId = generateSessionId();
        std::lock_guard<std::mutex> lock(dbMutex_);
        auto now = std::chrono::system_clock::now();
        auto expire = now + std::chrono::hours(24);
        auto expireTime = std::chrono::system_clock::to_time_t(expire);
        std::string expireStr = timeToString(expireTime);

        std::string query = "INSERT INTO user_sessions (session_id, user_id, expire_time) "
                            "VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE expire_time = ?";
        executeUpdate(query, {sessionId, std::to_string(userId), expireStr, expireStr});

        mockSessions_[sessionId] = userId;
        return sessionId;
    }

    int verifySession(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT user_id FROM user_sessions WHERE session_id = ? AND expire_time > NOW()";
        auto rows = executeQuery(query, {sessionId});
        if (!rows.empty()) return std::stoi(rows[0][0]);

        auto it = mockSessions_.find(sessionId);
        if (it != mockSessions_.end()) return it->second;
        return -1;
    }

    void deleteSession(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        executeUpdate("DELETE FROM user_sessions WHERE session_id = ?", {sessionId});
        mockSessions_.erase(sessionId);
    }

    int cleanupExpiredSessions() {
        std::lock_guard<std::mutex> lock(dbMutex_);
        return executeUpdate("DELETE FROM user_sessions WHERE expire_time < NOW()", {});
    }

    std::string getQuestionnaire() {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string qQuery = "SELECT question_id, question_text, sort_order "
                             "FROM tcm_questions ORDER BY sort_order";
        auto qRows = executeQuery(qQuery, {});

        std::ostringstream ss;
        ss << "{\"questions\":[";
        for (size_t i = 0; i < qRows.size(); ++i) {
            if (i > 0) ss << ",";
            int qid = std::stoi(qRows[i][0]);
            ss << "{\"question_id\":" << qid
               << ",\"text\":\"" << json::escape(qRows[i][1]) << "\""
               << ",\"options\":[";

            std::string optQuery = "SELECT option_key, option_text, "
                                   "score_pinghe, score_qixu, score_yangxu, score_yinxu, "
                                   "score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing "
                                   "FROM tcm_question_options WHERE question_id = ? ORDER BY option_key";
            auto optRows = executeQuery(optQuery, {std::to_string(qid)});

            for (size_t j = 0; j < optRows.size(); ++j) {
                if (j > 0) ss << ",";
                ss << "{\"key\":\"" << json::escape(optRows[j][0]) << "\""
                   << ",\"text\":\"" << json::escape(optRows[j][1]) << "\""
                   << ",\"scores\":{"
                   << "\"平和\":" << optRows[j][2]
                   << ",\"气虚\":" << optRows[j][3]
                   << ",\"阳虚\":" << optRows[j][4]
                   << ",\"阴虚\":" << optRows[j][5]
                   << ",\"痰湿\":" << optRows[j][6]
                   << ",\"湿热\":" << optRows[j][7]
                   << ",\"血瘀\":" << optRows[j][8]
                   << ",\"气郁\":" << optRows[j][9]
                   << ",\"特禀\":" << optRows[j][10]
                   << "}}";
            }
            ss << "]}";
        }
        ss << "]}";
        return ss.str();
    }

    std::string analyzeConstitution(int userId, const std::map<int, std::string>& answers) {
        std::lock_guard<std::mutex> lock(dbMutex_);

        struct ConstitutionScores {
            double pinghe = 0, qixu = 0, yangxu = 0, yinxu = 0;
            double tanshi = 0, shire = 0, xueyu = 0, qiyu = 0, tebing = 0;
        };

        ConstitutionScores scores;

        for (auto& [qid, optionKey] : answers) {
            std::string optQuery = "SELECT score_pinghe, score_qixu, score_yangxu, score_yinxu, "
                                   "score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing "
                                   "FROM tcm_question_options WHERE question_id = ? AND option_key = ?";
            auto optRows = executeQuery(optQuery, {std::to_string(qid), optionKey});
            if (optRows.empty()) continue;

            auto& row = optRows[0];
            scores.pinghe += std::stod(row[0]);
            scores.qixu   += std::stod(row[1]);
            scores.yangxu += std::stod(row[2]);
            scores.yinxu  += std::stod(row[3]);
            scores.tanshi += std::stod(row[4]);
            scores.shire  += std::stod(row[5]);
            scores.xueyu  += std::stod(row[6]);
            scores.qiyu   += std::stod(row[7]);
            scores.tebing += std::stod(row[8]);
        }

        std::vector<std::pair<std::string, double>> sorted = {
            {"平和", scores.pinghe}, {"气虚", scores.qixu}, {"阳虚", scores.yangxu},
            {"阴虚", scores.yinxu},  {"痰湿", scores.tanshi}, {"湿热", scores.shire},
            {"血瘀", scores.xueyu},  {"气郁", scores.qiyu},   {"特禀", scores.tebing}
        };
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        std::string primaryType = sorted[0].first;
        if (primaryType == "平和") {
            for (size_t i = 1; i < sorted.size(); ++i) {
                if (sorted[i].second >= scores.pinghe * 0.6) {
                    primaryType = sorted[i].first;
                    break;
                }
            }
        }

        std::string secondaryType = "";
        for (size_t i = 0; i < sorted.size(); ++i) {
            if (sorted[i].first != primaryType && sorted[i].first != "平和"
                && sorted[i].second >= scores.pinghe * 0.4) {
                secondaryType = sorted[i].first;
                break;
            }
        }

        executeUpdate("DELETE FROM tcm_constitution_results WHERE user_id = ?",
                      {std::to_string(userId)});

        std::string insertQuery = "INSERT INTO tcm_constitution_results "
                                   "(user_id, score_pinghe, score_qixu, score_yangxu, score_yinxu, "
                                   "score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing, primary_type) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        executeInsert(insertQuery, {
            std::to_string(userId),
            std::to_string(scores.pinghe), std::to_string(scores.qixu),
            std::to_string(scores.yangxu), std::to_string(scores.yinxu),
            std::to_string(scores.tanshi), std::to_string(scores.shire),
            std::to_string(scores.xueyu), std::to_string(scores.qiyu),
            std::to_string(scores.tebing), primaryType
        });

        std::map<std::string, std::string> dietAdvice = {
            {"气虚", "宜食益气健脾食物：黄芪、山药、大枣、鸡肉。忌耗气食物"},
            {"阳虚", "宜食温阳食物：羊肉、生姜、桂圆、韭菜。忌生冷寒凉"},
            {"阴虚", "宜食滋阴润燥食物：银耳、百合、枸杞、鸭肉。忌辛辣温燥"},
            {"痰湿", "宜食健脾祛湿食物：薏米、冬瓜、陈皮、白萝卜。忌甜腻厚味"},
            {"湿热", "宜食清热祛湿食物：绿豆、苦瓜、薏米、莲藕。忌辛辣油腻"},
            {"血瘀", "宜食活血化瘀食物：山楂、黑木耳、玫瑰花、醋。忌寒凉凝滞"},
            {"气郁", "宜食疏肝理气食物：佛手、柑橘、萝卜、薄荷。忌气郁食物"},
            {"特禀", "宜食清淡均衡食物：粳米、蔬菜、猪肉。忌过敏原食物"},
            {"平和", "饮食有节，不偏食，五谷杂粮、蔬菜水果均衡摄入"}
        };

        std::ostringstream ss;
        ss << "{";
        ss << "\"primary_type\":\"" << json::escape(primaryType) << "\",";
        ss << "\"secondary_type\":\"" << json::escape(secondaryType) << "\",";
        ss << "\"scores\":{";
        ss << "\"平和\":" << std::fixed << std::setprecision(1) << scores.pinghe;
        ss << ",\"气虚\":" << scores.qixu;
        ss << ",\"阳虚\":" << scores.yangxu;
        ss << ",\"阴虚\":" << scores.yinxu;
        ss << ",\"痰湿\":" << scores.tanshi;
        ss << ",\"湿热\":" << scores.shire;
        ss << ",\"血瘀\":" << scores.xueyu;
        ss << ",\"气郁\":" << scores.qiyu;
        ss << ",\"特禀\":" << scores.tebing;
        ss << "}";
        auto it = dietAdvice.find(primaryType);
        if (it != dietAdvice.end()) {
            ss << ",\"diet_advice\":\"" << json::escape(it->second) << "\"";
        }
        ss << "}";
        return ss.str();
    }

    std::string getConstitutionResult(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT score_pinghe, score_qixu, score_yangxu, score_yinxu, "
                            "score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing, "
                            "primary_type, test_date "
                            "FROM tcm_constitution_results WHERE user_id = ? ORDER BY test_date DESC LIMIT 1";
        auto rows = executeQuery(query, {std::to_string(userId)});
        if (rows.empty()) return "{}";

        auto& row = rows[0];
        std::string primaryType = row[9];

        std::map<std::string, std::string> dietAdvice = {
            {"气虚", "宜食益气健脾食物：黄芪、山药、大枣、鸡肉。忌耗气食物"},
            {"阳虚", "宜食温阳食物：羊肉、生姜、桂圆、韭菜。忌生冷寒凉"},
            {"阴虚", "宜食滋阴润燥食物：银耳、百合、枸杞、鸭肉。忌辛辣温燥"},
            {"痰湿", "宜食健脾祛湿食物：薏米、冬瓜、陈皮、白萝卜。忌甜腻厚味"},
            {"湿热", "宜食清热祛湿食物：绿豆、苦瓜、薏米、莲藕。忌辛辣油腻"},
            {"血瘀", "宜食活血化瘀食物：山楂、黑木耳、玫瑰花、醋。忌寒凉凝滞"},
            {"气郁", "宜食疏肝理气食物：佛手、柑橘、萝卜、薄荷。忌气郁食物"},
            {"特禀", "宜食清淡均衡食物：粳米、蔬菜、猪肉。忌过敏原食物"},
            {"平和", "饮食有节，不偏食，五谷杂粮、蔬菜水果均衡摄入"}
        };

        std::ostringstream ss;
        ss << "{";
        ss << "\"primary_type\":\"" << json::escape(primaryType) << "\",";
        ss << "\"scores\":{";
        ss << "\"平和\":" << row[0] << ",\"气虚\":" << row[1];
        ss << ",\"阳虚\":" << row[2] << ",\"阴虚\":" << row[3];
        ss << ",\"痰湿\":" << row[4] << ",\"湿热\":" << row[5];
        ss << ",\"血瘀\":" << row[6] << ",\"气郁\":" << row[7];
        ss << ",\"特禀\":" << row[8] << "}";
        ss << ",\"test_date\":\"" << json::escape(row[10]) << "\"";
        auto it = dietAdvice.find(primaryType);
        if (it != dietAdvice.end()) {
            ss << ",\"diet_advice\":\"" << json::escape(it->second) << "\"";
        }
        ss << "}";
        return ss.str();
    }

    std::string getFoodList() {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT recipe_id, name, category, nature, flavor, efficacy, "
                            "calories, protein, fat, carbs FROM tcm_recipes ORDER BY recipe_id";
        auto rows = executeQuery(query, {});

        std::ostringstream ss;
        ss << "{\"foods\":[";
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{";
            ss << "\"id\":" << rows[i][0] << ",";
            ss << "\"name\":\"" << json::escape(rows[i][1]) << "\",";
            ss << "\"category\":\"" << json::escape(rows[i][2]) << "\",";
            ss << "\"nature\":\"" << json::escape(rows[i][3]) << "\",";
            ss << "\"flavor\":\"" << json::escape(rows[i][4]) << "\",";
            ss << "\"efficacy\":\"" << json::escape(rows[i][5]) << "\",";
            ss << "\"calories\":" << rows[i][6] << ",";
            ss << "\"protein\":" << rows[i][7] << ",";
            ss << "\"fat\":" << rows[i][8] << ",";
            ss << "\"carbs\":" << rows[i][9];
            ss << "}";
        }
        ss << "]}";
        return ss.str();
    }

    int saveDietRecord(int userId, const std::string& mealType, const std::string& body, const std::string& notes) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        int recipeId = json::getInt(body, "recipe_id", 0);
        double quantity = json::getDouble(body, "quantity", 1.0);
        double calories = json::getDouble(body, "calories", 0);
        std::string foodName = json::getString(body, "food_name");

        if (recipeId <= 0 && foodName.empty()) return -1;

        int inputMode = (recipeId > 0) ? 1 : 2;

        std::string query = "INSERT INTO diet_records (user_id, meal_type, recipe_id, quantity, calories, input_mode, record_date) "
                            "VALUES (?, ?, ?, ?, ?, ?, CURDATE())";
        int id = executeInsert(query, {std::to_string(userId), mealType,
                                       std::to_string(recipeId > 0 ? recipeId : 0), std::to_string(quantity),
                                       std::to_string(calories), std::to_string(inputMode)});
        executeUpdate("UPDATE daily_reports SET is_stale = 1 WHERE user_id = ? AND report_date = CURDATE()",
                      {std::to_string(userId)});
        return id;
    }

    std::string getTodayRecords(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT dr.record_id, dr.meal_type, dr.quantity, dr.calories, "
                            "dr.input_mode, tr.name, tr.protein, tr.fat, tr.carbs "
                            "FROM diet_records dr JOIN tcm_recipes tr ON dr.recipe_id = tr.recipe_id "
                            "WHERE dr.user_id = ? AND dr.record_date = CURDATE() ORDER BY dr.meal_type";
        auto rows = executeQuery(query, {std::to_string(userId)});

        std::ostringstream ss;
        ss << "{\"records\":[";
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{";
            ss << "\"id\":" << rows[i][0] << ",";
            ss << "\"meal_type\":" << rows[i][1] << ",";
            ss << "\"quantity\":" << rows[i][2] << ",";
            ss << "\"calories\":" << rows[i][3] << ",";
            ss << "\"input_mode\":" << rows[i][4] << ",";
            ss << "\"name\":\"" << json::escape(rows[i][5]) << "\",";
            ss << "\"protein\":" << rows[i][6] << ",";
            ss << "\"fat\":" << rows[i][7] << ",";
            ss << "\"carbs\":" << rows[i][8];
            ss << "}";
        }
        ss << "]}";
        return ss.str();
    }

    bool deleteDietRecord(int recordId, int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        int affected = executeUpdate("DELETE FROM diet_records WHERE record_id = ? AND user_id = ?",
                                     {std::to_string(recordId), std::to_string(userId)});
        if (affected > 0) {
            executeUpdate("UPDATE daily_reports SET is_stale = 1 WHERE user_id = ? AND report_date = CURDATE()",
                          {std::to_string(userId)});
        }
        return affected > 0;
    }

    std::string getTodayNutrition(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT COALESCE(SUM(dr.calories),0), COALESCE(SUM(tr.protein * dr.quantity),0), "
                            "COALESCE(SUM(tr.carbs * dr.quantity),0), COALESCE(SUM(tr.fat * dr.quantity),0), "
                            "COUNT(*) "
                            "FROM diet_records dr JOIN tcm_recipes tr ON dr.recipe_id = tr.recipe_id "
                            "WHERE dr.user_id = ? AND dr.record_date = CURDATE()";
        auto rows = executeQuery(query, {std::to_string(userId)});
        if (rows.empty()) return "{\"total_calories\":0,\"protein_g\":0,\"carbohydrates_g\":0,\"fat_g\":0,\"meals_count\":0}";

        auto& row = rows[0];
        std::ostringstream ss;
        ss << "{\"total_calories\":" << row[0];
        ss << ",\"protein_g\":" << row[1];
        ss << ",\"carbohydrates_g\":" << row[2];
        ss << ",\"fat_g\":" << row[3];
        ss << ",\"meals_count\":" << row[4] << "}";
        return ss.str();
    }

    std::string getNutritionRecommendation(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        auto userOpt = getUserByIdInternal(userId);
        if (!userOpt.has_value()) return "{\"target_calories\":2000,\"bmr\":1650,\"tdee\":2000}";

        auto& user = userOpt.value();
        double bmr;
        if (user.gender == "male" || user.gender == "1") {
            bmr = 10.0 * user.weight + 6.25 * user.height - 5.0 * user.age + 5.0;
        } else {
            bmr = 10.0 * user.weight + 6.25 * user.height - 5.0 * user.age - 161.0;
        }

        double activityFactors[] = {1.2, 1.375, 1.55, 1.725, 1.9};
        int level = user.activityLevel - 1;
        if (level < 0 || level > 4) level = 0;
        double tdee = bmr * activityFactors[level];

        std::ostringstream ss;
        ss << "{\"target_calories\":" << std::fixed << std::setprecision(0) << tdee;
        ss << ",\"bmr\":" << std::fixed << std::setprecision(0) << bmr;
        ss << ",\"tdee\":" << std::fixed << std::setprecision(0) << tdee;
        ss << ",\"target_protein_g\":" << std::fixed << std::setprecision(0) << (tdee * 0.15 / 4);
        ss << ",\"target_carbohydrates_g\":" << std::fixed << std::setprecision(0) << (tdee * 0.55 / 4);
        ss << ",\"target_fat_g\":" << std::fixed << std::setprecision(0) << (tdee * 0.30 / 9);
        ss << "}";
        return ss.str();
    }

    int addExerciseRecord(int userId, int typeId, int duration, int intensity, double calories) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "INSERT INTO exercise_records (user_id, type_id, duration, intensity, calories_burned, record_date) "
                            "VALUES (?, ?, ?, ?, ?, CURDATE())";
        int id = executeInsert(query, {std::to_string(userId), std::to_string(typeId),
                                       std::to_string(duration), std::to_string(intensity),
                                       std::to_string(calories)});
        executeUpdate("UPDATE daily_reports SET is_stale = 1 WHERE user_id = ? AND report_date = CURDATE()",
                      {std::to_string(userId)});
        return id;
    }

    std::string getExerciseTypes() {
        std::lock_guard<std::mutex> lock(dbMutex_);
        auto rows = executeQuery("SELECT type_id, name, met_value, category FROM exercise_types ORDER BY type_id", {});
        std::ostringstream ss;
        ss << "{\"types\":[";
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{\"id\":" << rows[i][0] << ",";
            ss << "\"name\":\"" << json::escape(rows[i][1]) << "\",";
            ss << "\"met\":" << rows[i][2] << ",";
            ss << "\"category\":\"" << json::escape(rows[i][3]) << "\"}";
        }
        ss << "]}";
        return ss.str();
    }

    std::string getTodayExercise(int userId) {
        std::lock_guard<std::mutex> lock(dbMutex_);
        std::string query = "SELECT er.record_id, et.name, er.duration, er.intensity, er.calories_burned "
                            "FROM exercise_records er JOIN exercise_types et ON er.type_id = et.type_id "
                            "WHERE er.user_id = ? AND er.record_date = CURDATE()";
        auto rows = executeQuery(query, {std::to_string(userId)});
        double totalCal = 0;
        std::ostringstream ss;
        ss << "{\"exercises\":[";
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i > 0) ss << ",";
            double cal = std::stod(rows[i][4]);
            totalCal += cal;
            ss << "{\"id\":" << rows[i][0] << ",";
            ss << "\"type\":\"" << json::escape(rows[i][1]) << "\",";
            ss << "\"duration\":" << rows[i][2] << ",";
            ss << "\"intensity\":" << rows[i][3] << ",";
            ss << "\"calories\":" << rows[i][4] << "}";
        }
        ss << "],\"total_calories\":" << std::fixed << std::setprecision(0) << totalCal << "}";
        return ss.str();
    }

    std::string getCityList() {
        std::lock_guard<std::mutex> lock(dbMutex_);
        auto rows = executeQuery("SELECT city_name, province, region_label, weather_city_id FROM city_region_mapping ORDER BY city_name", {});
        std::ostringstream ss;
        ss << "{\"cities\":[";
        for (size_t i = 0; i < rows.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{\"name\":\"" << json::escape(rows[i][0]) << "\",";
            ss << "\"province\":\"" << json::escape(rows[i][1]) << "\",";
            ss << "\"region\":\"" << json::escape(rows[i][2]) << "\",";
            ss << "\"weather_id\":\"" << json::escape(rows[i][3]) << "\"}";
        }
        ss << "]}";
        return ss.str();
    }

private:
    DbManager() : mysql_(nullptr) {}
    std::mutex dbMutex_;
    bool connected_{false};
    MYSQL* mysql_{nullptr};

    std::map<std::string, int> mockSessions_;
    std::map<int, UserInfo> mockUsers_;
    int mockUserIdCounter_{0};

    bool mysqlInit() {
        mysql_ = mysql_init(nullptr);
        if (!mysql_) {
            std::cerr << "[DB] mysql_init failed" << std::endl;
            return false;
        }

        mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        std::string host = "localhost";
        std::string user = "root";
        std::string password = "zzmy";
        std::string database = "health_management";
        int port = 3306;

        std::ifstream cfgFile("server_config.json");
        if (cfgFile.is_open()) {
            std::string content((std::istreambuf_iterator<char>(cfgFile)),
                                 std::istreambuf_iterator<char>());
            host = json::getString(content, "db_host", host);
            user = json::getString(content, "db_user", user);
            password = json::getString(content, "db_password", password);
            database = json::getString(content, "db_name", database);
            port = json::getInt(content, "db_port", port);
            cfgFile.close();
        }

        if (!mysql_real_connect(mysql_, host.c_str(), user.c_str(),
                                 password.c_str(), database.c_str(), port, nullptr, 0)) {
            std::cerr << "[DB] mysql_real_connect failed: " << mysql_error(mysql_) << std::endl;
            mysql_close(mysql_);
            mysql_ = nullptr;
            return false;
        }

        connected_ = true;
        std::cout << "[DB] Connected to MySQL " << host << ":" << port << "/" << database << std::endl;

        initConstitutionTables();
        return true;
    }

    void initConstitutionTables() {
        auto check = executeQuery("SELECT COUNT(*) FROM information_schema.tables "
                                  "WHERE table_schema = DATABASE() AND table_name = 'tcm_questions'", {});
        if (!check.empty() && std::stoi(check[0][0]) > 0) {
            std::cout << "[DB] Constitution tables already exist" << std::endl;
            return;
        }

        std::cout << "[DB] Creating constitution tables..." << std::endl;

        executeUpdate("CREATE TABLE IF NOT EXISTS tcm_questions ("
                      "question_id INT PRIMARY KEY AUTO_INCREMENT,"
                      "question_text VARCHAR(200) NOT NULL,"
                      "sort_order INT NOT NULL"
                      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", {});

        executeUpdate("CREATE TABLE IF NOT EXISTS tcm_question_options ("
                      "id INT PRIMARY KEY AUTO_INCREMENT,"
                      "question_id INT NOT NULL,"
                      "option_key CHAR(1) NOT NULL,"
                      "option_text VARCHAR(20) NOT NULL,"
                      "score_pinghe DECIMAL(5,2) DEFAULT 0,"
                      "score_qixu DECIMAL(5,2) DEFAULT 0,"
                      "score_yangxu DECIMAL(5,2) DEFAULT 0,"
                      "score_yinxu DECIMAL(5,2) DEFAULT 0,"
                      "score_tanshi DECIMAL(5,2) DEFAULT 0,"
                      "score_shire DECIMAL(5,2) DEFAULT 0,"
                      "score_xueyu DECIMAL(5,2) DEFAULT 0,"
                      "score_qiyu DECIMAL(5,2) DEFAULT 0,"
                      "score_tebing DECIMAL(5,2) DEFAULT 0,"
                      "FOREIGN KEY (question_id) REFERENCES tcm_questions(question_id) ON DELETE CASCADE,"
                      "UNIQUE KEY uk_qid_opt (question_id, option_key)"
                      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", {});

        executeUpdate("CREATE TABLE IF NOT EXISTS tcm_constitution_results ("
                      "result_id INT PRIMARY KEY AUTO_INCREMENT,"
                      "user_id INT NOT NULL,"
                      "score_pinghe DECIMAL(5,2) DEFAULT 0,"
                      "score_qixu DECIMAL(5,2) DEFAULT 0,"
                      "score_yangxu DECIMAL(5,2) DEFAULT 0,"
                      "score_yinxu DECIMAL(5,2) DEFAULT 0,"
                      "score_tanshi DECIMAL(5,2) DEFAULT 0,"
                      "score_shire DECIMAL(5,2) DEFAULT 0,"
                      "score_xueyu DECIMAL(5,2) DEFAULT 0,"
                      "score_qiyu DECIMAL(5,2) DEFAULT 0,"
                      "score_tebing DECIMAL(5,2) DEFAULT 0,"
                      "primary_type VARCHAR(20) NOT NULL,"
                      "test_date DATETIME DEFAULT CURRENT_TIMESTAMP,"
                      "FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE"
                      ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", {});

        executeUpdate("INSERT IGNORE INTO tcm_questions (question_id, question_text, sort_order) VALUES "
                      "(1,'您容易感到疲乏吗？',1),"
                      "(2,'您手脚发凉吗？',2),"
                      "(3,'您感觉口干咽燥吗？',3),"
                      "(4,'您感到胸闷或腹部胀满吗？',4),"
                      "(5,'您面部或鼻部有油腻感或油亮发光吗？',5),"
                      "(6,'您皮肤常在不知不觉中出现青紫瘀斑吗？',6),"
                      "(7,'您感到闷闷不乐、情绪低沉吗？',7),"
                      "(8,'您容易过敏（对药物、食物、气味、花粉等）吗？',8),"
                      "(9,'您精力充沛吗？',9),"
                      "(10,'您适应外界自然和社会环境变化吗？',10)", {});

        struct OptData { int qid; char key; const char* text; double ph; double qx; double yx; double ynx; double ts; double sr; double xy; double qy; double tb; };
        OptData opts[] = {
            {1,'A',"从不",5,1,0,0,0,0,0,0,0}, {1,'B',"偶尔",4,2,0,0,0,0,0,0,0}, {1,'C',"有时",3,3,0,0,0,0,0,0,0}, {1,'D',"经常",2,4,0,0,0,0,0,0,0}, {1,'E',"总是",1,5,0,0,0,0,0,0,0},
            {2,'A',"从不",5,0,1,0,0,0,0,0,0}, {2,'B',"偶尔",4,0,2,0,0,0,0,0,0}, {2,'C',"有时",3,0,3,0,0,0,0,0,0}, {2,'D',"经常",2,0,4,0,0,0,0,0,0}, {2,'E',"总是",1,0,5,0,0,0,0,0,0},
            {3,'A',"从不",5,0,0,1,0,0,0,0,0}, {3,'B',"偶尔",4,0,0,2,0,0,0,0,0}, {3,'C',"有时",3,0,0,3,0,0,0,0,0}, {3,'D',"经常",2,0,0,4,0,0,0,0,0}, {3,'E',"总是",1,0,0,5,0,0,0,0,0},
            {4,'A',"从不",5,0,0,0,1,0,0,0,0}, {4,'B',"偶尔",4,0,0,0,2,0,0,0,0}, {4,'C',"有时",3,0,0,0,3,0,0,0,0}, {4,'D',"经常",2,0,0,0,4,0,0,0,0}, {4,'E',"总是",1,0,0,0,5,0,0,0,0},
            {5,'A',"从不",5,0,0,0,0,1,0,0,0}, {5,'B',"偶尔",4,0,0,0,0,2,0,0,0}, {5,'C',"有时",3,0,0,0,0,3,0,0,0}, {5,'D',"经常",2,0,0,0,0,4,0,0,0}, {5,'E',"总是",1,0,0,0,0,5,0,0,0},
            {6,'A',"从不",5,0,0,0,0,0,1,0,0}, {6,'B',"偶尔",4,0,0,0,0,0,2,0,0}, {6,'C',"有时",3,0,0,0,0,0,3,0,0}, {6,'D',"经常",2,0,0,0,0,0,4,0,0}, {6,'E',"总是",1,0,0,0,0,0,5,0,0},
            {7,'A',"从不",5,0,0,0,0,0,0,1,0}, {7,'B',"偶尔",4,0,0,0,0,0,0,2,0}, {7,'C',"有时",3,0,0,0,0,0,0,3,0}, {7,'D',"经常",2,0,0,0,0,0,0,4,0}, {7,'E',"总是",1,0,0,0,0,0,0,5,0},
            {8,'A',"从不",5,0,0,0,0,0,0,0,1}, {8,'B',"偶尔",4,0,0,0,0,0,0,0,2}, {8,'C',"有时",3,0,0,0,0,0,0,0,3}, {8,'D',"经常",2,0,0,0,0,0,0,0,4}, {8,'E',"总是",1,0,0,0,0,0,0,0,5},
            {9,'A',"总是",5,0,0,0,0,0,0,0,0}, {9,'B',"经常",4,0,0,0,0,0,0,0,0}, {9,'C',"有时",3,0,0,0,0,0,0,0,0}, {9,'D',"偶尔",2,0,0,0,0,0,0,0,0}, {9,'E',"从不",1,0,0,0,0,0,0,0,0},
            {10,'A',"总是",5,0,0,0,0,0,0,0,0}, {10,'B',"经常",4,0,0,0,0,0,0,0,0}, {10,'C',"有时",3,0,0,0,0,0,0,0,0}, {10,'D',"偶尔",2,0,0,0,0,0,0,0,0}, {10,'E',"从不",1,0,0,0,0,0,0,0,0}
        };

        for (auto& o : opts) {
            std::string q = "INSERT IGNORE INTO tcm_question_options "
                            "(question_id, option_key, option_text, "
                            "score_pinghe, score_qixu, score_yangxu, score_yinxu, "
                            "score_tanshi, score_shire, score_xueyu, score_qiyu, score_tebing) VALUES ("
                            + std::to_string(o.qid) + ",'" + std::string(1, o.key) + "','" + o.text + "',"
                            + std::to_string(o.ph) + "," + std::to_string(o.qx) + ","
                            + std::to_string(o.yx) + "," + std::to_string(o.ynx) + ","
                            + std::to_string(o.ts) + "," + std::to_string(o.sr) + ","
                            + std::to_string(o.xy) + "," + std::to_string(o.qy) + ","
                            + std::to_string(o.tb) + ")";
            executeUpdate(q, {});
        }

        std::cout << "[DB] Constitution tables created and data inserted" << std::endl;
    }

    std::optional<UserInfo> getUserByIdInternal(int userId) {
        std::string query = "SELECT user_id, username, password_hash, gender, age, height, weight, activity_level "
                            "FROM users WHERE user_id = ?";
        auto rows = executeQuery(query, {std::to_string(userId)});
        if (rows.empty()) return std::nullopt;

        auto& row = rows[0];
        UserInfo user;
        user.userId = std::stoi(row[0]);
        user.phone = row[1];
        user.passwordHash = row[2];
        user.gender = row[3];
        user.age = std::stoi(row[4]);
        user.height = std::stod(row[5]);
        user.weight = std::stod(row[6]);
        user.activityLevel = std::stoi(row[7]);
        user.nickname = user.phone;
        return user;
    }

    std::vector<std::vector<std::string>> executeQuery(const std::string& query,
                                                        const std::vector<std::string>& params) {
        if (!mysql_) {
            return {};
        }

        std::string escapedQuery;
        size_t paramIdx = 0;
        for (size_t i = 0; i < query.size(); ++i) {
            if (query[i] == '?' && paramIdx < params.size()) {
                std::string escaped(params[paramIdx].size() * 2 + 1, '\0');
                unsigned long len = mysql_real_escape_string(mysql_, &escaped[0],
                                                              params[paramIdx].c_str(),
                                                              static_cast<unsigned long>(params[paramIdx].size()));
                escapedQuery += "'";
                escapedQuery.append(escaped.c_str(), len);
                escapedQuery += "'";
                paramIdx++;
            } else {
                escapedQuery += query[i];
            }
        }

        if (mysql_query(mysql_, escapedQuery.c_str()) != 0) {
            std::cerr << "[DB] Query error: " << mysql_error(mysql_) << std::endl;
            return {};
        }

        MYSQL_RES* result = mysql_store_result(mysql_);
        if (!result) return {};

        int numFields = mysql_num_fields(result);
        MYSQL_ROW row;
        std::vector<std::vector<std::string>> rows;
        while ((row = mysql_fetch_row(result))) {
            std::vector<std::string> rowData;
            unsigned long* lengths = mysql_fetch_lengths(result);
            for (int i = 0; i < numFields; ++i) {
                if (row[i]) {
                    rowData.emplace_back(row[i], lengths[i]);
                } else {
                    rowData.emplace_back("");
                }
            }
            rows.push_back(std::move(rowData));
        }
        mysql_free_result(result);
        return rows;
    }

    int executeInsert(const std::string& query, const std::vector<std::string>& params) {
        if (!mysql_) {
            static int counter = 100;
            return ++counter;
        }

        std::string escapedQuery;
        size_t paramIdx = 0;
        for (size_t i = 0; i < query.size(); ++i) {
            if (query[i] == '?' && paramIdx < params.size()) {
                std::string escaped(params[paramIdx].size() * 2 + 1, '\0');
                unsigned long len = mysql_real_escape_string(mysql_, &escaped[0],
                                                              params[paramIdx].c_str(),
                                                              static_cast<unsigned long>(params[paramIdx].size()));
                escapedQuery += "'";
                escapedQuery.append(escaped.c_str(), len);
                escapedQuery += "'";
                paramIdx++;
            } else {
                escapedQuery += query[i];
            }
        }

        if (mysql_query(mysql_, escapedQuery.c_str()) != 0) {
            std::cerr << "[DB] Insert error: " << mysql_error(mysql_) << std::endl;
            return -1;
        }
        return static_cast<int>(mysql_insert_id(mysql_));
    }

    int executeUpdate(const std::string& query, const std::vector<std::string>& params) {
        if (!mysql_) return 1;

        std::string escapedQuery;
        size_t paramIdx = 0;
        for (size_t i = 0; i < query.size(); ++i) {
            if (query[i] == '?' && paramIdx < params.size()) {
                std::string escaped(params[paramIdx].size() * 2 + 1, '\0');
                unsigned long len = mysql_real_escape_string(mysql_, &escaped[0],
                                                              params[paramIdx].c_str(),
                                                              static_cast<unsigned long>(params[paramIdx].size()));
                escapedQuery += "'";
                escapedQuery.append(escaped.c_str(), len);
                escapedQuery += "'";
                paramIdx++;
            } else {
                escapedQuery += query[i];
            }
        }

        if (mysql_query(mysql_, escapedQuery.c_str()) != 0) {
            std::cerr << "[DB] Update error: " << mysql_error(mysql_) << std::endl;
            return 0;
        }
        return static_cast<int>(mysql_affected_rows(mysql_));
    }

    std::string hashPassword(const std::string& password) {
        std::string salt = "health_mgmt_";
        unsigned int hash = 5381;
        std::string combined = salt + password;
        for (char c : combined) {
            hash = ((hash << 5) + hash) + c;
        }
        std::ostringstream ss;
        ss << std::hex << hash;
        return salt + ss.str();
    }

    bool verifyPassword(const std::string& password, const std::string& storedHash) {
        return hashPassword(password) == storedHash;
    }

    std::string generateSessionId() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dist;
        std::ostringstream ss;
        ss << std::hex << dist(gen) << dist(gen) << dist(gen) << dist(gen);
        return ss.str();
    }

    std::string timeToString(time_t t) {
        struct tm tm_info;
        #ifdef _WIN32
            localtime_s(&tm_info, &t);
        #else
            localtime_r(&t, &tm_info);
        #endif
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
        return buf;
    }
};

inline int AuthMiddleware::getUserId(const HttpRequest& req) {
    std::string token = extractToken(req);
    if (token.empty()) return -1;
    return DbManager::getInstance().verifySession(token);
}

#endif // DB_H
