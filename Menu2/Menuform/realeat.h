#ifndef REALEAT_H
#define REALEAT_H

#include <QWidget>
#include <QMessageBox>
#include <QShowEvent>
#include "viewmodels/realeat_viewmodel.h"

namespace Ui {
class RealEat;
}

class RealEat : public QWidget
{
    Q_OBJECT

public:
    explicit RealEat(QWidget *parent = nullptr);
    ~RealEat();

signals:
    void backToMain();
    void navigateToToday();

private slots:
    void onConfirmClicked();
    void onMealRecorded(const QJsonObject& data);

private:
    void showEvent(QShowEvent* event) override;
    Ui::RealEat *ui;
    RealEatViewModel* viewmodel_;
};

#endif // REALEAT_H
