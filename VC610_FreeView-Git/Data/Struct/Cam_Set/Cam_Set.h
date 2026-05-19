#ifndef CAM_SET_H
#define CAM_SET_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


struct Cam_Set{
    QString IP;
    QString videoResolution;
    QString videoFrameRate;
    int videoGop;
    QString videoBitRate;
    QString exposureTime;
    QString iso;
    QString ev;
    QString ae;
    QString wdr;
    QString wb;
    QString micSelect;
    int wbCT;

    Cam_Set()
    {
        IP = "190.168.0.1";
        videoResolution = "1920x1080";
        videoFrameRate = "FPS60";
        videoGop = 4;
        videoBitRate = "High";

        micSelect = "0";

        exposureTime = "auto";
        iso = "auto";
        ev = "0.0";
        ae = "center";
        wdr = "off";
        wb = "auto";
        wbCT = 4600;
    }

    static QJsonObject toObj(Cam_Set _cam_set);
    static Cam_Set toStruct(QJsonObject obj);
    QJsonObject toObj(bool = false);
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};

#endif
