#include "ServerParam.h"
QJsonObject ServerParam::toObj(ServerParam _serverparam)
{
    QJsonObject obj;
    obj["SYS_Name"] = _serverparam.SYS_Name;
    obj["IP"] = _serverparam.IP;


    return obj;
}

ServerParam ServerParam::toStruct(QJsonObject obj)
{
    ServerParam _serverparam;

    _serverparam.SYS_Name = obj["SYS_Name"].toString();
    _serverparam.IP = obj["IP"].toString();



    return _serverparam;
}

QJsonObject ServerParam::toObj()
{
    QJsonObject obj;

    obj["SYS_Name"] = this->SYS_Name;
    obj["IP"] = this->IP;



    return obj;
}

void ServerParam::setStruct(QJsonObject obj)
{
    this->SYS_Name = obj["SYS_Name"].toString();
    this->IP = obj["IP"].toString();


}

bool ServerParam::isMatch(QJsonObject obj)
{
    if (!obj.contains("SYS_Name")) return false;
    if (!obj.contains("IP")) return false;


    return true;
}
