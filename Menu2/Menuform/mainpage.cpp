#include "mainpage.h"
#include "ui_mainpage.h"
#include <QMessageBox>

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPage),
    viewmodel_(new MainPageViewModel(this)),
    constitutionChecked_(false)
{
    ui->setupUi(this);
    setWindowTitle("每日饮食推荐");

    connect(ui->pushButton, &QPushButton::clicked, this, &MainPage::onEatWhatClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainPage::onRealEatClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainPage::onRunPageClicked);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainPage::onConstitutionClicked);
    connect(viewmodel_, &MainPageViewModel::userInfoLoaded, this, &MainPage::onUserInfoLoaded);
    connect(viewmodel_, &MainPageViewModel::recommendationReceived, this, &MainPage::onRecommendationReceived);
}

MainPage::~MainPage()
{
    delete ui;
}

void MainPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (ApiClient::getInstance().getAuthToken().isEmpty()) return;

    viewmodel_->loadUserInfo();
    viewmodel_->getTodayRecommendation();

    if (!constitutionChecked_) {
        constitutionChecked_ = true;
        checkConstitution();
    }
}

void MainPage::checkConstitution()
{
    ApiClient::getInstance().get("/api/constitution/result",
        [this](const QJsonObject& data) {
            if (data.value("primary_type").toString().isEmpty()) {
                int ret = QMessageBox::information(this, "体质检测",
                    "您还未进行中医体质评估，建议先完成体质测试以获得个性化饮食推荐。\n是否现在进行测试？",
                    QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    emit navigateToConstitution();
                }
            } else {
                QString previousLogin = ApiClient::getInstance().getPreviousLogin();
                if (!previousLogin.isEmpty() && isMoreThanOneMonth(previousLogin)) {
                    int ret = QMessageBox::information(this, "体质检测",
                        "距您上次登录已超过一个月，建议重新进行体质测试以获得更准确的饮食推荐。\n是否现在重新测试？",
                        QMessageBox::Yes | QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        emit navigateToConstitution();
                    }
                }
            }
        },
        [](int, const QString&) {});
}

bool MainPage::isMoreThanOneMonth(const QString& dateTimeStr)
{
    QDateTime loginTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd HH:mm:ss");
    if (!loginTime.isValid()) {
        loginTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd");
    }
    if (!loginTime.isValid()) return false;

    qint64 daysDiff = loginTime.daysTo(QDateTime::currentDateTime());
    return daysDiff > 30;
}

void MainPage::onEatWhatClicked()
{
    emit navigateToEatWhat();
}

void MainPage::onRealEatClicked()
{
    emit navigateToRealEat();
}

void MainPage::onConstitutionClicked()
{
    emit navigateToConstitution();
}

void MainPage::onRunPageClicked()
{
    emit navigateToRunPage();
}

void MainPage::onUserInfoLoaded(const QJsonObject& data)
{
    QString nickname = data.value("nickname").toString("用户");
    ui->label->setText("欢迎您，" + nickname);
}

void MainPage::onRecommendationReceived(const QJsonObject& data)
{
    ui->recommendList->clear();

    QStringList meals = {"breakfast", "lunch", "dinner"};
    QStringList mealNames = {"早餐", "午餐", "晚餐"};

    for (int i = 0; i < meals.size(); ++i) {
        if (!data.contains(meals[i])) continue;
        QJsonObject meal = data[meals[i]].toObject();
        QString mealName = meal.value("name").toString(mealNames[i]);
        int calories = (int)meal.value("calories").toDouble();

        QJsonArray foods = meal.value("foods").toArray();
        QStringList foodNames;
        for (const auto& food : foods) {
            foodNames.append(food.toObject().value("name").toString());
        }

        QString summary = "【" + mealNames[i] + "】" + mealName
                         + " (" + QString::number(calories) + "kcal)"
                         + "\n  " + foodNames.join("、");

        QListWidgetItem* item = new QListWidgetItem(summary);
        item->setSizeHint(QSize(0, 48));
        ui->recommendList->addItem(item);
    }

    if (data.contains("total_calories")) {
        int totalCal = (int)data.value("total_calories").toDouble();
        QListWidgetItem* totalItem = new QListWidgetItem(
            "总热量: " + QString::number(totalCal) + " kcal");
        QFont boldFont;
        boldFont.setBold(true);
        totalItem->setFont(boldFont);
        totalItem->setForeground(QColor("#2196F3"));
        totalItem->setSizeHint(QSize(0, 30));
        ui->recommendList->addItem(totalItem);
    }

    if (data.contains("nutrition_tips")) {
        QListWidgetItem* tipsItem = new QListWidgetItem(
            "营养建议: " + data.value("nutrition_tips").toString());
        tipsItem->setForeground(QColor("#888"));
        tipsItem->setSizeHint(QSize(0, 30));
        ui->recommendList->addItem(tipsItem);
    }
}
