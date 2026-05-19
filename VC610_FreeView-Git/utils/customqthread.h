#ifndef CUSTOMQTHREAD_H
#define CUSTOMQTHREAD_H


#include <QMessageBox>
#include <QPainter>
#include <QThread>
#include <QMainWindow>
#include <QFileDialog>

#include <chrono>

using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;


typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;


/**
 * @brief 取得畫面執行緒
 */
class FileThread: public QThread
{
    Q_OBJECT

signals:
    /**
     * @brief 根據FPS速率，傳送信號
     */
    void onOpen(QStringList fileNames);

public:

    /**
     * @brief 初始化執行緒
     * @param FPS 每秒傳送信號次數
     */
    FileThread(QWidget* parent, QString title, QString filter);

    /**
     * @brief 執行緒解構式
     */
    ~FileThread();

    /**
     * @brief [emit]根據FPS速率，傳送信號
     */
    void run() override;

private:
    QWidget* parent;
    QString title;
    QString filter;
};

/**
 * @brief 取得畫面執行緒
 */
class FrameThread: public QThread
{
    Q_OBJECT

signals:
    /**
     * @brief 根據FPS速率，傳送信號
     */
    void refreash();

public:

    /**
     * @brief 初始化執行緒
     * @param FPS 每秒傳送信號次數
     */
    FrameThread(int FPS);

    /**
     * @brief 執行緒解構式
     */
    ~FrameThread();

    /**
     * @brief [emit]根據FPS速率，傳送信號
     */
    void run() override;

    /**
     * @brief 設定執行緒是否關閉
     * @param bln 執行緒開關
     */
    void setStart(bool bln);

    void setFPS(int fps);
private:
    int sleepMs;            /*!< 每隔?毫秒傳送信號 */
    bool isStart = true;    /*!< 取得畫面執行緒flag */
};

/**
 * @brief Loading 元件
 */
class RotatingWidget : public QWidget
{
public:
    /**
     * @brief Loading 元件
     * @param parent 父控制項
     */
    RotatingWidget(QWidget *parent = nullptr);

    void setEnable(bool);
protected:
    /**
     * @brief 更新動畫畫面
     */
    void paintEvent(QPaintEvent *) override;

    /**
     * @brief 每秒更新旋轉角度
     */
    void timerEvent(QTimerEvent *) override;

private:
    qreal m_rotationAngle = 0;      /*!< 目前旋轉角度 */
    int loaderWidth;                /*!< 動畫寬度 */
    bool isEnable = false;  

};


#endif // CUSTOMQTHREAD_H
