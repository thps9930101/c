#include "encoder_param.h"

#include "Encoder_param.h"

QJsonObject Encoder_param::toObj(Encoder_param _encoder_param)
{
    QJsonObject obj;
    obj["width"] = _encoder_param.width;
    obj["height"] = _encoder_param.height;
    obj["fps"] = _encoder_param.fps;
    obj["gop"] = _encoder_param.gop;
    obj["bitrate"] = _encoder_param.bitrate;
    obj["pix_format"] = static_cast<int>(_encoder_param.pix_format);
    obj["en_format"] = static_cast<int>(_encoder_param.en_format);

    //new
    obj["max_b_frame"] = _encoder_param.max_b_frame;
    obj["preset"] = static_cast<int>(_encoder_param.preset);
    obj["profile"] = static_cast<int>(_encoder_param.profile);

    return obj;
}

Encoder_param Encoder_param::toStruct(QJsonObject obj)
{
    Encoder_param _encoder_param;

    _encoder_param.width = obj["width"].toInt();
    _encoder_param.height = obj["height"].toInt();
    _encoder_param.fps = obj["fps"].toInt();
    _encoder_param.gop = obj["gop"].toInt();
    _encoder_param.bitrate = obj["bitrate"].toDouble();
    _encoder_param.pix_format = static_cast<AVPixelFormat>(obj["pix_format"].toInt());
    _encoder_param.en_format = static_cast<Encoder_class_format>(obj["en_format"].toInt());

    //new
    _encoder_param.max_b_frame = obj["max_b_frame"].toInt();
    _encoder_param.preset = static_cast<Encoder_class_preset>(obj["preset"].toInt());
    _encoder_param.profile = static_cast<Encoder_class_profile>(obj["profile"].toInt());

    return _encoder_param;
}

QJsonObject Encoder_param::toObj()
{
    QJsonObject obj;

    obj["width"] = this->width;
    obj["height"] = this->height;
    obj["fps"] = this->fps;
    obj["gop"] = this->gop;
    obj["bitrate"] = this->bitrate;
    obj["pix_format"] = static_cast<int>(this->pix_format);
    obj["en_format"] = static_cast<int>(this->en_format);

    //new
    obj["max_b_frame"] = this->max_b_frame;
    obj["preset"] = static_cast<int>(this->preset);
    obj["profile"] = static_cast<int>(this->profile);

    return obj;
}

void Encoder_param::setStruct(QJsonObject obj)
{
    this->width      = obj["width"].toInt();
    this->height     = obj["height"].toInt();
    this->fps        = obj["fps"].toInt();
    this->gop        = obj["gop"].toInt();
    this->bitrate    = obj["bitrate"].toDouble();
    this->pix_format = static_cast<AVPixelFormat>(obj["pix_format"].toInt());
    this->en_format  = static_cast<Encoder_class_format>(obj["en_format"].toInt());

    //new
    this->max_b_frame = obj["max_b_frame"].toInt();
    this->preset = static_cast<Encoder_class_preset>(obj["preset"].toInt());
    this->profile = static_cast<Encoder_class_profile>(obj["profile"].toInt());
}


