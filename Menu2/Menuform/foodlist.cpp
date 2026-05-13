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

void FoodList::setFoodData(const QString& name, const QString& amount)
{
    ui->label->setText(name);
    ui->label_2->setText(amount);
}

QString FoodList::foodName() const
{
    return ui->label->text();
}

QString FoodList::foodAmount() const
{
    return ui->label_2->text();
}

void FoodList::onReplaceClicked()
{
    QString currentName = ui->label->text();
    emit replaceRequested(currentName);

    // 兼容原有逻辑：autoLoad 模式下直接调 API
    if (viewmodel_) {
        ApiClient::getInstance().get("/api/diet/substitutes",
            [this](const QJsonObject& data) {
                QJsonArray replacements = data.value("substitutes").toArray();
                if (!replacements.isEmpty()) {
                    QJsonObject first = replacements[0].toObject();
                    ui->label->setText(first.value("name").toString("暂无替代"));
                }
            },
            [](int, const QString&) {});
    }
}

void FoodList::onFoodListLoaded(const QJsonObject& data)
{
    QJsonArray foods = data.value("foods").toArray();
    if (!foods.isEmpty()) {
        QJsonObject first = foods[0].toObject();
        ui->label->setText(first.value("name").toString("食物名称"));
        ui->label_2->setText("所需重量: " + QString::number(first.value("amount").toDouble(100)) + "g");
    }
}

