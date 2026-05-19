#include "rot_param.h"
QJsonObject rot_param::toObj(rot_param _rot_param)
{
    QJsonObject obj;
    QJsonArray jArray;
    for (auto item : _rot_param.sp_rot) {
        jArray.append(static_cast<int>(item));
    }
    obj["sp_rot"] = jArray;
    return obj;
}

rot_param rot_param::toStruct(QJsonObject obj)
{
    rot_param _rot_param;

    QJsonArray jArray = obj["sp_rot"].toArray();
    for (auto item : jArray) {
        _rot_param.sp_rot.push_back(static_cast<SP_Rot>(item.toInt()));
    }

    return _rot_param;
}

QJsonObject rot_param::toObj()
{
    QJsonObject obj;

    QJsonArray jArray;
    for (auto item : this->sp_rot) {
        jArray.append(item);
    }
    obj["sp_rot"] = jArray;

    return obj;
}

void rot_param::setStruct(QJsonObject obj)
{
    QJsonArray jArray = obj["sp_rot"].toArray();
    for (auto item : jArray) {
        this->sp_rot.push_back(static_cast<SP_Rot>(item.toInt()));
    }
}

bool rot_param::isMatch(QJsonObject obj)
{
    if (!obj.contains("sp_rot")) return false;
    return true;
}
