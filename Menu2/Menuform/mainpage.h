#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QShowEvent>
#include <QDateTime>
#include "viewmodels/mainpage_viewmodel.h"
#include "api_client.h"

namespace Ui {
class MainPage;
}

class MainPage : public QWidget
{
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage();

signals:
    void navigateToEatWhat();
    void navigateToToday();
    void navigateToRealEat();
    void navigateToConstitution();
    void navigateToRunPage();

private slots:
    void onEatWhatClicked();
    void onRealEatClicked();
    void onConstitutionClicked();
    void onRunPageClicked();
    void onUserInfoLoaded(const QJsonObject& data);
    void onRecommendationReceived(const QJsonObject& data);

private:
    void showEvent(QShowEvent* event) override;
    void checkConstitution();
    bool isMoreThanOneMonth(const QString& dateTimeStr);
    Ui::MainPage *ui;
    MainPageViewModel* viewmodel_;
    bool constitutionChecked_;
};

#endif // MAINPAGE_H
