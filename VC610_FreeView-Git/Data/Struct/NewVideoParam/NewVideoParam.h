
#ifndef NEWVIDEOPARAM_H
#define NEWVIDEOPARAM_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

struct NewVideoParam{
    QString sessionID;
    QString training_ID;
    QString vid_L_ID;
    QString vid_R_ID;
    QString sysName;

    static QJsonObject toObj(NewVideoParam _newvideoparam);
    static NewVideoParam toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};

#endif
