#ifndef TOOLDOCK_H
#define TOOLDOCK_H

#include <QDockWidget>

#include "Data/Enum/appStatus.h"
#include "element/CameraScan.h"
#include "Data/Struct/AppSettings/AppSettings.h"

namespace Ui {
class ToolDock;
}

enum UI_ToolDock{
    tool_allDown = -1,
    tool_init = 0,
    tool_bt_scan,
    tool_bt_stream,
    tool_bt_startRecord,
    tool_bt_stopRecord,
    tool_bt_RTSP,
    tool_bt_record,
    tool_combo_mode,
    tool_bt_rotate,
    tool_rb_single,
    tool_rb_multi
};

class ToolDock : public QDockWidget
{
    Q_OBJECT

signals:


    void tool_scanButtonClick();
    void scanSignal(QVector<CamInfo>);
    void tool_rtsp_Signal();
    void StreamSignal();
    void record_Signal(int ,bool);
    void tool_rotate_Signal();
    void tool_combo_change(int);


public:
    explicit ToolDock(QWidget *parent = nullptr);
    ~ToolDock();
    void StartRecord();
    void setAppSetting(AppSettings* set);
    void InitalizeUI();
    QObject* selectUIElement(UI_ToolDock);
    void setButton_Disable(UI_ToolDock, bool = false);
    void setButton_Text(UI_ToolDock, QString);
    void RTSP_Change(bool isSusccess, bool isOpen);
    void recordMode();

    bool start_record = false;
    int mode;

private slots:
    void on_bt_scan_clicked();

    void on_bt_stream_clicked();

    void on_bt_record_clicked();


    void on_bt_RTSP_clicked();

    void on_bt_rotate_clicked();

    void on_combo_mode_currentIndexChanged(int index);

private:
    Ui::ToolDock *ui;
    AppSettings* appSettings = nullptr;

};

#endif // TOOLDOCK_H
