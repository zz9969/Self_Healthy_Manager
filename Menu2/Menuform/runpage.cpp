#include "runpage.h"
#include "ui_runpage.h"

runPage::runPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::runPage)
{
    ui->setupUi(this);
}

runPage::~runPage()
{
    delete ui;
}
