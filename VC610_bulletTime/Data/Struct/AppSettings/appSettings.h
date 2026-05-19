#ifndef APPSETTING_H
#define APPSETTING_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "Data/Struct/Decoder_param/decoder_class_mode.h"
#include "Data/Struct/Encoder_param/encoder_param.h"
#include "Data/Struct/Cam_Set/Cam_Set.h"
#include "Data/Struct/App/app.h"
#include "Data/Struct/AoFunSetting/AoFunSetting.h"
#include "Data/Struct/Record_param/Record_param.h"
#include "Data/Struct/RTSP_param/RTSP_param.h"
#include "Data/Struct/multiple_vid_ctx_param/multiple_vid_ctx_param.h"
#include "Data/Struct/rot_param/rot_param.h"

//#include "Data/Struct/CamInfo/CamInfo.h"


struct AppSettings{
    Decoder_class_param Decoder;
    Encoder_param Encoder;
    Cam_Set camSetting;
    AoFunSetting aoFunSetting;
    App app;
    Record_param recordSetting;
    RTSP_param RTSPSetting;
    vid_ctx_param vidSetting;
//    rot_param rotSetting;

    AppSettings()
    {
        Decoder = Decoder_class_param();
        Encoder = Encoder_param();
        camSetting = Cam_Set();
        app = App();
        aoFunSetting = AoFunSetting();
        recordSetting = Record_param();
        RTSPSetting = RTSP_param();
        vidSetting = vid_ctx_param();
//        rotSetting = rot_param();
    }

    static QJsonObject toObj(AppSettings _appsetting);
    static AppSettings toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
};

#endif
