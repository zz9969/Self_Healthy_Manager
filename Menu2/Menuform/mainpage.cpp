#include "mainpage.h"
#include "ui_mainpage.h"
#include "foodlist.h"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPage),
    viewmodel_(new MainPageViewModel(this)),
    recommendList_(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("每日饮食推荐");

    QLabel* recTitle = new QLabel("今日饮食推荐", this);
    recTitle->setGeometry(10, 290, 200, 24);
    recTitle->setStyleSheet("font-size:14px; font-weight:bold; color:#333;");

    recommendList_ = new QListWidget(this);
    recommendList_->setGeometry(10, 316, 431, 340);
    recommendList_->setStyleSheet(
        "QListWidget { border:1px solid #E0E0E0; border-radius:6px; background:#FAFAFA; }"
        "QListWidget::item { padding:6px; border-bottom:1px solid #EEE; }"
        "QListWidget::item:selected { background:#E3F2FD; }"
    );

    QPushButton* constitutionBtn = new QPushButton("体质评估", this);
    constitutionBtn->setGeometry(10, 280, 131, 31);
    constitutionBtn->setStyleSheet("QPushButton { background-color:#FF9800; color:white; border-radius:4px; font-weight:bold; }");
    connect(constitutionBtn, &QPushButton::clicked, this, &MainPage::onConstitutionClicked);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainPage::onEatWhatClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainPage::onRealEatClicked);
    connect(viewmodel_, &MainPageViewModel::userInfoLoaded, this, &MainPage::onUserInfoLoaded);
    connect(viewmodel_, &MainPageViewModel::recommendationReceived, this, &MainPage::onRecommendationReceived);

    viewmodel_->getTodayRecommendation();
}

MainPage::~MainPage()
{
    delete ui;
}

void MainPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    viewmodel_->loadUserInfo();
    viewmodel_->getTodayRecommendation();
}

void MainPage::onEatWhatClicked()
{
    emit navigateToEatWhat();
}

void MainPage::onTodayClicked()
{
    emit navigateToToday();
}

void MainPage::onRealEatClicked()
{
    emit navigateToRealEat();
}

void MainPage::onConstitutionClicked()
{
    emit navigateToConstitution();
}

void MainPage::onUserInfoLoaded(const QJsonObject& data)
{
    QString nickname = data.value("nickname").toString("用户");
    int userId = data.value("user_id").toInt(0);
    ui->userid->setText(QString::number(userId));
    ui->label->setText("欢迎您，" + nickname);
}

void MainPage::onRecommendationReceived(const QJsonObject& data)
{
    recommendList_->clear();

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
        recommendList_->addItem(item);
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
        recommendList_->addItem(totalItem);
    }

    if (data.contains("nutrition_tips")) {
        QListWidgetItem* tipsItem = new QListWidgetItem(
            "营养建议: " + data.value("nutrition_tips").toString());
        tipsItem->setForeground(QColor("#888"));
        tipsItem->setSizeHint(QSize(0, 30));
        recommendList_->addItem(tipsItem);
    }
}
