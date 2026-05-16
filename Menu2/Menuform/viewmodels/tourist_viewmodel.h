#ifndef TOURIST_VIEWMODEL_H
#define TOURIST_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "api_client.h"

class TouristViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QString welcomeMessage READ welcomeMessage NOTIFY welcomeMessageChanged)

public:
    explicit TouristViewModel(QObject* parent = nullptr) : QObject(parent) {}

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QString welcomeMessage() const { return welcome_message_; }

public slots:
    void startTouristSession() {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["device_info"] = "Qt Client - Tourist Mode";

        ApiClient::getInstance().post("/api/auth/tourist", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                setWelcomeMessage("欢迎使用游客模式！您可以体验基本功能。");
                emit touristSessionStarted(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void convertToRegisteredUser(const QString& phone, const QString& password,
                                  double height, double weight,
                                  const QString& gender, int age) {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["phone"] = phone;
        body["password"] = password;
        body["height"] = height;
        body["weight"] = weight;
        body["gender"] = gender;
        body["age"] = age;

        ApiClient::getInstance().post("/api/user/register", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                emit userConverted(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

signals:
    void loadingChanged();
    void errorChanged();
    void welcomeMessageChanged();
    void touristSessionStarted(const QJsonObject& sessionInfo);
    void userConverted(const QJsonObject& userInfo);

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

    void setWelcomeMessage(const QString& msg) {
        if (welcome_message_ != msg) {
            welcome_message_ = msg;
            emit welcomeMessageChanged();
        }
    }

    bool loading_{false};
    QString error_msg_;
    QString welcome_message_;
};

#endif // TOURIST_VIEWMODEL_H
