#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <functional>

struct RequestCallbacks {
    std::function<void(const QJsonObject&)> onSuccess;
    std::function<void(int statusCode, const QString& errorMsg)> onError;
};

class ApiClient : public QObject
{
    Q_OBJECT

public:
    static ApiClient& getInstance() {
        static ApiClient instance;
        return instance;
    }

    void setBaseUrl(const QString& url) {
        baseUrl_ = url;
    }

    QString getBaseUrl() const {
        return baseUrl_;
    }

    void setAuthToken(const QString& token) {
        authToken_ = token;
        QSettings settings("DailyDiet", "Client");
        settings.setValue("auth_token", token);
    }

    QString getAuthToken() const {
        if (!authToken_.isEmpty()) {
            return authToken_;
        }
        QSettings settings("DailyDiet", "Client");
        return settings.value("auth_token", "").toString();
    }

    void clearAuth() {
        authToken_.clear();
        QSettings settings("DailyDiet", "Client");
        settings.remove("auth_token");
        emit unauthorized();
    }

    void get(const QString& endpoint,
             std::function<void(const QJsonObject&)> onSuccess,
             std::function<void(int, const QString&)> onError) {
        createRequest("GET", endpoint, QByteArray(), onSuccess, onError);
    }

    void post(const QString& endpoint, const QJsonObject& body,
              std::function<void(const QJsonObject&)> onSuccess,
              std::function<void(int, const QString&)> onError) {
        QJsonDocument doc(body);
        createRequest("POST", endpoint, doc.toJson(), onSuccess, onError);
    }

    void put(const QString& endpoint, const QJsonObject& body,
             std::function<void(const QJsonObject&)> onSuccess,
             std::function<void(int, const QString&)> onError) {
        QJsonDocument doc(body);
        createRequest("PUT", endpoint, doc.toJson(), onSuccess, onError);
    }

    void deleteRequest(const QString& endpoint,
                       std::function<void(const QJsonObject&)> onSuccess,
                       std::function<void(int, const QString&)> onError) {
        createRequest("DELETE", endpoint, QByteArray(), onSuccess, onError);
    }

signals:
    void unauthorized();

private:
    explicit ApiClient(QObject* parent = nullptr) : QObject(parent) {
        baseUrl_ = "http://localhost:8888";
        manager_ = new QNetworkAccessManager(this);
    }

    ~ApiClient() = default;
    ApiClient(const ApiClient&) = delete;
    ApiClient& operator=(const ApiClient&) = delete;

    void createRequest(const QString& method, const QString& endpoint,
                       const QByteArray& body,
                       std::function<void(const QJsonObject&)> onSuccess,
                       std::function<void(int, const QString&)> onError) {
        QUrl url(baseUrl_ + endpoint);
        QNetworkRequest request(url);

        qDebug() << "[ApiClient]" << method << url.toString()
                 << "token:" << (getAuthToken().isEmpty() ? "(none)" : getAuthToken().left(8) + "...");

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("X-Request-ID", QUuid::createUuid().toString().toUtf8());

        if (!getAuthToken().isEmpty()) {
            request.setRawHeader("Authorization",
                                ("Bearer " + getAuthToken()).toUtf8());
        }

        QNetworkReply* reply = nullptr;
        if (method == "GET") {
            reply = manager_->get(request);
        } else if (method == "POST") {
            reply = manager_->post(request, body);
        } else if (method == "PUT") {
            reply = manager_->put(request, body);
        } else if (method == "DELETE") {
            reply = manager_->deleteResource(request);
        }

        if (reply) {
            callbacks_[reply] = RequestCallbacks{onSuccess, onError};

            connect(reply, &QNetworkReply::finished, this, [this, reply]() {
                onReplyFinished(reply);
            });
        }
    }

    void onReplyFinished(QNetworkReply* reply) {
        auto it = callbacks_.find(reply);
        if (it == callbacks_.end()) {
            reply->deleteLater();
            return;
        }

        RequestCallbacks callbacks = it.value();
        callbacks_.remove(reply);

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();
        QString errorString = reply->errorString();

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
                QJsonParseError parseError;
                QJsonDocument errDoc = QJsonDocument::fromJson(responseData, &parseError);
                if (parseError.error == QJsonParseError::NoError) {
                    QJsonObject errObj = errDoc.object();
                    int code = errObj.value("code").toInt();
                    QString msg = errObj.value("msg").toString();
                    clearAuth();
                    if (callbacks.onError) {
                        callbacks.onError(code > 0 ? code : 9001, msg.isEmpty() ? "认证已过期，请重新登录" : msg);
                    }
                } else {
                    clearAuth();
                    if (callbacks.onError) {
                        callbacks.onError(9001, "认证已过期，请重新登录");
                    }
                }
            } else if (callbacks.onError) {
                callbacks.onError(statusCode, errorString);
            }
            reply->deleteLater();
            return;
        }

        if (statusCode == 401) {
            clearAuth();
            if (callbacks.onError) {
                callbacks.onError(401, "认证已过期，请重新登录");
            }
            reply->deleteLater();
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            if (callbacks.onError) {
                callbacks.onError(statusCode, "响应解析失败: " + parseError.errorString());
            }
            reply->deleteLater();
            return;
        }

        QJsonObject responseObj = doc.object();
        int code = responseObj.value("code").toInt(-1);
        QString msg = responseObj.value("msg").toString();
        QJsonObject data = responseObj.value("data").toObject();

        if (code == 0) {
            if (callbacks.onSuccess) {
                callbacks.onSuccess(data);
            }
        } else {
            if (code == 9001) {
                clearAuth();
            }
            if (callbacks.onError) {
                callbacks.onError(code, msg.isEmpty() ? QString("错误码: %1").arg(code) : msg);
            }
        }

        reply->deleteLater();
    }

    QString baseUrl_;
    QString authToken_;
    QNetworkAccessManager* manager_;
    QMap<QNetworkReply*, RequestCallbacks> callbacks_;
};

#endif // API_CLIENT_H
