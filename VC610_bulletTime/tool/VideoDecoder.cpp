#include "VideoDecoder.h"
#include <QDebug>
#include <chrono>

VideoDecoder::VideoDecoder() {
    av_log_set_level(AV_LOG_ERROR);
    av_register_all();
    avformat_network_init();
}

bool VideoDecoder::openFile(const QString& filename) {
    release();

    if (avformat_open_input(&fmtCtx, filename.toStdString().c_str(), nullptr, nullptr) != 0) {
        qWarning() << "❌ Cannot open file!";
        return false;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        qWarning() << "❌ Cannot find stream info!";
        return false;
    }

    for (unsigned i = 0; i < fmtCtx->nb_streams; ++i) {
        AVMediaType type = fmtCtx->streams[i]->codecpar->codec_type;
        if (debug) qDebug() << "[Stream] index:" << i << "type:" << av_get_media_type_string(type);

        if (type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qWarning() << "❌ Cannot find video stream!";
        return false;
    }

    videoStream = fmtCtx->streams[videoStreamIndex];


    AVCodec* codec = avcodec_find_decoder_by_name("hevc_cuvid");
    if (!codec) {
        qWarning() << "❌ Cannot find HEVC CUVID decoder!";
        return false;
    } else {
        if (debug) qDebug() << "✅ Loaded CUVID decoder:" << codec->name;
    }

    qDebug()<<"1:"<<codecCtx;
    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, videoStream->codecpar);

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        qWarning() << "❌ Cannot open codec!";
        return false;
    }

    qDebug()<<"2:"<<codecCtx;
    frame = av_frame_alloc();
    packet = av_packet_alloc();

    width = codecCtx->width;
    height = codecCtx->height;

    rgbFrame = av_frame_alloc();

    AVRational frameRate = videoStream->avg_frame_rate;
    if (frameRate.num == 0 || frameRate.den == 0) {
        frameRate = videoStream->r_frame_rate;
    }

    double fps = 0.0;
    if (frameRate.den != 0) {
        fps = av_q2d(frameRate);
        qDebug() << "FPS:" << fps;
    } else {
        qWarning() << "無法取得有效的 FPS";
    }
    FrameData.fps = static_cast<int>(fps);  // 結果是 29（自動去掉小數點）
    FrameData.FrameCount = getTotalFrames();
    qDebug() << "FrameCount:" << FrameData.FrameCount;


    return true;
}

FrameData VideoDecoder::getFrameData() {
    return FrameData;
}

QImage VideoDecoder::getNextFrame() {
    static bool flushing = false;

    while (true) {
        if (stopRequested) {  // 🚨 外部要求停止
            avcodec_flush_buffers(codecCtx);  // 清掉 decoder queue
            flushing = false;
            return QImage();
        }

        int ret = 0;

        if (!flushing) {
            ret = av_read_frame(fmtCtx, packet);

            if (ret < 0) {
                ret = avcodec_send_packet(codecCtx, nullptr);
                flushing = true;
                if (ret < 0) return QImage();
                continue;
            }

            if (packet->stream_index != videoStreamIndex) {
                av_packet_unref(packet);
                continue;
            }
            ret = avcodec_send_packet(codecCtx, packet);
            av_packet_unref(packet);
            if (ret < 0) continue;
        }

        // ====== 量測 receive_frame（解碼出影格）的耗時 ======
        auto t0 = std::chrono::high_resolution_clock::now();
        ret = avcodec_receive_frame(codecCtx, frame);

        if (ret == AVERROR(EAGAIN)) {
            if (flushing) {
                flushing = false;
                return QImage();
            }
            continue;
        } else if (ret == AVERROR_EOF) {
            flushing = false;
            return QImage();
        } else if (ret < 0) {
            return QImage();
        }

        // ✅ 每次用完 frame 要清
        rgbFrame = av_frame_alloc();

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
        buffer = (uint8_t*)av_malloc(numBytes);
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);





        swsCtx = sws_getCachedContext(swsCtx, width, height, (AVPixelFormat)frame->format,
                                      width, height, AV_PIX_FMT_RGB24, SWS_BILINEAR,
                                      nullptr, nullptr, nullptr);
        auto t1 = std::chrono::high_resolution_clock::now();
        // 只有成功拿到一張 frame 才會走到這裡
        double decode_ms =
            std::chrono::duration<double, std::milli>(t1 - t0).count();

        // 取得 I/P/B 型別
        char pict = av_get_picture_type_char(frame->pict_type);
        ++decodedFrameIndex;
//        qDebug() << "[Decode]"
//                 << "frame#" << decodedFrameIndex
//                 << "type:" << QString::fromLatin1(&pict, 1)
//                 << "decode_ms:" << QString::number(decode_ms, 'f', 5);

        sws_scale(swsCtx, frame->data, frame->linesize, 0, height,
                  rgbFrame->data, rgbFrame->linesize);



        QImage img(rgbFrame->data[0], width, height, rgbFrame->linesize[0], QImage::Format_RGB888);


        FrameData.FrameImage = img.copy();




        // ✅ 資源釋放
        av_free(buffer);
        buffer = nullptr;

        av_frame_free(&rgbFrame);
        av_frame_unref(frame);

        return FrameData.FrameImage;
    }
}

void VideoDecoder::stop() {
    stopRequested = true;
}

int VideoDecoder::getTotalFrames() const {
    if (!videoStream) return 0;
    return videoStream->nb_frames;
}

double VideoDecoder::getFrameRate() const {
    if (!videoStream) return 0.0;

    AVRational r = videoStream->avg_frame_rate;
    if (r.den != 0)
        return av_q2d(r);
    else
        return 0.0;
}

void VideoDecoder::release() {

    qDebug()<<"5:"<<codecCtx;
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (rgbFrame) {
        av_frame_free(&rgbFrame);
        rgbFrame = nullptr;
    }

    if (swsCtx) {
        sws_freeContext(swsCtx);
        swsCtx = nullptr;
    }
//    qDebug()<<"test123456:"<<codecCtx;

    if (codecCtx) {
        // ❗️這一步很重要，會釋放 GPU buffer pool（NVDEC）
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }

    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
        fmtCtx = nullptr;
    }

    videoStream = nullptr;
    videoStreamIndex = -1;
}

VideoDecoder::~VideoDecoder() {
    release();
}

// 幀號 -> pts（以 videoStream->time_base 為單位）
// 若沒有 fps，保守回傳 0
int64_t VideoDecoder::ptsFromFrameIndex(int64_t idx) const {
    if (!videoStream) return 0;
    double fps = getFrameRate();
    if (fps <= 0.0) return 0;

    // 幀號 -> 秒 -> pts
    double seconds = static_cast<double>(idx) / fps;
    AVRational tb = videoStream->time_base; // e.g. 1/90000
    // seconds / (tb.num/tb.den) = seconds * tb.den / tb.num
    double pts_d = seconds * tb.den / tb.num;

    // 四捨五入成整數 pts
    if (pts_d < 0) pts_d = 0;
    return static_cast<int64_t>(llround(pts_d));
}

// pts（stream time_base）-> 幀號估算
int64_t VideoDecoder::frameIndexFromPts(int64_t pts) const {
    if (!videoStream) return 0;
    double fps = getFrameRate();
    if (fps <= 0.0) return 0;

    AVRational tb = videoStream->time_base;
    // pts * (tb.num/tb.den) = seconds
    double seconds = static_cast<double>(pts) * tb.num / tb.den;
    double idx_d = seconds * fps;
    // 使用四捨五入可減少 B-frame 重排造成的小數誤差
    if (idx_d < 0) idx_d = 0;
    return static_cast<int64_t>(llround(idx_d));
}

// 主要 API：指定第 N 幀（0-based）取圖
QImage VideoDecoder::getFrameAt(int64_t targetIndex) {
    if (!fmtCtx || !codecCtx || videoStreamIndex < 0 || !videoStream) {
        qWarning() << "Decoder not initialized.";
        return QImage();
    }
    if (targetIndex < 0) targetIndex = 0;

    // 1) 計算目標 pts 並 seek 到就近關鍵幀（往回）
    int64_t targetPts = ptsFromFrameIndex(targetIndex);
    int seekFlags = AVSEEK_FLAG_BACKWARD;
    if (av_seek_frame(fmtCtx, videoStreamIndex, targetPts, seekFlags) < 0) {
        qWarning() << "av_seek_frame failed, try to seek to 0";
        if (av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD) < 0) {
            qWarning() << "av_seek_frame to 0 also failed.";
            return QImage();
        }
    }

    // 2) Flush 解碼器狀態，從 seek 點重新送包
    avcodec_flush_buffers(codecCtx);

    // 3) 從 seek 後開始讀包解碼，直到抵達/超過目標幀
    QImage result;
    bool flushing = false;

    // 構建一個可重用的 RGB 緩衝（建議你在 openFile() 時就配置好並重用，這裡為簡潔直接配置）
    AVFrame* rgbFrameLocal = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    uint8_t* bufferLocal = (uint8_t*)av_malloc(numBytes);
    av_image_fill_arrays(rgbFrameLocal->data, rgbFrameLocal->linesize, bufferLocal,
                         AV_PIX_FMT_RGB24, width, height, 1);

    // swsCtx 可重用；若你在 openFile() 已建立 cached context，可直接使用
    swsCtx = sws_getCachedContext(
        swsCtx,
        width, height, (AVPixelFormat)videoStream->codecpar->format, // 注意：有些硬解輸出格式在 codecCtx->pix_fmt
        width, height, AV_PIX_FMT_RGB24,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
    );

    // 追蹤「目前解到第幾幀」（用 best_effort_timestamp 估算）
    int64_t currentIndexEstimate = 0;

    for (;;) {
        int ret = 0;

        if (!flushing) {
            ret = av_read_frame(fmtCtx, packet);
            if (ret < 0) {
                // 檔案讀完，送 NULL 讓解碼器吐完緩衝
                ret = avcodec_send_packet(codecCtx, nullptr);
                flushing = true;
                if (ret < 0) break;
                continue;
            }

            if (packet->stream_index != videoStreamIndex) {
                av_packet_unref(packet);
                continue;
            }

            ret = avcodec_send_packet(codecCtx, packet);
            av_packet_unref(packet);
            if (ret < 0) {
                // 解碼器暫時吃不下或錯誤，繼續讀
                continue;
            }
        }

        ret = avcodec_receive_frame(codecCtx, frame);
        if (ret == AVERROR(EAGAIN)) {
            if (flushing) break; // 已在 flush 階段還 EAGAIN，視為結束
            continue;
        } else if (ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            qWarning() << "receive_frame error:" << ret;
            break;
        }

        // 以 best_effort_timestamp 估算目前幀號（避免 B-frame 亂序）
        int64_t ts = (frame->best_effort_timestamp != AV_NOPTS_VALUE)
                         ? frame->best_effort_timestamp
                         : frame->pts; // 退而求其次
        if (ts == AV_NOPTS_VALUE) {
            // 沒有時間戳：只能從 seek 後累加
            // 注意：這種情況較少見，但仍做個保護
            currentIndexEstimate++;
        } else {
            currentIndexEstimate = frameIndexFromPts(ts);
        }

        // 一旦抵達/超過目標幀，就轉成 QImage 回傳
        if (currentIndexEstimate >= targetIndex) {
            // swscale（若你有導入 libyuv，可改用 libyuv 進行 NV12/I420 -> ARGB 更快）
            swsCtx = sws_getCachedContext(swsCtx,
                                          width, height, (AVPixelFormat)frame->format,
                                          width, height, AV_PIX_FMT_RGB24,
                                          SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

            sws_scale(swsCtx, frame->data, frame->linesize, 0, height,
                      rgbFrameLocal->data, rgbFrameLocal->linesize);

            // 建 QImage（為避免生命週期問題，copy 一份）
            QImage img(rgbFrameLocal->data[0], width, height,
                       rgbFrameLocal->linesize[0], QImage::Format_RGB888);
            result = img.copy();

            // 額外：印出該幀型別與索引
            char pict = av_get_picture_type_char(frame->pict_type);
            qDebug() << "[getFrameAt] target" << targetIndex
                     << "got idx" << currentIndexEstimate
                     << "type:" << QChar(pict);

            av_frame_unref(frame);
            break;
        }

        av_frame_unref(frame);
    }

    // 釋放臨時 RGB 資源（若你改成全域重用，這段請移到 release()）
    if (bufferLocal) av_free(bufferLocal);
    if (rgbFrameLocal) av_frame_free(&rgbFrameLocal);

    return result; // 若失敗則為空 QImage
}
