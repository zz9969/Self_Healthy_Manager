#include "eatwhat.h"
#include "ui_eatwhat.h"

eatwhat::eatwhat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::eatwhat),
    viewmodel_(new EatWhatViewModel(this)),
    loaded_(false)
{
    ui->setupUi(this);
    setWindowTitle("这顿该吃什么 - AI推荐");

    connect(ui->pushButton, &QPushButton::clicked, this, &eatwhat::onGenerateClicked);
    connect(viewmodel_, &EatWhatViewModel::recommendationReceived, this, &eatwhat::onRecommendationReceived);
    connect(viewmodel_, &EatWhatViewModel::recommendationError, this, &eatwhat::onRecommendationError);
    connect(viewmodel_, &EatWhatViewModel::replacementReceived, this, &eatwhat::onReplacementReceived);

    QPushButton* backBtn = new QPushButton("返回主页", this);
    backBtn->setGeometry(320, 480, 100, 31);
    connect(backBtn, &QPushButton::clicked, this, [this]() { emit backToMain(); });

    loadRecommendation();
}

eatwhat::~eatwhat()
{
    delete ui;
}

void eatwhat::loadRecommendation()
{
    ui->listWidget->clear();
    ui->listWidget->addItem("正在生成推荐菜单...");
    ui->pushButton->setEnabled(false);
    viewmodel_->getRecommendation();
}

void eatwhat::onGenerateClicked()
{
    loaded_ = false;
    loadRecommendation();
}

void eatwhat::onRecommendationReceived(const QJsonObject& data)
{
    loaded_ = true;
    currentRecommendation_ = data;
    buildFoodList(data);
}

void eatwhat::onRecommendationError(const QString& errorMsg)
{
    Q_UNUSED(errorMsg);
    ui->listWidget->clear();
    ui->pushButton->setEnabled(true);

    QJsonObject fallback;
    QJsonArray breakfastFoods, lunchFoods, dinnerFoods;

    QJsonObject b1; b1["name"] = "小米粥"; b1["amount"] = "200g"; breakfastFoods.append(b1);
    QJsonObject b2; b2["name"] = "鸡蛋"; b2["amount"] = "1个"; breakfastFoods.append(b2);
    QJsonObject b3; b3["name"] = "馒头"; b3["amount"] = "1个"; breakfastFoods.append(b3);

    QJsonObject l1; l1["name"] = "清蒸鲈鱼"; l1["amount"] = "150g"; lunchFoods.append(l1);
    QJsonObject l2; l2["name"] = "蒜蓉西兰花"; l2["amount"] = "200g"; lunchFoods.append(l2);
    QJsonObject l3; l3["name"] = "糙米饭"; l3["amount"] = "1碗"; lunchFoods.append(l3);

    QJsonObject d1; d1["name"] = "番茄牛腩"; d1["amount"] = "180g"; dinnerFoods.append(d1);
    QJsonObject d2; d2["name"] = "凉拌黄瓜"; d2["amount"] = "150g"; dinnerFoods.append(d2);
    QJsonObject d3; d3["name"] = "杂粮饭"; d3["amount"] = "1碗"; dinnerFoods.append(d3);

    QJsonObject breakfast; breakfast["name"] = "营养早餐"; breakfast["foods"] = breakfastFoods; breakfast["calories"] = 450;
    QJsonObject lunch; lunch["name"] = "健康午餐"; lunch["foods"] = lunchFoods; lunch["calories"] = 530;
    QJsonObject dinner; dinner["name"] = "清淡晚餐"; dinner["foods"] = dinnerFoods; dinner["calories"] = 420;

    fallback["breakfast"] = breakfast;
    fallback["lunch"] = lunch;
    fallback["dinner"] = dinner;
    fallback["total_calories"] = 1400;
    fallback["nutrition_tips"] = "本地推荐（AI服务暂不可用）";
    fallback["is_fallback"] = true;

    buildFoodList(fallback);
}

void eatwhat::buildFoodList(const QJsonObject& data)
{
    ui->listWidget->clear();
    ui->pushButton->setEnabled(true);

    QStringList meals = {"breakfast", "lunch", "dinner"};
    QStringList mealNames = {"早餐", "午餐", "晚餐"};

    for (int i = 0; i < meals.size(); ++i) {
        if (!data.contains(meals[i])) continue;
        QJsonObject meal = data[meals[i]].toObject();
        QString mealName = meal.value("name").toString(mealNames[i]);
        int calories = (int)meal.value("calories").toDouble();

        QListWidgetItem* headerItem = new QListWidgetItem(
            "【" + mealNames[i] + "】" + mealName + "  " + QString::number(calories) + " kcal");
        QFont headerFont;
        headerFont.setBold(true);
        headerFont.setPointSize(11);
        headerItem->setFont(headerFont);
        headerItem->setForeground(QColor("#4CAF50"));
        headerItem->setSizeHint(QSize(0, 30));
        ui->listWidget->addItem(headerItem);

        QJsonArray foods = meal.value("foods").toArray();
        for (int j = 0; j < foods.size(); ++j) {
            QJsonObject foodObj = foods[j].toObject();
            QString name = foodObj.value("name").toString();
            QString amount = foodObj.value("amount").toString();

            FoodList* foodWidget = new FoodList(nullptr, false);
            foodWidget->setFoodData(name, amount);

            QJsonObject foodCopy = foodObj;
            connect(foodWidget, &FoodList::replaceRequested,
                    this, [this, foodCopy](const QString&) {
                onReplaceFoodClicked(foodCopy);
            });

            QListWidgetItem* foodItem = new QListWidgetItem();
            foodItem->setSizeHint(QSize(0, 68));
            foodItem->setData(Qt::UserRole, foodObj);
            ui->listWidget->addItem(foodItem);
            ui->listWidget->setItemWidget(foodItem, foodWidget);
        }
    }

    if (data.contains("total_calories")) {
        int totalCal = (int)data.value("total_calories").toDouble();
        QListWidgetItem* totalItem = new QListWidgetItem(
            "总热量: " + QString::number(totalCal) + " kcal");
        QFont totalFont;
        totalFont.setBold(true);
        totalFont.setPointSize(12);
        totalItem->setFont(totalFont);
        totalItem->setForeground(QColor("#2196F3"));
        totalItem->setSizeHint(QSize(0, 30));
        ui->listWidget->addItem(totalItem);
    }

    if (data.contains("nutrition_tips")) {
        QListWidgetItem* tipsItem = new QListWidgetItem(
            "营养建议: " + data.value("nutrition_tips").toString());
        tipsItem->setForeground(QColor("#888888"));
        tipsItem->setSizeHint(QSize(0, 25));
        ui->listWidget->addItem(tipsItem);
    }

    if (data.value("is_fallback").toBool()) {
        QListWidgetItem* fbItem = new QListWidgetItem(
            "(当前为本地推荐，AI服务暂不可用)");
        fbItem->setForeground(QColor("#F44336"));
        fbItem->setSizeHint(QSize(0, 25));
        ui->listWidget->addItem(fbItem);
    }
}

void eatwhat::onReplacementReceived(const QJsonObject& data)
{
    QJsonArray replacements = data.value("substitutes").toArray();
    if (replacements.isEmpty()) return;

    QJsonObject firstReplacement = replacements[0].toObject();
    QString newName = firstReplacement.value("name").toString();
    QString newAmount = firstReplacement.value("amount").toString("适量");

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        if (!item->data(Qt::UserRole + 1).toBool()) continue;

        FoodList* fw = qobject_cast<FoodList*>(ui->listWidget->itemWidget(item));
        if (fw) {
            fw->setFoodData(newName, newAmount);
        }
        item->setData(Qt::UserRole + 1, false);
        item->setData(Qt::UserRole, firstReplacement);
        break;
    }
}

void eatwhat::onReplaceFoodClicked(const QJsonObject& foodItem)
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        QJsonObject foodData = item->data(Qt::UserRole).toJsonObject();
        if (foodData.isEmpty()) continue;

        if (foodData.value("name").toString() == foodItem.value("name").toString()) {
            item->setData(Qt::UserRole + 1, true);

            FoodList* fw = qobject_cast<FoodList*>(ui->listWidget->itemWidget(item));
            if (fw) {
                fw->setFoodData("替换中...", "");
            }
            break;
        }
    }

    QJsonObject preferences;
    preferences["goal"] = "healthy";
    viewmodel_->replaceFood(foodItem, preferences);
}
