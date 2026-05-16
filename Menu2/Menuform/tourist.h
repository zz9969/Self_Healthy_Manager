#ifndef TOURIST_H
#define TOURIST_H

#include <QWidget>
#include <QMessageBox>
#include "viewmodels/tourist_viewmodel.h"

namespace Ui {
class tourist;
}

class tourist : public QWidget
{
    Q_OBJECT

public:
    explicit tourist(QWidget *parent = nullptr);
    ~tourist();

signals:
    void sessionStarted(const QJsonObject& sessionInfo);
    void convertToRegistered(const QJsonObject& userInfo);

private slots:
    void onConfirmClicked();
    void onSessionStarted(const QJsonObject& data);
    void onUserConverted(const QJsonObject& data);

private:
    Ui::tourist *ui;
    TouristViewModel* viewmodel_;
};

#endif // TOURIST_H
