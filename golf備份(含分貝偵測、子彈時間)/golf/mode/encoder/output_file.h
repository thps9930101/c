#ifndef OUTPUT_FILE_H
#define OUTPUT_FILE_H

#include <string>
#include <QImage>
#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
}
#include "Data/Struct/Encoder_param/encoder_param.h"

class output_file
{
public:
    output_file();
    ~output_file();
    int create(const char* output_path, Encoder_param enc,AVCodecContext *CodecContext);
    int release();
    bool is_open();

    int write_frame(AVPacket *av_pack);
    int write_au(AVPacket *av_pack);
    int write_au(uint8_t * data,uint32_t len);
private:
    int create_file(const char* output_path,AVCodecContext *CodecContext);
    int release_file();

public:

private:
    enum file_state {
        file_create = 0,
        file_release
    };


    Encoder_param my_encoder;

    std::mutex  file_lock;
    file_state fi_state;

    AVFormatContext* format_ctx;

    //----------------------------------
    //video
    AVCodecContext *v_CodecContext = nullptr;
    AVStream* v_stream = nullptr;

    int v_index;

    AVRational v_rate;
    int64_t v_pkt_duration;

    int64_t v_record_cnt = 0;

    //----------------------------------
    //audio
    AVCodecContext* a_CodecContext = nullptr;
    AVStream* a_stream = nullptr;

    int a_index;

    int64_t a_record_cnt = 0;


    AVPacket au_pack;


};

#endif // OUTPUT_FILE_H
