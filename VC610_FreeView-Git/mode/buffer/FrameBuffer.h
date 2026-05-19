#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <vector>
#include <thread>
#include "AudioAnalyzer.h"
#include "ao_client.h"
#include <QElapsedTimer>  // 加入這行
#include <QImage>
#include <QMap>
#include <QHash>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <QElapsedTimer>

#include <QJsonArray>
#include <QJsonObject>

#include "mode/cam/ao_readcam.h"
#include "mode/buffer/AudioAnalyzer.h"
#include "mode/buffer/VideoEncoder.h"
#include "mode/buffer/jsonfilemanager.h"

#include "mode/decode/decode.h"
#include "mode/my_container/RingBuffer.h"
#include "mode/encoder/output_file.h"

#include "conver.h"
#include "SafeQueue.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"
#include <opencv2/bgsegm.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/stitching.hpp"
#include "opencv2/cudacodec.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgcodecs/legacy/constants_c.h"

#include <opencv2/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudabgsegm.hpp>
#include <opencv2/cudafilters.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

extern "C" {
#include <libswscale/swscale.h>
}

//struct AudioAnalysisResult {
//    float db = -100.0f;
//    std::vector<int16_t> samples;  // 單聲道 PCM
//    int sampleRate = 0;
//};

// 原始影片資料結構（外部傳入）
// 儲存在 FrameBuffer 內部的封裝版本
struct FrameItem {
    QString ip;
    int frameIndex;
    ao::lm_video_data* ptr;

    uint32_t  len;
    uint64_t  ntp_timestamp;
    uint32_t  frm_no;
    uint32_t  type;
    QByteArray data;  // 使用 QByteArray 儲存複製後的資料

    int bufferIndex = -1;  // ✅ 新增：記錄寫入時在 buffer 裡的位置

};

struct FramePair {
    std::vector<FrameItem> activeBuffer;
    std::vector<FrameItem> lockedBuffer;
    int currentIndex = 0;
    bool filledOnce = false;

    FramePair(int maxFrames) {
        activeBuffer.resize(maxFrames);
        lockedBuffer.resize(maxFrames);
    }

    FramePair() = default; // 為了讓 QHash 的 value 可 default construct
};

struct EncoderContext {
    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    AVStream* stream = nullptr;
    int frameIndex = 0;
    SwsContext* swsCtx = nullptr;
};

using milli_type = std::chrono::duration<double,std::milli>;
typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration<long long ,std::ratio<1,1000000000>>> _time;
struct Cam_data2 {
    uint32_t CamNO;

    // 原始壓縮資料緩衝區
    RingBuffer<ao::lm_video_data>* vid_data_buf = nullptr;

    // Decoder 模組
    Decoder_class* dc = nullptr;

    bool decoding = true;
    std::thread decoderThread;

    // 解碼控制旗標與解碼執行緒
    std::thread* dc_thr = nullptr;

    // 停止解碼用的共享旗標
    std::shared_ptr<bool> sy_stop;

    // 解碼統計資訊（可保留）
    uint32_t dc2_count = 0;
    uint32_t dc2_lost_count = 0;

    _time dc2_start_time;
    _time dc2_end_time;
    milli_type dc2_cost_time;

    _time dc2_start_time2;
    _time dc2_end_time2;
    milli_type dc2_cost_time2;

    Cam_data2() = default;

    ~Cam_data2() {
        if (vid_data_buf != nullptr) {
            delete vid_data_buf;
            vid_data_buf = nullptr;
        }
        if (dc != nullptr) {
            delete dc;
            dc = nullptr;
        }
    }
};

// 影像 Frame 環狀緩衝區類別
class FrameBuffer {
public:
    FrameBuffer(int frameRate, int durationSeconds, int maxFrameSizeKB);
    void addFrame(const char*, const ao::lm_video_data& input);
    void processAudioData(const ao::lm_audio_data& input);
    QList<FrameItem> getCapturedFrames() const;
    void printOrderedTypesSimple() const;
    void decodeCapturedFrames(const QString& ip, const QList<FrameItem>& frames);
    void registerDecoder(const QString& ip, std::shared_ptr<Cam_data2> cam);
    void composeAndSaveGridFrameSafe_GPU(const QList<AVFrame*>&, const QString&);
    void checkAndComposeFrames();
    void init(const char*);
    bool shouldSyncBuffers();
    float returnDB();
    int findBackwardIFrameStartIndex(const QVector<FrameItem>&, int, int);

private:
    int maxFrames;
    int maxFrameSize;

    int currentIndex;
    bool filledOnce;
    bool switched;
    QString ip; // ← 每組專屬 IP

    QHash<QString, FramePair> ipBuffers;
    QMap<QString, std::shared_ptr<AudioThreadProcessor>> processors;
    QHash<QString, std::shared_ptr<Cam_data2>> camMap;
    QMap<QString, QList<AVFrame*>> decodedFrameBuffer;
    QList<AVFrame*> pendingCloneFrames;

    mutable QMutex mutex;
    QMutex bufferMutex;

    VideoEncoder encoder;
    int tmpCount = 0;
    bool isDetecting = true;
    bool startcompose = true;
    bool FirstInit = false;

    QString timestamp;
    QString filename;
    QString tmpfilename;

    QElapsedTimer switchCooldown;
    qint64 cooldownIntervalMs = 1000;

    QJsonObject bufferObj;
    Decoder_class_param dc_par;

    QList<FrameItem> capturedFrames;
    bool state = false;
    QList<AVFrame*> framesToCompose;

    struct GridCell {
        QString ip;   // 哪台相機
        bool rotate;  // 是否旋轉 180 度
    };

    int waitSec = 0;
    int frameCount = 0;
    int framesPerIP = 0;
    int videoFrame = 0;
    AudioAnalysisResult db;
};

#endif // FRAMEBUFFER_H
