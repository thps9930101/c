#include "AoFunSetting.h"

QJsonObject AoFunSetting::toObj(AoFunSetting _aofunsetting)
{
    QJsonObject obj;
    QJsonArray jArray;
    for (auto item : _aofunsetting.SPCamList) {
        jArray.append(item);
    }
    obj["SPCamList"] = jArray;
    obj["mode"] = _aofunsetting.mode;
    //obj["rtsp_path"] = _aofunsetting.rtsp_path;
    obj["SP_mode"] = static_cast<int>(_aofunsetting.SP_mode);
    return obj;
}

QJsonObject AoFunSetting::toObj()
{
    QJsonObject obj;

    obj["mode"] = this->mode;
    //obj["rtsp_path"] = this->rtsp_path;
    obj["SP_mode"] = static_cast<int>(this->SP_mode);

    QJsonArray jArray;
    for (auto item : this->SPCamList) {
        jArray.append(item);
    }
    obj["SPCamList"] = jArray;

    return obj;
}


AoFunSetting AoFunSetting::toStruct(QJsonObject obj)
{
    AoFunSetting _aofunsetting;

    _aofunsetting.mode = obj["mode"].toInt();
    //_aofunsetting.rtsp_path = obj["rtsp_path"].toString();
    _aofunsetting.SP_mode = static_cast<SP_MODE>(obj["SP_mode"].toInt());

    QJsonArray jArray = obj["SPCamList"].toArray();
    for (auto item : jArray) {
        _aofunsetting.SPCamList.push_back(item.toString());
    }

    return _aofunsetting;
}

void AoFunSetting::setStruct(QJsonObject obj)
{
    this->mode = obj["mode"].toInt();
    //this->rtsp_path = obj["rtsp_path"].toString();
    this->SP_mode = static_cast<SP_MODE>(obj["SP_mode"].toInt());
    QJsonArray jArray = obj["SPCamList"].toArray();
    for (auto item : jArray) {
        this->SPCamList.push_back(item.toString());
    }
}

