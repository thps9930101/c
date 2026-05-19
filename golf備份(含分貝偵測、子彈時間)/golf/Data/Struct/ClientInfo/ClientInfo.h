#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "Data/Struct/ServerParam/ServerParam.h"

struct ClientInfo{
    ServerParam System_Record;
    ServerParam System_StationA;
    ServerParam System_ITRI;
    ServerParam System_PitchMachine;
    ServerParam System_WebSocket;

    static QJsonObject toObj(ClientInfo _clientinfo);
    static ClientInfo toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);


    ServerParam searchByName(QString sysName);
    QString searchIPByName(QString sysName);
    QString searchNameByIP(QString IP);
};
