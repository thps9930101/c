#ifndef DECODER_CLASS_MODE_H
#define DECODER_CLASS_MODE_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

enum Decoder_class_Mode
{
    Decoder_class_PacketMode = 0,
    Decoder_class_FileMode
};

enum Decoder_class_CodecType
{
    Decoder_class_H264 = 0,
    Decoder_class_H265
};

struct Decoder_class_param
{
    Decoder_class_Mode mode;
    Decoder_class_CodecType codec_type;

    Decoder_class_param()
    {
        mode = Decoder_class_PacketMode;
        codec_type = Decoder_class_H265;
    }

    static QJsonObject toObj(Decoder_class_param _encoder_param);
    static Decoder_class_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
};

#endif // DECODER_CLASS_MODE_H
