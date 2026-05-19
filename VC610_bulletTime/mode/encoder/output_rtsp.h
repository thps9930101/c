#ifndef OUTPUT_RTSP_H
#define OUTPUT_RTSP_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}

#include "conver.h"
#include "Data/Struct/Encoder_param/encoder_param.h"


class output_rtsp
{
public:
    output_rtsp();
    ~output_rtsp();

    int create(const char *output_rtsp, Encoder_param en_par);
    int write_frame(uint8_t *nv12_data);
    int write_frame(AVFrame *nv12_frame);
    int release();

private:

    enum rtsp_state {
        rtsp_create = 0,
        rtsp_release
    };


    Encoder_param encoder_par;

    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVStream *videoStream;

    AVFrame *avTemp;
    AVPacket *en_avpkt;

    uint32_t frame_count = 0;


    std::mutex  rtsp_lock;

    rtsp_state rt_state;






};

#endif // OUTPUT_RTSP_H
