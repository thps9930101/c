// DecoderThreadProcessor.h
#pragma once

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QList>
#include <QSharedPointer>

#include "ao_client.h"

struct FrameItem {
    QString ip;
    int frameIndex;
    ao::lm_video_data* ptr;

    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
    uint32_t  type;
    QByteArray data;  // 使用 QByteArray 儲存複製後的資料
};

struct DecodedFrame {
    QString ip;
    int frameNumber;
    QByteArray imageData; // or QImage 或 cv::Mat 視解碼結果而定
};

class DecoderThreadProcessor : public QThread {
    Q_OBJECT
public:
    explicit DecoderThreadProcessor(const QString& ip);
    ~DecoderThreadProcessor();

    void enqueue(const QList<FrameItem>& frames);
    QList<DecodedFrame> takeDecodedFrames();
    void stop();

protected:
    void run() override;

private:
    QString ip;
    QQueue<QList<FrameItem>> frameQueue;
    QQueue<DecodedFrame> decodedQueue;
    QMutex mutex;
    QWaitCondition condition;
    bool running = true;

    void decodeFrames(const QList<FrameItem>& frames);
};
