#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "viewmodels/login_viewmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void loginSuccessful(const QJsonObject& userInfo);
    void touristModeSelected(const QJsonObject& touristInfo);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onClearLoginClicked();
    void onClearRegisterClicked();
    void onTouristClicked();
    void onLoginSuccess(const QJsonObject& data);
    void onRegisterSuccess(const QJsonObject& data);
    void onTouristSuccess(const QJsonObject& data);
    void onLoadingChanged();
    void onErrorChanged();

private:
    Ui::LoginDialog *ui;
    LoginViewModel* viewmodel_;
};

#endif // LOGINDIALOG_H
