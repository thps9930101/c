#pragma once

#include <QObject>
#include <QThread>
#include <QByteArray>
#include <functional>
#include <algorithm>
#include "ao_client.h"

#include "mode/buffer/FFT/kiss_fft.h" // ✅ 輕量 FFT 函式庫
#include <cmath>

// 假設你已經定義在某個命名空間
struct lm_audio_data {
    uint32_t len;
    uint64_t ntp_timestamp;
    uint32_t frm_no;
    uint8_t* data;
};

struct AudioAnalysisResult {
    float db = -100.0f;
    std::vector<int16_t> samples;  // 單聲道 PCM
    int sampleRate = 0;
    float normalized;   // 映射後 0~100 的值
};

class AudioAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit AudioAnalyzer(QObject* parent = nullptr);
    ~AudioAnalyzer();

    AudioAnalysisResult analyze(const ao::lm_audio_data& input);  // 直接回傳 dB
    QVector<float> getWaveform() const { return waveform; }
    QVector<float> getDbLevels() const { return dbLevels; }
    QVector<float> computeFFT(const QVector<float>&, int, int&);
    float AudioAnalyzer::analyzeFrequency(const std::vector<int16_t>&, int);

private:
    AudioAnalysisResult decodeAndAnalyze(const QByteArray& data); // 內部用解碼分析器
    QVector<float> waveform;  // Normalized PCM [-1.0, 1.0]
    QVector<float> dbLevels;  // 每 10ms 分貝
    float sampleVal;
};
