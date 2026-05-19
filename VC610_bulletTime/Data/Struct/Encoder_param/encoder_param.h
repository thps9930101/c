#ifndef ENCODER_PARAM_H
#define ENCODER_PARAM_H

#include <stdint.h>
#include <libavutil/pixfmt.h>


#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

Q_DECLARE_METATYPE(AVPixelFormat)

enum Encoder_class_format{
    Encoder_class_h264_nvenc = 0,
    Encoder_class_hevc_nvenc
};

enum Encoder_class_preset{
    Encoder_class_mq = 0,     //品質最高
    Encoder_class_hq = 1,     //畫質
    Encoder_class_hp,         //效能最高
    Encoder_class_default,    //效能
    Encoder_class_ll,         //低延遲
    Encoder_class_llhq,       //低延遲品質
    Encoder_class_llhp        //低延遲效能
};

enum Encoder_class_profile{
    Encoder_class_baseline = 0, //基本畫質
    Encoder_class_main,         //主流畫質
    Encoder_class_high          //最高畫質
};

struct Encoder_param{

    int width;
    int height;
    int fps;
    int gop;
    int64_t bitrate;

    AVPixelFormat pix_format;

    Encoder_class_format en_format;

    //new
    int max_b_frame;
    Encoder_class_preset preset;
    Encoder_class_profile profile;

    Encoder_param()
    {
        width = 3840;
        height = 2160;
        fps = 60;
        gop = 4;
        bitrate = 60000000;
        max_b_frame = 0;
        pix_format = AV_PIX_FMT_NV12;
        en_format = Encoder_class_format::Encoder_class_h264_nvenc;

        //new
        preset = Encoder_class_preset::Encoder_class_mq;
        profile = Encoder_class_profile::Encoder_class_high;
    }
    static QJsonObject toObj(Encoder_param _encoder_param);
    static Encoder_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
};
#endif // ENCODER_PARAM_H
