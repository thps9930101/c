#include "decoder_class_mode.h"


QJsonObject Decoder_class_param::toObj(Decoder_class_param _Decoder_class_param)
{
    QJsonObject obj;

    obj["mode"] = static_cast<int>(_Decoder_class_param.mode);
    obj["codec_type"] = static_cast<int>(_Decoder_class_param.codec_type);

    return obj;
}

Decoder_class_param Decoder_class_param::toStruct(QJsonObject obj)
{
    Decoder_class_param _Decoder_class_param;

    _Decoder_class_param.mode = static_cast<Decoder_class_Mode>(obj["mode"].toInt());
    _Decoder_class_param.codec_type = static_cast<Decoder_class_CodecType>(obj["codec_type"].toInt());

    return _Decoder_class_param;
}

QJsonObject Decoder_class_param::toObj()
{
    QJsonObject obj;

    obj["mode"] = static_cast<int>(this->mode);
    obj["codec_type"] = static_cast<int>(this->codec_type);


    return obj;
}

void Decoder_class_param::setStruct(QJsonObject obj)
{
    this->mode = static_cast<Decoder_class_Mode>(obj["mode"].toInt());
    this->codec_type = static_cast<Decoder_class_CodecType>(obj["codec_type"].toInt());
}


