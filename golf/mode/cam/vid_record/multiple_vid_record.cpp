#include "multiple_vid_record.h"
//=================================================
/*
 * error code
 * -1000 create output context fail
 * -1001 open context file fail
 * -1002 not found data codec
 * -1003 create new stream fail
 * -1004 open stream codec context fail
 * -1005 stream codecpar copy from stream codec context fail
 * -1006 output file write header context fail
 *
 * -2000 write frame fail
*/
//=================================================

multiple_vid_record::multiple_vid_record()
{
    file_write_suscess = false;
    out_fmt_ctx = nullptr;
    stream = nullptr;
    a_stream = nullptr;
    a_index = -1;

    av_init_packet(&au_pack);
}

multiple_vid_record::~multiple_vid_record()
{
    file_lock.lock();
    release_file();
    file_lock.unlock();
}

//=================================================
//public
int multiple_vid_record::create(const char* output_file, const AVCodecContext* ctx)
{
    int ret;

    file_lock.lock();
    release_file();
    file_lock.unlock();

    codec_ctx = ctx;

    file_lock.lock();
    ret = create_file(output_file);
    file_lock.unlock();

    return ret; //
}

int multiple_vid_record::release()
{
    int ret;

    file_lock.lock();
    ret = release_file();
    file_lock.unlock();

    return ret;
}

int multiple_vid_record::write_pkt_data(uint8_t *data, size_t len)
{
    file_lock.lock();

    if(!file_write_suscess)
    {
        file_lock.unlock();
        return -1;
    }

    packet->data = data;
    packet->size = len;

    /*
     * av_rescale_q(a, b, c) == a * (b.num / b.den) / (c.num / c.den) or a * (b.num * c.den) / (b.den * c.num)
     * packet pts = record_cnt * 1(codec_ctx.num) * 1,000,000us(stream.den) / (60fps(codec_ctx.den) * 1(stream.num) )
    */

    /*packet->pts = av_rescale_q(record_cnt, codec_ctx->time_base, stream->time_base);
    packet->dts = packet->pts;
    packet->duration = av_rescale_q(1, codec_ctx->time_base, stream->time_base);*/

    packet->pts = record_cnt;
    packet->pts *=v_pkt_duration;
    packet->dts = packet->pts;
    packet->duration =v_pkt_duration;
    packet->stream_index = stream->index;


    av_packet_rescale_ts(packet, codec_ctx->time_base, this->stream->time_base);

    packet->stream_index = stream->index;

    record_cnt++;
    if (av_interleaved_write_frame(out_fmt_ctx, packet) < 0)
    {
        file_lock.unlock();
        return -2000;
    }
    file_lock.unlock();
    return 0;
}

int multiple_vid_record::write_pkt_au(uint8_t *data, size_t len)
{
    file_lock.lock();

    if(!file_write_suscess)
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

    if (av_interleaved_write_frame(out_fmt_ctx, &au_pack) < 0)
    {
        file_lock.unlock();
        return -1;
    }

    a_record_cnt++;
    file_lock.unlock();
    return 0;
}

bool multiple_vid_record::is_open()
{
    bool ret =false;

    if(file_write_suscess)
        ret = true;

    return ret;
}

//=================================================
static void print_info(const AVCodecParameters * par)
{
    printf("============info=============\n");
    printf("par->codec_type %d \n", par->codec_type);
    printf("par->codec_id %d \n", par->codec_id);
    printf("par->codec_tag %u \n", par->codec_tag);
    printf("\n");


    printf("par->bit_rate %lld \n", par->bit_rate);
    printf("par->bits_per_coded_sample %d \n", par->bits_per_coded_sample);
    printf("par->bits_per_raw_sample %d \n", par->bits_per_raw_sample);
    printf("par->profile %d \n", par->profile);
    printf("par->level %d \n",  par->level);
    printf("\n");

    //par->profile = FF_PROFILE_HEVC_MAIN;

    printf("par->format %d \n", par->format);
    printf("par->width %d \n",  par->width);
    printf("par->height %d \n", par->height);
    printf("par->field_order %d \n", par->field_order);
    printf("par->color_range %d \n", par->color_range);
    printf("par->color_primaries %d \n", par->color_primaries);
    printf("par->color_trc %d \n", par->color_trc);
    printf("par->color_space %d \n", par->color_space);
    printf("par->chroma_location %d \n", par->chroma_location);
    printf("par->sample_aspect_ratio [num %d] [den %d] \n", par->sample_aspect_ratio.num , par->sample_aspect_ratio.den);
    printf("par->video_delay %d \n", par->video_delay);
    printf("par->extradata_size %d \n", par->extradata_size);
    printf("AV_INPUT_BUFFER_PADDING_SIZE %d \n", AV_INPUT_BUFFER_PADDING_SIZE);
    printf("\n");
    printf("=============================\n");
}

//private
int multiple_vid_record::create_file(const char* output_path)
{
    file_write_suscess = false;

    //open file
    if (avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, output_path) < 0)
    {
        return -1000;
    }

    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open2( &(out_fmt_ctx)->pb, output_path, AVIO_FLAG_WRITE, nullptr, nullptr) < 0)
        {
            release_file();
            return -1001;
        }
    }

    //video stream create & setting
    stream = avformat_new_stream(out_fmt_ctx, codec_ctx->codec);
    if (!stream)
    {
        release_file();
        return -1003;
    }

    stream->time_base.num = 1;
    stream->time_base.den = codec_ctx->time_base.den;
    stream->r_frame_rate.num = codec_ctx->time_base.den;
    stream->r_frame_rate.den = 1;

    if(avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0)
    {
        release_file();
        return -1005;
    }

    //--------

    a_stream = avformat_new_stream(out_fmt_ctx, nullptr);
    if (!a_stream) {
        release_file();
        return -1;
    }

    AVCodec* audioCodec = avcodec_find_encoder_by_name("aac");
    a_CodecContext = avcodec_alloc_context3(audioCodec);
    a_CodecContext->sample_rate = 48000;
    a_CodecContext->channels = 2;
    a_CodecContext->sample_fmt = AV_SAMPLE_FMT_FLT;
    a_CodecContext->time_base.den  = 48000;
    a_CodecContext->time_base.num  = 1;

    avcodec_parameters_from_context(a_stream->codecpar, a_CodecContext);
    a_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    this->a_index = a_stream->index;

    a_stream->codec->sample_rate = 48000;
    a_stream->codec->channels = 2;
    a_stream->codec->sample_fmt = AV_SAMPLE_FMT_FLT;
    a_stream->codec->time_base.den  = 48000;
    a_stream->codec->time_base.num  = 1;
    a_stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;


    //--------

    //write file header
    if (avformat_write_header(out_fmt_ctx, nullptr) < 0)
    {
        release_file();
        return -1006;
    }

    av_dump_format(out_fmt_ctx, 0, output_path, 1);

    //packet init
    packet = av_packet_alloc();
    av_init_packet(packet);
    record_cnt = 0;

    v_rate = av_make_q(codec_ctx->time_base.den, 1);
    v_pkt_duration = (
                (codec_ctx->time_base.den * this->v_rate.den) /
                (codec_ctx->time_base.num * this->v_rate.num)
                );

    file_write_suscess = true;

    return 0;
}

int multiple_vid_record::release_file()
{
    if(packet != nullptr)
    {
        av_packet_unref(packet);
        av_packet_free(&(packet));
        packet = nullptr;
    }

    if(out_fmt_ctx != nullptr)
    {
        if(out_fmt_ctx->pb != nullptr && file_write_suscess)
        {
            av_write_trailer(out_fmt_ctx);
        }
        avformat_close_input(&out_fmt_ctx);
        avformat_free_context(out_fmt_ctx);
        out_fmt_ctx = nullptr;
        stream = nullptr;
    }

    if(a_CodecContext != nullptr)
    {
       avcodec_free_context(&(this->a_CodecContext));
       a_CodecContext = nullptr;
    }


    record_cnt = 0;
    codec_ctx = nullptr;
    file_write_suscess = false;

    this->a_stream = nullptr;
    this->a_index = -1;

    return 0;
}
//=================================================
