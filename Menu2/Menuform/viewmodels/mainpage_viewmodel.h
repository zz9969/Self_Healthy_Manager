#ifndef MAINPAGE_VIEWMODEL_H
#define MAINPAGE_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "api_client.h"

class MainPageViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QJsonArray todayRecommendations READ todayRecommendations NOTIFY recommendationsChanged)

public:
    explicit MainPageViewModel(QObject* parent = nullptr) : QObject(parent) {}

    QString userName() const { return user_name_; }
    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QJsonArray todayRecommendations() const { return recommendations_; }

public slots:
    void loadUserInfo() {
        setLoading(true);

        ApiClient::getInstance().get("/api/user/profile",
            [this](const QJsonObject& data) {
                setLoading(false);
                setUserName(data.value("nickname").toString("用户"));
                emit userInfoLoaded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void getTodayRecommendation() {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["meal_type"] = "lunch";

        ApiClient::getInstance().post("/api/diet/plan/generate", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                QJsonArray menuArr;
                QStringList meals = {"breakfast", "lunch", "dinner"};
                for (const auto& key : meals) {
                    if (data.contains(key)) {
                        QJsonObject meal = data[key].toObject();
                        meal["meal_type"] = key;
                        menuArr.append(meal);
                    }
                }
                setRecommendations(menuArr);
                emit recommendationReceived(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void navigateToEatWhat() {
        emit navigateToPage("eatwhat");
    }

    void navigateToFoodList() {
        emit navigateToPage("foodlist");
    }

    void navigateToToday() {
        emit navigateToPage("today");
    }

    void navigateToRealEat() {
        emit navigateToPage("realeat");
    }

signals:
    void userNameChanged();
    void loadingChanged();
    void errorChanged();
    void recommendationsChanged();
    void userInfoLoaded(const QJsonObject& userInfo);
    void recommendationReceived(const QJsonObject& recommendation);
    void navigateToPage(const QString& pageName);

private:
    void setLoading(bool loading) {
        if (loading_ != loading) {
            loading_ = loading;
            emit loadingChanged();
        }
    }

    void setError(const QString& error) {
        if (error_msg_ != error) {
            error_msg_ = error;
            emit errorChanged();
        }
    }

    void setUserName(const QString& name) {
        if (user_name_ != name) {
            user_name_ = name;
            emit userNameChanged();
        }
    }

    void setRecommendations(const QJsonArray& recs) {
        if (recommendations_ != recs) {
            recommendations_ = recs;
            emit recommendationsChanged();
        }
    }

    QString user_name_;
    bool loading_{false};
    QString error_msg_;
    QJsonArray recommendations_;
};

#endif // MAINPAGE_VIEWMODEL_H
