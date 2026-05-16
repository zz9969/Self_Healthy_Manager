#include "realeat.h"
#include "ui_realeat.h"
#include <QRegularExpression>
#include <QTime>

RealEat::RealEat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealEat),
    viewmodel_(new RealEatViewModel(this))
{
    ui->setupUi(this);
    setWindowTitle("记录今日饮食");

    connect(ui->pushButton, &QPushButton::clicked, this, &RealEat::onConfirmClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() { emit backToMain(); });
    connect(viewmodel_, &RealEatViewModel::mealRecorded, this, &RealEat::onMealRecorded);
}

RealEat::~RealEat()
{
    delete ui;
}

void RealEat::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    viewmodel_->loadTodayRecords();
}

void RealEat::onConfirmClicked()
{
    QString text = ui->textEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入今日所吃菜品");
        return;
    }

    ui->pushButton->setEnabled(false);

    QStringList foodNames = text.split(QRegularExpression("[,，、\n]"), QString::SkipEmptyParts);

    QTime now = QTime::currentTime();
    int mealType = 1;
    int hour = now.hour();
    if (hour >= 5 && hour < 10) mealType = 0;
    else if (hour >= 10 && hour < 15) mealType = 1;
    else if (hour >= 15 && hour < 21) mealType = 2;
    else mealType = 3;

    for (const QString& name : foodNames) {
        QString trimmed = name.trimmed();
        if (trimmed.isEmpty()) continue;

        QJsonObject body;
        body["meal_type"] = mealType;
        body["food_name"] = trimmed;
        body["quantity"] = 1.0;
        body["calories"] = 0.0;

        ApiClient::getInstance().post("/api/record/diet", body,
            [this](const QJsonObject& data) {
                onMealRecorded(data);
            },
            [this](int /*statusCode*/, const QString& errorMsg) {
                ui->pushButton->setEnabled(true);
                QMessageBox::warning(this, "记录失败", errorMsg);
            });
    }
}

void RealEat::onMealRecorded(const QJsonObject& /*data*/)
{
    ui->pushButton->setEnabled(true);
    ui->textEdit->clear();
    emit navigateToToday();
}
