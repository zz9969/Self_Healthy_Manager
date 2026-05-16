#include "today.h"
#include "ui_today.h"

Today::Today(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Today),
    viewmodel_(new TodayViewModel(this))
{
    ui->setupUi(this);
    setWindowTitle("今日营养追踪");

    connect(viewmodel_, &TodayViewModel::statsLoaded, this, &Today::onStatsLoaded);
    connect(ui->pushButton, &QPushButton::clicked, this, [this]() { emit backToMain(); });
}

Today::~Today()
{
    delete ui;
}

void Today::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    viewmodel_->loadDailyReport();
}

void Today::onStatsLoaded(const QJsonObject& data)
{
    ui->listWidget->clear();

    QJsonObject intake = data.value("intake").toObject();
    QJsonObject target = data.value("target").toObject();

    ui->listWidget->addItem("=== 今日营养摄入 ===");
    ui->listWidget->addItem("");

    double cal = intake.value("total_calories").toDouble();
    double calTarget = target.value("target_calories").toDouble();
    double calPercent = calTarget > 0 ? (cal / calTarget * 100.0) : 0;
    ui->listWidget->addItem(QString("热量: %1 / %2 kcal (%3%)")
        .arg(cal, 0, 'f', 0).arg(calTarget, 0, 'f', 0).arg(calPercent, 0, 'f', 1));

    double protein = intake.value("protein_g").toDouble();
    double proteinTarget = target.value("target_protein_g").toDouble();
    double proteinPercent = proteinTarget > 0 ? (protein / proteinTarget * 100.0) : 0;
    ui->listWidget->addItem(QString("蛋白质: %1 / %2 g (%3%)")
        .arg(protein, 0, 'f', 1).arg(proteinTarget, 0, 'f', 1).arg(proteinPercent, 0, 'f', 1));

    double fat = intake.value("fat_g").toDouble();
    double fatTarget = target.value("target_fat_g").toDouble();
    double fatPercent = fatTarget > 0 ? (fat / fatTarget * 100.0) : 0;
    ui->listWidget->addItem(QString("脂肪: %1 / %2 g (%3%)")
        .arg(fat, 0, 'f', 1).arg(fatTarget, 0, 'f', 1).arg(fatPercent, 0, 'f', 1));

    double carbs = intake.value("carbohydrates_g").toDouble();
    double carbsTarget = target.value("target_carbohydrates_g").toDouble();
    double carbsPercent = carbsTarget > 0 ? (carbs / carbsTarget * 100.0) : 0;
    ui->listWidget->addItem(QString("碳水: %1 / %2 g (%3%)")
        .arg(carbs, 0, 'f', 1).arg(carbsTarget, 0, 'f', 1).arg(carbsPercent, 0, 'f', 1));

    ui->listWidget->addItem("");

    double bmr = target.value("bmr").toDouble();
    double tdee = target.value("tdee").toDouble();
    ui->listWidget->addItem(QString("基础代谢率(BMR): %1 kcal").arg(bmr, 0, 'f', 0));
    ui->listWidget->addItem(QString("每日总消耗(TDEE): %1 kcal").arg(tdee, 0, 'f', 0));

    int mealsCount = intake.value("meals_count").toInt();
    ui->listWidget->addItem(QString("记录数: %1").arg(mealsCount));
}
