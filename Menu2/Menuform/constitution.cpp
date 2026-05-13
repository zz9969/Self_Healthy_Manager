#include "constitution.h"
#include "ui_constitution.h"
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QFrame>

ConstitutionWidget::ConstitutionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConstitutionWidget),
    viewmodel_(new ConstitutionViewModel(this)),
    questionsLayout_(nullptr),
    questionsContainer_(nullptr),
    scrollArea_(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("中医体质评估");
    setMinimumSize(450, 550);

    connect(viewmodel_, &ConstitutionViewModel::questionsLoaded, this, &ConstitutionWidget::onQuestionsLoaded);
    connect(viewmodel_, &ConstitutionViewModel::analysisCompleted, this, &ConstitutionWidget::onAnalysisCompleted);
    connect(viewmodel_, &ConstitutionViewModel::resultLoaded, this, &ConstitutionWidget::onResultLoaded);
    connect(viewmodel_, &ConstitutionViewModel::errorChanged, this, [this]() {
        QString err = viewmodel_->errorMessage();
        if (!err.isEmpty()) {
            QMessageBox::warning(this, "提示", err);
        }
    });

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    QLabel* headerLabel = new QLabel("中医九种体质问卷");
    headerLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#2E7D32; padding:4px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumHeight(400);
    mainLayout->addWidget(scrollArea_, 1);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    QPushButton* backBtn = new QPushButton("返回主页");
    backBtn->setStyleSheet("QPushButton{padding:8px 16px; font-size:14px; border-radius:4px; background:#E0E0E0;}"
                           "QPushButton:hover{background:#BDBDBD;}");
    connect(backBtn, &QPushButton::clicked, this, [this]() { emit backToMain(); });

    QPushButton* submitBtn = new QPushButton("提交评估");
    submitBtn->setStyleSheet("QPushButton{padding:8px 24px; font-size:14px; font-weight:bold; "
                             "border-radius:4px; background:#4CAF50; color:white;}"
                             "QPushButton:hover{background:#388E3C;}");
    connect(submitBtn, &QPushButton::clicked, this, &ConstitutionWidget::onSubmitClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(backBtn);
    btnLayout->addWidget(submitBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}

ConstitutionWidget::~ConstitutionWidget()
{
    delete ui;
}

void ConstitutionWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    viewmodel_->loadQuestions();
}

void ConstitutionWidget::onSubmitClicked()
{
    QJsonArray answers;
    for (int i = 0; i < questions_.size(); ++i) {
        QJsonObject q = questions_[i].toObject();
        int qid = q.value("question_id").toInt();
        QButtonGroup* group = buttonGroups_.value(i, nullptr);
        if (!group || group->checkedId() < 0) {
            QMessageBox::warning(this, "提示",
                QString("请完成第 %1 题后再提交").arg(i + 1));
            return;
        }
        QString optionKey = group->button(group->checkedId())->property("option_key").toString();
        QJsonObject ans;
        ans["question_id"] = qid;
        ans["option_key"] = optionKey;
        answers.append(ans);
    }

    qDebug() << "[Constitution] submit answers:" << answers;
    viewmodel_->submitAnswers(answers);
}

void ConstitutionWidget::onQuestionsLoaded(const QJsonObject& data)
{
    questions_ = data.value("questions").toArray();
    qDebug() << "[Constitution] questions loaded, count:" << questions_.size();
    buildQuestionUI();
}

void ConstitutionWidget::onAnalysisCompleted(const QJsonObject& data)
{
    qDebug() << "[Constitution] analysis completed:" << data;
    showResult(data);
    emit analysisCompleted(data);
}

void ConstitutionWidget::onResultLoaded(const QJsonObject& data)
{
    showResult(data);
}

void ConstitutionWidget::buildQuestionUI()
{
    qDeleteAll(buttonGroups_);
    buttonGroups_.clear();

    if (questionsContainer_) {
        delete questionsContainer_;
        questionsContainer_ = nullptr;
    }

    questionsContainer_ = new QWidget();
    questionsLayout_ = new QVBoxLayout(questionsContainer_);
    questionsLayout_->setSpacing(4);
    questionsLayout_->setContentsMargins(4, 4, 4, 4);

    for (int i = 0; i < questions_.size(); ++i) {
        QJsonObject q = questions_[i].toObject();
        QString text = q.value("text").toString();
        QJsonArray options = q.value("options").toArray();

        QLabel* qLabel = new QLabel(QString("%1. %2").arg(i + 1).arg(text));
        qLabel->setWordWrap(true);
        qLabel->setStyleSheet("font-weight:bold; font-size:13px; margin-top:10px; color:#333;");
        questionsLayout_->addWidget(qLabel);

        QButtonGroup* group = new QButtonGroup(this);
        for (int j = 0; j < options.size(); ++j) {
            QJsonObject opt = options[j].toObject();
            QString key = opt.value("key").toString();
            QString optText = opt.value("text").toString();

            QString label = QString("%1. %2").arg(key).arg(optText);
            QRadioButton* radio = new QRadioButton(label);
            radio->setProperty("option_key", key);
            radio->setStyleSheet("font-size:12px; margin-left:20px; padding:2px;");
            group->addButton(radio, j);
            questionsLayout_->addWidget(radio);
        }
        buttonGroups_.append(group);

        QFrame* separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setStyleSheet("color:#E0E0E0;");
        questionsLayout_->addWidget(separator);
    }

    questionsLayout_->addStretch();
    scrollArea_->setWidget(questionsContainer_);
}

void ConstitutionWidget::showResult(const QJsonObject& data)
{
    qDeleteAll(buttonGroups_);
    buttonGroups_.clear();

    if (questionsContainer_) {
        delete questionsContainer_;
        questionsContainer_ = nullptr;
    }

    questionsContainer_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(questionsContainer_);
    layout->setSpacing(6);
    layout->setContentsMargins(8, 8, 8, 8);

    QString primaryType = data.value("primary_type").toString();
    QString secondaryType = data.value("secondary_type").toString();

    QLabel* title = new QLabel("体质评估结果");
    title->setStyleSheet("font-size:20px; font-weight:bold; color:#4CAF50; padding:6px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel* primaryLabel = new QLabel("主要体质: " + primaryType + "质");
    primaryLabel->setStyleSheet("font-size:16px; font-weight:bold; color:#2196F3; padding:4px;");
    primaryLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(primaryLabel);

    if (!secondaryType.isEmpty()) {
        QLabel* secondaryLabel = new QLabel("兼夹体质: " + secondaryType + "质");
        secondaryLabel->setStyleSheet("font-size:14px; color:#FF9800; padding:2px;");
        secondaryLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(secondaryLabel);
    }

    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#BDBDBD;");
    layout->addWidget(sep);

    QLabel* scoreTitle = new QLabel("九种体质分值");
    scoreTitle->setStyleSheet("font-weight:bold; font-size:14px; color:#555;");
    layout->addWidget(scoreTitle);

    QJsonObject scores = data.value("scores").toObject();
    QStringList typeKeys = {"平和", "气虚", "阳虚", "阴虚", "痰湿", "湿热", "血瘀", "气郁", "特禀"};

    double maxScore = 0;
    for (const QString& type : typeKeys) {
        double s = scores.value(type).toDouble(0);
        if (s > maxScore) maxScore = s;
    }
    if (maxScore <= 0) maxScore = 1;

    for (const QString& type : typeKeys) {
        double score = scores.value(type).toDouble(0);
        int percent = qBound(0, static_cast<int>(score / maxScore * 100), 100);

        QHBoxLayout* rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(6);

        QLabel* nameLabel = new QLabel(type + "质");
        nameLabel->setFixedWidth(50);
        nameLabel->setStyleSheet("font-size:12px; color:#333;");

        QProgressBar* bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(percent);
        bar->setTextVisible(true);
        bar->setFormat(QString("%1").arg(score, 0, 'f', 1));
        bar->setMaximumHeight(16);
        bar->setMinimumWidth(200);

        if (type == primaryType) {
            bar->setStyleSheet("QProgressBar::chunk{background:#2196F3; border-radius:3px;}"
                               "QProgressBar{border:1px solid #BBDEFB; border-radius:3px; text-align:center;}");
        } else if (type == secondaryType) {
            bar->setStyleSheet("QProgressBar::chunk{background:#FF9800; border-radius:3px;}"
                               "QProgressBar{border:1px solid #FFE0B2; border-radius:3px; text-align:center;}");
        } else {
            bar->setStyleSheet("QProgressBar::chunk{background:#81C784; border-radius:3px;}"
                               "QProgressBar{border:1px solid #C8E6C9; border-radius:3px; text-align:center;}");
        }

        rowLayout->addWidget(nameLabel);
        rowLayout->addWidget(bar, 1);
        layout->addLayout(rowLayout);
    }

    QString dietAdvice = data.value("diet_advice").toString();
    if (!dietAdvice.isEmpty()) {
        QFrame* sep2 = new QFrame();
        sep2->setFrameShape(QFrame::HLine);
        sep2->setStyleSheet("color:#BDBDBD;");
        layout->addWidget(sep2);

        QLabel* adviceTitle = new QLabel("饮食调养建议");
        adviceTitle->setStyleSheet("font-weight:bold; font-size:14px; color:#2E7D32;");
        layout->addWidget(adviceTitle);

        QLabel* adviceLabel = new QLabel(dietAdvice);
        adviceLabel->setWordWrap(true);
        adviceLabel->setStyleSheet("font-size:13px; color:#555; padding:4px; "
                                   "background:#F1F8E9; border-radius:4px;");
        layout->addWidget(adviceLabel);
    }

    layout->addStretch();
    scrollArea_->setWidget(questionsContainer_);
}
