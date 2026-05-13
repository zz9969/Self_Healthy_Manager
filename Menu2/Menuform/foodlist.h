#ifndef FOODLIST_H
#define FOODLIST_H

#include <QWidget>
#include <QJsonObject>
#include <QShowEvent>
#include "viewmodels/foodlist_viewmodel.h"

namespace Ui {
class FoodList;
}

class FoodList : public QWidget
{
    Q_OBJECT

public:
    explicit FoodList(QWidget *parent = nullptr, bool autoLoad = true);
    ~FoodList();

    void setFoodData(const QString& name, const QString& amount);

    QString foodName() const;
    QString foodAmount() const;

signals:
    void backToMain();
    void foodSelected(const QJsonObject& food);
    void replaceRequested(const QString& foodName);

private slots:
    void onReplaceClicked();
    void onFoodListLoaded(const QJsonObject& data);

private:
    void showEvent(QShowEvent* event) override;
    Ui::FoodList *ui;
    FoodListViewModel* viewmodel_;
    bool autoLoad_;
};

#endif // FOODLIST_H
