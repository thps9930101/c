#ifndef MULTIPLE_VID_CTX_H
#define MULTIPLE_VID_CTX_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

struct multiple_vid_ctx_param{
    AVCodecID codec_id;
    int width;
    int height;
    int fps;
};

class multiple_vid_ctx
{
public:
    multiple_vid_ctx();
    ~multiple_vid_ctx();

    int create(multiple_vid_ctx_param param);
    int release();

    multiple_vid_ctx_param ctx_param;
    AVCodecContext* ctx = nullptr;
    AVCodec* codec = nullptr;

private:
    int Create();
    int Release();
};

#endif // MULTIPLE_VID_CTX_H
