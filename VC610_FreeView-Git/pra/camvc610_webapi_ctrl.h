#ifndef CAMVC610_WEBAPI_CTRL_H
#define CAMVC610_WEBAPI_CTRL_H

#include <string>
#include <QString>
#include <sstream>
#include <qDebug>
#include <QDateTime>
#include <qthread.h>
#include <QNetworkInterface>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <curl/curl.h>
#include "Data/Struct/CamInfo/CamInfo.h"
#include "Data/Struct/Cam_Set/Cam_Set.h"

#include <iostream>

enum state
{
    unInvalid   = -4,
    undefind    = -3,
    unknow      = -2,
    error       = -1,
    inProgress  = 0,
    done        = 1
};

class CamVC610_WebAPI_Ctrl: public QObject
{
    Q_OBJECT

public:
    CamVC610_WebAPI_Ctrl();
    /// !-- 設定IP -- ///
    void setIP(QString ip);

    int get_ProgressID()
    {
        return id;
    }

    void setListDatetime(QVector<CamInfo>);

    int setDatetime();
    int checkState(QString);
    /// !-- 根據StatusID 查設定狀態 -- ///
    int checkStatus(int StatusID);


    /// !-- 設定參數 -- ///
    int settingParam1(QString key, QString value);
    int settingParam2(QString key, int value);

    Cam_Set getParam();
    void autoScanCamIP();
    void scanCamIP_Range(QString ip, int start,int end);
    int scanCamIP(QString ip);
    /// !-- 儲存設定 -- ///
    int saveParam();

signals:
    void scanFinish(QVector<CamInfo> list);
    void setListFinish();
    void setListDateTimeFinish();
    void masterCam_signal(QString);
private:


    /// !-- 發送API -- ///
    std::string send_url_API(QString IP, std::string action,std::string json);


    std::string get_url(QString IP, std::string action);
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

    /// !-- 取得StatusID -- ///
    int getStatusID(std::string resultString);
    int isStreaming(std::string resultString);
    int isOperationDone(std::string resultString);
    QJsonObject stdStringToQJsonObject(std::string);

    QString getCurrentDateTime();

    std::mutex mtx;
    int count = 0;
    int countMax = 0;
    QVector<CamInfo> Camlist;

    QString head = "http://";
    QString ip   = "";
    QString api  = "/ctrl/";
    int id = -1;
    bool isScan = false;

};

#endif // CAMVC610_WEBAPI_CTRL_H
