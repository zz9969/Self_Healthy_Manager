#ifndef LOGIN_VIEWMODEL_H
#define LOGIN_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include "api_client.h"

class LoginViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString phone READ phone WRITE setPhone NOTIFY phoneChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)

public:
    explicit LoginViewModel(QObject* parent = nullptr) : QObject(parent) {}

    QString phone() const { return phone_; }
    void setPhone(const QString& phone) {
        if (phone_ != phone) {
            phone_ = phone;
            emit phoneChanged();
        }
    }

    QString password() const { return password_; }
    void setPassword(const QString& pwd) {
        if (password_ != pwd) {
            password_ = pwd;
            emit passwordChanged();
        }
    }

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }

public slots:
    void login() {
        if (phone_.isEmpty() || password_.isEmpty()) {
            setError("请输入手机号和密码");
            return;
        }

        setLoading(true);
        setError("");

        QJsonObject body;
        body["phone"] = phone_;
        body["password"] = password_;

        ApiClient::getInstance().post("/api/user/login", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                emit loginSuccess(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void registerUser() {
        if (phone_.isEmpty() || password_.isEmpty()) {
            setError("请输入手机号和密码");
            return;
        }

        setLoading(true);
        setError("");

        QJsonObject body;
        body["phone"] = phone_;
        body["password"] = password_;
        body["height"] = 170;
        body["weight"] = 65;
        body["gender"] = "male";
        body["age"] = 25;

        ApiClient::getInstance().post("/api/user/register", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                emit registerSuccess(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void enterTouristMode() {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["device_info"] = "Qt Client";

        ApiClient::getInstance().post("/api/auth/tourist", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                emit touristSuccess(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

signals:
    void phoneChanged();
    void passwordChanged();
    void loadingChanged();
    void errorChanged();
    void loginSuccess(const QJsonObject& userInfo);
    void registerSuccess(const QJsonObject& userInfo);
    void touristSuccess(const QJsonObject& info);

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

    QString phone_;
    QString password_;
    bool loading_{false};
    QString error_msg_;
};

#endif // LOGIN_VIEWMODEL_H
