#include "foodlist.h"
#include "ui_foodlist.h"
#include <QPushButton>

FoodList::FoodList(QWidget *parent, bool autoLoad) :
    QWidget(parent),
    ui(new Ui::FoodList),
    viewmodel_(nullptr),
    autoLoad_(autoLoad)
{
    ui->setupUi(this);
    setWindowTitle("食物列表");

    connect(ui->pushButton_2, &QPushButton::clicked, this, &FoodList::onReplaceClicked);

    if (autoLoad_) {
        viewmodel_ = new FoodListViewModel(this);
        connect(viewmodel_, &FoodListViewModel::foodListLoaded, this, &FoodList::onFoodListLoaded);

        QPushButton* backBtn = new QPushButton("返回主页", this);
        connect(backBtn, &QPushButton::clicked, this, [this]() { emit backToMain(); });
    }
}

FoodList::~FoodList()
{
    delete ui;
}

void FoodList::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (autoLoad_ && viewmodel_) {
        viewmodel_->loadFoodList();
    }
}

void FoodList::setFoodData(const QString& name, const QString& amount, const QString& category)
{
    ui->label->setText(name);
    ui->label_2->setText(amount);
    if (!category.isEmpty()) {
        currentCategory_ = category;
    }
}

QString FoodList::foodName() const
{
    return ui->label->text();
}

QString FoodList::foodAmount() const
{
    return ui->label_2->text();
}

QString FoodList::foodCategory() const
{
    return currentCategory_;
}

void FoodList::onReplaceClicked()
{
    QString currentName = ui->label->text();
    emit replaceRequested(currentName);

    QJsonObject body;
    body["food_name"] = currentName;
    if (!currentCategory_.isEmpty()) {
        body["category"] = currentCategory_;
    }

    ApiClient::getInstance().post("/api/diet/substitutes", body,
        [this](const QJsonObject& data) {
            QJsonArray substitutes = data.value("substitutes").toArray();
            if (!substitutes.isEmpty()) {
                int idx = qrand() % substitutes.size();
                QJsonObject replacement = substitutes[idx].toObject();
                ui->label->setText(replacement.value("name").toString("暂无替代"));
                QString newCategory = replacement.value("category").toString();
                if (!newCategory.isEmpty()) {
                    currentCategory_ = newCategory;
                }
            }
        },
        [](int, const QString&) {});
}

void FoodList::onFoodListLoaded(const QJsonObject& data)
{
    QJsonArray foods = data.value("foods").toArray();
    if (!foods.isEmpty()) {
        QJsonObject first = foods[0].toObject();
        ui->label->setText(first.value("name").toString("食物名称"));
        currentCategory_ = first.value("category").toString();
        ui->label_2->setText("所需重量: " + QString::number(first.value("amount").toDouble(100)) + "g");
    }
}
