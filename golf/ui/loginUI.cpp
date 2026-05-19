#include "loginUI.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QFont>

namespace LoginUI {

void setupLoginUI(QDialog *dialog, QLineEdit *&username, QLineEdit *&password, QPushButton *&loginButton)
{
    dialog->setWindowFlags(Qt::FramelessWindowHint);  // 無邊框
    dialog->setStyleSheet("background-color: #333333;");
    dialog->showFullScreen();

    // 登入容器
    QFrame *loginBox = new QFrame;
    loginBox->setStyleSheet("background-color: white; border-radius: 15px;");
    loginBox->setFixedSize(600, 450);  // ⬅️ 放大一點（寬、高）

    // 標題
    QLabel *title = new QLabel("Login");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(32);   // ⬅️ 更大
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: black;");

    // 帳號欄位
    username = new QLineEdit;
    username->setPlaceholderText("Username");
    username->setStyleSheet("background-color: #f0f9ff; color: black; padding: 15px; border: none; border-radius: 6px;");
    username->setFont(QFont("Arial", 16));   // ⬅️ 字體加大
    username->setFixedHeight(50);            // ⬅️ 高度加大

    // 密碼欄位
    password = new QLineEdit;
    password->setPlaceholderText("Password");
    password->setEchoMode(QLineEdit::Password);
    password->setStyleSheet("background-color: #f0f9ff; color: black; padding: 15px; border: none; border-radius: 6px;");
    password->setFont(QFont("Arial", 16));   // ⬅️ 字體加大
    password->setFixedHeight(50);            // ⬅️ 高度加大

    // 登入按鈕
    loginButton = new QPushButton("Sign In");
    loginButton->setStyleSheet("background-color: #0b3d56; color: white; padding: 15px; border-radius: 6px;");
    loginButton->setFont(QFont("Arial", 16));   // ⬅️ 字體加大
    loginButton->setFixedHeight(55);            // ⬅️ 高度加大

    // 排版
    QVBoxLayout *boxLayout = new QVBoxLayout(loginBox);
    boxLayout->addWidget(title);
    boxLayout->addWidget(username);
    boxLayout->addWidget(password);
    boxLayout->addWidget(loginButton);
    boxLayout->setSpacing(30);                  // ⬅️ 間距加大
    boxLayout->setContentsMargins(50, 40, 50, 40);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addStretch();
    mainLayout->addWidget(loginBox, 0, Qt::AlignCenter);
    mainLayout->addStretch();
}

}
