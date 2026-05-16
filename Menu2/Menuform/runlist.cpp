#include "runlist.h"
#include "ui_runlist.h"

runlist::runlist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::runlist)
{
    ui->setupUi(this);
}

runlist::~runlist()
{
    delete ui;
}

void runlist::setData(const QString& type, const QString& bodyPart)
{
    ui->label->setText(type);
    ui->label_2->setText(bodyPart);
}

void runlist::setData(const QJsonObject& data)
{
    QString type = data.value("type").toString("未知运动");
    QString bodyPart = data.value("bodyPart").toString("全身");
    QString category = data.value("category").toString();
    QString reason = data.value("reason").toString();

    ui->label->setText(type + (category.isEmpty() ? "" : " [" + category + "]"));
    ui->label_2->setText(bodyPart + (reason.isEmpty() ? "" : " - " + reason));
}
