#ifndef CAMINFO_H
#define CAMINFO_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


struct CamInfo{
    int idex;
    uint32_t id; //不儲存
    QString ip;
    bool isConnect; //不儲存
    QString mode;

    void setCamInfo(QString IP,bool bl, QString Mode)
    {
        idex = -1;
        id = 0;
        ip = IP;
        isConnect = false;
        mode = Mode;
    }

    static QJsonObject toObj(CamInfo _caminfo);
    static CamInfo toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};


#endif
