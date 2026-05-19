#ifndef APP_H
#define APP_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "Data/Struct/CamInfo/CamInfo.h"

struct App{
    QString lang;
    QString fontSize;
    QList<QString> IpRangeList;
    int FPS;
    bool AUTOSTART;
    QVector<CamInfo> autoCamList;
    // Sura可能會新增娉接畫面的參數

    App()
    {
        lang = "languages/lang_en.qm";
        fontSize = "font-size: 18px; ";
        FPS = 60;
        IpRangeList = QList<QString>();
        IpRangeList.append("190.168.0.1");
        IpRangeList.append("190.168.0.58");

        AUTOSTART = false;
        autoCamList = QVector<CamInfo>();
    }

    static QJsonObject toObj(App _app);
    static App toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
};

#endif
