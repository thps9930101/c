#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

struct vid_ctx_param{

    AVCodecID codec_id;
    int width;
    int height;
    int fps;

    vid_ctx_param()
    {
        codec_id = AV_CODEC_ID_HEVC;
        width = 1920;
        height = 1080;
        fps = 60;
    }

    static QJsonObject toObj(vid_ctx_param _multiple_vid_ctx_param);
    static vid_ctx_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};
