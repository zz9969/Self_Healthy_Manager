#include "runpage.h"
#include "ui_runpage.h"
#include <QDebug>
#include <QMessageBox>
#include <QSet>
#include <QRegularExpression>

struct ExerciseRule {
    QString keyword;
    QString type;
    QString bodyPart;
    QString category;
};

static const ExerciseRule exerciseRules[] = {
    {"走", "步行", "腿部、臀部", "有氧"},
    {"跑", "跑步", "腿部、心肺", "有氧"},
    {"游泳", "游泳", "全身、心肺", "有氧"},
    {"骑", "骑行", "腿部、心肺", "有氧"},
    {"跳", "跳跃", "腿部、心肺", "有氧"},
    {"爬", "攀爬", "腿部、手臂", "有氧"},
    {"篮球", "篮球", "全身、心肺", "有氧"},
    {"足球", "足球", "腿部、心肺", "有氧"},
    {"羽毛球", "羽毛球", "手臂、肩部", "有氧"},
    {"乒乓球", "乒乓球", "手臂、手腕", "有氧"},
    {"网球", "网球", "手臂、肩部", "有氧"},
    {"排球", "排球", "手臂、腿部", "有氧"},
    {"瑜伽", "瑜伽", "全身、柔韧性", "柔韧"},
    {"拉伸", "拉伸", "全身、柔韧性", "柔韧"},
    {"举", "举重", "手臂、肩部", "力量"},
    {"哑铃", "哑铃", "手臂、肩部", "力量"},
    {"俯卧撑", "俯卧撑", "胸部、手臂", "力量"},
    {"仰卧起坐", "仰卧起坐", "腹部", "力量"},
    {"深蹲", "深蹲", "腿部、臀部", "力量"},
    {"引体向上", "引体向上", "背部、手臂", "力量"},
    {"平板支撑", "平板支撑", "核心、腹部", "力量"},
    {"太极", "太极拳", "全身、平衡", "柔韧"},
    {"舞", "舞蹈", "全身、心肺", "有氧"},
    {"划", "划船", "背部、手臂", "有氧"},
    {"滑雪", "滑雪", "腿部、核心", "有氧"},
    {"登山", "登山", "腿部、心肺", "有氧"},
    {"散步", "散步", "腿部", "有氧"},
    {"慢跑", "慢跑", "腿部、心肺", "有氧"},
};

runPage::runPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::runPage)
{
    ui->setupUi(this);
    setWindowTitle("今日锻炼");

    connect(ui->pushButton, &QPushButton::clicked, this, &runPage::onConfirmClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &runPage::onBackClicked);
}

runPage::~runPage()
{
    delete ui;
}

QJsonObject runPage::analyzeExercise(const QString& text)
{
    QJsonObject result;
    result["type"] = "其他运动";
    result["bodyPart"] = "全身";
    result["category"] = "有氧";

    for (const auto& rule : exerciseRules) {
        if (text.contains(rule.keyword)) {
            result["type"] = rule.type;
            result["bodyPart"] = rule.bodyPart;
            result["category"] = rule.category;
            return result;
        }
    }
    return result;
}

QJsonArray runPage::analyzeExercises(const QString& text)
{
    QJsonArray results;
    QSet<QString> foundTypes;

    QStringList sentences = text.split(QRegularExpression("[，。、,.\n!?！？；;]"), QString::SkipEmptyParts);
    if (sentences.isEmpty()) {
        sentences << text;
    }

    for (const QString& sentence : sentences) {
        QString trimmed = sentence.trimmed();
        if (trimmed.isEmpty()) continue;

        QJsonObject ex = analyzeExercise(trimmed);
        QString type = ex.value("type").toString();

        if (!foundTypes.contains(type)) {
            foundTypes.insert(type);
            ex["description"] = trimmed;
            results.append(ex);
        }
    }

    if (results.isEmpty()) {
        QJsonObject ex = analyzeExercise(text);
        ex["description"] = text;
        results.append(ex);
    }

    return results;
}

QJsonArray runPage::suggestExercises(const QJsonArray& done)
{
    QSet<QString> doneCategories;
    QSet<QString> doneBodyParts;

    for (const auto& item : done) {
        QJsonObject ex = item.toObject();
        doneCategories.insert(ex.value("category").toString());
        QStringList parts = ex.value("bodyPart").toString().split("、");
        for (const QString& p : parts) {
            doneBodyParts.insert(p.trimmed());
        }
    }

    struct Suggestion { QString type; QString bodyPart; QString category; QString reason; };
    QVector<Suggestion> allSuggestions = {
        {"步行", "腿部、臀部", "有氧", "低强度有氧运动，促进血液循环"},
        {"跑步", "腿部、心肺", "有氧", "高效燃脂，增强心肺功能"},
        {"游泳", "全身、心肺", "有氧", "全身运动，关节压力小"},
        {"骑行", "腿部、心肺", "有氧", "低冲击有氧，保护膝关节"},
        {"瑜伽", "全身、柔韧性", "柔韧", "提升柔韧性和平衡感"},
        {"拉伸", "全身、柔韧性", "柔韧", "缓解肌肉紧张，预防受伤"},
        {"俯卧撑", "胸部、手臂", "力量", "增强上肢推力"},
        {"深蹲", "腿部、臀部", "力量", "增强下肢力量"},
        {"平板支撑", "核心、腹部", "力量", "增强核心稳定性"},
        {"仰卧起坐", "腹部", "力量", "强化腹肌力量"},
        {"引体向上", "背部、手臂", "力量", "增强上肢拉力"},
        {"太极拳", "全身、平衡", "柔韧", "中医养生，调和气血"},
    };

    QJsonArray suggestions;
    for (const auto& s : allSuggestions) {
        if (doneCategories.contains(s.category) && doneBodyParts.contains(s.bodyPart.split("、")[0].trimmed())) {
            continue;
        }

        QJsonObject obj;
        obj["type"] = s.type;
        obj["bodyPart"] = s.bodyPart;
        obj["category"] = s.category;
        obj["reason"] = s.reason;
        suggestions.append(obj);

        if (suggestions.size() >= 5) break;
    }

    if (suggestions.isEmpty()) {
        QJsonObject obj;
        obj["type"] = "散步";
        obj["bodyPart"] = "腿部";
        obj["category"] = "有氧";
        obj["reason"] = "轻松活动，保持身体活力";
        suggestions.append(obj);
    }

    return suggestions;
}

void runPage::onConfirmClicked()
{
    QString text = ui->textEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入今日运动情况");
        return;
    }

    QJsonArray doneExercises = analyzeExercises(text);
    QJsonArray suggestedExercises = suggestExercises(doneExercises);

    qDebug() << "[RunPage] done:" << doneExercises;
    qDebug() << "[RunPage] suggested:" << suggestedExercises;

    emit exerciseAnalyzed(doneExercises, suggestedExercises);
}

void runPage::onBackClicked()
{
    emit backToMain();
}
