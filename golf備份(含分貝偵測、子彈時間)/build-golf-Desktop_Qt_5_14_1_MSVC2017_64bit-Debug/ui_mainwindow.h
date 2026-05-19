/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_4;
    QGraphicsView *graphicsView;
    QVBoxLayout *ListLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *live_bt;
    QPushButton *re_bt;
    QListWidget *listWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_4 = new QHBoxLayout(centralwidget);
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));

        horizontalLayout_4->addWidget(graphicsView);

        ListLayout = new QVBoxLayout();
        ListLayout->setObjectName(QString::fromUtf8("ListLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        live_bt = new QPushButton(centralwidget);
        live_bt->setObjectName(QString::fromUtf8("live_bt"));
        live_bt->setEnabled(false);
        live_bt->setMinimumSize(QSize(0, 40));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font.setPointSize(18);
        font.setBold(false);
        font.setWeight(50);
        live_bt->setFont(font);

        horizontalLayout->addWidget(live_bt);

        re_bt = new QPushButton(centralwidget);
        re_bt->setObjectName(QString::fromUtf8("re_bt"));
        re_bt->setMinimumSize(QSize(0, 40));
        re_bt->setSizeIncrement(QSize(0, 0));

        horizontalLayout->addWidget(re_bt);

        horizontalLayout->setStretch(0, 4);
        horizontalLayout->setStretch(1, 1);

        ListLayout->addLayout(horizontalLayout);

        listWidget = new QListWidget(centralwidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font1.setPointSize(12);
        listWidget->setFont(font1);
        listWidget->setSpacing(5);

        ListLayout->addWidget(listWidget);


        horizontalLayout_4->addLayout(ListLayout);

        horizontalLayout_4->setStretch(0, 4);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        live_bt->setText(QCoreApplication::translate("MainWindow", "Live", nullptr));
        re_bt->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
