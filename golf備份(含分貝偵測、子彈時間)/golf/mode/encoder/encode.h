#ifndef ENCODE_H
#define ENCODE_H

#include <mutex>
#include <string>

#include "Data/Struct/Encoder_param/encoder_param.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}
typedef Encoder_param Encoder_class_param;

class Encoder_class
{
public:
    Encoder_class();
    ~Encoder_class();

    int create();
    int create(Encoder_class_param fmt);
    int release();
    int update_encode_param(Encoder_class_param fmt);

    int send_encode_frame(AVFrame* frame);
    int get_encode_pkt(AVPacket* pkt);

    Encoder_class_param get_now_param();
    AVCodecContext* get_ctx();
private:
    int Encode_Create();
    int Encode_Delete();
    int Encode_Update(Encoder_class_param fmt);

public:


private:
    Encoder_class_param enc_param;
    AVCodecContext* enc_ctx;
    AVCodecContext* tmp_ctx;

    std::mutex  ec_lock;
};

#endif // ENCODE_H
