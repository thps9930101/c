#ifndef MULTIPLE_VID_RECORD_H
#define MULTIPLE_VID_RECORD_H

#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}


class multiple_vid_record
{
public:
    multiple_vid_record();
    ~multiple_vid_record();

    int create(const char* output_file, const AVCodecContext* ctx);
    int release();
    bool is_open();

    int write_pkt_data(uint8_t* data, size_t len);
    int write_pkt_au(uint8_t *data, size_t len);

private:
    int create_file(const char* output_path);
    int release_file();

private:
    const AVCodecContext *codec_ctx = nullptr;

    std::mutex  file_lock;

    AVFormatContext *out_fmt_ctx = nullptr;
    AVStream *stream = nullptr;
    AVPacket* packet= nullptr;

    bool file_write_suscess = false;
    int64_t record_cnt = 0;

    AVRational v_rate;
    int64_t v_pkt_duration;


    //audio
    AVCodecContext* a_CodecContext = nullptr;
    AVStream* a_stream = nullptr;

    int a_index;

    int64_t a_record_cnt = 0;


    AVPacket au_pack;

};

#endif // MULTIPLE_VID_RECORD_H
