#include "CamInfo.h"

QJsonObject CamInfo::toObj(CamInfo _caminfo)
{
    QJsonObject obj;
    obj["idex"] = _caminfo.idex;
//    obj["id"] = static_cast<int>(_caminfo.id);

    obj["ip"] = _caminfo.ip;
    obj["mode"] = _caminfo.mode;
    return obj;
}

CamInfo CamInfo::toStruct(QJsonObject obj)
{
    CamInfo _caminfo;
    _caminfo.idex = obj["idex"].toInt();
    _caminfo.id = static_cast<uint32_t>(obj["id"].toInt());
    _caminfo.ip = obj["ip"].toString();
    _caminfo.mode = obj["mode"].toString();
    return _caminfo;
}

QJsonObject CamInfo::toObj()
{
    QJsonObject obj;
    obj["idex"] = this->idex;
//    obj["id"] = static_cast<int>(this->id);

    obj["ip"] = this->ip;
    obj["mode"] = this->mode;
    return obj;
}

void CamInfo::setStruct(QJsonObject obj)
{
    this->idex = obj["idex"].toInt();
    this->id = static_cast<uint32_t>(obj["id"].toInt());

    this->ip = obj["ip"].toString();
    this->mode = obj["mode"].toString();
}

bool CamInfo::isMatch(QJsonObject obj)
{
    if (!obj.contains("idex")) return false;
    if (!obj.contains("id")) return false;
    if (!obj.contains("ip")) return false;
    if (!obj.contains("mode")) return false;
    return true;
}
