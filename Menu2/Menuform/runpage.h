#ifndef RUNPAGE_H
#define RUNPAGE_H

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class runPage;
}

class runPage : public QWidget
{
    Q_OBJECT

public:
    explicit runPage(QWidget *parent = nullptr);
    ~runPage();

signals:
    void backToMain();
    void exerciseAnalyzed(const QJsonArray& doneExercises, const QJsonArray& suggestedExercises);

private slots:
    void onConfirmClicked();
    void onBackClicked();

private:
    QJsonObject analyzeExercise(const QString& text);
    QJsonArray analyzeExercises(const QString& text);
    QJsonArray suggestExercises(const QJsonArray& done);
    Ui::runPage *ui;
};

#endif // RUNPAGE_H
