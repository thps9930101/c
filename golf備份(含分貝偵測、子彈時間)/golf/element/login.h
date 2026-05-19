#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>

#include "loginUI.h"
#include "ToastMessage.h"


namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private:
    Ui::Login *ui;

private slots:
    void handleLogin();
    void autoLogin(QJsonObject);

private:
    QLineEdit *username;
    QLineEdit *password;
    QPushButton *loginButton;
    QJsonObject settingJson;
    QJsonObject loginObj;

};

#endif // LOGIN_H
