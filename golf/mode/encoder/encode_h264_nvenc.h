#ifndef ENCODE_H264_NVENC_H
#define ENCODE_H264_NVENC_H

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}
#include <Data/Struct/Encoder_param/encoder_param.h>

struct encode_h264_nvenc_param{
    int width;
    int height;
    int fps;
    int gop;
    int max_b_frame;
    int64_t bitrate;
    AVPixelFormat pix_format;

    Encoder_class_preset preset;
    Encoder_class_profile profile;
};

class encode_h264_nvenc
{
public:
    encode_h264_nvenc();
    ~encode_h264_nvenc();

    static int encoder_init(AVCodecContext** enc_ctx, encode_h264_nvenc_param* param);
};

#endif // ENCODE_H264_NVENC_H
