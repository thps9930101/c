#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QScreen>
#include <QMainWindow>
#include <QProcess>
#include <QInputDialog>
#include <QTimer>
#include <QRadioButton>
#include <QPushButton>
#include <iostream>
#include <locale>
#include <string>

#include "mode/buffer/AudioWaveformWidget.h"
#include <QAudioInput>
#include <QAudioFormat>

#include "Data/Define/DefineData.h"
#include "Data/Struct/AppSettings/appSettings.h"
#include "Data/Enum/appStatus.h"
#include "Data/Enum/SP_Rot.h"

#include "element/QQrcode/QrCodeGenerator.h"

#include "element/allsetting.h"
#include "element/FunctionDock.h"
#include "element/aocamsetting.h"
#include "element/camsetting.h"
#include "element/CameraScan.h"
#include "element/EncoderSetting.h"
#include "element/customgraphicsview.h"
#include "element/tooldock.h"
#include "element/secwindow.h"

#include "element/paramdockbase.h"

#include "mode/ao_cameram_fun.h"

#include "utils/customqthread.h"
#include "utils/Logger.h"
#include <chrono>
#include <codecvt>


//!!!
using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ///INIT
    QString readFileToString(QString name);
    void readParamFile();
    void Initalize();
    void InitalizeUI();
    void connectSignals();
    void autoStart();
    void wirteParamFile(AppSettings set = AppSettings(), QString fileName = "");
    void camera_Init();

    //setting & stream
    void camera_Setting();
    void bt_stream_Click(QVector<CamInfo>);
    void scanButtonClick();

    bool streamStart(QVector<CamInfo> List);
    bool streamStop();

    void updateScreen();
    void setScale(CustomGraphicsView* CustomView, QGraphicsView* graphicsView);

    //TODO 把signal接上
    void setView_SP(std::vector<uint32_t>);

    //functionDock slot
    void onCameraNumberChange(int);
    void onCameraDelete(int);
    void onCameraStop(int);
    void onCameraStart(int);
    void onCameraAllDelete();

    void onModeChange(const Show_Mode );




    void setStatus(AppStatus stat)
    {
        status = stat;
    }

    /**
     * @brief 非同步取得掃描結果
     */
    void scanFinished(QVector<CamInfo>);

    void addAllCam(QVector<CamInfo>);
    void addCam(CamInfo);
    void deleteAllCam();
    void deleteCam(CamInfo);

    void camrea_release();

    //tool
    int checkFPS_string(QString str)
    {

        // 檢查字串是否足夠長
        if (str.length() >= 3)
        {
            // 刪除前三個字元
            QString trimmedString = str.mid(3);

            // 檢查剩下的字串是否為有效的數據（非空）
            if (!trimmedString.isEmpty())
            {
                return trimmedString.toInt();
            }
            else
            {
                qDebug() << "Error: Trimmed string is empty.";
                return -1;
            }
        }
        else
        {
            qDebug() << "Error: String is too short.";
            return -2;
        }
    }

    void setCurrentSelect(QString currentFile);

    void RTSP_Clicked();

    //tool
    void showMessage(QString, QString);

    //record
    void Screen_Record(int,bool);
    void Screen_Rotate();

    void set_SPID(std::vector<QString>);
    void start_Timer();

    void getkeyboard(int);
    QWidget* createRedDot();
    QString getLastProject();
protected:
    // 事件过滤器
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    /// Class
    FunctionDock *functionDock;
    ToolDock *toolDock;
    CustomGraphicsView *Screen;     /*!< GraphicsView */
    QList<secWindow *> SecondScreen;

    FrameThread *setth = nullptr;      /*!< [執行緒]即時影像刷新 */
    Allsetting *allSetting;
    CamSetting* camSetting;
    AoCamSetting* aoCamSetting;
    CameraScan *camScan;
    CamVC610_WebAPI_Ctrl apiCtrl;
    EncoderSetting* enc_set;
    ao_cameram_fun Cam_Fun;         /*!< cameraFunc */
    Logger logger = Logger("log.txt");
    QVector<CamInfo> camList;

    /// Struct
    AppSettings appSettings;

    /// Bool
    bool isShowTime = false;
    bool isReadParamFile = false;   /*!< 是否已初始化參數*/
    bool isRTSP_OPEN = false;       /*!< 是否正在錄影 */
    bool Stream_first = false;      /*!< 是否成功取得畫面 */

    std::mutex Lock;
    AppStatus status;

    _time start_time;
    _time end_time;
    milli_type DC_cost_time;

    QString currentSettingsFile = "";
    QImage* img = new QImage();

    //test
    int angle;
    bool file_open =false;
    QString record_initialPath = QDir::homePath();
    std::vector<uint32_t> SPID_list;
    Show_Mode show_mode;
    int ScreenNum;
    bool fullScreen = false;
    bool add_secScreen = false;

    QTimer* timer = new QTimer();
    int time;
    int sec;
    int table_line_height;
    int key;
    QString masterCam = "";

    QLabel* redDot;
};
#endif // MAINWINDOW_H
