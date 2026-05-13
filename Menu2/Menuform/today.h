#ifndef TODAY_H
#define TODAY_H

#include <QWidget>
#include <QShowEvent>
#include "viewmodels/today_viewmodel.h"

namespace Ui {
class Today;
}

class Today : public QWidget
{
    Q_OBJECT

public:
    explicit Today(QWidget *parent = nullptr);
    ~Today();

signals:
    void backToMain();

private slots:
    void onStatsLoaded(const QJsonObject& data);

private:
    void showEvent(QShowEvent* event) override;
    Ui::Today *ui;
    TodayViewModel* viewmodel_;
};

#endif // TODAY_H
