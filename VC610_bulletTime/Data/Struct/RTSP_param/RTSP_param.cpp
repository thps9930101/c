#include "RTSP_param.h"
QJsonObject RTSP_param::toObj(RTSP_param _rtsp_param)
{
    QJsonObject obj;
    obj["rtsp_path"] = _rtsp_param.rtsp_path;
    obj["rtmp_path"] = _rtsp_param.rtmp_path;
    obj["service"] = _rtsp_param.service;
    return obj;
}

RTSP_param RTSP_param::toStruct(QJsonObject obj)
{
    RTSP_param _rtsp_param;
    _rtsp_param.rtsp_path = obj["rtsp_path"].toString();
    _rtsp_param.rtmp_path = obj["rtmp_path"].toString();
    _rtsp_param.service = obj["service"].toInt();
    return _rtsp_param;
}

QJsonObject RTSP_param::toObj()
{
    QJsonObject obj;

    obj["rtsp_path"] = this->rtsp_path;
    obj["rtmp_path"] = this->rtmp_path;
    obj["service"] = this->service;
    return obj;
}

void RTSP_param::setStruct(QJsonObject obj)
{
    this->rtmp_path = obj["rtmp_path"].toString();
    this->rtsp_path = obj["rtsp_path"].toString();
    this->service = obj["service"].toInt();
}

bool RTSP_param::isMatch(QJsonObject obj)
{
    if (!obj.contains("rtsp_path")) return false;
    return true;
}
