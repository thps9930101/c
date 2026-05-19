#include "login.h"
#include "ui_login.h"
#include "jsonfilemanager.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    LoginUI::setupLoginUI(this, username, password, loginButton);
    settingJson = JsonFileManager::readConfig();
    loginObj = settingJson["login"].toObject();

    connect(loginButton, &QPushButton::clicked, this, &Login::handleLogin);
//    ui->setupUi(this);

    if(loginObj["auto"].toBool())
    {
        autoLogin(loginObj);
    }
//    ui->setupUi(this);              // 只顯示 login.ui

}

void Login::handleLogin()
{
    // 簡易驗證，可自行擴充
    if (username->text() == "test" && password->text() == "1234") {
        accept();  // 關閉登入，進入主畫面
    } else {
        ToastMessage::showMessage(this, "登入失敗", "帳號密碼輸入錯誤");

        username->clear();
        password->clear();
        // TODO: 加上錯誤提示
    }
}

void Login::autoLogin(QJsonObject set)
{
    username->setText(set["account"].toString());
    password->setText(set["password"].toString());

    // 延遲 1000ms 後觸發點擊
    QTimer::singleShot(1000, this, [this]() {
        loginButton->click();
    });
}

Login::~Login()
{
    delete ui;
}
