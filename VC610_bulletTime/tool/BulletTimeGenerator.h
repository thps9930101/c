#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>

/**
 * BulletTimeGenerator
 * 以主影片之關鍵禎為基準，產生多機位的「子彈時間」HEVC 影片。
 *
 * 主要流程：
 *  1) 以 ffprobe 取得主影片 FPS 與總幀數
 *  2) 計算關鍵禎時間點 (keyFrameIndex / fps)
 *  3) 掃描資料夾中所有影片（排除主影片），逐一在相同時間點截圖
 *  4) 每張截圖產生一段靜止影片（長度為 keyHoldFrames / fps）
 *  5) 可選擇拼上主影片關鍵禎前後各一小段（預設各 30 幀，可調）
 *  6) concat 串接成單一 HEVC 輸出
 */
class BulletTimeGenerator : public QObject
{
    Q_OBJECT
public:
    explicit BulletTimeGenerator(QObject* parent = nullptr);

    struct Params {
        QString folderPath;        // 影片資料夾
        QString mainFileName;      // 主影片檔名（位於 folderPath）
        int     keyFrameIndex;     // 關鍵禎（0-based）
        int     keyHoldFrames;     // 關鍵禎播放幀數（每一機位靜止段的幀數）
        QString ffmpegPath = "ffmpeg";   // 可設定 ffmpeg 絕對路徑
        QString ffprobePath = "ffprobe"; // 可設定 ffprobe 絕對路徑
        QString outputName = "bullet_time_output.mp4"; // 輸出檔名（放在 folderPath 下）
        int     preFrames = 30;    // 關鍵禎前要接幾幀主影片（0 = 不接）
        int     postFrames = 30;   // 關鍵禎後要接幾幀主影片（0 = 不接）
        int     crf = 22;          // x265 CRF (18~28 越小越清晰)
        QString preset = "medium"; // x265 preset
        bool    verbose = true;    // 是否輸出詳細 log
    };

    // 入口：執行整個子彈時間流程
    // 成功回傳 true，失敗回傳 false（可查看 lastError()）
    bool generate(const Params& p);

    QString lastError() const { return m_lastError; }

private:
    // 探測資訊
    bool   probeFpsAndFrames(const QString& filePath, double& fpsOut, qint64& totalFramesOut);
    double probeFps(const QString& filePath);
    qint64 probeTotalFrames(const QString& filePath);

    // 擷取單張影格（PNG）
    // tsSec = 目標秒數（支援小數），outPngPath = 輸出 png
    bool extractFrameAt(const QString& filePath, double tsSec, const QString& outPngPath, const Params& p);

    // 由單張 PNG 生成一段 HEVC 靜態影片
    // durationFrames / fps = 秒
    bool stillImageToVideo(const QString& pngPath, double fps, int durationFrames,
                           const QString& outVideoPath, const Params& p);

    // 取主影片的片段（以幀為單位，會重編碼成 HEVC）
    // [startFrame, endFrame]（含 start，不含 end），自動轉成秒並 -ss/-t
    bool cutMainVideoByFrames(const QString& filePath, double fps,
                              qint64 startFrame, qint64 endFrame,
                              const QString& outVideoPath, const Params& p);

    // 串接多段同編碼影片
    bool concatVideos(const QStringList& partPaths, const QString& outPath, const Params& p);

    // 工具
    QStringList listVideoFiles(const QString& folder, const QString& mainFile);
    bool runProcess(const QString& program, const QStringList& args,
                    QString* stdOut = nullptr, QString* stdErr = nullptr, bool verbose = true);

private:
    QString m_lastError;
};
