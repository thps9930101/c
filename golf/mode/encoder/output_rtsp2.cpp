#include "output_rtsp2.h"

output_rtsp::output_rtsp()
{
    formatContext = nullptr;
    v_stream = nullptr;
    a_stream = nullptr;
    v_index = -1;
    a_index = -1;

    release();

    av_init_packet(&au_pack);

}

output_rtsp::~output_rtsp()
{
    release();
}

int output_rtsp::create(const char *output_rtsp, Encoder_param en_par,AVCodecContext *CodecContext)
{
    int ret;

    release();

    rtsp_lock.lock();
    av_register_all();
    avformat_network_init();


    ret = avformat_alloc_output_context2(&formatContext, NULL, "rtsp", output_rtsp);
    //ret = avformat_alloc_output_context2(&formatContext, outputFormat, nullptr, output_rtsp);
    if(ret < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }

    av_dict_set(&formatContext->metadata, "fflags", "nobuffer", 0);

    v_stream = avformat_new_stream(formatContext, NULL);
    if(!v_stream)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtsp_lock.unlock();
        return -1;
    }


    ret = avcodec_parameters_from_context(v_stream->codecpar, CodecContext);
    if(ret < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }
    v_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->v_index = v_stream->index;

    a_stream = avformat_new_stream(formatContext, nullptr);
    if (!a_stream) {
        avformat_free_context(formatContext);
        rtsp_lock.unlock();
        return -1;
    }
    AVCodec *audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audioCodec) {
        rtsp_lock.unlock();
        return -1;
    }

    a_CodecContext = avcodec_alloc_context3(audioCodec);
    if (!a_CodecContext) {
        rtsp_lock.unlock();
        return -1;
    }

    a_CodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    a_CodecContext->sample_rate = 48000;
    a_CodecContext->channels = 2;
    a_CodecContext->time_base.den  = 48000;
    a_CodecContext->time_base.num  = 1;
    a_CodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
    a_CodecContext->channel_layout = av_get_default_channel_layout(a_CodecContext->channels);
    this->a_index = a_stream->index;

    if (avcodec_open2(a_CodecContext, audioCodec, nullptr) < 0) {
        rtsp_lock.unlock();
        return -1;
    }

    if (avcodec_parameters_from_context(a_stream->codecpar, a_CodecContext) < 0) {
        rtsp_lock.unlock();
        return -1;
    }

    AVDictionary* opts1 = NULL;
    av_dict_set(&opts1, "stimeout", "500000", 0);
    ret = avformat_write_header(formatContext, &opts1 );
    if(ret < 0)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtsp_lock.unlock();
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        printf("Error occurred: %s\n", errbuf);
        return -1;
    }



    this->v_pkt_duration = this->v_stream->time_base.den / CodecContext->time_base.den ;
//    this->a_pkt_duration = this->a_stream->time_base.den / a_CodecContext->time_base.den;
//    printf("v_pkt_duration [%lld] v_stream->time_base.den [%d] CodecContext->time_base.den [%d]\n",v_pkt_duration,v_stream->time_base.den,CodecContext->time_base.den);
//    printf("a_pkt_duration [%lld] a_stream->time_base.den [%d] CodecContext->time_base.den [%d]\n",a_pkt_duration,a_stream->time_base.den,a_CodecContext->time_base.den);


    encoder_par = en_par;
    v_CodecContext = CodecContext;

    rt_state = rtsp_state::rtsp_create;
    v_rtsp_cnt = 0;
    a_rtsp_cnt = 0;
    rtsp_lock.unlock();

    return 0 ;
}

int output_rtsp::write_frame(AVPacket *av_pack)
{
    int ret;

    rtsp_lock.lock();

    if(rt_state == rtsp_state::rtsp_release)
    {
        rtsp_lock.unlock();
        return -1;
    }

    av_pack->pts = v_rtsp_cnt;
    av_pack->pts *= this->v_pkt_duration;
    av_pack->dts = av_pack->pts;
    av_pack->duration = this->v_pkt_duration;
    av_pack->stream_index = this->v_index;
    //av_packet_rescale_ts(av_pack, v_CodecContext->time_base, this->v_stream->time_base);


    v_rtsp_cnt++;
    ret = av_interleaved_write_frame(formatContext,av_pack);
    if(ret <0)
    {
        rtsp_lock.unlock();
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        printf("rtsp Error: %s\n", errbuf);
        return -1;
    }

    rtsp_lock.unlock();

    return 0 ;

}

int output_rtsp::write_au(AVPacket *av_pack)
{
    rtsp_lock.lock();

    if(rt_state == rtsp_state::rtsp_release)
    {
        rtsp_lock.unlock();
        return -1;
    }

    av_pack->pts = a_rtsp_cnt;
    av_pack->pts *= 1024;
    av_pack->dts = av_pack->pts;
    av_pack->duration = 1024;
    av_pack->stream_index = this->a_index;
    av_packet_rescale_ts(av_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->formatContext, av_pack) < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }

    a_rtsp_cnt++;
    rtsp_lock.unlock();
    return 0;
}

int output_rtsp::write_au(uint8_t * data,uint32_t  len)
{
    rtsp_lock.lock();

    if(rt_state == rtsp_state::rtsp_release)
    {
        rtsp_lock.unlock();
        return -1;
    }

    au_pack.data = data;
    au_pack.size = len;

    au_pack.pts = a_rtsp_cnt;
    au_pack.pts *= 1024;
    au_pack.dts = au_pack.pts;
    au_pack.duration = 1024;
    au_pack.stream_index = this->a_index;

    av_packet_rescale_ts(&au_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->formatContext, &au_pack) < 0)
    {
        rtsp_lock.unlock();
        return -1;
    }

    a_rtsp_cnt++;
    rtsp_lock.unlock();
    return 0;
}

int output_rtsp::release()
{
    rtsp_lock.lock();
    if(formatContext != nullptr)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;

    }

    if(v_stream != nullptr)
        v_stream = nullptr;

    if(a_stream != nullptr)
        a_stream = nullptr;

    v_index = -1;
    a_index = -1;

    rt_state = rtsp_state::rtsp_release;
    rtsp_lock.unlock();
    return 0;
}

bool output_rtsp::is_open()
{
    bool ret =false;

    if(rt_state == rtsp_state::rtsp_create)
        ret = true;

    return ret;
}
