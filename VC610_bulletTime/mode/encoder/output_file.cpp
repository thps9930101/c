#include "output_file.h"


//=================================================
/*
 * error code
 * -1000 not set encoder
 * -1001 create output context fail
 * -1002 create new stream fail
 * -1003 set stream param from ctx fail
 * -1004 open output stream & file fail
 * -1005 output file does not any stream
 * -1006 output file write header context fail
*/
//=================================================
output_file::output_file()
{
    format_ctx = nullptr;
    v_stream = nullptr;
    a_stream = nullptr;
    v_index = -1;
    a_index = -1;

    av_init_packet(&au_pack);
    fi_state = file_state::file_release;
}

output_file::~output_file()
{
    release();
}
//=================================================
//public

//create
int output_file::create(const char *output_path, Encoder_param enc,AVCodecContext *CodecContext)
{
    int ret;

    file_lock.lock();
    my_encoder = enc;
    this->v_CodecContext = CodecContext;
    ret = create_file(output_path,v_CodecContext);
    file_lock.unlock();
    return ret;
}

//release
int output_file::release()
{
    int ret;

    file_lock.lock();
    ret = release_file();
    file_lock.unlock();

    return ret;
}

bool output_file::is_open()
{
    bool ret =false;

    if(fi_state == file_state::file_create)
        ret = true;

    return ret;
}

//write
int output_file::write_frame(AVPacket *av_pack)
{
    file_lock.lock();

    if(fi_state == file_state::file_release)
    {
        file_lock.unlock();
        return -1;
    }

    av_pack->pts = v_record_cnt;
    av_pack->pts *=this->v_pkt_duration;
    av_pack->dts = av_pack->pts;
    av_pack->duration =this->v_pkt_duration;
    av_pack->stream_index = this->v_index;

    av_packet_rescale_ts(av_pack, v_CodecContext->time_base, this->v_stream->time_base);

    if (av_interleaved_write_frame(this->format_ctx, av_pack) < 0)
    {
        file_lock.unlock();
        return -1;
    }

    v_record_cnt++;
    file_lock.unlock();
    return 0;
}

int output_file::write_au(AVPacket *av_pack)
{
    file_lock.lock();

    if(fi_state == file_state::file_release)
    {
        file_lock.unlock();
        return -1;
    }

    av_pack->pts = a_record_cnt;
    av_pack->pts *= 1024;
    av_pack->dts = av_pack->pts;
    av_pack->duration = 1024;
    av_pack->stream_index = this->a_index;
    av_packet_rescale_ts(av_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->format_ctx, av_pack) < 0)
    {
        file_lock.unlock();
        return -1;
    }

    a_record_cnt++;
    file_lock.unlock();
    return 0;
}

int output_file::write_au(uint8_t * data,uint32_t  len)
{
    file_lock.lock();

    if(fi_state == file_state::file_release)
    {
        file_lock.unlock();
        return -1;
    }

    au_pack.data = data;
    au_pack.size = len;

    au_pack.pts = a_record_cnt;
    au_pack.pts *= 1024;
    au_pack.dts = au_pack.pts;
    au_pack.duration = 1024;
    au_pack.stream_index = this->a_index;

    av_packet_rescale_ts(&au_pack, a_CodecContext->time_base, this->a_stream->time_base);

    if (av_interleaved_write_frame(this->format_ctx, &au_pack) < 0)
    {
        file_lock.unlock();
        return -1;
    }

    a_record_cnt++;
    file_lock.unlock();
    return 0;
}


//=================================================
//private
int output_file::create_file(const char* output_path,AVCodecContext *CodecContext)
{
    release_file();

    //format
    int ret;
    AVFormatContext* out_ftx;

    ret = avformat_alloc_output_context2(&out_ftx, nullptr, nullptr, output_path);
    if(ret < 0)
    {
        return -1001;
    }

    //stream
    AVStream* out_stream;
    out_stream = avformat_new_stream(out_ftx, nullptr);
    if(!out_stream)
    {
        avformat_free_context(out_ftx);
        return -1002;
    }

    out_stream->time_base.num = 1;
    out_stream->time_base.den = my_encoder.fps;
    out_stream->r_frame_rate.num = my_encoder.fps;
    out_stream->r_frame_rate.den = 1;

    ret = avcodec_parameters_from_context(out_stream->codecpar, CodecContext);
    if(ret < 0)
    {
        avformat_free_context(out_ftx);
        return -1003;
    }

    for(int i = 0; i < out_ftx->nb_streams; i++)
    {
        if(out_ftx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && this->v_index == -1)
        {
            this->v_index = i;
        }
    }

    a_stream = avformat_new_stream(out_ftx, nullptr);
    if (!a_stream) {
        avformat_free_context(out_ftx);
        return -1;
    }
    AVCodec *audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audioCodec) {
        return -1;
    }

    a_CodecContext = avcodec_alloc_context3(audioCodec);
    if (!a_CodecContext) {
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
        return -1;
    }

    if (avcodec_parameters_from_context(a_stream->codecpar, a_CodecContext) < 0) {
        return -1;
    }


    //open output stream, open output file
    if (!(out_ftx->flags & AVFMT_NOFILE))
    {
        if (avio_open2( &(out_ftx)->pb, output_path, AVIO_FLAG_WRITE, nullptr, nullptr) < 0)
        {
            avformat_free_context(out_ftx);
            return -1004;
        }
    }

    if(!out_ftx->nb_streams)
    {
        avformat_free_context(out_ftx);
        return -1005;
    }

    //write header
    ret = avformat_write_header(out_ftx, nullptr);
    if(ret < 0)
    {
        avio_close(out_ftx->pb);
        avformat_free_context(out_ftx);
        return -1006;
    }

    av_dump_format(out_ftx, 0, output_path, 1);
    this->v_stream = out_stream;
    this->format_ctx = out_ftx;
    this->v_rate = av_make_q(my_encoder.fps, 1);
    this->v_pkt_duration = (
                (CodecContext->time_base.den * this->v_rate.den) /
                (CodecContext->time_base.num * this->v_rate.num)
                );


    v_record_cnt = 0;
    a_record_cnt = 0;
    fi_state = file_state::file_create;
    return 0;
}

int output_file::release_file()
{
    if(this->format_ctx != nullptr)
    {
        if(this->format_ctx->pb != nullptr)
        {
            av_write_trailer(this->format_ctx);
            avio_close(this->format_ctx->pb);
        }

        avformat_free_context(this->format_ctx);
        avcodec_free_context(&(this->a_CodecContext));

        this->format_ctx = nullptr;
        this->a_CodecContext = nullptr;

        this->v_stream = nullptr;
        this->a_stream = nullptr;
    }

    this->v_index = -1;
    this->a_index = -1;
    fi_state = file_state::file_release;

    return 0;
}

//=================================================
