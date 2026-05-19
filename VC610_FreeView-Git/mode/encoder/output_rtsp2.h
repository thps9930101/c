#ifndef OUTPUT_RTSP_H
#define OUTPUT_RTSP_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include <mutex>
#include "Data/Struct/Encoder_param/encoder_param.h"


class output_rtsp
{
public:
    output_rtsp();
    ~output_rtsp();

    int create(const char *output_rtsp, Encoder_param en_par,AVCodecContext *Context);
    int write_frame(AVPacket *av_pack);
    int write_au(AVPacket *av_pack);
    int write_au(uint8_t * data,uint32_t  len);
    int release();
    bool is_open();

private:

    enum rtsp_state {
        rtsp_create = 0,
        rtsp_release
    };

    Encoder_param encoder_par;

    std::mutex  rtsp_lock;
    rtsp_state rt_state;

    AVFormatContext *formatContext;

    //----------------------------------
    //video
    AVCodecContext *v_CodecContext = nullptr;
    AVStream* v_stream = nullptr;

    int v_index;

    AVRational v_rate;
    int64_t v_pkt_duration;

    int64_t v_rtsp_cnt = 0;

    //----------------------------------
    //audio
    AVCodecContext* a_CodecContext = nullptr;
    AVStream* a_stream = nullptr;

    int a_index;

    AVRational a_rate;
    int64_t a_pkt_duration;

    int64_t a_rtsp_cnt = 0;

    AVPacket au_pack;
};

#endif // OUTPUT_RTSP_H
