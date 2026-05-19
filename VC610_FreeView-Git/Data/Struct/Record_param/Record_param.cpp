#include "Record_param.h"
QJsonObject Record_param::toObj(Record_param _record_param)
{
    QJsonObject obj;
    obj["record_path"] = _record_param.record_path;
    obj["frontName"] = _record_param.frontName;
    obj["backName"] = _record_param.backName;
    obj["sec"] = _record_param.sec;
    obj["record_type"] = _record_param.record_type;
    return obj;
}

Record_param Record_param::toStruct(QJsonObject obj)
{
    Record_param _record_param;

    _record_param.record_path = obj["record_path"].toString();
    _record_param.frontName = obj["frontName"].toString();
    _record_param.backName = obj["backName"].toString();
    _record_param.sec = obj["sec"].toInt();
    _record_param.record_type = obj["record_type"].toInt();
    return _record_param;
}

QJsonObject Record_param::toObj()
{
    QJsonObject obj;

    obj["record_path"] = this->record_path;
    obj["frontName"] = this->frontName;
    obj["backName"] = this->backName;
    obj["sec"] = this->sec;
    obj["record_type"] = this->record_type;
    return obj;
}

void Record_param::setStruct(QJsonObject obj)
{
    this->record_path = obj["record_path"].toString();
    this->frontName = obj["frontName"].toString();
    this->backName = obj["backName"].toString();
    this->sec = obj["sec"].toInt();
    this->record_type = obj["record_type"].toInt();
}

//bool Record_param::isMatch(QJsonObject obj)
//{
//    if (!obj.contains("record_path")) return false;
//    return true;
//}
