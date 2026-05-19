#include "secwindow.h"
#include "ui_secwindow.h"

secWindow::secWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::secWindow)
{
    ui->setupUi(this);
    ui->statusbar->setVisible(false);
    secScreen = new CustomGraphicsView(ui->graphicsView, this);
    setCentralWidget(ui->graphicsView);
}

secWindow::~secWindow()
{
    delete ui;
}
