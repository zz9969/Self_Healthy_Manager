#ifndef EATWHAT_H
#define EATWHAT_H

#include <QWidget>
#include <QListWidget>
#include "viewmodels/eatwhat_viewmodel.h"
#include "foodlist.h"

namespace Ui {
class eatwhat;
}

class eatwhat : public QWidget
{
    Q_OBJECT

public:
    explicit eatwhat(QWidget *parent = nullptr);
    ~eatwhat();

signals:
    void backToMain();

private slots:
    void onGenerateClicked();
    void onRecommendationReceived(const QJsonObject& data);
    void onRecommendationError(const QString& errorMsg);
    void onReplacementReceived(const QJsonObject& data);
    void onReplaceFoodClicked(const QJsonObject& foodItem);

private:
    void loadRecommendation();
    void buildFoodList(const QJsonObject& data);

    Ui::eatwhat *ui;
    EatWhatViewModel* viewmodel_;
    QJsonObject currentRecommendation_;
    bool loaded_;
};

#endif // EATWHAT_H
