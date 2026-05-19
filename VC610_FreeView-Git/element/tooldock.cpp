#include "tooldock.h"
#include "ui_tooldock.h"

ToolDock::ToolDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ToolDock)
{
    ui->setupUi(this);
    InitalizeUI();
}

ToolDock::~ToolDock()
{
    delete ui;
}

void ToolDock::InitalizeUI()
{
    ui->combo_mode->addItem("Single Mode");
    ui->combo_mode->addItem("Multi-View Mode");
}

/// element
void ToolDock::on_bt_scan_clicked()
{
    emit tool_scanButtonClick();
}

void ToolDock::on_bt_stream_clicked()
{
    emit StreamSignal();
}


void ToolDock::on_bt_record_clicked()
{
    StartRecord();
}

void ToolDock::StartRecord()
{
    int ret = 0;

//    if(start_record)
//    {
//        ui->bt_record->setText("Start Recording");

//    }else
//    {
//        ui->bt_record->setText("Stop Recording");

//    }

    emit record_Signal(ret,start_record);
    start_record = !start_record;
}

void ToolDock::setAppSetting(AppSettings* set)
{
    appSettings = set;
    mode = appSettings->aoFunSetting.mode;
    ui->combo_mode->setCurrentIndex(mode);

}


void ToolDock::on_bt_RTSP_clicked()
{
    ui->bt_RTSP->setEnabled(false);
    emit tool_rtsp_Signal();
}

void ToolDock::on_bt_rotate_clicked()
{
    emit tool_rotate_Signal();
}


void ToolDock::on_combo_mode_currentIndexChanged(int index)
{
    emit tool_combo_change(index);
}

QObject* ToolDock::selectUIElement(UI_ToolDock funDuck)
{
    switch (funDuck) {
    case tool_allDown:
    case tool_init:
        break;

    case tool_bt_scan:
        return ui->bt_scan;

    case tool_bt_stream:
        return ui->bt_stream;

    case tool_bt_RTSP:
        return ui->bt_RTSP;

    case tool_bt_record:
        return  ui->bt_record;

    case tool_combo_mode:
        return  ui->combo_mode;

    case tool_bt_rotate:
        return  ui->bt_rotate;

    case tool_rb_single:
        return ui->rb_single;

    case tool_rb_multi:
        return  ui->rb_multi;

    }
}


void ToolDock::setButton_Disable(UI_ToolDock funDuck, bool bl)
{
    switch (funDuck) {
    case tool_allDown:
        ui->bt_scan->setDisabled(true);
        ui->bt_stream->setDisabled(true);
        ui->bt_RTSP->setDisabled(true);
        ui->bt_record->setDisabled(true);
        ui->bt_rotate->setDisabled(true);
        return;
    case tool_init:
        ui->bt_scan->setDisabled(false);
        ui->bt_stream->setDisabled(true);
        ui->bt_RTSP->setDisabled(true);
        ui->bt_record ->setDisabled(true);
        ui->bt_rotate->setDisabled(true);
        return;
    }

    QPushButton* temp = qobject_cast<QPushButton*>(selectUIElement(funDuck));
    if (temp) {
        temp->setDisabled(bl);
    }
    QComboBox* cotemp = qobject_cast<QComboBox*>(selectUIElement(funDuck));
    if (cotemp) {
        cotemp->setDisabled(bl);
    }
    QRadioButton* rbtemp = qobject_cast<QRadioButton*>(selectUIElement(funDuck));
    if (rbtemp) {
        rbtemp->setDisabled(bl);
    }
}

void ToolDock::setButton_Text(UI_ToolDock funDuck, QString text)
{
    switch (funDuck) {
    case tool_allDown:
        ui->bt_scan->setDisabled(true);
        ui->bt_stream->setDisabled(true);
        return;
    case tool_init:
        ui->bt_scan->setDisabled(false);
        ui->bt_stream->setDisabled(true);
        return;
    case tool_bt_record:
        start_record =true;
        ui->bt_record->setText(text);
        return;
    }

    QPushButton* temp = qobject_cast<QPushButton*>(selectUIElement(funDuck));
    if (temp) {
        temp->setText(text);
    }
}

void ToolDock::RTSP_Change(bool isSusccess, bool isOpen) {
    ui->bt_RTSP->setEnabled(true);
    if (!isSusccess)
        return;

    if (isOpen)
        ui->bt_RTSP->setText("Stream Stop");
    else
        ui->bt_RTSP->setText("Stream Start");
}

void ToolDock::recordMode()
{
//    if(ui->combo_mode->currentIndex() == 0)
//    {
//        ui->pb_StartRecord->setEnabled(false);
//    }else
//    {
//        ui->pb_StartRecord->setEnabled(true);
//    }

    //ui->bt_record->setEnabled(true);
}
