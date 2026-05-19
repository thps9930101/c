
#ifndef SERVERPARAM_H
#define SERVERPARAM_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "Data/Struct/NewVideoParam/NewVideoParam.h"

struct ServerParam{
    QString SYS_Name;
    QString IP;

    QList<NewVideoParam> TrainingList;
    QList<NewVideoParam> priorityTrainingList;

    static QJsonObject toObj(ServerParam _serverparam);
    static ServerParam toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};

#endif
