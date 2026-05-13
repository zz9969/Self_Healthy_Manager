#ifndef CONSTITUTION_H
#define CONSTITUTION_H

#include <QWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QShowEvent>
#include "viewmodels/constitution_viewmodel.h"

namespace Ui {
class ConstitutionWidget;
}

class ConstitutionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConstitutionWidget(QWidget *parent = nullptr);
    ~ConstitutionWidget();

signals:
    void backToMain();
    void analysisCompleted(const QJsonObject& result);

private slots:
    void onSubmitClicked();
    void onQuestionsLoaded(const QJsonObject& data);
    void onAnalysisCompleted(const QJsonObject& data);
    void onResultLoaded(const QJsonObject& data);

private:
    void buildQuestionUI();
    void showResult(const QJsonObject& data);
    void showEvent(QShowEvent* event) override;

    Ui::ConstitutionWidget *ui;
    ConstitutionViewModel* viewmodel_;
    QJsonArray questions_;
    QList<QButtonGroup*> buttonGroups_;
    QVBoxLayout* questionsLayout_;
    QWidget* questionsContainer_;
    QScrollArea* scrollArea_;
};

#endif // CONSTITUTION_H
