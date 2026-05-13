#include "tourist.h"
#include "ui_tourist.h"

tourist::tourist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tourist),
    viewmodel_(new TouristViewModel(this))
{
    ui->setupUi(this);
    setWindowTitle("游客模式 - 基本信息");

    connect(ui->pushButton, &QPushButton::clicked, this, &tourist::onConfirmClicked);
    connect(viewmodel_, &TouristViewModel::touristSessionStarted, this, &tourist::onSessionStarted);
    connect(viewmodel_, &TouristViewModel::userConverted, this, &tourist::onUserConverted);
}

tourist::~tourist()
{
    delete ui;
}

void tourist::onConfirmClicked()
{
    double height = ui->lineEdit->text().isEmpty() ? 170.0 : ui->lineEdit->text().toDouble();
    double weight = ui->lineEdit_2->text().isEmpty() ? 65.0 : ui->lineEdit_2->text().toDouble();
    QString gender = ui->lineEdit_3->text().isEmpty() ? "male" : ui->lineEdit_3->text();
    int age = ui->lineEdit_4->text().isEmpty() ? 25 : ui->lineEdit_4->text().toInt();

    QJsonObject body;
    body["height"] = height;
    body["weight"] = weight;
    body["gender"] = gender;
    body["age"] = age;

    ApiClient::getInstance().post("/api/auth/tourist", body,
        [this](const QJsonObject& data) {
            onSessionStarted(data);
        },
        [this](int /*statusCode*/, const QString& errorMsg) {
            QMessageBox::warning(this, "提示", errorMsg);
        });
}

void tourist::onSessionStarted(const QJsonObject& data)
{
    QString token = data.value("token").toString();
    if (!token.isEmpty()) {
        ApiClient::getInstance().setAuthToken(token);
    }
    emit sessionStarted(data);
}

void tourist::onUserConverted(const QJsonObject& data)
{
    emit convertToRegistered(data);
}
