#include "multiple_vid_ctx.h"

//=================================================
multiple_vid_ctx::multiple_vid_ctx()
{
    ctx_param.codec_id = AV_CODEC_ID_NONE;
    ctx_param.width = 0;
    ctx_param.height = 0;
    ctx_param.fps = 0;
}

multiple_vid_ctx::~multiple_vid_ctx()
{
    Release();
}

//=================================================
//public
int multiple_vid_ctx::create(multiple_vid_ctx_param param)
{
    if(param.codec_id == AV_CODEC_ID_NONE)
        return -1;

    if(param.width == 0)
        return -1;

    if(param.height == 0)
        return -1;

    if(param.fps < 5)
        return -1;

    ctx_param = param;
    return Create();
}

int multiple_vid_ctx::release()
{
    return Release();
}

//=================================================
//private
int multiple_vid_ctx::Create()
{
    codec = avcodec_find_encoder(ctx_param.codec_id);
    if(!codec)
        return -1000;

    ctx = avcodec_alloc_context3(codec);
    if(!ctx)
        return -1001;

    ctx->codec_id = codec->id;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->width = ctx_param.width;
    ctx->height = ctx_param.height;
    ctx->time_base = AVRational{1, ctx_param.fps};
    ctx->framerate = AVRational{ctx_param.fps, 1};


    //ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


    if (avcodec_open2(ctx, codec, nullptr) < 0)
    {
        avcodec_free_context(&ctx);
        return -1002;
    }

    return 0;
}

int multiple_vid_ctx::Release()
{
    codec = nullptr;
    if(ctx != nullptr)
    {
        avcodec_free_context(&ctx);
    }

    ctx_param.codec_id = AV_CODEC_ID_NONE;
    ctx_param.width = 0;
    ctx_param.height = 0;
    ctx_param.fps = 0;

    return 0;
}

//=================================================
