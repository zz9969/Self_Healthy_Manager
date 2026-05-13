#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , viewmodel_(new LoginViewModel(this))
{
    ui->setupUi(this);
    setWindowTitle("每日饮食推荐 - 登录");

    connect(ui->pb_Login, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->pb_login_clear, &QPushButton::clicked, this, &LoginDialog::onClearLoginClicked);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &LoginDialog::onTouristClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &LoginDialog::onClearRegisterClicked);

    connect(viewmodel_, &LoginViewModel::loginSuccess, this, &LoginDialog::onLoginSuccess);
    connect(viewmodel_, &LoginViewModel::registerSuccess, this, &LoginDialog::onRegisterSuccess);
    connect(viewmodel_, &LoginViewModel::touristSuccess, this, &LoginDialog::onTouristSuccess);
    connect(viewmodel_, &LoginViewModel::loadingChanged, this, &LoginDialog::onLoadingChanged);
    connect(viewmodel_, &LoginViewModel::errorChanged, this, &LoginDialog::onErrorChanged);

    connect(ui->lineEdit_3, &QLineEdit::textChanged, viewmodel_, &LoginViewModel::setPhone);
    connect(ui->lineEdit, &QLineEdit::textChanged, viewmodel_, &LoginViewModel::setPassword);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onLoginClicked()
{
    viewmodel_->setPhone(ui->lineEdit_3->text());
    viewmodel_->setPassword(ui->lineEdit->text());
    qDebug() << "[LoginDialog] onLoginClicked phone=" << ui->lineEdit_3->text() << "pwd=" << ui->lineEdit->text();
    viewmodel_->login();
}

void LoginDialog::onRegisterClicked()
{
    QString phone = ui->lineEdit_2->text();
    QString password = ui->lineEdit_4->text();
    QString height = ui->lineEdit_5->text();
    QString weight = ui->lineEdit_6->text();
    QString gender = ui->lineEdit_7->text();
    QString age = ui->lineEdit_8->text();

    if (phone.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入手机号和密码");
        return;
    }

    viewmodel_->setPhone(phone);
    viewmodel_->setPassword(password);

    QJsonObject body;
    body["phone"] = phone;
    body["password"] = password;
    body["height"] = height.isEmpty() ? 170.0 : height.toDouble();
    body["weight"] = weight.isEmpty() ? 65.0 : weight.toDouble();
    body["gender"] = gender.isEmpty() ? "male" : gender;
    body["age"] = age.isEmpty() ? 25 : age.toInt();

    ApiClient::getInstance().post("/api/user/register", body,
        [this](const QJsonObject& data) {
            viewmodel_->setProperty("loading", false);
            onRegisterSuccess(data);
        },
        [this](int /*statusCode*/, const QString& errorMsg) {
            viewmodel_->setProperty("loading", false);
            QMessageBox::warning(this, "注册失败", errorMsg);
        });
}

void LoginDialog::onClearLoginClicked()
{
    ui->lineEdit_3->clear();
    ui->lineEdit->clear();
}

void LoginDialog::onClearRegisterClicked()
{
    ui->lineEdit_2->clear();
    ui->lineEdit_4->clear();
    ui->lineEdit_5->clear();
    ui->lineEdit_6->clear();
    ui->lineEdit_7->clear();
    ui->lineEdit_8->clear();
}

void LoginDialog::onTouristClicked()
{
    viewmodel_->enterTouristMode();
}

void LoginDialog::onLoginSuccess(const QJsonObject& data)
{
    QString token = data.value("token").toString();
    if (!token.isEmpty()) {
        ApiClient::getInstance().setAuthToken(token);
    }
    emit loginSuccessful(data);
    accept();
}

void LoginDialog::onRegisterSuccess(const QJsonObject& data)
{
    QString token = data.value("token").toString();
    if (!token.isEmpty()) {
        ApiClient::getInstance().setAuthToken(token);
    }
    QMessageBox::information(this, "注册成功", "注册成功，即将进入主页");
    emit loginSuccessful(data);
    accept();
}

void LoginDialog::onTouristSuccess(const QJsonObject& data)
{
    QString token = data.value("token").toString();
    if (!token.isEmpty()) {
        ApiClient::getInstance().setAuthToken(token);
    }
    emit touristModeSelected(data);
    accept();
}

void LoginDialog::onLoadingChanged()
{
    bool loading = viewmodel_->isLoading();
    ui->pb_Login->setEnabled(!loading);
    ui->pb_login_clear->setEnabled(!loading);
    ui->pushButton_5->setEnabled(!loading);
}

void LoginDialog::onErrorChanged()
{
    QString error = viewmodel_->errorMessage();
    if (!error.isEmpty()) {
        ui->tabWidget->setCurrentIndex(0);
    }
}
