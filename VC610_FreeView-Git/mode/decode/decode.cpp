#include "decode.h"

//================================================
/*
 * error code
 * -10001 decoder codec not found
 * -10002 decoder context create fail
 * -10003 decoder hardware device create fail
 * -10004 decoder open fail
 * -10005 decoder hardware open different decoder (cuda fail)
 *
 * -20001 decoder get hwframe fail
 * -20002 decoder hwframe to frame fail
*/
//================================================
Decoder_class::Decoder_class()
{
    dec_ctx = nullptr;
    tmp_ctx = nullptr;

    hw_ctx = nullptr;
    tmp_hw_ctx = nullptr;

    dec_GPU_frame = nullptr;
}

Decoder_class::~Decoder_class()
{
    release();
}

//================================================
static int create_my_decode_PktMode(AVCodecContext** dec_ctx, AVBufferRef** hw_ctx, Decoder_class_param* param)
{
    ///////////////convert param to PktMode/////////////
    AVCodecID codec_id;
    std::string codec_str;
    switch (param->codec_type)
    {
    case Decoder_class_CodecType::Decoder_class_H264:
        codec_id = AV_CODEC_ID_H264;
        codec_str = "h264_cuvid";
        break;

    case Decoder_class_CodecType::Decoder_class_H265:
        codec_id = AV_CODEC_ID_HEVC;
        codec_str = "hevc_cuvid";
        break;
    }

    ////////////////////////////////////////////////////
    //Codec
    AVCodec* codec = nullptr;

    //codec = avcodec_find_decoder_by_name(codec_str.c_str());
    if(!codec)
    {
        codec = avcodec_find_decoder(codec_id);
        if(!codec)
        {
            return -10001;
        }
    }


    //CodecContext
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if(!ctx)
    {
        return -10002;
    }


    //HW device
    AVBufferRef* hw_device_ctx = nullptr;


    int ret = av_hwdevice_ctx_create(&hw_device_ctx, AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
    if(ret < 0)
    {
        av_buffer_unref(&(hw_device_ctx));
        avcodec_free_context(&ctx);
        return -10003;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    ctx->pix_fmt = AV_PIX_FMT_CUDA;

    //open decoder
    if (avcodec_open2(ctx, codec, nullptr) < 0)
    {
        av_buffer_unref(&(hw_device_ctx));
        avcodec_free_context(&ctx);
        return -10004;
    }

    *dec_ctx = ctx;
    *hw_ctx = hw_device_ctx;
    return 0;
}

static int create_my_decode(AVCodecContext** dec_ctx, AVBufferRef** hw_ctx, Decoder_class_param* param)
{
    if(param->mode == Decoder_class_PacketMode)
    {
        return create_my_decode_PktMode(dec_ctx, hw_ctx, param);
    }

    return -1;
}


//================================================
//public

//create
int Decoder_class::create()
{
    return Decode_Create();
}

int Decoder_class::create(Decoder_class_param param)
{
    this->param = param;
    return Decode_Create();
}


//release
int Decoder_class::release()
{
    return Decode_Delete();
}


//
int Decoder_class::send_decode_pkt(AVPacket *pkt)
{
    return avcodec_send_packet(this->dec_ctx, pkt);
}

int Decoder_class::get_decode_frame(AVFrame *frame)
{

    if(avcodec_receive_frame(this->dec_ctx, this->dec_GPU_frame) != 0)
    {
        return -20001;
    }


    if(this->dec_GPU_frame->format != AV_PIX_FMT_CUDA)
    {
        av_frame_move_ref(frame, this->dec_GPU_frame);
        return 0;
    }

    if(av_hwframe_transfer_data(frame, this->dec_GPU_frame, 0) < 0)
    {
        return -20002;
    }

    return 0;
}

int Decoder_class::get_decode_frame_GPU(AVFrame *frame)
{

    if(avcodec_receive_frame(this->dec_ctx, frame) != 0)
    {
        return -20001;
    }


    return 0;
}


//================================================
//private
int Decoder_class::Decode_Create()
{
    int ret = create_my_decode(&this->tmp_ctx, &this->tmp_hw_ctx, &this->param);
    if(ret < 0)
    {
        return ret;
    }

    if(this->dec_ctx != nullptr)
    {
        this->Decode_Delete();
    }

    dec_GPU_frame = av_frame_alloc();

    this->dec_ctx = this->tmp_ctx;
    this->hw_ctx = this->tmp_hw_ctx;

    this->tmp_ctx = nullptr;
    this->tmp_hw_ctx = nullptr;
    return 0;
}


int Decoder_class::Decode_Delete()
{
    if(this->dec_ctx != nullptr)
    {
        avcodec_free_context(&(this->dec_ctx));
    }

    while(this->hw_ctx != nullptr)
    {
        av_buffer_unref(&this->hw_ctx);
    }

    if(dec_GPU_frame != nullptr)
    {
        av_frame_free(&(this->dec_GPU_frame));
    }

    return 0;
}
