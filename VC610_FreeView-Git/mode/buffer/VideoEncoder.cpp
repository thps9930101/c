#include "VideoEncoder.h"
#include <QDebug>

//VideoEncoder::VideoEncoder() {}

//VideoEncoder::~VideoEncoder() {
//    finish();
//}

bool VideoEncoder::initialize(const QString& filename, int width, int height, int fps) {
    // 🔥 最前面補上保險清理
    if (initialized) {
        qWarning() << "⚠️ initialize() called while encoder was already initialized. Cleaning up.";
        finish();  // 強制清除上一次殘留
    }
    qDebug()<<"Encoder init";
    outputFilename = filename;
    frameIndex = 0;

    if (avformat_alloc_output_context2(&fmtCtx, nullptr, nullptr, filename.toUtf8().constData()) < 0 || !fmtCtx) {
        qWarning() << "❌ Failed to create output context";
        return false;
    }

    AVCodec* codec = avcodec_find_encoder_by_name("hevc_nvenc");
    if (!codec) {
        qWarning() << "❌ H.265 NVENC codec (hevc_nvenc) not found";
        return false;
    }

    stream = avformat_new_stream(fmtCtx, nullptr);
    if (!stream) {
        qWarning() << "❌ Failed to create new stream";
        return false;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        qWarning() << "❌ Failed to allocate codec context";
        return false;
    }

    int err = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_CUDA, nullptr, nullptr, 0);
    if (err < 0) {
        qWarning() << "❌ Failed to create CUDA device context";
        return false;
    }
    codecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    codecCtx->codec_id = codec->id;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    codecCtx->width = width;
    codecCtx->height = height;
    codecCtx->pix_fmt = AV_PIX_FMT_CUDA;
    codecCtx->bit_rate = 50000000;
    codecCtx->gop_size = 12;
    codecCtx->max_b_frames = 0;

    AVRational framerate = {fps, 1};
    codecCtx->framerate = framerate;
    codecCtx->time_base = av_inv_q(framerate);
    stream->time_base = codecCtx->time_base;

    if (fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


    hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx);

    int refCount = av_buffer_get_ref_count(hw_frames_ctx);
    qDebug() << "🔍 hw_frames_ctx 引用計數2:" << refCount;

    if (!hw_frames_ctx) {
        qWarning() << "❌ Failed to allocate hw_frames_ctx";
        return false;
    }
    frames_ctx = (AVHWFramesContext*)(hw_frames_ctx->data);
    frames_ctx->format = AV_PIX_FMT_CUDA;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->width = width;
    frames_ctx->height = height;
    frames_ctx->initial_pool_size = 64;

    err = av_hwframe_ctx_init(hw_frames_ctx);
    if (err < 0) {
        qWarning() << "❌ Failed to initialize hw_frames_ctx";
        return false;
    }
    codecCtx->hw_frames_ctx = av_buffer_ref(hw_frames_ctx);

    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "llhp", 0);
    av_dict_set(&opts, "rc", "cbr", 0);
    av_dict_set(&opts, "cq", "19", 0);
    av_dict_set(&opts, "zerolatency", "1", 0);

    err = avcodec_open2(codecCtx, codec, &opts);
    av_dict_free(&opts);
    if (err < 0) {
        char errbuf[256];
        av_strerror(err, errbuf, sizeof(errbuf));
        qWarning() << "❌ Could not open codec:" << errbuf;
        return false;
    }

    if (avcodec_parameters_from_context(stream->codecpar, codecCtx) < 0) {
        qWarning() << "❌ Failed to copy codec parameters";
        return false;
    }

    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&fmtCtx->pb, filename.toUtf8().constData(), AVIO_FLAG_WRITE) < 0) {
            qWarning() << "❌ Failed to open output file";
            return false;
        }
    }

    if (avformat_write_header(fmtCtx, nullptr) < 0) {
        qWarning() << "❌ Failed to write header";
        return false;
    }

    running = true;
    encodeThread = std::thread(&VideoEncoder::encodeLoop, this);
    initialized = true;
    timer.start();
    return true;
}

void VideoEncoder::submitFrame(AVFrame* frame) {
    if (!frame) return;

    AVFrame* refFrame = av_frame_alloc();
    if (!refFrame) return;


    if (av_frame_ref(refFrame, frame) < 0) {
        av_buffer_unref(&refFrame->hw_frames_ctx);
        av_frame_free(&refFrame);
        return;
    }

    std::unique_lock<std::mutex> lock(queueMutex);
    frameQueue.push(refFrame);
    queueCond.notify_one();
}

bool VideoEncoder::encodeFrame(AVFrame* frame) {
    if (!initialized) return false;

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) return false;

    if (frame) {
        frame->pts = av_rescale_q(frameIndex, AVRational{1, 60}, codecCtx->time_base);
    }

    // ❗這裡允許 frame 為 nullptr，表示 flush
    int ret = avcodec_send_frame(codecCtx, frame);
    if (ret < 0) {
        qWarning() << "Error sending frame";
        av_packet_free(&pkt);
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0) {
            qWarning() << "Error encoding frame";
            break;
        }

        pkt->stream_index = stream->index;
        pkt->pts = pkt->dts = av_rescale_q(frameIndex, AVRational{1, 60}, stream->time_base);
        pkt->duration = av_rescale_q(1, AVRational{1, 60}, stream->time_base);

        av_interleaved_write_frame(fmtCtx, pkt);
        av_packet_unref(pkt);
        frameIndex++;
    }

    av_packet_free(&pkt);
    return true;
}

void VideoEncoder::encodeLoop() {
    while (running || !frameQueue.empty()) {
        AVFrame* frame = nullptr;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [&]() { return !frameQueue.empty() || !running; });

            if (!frameQueue.empty()) {
                frame = frameQueue.front();
                frameQueue.pop();
            } else if (!running) {
                break;
            }
        }

        if (frame) {
            encodeFrame(frame);

            // ✅ 不再手動 unref hw_frames_ctx，避免引用計數不平衡
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }
}

//void VideoEncoder::finish() {
//    if (!initialized) return;
//    qDebug() << "🔚 Cleaning VideoEncoder resources...";

//    running = false;
//    queueCond.notify_all();
//    if (encodeThread.joinable()) encodeThread.join();

//    encodeFrame(nullptr);  // flush
//    avcodec_flush_buffers(codecCtx);

//    std::lock_guard<std::mutex> lock(queueMutex);
//    while (!frameQueue.empty()) {
//        AVFrame* f = frameQueue.front(); frameQueue.pop();
//        if (f) {
//            av_frame_unref(f);
//            av_frame_free(&f);
//        }
//    }

//    av_write_trailer(fmtCtx);

//    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE))
//        avio_closep(&fmtCtx->pb);

//    // ✅ 正確順序釋放
//    if (codecCtx && codecCtx->hw_frames_ctx) {
//        av_buffer_unref(&codecCtx->hw_frames_ctx);
//        codecCtx->hw_frames_ctx = nullptr;
//    }

//    avcodec_free_context(&codecCtx);
//    codecCtx = nullptr;

//    avformat_free_context(fmtCtx);
//    fmtCtx = nullptr;

//    av_buffer_unref(&hw_frames_ctx);
//    hw_frames_ctx = nullptr;

//    av_buffer_unref(&hw_device_ctx);
//    hw_device_ctx = nullptr;

//    initialized = false;
//    qDebug() << "✅ VideoEncoder::finish() 完成";
//}

void VideoEncoder::finish() {
    if (!initialized)
        return;

    qDebug() << "🔚 開始釋放 VideoEncoder 資源";


    // 1. 停止執行緒
    running = false;
    queueCond.notify_all();
    if (encodeThread.joinable()) {
        encodeThread.join();
        qDebug() << "🧵 encodeThread 已成功結束";
    }

    // 2. Flush 編碼器
    encodeFrame(nullptr);
    avcodec_flush_buffers(codecCtx);

    // 3. 清空 Frame Queue
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        qDebug() << "🧹 開始清空 frameQueue，剩餘數量:" << frameQueue.size();
        while (!frameQueue.empty()) {
            AVFrame* f = frameQueue.front();
            frameQueue.pop();
            if (f) {
                if (f->hw_frames_ctx) {
                    qDebug() << "⚠️ 殘留 frame->hw_frames_ctx 正在釋放";
                    av_buffer_unref(&f->hw_frames_ctx);
                }
                av_frame_unref(f);
                av_frame_free(&f);
            }
        }
        qDebug() << "✅ frameQueue 已清空";
    }

    // 4. 寫入結尾資訊
    if (av_write_trailer(fmtCtx) < 0)
        qWarning() << "⚠️ av_write_trailer 寫入失敗";

    // 5. 關閉文件
    if (!(fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&fmtCtx->pb);
        qDebug() << "📄 輸出檔案已關閉";
    }

    // 6. 釋放 codecCtx 及其附帶資源
    if (codecCtx) {
        releaseHWFrameCtx();  // 💥 移至這裡，安全統一處理
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
        qDebug() << "🎯 codecCtx 已釋放";
    }

    // 7. 釋放 fmtCtx
    if (fmtCtx) {
        avformat_free_context(fmtCtx);
        fmtCtx = nullptr;
        qDebug() << "🎯 fmtCtx 已釋放";
    }
//    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    // 8. 釋放 GPU Device
    if (hw_device_ctx) {
        int refCount = av_buffer_get_ref_count(hw_device_ctx);
        qDebug() << "🔍 hw_device_ctx 引用計數:" << refCount;
        av_buffer_unref(&hw_device_ctx);
        hw_device_ctx = nullptr;
        qDebug() << "🎯 hw_device_ctx 已釋放";
    }

    if (!moveFile(outputFilename, Filename)) {
        qWarning() << "檔案搬移失敗";
    }

    // 9. 結束
    initialized = false;
    qDebug() << "⏱️ 總編碼時間:" << timer.elapsed() << "ms";
    qDebug() << "✅ VideoEncoder::finish() 完成所有資源釋放";
}


bool VideoEncoder::isInitialized() const {
    return initialized;
}

void VideoEncoder::releaseHWFrameCtx() {
    if (codecCtx && codecCtx->hw_frames_ctx) {
        int refCount = av_buffer_get_ref_count(codecCtx->hw_frames_ctx);
        qDebug() << "🔍 codecCtx->hw_frames_ctx 引用計數:" << refCount;
        av_buffer_unref(&codecCtx->hw_frames_ctx);
        codecCtx->hw_frames_ctx = nullptr;
        qDebug() << "🎯 codecCtx->hw_frames_ctx 已釋放";
    }

    if (hw_frames_ctx) {
        int refCount = av_buffer_get_ref_count(hw_frames_ctx);
        qDebug() << "🔍 hw_frames_ctx 引用計數:" << refCount;
        av_buffer_unref(&hw_frames_ctx);
        hw_frames_ctx = nullptr;
        qDebug() << "🎯 hw_frames_ctx 已釋放";
    }
}

bool VideoEncoder::moveFile(const QString& sourcePath, const QString& destinationPath) {
    QFileInfo srcInfo(sourcePath);
    if (!srcInfo.exists() || !srcInfo.isFile()) {
        qWarning() << "❌ Source file does not exist:" << sourcePath;
        return false;
    }

    QDir destDir = QFileInfo(destinationPath).absoluteDir();
    if (!destDir.exists()) {
        qDebug() << "📁 Destination directory does not exist, creating:" << destDir.path();
        if (!destDir.mkpath(".")) {
            qWarning() << "❌ Failed to create destination directory:" << destDir.path();
            return false;
        }
    }

    // 若目的地已有相同檔名，先刪除
    if (QFile::exists(destinationPath)) {
        qDebug() << "⚠️ Destination file exists, removing:" << destinationPath;
        if (!QFile::remove(destinationPath)) {
            qWarning() << "❌ Failed to remove existing destination file:" << destinationPath;
            return false;
        }
    }

    if (QFile::rename(sourcePath, destinationPath)) {
        qDebug() << "✅ File moved successfully from" << sourcePath << "to" << destinationPath;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        renameFile(Filename,"test","",false);
        return true;
    } else {
        qWarning() << "❌ Failed to move file from" << sourcePath << "to" << destinationPath;
        return false;
    }
}

QString VideoEncoder::renameFile(const QString& originalPath,
                   const QString& suffix = "",
                   const QString& newExtension = "",
                   bool addTimestamp = false)
{
    QFileInfo fileInfo(originalPath);
    QString dir = fileInfo.absolutePath();
    QString baseName = fileInfo.completeBaseName(); // 不含副檔名
    QString extension = newExtension.isEmpty() ? fileInfo.suffix() : newExtension;

    if (addTimestamp) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        baseName += "_" + timestamp;
    }

    if (!suffix.isEmpty()) {
        baseName += "_" + suffix;
    }

    QString newFileName = QString("%1.%2").arg(baseName, extension);
    QString newFullPath = QDir(dir).filePath(newFileName);

    // 實際改檔名
    if (QFile::exists(originalPath)) {
        bool success = QFile::rename(originalPath, newFullPath);
        if (success) {
            qDebug() << "✅ 檔案名稱已更改:" << originalPath << "➡️" << newFullPath;
        } else {
            qWarning() << "❌ 檔案改名失敗:" << originalPath;
        }
    } else {
        qWarning() << "⚠️ 原始檔不存在:" << originalPath;
    }

    return newFullPath;
}
