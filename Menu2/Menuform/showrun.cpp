#include "showrun.h"
#include "ui_showrun.h"
#include <QJsonObject>

showRun::showRun(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::showRun),
    doneModel_(new QStandardItemModel(this)),
    suggestModel_(new QStandardItemModel(this))
{
    ui->setupUi(this);
    setWindowTitle("锻炼分析结果");

    ui->listView->setModel(doneModel_);
    ui->listView_2->setModel(suggestModel_);

    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->pushButton, &QPushButton::clicked, this, &QDialog::accept);
}

showRun::~showRun()
{
    delete ui;
}

void showRun::setData(const QJsonArray& doneExercises, const QJsonArray& suggestedExercises)
{
    doneModel_->clear();
    for (const auto& item : doneExercises) {
        QJsonObject ex = item.toObject();
        QString type = ex.value("type").toString();
        QString bodyPart = ex.value("bodyPart").toString();
        QString category = ex.value("category").toString();

        QString display = type + " [" + category + "]\n锻炼部位: " + bodyPart;
        QStandardItem* stdItem = new QStandardItem(display);
        stdItem->setEditable(false);
        doneModel_->appendRow(stdItem);
    }

    suggestModel_->clear();
    for (const auto& item : suggestedExercises) {
        QJsonObject ex = item.toObject();
        QString type = ex.value("type").toString();
        QString bodyPart = ex.value("bodyPart").toString();
        QString category = ex.value("category").toString();
        QString reason = ex.value("reason").toString();

        QString display = type + " [" + category + "]\n锻炼部位: " + bodyPart
                        + (reason.isEmpty() ? "" : "\n推荐理由: " + reason);
        QStandardItem* stdItem = new QStandardItem(display);
        stdItem->setEditable(false);
        suggestModel_->appendRow(stdItem);
    }
}
