#include "realeat.h"
#include "ui_realeat.h"
#include <QRegularExpression>
#include <QPushButton>

RealEat::RealEat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealEat),
    viewmodel_(new RealEatViewModel(this))
{
    ui->setupUi(this);
    setWindowTitle("记录今日饮食");

    connect(ui->pushButton, &QPushButton::clicked, this, &RealEat::onConfirmClicked);
    connect(viewmodel_, &RealEatViewModel::mealRecorded, this, &RealEat::onMealRecorded);

    QPushButton* backBtn = new QPushButton("返回主页", this);
    connect(backBtn, &QPushButton::clicked, this, [this]() { emit backToMain(); });
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

    for (const QString& name : foodNames) {
        QString trimmed = name.trimmed();
        if (trimmed.isEmpty()) continue;

        QJsonObject body;
        body["meal_type"] = "lunch";
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
