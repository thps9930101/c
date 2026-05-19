#include "multiple_vid_ctx_param.h"
QJsonObject vid_ctx_param::toObj(vid_ctx_param _multiple_vid_ctx_param)
{
    QJsonObject obj;
    obj["width"] = _multiple_vid_ctx_param.width;
    obj["height"] = _multiple_vid_ctx_param.height;
    obj["fps"] = _multiple_vid_ctx_param.fps;
    obj["codec_id"] = _multiple_vid_ctx_param.codec_id;

    return obj;
}

vid_ctx_param vid_ctx_param::toStruct(QJsonObject obj)
{
    vid_ctx_param _multiple_vid_ctx_param;

    _multiple_vid_ctx_param.width = obj["width"].toInt();
    _multiple_vid_ctx_param.height = obj["height"].toInt();
    _multiple_vid_ctx_param.fps = obj["fps"].toInt();
    _multiple_vid_ctx_param.codec_id = static_cast<AVCodecID>(obj["codec_id"].toInt());

    return _multiple_vid_ctx_param;
}

QJsonObject vid_ctx_param::toObj()
{
    QJsonObject obj;

    obj["width"] = this->width;
    obj["height"] = this->height;
    obj["fps"] = this->fps;
    obj["codec_id"] = static_cast<int>(this->codec_id);

    return obj;
}

void vid_ctx_param::setStruct(QJsonObject obj)
{
    this->width = obj["width"].toInt();
    this->height = obj["height"].toInt();
    this->fps = obj["fps"].toInt();
    this->codec_id = static_cast<AVCodecID>(obj["codec_id"].toInt());
}

bool vid_ctx_param::isMatch(QJsonObject obj)
{
    if (!obj.contains("width")) return false;
    if (!obj.contains("height")) return false;
    if (!obj.contains("fps")) return false;
    if (!obj.contains("codec_id")) return false;
    return true;
}
