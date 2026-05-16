#include "logindialog.h"
#include "mainpage.h"
#include "eatwhat.h"
#include "foodlist.h"
#include "realeat.h"
#include "today.h"
#include "tourist.h"
#include "constitution.h"
#include "runpage.h"
#include "showrun.h"
#include "api_client.h"

#include <QApplication>
#include <QStackedWidget>

class AppNavigator : public QObject
{
    Q_OBJECT

public:
    AppNavigator(QStackedWidget* stack) : stack_(stack) {
        login_ = new LoginDialog();
        mainPage_ = new MainPage();
        eatWhat_ = new eatwhat();
        foodList_ = new FoodList();
        realEat_ = new RealEat();
        todayPage_ = new Today();
        touristPage_ = new tourist();
        constitutionPage_ = new ConstitutionWidget();
        runPage_ = new runPage();
        showRunDlg_ = new showRun();

        stack_->addWidget(login_);
        stack_->addWidget(mainPage_);
        stack_->addWidget(eatWhat_);
        stack_->addWidget(foodList_);
        stack_->addWidget(realEat_);
        stack_->addWidget(todayPage_);
        stack_->addWidget(touristPage_);
        stack_->addWidget(constitutionPage_);
        stack_->addWidget(runPage_);

        connect(login_, &LoginDialog::loginSuccessful, this, &AppNavigator::onLoginSuccess);
        connect(login_, &LoginDialog::touristModeSelected, this, &AppNavigator::onTouristSelected);

        connect(touristPage_, &tourist::sessionStarted, this, &AppNavigator::onTouristSessionStarted);

        connect(mainPage_, &MainPage::navigateToEatWhat, this, [this]() { stack_->setCurrentWidget(eatWhat_); });
        connect(mainPage_, &MainPage::navigateToToday, this, [this]() { stack_->setCurrentWidget(todayPage_); });
        connect(mainPage_, &MainPage::navigateToRealEat, this, [this]() { stack_->setCurrentWidget(realEat_); });
        connect(mainPage_, &MainPage::navigateToConstitution, this, [this]() { stack_->setCurrentWidget(constitutionPage_); });
        connect(mainPage_, &MainPage::navigateToRunPage, this, [this]() { stack_->setCurrentWidget(runPage_); });

        connect(eatWhat_, &eatwhat::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });
        connect(foodList_, &FoodList::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });
        connect(realEat_, &RealEat::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });
        connect(realEat_, &RealEat::navigateToToday, this, [this]() { stack_->setCurrentWidget(todayPage_); });
        connect(todayPage_, &Today::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });
        connect(constitutionPage_, &ConstitutionWidget::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });

        connect(runPage_, &runPage::backToMain, this, [this]() { stack_->setCurrentWidget(mainPage_); });
        connect(runPage_, &runPage::exerciseAnalyzed, this, [this](const QJsonArray& done, const QJsonArray& suggested) {
            showRunDlg_->setData(done, suggested);
            showRunDlg_->exec();
        });
        connect(showRunDlg_, &showRun::accepted, this, [this]() {
            stack_->setCurrentWidget(mainPage_);
        });

        connect(&ApiClient::getInstance(), &ApiClient::unauthorized, this, &AppNavigator::onUnauthorized);

        stack_->setCurrentWidget(login_);
    }

private slots:
    void onLoginSuccess(const QJsonObject&) {
        stack_->setCurrentWidget(mainPage_);
    }

    void onTouristSelected(const QJsonObject&) {
        stack_->setCurrentWidget(touristPage_);
    }

    void onTouristSessionStarted(const QJsonObject&) {
        stack_->setCurrentWidget(mainPage_);
    }

    void onUnauthorized() {
        stack_->setCurrentWidget(login_);
        login_->show();
    }

private:
    QStackedWidget* stack_;
    LoginDialog* login_;
    MainPage* mainPage_;
    eatwhat* eatWhat_;
    FoodList* foodList_;
    RealEat* realEat_;
    Today* todayPage_;
    tourist* touristPage_;
    ConstitutionWidget* constitutionPage_;
    runPage* runPage_;
    showRun* showRunDlg_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("每日饮食推荐");
    a.setOrganizationName("DailyDiet");

    ApiClient::getInstance().setBaseUrl("http://localhost:8888");

    QStackedWidget stack;
    stack.resize(600, 700);

    AppNavigator navigator(&stack);

    stack.show();
    return a.exec();
}

#include "main.moc"
