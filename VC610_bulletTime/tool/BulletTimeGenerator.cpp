#include "BulletTimeGenerator.h"
#include <QFileInfo>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QtConcurrent>
#include <QThreadPool>

BulletTimeGenerator::BulletTimeGenerator(QObject* parent) : QObject(parent) {}

bool BulletTimeGenerator::generate(const Params& p)
{
    m_lastError.clear();

    // === 資料夾與主影片檢查 ===
    QDir dir(p.folderPath);
    if (!dir.exists()) {
        m_lastError = QString("資料夾不存在：%1").arg(p.folderPath);
        return false;
    }

    const QString mainPath = dir.filePath(p.mainFileName);
    if (!QFileInfo::exists(mainPath)) {
        m_lastError = QString("主影片不存在：%1").arg(mainPath);
        return false;
    }

    // === 取得 FPS 與總幀數 ===
    double mainFps = 0.0;
    qint64 mainTotalFrames = 0;
    if (!probeFpsAndFrames(mainPath, mainFps, mainTotalFrames) || mainFps <= 0.0) {
        mainFps = 30.0; // fallback
        qWarning() << "⚠️ 使用預設 FPS = 30.0";
    }
    if (mainTotalFrames <= 0) mainTotalFrames = static_cast<qint64>(mainFps * 2.0); // 保底 2 秒

    if (p.verbose)
        qDebug() << "[Probe] mainFps =" << mainFps << ", totalFrames =" << mainTotalFrames;

    // === 關鍵禎時間 ===
    const double keyTs = static_cast<double>(p.keyFrameIndex) / mainFps;

    // === 建立暫存資料夾 ===
    QTemporaryDir tmp(dir.filePath(".bt_tmp_XXXXXX"));
    if (!tmp.isValid()) {
        m_lastError = "無法建立暫存資料夾。";
        return false;
    }
    const QString tmpDir = tmp.path();

    QStringList parts;

    // === 1️⃣ 前段主影片（保留主影片前半段） ===
    {
        const qint64 startF = 0;
        const qint64 endF = qMin(mainTotalFrames, static_cast<qint64>(p.keyFrameIndex));
        const QString preOut = QDir(tmpDir).filePath("part_000_pre.mp4");
        if (!cutMainVideoByFrames(mainPath, mainFps, startF, endF, preOut, p)) {
            m_lastError = "前段主影片擷取失敗。";
            return false;
        }
        parts << preOut;
    }

    // === 2️⃣ 子彈時間段 ===
    const QStringList cameras = listVideoFiles(p.folderPath, p.mainFileName);
    if (cameras.isEmpty()) {
        m_lastError = "未找到其他機位影片（資料夾僅有主影片？）";
        return false;
    }

    // 預設執行緒數
    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());
    QList<QFuture<QString>> futures;
    int idx = 0;

    for (const QString& cam : cameras) {
        const QString png = QDir(tmpDir).filePath(
            QString("grab_%1_%2.png").arg(idx, 3, 10, QChar('0')).arg(QUuid::createUuid().toString(QUuid::Id128)));
        const QString seg = QDir(tmpDir).filePath(
            QString("part_%1_bt.mp4").arg(idx + 100, 3, 10, QChar('0')));

        futures << QtConcurrent::run([=, &p]() -> QString {
            BulletTimeGenerator sub;
            if (!sub.extractFrameAt(cam, keyTs, png, p)) return QString();
            if (!sub.stillImageToVideo(png, mainFps, p.keyHoldFrames, seg, p)) return QString();
            return seg;
        });
        ++idx;
    }

    for (auto& f : futures) {
        f.waitForFinished();
        if (f.result().isEmpty()) {
            m_lastError = "子彈時間靜態片段生成失敗。";
            return false;
        }
        parts << f.result();
    }

    // === 3️⃣ 後段主影片 ===
    {
        const qint64 startF = qBound<qint64>(0, p.keyFrameIndex, mainTotalFrames);
        const qint64 endF = mainTotalFrames;  // 從關鍵禎一路到結尾
        const QString postOut = QDir(tmpDir).filePath("part_999_post.mp4");
        if (!cutMainVideoByFrames(mainPath, mainFps, startF, endF, postOut, p)) {
            m_lastError = "後段主影片擷取失敗。";
            return false;
        }
        parts << postOut;
    }

    // === 4️⃣ 串接所有片段 ===

    // ✅ 在原資料夾底下建立 output 子資料夾
    QString outputFolder = dir.filePath("output");
    if (!QDir(outputFolder).exists()) {
        QDir().mkpath(outputFolder);
    }

    // ✅ 輸出路徑改為 output 資料夾中
    const QString outPath = QDir(outputFolder).filePath(p.outputName);

    if (!concatVideos(parts, outPath, p)) {
        m_lastError = "影片串接失敗。";
        return false;
    }

    qDebug() << "✅ BulletTime 完成：" << outPath;
    return true;
}


// === 探測 ===
bool BulletTimeGenerator::probeFpsAndFrames(const QString& filePath, double& fpsOut, qint64& totalFramesOut)
{
    fpsOut = probeFps(filePath);
    totalFramesOut = probeTotalFrames(filePath);
    return fpsOut > 0.0;
}

double BulletTimeGenerator::probeFps(const QString& filePath)
{
    QString out, err;
    QStringList args{
        "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "stream=avg_frame_rate,r_frame_rate",
        "-of", "default=noprint_wrappers=1:nokey=1",
        filePath
    };
    runProcess("ffprobe", args, &out, &err, false);

    QStringList lines = out.split('\n', Qt::SkipEmptyParts);
    QString fpsStr;
    for (const QString& line : lines) {
        if (line.contains('/')) fpsStr = line.trimmed();
        else if (line.trimmed().contains(QRegularExpression("^[0-9.]+$")))
            fpsStr = line.trimmed();
    }

    if (fpsStr.isEmpty()) return 0.0;

    if (fpsStr.contains('/')) {
        auto parts = fpsStr.split('/');
        double a = parts.value(0).toDouble();
        double b = parts.value(1).toDouble();
        return (b != 0.0) ? (a / b) : 0.0;
    }
    return fpsStr.toDouble();
}

qint64 BulletTimeGenerator::probeTotalFrames(const QString& filePath)
{
    QString out, err;
    QStringList args{
        "-v", "error",
        "-select_streams", "v:0",
        "-count_frames",
        "-show_entries", "stream=nb_read_frames",
        "-of", "default=noprint_wrappers=1:nokey=1",
        filePath
    };

    runProcess("ffprobe", args, &out, &err, false);

    // 可能 ffprobe 輸出多行或混雜警告，逐行取第一個純數字
    QStringList lines = out.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    qint64 total = -1;
    for (QString line : lines) {
        line.remove(QRegularExpression("[^0-9]"));  // 僅保留數字
        if (!line.isEmpty()) {
            bool ok = false;
            qint64 val = line.toLongLong(&ok);
            if (ok && val > 0) {
                total = val;
                break;
            }
        }
    }

    // 若仍取不到 → fallback duration × fps
    if (total <= 0) {
        QString out2, err2;
        QStringList args2{
            "-v", "error",
            "-show_entries", "format=duration",
            "-of", "default=noprint_wrappers=1:nokey=1",
            filePath
        };
        runProcess("ffprobe", args2, &out2, &err2, false);
        bool ok2 = false;
        double duration = out2.trimmed().toDouble(&ok2);
        if (ok2 && duration > 0.0) {
            double fps = probeFps(filePath);
            total = static_cast<qint64>(duration * fps);
        }
    }

    if (total > 1000000) {  // 🚨 防呆：異常大值直接重算
        qWarning() << "[Warn] totalFrames too large, reset fallback";
        double fps = probeFps(filePath);
        QString out2, err2;
        QStringList args2{
            "-v", "error",
            "-show_entries", "format=duration",
            "-of", "default=noprint_wrappers=1:nokey=1",
            filePath
        };
        runProcess("ffprobe", args2, &out2, &err2, false);
        bool ok2 = false;
        double duration = out2.trimmed().toDouble(&ok2);
        if (ok2 && duration > 0.0) {
            total = static_cast<qint64>(duration * fps);
        } else {
            total = -1;
        }
    }

    qDebug() << "[TotalFrames parsed]" << total;
    return total;
}




// === 擷取影格 ===
bool BulletTimeGenerator::extractFrameAt(const QString& filePath, double tsSec, const QString& outPngPath, const Params& p)
{
    QStringList args{
        "-hwaccel", "cuda",   // ✅ GPU 解碼加速
        "-y",
        "-ss", QString::number(tsSec, 'f', 6),
        "-i", filePath,
        "-frames:v", "1",
        "-q:v", "2",
        outPngPath
    };
    QString so, se;
    return runProcess(p.ffmpegPath, args, &so, &se, p.verbose);
}

// === PNG → 靜態影片 ===
bool BulletTimeGenerator::stillImageToVideo(const QString& pngPath, double fps, int durationFrames,
                                            const QString& outVideoPath, const Params& p)
{
    const double durSec = durationFrames / fps;
    QStringList args{
        "-y",
        "-loop", "1",
        "-i", pngPath,
        "-t", QString::number(durSec, 'f', 3),
        "-r", QString::number(fps, 'f', 3),
        "-vf", "format=yuv420p",
        "-c:v", "hevc_nvenc",          // 預設 GPU
        "-preset", "medium",
        "-cq", QString::number(p.crf),
        "-b:v", "0",
        "-threads", QString::number(QThread::idealThreadCount()),
        outVideoPath
    };

    QString so, se;
    bool ok = runProcess(p.ffmpegPath, args, &so, &se, true);

    if (!ok || !QFile::exists(outVideoPath) || QFileInfo(outVideoPath).size() == 0) {
        qWarning() << "⚠️ GPU hevc_nvenc 失敗，嘗試改用 CPU libx264。";
        QStringList args2{
            "-y",
            "-loop", "1",
            "-i", pngPath,
            "-t", QString::number(durSec, 'f', 3),
            "-r", QString::number(fps, 'f', 3),
            "-vf", "format=yuv420p",
            "-c:v", "libx264",
            "-preset", "medium",
            "-crf", QString::number(p.crf),
            "-threads", QString::number(QThread::idealThreadCount()),
            outVideoPath
        };
        ok = runProcess(p.ffmpegPath, args2, &so, &se, true);
    }

    if (!ok) {
        qWarning().noquote() << "❌ stillImageToVideo ffmpeg failed\n" << se;
    }

    return ok && QFile::exists(outVideoPath) && QFileInfo(outVideoPath).size() > 0;
}


// === 主影片分段 ===
bool BulletTimeGenerator::cutMainVideoByFrames(const QString& filePath, double fps,
                                               qint64 startFrame, qint64 endFrame,
                                               const QString& outVideoPath, const Params& p)
{
    if (endFrame <= startFrame) return false;

    const double ss = static_cast<double>(startFrame) / fps;
    const double tt = static_cast<double>(endFrame - startFrame) / fps;

    QString so, se;
    QStringList args{
        "-y",
        "-hwaccel", "auto",                  // ✅ 自動選擇 CUDA / DXVA / QSV
        "-ss", QString::number(ss, 'f', 3),
        "-i", filePath,
        "-t", QString::number(tt, 'f', 3),
        "-r", QString::number(fps, 'f', 3),
        "-vf", "format=yuv420p",
        "-c:v", "hevc_nvenc",                // 🟢 GPU (有 NVENC 時最快)
        "-preset", "medium",
        "-cq", QString::number(p.crf),
        "-b:v", "0",
        "-threads", QString::number(QThread::idealThreadCount()),
        outVideoPath
    };

    // 🔹 嘗試 GPU
    bool ok = runProcess(p.ffmpegPath, args, &so, &se, true);

    // 🔹 若 GPU 不支援，改用 CPU
    if (!ok) {
        qWarning() << "⚠️ GPU 編碼失敗，改用 CPU 軟體編碼 libx264。";
        args.replace(args.indexOf("-c:v") + 1, "libx264");
        args.removeAll("-hwaccel");
        args.removeAll("auto");
        ok = runProcess(p.ffmpegPath, args, &so, &se, true);
    }

    // 🔹 若仍失敗，嘗試不用 -r
    if (!ok) {
        qWarning() << "⚠️ CPU 編碼仍失敗，嘗試移除 -r。";
        args.removeAll("-r");
        ok = runProcess(p.ffmpegPath, args, &so, &se, true);
    }

    if (!ok) {
        qWarning().noquote() << "❌ ffmpeg failed\n" << so << se;
    }

    return ok;
}


// === 串接 ===
bool BulletTimeGenerator::concatVideos(const QStringList& partPaths, const QString& outPath, const Params& p)
{
    if (partPaths.isEmpty()) return false;

    QTemporaryDir tmp(QFileInfo(outPath).dir().filePath(".bt_concat_XXXXXX"));
    tmp.setAutoRemove(false);
    const QString listFile = QDir(tmp.path()).filePath("list.txt");

    QFile f(listFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "無法建立 concat list 檔案。";
        return false;
    }

    QTextStream ts(&f);
    for (const QString& pth : partPaths) {
        QString escaped = QDir::toNativeSeparators(pth);
        escaped.replace("'", "\\'");
        ts << "file '" << escaped << "'\n";
    }
    f.close();

    QString so, se;

    // === 第一階段：嘗試直接 copy 串接（最快）
    QStringList argsCopy{
        "-y",
        "-f", "concat",
        "-safe", "0",
        "-i", listFile,
        "-c", "copy",
        "-fflags", "+genpts",
        outPath
    };

    bool ok = runProcess(p.ffmpegPath, argsCopy, &so, &se, true);
    if (ok && QFile::exists(outPath) && QFileInfo(outPath).size() > 0)
        return true;

    qWarning() << "⚠️ concat copy failed, fallback to re-encode 60fps.";

    // === 第二階段：強制重新編碼 + 固定60fps ===
    QStringList argsReencode{
        "-y",
        "-f", "concat",
        "-safe", "0",
        "-i", listFile,
        "-fflags", "+genpts",
        "-filter:v",
        "fps=60,setpts=N/(60*TB),minterpolate=fps=60:mi_mode=mci:mc_mode=aobmc:me_mode=bidir,format=yuv420p",
        "-vsync", "cfr",
        "-c:v", "hevc_nvenc",
        "-preset", "p4",
        "-rc", "constqp",
        "-qp", QString::number(p.crf),
        "-pix_fmt", "yuv420p",
        "-movflags", "+faststart",
        outPath
    };

    ok = runProcess(p.ffmpegPath, argsReencode, &so, &se, true);
    if (!ok) {
        qWarning().noquote() << "❌ concatVideos failed\n" << se;
    }

    return ok && QFile::exists(outPath) && QFileInfo(outPath).size() > 0;
}


// === 工具 ===
QStringList BulletTimeGenerator::listVideoFiles(const QString& folder, const QString& mainFile)
{
    static const QStringList exts = {"*.mp4","*.mov","*.m4v","*.mkv","*.hevc","*.h265","*.ts"};
    QDir d(folder);
    QStringList files = d.entryList(exts, QDir::Files, QDir::Name | QDir::IgnoreCase);
    files.removeAll(mainFile);
    QStringList full;
    for (const auto& f : files) full << d.filePath(f);
    return full;
}

bool BulletTimeGenerator::runProcess(const QString& program, const QStringList& args,
                                     QString* stdOut, QString* stdErr, bool verbose)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(program, args);
    if (!p.waitForStarted()) {
        m_lastError = QString("無法啟動程式：%1").arg(program);
        return false;
    }
    if (!p.waitForFinished(-1)) {
        m_lastError = QString("程式逾時或中止：%1").arg(program);
        return false;
    }

    const QByteArray out = p.readAllStandardOutput();
    const QByteArray err = p.readAllStandardError();
    if (stdOut) *stdOut = QString::fromLocal8Bit(out);
    if (stdErr) *stdErr = QString::fromLocal8Bit(err);

    if (verbose) {
        qDebug() << ">>" << program << args;
        if (!out.isEmpty()) qDebug().noquote() << QString::fromLocal8Bit(out);
        if (!err.isEmpty()) qDebug().noquote() << QString::fromLocal8Bit(err);
    }
    return p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0;
}
