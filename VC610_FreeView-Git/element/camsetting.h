#ifndef CAMSETTING_H
#define CAMSETTING_H

#include <QMainWindow>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>
#include <unordered_map>
#include <any>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "../pra/camvc610_webapi_ctrl.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CamSetting; }
QT_END_NAMESPACE

struct Cam_UI_Setting
{
    //Video
    QStringList resolutionList;
    QStringList frameRateList;
    struct Range{
        int min;
        int max;
    };
    Range gopRange;
    QStringList qualityList;

    //Image
    QStringList exposureTimeList;
    QStringList isoList;
    QStringList evList;
    QStringList aeAreaList;
    QStringList wdrList;
    QStringList wbList;
    Range wbCT;

    Cam_UI_Setting(){//QStringList({});
        //Video
        resolutionList = QStringList({"1920x1080","3840x2160"});
        frameRateList = QStringList({"FPS30","FPS60"});
        gopRange = Range({2,60});
        qualityList = QStringList({"Low","Middle","High"});
        //Image
        exposureTimeList = QStringList({
            "auto", "1/6400", "1/5000", "1/4000", "1/3200", "1/2500", "1/2000", "1/1600",
            "1/1250", "1/1200", "1/1000", "1/800", "1/640", "1/600", "1/500", "1/400",
            "1/320", "1/250", "1/240", "1/200", "1/160", "1/125", "1/120", "1/110",
            "1/100", "1/80", "1/60", "1/55", "1/50", "1/48", "1/40", "1/30"
        });
        isoList = QStringList({
            "auto", "100", "200", "400", "800", "1600", "3200", "6400"
        });
        evList = QStringList({
            "-3.0", "-2.7", "-2.3", "-2.0", "-1.7", "-1.3", "-1.0",
            "-0.7", "-0.3", "0.0", "+0.3", "+0.7", "+1.0", "+1.3", "+1.7",
            "+2.0", "+2.3", "+3.0"
        });
        aeAreaList = QStringList({
            "center", "average"
        });
        wdrList = QStringList({
            "off", "on"
        });
        wbList = QStringList({
            "auto", "daylight", "cloudy", "fluorescent", "incandescent", "fluorescentCWF", "customer"
        });
        wbCT = Range({2800, 9000});
    }
};


class CamSetting : public QMainWindow
{
    Q_OBJECT

signals:
    void cantGetResult();
    void Btn_Save_setDisable(bool);
    void getResult(Cam_Set);

public:
    CamSetting(QWidget *parent = nullptr);
    ~CamSetting();
    /** @brief 初始化UI */
    void InitalizeUI();
    void connectSignals();



    /** @brief 取得UI設定 */
    Cam_Set get_UI_set();
    /** @brief 設定UI畫面 */
    void setUIValue(Cam_Set set, bool setIP = true);
    void set_VideoSettings_Disable(bool);
    void set_ImageSettings_Disable(bool);

    bool setDisableByIP(QString);

    /// Param Func
    //IP
    /** @brief 判斷IP是否合理 */
    bool isValidIPv4(const QString &ip);
    /** @brief 將IP拆開 */
    QStringList splitIPv4(const QString &ip);
    /** @brief 組合IP */
    QString packIP();
    void showMessage(QString, QString);

    ///當作為獨立元件時或許會用到*****************
    /** @brief 判斷參數資料夾是否存在 */
    void checkDir();
    /** @brief 讀取參數 */
    Cam_Set readParam();
    /** @brief 寫入參數 */
    void writeParam(Cam_Set set = Cam_Set());
    ///***************************************

    QString readFileToString(QString name);
    void wirteParamFile(QJsonObject content, QString fileName);

signals:
    void returnParam(Cam_Set set);

private:

private slots:
    void on_bt_getSet_clicked();
    void on_bt_save_clicked();
    void on_bt_cs_clicked();    
private:
    Ui::CamSetting *ui;
    QWidget* content;
    CamVC610_WebAPI_Ctrl *wabAPI = new CamVC610_WebAPI_Ctrl();

    //param
    bool isReadParamFile = false;
    bool needSetWBCT = false;
    bool isStreaming = false;
    Cam_Set setting;
    QString ResourceDir = "./Resource";
    QString Param_file = "./Resource/camSet.camset";

    QStringList IPAddress;


};
#endif // CAMSETTING_H
