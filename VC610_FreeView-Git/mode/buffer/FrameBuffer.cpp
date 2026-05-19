// FrameBuffer.cpp
#include "FrameBuffer.h"
#include <QDebug>
#include <QMetaObject>
#include <QMutexLocker>
#include <algorithm>


#include <QImage>
#include <QFile>
#include <QDir>
#include <QDateTime>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

FrameBuffer::FrameBuffer(int frameRate, int durationSeconds, int maxFrameSizeKB)
    : maxFrames(frameRate * durationSeconds),
      maxFrameSize(maxFrameSizeKB * 1024),
      currentIndex(0),
      filledOnce(false),
      switched(false)
{
    switchCooldown.start();  // 開始冷卻計時器
}

bool FrameBuffer::shouldSyncBuffers() {
    int minIFrames = INT_MAX;

    for (const auto& pair : ipBuffers) {
        const auto& buf = pair.activeBuffer;
        int iCount = std::count_if(buf.begin(), buf.end(), [](const FrameItem& f) {
            return f.type == 1;
        });
        if (iCount < 2)  // 每台至少要有兩個 I-frame 才算穩定
            return false;
        minIFrames = std::min(minIFrames, iCount);
    }
    return true;
}

void FrameBuffer::addFrame(const char* ipChar, const ao::lm_video_data& input)
{
    if (input.len > static_cast<uint32_t>(maxFrameSize)) {
        qWarning() << "Frame size exceeds maximum of" << maxFrameSize << "bytes!";
        return;
    }

    QString ip = QString::fromUtf8(ipChar);
    QMutexLocker locker(&mutex);

    if (!ipBuffers.contains(ip)) {
        ipBuffers.insert(ip, FramePair(maxFrames));
    }

    FramePair& bufferPair = ipBuffers[ip];
    std::vector<FrameItem>& activeBuf = bufferPair.activeBuffer;

    // ⚙️ 正常寫入流程
    FrameItem& item = activeBuf[bufferPair.currentIndex];

    bool isOverwriting = (item.type != 0);
    if (isOverwriting) {
        int idx = (bufferPair.currentIndex + 1) % maxFrames;
        int safetyCounter = 0;
        while (safetyCounter < maxFrames) {
            FrameItem& nextItem = activeBuf[idx];
            if (nextItem.type == 1) {
                break;
            } else if (nextItem.type == 2) {
                nextItem.type = 0;
                nextItem.len = 0;
                nextItem.data.clear();
            } else {
                break;
            }
            idx = (idx + 1) % maxFrames;
            ++safetyCounter;
        }
    }

    item.ip = ip;
    item.len = input.len;
    item.ntp_timestamp = input.ntp_timestamp;
    item.frm_no = input.frm_no;
    item.type = input.type;
    item.data = QByteArray(reinterpret_cast<const char*>(input.data), static_cast<int>(input.len));
    item.bufferIndex = bufferPair.currentIndex;  // ✅ 紀錄在 activeBuffer 的位置

    bufferPair.currentIndex = (bufferPair.currentIndex + 1) % maxFrames;
    if (bufferPair.currentIndex == 0) {
        bufferPair.filledOnce = true;
    }

    // ✅ 只同步一次
    static bool hasSyncedOnce = false;
    if (hasSyncedOnce)
        return;

    // 檢查所有 IP 至少兩個 I-frame 才做同步
    for (const auto& pair : ipBuffers) {
        const auto& buf = pair.activeBuffer;
        int iCount = std::count_if(buf.begin(), buf.end(), [](const FrameItem& f) {
            return f.type == 1;
        });
        if (iCount < 1)
            return;
    }

    int minIFrameNo = INT_MAX;
    qDebug() << "🔍 INT_MAX:" << INT_MAX;

    QMap<QString, int> ipToFirstIFrameNo;

    for (auto it = ipBuffers.begin(); it != ipBuffers.end(); ++it) {
        const QString& ip = it.key();
        const auto& buf = it.value().activeBuffer;

        int firstI = -1;
        for (const FrameItem& f : buf) {
            if (f.type == 1) {
                firstI = f.frm_no;
                break;
            }
        }

        if (firstI == -1) {
            qWarning() << "⚠️ 找不到 I-frame，IP:" << ip;
            return;
        }

        ipToFirstIFrameNo[ip] = firstI;
        minIFrameNo = std::min(minIFrameNo, firstI);
        qDebug() << "✅ IP:" << ip << " 第一張 I-frame 幀號:" << firstI;
    }

    qDebug() << "🔹 最早的 I-frame 幀號（minIFrameNo）:" << minIFrameNo;

    // 2️⃣ 清除比 minIFrameNo 更早的完整 I-GOP 並補齊
    for (auto it = ipBuffers.begin(); it != ipBuffers.end(); ++it) {
        const QString& ip = it.key();
        std::vector<FrameItem>& buf = it.value().activeBuffer;

        int firstI = ipToFirstIFrameNo[ip];
        if (firstI > minIFrameNo) {
            qDebug() << "⏭️ IP:" << ip << " 的 I-frame 幀號較晚，不清除（" << firstI << " > " << minIFrameNo << ")";
            continue;
        }

        // 找第一個 I-frame 的 index
        int firstIIndex = -1;
        for (int i = 0; i < buf.size(); ++i) {
            if (buf[i].type == 1) {
                firstIIndex = i;
                break;
            }
        }

        if (firstIIndex == -1 || buf[firstIIndex].frm_no > minIFrameNo) {
            qDebug() << "⛔ IP:" << ip << " 找不到有效 I 或幀號比同步點新，不處理。";
            continue;
        }

        // 找下一個 I-frame
        int nextIIndex = buf.size();
        for (int i = firstIIndex + 1; i < buf.size(); ++i) {
            if (buf[i].type == 1) {
                nextIIndex = i;
                break;
            }
        }

        qDebug() << "⚠️ 清除早期 I-GOP（保留第一幀）IP:" << ip
                 << " index:" << (firstIIndex + 1) << "～" << (nextIIndex - 1)
                 << " 幀號範圍:"
                 << (firstIIndex + 1 < buf.size() ? buf[firstIIndex + 1].frm_no : -1)
                 << "～"
                 << (nextIIndex - 1 < buf.size() ? buf[nextIIndex - 1].frm_no : -1);

        // ✅ 清掉 I-frame 之後的早期 I-GOP（保留第一幀）
        for (int i = firstIIndex + 1; i < nextIIndex; ++i)
            buf[i] = FrameItem();

        // ✂️ Compact
        std::vector<FrameItem> compacted;
        compacted.reserve(buf.size());
        for (const FrameItem& f : buf) {
            if (f.type == 1 || f.type == 2)
                compacted.push_back(f);
        }

        int compactSize = compacted.size();
        for (int i = 0; i < buf.size(); ++i) {
            if (i < compactSize)
                buf[i] = compacted[i];
            else
                buf[i] = FrameItem();
        }

        // 更新寫入 index
        FramePair& pair = it.value();
        pair.currentIndex = compactSize % buf.size();
        pair.filledOnce = (compactSize >= buf.size());

        qDebug() << "🧹 清理完畢 IP:" << ip
                 << " 新的 currentIndex:" << pair.currentIndex
                 << " filledOnce:" << pair.filledOnce
                 << " buffer 剩餘幀數:" << compactSize;
    }

    hasSyncedOnce = true;
    qDebug() << "✅ 完成第一次同步！";
}

inline void checkFrameLeaks(const QList<AVFrame*>& list, const QString& name) {
    int alive = 0;
    for (int i = 0; i < list.size(); ++i) {
        AVFrame* f = list[i];
        if (f) {
            qDebug() << QString("    ⚠️ %1[%2] 尚未釋放: ptr=%3 data[0]=%4")
                        .arg(name)
                        .arg(i)
                        .arg((quintptr)f, 0, 16)
                        .arg((quintptr)f->data[0], 0, 16);
            ++alive;
        }
    }

    if (alive == 0) {
        qDebug() << QString("    ✅ %1 所有 AVFrame 已釋放").arg(name);
    } else {
        qDebug() << QString("    ❗ %1 尚有 %2 個 AVFrame 未釋放").arg(name).arg(alive);
    }
}

void FrameBuffer::composeAndSaveGridFrameSafe_GPU(const QList<AVFrame*>& frames, const QString& filename) {
    if (frames.isEmpty())
        return;

    // 預設 2x2 拼接範例
//    QVector<QVector<GridCell>> gridLayout = {
//        { {"172.16.0.1", false}, {"172.16.0.3", true} },
//        { {"172.16.0.4", true}, {"172.16.0.5", false} },
//    };
    QVector<QVector<GridCell>> gridLayout = {
        { {"172.16.0.1", false}, {"172.16.0.2", true} },
        { {"172.16.0.1", true}, {"172.16.0.2", false} },
    };
    const int singleWidth = 1920;
    const int singleHeight = 1080;
    int count = frames.size();
    int cols = std::ceil(std::sqrt(count));
    int rows = std::ceil(static_cast<double>(count) / cols);
    int outWidth = (cols * singleWidth + 1) & ~1;
    int outHeight = (rows * singleHeight + 1) & ~1;

    AVFrame* canvas = av_frame_alloc();
    canvas->format = AV_PIX_FMT_NV12;
    canvas->width = outWidth;
    canvas->height = outHeight;

    if (av_image_alloc(canvas->data, canvas->linesize, outWidth, outHeight, AV_PIX_FMT_NV12, 1) < 0) {
        qWarning() << "❌ Failed to allocate canvas image buffer";
        av_frame_free(&canvas);
        return;
    }

    QSet<int> rotatedIndex = {1, 2};  // 需要旋轉的畫面索引

    // 拼接到 canvas
    for (int i = 0; i < count; ++i) {
        AVFrame* src = frames[i];
        if (!src || src->format != AV_PIX_FMT_NV12) continue;

        int row = i / cols;
        int col = i % cols;
        bool rotate = rotatedIndex.contains(i);

        // ✅ Y 平面旋轉
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const GridCell& cell = gridLayout[row][col];
                QString ip = cell.ip;
                bool rotate = cell.rotate;

                if (!decodedFrameBuffer.contains(ip)) {
                    tmpCount++;
                    qDebug() << decodedFrameBuffer[ip].size();
                    qWarning() << "⚠️ IP" << ip << "沒有 frame 可用1";
                    return;
                }
                if (decodedFrameBuffer[ip].isEmpty()) {
                    tmpCount++;
                    qWarning() << "⚠️ IP" << ip << "沒有 frame 可用2";
                    return;
                }
                AVFrame* src = decodedFrameBuffer[ip].first();  // 你可以用 takeFirst() 或 clone
                if (!src || src->format != AV_PIX_FMT_NV12) continue;

                // --- Y 平面 ---
                for (int y = 0; y < singleHeight; ++y) {
                    int srcY = rotate ? (singleHeight - 1 - y) : y;
                    uint8_t* srcLine = src->data[0] + srcY * src->linesize[0];
                    uint8_t* dstLine = canvas->data[0] + (row * singleHeight + y) * canvas->linesize[0] + col * singleWidth;

                    if (rotate) {
                        for (int x = 0; x < singleWidth; ++x)
                            dstLine[x] = srcLine[singleWidth - 1 - x];
                    } else {
                        memcpy(dstLine, srcLine, singleWidth);
                    }
                }

                // --- UV 平面 ---
                for (int y = 0; y < singleHeight / 2; ++y) {
                    int srcY = rotate ? (singleHeight / 2 - 1 - y) : y;
                    uint8_t* srcLine = src->data[1] + srcY * src->linesize[1];
                    uint8_t* dstLine = canvas->data[1] + (row * (singleHeight / 2) + y) * canvas->linesize[1] + col * singleWidth;

                    if (rotate) {
                        for (int x = 0; x < singleWidth; x += 2) {
                            dstLine[x] = srcLine[singleWidth - 2 - x];
                            dstLine[x + 1] = srcLine[singleWidth - 1 - x];
                        }
                    } else {
                        memcpy(dstLine, srcLine, singleWidth);
                    }
                }
            }
        }
    }

//    if (!encoder.isInitialized()) {
//        if (!encoder.initialize(filename, outWidth, outHeight, 60)) {
//            av_freep(&canvas->data[0]);
//            av_frame_free(&canvas);
//            return;
//        }
//    }

    // ⚠️ GPU frame 建立與轉換
    AVFrame* gpuFrame = av_frame_alloc();
    gpuFrame->format = AV_PIX_FMT_CUDA;
    gpuFrame->width = outWidth;
    gpuFrame->height = outHeight;

    AVBufferRef* hw_frames_ctx = encoder.getHWFramesContext();

    if (!hw_frames_ctx) {
        qWarning() << "❌ Encoder did not return hw_frames_ctx";
        av_frame_free(&gpuFrame);
        av_freep(&canvas->data[0]);
        av_frame_free(&canvas);
        return;
    }


//    qDebug() << "🎯 使用完 gpuFrame 後 hw_frames_ctx 引用數:"
//             << av_buffer_get_ref_count(encoder.getHWFramesContext());

//    gpuFrame->hw_frames_ctx = av_buffer_ref(hw_frames_ctx);
//    av_buffer_unref(&gpuFrame->hw_frames_ctx);

//    return;
    if (av_hwframe_get_buffer(hw_frames_ctx, gpuFrame, 0) < 0) {
        qWarning() << "❌ av_hwframe_get_buffer failed";
        av_frame_free(&gpuFrame);
        av_freep(&canvas->data[0]);
        av_frame_free(&canvas);
        return;
    }

    if (!gpuFrame->hw_frames_ctx) {
        qWarning() << "❌ av_buffer_ref failed";
        av_frame_free(&gpuFrame);
        av_freep(&canvas->data[0]);
        av_frame_free(&canvas);
        return;
    }

    if (av_hwframe_transfer_data(gpuFrame, canvas, 0) < 0) {
        qWarning() << "❌ av_hwframe_transfer_data failed";
        av_frame_free(&gpuFrame);
        av_freep(&canvas->data[0]);
        av_frame_free(&canvas);
        return;
    }


    encoder.submitFrame(gpuFrame);  // 這份進 queue

    // 再釋放你原本的 gpuFrame
    av_buffer_unref(&gpuFrame->hw_frames_ctx);
    av_frame_unref(gpuFrame);
    av_frame_free(&gpuFrame);
//    qDebug() << "🎯 使用完 gpuFrame 後 hw_frames_ctx 引用數3:"
//             << av_buffer_get_ref_count(encoder.getHWFramesContext());
//    qDebug()<<"sp done";
//    return;
    tmpCount++;
    av_freep(&canvas->data[0]);  // 自動釋放整個 AV_PIX_FMT_NV12 所需記憶體
    av_frame_unref(canvas);
    av_frame_free(&canvas);      // 再釋放 AVFrame 結構本身
    qDebug()<<"tmpCount:"<<tmpCount;

    if (tmpCount >= videoFrame) {

        qDebug() << "影片完成，停止錄製";
        isDetecting = true;
        startcompose = false;
        encoder.finish();
        tmpCount = 0;

        // ✅ 完整釋放 decodedFrameBuffer 的 AVFrame 並清空整個 map
        for (auto& list : decodedFrameBuffer) {
            for (AVFrame*& f : list) {
                if (f) {
                    av_frame_unref(f);
                    av_frame_free(&f);
                    f = nullptr;
                }
            }
            list.clear();
        }
        decodedFrameBuffer.clear();  // ✅ 釋放掉整個 map 的 key-value 結構

        // 記憶體與狀態檢查
        qDebug() << "🧹 拼接與編碼結束，進行釋放狀態檢查：";
        qDebug() << "    🔍 canvas             :" << (canvas ? "❗未釋放" : "✅ OK");
        qDebug() << "    🔍 hw_frames_ctx      :" << (encoder.getHWFramesContext() ? "✅ 存在" : "⚠️ NULL");
        qDebug() << "    🔍 encoder 初始化狀態:" << (encoder.isInitialized() ? "❗仍在運行" : "✅ 已停止");

        checkFrameLeaks(pendingCloneFrames, "pendingCloneFrames");
        checkFrameLeaks(framesToCompose, "framesToCompose");
        checkFrameLeaks(decodedFrameBuffer["172.16.0.1"], "decodedFrameBuffer[172.16.0.1]");
        checkFrameLeaks(decodedFrameBuffer["172.16.0.2"], "decodedFrameBuffer[172.16.0.2]");
//        checkFrameLeaks(decodedFrameBuffer["172.16.0.3"], "decodedFrameBuffer[172.16.0.3]");
//        checkFrameLeaks(decodedFrameBuffer["172.16.0.4"], "decodedFrameBuffer[172.16.0.4]");
//        checkFrameLeaks(decodedFrameBuffer["172.16.0.5"], "decodedFrameBuffer[172.16.0.5]");
    }
}

void FrameBuffer::checkAndComposeFrames() {
    QMutexLocker locker(&bufferMutex);

    if (decodedFrameBuffer.isEmpty() || !startcompose)
    {
        return;
    }


//    QStringList requiredIPs = { "172.16.0.1", "172.16.0.3", "172.16.0.4" , "172.16.0.5" };
    QStringList requiredIPs = { "172.16.0.1", "172.16.0.2"};

    for (const QString& ip : requiredIPs) {
        if (!decodedFrameBuffer.contains(ip)) {
//            qWarning() << "❌ IP" << ip << "不存在於 buffer";
            return;
        }

        int validFrameCount = 0;
        for (AVFrame* f : decodedFrameBuffer[ip]) {
            if (f && f->width > 0 && f->height > 0 && f->format == AV_PIX_FMT_NV12) {
                validFrameCount++;
                if (validFrameCount >= 2)
                    break;
            }
        }

        if (validFrameCount < 2) {
//            qWarning() << "❌ IP" << ip << " 有效 frame 不足 (只有" << validFrameCount << ")";
            return;
        }
    }

    int totalFrames = 4;  // 固定 2x2 拼接

    // 取得有效 IP 與其有效 frame
    QList<QString> validIps;
    for (auto it = decodedFrameBuffer.begin(); it != decodedFrameBuffer.end(); ++it) {
        const QString& ip = it.key();
        const QList<AVFrame*>& frames = it.value();

        bool hasValid = false;
        for (AVFrame* f : frames) {
            if (f && f->width > 0 && f->height > 0 && f->format == AV_PIX_FMT_NV12) {
                hasValid = true;
                break;
            }
        }
        if (hasValid)
            validIps.append(ip);
    }

    int ipCount = validIps.size();
    if (ipCount == 0)
        return;

    int framesPerIPToUse = qMax(1, totalFrames / ipCount);
    int remainingFrames = totalFrames;

    QList<AVFrame*> framesToCompose;

    // 從每個有效 IP 取出可用 frames（clone 後加入 framesToCompose）
    for (const QString& ip : validIps) {
        auto& buffer = decodedFrameBuffer[ip];
        if (buffer.size() < framesPerIPToUse)
            continue;

        int numToTake = qMin(framesPerIPToUse, remainingFrames);
        int taken = 0;

        for (int i = 0; i < buffer.size() && taken < numToTake; ++i) {
            AVFrame* f = buffer[i];
            if (f && f->width > 0 && f->height > 0 && f->format == AV_PIX_FMT_NV12) {
                AVFrame* clone = av_frame_clone(f);
                if (clone) {
                    framesToCompose.append(clone);
                    taken++;
                    remainingFrames--;
                }
            }
        }

        if (remainingFrames <= 0)
            break;
    }

    if (framesToCompose.size() < totalFrames) {
        // 不足的話不拼接，避免失敗
        for (AVFrame* f : framesToCompose) {
            av_frame_unref(f);
            av_frame_free(&f);
        }
        framesToCompose.clear();
        return;
    }

    // 呼叫拼接函式
    qDebug()<<"1:"<<decodedFrameBuffer["172.16.0.1"].size()<<"2:"<<decodedFrameBuffer["172.16.0.2"].size()
            <<"3:"<<decodedFrameBuffer["172.16.0.3"].size()
            <<"4:"<<decodedFrameBuffer["172.16.0.4"].size()<<"5:"<<decodedFrameBuffer["172.16.0.5"].size();

    composeAndSaveGridFrameSafe_GPU(framesToCompose, tmpfilename);

    // 釋放 clone 出來的 frame
    for (AVFrame* f : framesToCompose) {
        av_frame_unref(f);
        av_frame_free(&f);
    }
    framesToCompose.clear();

//     移除原 decodedFrameBuffer 中用過的 frame
    for (const QString& ip : validIps) {
        auto& buffer = decodedFrameBuffer[ip];
        int removeCount = framesPerIPToUse;
        while (removeCount > 0 && !buffer.isEmpty()) {
            AVFrame* f = buffer.takeFirst();
            if (f) {
                av_frame_unref(f);
                av_frame_free(&f);
            }
            removeCount--;
        }
    }
}

void FrameBuffer::decodeCapturedFrames(const QString& ip, const QList<FrameItem>& inputFrames) {
    // 檢查是否有該 IP 的解碼器
    if (!camMap.contains(ip)) {
        qWarning() << "No decoder for IP" << ip;
        return;
    }

    qDebug() << "dec start:" << ip;
    const int MAX_BUFFER_SIZE = bufferObj["bufferSize"].toInt();

    auto cam = camMap[ip];

    if (!cam) {
        qWarning() << "camMap[ip] is nullptr for IP:" << ip;
        return;
    }

    if (!cam->dc) {
        qWarning() << "cam->dc is nullptr for IP:" << ip;
        return;
    }

    qDebug() << "dec start1:" << ip;

    QList<FrameItem> frames = inputFrames;

    // 逐幀處理擷取到的 FrameItem
    while (!frames.isEmpty()) {
           FrameItem frame = frames.takeFirst();

           if (frame.type == 0 || frame.data.isEmpty() || frame.len <= 0) {
               continue;
           }

           AVPacket* packet = av_packet_alloc();
           if (!packet || av_new_packet(packet, frame.len) < 0 || frame.data.size() < frame.len) {
               qWarning() << "❌ Failed to create AVPacket";
               av_packet_free(&packet);
               continue;
           }

           memcpy(packet->data, frame.data.constData(), frame.len);

           if (cam->dc->send_decode_pkt(packet) < 0) {
               qWarning() << "❌ Failed to send packet to decoder";
               av_packet_free(&packet);
               continue;
           }
           av_packet_free(&packet);

           while (true) {
               AVFrame* gpuFrame = av_frame_alloc();
               if (!gpuFrame)
                   break;

               int ret = cam->dc->get_decode_frame_GPU(gpuFrame);
               if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                   av_frame_free(&gpuFrame);
                   break;
               } else if (ret < 0) {
                   av_frame_free(&gpuFrame);
                   break;
               }

               AVFrame* cpuFrame = av_frame_alloc();
               cpuFrame->format = AV_PIX_FMT_NV12;
               cpuFrame->width = gpuFrame->width;
               cpuFrame->height = gpuFrame->height;

               if (av_hwframe_transfer_data(cpuFrame, gpuFrame, 0) < 0) {
                   qWarning() << "❌ GPU → CPU frame transfer failed";
                   av_frame_free(&gpuFrame);
                   av_frame_free(&cpuFrame);
                   break;
               }
               av_frame_free(&gpuFrame);

               // 🎯 加鎖僅針對該 IP 的 buffer，避免鎖全域

               QMutexLocker locker(&bufferMutex);  // 建議改為 per-IP mutex，如果你之後要 scale
               auto& list = decodedFrameBuffer[ip];

               const int maxSize = bufferObj["bufferSize"].toInt();
               while (list.size() >= maxSize) {
                   AVFrame* old = list.takeFirst();
                   av_frame_free(&old);
               }
               list.append(cpuFrame);
           }
       }

       // 解碼結束時可選擇記錄 debug 訊息

       QMutexLocker locker(&bufferMutex);
//       auto& list = decodedFrameBuffer[ip];
       qDebug() << "解碼完成 IP:" << ip << " buffer size:" << decodedFrameBuffer[ip].size();
}

void FrameBuffer::processAudioData(const ao::lm_audio_data& input)
{
    std::thread([this, input]() {
        // 建立分析器分析音量分貝
        AudioAnalyzer analyzer;
        db = analyzer.analyze(input);
        qDebug()<<"DB:"<<db.db;
//        float Hz = analyzer.analyzeFrequency(db.samples, db.sampleRate);
        qDebug()<<"bufferObj.toDouble():"<<bufferObj["db"].toDouble();

        // 若分貝高於設定的門檻值（從 JSON 設定檔中取得）
        if (db.db >= bufferObj["db"].toDouble()) {
//            analyzer.analyzeFrequency(db.samples, db.sampleRate);
            decodedFrameBuffer.clear();  // ✅ 清空已解碼緩衝區

            // 若尚未啟用偵測，則跳出
            if (!isDetecting)
            {
                qDebug()<<"isDetecting false";
                return;
            }
            qDebug()<<"===============================================";

            int actualFrames = 0;
            int remainingFrames = 0;

            // 上鎖保護 ipBuffers 與 switchCooldown 的操作
            QMutexLocker locker(&mutex);

            // 判斷是否在冷卻時間內，避免太頻繁切換
            if (switchCooldown.elapsed() < cooldownIntervalMs)
                return;

            // 重新啟動冷卻計時器
            switchCooldown.restart();

            // 先解鎖避免在 sleep 過程中鎖定整個系統
            locker.unlock();

            // 遍歷每一台相機的 buffer，統計 I/P 幀數
            for (auto it = ipBuffers.begin(); it != ipBuffers.end(); ++it) {
                actualFrames = 0;
                FramePair& pair = it.value();
                const auto& active = pair.activeBuffer;

                for (const FrameItem& f : active) {
                    if (f.type == 1 || f.type == 2)
                        actualFrames++;
                }

                // 若幀數不足 300，補上額外等待時間
                if (actualFrames < 300) {
                    remainingFrames = (300 - actualFrames) * 20;  // 每幀約 17ms
                }
            }

            qDebug() << "actualFrames:" << actualFrames;
            qDebug() << "remainingFrames:" << remainingFrames;
            qDebug() << "actualFrames:" << actualFrames;
            qDebug() << "frameCount:" << frameCount;
            int wait = waitSec + remainingFrames;

            qDebug() << "wait:" << wait;
            isDetecting =false;

            // 延遲一段時間以確保更多資料被收集
            std::this_thread::sleep_for(std::chrono::milliseconds(wait));


            // 延遲完成後重新上鎖
            locker.relock();

            qDebug() << "start";


            // 建立輸出檔案路徑
            timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
            filename = QString("output_%1.mp4").arg(timestamp);
            tmpfilename = "C:/Users/USER/Desktop/FreeviewLive-win32-x64/www/video/tmp/" + filename;
            filename = "C:/Users/USER/Desktop/FreeviewLive-win32-x64/www/video/" + filename;

            qDebug() << filename;

            encoder.Filename = filename;
            encoder.initialize(tmpfilename, 3840, 2160, 60);  // 4K, 60fps

//            encoder.finish();
            // 初始化編碼器

            // 遍歷每一台相機的緩衝區，準備擷取資料
            for (auto it = ipBuffers.begin(); it != ipBuffers.end(); ++it) {
                FramePair& pair = it.value();

                int oldCurrentIndex = pair.currentIndex;
                bool wasFilledOnce = pair.filledOnce;

                // 切換 active / locked buffer
                std::swap(pair.activeBuffer, pair.lockedBuffer);

                // 清空新的 active buffer，讓其開始接收新的資料
                for (int i = 0; i < maxFrames; ++i)
                    pair.activeBuffer[i] = FrameItem();

                pair.currentIndex = 0;
                pair.filledOnce = false;

                // 從 lockedBuffer 往回搜尋 I-frame 起始點
//                int maxFrames = pair.lockedBuffer.size();
//                int lastWriteIndex = (oldCurrentIndex == 0) ? maxFrames - 1 : oldCurrentIndex - 1;
//                int startSearchIndex = (lastWriteIndex + maxFrames - frameCount) % maxFrames;
                int maxFrames = pair.lockedBuffer.size();
                int lastWriteIndex = (oldCurrentIndex == 0) ? maxFrames - 1 : oldCurrentIndex - 1;
                int startSearchIndex = (lastWriteIndex - frameCount + maxFrames) % maxFrames;


                qDebug()<<"lastWriteIndex"<<lastWriteIndex ;
                qDebug()<<"startSearchIndex"<<startSearchIndex ;
                qDebug()<<"oldCurrentIndex"<<oldCurrentIndex ;
                int idx = startSearchIndex;
                int searchCount = 0;
                int iFrameIndex = -1;
                // 找出最靠近的 I-frame（type == 1）
                while (searchCount < maxFrames) {
                    if (pair.lockedBuffer[idx].type == 1) {
                        iFrameIndex = idx;
                        break;
                    }
                    idx = (idx == 0) ? maxFrames - 1 : idx - 1;
                    ++searchCount;
                }

                QList<FrameItem> captured;

                // 若找到 I-frame，則從該點開始擷取最多 600 幀
                if (iFrameIndex != -1) {
                    int idx = iFrameIndex;
                    int capturedCount = 0;

                    while (capturedCount < frameCount) {
                        captured.append(pair.lockedBuffer[idx]);
                        idx = (idx + 1) % maxFrames;
                        ++capturedCount;
                        if (idx == iFrameIndex)  // 防止環狀無限迴圈
                            break;
                    }
                    printOrderedTypesSimple();
                    // 印出擷取的 frame type 資訊
                    QString result;
                    for (const FrameItem& item : captured)
                        result += "[" + QString::number(item.frm_no) + "]";

                    qDebug() << "高音量觸發，IP:" << it.key()
                             << "最近 I-frame index:" << iFrameIndex
                             << "，擷取筆數:" << captured.size();
                    qDebug() << result;

                    QString ip = it.key();
                    QList<FrameItem> capturedFrames = captured;

                    // 啟動新的執行緒進行解碼
                    std::thread decodeThread([this, ip, capturedFrames]() {
                        this->decodeCapturedFrames(ip, capturedFrames);
                    });
                    decodeThread.detach();
                    startcompose = true;

                } else {
                    qDebug() << "高音量觸發，IP:" << it.key() << "找不到 I-frame，略過擷取";
                }
            }
        }
    }).detach();  // 背景執行 thread，不阻塞主流程
}

int FrameBuffer::findBackwardIFrameStartIndex(const QVector<FrameItem>& buffer, int lastWriteIndex, int frameCount) {
    int maxFrames = buffer.size();
    if (maxFrames == 0) return -1;

    // 1️⃣ 起始索引：往前 frameCount 幀
    int startIndex = (lastWriteIndex - frameCount + maxFrames) % maxFrames;

    // 2️⃣ 如果起點是 I-frame，就直接回傳
    if (buffer[startIndex].type == 1)
        return startIndex;

    // 3️⃣ 否則往前搜尋最近 I-frame
    int searchIndex = startIndex;
    int searched = 0;

    while (searched < maxFrames) {
        if (buffer[searchIndex].type == 1)
            return searchIndex;

        searchIndex = (searchIndex == 0) ? maxFrames - 1 : searchIndex - 1;
        ++searched;
    }

    return -1;  // 沒找到
}

QList<FrameItem> FrameBuffer::getCapturedFrames() const
{
    QMutexLocker locker(&mutex);
    return capturedFrames;
}

void FrameBuffer::printOrderedTypesSimple() const
{
    for (const QString& ip : ipBuffers.keys()) {
        const FramePair& pair = ipBuffers[ip];

        // 顯示 activeBuffer
        QString activeStr = "IP: " + ip + " Active: ";
        for (const FrameItem& item : pair.activeBuffer) {
            activeStr += "[" + QString::number(item.frm_no) + "]";
        }

        // 顯示 lockedBuffer
        QString lockedStr = "IP: " + ip + " Locked: ";
        for (const FrameItem& item : pair.lockedBuffer) {
            lockedStr += "[" + QString::number(item.frm_no) + "]";
        }

        qDebug().noquote() << activeStr;
        qDebug().noquote() << lockedStr;
    }
}

void FrameBuffer::init(const char* ip)
{
    // 讀取設定
    QJsonObject readConfig = JsonFileManager::readConfig();
    if (readConfig.contains("buffer") && readConfig["buffer"].isObject()) {
        bufferObj = readConfig["buffer"].toObject();
    } else {
        qWarning() << "[buffer] object not found in config.";
    }

    waitSec = bufferObj["waitSec"].toInt();
    frameCount = bufferObj["frameCount"].toInt();
    framesPerIP = bufferObj["framesPerIP"].toInt();
    videoFrame = bufferObj["videoFrame"].toInt();

    qDebug()<<"waitSec:"<<waitSec;
    qDebug()<<"frameCount:"<<frameCount;
    qDebug()<<"framesPerIP:"<<framesPerIP;

    isDetecting = true;
    auto cam = std::make_shared<Cam_data2>();
    cam->dc = new Decoder_class();
    cam->dc->create(dc_par);
    // cam->dc = new 解碼器實例（你自訂的 Decoder 類別）
    registerDecoder(ip, cam);

    if(!FirstInit)
    {
        FirstInit = true;
        qDebug()<<"initttttttttt";
        std::thread composeThread([this]() {
            while (true) {
                this->checkAndComposeFrames();
            }
        });
        composeThread.detach();
        startcompose = false;
    }
}

float FrameBuffer::returnDB()
{
    return db.db;
}

void FrameBuffer::registerDecoder(const QString& ip, std::shared_ptr<Cam_data2> cam)
{
    QMutexLocker locker(&mutex);
    camMap[ip] = cam;
}

