#include "AppSettings.h"

QJsonObject AppSettings::toObj(AppSettings _appsetting)
{
    QJsonObject obj;
    obj["Decoder"] = _appsetting.Decoder.toObj();
    obj["Encoder"] = _appsetting.Encoder.toObj();
    obj["camSetting"] = _appsetting.camSetting.toObj();
    obj["aoFunSetting"] = _appsetting.aoFunSetting.toObj();
    obj["recordSetting"] = _appsetting.recordSetting.toObj();
    obj["RTSPSetting"] = _appsetting.RTSPSetting.toObj();
    obj["vidSetting"] = _appsetting.vidSetting.toObj();
//    obj["rotSetting"] = _appsetting.rotSetting.toObj();
    obj["App"] = _appsetting.app.toObj();


    return obj;
}

AppSettings AppSettings::toStruct(QJsonObject obj)
{
    AppSettings _appsetting;

    _appsetting.Decoder = Decoder_class_param::toStruct(obj["Decoder"].toObject());
    _appsetting.Encoder = Encoder_param::toStruct(obj["Encoder"].toObject());
    _appsetting.camSetting = Cam_Set::toStruct(obj["camSetting"].toObject());
    _appsetting.aoFunSetting = AoFunSetting::toStruct(obj["aoFunSetting"].toObject());
    _appsetting.app = App::toStruct(obj["App"].toObject());
    _appsetting.recordSetting = Record_param::toStruct(obj["recordSetting"].toObject());
    _appsetting.RTSPSetting = RTSP_param::toStruct(obj["RTSPSetting"].toObject());
    _appsetting.vidSetting = vid_ctx_param::toStruct(obj["vidSetting"].toObject());
//    _appsetting.rotSetting = rot_param::toStruct(obj["rotSetting"].toObject());

    return _appsetting;
}

QJsonObject AppSettings::toObj()
{
    QJsonObject obj;

    obj["Decoder"] = this->Decoder.toObj();
    obj["Encoder"] = this->Encoder.toObj();
    obj["camSetting"] = this->camSetting.toObj();
    obj["aoFunSetting"] = this->aoFunSetting.toObj();
    obj["App"] = this->app.toObj();
    obj["recordSetting"] = this->recordSetting.toObj();
    obj["RTSPSetting"] = this->RTSPSetting.toObj();
    obj["vidSetting"] = this->vidSetting.toObj();
//    obj["rotSetting"] = this->rotSetting.toObj();

    return obj;
}

void AppSettings::setStruct(QJsonObject obj)
{
    this->Decoder = Decoder_class_param::toStruct(obj["Decoder"].toObject());
    this->Encoder = Encoder_param::toStruct(obj["Encoder"].toObject());
    this->camSetting = Cam_Set::toStruct(obj["camSetting"].toObject());
    this->aoFunSetting = AoFunSetting::toStruct(obj["aoFunSetting"].toObject());
    this->app = App::toStruct(obj["App"].toObject());
    this->recordSetting = Record_param::toStruct(obj["recordSetting"].toObject());
    this->RTSPSetting = RTSP_param::toStruct(obj["RTSPSetting"].toObject());
    this->vidSetting = vid_ctx_param::toStruct(obj["vidSetting"].toObject());
//    this->rotSetting = rot_param::toStruct(obj["rotSetting"].toObject());
}



