#ifndef FOODLIST_VIEWMODEL_H
#define FOODLIST_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "api_client.h"

class FoodListViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QJsonArray foodList READ foodList NOTIFY foodListChanged)
    Q_PROPERTY(QString searchKeyword READ searchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged)

public:
    explicit FoodListViewModel(QObject* parent = nullptr) : QObject(parent) {}

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QJsonArray foodList() const { return food_list_; }
    QString searchKeyword() const { return search_keyword_; }

    void setSearchKeyword(const QString& keyword) {
        if (search_keyword_ != keyword) {
            search_keyword_ = keyword;
            emit searchKeywordChanged();
            filterFoodList();
        }
    }

public slots:
    void loadFoodList() {
        setLoading(true);
        setError("");

        ApiClient::getInstance().get("/api/diet/recipes/search",
            [this](const QJsonObject& data) {
                setLoading(false);
                setFoodList(data.value("foods").toArray());
                emit foodListLoaded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void searchFoods(const QString& keyword) {
        setSearchKeyword(keyword);
    }

signals:
    void loadingChanged();
    void errorChanged();
    void foodListChanged();
    void searchKeywordChanged();
    void foodListLoaded(const QJsonObject& data);

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

    void setFoodList(const QJsonArray& list) {
        if (food_list_ != list) {
            food_list_ = list;
            emit foodListChanged();
        }
    }

    void filterFoodList() {
        if (search_keyword_.isEmpty()) {
            loadFoodList();
            return;
        }

        QJsonArray filtered;
        for (const auto& item : food_list_) {
            QJsonObject food = item.toObject();
            QString name = food.value("name").toString();
            if (name.contains(search_keyword_, Qt::CaseInsensitive)) {
                filtered.append(item);
            }
        }
        food_list_ = filtered;
        emit foodListChanged();
    }

    bool loading_{false};
    QString error_msg_;
    QJsonArray food_list_;
    QString search_keyword_;
};

#endif // FOODLIST_VIEWMODEL_H
