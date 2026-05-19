#include "App.h"

QJsonObject App::toObj(App _app)
{
    QJsonObject obj;
    obj["lang"] = _app.lang;
    obj["fontSize"] = _app.fontSize;

    QJsonArray obj_IpRange;
    for(int i = 0; i < _app.IpRangeList.size(); i++)
    {
        obj_IpRange.append(_app.IpRangeList[i]);
    }
    obj["IpRangeList"] = obj_IpRange;

    obj["FPS"] = _app.FPS;
    obj["AUTOSTART"] = _app.AUTOSTART;
    QJsonArray autoCamList;
    for(int i = 0; i < _app.autoCamList.size(); i++)
    {
        autoCamList.append(_app.autoCamList[i].toObj());
    }
    obj["autoCamList"] = autoCamList;

    return obj;
}

App App::toStruct(QJsonObject obj)
{
    App _app;

    _app.lang = obj["lang"].toString();
    _app.fontSize = obj["fontSize"].toString();

    QJsonArray jArray = obj["IpRangeList"].toArray();
    _app.IpRangeList.clear();
    for (auto item : jArray) {
        _app.IpRangeList.append(item.toString());
    }

    _app.FPS = obj["FPS"].toInt();
    _app.AUTOSTART = obj["AUTOSTART"].toBool();

    _app.autoCamList.clear();
    QJsonArray CamList = obj["autoCamList"].toArray();
    for (auto item : CamList) {
        _app.autoCamList.append(CamInfo::toStruct(item.toObject()));
    }

    return _app;
}

QJsonObject App::toObj()
{
    QJsonObject obj;

    obj["lang"] = this->lang;
    obj["fontSize"] = this->fontSize;

    QJsonArray obj_IpRange;
    for(int i = 0; i < this->IpRangeList.size(); i++)
    {
        obj_IpRange.append(this->IpRangeList[i]);
    }

    obj["IpRangeList"] = obj_IpRange;
    obj["FPS"] = this->FPS;
    obj["AUTOSTART"] = this->AUTOSTART;

    QJsonArray autoCamList;
    for(int i = 0; i < this->autoCamList.size(); i++)
    {
        autoCamList.append(this->autoCamList[i].toObj());
    }
    obj["autoCamList"] = autoCamList;

    return obj;
}

void App::setStruct(QJsonObject obj)
{
    this->lang = obj["lang"].toString();
    this->fontSize = obj["fontSize"].toString();
    QJsonArray jArray = obj["IpRangeList"].toArray();
    this->IpRangeList.clear();
    for (auto item : jArray) {
        this->IpRangeList.append(item.toString());
    }
    this->FPS = obj["FPS"].toInt();
    this->AUTOSTART = obj["AUTOSTART"].toBool();

    this->autoCamList.clear();
    QJsonArray CamList = obj["autoCamList"].toArray();
    for (auto item : CamList) {
        this->autoCamList.append(CamInfo::toStruct(item.toObject()));
    }

}

