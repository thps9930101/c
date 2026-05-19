#include "encode_h264_nvenc.h"

//===============================================
/*
 * error code
 * -10001 encoder codec not found
 * -10002 encoder context create fail
 * -10003 encoder open fail
*/
//===============================================
encode_h264_nvenc::encode_h264_nvenc()
{

}

encode_h264_nvenc::~encode_h264_nvenc()
{

}
//===============================================
static std::string get_h264_preset(Encoder_class_preset p)
{
    std::string str = "default";
    switch (p) {
    case Encoder_class_preset::Encoder_class_mq:
        str = "mq";
        break;

    case Encoder_class_preset::Encoder_class_hq:
        str = "hq";
        break;

    case Encoder_class_preset::Encoder_class_default:
        str = "default";
        break;

    case Encoder_class_preset::Encoder_class_hp:
        str = "hp";
        break;

    case Encoder_class_preset::Encoder_class_ll:
        str = "ll";
        break;

    case Encoder_class_preset::Encoder_class_llhq:
        str = "llhq";
        break;

    case Encoder_class_preset::Encoder_class_llhp:
        str = "llhp";
        break;
    }
    return str;
}

static std::string get_h264_profile(Encoder_class_profile p)
{
    std::string str = "high";
    switch (p) {
    case Encoder_class_profile::Encoder_class_baseline:
        str = "baseline";
        break;

    case Encoder_class_profile::Encoder_class_main:
        str = "main";
        break;

    case Encoder_class_profile::Encoder_class_high:
        str = "high";
        break;
    }

    return str;
}
//===============================================
int encode_h264_nvenc::encoder_init(AVCodecContext **enc_ctx, encode_h264_nvenc_param *param)
{
    //Codec
    AVCodec* codec = avcodec_find_encoder_by_name("h264_nvenc");
    if(!codec)
    {
        return -10001;
    }

    //CodecContext
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if(!ctx)
    {
        return -10002;
    }

    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->codec_id = codec->id;
    ctx->pix_fmt = param->pix_format;

    ctx->width = param->width;
    ctx->height = param->height;

    ctx->time_base.num = 1;
    ctx->time_base.den = param->fps;

    ctx->framerate.num = param->fps;
    ctx->framerate.den = 1;

    ctx->gop_size = param->gop;
    ctx->max_b_frames = param->max_b_frame;

    ctx->bit_rate = param->bitrate;

    ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

//    //---test
//    av_opt_set_int(ctx->priv_data, "cbr", true, 0);
//    ctx->rc_min_rate = param->bitrate;
//    ctx->rc_max_rate = param->bitrate;
//    ctx->rc_buffer_size = int(param->bitrate);
//    ctx->bit_rate_tolerance = int(param->bitrate);
//    ctx->rc_initial_buffer_occupancy = ctx->rc_buffer_size*3/4;
//    //-----

    ctx->color_trc = AVCOL_TRC_BT709;
    ctx->color_primaries = AVCOL_PRI_BT709;
    ctx->colorspace = AVCOL_SPC_BT709;

    std::string preser_str = get_h264_preset(param->preset);
    bool two_pass = false;
    if(_strcmpi(preser_str.c_str(), "mq") == 0)
    {
        two_pass = true;
        preser_str = "hq";
    }

    std::string profile_str = get_h264_profile(param->profile);

    av_opt_set(ctx->priv_data, "level", "auto", 0);
    av_opt_set_int(ctx->priv_data, "2pass", two_pass, 0);
    av_opt_set(ctx->priv_data, "preset", preser_str.c_str(), 0);
    av_opt_set(ctx->priv_data, "profile", profile_str.c_str(), 0);


    //open encoder
    if(avcodec_open2(ctx, codec, nullptr)<0)
    {
        avcodec_free_context(&ctx);
        return -10003;
    }


    *enc_ctx = ctx;
    return 0;
}


