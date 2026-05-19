#ifndef FUNCTIONDOCK_H
#define FUNCTIONDOCK_H

#include <QDockWidget>
#include <QStackedWidget>
#include <QDebug>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWidget>
#include <QThread>
#include <QModelIndex>
#include <QContextMenuEvent>
#include <QMenu>
#include <QObject>

#include "Data/Enum/appStatus.h"
#include "element/CameraScan.h"
#include "Data/Struct/AppSettings/AppSettings.h"

#include "mode/ao_cameram_fun.h"

enum UI_FunctionDock{
    allDown = -1,
    init = 0,
    bt_scan,
    bt_stream,
    bt_startRecord,
    bt_stopRecord,
    bt_RTSP,
    bt_record,
    combo_mode,
    bt_rotate
};

namespace Ui {
class FunctionDock;
}

class FunctionDock : public QDockWidget
{
    Q_OBJECT

signals:
    void onLiveCamNo_change(int);
    void onStreamModeChange(const Show_Mode);
    void onFPSChange(int);

    void scanButtonClick();
    void scanSignal(QVector<CamInfo>);
    void startButtonClick();
    void stopButtonClick();
    void StreamSignal(QVector<CamInfo>);
    void changeCam(int);

    void ondeleteCam(int index);
    void onstopCam(int index);
    void onstartCam(int index);
    void ondeleteAllCam();
    void rtsp_Signal();

    void record_Signal(int ,bool);
    void rotate_Signal();

public:
    explicit FunctionDock(QWidget *parent = nullptr);
    ~FunctionDock();

    void InitalizeUI();

    void setAppSetting(AppSettings* set);

    void setStatus(AppStatus stat)
    {
        status = stat;
    }

    void addCam(int num, const QString& camIP, const QString& status, const QString& mode);
    void deleteCam();
    void stopCam();
    void startCam();
    void deleteAllCam();

    void tableViewChange(int);

    void setTableView(QVector<CamInfo> list);
    void setBt_streamText(QString);

    QObject* selectUIElement(UI_FunctionDock);
    void setButton_Disable(UI_FunctionDock, bool = false);
    void setButton_Text(UI_FunctionDock ,QString );

    void RTSP_Change(bool isSusccess, bool isOpen);
    void recordMode();
    void StartRecord();
    void click_StreamButton();
    void change_mode(int);

private slots:
    void on_bt_scan_clicked();
    void on_bt_Start_clicked();
    void on_bt_Stop_clicked();
    void on_bt_stream_clicked();

    void showtableView(const QPoint& pos);
    void tableViewIndexOnChange(const QModelIndex &, const QModelIndex &);

    void on_combo_mode_currentIndexChanged(int index);

    void on_bt_RTSP_clicked();

    void on_spinBox_FPS_valueChanged(int arg1);

    void on_pb_StartRecord_clicked();

    void on_bt_rotate_clicked();

    void on_spinBox_FPS_textChanged(const QString &arg1);

public:
    int mode;


private:
    QWidget *parent;
    Ui::FunctionDock *ui;
    QStandardItemModel *model;
    AppStatus status;
    AppSettings* appSettings = nullptr;

    QAction *deleteItem;
    QAction *stopItem;
    QAction *startItem;

    int camCount = -1;
    bool start_record = false;

};

#endif // FUNCTIONDOCK_H
