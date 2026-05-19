#include "operationui.h"
#include "ui_operationui.h"

operationUI::operationUI(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::operationUI)
{
    ui->setupUi(this);
}

operationUI::~operationUI()
{
    delete ui;
}
