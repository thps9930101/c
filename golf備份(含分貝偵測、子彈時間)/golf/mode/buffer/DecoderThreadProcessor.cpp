#include "DecoderThreadProcessor.h"
#include <QDebug>

DecoderThreadProcessor::DecoderThreadProcessor(const QString& ip) : ip(ip) {}

DecoderThreadProcessor::~DecoderThreadProcessor() {
    stop();
    wait();
}

void DecoderThreadProcessor::enqueue(const QList<FrameItem>& frames) {
    QMutexLocker locker(&mutex);
    frameQueue.enqueue(frames);
    condition.wakeOne();
}

void DecoderThreadProcessor::stop() {
    QMutexLocker locker(&mutex);
    running = false;
    condition.wakeAll();
}

QList<DecodedFrame> DecoderThreadProcessor::takeDecodedFrames() {
    QMutexLocker locker(&mutex);
    QList<DecodedFrame> result;
    while (!decodedQueue.isEmpty()) {
        result.append(decodedQueue.dequeue());
    }
    return result;
}

void DecoderThreadProcessor::run() {
    while (true) {
        QList<FrameItem> frames;

        {
            QMutexLocker locker(&mutex);
            if (!running && frameQueue.isEmpty())
                break;

            if (frameQueue.isEmpty()) {
                condition.wait(&mutex);
                continue;
            }

            frames = frameQueue.dequeue();
        }

        decodeFrames(frames);
    }
}

void DecoderThreadProcessor::decodeFrames( const QList<FrameItem>& frames) {

}
