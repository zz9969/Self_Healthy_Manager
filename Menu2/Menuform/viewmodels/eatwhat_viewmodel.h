#ifndef EATWHAT_VIEWMODEL_H
#define EATWHAT_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "api_client.h"

class EatWhatViewModel : public QObject
{
    Q_OBJECT

public:
    explicit EatWhatViewModel(QObject* parent = nullptr) : QObject(parent) {}

public slots:
    void getRecommendation(const QString& mealType = "lunch") {
        QJsonObject body;
        body["meal_type"] = mealType;

        ApiClient::getInstance().post("/api/diet/plan/generate", body,
            [this](const QJsonObject& data) {
                emit recommendationReceived(data);
            },
            [this](int, const QString& errorMsg) {
                emit recommendationError(errorMsg.isEmpty() ? "网络请求失败，请检查服务器连接" : errorMsg);
            });
    }

    void replaceFood(const QJsonObject& currentFood, const QJsonObject& /*preferences*/) {
        QJsonObject body;
        body["food_name"] = currentFood.value("name").toString();
        body["category"] = currentFood.value("category").toString();

        ApiClient::getInstance().post("/api/diet/substitutes", body,
            [this](const QJsonObject& data) {
                emit replacementReceived(data);
            },
            [this](int, const QString& errorMsg) {
                emit recommendationError(errorMsg.isEmpty() ? "替换食物请求失败" : errorMsg);
            });
    }

signals:
    void recommendationReceived(const QJsonObject& recommendation);
    void recommendationError(const QString& errorMsg);
    void replacementReceived(const QJsonObject& replacement);
};

#endif // EATWHAT_VIEWMODEL_H
