#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <QString>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

class VideoEncoder {
public:
    bool initialize(const QString& filename, int width, int height, int fps);
    void submitFrame(AVFrame* frame);
    void finish();
    bool isInitialized() const;
    AVBufferRef* getHWFramesContext() const { return hw_frames_ctx; }

    AVBufferRef* hw_frames_ctx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    QString Filename;

private:
    void encodeLoop();
    bool encodeFrame(AVFrame* frame);
    void releaseHWFrameCtx();
    bool moveFile(const QString&, const QString&);
    QString renameFile(const QString&,const QString&,const QString&,bool);
    AVFormatContext* fmtCtx = nullptr;
    AVStream* stream = nullptr;
    AVBufferRef* hw_device_ctx = nullptr;

    QString outputFilename;
    int frameIndex = 0;
    bool initialized = false;
    bool running = false;

    std::thread encodeThread;
    std::queue<AVFrame*> frameQueue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    QElapsedTimer timer;
    AVHWFramesContext* frames_ctx;
};

#endif // VIDEOENCODER_H
