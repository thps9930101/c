#include "encode.h"
#include "encode_h264_nvenc.h"


//================================================
Encoder_class::Encoder_class()
{
    enc_ctx = nullptr;
    tmp_ctx = nullptr;
}

Encoder_class::~Encoder_class()
{
    release();
}

//================================================
static int create_my_h264_nvenc(AVCodecContext** enc_ctx, Encoder_class_param* param)
{
    ///////////////convert param to h264_nvenc/////////////
    encode_h264_nvenc_param p;

    //preset
    switch (param->preset) {
    case Encoder_class_preset::Encoder_class_mq:
        p.preset = Encoder_class_preset::Encoder_class_mq;
        break;

    case Encoder_class_preset::Encoder_class_hq:
        p.preset = Encoder_class_preset::Encoder_class_hq;
        break;

    case Encoder_class_preset::Encoder_class_default:
        p.preset = Encoder_class_preset::Encoder_class_default;
        break;

    case Encoder_class_preset::Encoder_class_hp:
        p.preset = Encoder_class_preset::Encoder_class_hp;
        break;

    case Encoder_class_preset::Encoder_class_ll:
        p.preset = Encoder_class_preset::Encoder_class_ll;
        break;

    case Encoder_class_preset::Encoder_class_llhq:
        p.preset = Encoder_class_preset::Encoder_class_llhq;
        break;

    case Encoder_class_preset::Encoder_class_llhp:
        p.preset = Encoder_class_preset::Encoder_class_llhp;
        break;

    default:
        p.preset = Encoder_class_preset::Encoder_class_default;
        break;
    }

    //profile
    switch (param->profile) {
    case Encoder_class_profile::Encoder_class_baseline:
        p.profile = Encoder_class_profile::Encoder_class_baseline;
        break;

    case Encoder_class_profile::Encoder_class_main:
        p.profile = Encoder_class_profile::Encoder_class_main;
        break;

    case Encoder_class_profile::Encoder_class_high:
        p.profile = Encoder_class_profile::Encoder_class_high;
        break;

    default:
        p.profile = Encoder_class_profile::Encoder_class_high;
        break;
    }

    //other param
    p.width = param->width;
    p.height = param->height;
    p.fps = param->fps;
    p.gop = param->gop;
    p.max_b_frame = param->max_b_frame;
    p.bitrate = param->bitrate;
    p.pix_format = param->pix_format;

    //////////////////////////////////////////////
    //encode_h264_nvenc enc264;
    return encode_h264_nvenc::encoder_init(enc_ctx, &p);
}

static int create_my_encode(AVCodecContext** enc_ctx, Encoder_class_param* param)
{
    if(param->en_format == Encoder_class_h264_nvenc)
    {
        return create_my_h264_nvenc(enc_ctx, param);
    }

    return -1;
}

//================================================
//public

//create
int Encoder_class::create()
{
    int ret;

    ec_lock.lock();
    ret = Encode_Create() ;
    ec_lock.unlock();
    return ret;
}

int Encoder_class::create(Encoder_class_param fmt)
{
    int ret;

    ec_lock.lock();
    enc_param = fmt;
    ret = Encode_Create();
    ec_lock.unlock();
    return ret;
}


//release
int Encoder_class::release()
{
    int ret;

    ec_lock.lock();
    ret = Encode_Delete();
    ec_lock.unlock();

    return ret;
}


//update
int Encoder_class::update_encode_param(Encoder_class_param fmt)
{
    int ret;

    ec_lock.lock();
    ret = Encode_Update(fmt);
    ec_lock.unlock();

    return ret;
}

//
int Encoder_class::send_encode_frame(AVFrame* frame)
{
    int ret;

    ec_lock.lock();

    if(this->enc_ctx == nullptr)
    {
        ec_lock.unlock();
        return -1;
    }

    ret = avcodec_send_frame(this->enc_ctx, frame);
    ec_lock.unlock();

    return ret;
}

int Encoder_class::get_encode_pkt(AVPacket* pkt)
{
    int ret;

    ec_lock.lock();

    if(this->enc_ctx == nullptr)
    {
        ec_lock.unlock();
        return -1;
    }

    ret = avcodec_receive_packet(this->enc_ctx, pkt);
    ec_lock.unlock();

    return ret;
}

Encoder_class_param Encoder_class::get_now_param()
{
    return this->enc_param;
}

AVCodecContext* Encoder_class::get_ctx()
{
    return this->enc_ctx;
}
//================================================
//private
int Encoder_class::Encode_Create()
{
    int ret = create_my_encode(&this->tmp_ctx, &this->enc_param);
    if(ret < 0)
    {
        return ret;
    }

    if(this->enc_ctx != nullptr)
    {
        this->Encode_Delete();
    }

    this->enc_ctx = this->tmp_ctx;
    this->tmp_ctx = nullptr;
    return 0;
}

int Encoder_class::Encode_Delete()
{
    if(this->enc_ctx != nullptr)
    {
        avcodec_free_context(&(this->enc_ctx));
        this->enc_ctx = nullptr;
    }
    return 0;
}

int Encoder_class::Encode_Update(Encoder_class_param fmt)
{
    enc_param = fmt;
    return Encode_Create();
}



