#ifndef DECODE_M_H
#define DECODE_M_H

#include <string>
#include "Data/Struct/Decoder_param/decoder_class_mode.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
}



class Decoder_class
{
public:
    Decoder_class();
    ~Decoder_class();

    int create();
    int create(Decoder_class_param param);
    int release();

    int send_decode_pkt(AVPacket* pkt);
    int get_decode_frame(AVFrame* frame);
    int get_decode_frame_GPU(AVFrame *frame);

private:
    int Decode_Create();
    int Decode_Delete();

public:

private:
    Decoder_class_param param;
    AVCodecContext* dec_ctx;
    AVCodecContext* tmp_ctx;

    AVBufferRef* hw_ctx;
    AVBufferRef* tmp_hw_ctx;

    AVFrame* dec_GPU_frame;


};

#endif // DECODE_H
