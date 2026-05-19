#include "AudioAnalyzer.h"
#include <QDebug>
#include <cmath>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AudioAnalyzer::AudioAnalyzer(QObject* parent)
    : QObject(parent)
{
    avcodec_register_all();
}

AudioAnalyzer::~AudioAnalyzer() = default;

AudioAnalysisResult AudioAnalyzer::analyze(const ao::lm_audio_data& input)
{
    if (!input.data || input.len == 0) {
        qWarning() << "無效的音訊資料";
//        return -100.0f;
    }

    QByteArray audioBytes(reinterpret_cast<const char*>(input.data), static_cast<int>(input.len));
    return decodeAndAnalyze(audioBytes);
}

AudioAnalysisResult AudioAnalyzer::decodeAndAnalyze(const QByteArray& data)
{
    AudioAnalysisResult result;

    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!codec) {
        qWarning() << "AAC 解碼器未找到。";
        return result;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    avcodec_open2(ctx, codec, nullptr);
    av_new_packet(packet, data.size());
    memcpy(packet->data, data.constData(), data.size());

    float resultDb = -100.0f;

    if (avcodec_send_packet(ctx, packet) == 0) {
        while (avcodec_receive_frame(ctx, frame) == 0) {
            SwrContext* swr = swr_alloc_set_opts(
                nullptr,
//                AV_CH_LAYOUT_MONO,
                av_get_default_channel_layout(frame->channels),
                AV_SAMPLE_FMT_S16,
                frame->sample_rate,
                av_get_default_channel_layout(frame->channels),
                (AVSampleFormat)frame->format,
                frame->sample_rate,
                0, nullptr);
            swr_init(swr);

            uint8_t* out_buf = nullptr;
            int out_linesize = 0;
            int out_samples = swr_get_out_samples(swr, frame->nb_samples);
//            av_samples_alloc(&out_buf, &out_linesize, 1, out_samples, AV_SAMPLE_FMT_S16, 0);
            av_samples_alloc(&out_buf, &out_linesize, frame->channels, out_samples, AV_SAMPLE_FMT_S16, 0);

            int conv_samples = swr_convert(
                swr, &out_buf, out_samples,
                (const uint8_t**)frame->extended_data, frame->nb_samples);

            int16_t* samples = reinterpret_cast<int16_t*>(out_buf);
//            int total_samples = conv_samples;
            int total_samples = conv_samples * frame->channels;


            // ✅ RMS 計算（分貝）
            double sumSquares = 0.0;
            for (int i = 0; i < total_samples; ++i)
                sumSquares += samples[i] * samples[i];

            double rms = std::sqrt(sumSquares / total_samples);
            result.db = (rms > 0.0)
                ? 20.0f * std::log10(rms / 32767.0f)
                : -100.0f;
//            result.db = std::clamp(result.db, -100.0f, -30.0f);

//            // 映射為 0~100：越大聲 → 越接近 100
//            result.normalized = ((result.db + 100.0f) / 70.0f) * 100.0f;
//            result.normalized = std::clamp(result.normalized, 0.0f, 100.0f);

            // ✅ 將 PCM 數據存入 result 結構
            result.samples.assign(samples, samples + total_samples);
            result.sampleRate = frame->sample_rate;

            // ✅ 若要額外分析頻率，可這樣調用：
            // float freq = analyzeFrequency(samples, total_samples, frame->sample_rate);

            av_freep(&out_buf);
            swr_free(&swr);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&ctx);

    return result;
}

//AudioAnalysisResult AudioAnalyzer::decodeAndAnalyze(const QByteArray& data)
//{
//    AudioAnalysisResult result;

//    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
//    if (!codec) {
//        qWarning() << "AAC 解碼器未找到。";
//        return result;
//    }

//    AVCodecContext* ctx = avcodec_alloc_context3(codec);
//    AVPacket* packet = av_packet_alloc();
//    AVFrame* frame = av_frame_alloc();

//    if (avcodec_open2(ctx, codec, nullptr) < 0) {
//        qWarning() << "AAC 解碼器開啟失敗。";
//        return result;
//    }

//    av_new_packet(packet, data.size());
//    memcpy(packet->data, data.constData(), data.size());

//    QVector<int16_t> fullSamples;
//    int sampleRate = 0;

//    if (avcodec_send_packet(ctx, packet) == 0) {
//        while (avcodec_receive_frame(ctx, frame) == 0) {
//            SwrContext* swr = swr_alloc_set_opts(
//                nullptr,
//                AV_CH_LAYOUT_MONO,
//                AV_SAMPLE_FMT_S16,
//                frame->sample_rate,
//                av_get_default_channel_layout(frame->channels),
//                (AVSampleFormat)frame->format,
//                frame->sample_rate,
//                0, nullptr);
//            swr_init(swr);

//            uint8_t* out_buf = nullptr;
//            int out_linesize = 0;
//            int out_samples = swr_get_out_samples(swr, frame->nb_samples);
//            av_samples_alloc(&out_buf, &out_linesize, 1, out_samples, AV_SAMPLE_FMT_S16, 0);

//            int conv_samples = swr_convert(
//                swr, &out_buf, out_samples,
//                (const uint8_t**)frame->extended_data, frame->nb_samples);

//            int oldSize = fullSamples.size();
//            fullSamples.resize(oldSize + conv_samples);
//            memcpy(fullSamples.data() + oldSize, out_buf, conv_samples * sizeof(int16_t));

//            sampleRate = frame->sample_rate;

//            av_freep(&out_buf);
//            swr_free(&swr);
//        }
//    }

//    av_frame_free(&frame);
//    av_packet_free(&packet);
//    avcodec_free_context(&ctx);

//    int total_samples = fullSamples.size();
//    if (total_samples > 0) {
//        double sumSquares = 0.0;
//        for (int i = 0; i < total_samples; ++i)
//            sumSquares += fullSamples[i] * fullSamples[i];

//        double rms = std::sqrt(sumSquares / total_samples);

//        // RMS 下限保底，防止靜音導致數值不穩
//        const double rmsFloor = 10.0;
//        if (rms < rmsFloor)
//            rms = rmsFloor;

//        float db = 20.0f * std::log10(rms / 32767.0f);
//        db = std::clamp(db, -100.0f, -0.0f);

//        // 非線性平方增強 normalized，使大小聲區分更明顯
//        float normalized = std::pow((db + 100.0f) / 70.0f, 3.5f) * 100.0f;
//        normalized = std::clamp(normalized, 0.0f, 100.0f);

////        qDebug() << "Total samples:" << total_samples
////                 << "RMS:" << rms
////                 << "→ dB:" << db
////                 << "| Normalized:" << normalized;

//        result.db = db;
//        result.normalized = normalized;
//        result.samples = std::vector<int16_t>(fullSamples.begin(), fullSamples.end());  // ✅ OK
//        result.sampleRate = sampleRate;
//    } else {
//        result.db = -100.0;
//        result.normalized = 0.0;
//    }

//    return result;
//}


float AudioAnalyzer::analyzeFrequency(const std::vector<int16_t>& samples, int sampleRate)
{
    const int N = 1024;
    std::vector<int16_t> padded(N, 0);
    if (samples.size() >= N) {
        std::copy(samples.begin(), samples.begin() + N, padded.begin());
    } else {
        std::copy(samples.begin(), samples.end(), padded.begin());
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, nullptr, nullptr);
    if (!cfg) {
        qWarning() << "kiss_fft_alloc 失敗";
        return 0.0f;
    }

    std::vector<kiss_fft_cpx> in(N), out(N);

    // 加 Hanning 窗
    for (int i = 0; i < N; ++i) {
        float window = 0.5f * (1 - cosf(2.0f * M_PI * i / (N - 1)));
        in[i].r = static_cast<float>(padded[i]) * window;
        in[i].i = 0.0f;
    }

    kiss_fft(cfg, in.data(), out.data());

    float maxMag = 0.0f;
    int maxIndex = 1; // 從1開始跳過直流成分
    for (int i = 1; i < N / 2; ++i) {
        float real = out[i].r;
        float imag = out[i].i;
        float mag = std::sqrt(real * real + imag * imag);
        if (mag > maxMag) {
            maxMag = mag;
            maxIndex = i;
        }
    }

    const float magnitudeThreshold = 1e4f; // 門檻值，可視狀況調整
    float dominantFreq = 0.0f;
    if (maxMag >= magnitudeThreshold) {
        dominantFreq = static_cast<float>(maxIndex) * sampleRate / N;
    }

//    qDebug() << "🎯 主頻率：" << dominantFreq << "Hz (mag:" << maxMag << ")";

    kiss_fft_free(cfg);
    return dominantFreq;
}
