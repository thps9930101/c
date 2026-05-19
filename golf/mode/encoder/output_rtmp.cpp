#include "output_rtmp.h"


using clock_type = std::chrono::high_resolution_clock;
using milli_type = std::chrono::duration<double,std::milli>;
using micro_type = std::chrono::duration<double,std::micro>;

//auto t1 = clock_type::now();
//printf("time3 [%lf]\n",milli_type(t2-t1).count());
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;


_time statr_time;
_time end_time;
milli_type cost_time;
output_rtmp::output_rtmp()
{
    formatContext = nullptr;
    v_stream = nullptr;
    a_stream = nullptr;
    v_index = -1;
    a_index = -1;

    release();

    av_init_packet(&au_pack);

}

output_rtmp::~output_rtmp()
{
    release();
}

int output_rtmp::create(const char *output_rtmp, Encoder_param en_par,AVCodecContext *CodecContext)
{
    int ret;

    release();

    rtmp_lock.lock();
    av_register_all();
    avformat_network_init();

    ret = avformat_alloc_output_context2(&formatContext, NULL, "flv", output_rtmp);
    if(ret < 0)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtmp_lock.unlock();
        return -1;
    }

    av_dict_set(&formatContext->metadata, "fflags", "nobuffer", 0);
    av_dict_set(&formatContext->metadata, "analyzeduration", "0", 0);

    v_stream = avformat_new_stream(formatContext, NULL);
    if(!v_stream)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        v_stream = nullptr;
        rtmp_lock.unlock();
        return -1;
    }

    ret = avcodec_parameters_from_context(v_stream->codecpar, CodecContext);
    if(ret < 0)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        v_stream = nullptr;
        rtmp_lock.unlock();
        return -1;
    }
    v_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->v_index = v_stream->index;

//----
    a_stream = avformat_new_stream(formatContext, nullptr);
    if (!a_stream) {
        avformat_free_context(formatContext);
        rtmp_lock.unlock();
        return -1;
    }
    AVCodec *audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audioCodec) {
        rtmp_lock.unlock();
        return -1;
    }

    a_CodecContext = avcodec_alloc_context3(audioCodec);
    if (!a_CodecContext) {
        rtmp_lock.unlock();
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
        rtmp_lock.unlock();
        return -1;
    }

    if (avcodec_parameters_from_context(a_stream->codecpar, a_CodecContext) < 0) {
        rtmp_lock.unlock();
        return -1;
    }

//----

    if (avio_open2(&formatContext->pb, output_rtmp, AVIO_FLAG_WRITE,NULL,NULL) < 0) {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtmp_lock.unlock();
        return -1;
    }

    ret = avformat_write_header(formatContext, NULL );
    if(ret < 0)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
        rtmp_lock.unlock();
        return -1;
    }

    this->v_pkt_duration = (double)this->v_stream->time_base.den / (double)CodecContext->time_base.den ;


    encoder_par = en_par;
    v_CodecContext = CodecContext;

    rt_state = rtmp_state::rtmp_create;
    v_rtmp_cnt = 0;
    a_rtmp_cnt = 0;
    rtmp_lock.unlock();
    return 0 ;
}

int output_rtmp::write_frame(AVPacket *av_pack)
{
    int ret;

    rtmp_lock.lock();

    if(rt_state == rtmp_state::rtmp_release)
    {
        rtmp_lock.unlock();
        return -1;
    }  

    av_pack->pts = v_rtmp_cnt;
    av_pack->pts *= this->v_pkt_duration;
    av_pack->dts = av_pack->pts;
    av_pack->duration = this->v_pkt_duration;
    av_pack->stream_index = this->v_index;

    v_rtmp_cnt++;
    ret = av_interleaved_write_frame(formatContext,av_pack);
    if(ret <0)
    {
        rtmp_lock.unlock();
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        printf("rtmp Error: %s\n", errbuf);
        return -1;
    }
    rtmp_lock.unlock();
    return 0 ;

}

int output_rtmp::write_au(AVPacket *av_pack)
{
    rtmp_lock.lock();

   if(rt_state == rtmp_state::rtmp_release)
    {
        rtmp_lock.unlock();
        return -1;
    }

    av_pack->pts = a_rtmp_cnt;
    av_pack->pts *= 1024;
    av_pack->dts = av_pack->pts;
    av_pack->duration = 1024;
    av_pack->stream_index = this->a_index;
    av_packet_rescale_ts(av_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->formatContext, av_pack) < 0)
    {
        rtmp_lock.unlock();
        return -1;
    }

    a_rtmp_cnt++;
    printf("a_rtmp_cnt %llu \n",a_rtmp_cnt);
    rtmp_lock.unlock();
    return 0;
}

int output_rtmp::write_au(uint8_t * data,uint32_t  len)
{
    rtmp_lock.lock();

    if(rt_state == rtmp_state::rtmp_release)
    {
        rtmp_lock.unlock();
        return -1;
    }

    au_pack.data = data;
    au_pack.size = len;

    au_pack.pts = a_rtmp_cnt;
    au_pack.pts *= 1024;
    au_pack.dts = au_pack.pts;
    au_pack.duration = 1024;
    au_pack.stream_index = this->a_index;

    av_packet_rescale_ts(&au_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->formatContext, &au_pack) < 0)
    {
        rtmp_lock.unlock();
        return -1;
    }

    a_rtmp_cnt++;
    rtmp_lock.unlock();
    return 0;
}


int output_rtmp::release()
{
    rtmp_lock.lock();
    if(formatContext != nullptr)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;

    }

    if(v_stream !=nullptr)
        v_stream = nullptr;

    if(a_stream != nullptr)
        a_stream = nullptr;

    v_index = -1;
    a_index = -1;

    rt_state = rtmp_state::rtmp_release;
    rtmp_lock.unlock();
    return 0;
}

bool output_rtmp::is_open()
{
    bool ret =false;

    if(rt_state == rtmp_state::rtmp_create)
        ret = true;

    return ret;
}
