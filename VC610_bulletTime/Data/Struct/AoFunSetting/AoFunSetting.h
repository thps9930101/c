#ifndef AOFUNSETTING_H
#define AOFUNSETTING_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "Data/Enum/SP_Mode.h"

struct AoFunSetting{
    int mode;
    //QString rtsp_path;
    SP_MODE SP_mode;
    std::vector<QString> SPCamList = std::vector<QString>();

    AoFunSetting()
    {
        mode = 0;
        //rtsp_path = "rtsp://127.0.0.1:8554/stream";
        SP_mode = SP_MODE::sp_4k_4x3;
    }

    static QJsonObject toObj(AoFunSetting _aofunsetting);
    static AoFunSetting toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
};

#endif
