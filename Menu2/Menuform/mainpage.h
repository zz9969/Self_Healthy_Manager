#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QShowEvent>
#include "viewmodels/mainpage_viewmodel.h"

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

private slots:
    void onEatWhatClicked();
    void onTodayClicked();
    void onRealEatClicked();
    void onConstitutionClicked();
    void onUserInfoLoaded(const QJsonObject& data);
    void onRecommendationReceived(const QJsonObject& data);

private:
    void showEvent(QShowEvent* event) override;
    Ui::MainPage *ui;
    MainPageViewModel* viewmodel_;
    QListWidget* recommendList_;
};

#endif // MAINPAGE_H
