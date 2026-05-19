// AudioThreadProcessor.h

#pragma once
#include <QString>
#include <thread>
#include <atomic>

class AudioThreadProcessor {
public:
    AudioThreadProcessor(const QString& ip) : ip(ip), running(false) {}

    void init() {
        running = true;
        worker = std::thread([this]() {
            while (running) {
                // 模擬工作
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }

    void shutdown() {
        running = false;
        if (worker.joinable()) worker.join();
    }

private:
    QString ip;
    std::atomic<bool> running;
    std::thread worker;
};
