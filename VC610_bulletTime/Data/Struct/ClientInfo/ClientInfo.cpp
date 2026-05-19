#include "ClientInfo.h"
#include "Data/Define/DefineData.h"

QJsonObject ClientInfo::toObj(ClientInfo _clientinfo)
{
    QJsonObject obj;
    obj["System_Record"] = _clientinfo.System_Record.toObj();
    obj["System_StationA"] = _clientinfo.System_StationA.toObj();
    obj["System_ITRI"] = _clientinfo.System_ITRI.toObj();
    obj["System_PitchMachine"] = _clientinfo.System_PitchMachine.toObj();
    obj["System_WebSocket"] = _clientinfo.System_WebSocket.toObj();
    return obj;
}

ClientInfo ClientInfo::toStruct(QJsonObject obj)
{
    ClientInfo _clientinfo;

    _clientinfo.System_Record = ServerParam::toStruct(obj["System_Record"].toObject());

    _clientinfo.System_StationA = ServerParam::toStruct(obj["System_StationA"].toObject());

    _clientinfo.System_ITRI = ServerParam::toStruct(obj["System_ITRI"].toObject());

    _clientinfo.System_PitchMachine = ServerParam::toStruct(obj["System_PitchMachine"].toObject());

    _clientinfo.System_WebSocket = ServerParam::toStruct(obj["System_WebSocket"].toObject());


    return _clientinfo;
}

QJsonObject ClientInfo::toObj()
{
    QJsonObject obj;

    obj["System_Record"] = this->System_Record.toObj();
    obj["System_StationA"] = this->System_StationA.toObj();
    obj["System_ITRI"] = this->System_ITRI.toObj();
    obj["System_PitchMachine"] = this->System_PitchMachine.toObj();
    obj["System_WebSocket"] = this->System_WebSocket.toObj();

    return obj;
}

void ClientInfo::setStruct(QJsonObject obj)
{
    this->System_Record = ServerParam::toStruct(obj["System_Record"].toObject());

    this->System_StationA = ServerParam::toStruct(obj["System_StationA"].toObject());

    this->System_ITRI = ServerParam::toStruct(obj["System_ITRI"].toObject());

    this->System_PitchMachine = ServerParam::toStruct(obj["System_PitchMachine"].toObject());

    this->System_WebSocket = ServerParam::toStruct(obj["System_WebSocket"].toObject());

}

bool ClientInfo::isMatch(QJsonObject obj)
{
    if (!obj.contains("System_Record")) return false;
    if (!ServerParam::isMatch(obj["System_Record"].toObject())) return false;
    if (!obj.contains("System_StationA")) return false;
    if (!ServerParam::isMatch(obj["System_StationA"].toObject())) return false;
    if (!obj.contains("System_ITRI")) return false;
    if (!ServerParam::isMatch(obj["System_ITRI"].toObject())) return false;
    if (!obj.contains("System_PitchMachine")) return false;
    if (!ServerParam::isMatch(obj["System_PitchMachine"].toObject())) return false;
    if (!obj.contains("System_WebSocket")) return false;
    if (!ServerParam::isMatch(obj["System_WebSocket"].toObject())) return false;
    return true;
}
ServerParam ClientInfo::searchByName(QString sysName){
    if (sysName == STATION_A)
        return System_StationA;
    else if (sysName == RECORD)
        return System_Record;
    else if (sysName == ITRI)
        return System_ITRI;
    else if (sysName == PITCH_MACHINE)
        return System_PitchMachine;
    else if (sysName == WEB_CLIENT)
        return System_WebSocket;
    else
        return ServerParam();
}

QString ClientInfo::searchIPByName(QString sysName){
    return searchByName(sysName).IP;
}

QString ClientInfo::searchNameByIP(QString IP){
    if (IP.startsWith("::ffff:")) {
        IP = IP.mid(7);
    }
    if (IP == System_StationA.IP)
        return System_StationA.SYS_Name;
    else if (IP == System_Record.IP)
        return System_Record.SYS_Name;
    else if (IP == System_ITRI.IP)
        return System_ITRI.SYS_Name;
    else if (IP == System_PitchMachine.IP)
        return System_PitchMachine.SYS_Name;
    else if (IP == System_WebSocket.IP)
        return System_WebSocket.SYS_Name;

    return "UNDEFINE";
}
