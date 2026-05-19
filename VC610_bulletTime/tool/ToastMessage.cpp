#include "ToastMessage.h"
#include <QApplication>

ToastMessage::ToastMessage(QWidget *parent, const QString &title, const QString &message)
    : QDialog(parent)
{
    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
    setModal(true);
    resize(350, 200);

    // 對話框背景白色
    this->setStyleSheet("background-color: white;");


    QFont font("Microsoft JhengHei");  // 微軟正黑體


    label = new QLabel(message, this);
    label->setWordWrap(true);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 20px; color: black; background-color: transparent;");

    okButton = new QPushButton("確定", this);
    okButton->setFont(font);
    okButton->setFixedSize(120, 40);  // 寬120，高50
    okButton->setStyleSheet(R"(
        QPushButton {
            border: 2px solid black;
            color: black;
            background-color: white;
            font-size: 18px;
            padding: 8px 20px;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #f0f0f0;
        }
        QPushButton:pressed {
            background-color: #d0d0d0;
        }
    )");

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(label, 1, Qt::AlignCenter);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);
}

void ToastMessage::showMessage(QWidget *parent, const QString &title, const QString &message)
{
    ToastMessage msg(parent, title, message);
    msg.exec();
}
