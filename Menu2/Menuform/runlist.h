#ifndef RUNLIST_H
#define RUNLIST_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class runlist;
}

class runlist : public QWidget
{
    Q_OBJECT

public:
    explicit runlist(QWidget *parent = nullptr);
    ~runlist();

    void setData(const QString& type, const QString& bodyPart);
    void setData(const QJsonObject& data);

private:
    Ui::runlist *ui;
};

#endif // RUNLIST_H
