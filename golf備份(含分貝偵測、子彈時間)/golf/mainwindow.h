#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QList>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QListWidget>
#include <QMouseEvent>
#include <QDir>
#include <QTimer>
#include <QLabel>
#include <QPixmap>
#include <QMutex>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QScreen>
#include <QOpenGLWidget>
#include <QPointer>              // ★ 一定要有

#include "pressevent.h"
#include "login.h"
#include "setting.h"
#include "VideoDecoder.h"
#include "FrameData.h"
#include "Slider.h"
#include "ToastMessage.h"
#include "playthread.h"
#include "login.h"   // 需要用到你的 Login 對話框

#include <QJsonArray>
#include <QJsonObject>
#include <QThread>
#include <QWaitCondition>
#include <QPropertyAnimation>
#include <QElapsedTimer>

#include "mode/ao_cameram_fun.h"
#include "Data/Struct/CamInfo/CamInfo.h"
#include "Data/Enum/SP_Rot.h"
#include "Data/Enum/Drag_mode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    PressEvent *eventHandler;
    QJsonObject imageObj;
    QJsonObject settingJson;
    setting *settingDialog;

    void initConnect();

    void setupPressEvent();  //按鍵事件初始化
    void setupGlobalShortcuts(); // 新增這個 function
    void loadVideosFromPath(const QString &);
    void InitUI();
    bool DecoderAndShow(QString);
    void startMonitoring();
    void updateVideoList(const QStringList&);
    void showSpeedOverlay(double);
    void showOverlayText(const QString&, int fontSize, QColor);
    void showLoading(bool, const QString& = "載入中...");
    void showSeekOverlay(bool, int, double);
    void apiResController(QString, bool, const QJsonObject& response = QJsonObject());
    // 函式宣告
    void adjustUIPositions();
    void setupGraphicsView();
    void setupCornerButton();
    void setupProgressSlider();
    void setupPlayPauseButton();
    void Cam_Fun_init();
    void setCamList(QList<QString>);
    void startCamList();
    void show_live_frame();
    void setVideoTool(bool);
    void rotateAndFitImage(QGraphicsPixmapItem*, QGraphicsScene*);
    void autoLogin(QJsonObject);

    void createCountdownOverlay();  // 建立遮罩與倒數物件
    void updateCountdown();
    void updateOverlayPosition();   // 視窗大小改變時自適應
    void startRec();
    void startCountdown(int, int);
    int updateTimerInterval();
    void setRefreshIcon(QPushButton*);
    void startPlayback();
    void stopPlayback();

    //----------


    QGraphicsScene* scene = nullptr;
    QGraphicsPixmapItem* imageItem = nullptr;
    QList<QString> ipList;

    QTimer* LiveTimer;
    QTimer* VideoTimer;

    VideoDecoder *decoder;
    QImage currentImage;
    QThread* decodeThread = nullptr;
    bool imageReady = false;
    QList<QImage> frameBuffer;
    int currentFrameIndex = 0;
    bool decodingFinished = false;
    QMutex bufferMutex;
    QSize targetResolution = QSize(1920, 1080);  // 預設解析度（可改）

    QListWidget* videoListWidget;
    QThread* monitorThread = nullptr;
    QString monitoredPath;
    FrameData FrameData;
    bool isStream = false;
    bool isRecord = false;

    ApiClient *apiClient;
    //----UI----
    // 新增
    bool isPlaying = false; // 目前是否在播放
    bool wasPlayingBeforeDrag = false;
    QPushButton* cornerButton;
    QSlider* progressSlider = nullptr;
    QPushButton* playPauseButton = nullptr;

    QWaitCondition frameReady;
    QMutex frameMutex;

    ao_cameram_fun Cam_Fun;         /*!< cameraFunc */
    QVector<CamInfo> camList;
    std::vector<uint32_t> SPID_list;
    Show_Mode show_mode;
    SP_MODE SP_mode;

    // 視覺元件
    QGraphicsRectItem* overlayMask = nullptr;      // 黑色倒數遮罩
    QGraphicsTextItem* countdownText = nullptr;    // 倒數文字
    QGraphicsRectItem* recordingBorder = nullptr;  // 錄影紅色邊框
    QGraphicsTextItem* doneText = nullptr;         // Done 提示

    // 計時器
    QTimer* countdownTimer = nullptr;              // 倒數計時器
    QTimer* recordingTimer = nullptr;              // 錄影計時器

    // 狀態
    int countdownValue = 3;                        // 倒數剩餘秒數
    int recordingDuration = 10;                     // 錄影秒數
    int recordingElapsed = 0;                      // 錄影已經過秒數

    QPoint lastMousePos;   // 上一次滑鼠位置
    int accumulatedMove = 0; // 累積的Y位移
    bool isDragging = false; // 是否正在按住滑動
    double playbackSpeed = 1.0; // 1.0 = 正常速度，2.0 = 兩倍速，0.5 = 半速
    double framePosition = 0.0; // 用來儲存浮點幀位置
    double playbackPosition = 0.0; // 成員變數
    QGraphicsTextItem* speedOverlayText = nullptr;  // 倍速提示文字
    QLabel* overlayLabel = nullptr;   // 疊在 graphicsView 上的文字 (暫時訊息)

    QGraphicsRectItem* loadingBackground = nullptr;
    QGraphicsTextItem* loadingText = nullptr;

    PlayThread* playThread = nullptr;
    QString timeStr;
    bool isVideo = false;
    QLabel* loadingLabel = nullptr;
    QString lastClickedFile;
    DragMode dragMode = None;
    QLabel* seekLabel = nullptr;

    bool isLoading = false;
    bool  m_viewRotated = false;
    QString currentVideoPath;   // 目前已載入/正在載入的影片
    bool openingVideo = false; // 防止同時重入

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void showNextFrame();
    void on_live_bt_clicked();
    void on_re_bt_clicked();
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
