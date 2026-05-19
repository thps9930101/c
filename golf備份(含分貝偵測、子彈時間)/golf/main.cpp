#include "mainwindow.h"
#include <Windows.h>
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile theme_file("./qss/template.qss");
    theme_file.open(QFile::ReadOnly);   //open theme file

    if(theme_file.isOpen())
    {
        a.setStyleSheet(theme_file.readAll());
        theme_file.close();
    }
    else
    {
        qDebug("File couldn't be opened!");
    }
    Login loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow *w = new MainWindow();
        qDebug()<<"Show";
        w->showFullScreen();  // 全螢幕
        w->show();

    }

    return a.exec();  // 進入事件循環
}
