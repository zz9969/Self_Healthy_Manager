#ifndef SHOWRUN_H
#define SHOWRUN_H

#include <QDialog>
#include <QJsonArray>
#include <QStandardItemModel>

namespace Ui {
class showRun;
}

class showRun : public QDialog
{
    Q_OBJECT

public:
    explicit showRun(QWidget *parent = nullptr);
    ~showRun();

    void setData(const QJsonArray& doneExercises, const QJsonArray& suggestedExercises);

private:
    Ui::showRun *ui;
    QStandardItemModel* doneModel_;
    QStandardItemModel* suggestModel_;
};

#endif // SHOWRUN_H
