#ifndef TODAY_VIEWMODEL_H
#define TODAY_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "api_client.h"

class TodayViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QJsonObject nutritionStats READ nutritionStats NOTIFY statsChanged)
    Q_PROPERTY(QJsonObject recommendation READ recommendation NOTIFY recommendationChanged)
    Q_PROPERTY(double progressPercent READ progressPercent NOTIFY progressChanged)

public:
    explicit TodayViewModel(QObject* parent = nullptr) : QObject(parent) {}

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QJsonObject nutritionStats() const { return nutrition_stats_; }
    QJsonObject recommendation() const { return recommendation_; }
    double progressPercent() const { return progress_percent_; }

public slots:
    void loadDailyReport() {
        setLoading(true);
        setError("");

        ApiClient::getInstance().get("/api/report/daily",
            [this](const QJsonObject& data) {
                setLoading(false);

                QJsonObject intake = data.value("intake").toObject();
                QJsonObject target = data.value("target").toObject();

                setNutritionStats(intake);
                setRecommendation(target);
                calculateProgress(intake, target);
                emit statsLoaded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

    void refreshAll() {
        loadDailyReport();
    }

signals:
    void loadingChanged();
    void errorChanged();
    void statsChanged();
    void recommendationChanged();
    void progressChanged();
    void statsLoaded(const QJsonObject& stats);
    void recommendationLoaded(const QJsonObject& recommendation);

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

    void setNutritionStats(const QJsonObject& stats) {
        if (nutrition_stats_ != stats) {
            nutrition_stats_ = stats;
            emit statsChanged();
        }
    }

    void setRecommendation(const QJsonObject& rec) {
        if (recommendation_ != rec) {
            recommendation_ = rec;
            emit recommendationChanged();
        }
    }

    void calculateProgress(const QJsonObject& intake, const QJsonObject& target) {
        double targetCal = target.value("target_calories").toDouble(2000);
        double consumed = intake.value("total_calories").toDouble(0);

        if (targetCal > 0) {
            progress_percent_ = (consumed / targetCal) * 100.0;
            if (progress_percent_ > 100.0) {
                progress_percent_ = 100.0;
            }
        } else {
            progress_percent_ = 0;
        }

        emit progressChanged();
    }

    bool loading_{false};
    QString error_msg_;
    QJsonObject nutrition_stats_;
    QJsonObject recommendation_;
    double progress_percent_{0};
};

#endif // TODAY_VIEWMODEL_H
