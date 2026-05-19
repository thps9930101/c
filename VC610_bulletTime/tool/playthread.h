// playthread.h
#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QDebug>

class PlayThread : public QThread {
    Q_OBJECT
public:
    PlayThread(int fps, QObject* parent = nullptr)
        : QThread(parent), fps(fps), running(true), paused(false) {}

    void stop() {
        QMutexLocker locker(&mutex);
        running = false;
        paused = false;
        cond.wakeAll();
    }

    void setFps(int newFps) {
        QMutexLocker locker(&mutex);
        fps = newFps;
    }

    void pause() {
        QMutexLocker locker(&mutex);
        paused = true;
    }

    void resume() {
        QMutexLocker locker(&mutex);
        paused = false;
        cond.wakeAll();
    }

signals:
    void nextFrame();

protected:
    void run() override {
        QElapsedTimer timer;
        timer.start();
        qint64 nextFrameTime = 0; // 下一幀應該出現的時間 (ms)

        while (true) {
            {
                QMutexLocker locker(&mutex);
                if (!running) break;

                if (paused) {
                    cond.wait(&mutex);
                    // 重新校正時間（避免暫停後 drift）
                    timer.restart();
                    nextFrameTime = 0;
                    continue;
                }
            }

            // 應該出現的時間
            int interval = 1000 / fps;
            qint64 now = timer.elapsed();
            qDebug()<<interval;
            if (now >= nextFrameTime) {
                emit nextFrame();
                nextFrameTime += interval;
            } else {
                // 睡到下一個幀時間，避免 busy loop
                int sleepTime = int(nextFrameTime - now);
                if (sleepTime > 0)
                    msleep(sleepTime);
            }
        }
    }

private:
    int fps;
    bool running;
    bool paused;
    QMutex mutex;
    QWaitCondition cond;
};
