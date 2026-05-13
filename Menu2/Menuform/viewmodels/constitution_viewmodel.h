#ifndef CONSTITUTION_VIEWMODEL_H
#define CONSTITUTION_VIEWMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "api_client.h"

class ConstitutionViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QJsonArray questions READ questions NOTIFY questionsChanged)
    Q_PROPERTY(QJsonObject result READ result NOTIFY resultChanged)

public:
    explicit ConstitutionViewModel(QObject* parent = nullptr) : QObject(parent) {}

    bool isLoading() const { return loading_; }
    QString errorMessage() const { return error_msg_; }
    QJsonArray questions() const { return questions_; }
    QJsonObject result() const { return result_; }

public slots:
    void loadQuestions() {
        setLoading(true);
        setError("");

        ApiClient::getInstance().get("/api/constitution/questions",
            [this](const QJsonObject& data) {
                setLoading(false);
                QJsonArray qArr = data.value("questions").toArray();
                qDebug() << "[ConstitutionVM] questions received, count:" << qArr.size();
                if (qArr.size() > 0) {
                    qDebug() << "[ConstitutionVM] first question keys:" << qArr[0].toObject().keys();
                    qDebug() << "[ConstitutionVM] first question text:" << qArr[0].toObject().value("text").toString();
                }
                setQuestions(qArr);
                emit questionsLoaded(data);
            },
            [this](int statusCode, const QString& errorMsg) {
                setLoading(false);
                qDebug() << "[ConstitutionVM] questions error:" << statusCode << errorMsg;
                setError(errorMsg);
            });
    }

    void submitAnswers(const QJsonArray& answers) {
        setLoading(true);
        setError("");

        QJsonObject body;
        body["answers"] = answers;

        qDebug() << "[ConstitutionVM] submitAnswers:" << answers;

        ApiClient::getInstance().post("/api/constitution/analyze", body,
            [this](const QJsonObject& data) {
                setLoading(false);
                qDebug() << "[ConstitutionVM] analyze result:" << data;
                setResult(data);
                emit analysisCompleted(data);
            },
            [this](int statusCode, const QString& errorMsg) {
                setLoading(false);
                qDebug() << "[ConstitutionVM] analyze error:" << statusCode << errorMsg;
                setError(errorMsg);
            });
    }

    void loadResult() {
        setLoading(true);
        setError("");

        ApiClient::getInstance().get("/api/constitution/result",
            [this](const QJsonObject& data) {
                setLoading(false);
                setResult(data);
                emit resultLoaded(data);
            },
            [this](int statusCode, const QString& errorMsg) {
                setLoading(false);
                setError(errorMsg);
            });
    }

signals:
    void loadingChanged();
    void errorChanged();
    void questionsChanged();
    void resultChanged();
    void questionsLoaded(const QJsonObject& data);
    void analysisCompleted(const QJsonObject& result);
    void resultLoaded(const QJsonObject& result);

private:
    void setLoading(bool loading) {
        if (loading_ != loading) { loading_ = loading; emit loadingChanged(); }
    }
    void setError(const QString& error) {
        if (error_msg_ != error) { error_msg_ = error; emit errorChanged(); }
    }
    void setQuestions(const QJsonArray& questions) {
        if (questions_ != questions) { questions_ = questions; emit questionsChanged(); }
    }
    void setResult(const QJsonObject& result) {
        if (result_ != result) { result_ = result; emit resultChanged(); }
    }

    bool loading_{false};
    QString error_msg_;
    QJsonArray questions_;
    QJsonObject result_;
};

#endif // CONSTITUTION_VIEWMODEL_H
