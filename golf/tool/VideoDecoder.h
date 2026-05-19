#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <QImage>
#include <QString>
#include "FrameData.h"

class VideoDecoder {
public:
    VideoDecoder();
    QImage getNextFrame();
    void stop();   // 手動停止解碼
    void release();
    FrameData getFrameData();
    bool openFile(const QString& filename);
    QImage getFrameAt(int64_t frameIndex);  // 指定幀號（0-based）取得影像
    int getTotalFrames() const;

    ~VideoDecoder();

private:

    double getFrameRate() const;

    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVStream* videoStream = nullptr;
    int videoStreamIndex = -1;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    SwsContext* swsCtx = nullptr;
    AVBufferRef* hw_device_ctx = nullptr;

    int width = 0;
    int height = 0;

    AVFrame* rgbFrame = nullptr;
    uint8_t* buffer = nullptr;
    int numBytes = 0;

    bool debug = false;
    long long decodedFrameIndex = 0; // 用來印第幾禎

    FrameData FrameData; 
    std::atomic<bool> stopRequested {false};  // 🔑 停止旗標

    int64_t frameIndexFromPts(int64_t pts) const;     // pts -> 幀號估算
    int64_t ptsFromFrameIndex(int64_t idx) const;     // 幀號 -> 目標 pts（stream time_base）
};

#endif // VIDEODECODER_H
