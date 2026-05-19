#include "Cam_Set.h"

QJsonObject Cam_Set::toObj(Cam_Set _cam_set)
{
    QJsonObject obj;
    obj["IP"] = _cam_set.IP;
    obj["videoResolution"] = _cam_set.videoResolution;
    obj["videoFrameRate"] = _cam_set.videoFrameRate;
    obj["videoGop"] = _cam_set.videoGop;
    obj["videoBitRate"] = _cam_set.videoBitRate;
    obj["exposureTime"] = _cam_set.exposureTime;
    obj["iso"] = _cam_set.iso;
    obj["ev"] = _cam_set.ev;
    obj["ae"] = _cam_set.ae;
    obj["wdr"] = _cam_set.wdr;
    obj["wb"] = _cam_set.wb;
    obj["wbCT"] = _cam_set.wbCT;
    obj["micSelect"] = _cam_set.micSelect;
    return obj;
}

Cam_Set Cam_Set::toStruct(QJsonObject obj)
{
    Cam_Set _cam_set;

    _cam_set.IP = obj["IP"].toString();
    _cam_set.videoResolution = obj["videoResolution"].toString();
    _cam_set.videoFrameRate = obj["videoFrameRate"].toString();
    _cam_set.videoGop = obj["videoGop"].toInt();
    _cam_set.videoBitRate = obj["videoBitRate"].toString();
    _cam_set.exposureTime = obj["exposureTime"].toString();
    _cam_set.iso = obj["iso"].toString();
    _cam_set.ev = obj["ev"].toString();
    _cam_set.ae = obj["ae"].toString();
    _cam_set.wdr = obj["wdr"].toString();
    _cam_set.wb = obj["wb"].toString();
    _cam_set.wbCT = obj["wbCT"].toInt();
    _cam_set.micSelect = obj["micSelect"].toString();

    return _cam_set;
}

QJsonObject Cam_Set::toObj(bool isStream)
{
    QJsonObject obj;

    obj["IP"] = this->IP;

    if(!isStream){
        obj["videoResolution"] = this->videoResolution;
        obj["videoFrameRate"] = this->videoFrameRate;
        obj["videoGop"] = this->videoGop;
        obj["videoBitRate"] = this->videoBitRate;
    }

    obj["exposureTime"] = this->exposureTime;
    obj["iso"] = this->iso;
    obj["ev"] = this->ev;
    obj["ae"] = this->ae;
    obj["wdr"] = this->wdr;
    obj["wb"] = this->wb;
    obj["wbCT"] = this->wbCT;
    obj["micSelect"] = this->micSelect;

    return obj;
}

void Cam_Set::setStruct(QJsonObject obj)
{
    this->IP = obj["IP"].toString();
    this->videoResolution = obj["videoResolution"].toString();
    this->videoFrameRate = obj["videoFrameRate"].toString();
    this->videoGop = obj["videoGop"].toInt();
    this->videoBitRate = obj["videoBitRate"].toString();
    this->exposureTime = obj["exposureTime"].toString();
    this->iso = obj["iso"].toString();
    this->ev = obj["ev"].toString();
    this->ae = obj["ae"].toString();
    this->wdr = obj["wdr"].toString();
    this->wb = obj["wb"].toString();
    this->wbCT = obj["wbCT"].toInt();
    this->micSelect = obj["micSelect"].toString();
}

bool Cam_Set::isMatch(QJsonObject obj)
{
    if (!obj.contains("IP")) return false;
    if (!obj.contains("videoResolution")) return false;
    if (!obj.contains("videoFrameRate")) return false;
    if (!obj.contains("videoGop")) return false;
    if (!obj.contains("videoBitRate")) return false;
    if (!obj.contains("exposureTime")) return false;
    if (!obj.contains("iso")) return false;
    if (!obj.contains("ev")) return false;
    if (!obj.contains("ae")) return false;
    if (!obj.contains("wdr")) return false;
    if (!obj.contains("wb")) return false;
    if (!obj.contains("wbCT")) return false;
    if (!obj.contains("micSelect")) return false;

    return true;
}
