#include "NewVideoParam.h"

QJsonObject NewVideoParam::toObj(NewVideoParam _newvideoparam)
{
    QJsonObject obj;
    obj["sessionID"] = _newvideoparam.sessionID;
    obj["training_ID"] = _newvideoparam.training_ID;
    obj["vid_L_ID"] = _newvideoparam.vid_L_ID;
    obj["vid_R_ID"] = _newvideoparam.vid_R_ID;
    obj["sysName"] = _newvideoparam.sysName;
    return obj;
}

NewVideoParam NewVideoParam::toStruct(QJsonObject obj)
{
    NewVideoParam _newvideoparam;

    _newvideoparam.sessionID = obj["sessionID"].toString();
    _newvideoparam.training_ID = obj["training_ID"].toString();
    _newvideoparam.vid_L_ID = obj["vid_L_ID"].toString();
    _newvideoparam.vid_R_ID = obj["vid_R_ID"].toString();
    _newvideoparam.sysName = obj["sysName"].toString();

    return _newvideoparam;
}

QJsonObject NewVideoParam::toObj()
{
    QJsonObject obj;

    obj["sessionID"] = this->sessionID;
    obj["training_ID"] = this->training_ID;
    obj["vid_L_ID"] = this->vid_L_ID;
    obj["vid_R_ID"] = this->vid_R_ID;
    obj["sysName"] = this->sysName;

    return obj;
}

void NewVideoParam::setStruct(QJsonObject obj)
{
    this->sessionID = obj["sessionID"].toString();
    this->training_ID = obj["training_ID"].toString();
    this->vid_L_ID = obj["vid_L_ID"].toString();
    this->vid_R_ID = obj["vid_R_ID"].toString();
    this->sysName = obj["sysName"].toString();
}

bool NewVideoParam::isMatch(QJsonObject obj)
{
    if (!obj.contains("sessionID")) return false;
    if (!obj.contains("training_ID")) return false;
    if (!obj.contains("vid_L_ID")) return false;
    if (!obj.contains("vid_R_ID")) return false;
    if (!obj.contains("sysName")) return false;
    return true;
}
