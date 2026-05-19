#include "output_rtsp.h"

output_rtsp::output_rtsp()
{
    formatContext = nullptr;
    codecContext = nullptr;
    videoStream = nullptr;

    avTemp = nullptr;

    frame_count = 0;

    en_avpkt = nullptr;
}

output_rtsp::~output_rtsp()
{
    release();
}


int output_rtsp::create(const char *output_rtsp, Encoder_param en_par)
{
    int ret;

    release();

    rtsp_lock.lock();
    av_register_all();
    avformat_network_init();

    ret = avformat_alloc_output_context2(&formatContext, NULL, "rtsp", output_rtsp);
    if(ret < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }

    videoStream = avformat_new_stream(formatContext, NULL);
    if(!videoStream)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtsp_lock.unlock();
        return -1;
    }

    AVCodec* videoCodec;

    if(en_par.en_format == Encoder_class_format::Encoder_class_h264_nvenc)
        videoCodec = avcodec_find_encoder_by_name("h264_nvenc");

    if(!videoCodec)
    {
        rtsp_lock.unlock();
        return -1;
    }

    codecContext = avcodec_alloc_context3(videoCodec);
    if(!codecContext)
    {
        rtsp_lock.unlock();
        return -1;
    }

    codecContext->width = en_par.width;
    codecContext->height = en_par.height;
    codecContext->pix_fmt = en_par.pix_format;//AV_PIX_FMT_YUV444P;//AV_PIX_FMT_CUDA;//AV_PIX_FMT_NV12; //AV_PIX_FMT_YUV420P;
    codecContext->time_base.num = 1;
    codecContext->time_base.den = en_par.fps;

    codecContext->framerate.num = en_par.fps;
    codecContext->framerate.den = 1;

    codecContext->gop_size = en_par.gop;
    codecContext->bit_rate = en_par.bitrate;

    ret = avcodec_parameters_from_context(videoStream->codecpar, codecContext);
    if(ret < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }

    videoStream->time_base.num = 1;
    videoStream->time_base.den = en_par.fps;

    AVDictionary *options = NULL;
    //av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "rtsp_transport", "udp", 0);

    if(avcodec_open2(codecContext, videoCodec, &options)<0)
    {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
        rtsp_lock.unlock();
        return -1;
    }

    AVDictionary* opts = NULL;
    av_dict_set(&opts, "stimeout", "500000", 0);
    ret = avformat_write_header(formatContext, &opts );
    if(ret < 0)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtsp_lock.unlock();
        return -1;
    }

    avTemp= av_frame_alloc();
    avTemp->format = en_par.pix_format;
    avTemp->width = en_par.width;
    avTemp->height = en_par.height;
    av_frame_get_buffer(avTemp, 0);

    en_avpkt = av_packet_alloc();
    av_init_packet(en_avpkt);

    frame_count = 0;
    encoder_par = en_par;

    rt_state = rtsp_state::rtsp_create;
    rtsp_lock.unlock();
    return 0 ;
}


int output_rtsp::write_frame(uint8_t *cpu_nv12)
{
    int ret;

    rtsp_lock.lock();

    if(rt_state == rtsp_state::rtsp_release)
    {
        rtsp_lock.unlock();
        return -1;
    }

    int y_size = avTemp->width * avTemp->height;
    int uv_size = y_size / 2;

    avTemp->data[0] = cpu_nv12 ;
    avTemp->data[1] = cpu_nv12+y_size;

    avTemp->pts = frame_count;

    ret = avcodec_send_frame(codecContext, avTemp);
    if(ret < 0 )
    {
        rtsp_lock.unlock();
        return -1;
    }

    ret = avcodec_receive_packet(codecContext, en_avpkt);
    if(ret < 0 )
    {
        rtsp_lock.unlock();
        return -1;
    }

    en_avpkt->pts *= (90000 / encoder_par.fps);
    en_avpkt->dts = en_avpkt->pts;
    en_avpkt->duration=90000 / encoder_par.fps;
    en_avpkt->pos = -1;

    ret = av_interleaved_write_frame(formatContext,en_avpkt);
    if(ret <0)
    {
        rtsp_lock.unlock();
        av_packet_unref(en_avpkt);
        return -1;
    }

    av_packet_unref(en_avpkt);
    frame_count++;
    rtsp_lock.unlock();
    return 0 ;

}


int output_rtsp::write_frame(AVFrame *av_frame)
{
    int ret;

    rtsp_lock.lock();

    if(rt_state == rtsp_state::rtsp_release)
    {
        rtsp_lock.unlock();
        return -1;
    }

    av_frame->pts = frame_count;

    ret = avcodec_send_frame(codecContext, avTemp);
    if(ret < 0 )
    {
        rtsp_lock.unlock();
        return -1;
    }

    ret = avcodec_receive_packet(codecContext, en_avpkt);
    if(ret < 0 )
    {
        rtsp_lock.unlock();
        return -1;
    }

    en_avpkt->pts *=(90000 / encoder_par.fps);
    en_avpkt->dts = en_avpkt->pts;
    en_avpkt->duration=90000 / encoder_par.fps;
    en_avpkt->pos = -1;

    ret = av_interleaved_write_frame(formatContext,en_avpkt);
    if(ret <0)
    {
        av_packet_unref(en_avpkt);
        rtsp_lock.unlock();
        return -1;
    }

    av_packet_unref(en_avpkt);
    frame_count++;
    rtsp_lock.unlock();
    return 0 ;

}


int output_rtsp::release()
{
    rtsp_lock.lock();
    if(formatContext != nullptr)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;

    }

    if(videoStream !=nullptr)
        videoStream = nullptr;

    if(codecContext!= nullptr)
    {
       avcodec_free_context(&(codecContext));
       codecContext = nullptr;
    }


    if(avTemp != nullptr)
    {
        av_frame_free(&avTemp);
        avTemp=nullptr;
    }

    if(en_avpkt != nullptr)
    {
        av_packet_free(&en_avpkt);
        en_avpkt=nullptr;
    }

    rt_state = rtsp_state::rtsp_release;
    rtsp_lock.unlock();
    return 0;
}
