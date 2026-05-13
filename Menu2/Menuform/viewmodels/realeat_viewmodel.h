#ifndef REALEAT_VIEWMODEL_H
#define REALEAT_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "api_client.h"

class RealEatViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QJsonArray todayRecords READ todayRecords NOTIFY recordsChanged)
    Q_PROPERTY(double totalCalories READ totalCalories NOTIFY statsChanged)
    Q_PROPERTY(double totalProtein READ totalProtein NOTIFY statsChanged)
    Q_PROPERTY(double totalCarbs READ totalCarbs NOTIFY statsChanged)
    Q_PROPERTY(double totalFat READ totalFat NOTIFY statsChanged)

public:
    explicit RealEatViewModel(QObject* parent = nullptr) : QObject(parent) {}

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QJsonArray todayRecords() const { return today_records_; }
    double totalCalories() const { return total_calories_; }
    double totalProtein() const { return total_protein_; }
    double totalCarbs() const { return total_carbs_; }
    double totalFat() const { return total_fat_; }

public slots:
    void loadTodayRecords() {
        setLoading(true);
        setError("");

        ApiClient::getInstance().get("/api/record/diet",
            [this](const QJsonObject& data) {
                setLoading(false);
                setTodayRecords(data.value("records").toArray());
                calculateStats();
                emit recordsLoaded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void recordMeal(const QString& mealType, const QJsonArray& foods,
                    const QString& notes = "") {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["meal_type"] = mealType;
        body["foods"] = foods;
        body["notes"] = notes;

        ApiClient::getInstance().post("/api/record/diet", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                loadTodayRecords();
                emit mealRecorded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void deleteRecord(int recordId) {
        setLoading(true);
        setError("");

        QString endpoint = "/api/record/diet/" + QString::number(recordId);

        ApiClient::getInstance().deleteRequest(endpoint,
            [this](const QJsonObject& data) {
                setLoading(false);
                loadTodayRecords();
                emit recordDeleted(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

signals:
    void loadingChanged();
    void errorChanged();
    void recordsChanged();
    void statsChanged();
    void recordsLoaded(const QJsonObject& data);
    void mealRecorded(const QJsonObject& record);
    void recordDeleted(const QJsonObject& result);

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

    void setTodayRecords(const QJsonArray& records) {
        if (today_records_ != records) {
            today_records_ = records;
            emit recordsChanged();
        }
    }

    void calculateStats() {
        double calories = 0, protein = 0, carbs = 0, fat = 0;

        for (const auto& record : today_records_) {
            QJsonObject obj = record.toObject();
            calories += obj.value("calories").toDouble();
            protein += obj.value("protein").toDouble();
            carbs += obj.value("carbs").toDouble();
            fat += obj.value("fat").toDouble();
        }

        total_calories_ = calories;
        total_protein_ = protein;
        total_carbs_ = carbs;
        total_fat_ = fat;

        emit statsChanged();
    }

    bool loading_{false};
    QString error_msg_;
    QJsonArray today_records_;
    double total_calories_{0};
    double total_protein_{0};
    double total_carbs_{0};
    double total_fat_{0};
};

#endif // REALEAT_VIEWMODEL_H
