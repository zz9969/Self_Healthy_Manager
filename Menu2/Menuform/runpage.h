#ifndef RUNPAGE_H
#define RUNPAGE_H

#include <QWidget>

namespace Ui {
class runPage;
}

class runPage : public QWidget
{
    Q_OBJECT

public:
    explicit runPage(QWidget *parent = nullptr);
    ~runPage();

private:
    Ui::runPage *ui;
};

#endif // RUNPAGE_H
