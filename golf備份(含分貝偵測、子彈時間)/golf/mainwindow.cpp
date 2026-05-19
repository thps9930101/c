#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "apiclient.h"
#include "jsonfilemanager.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    ui->pushButton->setVisible(false);

    ao_cameram_fun::notifyFunc = [this](const QString& ip, const QString& type) {
        QMetaObject::invokeMethod(this, [this, ip, type]() {
            qDebug() << "Camera event:" << type << "from" << ip;
            isStream = false;
            showOverlayText("相機未知斷線",48,Qt::red);
        }, Qt::QueuedConnection);
    };

    InitUI();
    showLoading(true, "系統初始化中");

//    DecoderAndShow("C:/Users/user/Desktop/FreeviewLive-win32-x64/www/video/2886729730.mp4");

    monitoredPath = "C:/Users/user/Desktop/FreeviewLive-win32-x64/www/video";  // 要監控的資料夾
    startMonitoring();

    settingDialog = new setting(this);
    settingDialog->get_encode_param();
    // 讀取設定
    settingJson = JsonFileManager::readConfig();

    QJsonArray ipArray = settingJson["IP"].toArray();

    QString targetIP = ipArray[0].toString();

    apiClient = new ApiClient(this);

    QJsonObject data;

    QUrl url("http://" + targetIP + "/ctrl/info");

    // 發送請求
    apiClient->postJson(url, data, "info");

    // 接收回傳結果
    Cam_Fun_init();

    initConnect();

    setupPressEvent();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitUI()
{
    // 基本畫面分段初始化
    setupGraphicsView();
    setupCornerButton();
    setupProgressSlider();
    setupPlayPauseButton();
    adjustUIPositions();
    createCountdownOverlay();

    setRefreshIcon(ui->re_bt);

    setVideoTool(false);

    ui->listWidget->setEnabled(false);
    ui->live_bt->setEnabled(false);
    ui->re_bt->setEnabled(false);

    LiveTimer = new QTimer(this);
    VideoTimer = new QTimer(this);

}

void MainWindow::setupGraphicsView()
{
    ui->graphicsView->setViewport(new QOpenGLWidget(ui->graphicsView));
    ui->graphicsView->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    ui->graphicsView->viewport()->setStyleSheet("background: transparent;");
    this->statusBar()->hide();
    showFullScreen();

    scene = new QGraphicsScene(this);
    imageItem = new QGraphicsPixmapItem();
    scene->addItem(imageItem);
    ui->graphicsView->setScene(scene);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing, false);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform, false);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); // 需要平滑再開

    ui->graphicsView->viewport()->installEventFilter(this);
}

void MainWindow::initConnect()
{
    connect(apiClient, &ApiClient::requestSucceededJson, this, [this](const QString &api, const QJsonObject &response){

        apiResController(api, true, response);

        qDebug() << "[API Success]" << response;
    });

    connect(apiClient, &ApiClient::requestFailed, this, [this](const QString &api, const QString &error){
        qDebug() << "[API Error]" << error<<"  API:"<<api;
        apiResController(api, false);
    });

    connect(apiClient, &ApiClient::requestSucceededJson, this, [](const QString &api, const QJsonObject &json){
        qDebug() << "[Parsed JSON] Manufacturer:" << json["manufacturer"].toString();
        qDebug() << "IP:" << json["IP"].toArray().first().toString();
        qDebug() << "Lens ID:" << json["lensID"].toArray().first().toString();
    });

}


bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{       
    if (obj == ui->graphicsView->viewport()) {
        int viewHeight = ui->graphicsView->viewport()->height();
        int sectionHeight = viewHeight / 5; // 分成 5 等份

        QVector<double> speedSteps = {0.125, 0.25, 0.5, 1.0};

        static QPoint pressPos;
        static qint64 pressTime = 0; // 記錄點擊時間

        if (event->type() == QEvent::MouseButtonPress) {
            if (!isVideo) {
//                qDebug() << "==============isVideo================";
                return true;
            }
            if(isLoading)
            {
//                qDebug() << "==============isLoading================";
                return true;
            }

            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                isDragging = true;
                lastMousePos = mouseEvent->pos();
                accumulatedMove = 0;
                dragMode = None;  // 開始時不確定方向

                // 記錄點擊資訊
                pressPos = mouseEvent->pos();
                pressTime = QDateTime::currentMSecsSinceEpoch();
            }
            return true;
        }
        else if (event->type() == QEvent::MouseMove) {
            if (!isVideo) {
//                qDebug() << "==============isVideo================";
                return true;
            }

            if(isLoading)
            {
//                qDebug() << "==============isLoading================";
                return true;
            }

            if (!isDragging) return false;

            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint delta = mouseEvent->pos() - pressPos;

            // 判斷拖曳模式（只判斷一次）
            if (dragMode == None) {
                if (abs(delta.y()) > 10 && abs(delta.y()) > abs(delta.x())) {
                    dragMode = Vertical;
                    qDebug() << "👉 垂直模式（調整速度）";
                } else if (abs(delta.x()) > 10 && abs(delta.x()) > abs(delta.y())) {
                    dragMode = Horizontal;
                    qDebug() << "👉 水平模式（調整進度）";
                }
            }

            if (dragMode == Vertical) {
                int deltaY = mouseEvent->pos().y() - lastMousePos.y();
                accumulatedMove += deltaY;

                int idx = speedSteps.indexOf(playbackSpeed);
                if (idx == -1) idx = 2; // 預設回到 1.0x

                while (accumulatedMove >= sectionHeight) {
                    if (idx > 0) {
                        idx--;
                        playbackSpeed = speedSteps[idx];
                        if (isPlaying)
                            VideoTimer->start(updateTimerInterval());
                        else
                            updateTimerInterval();

                        qDebug() << "↓ 降速，當前倍速:" << playbackSpeed;
                        showSpeedOverlay(playbackSpeed);
                    }
                    accumulatedMove -= sectionHeight;
                }
                while (accumulatedMove <= -sectionHeight) {
                    if (idx < speedSteps.size() - 1) {
                        idx++;
                        playbackSpeed = speedSteps[idx];
                        if (isPlaying)
                            VideoTimer->start(updateTimerInterval());
                        else
                            updateTimerInterval();

                        qDebug() << "↑ 加速，當前倍速:" << playbackSpeed;
                        showSpeedOverlay(playbackSpeed);
                    }
                    accumulatedMove += sectionHeight;
                }
            }
            else if (dragMode == Horizontal) {
            int deltaX = mouseEvent->pos().x() - lastMousePos.x();
            accumulatedMove += deltaX;

            if (progressSlider && frameBuffer.size() > 0) {           
                int newValue = progressSlider->value() + accumulatedMove / 3; // 每3px移動一格
                newValue = qBound(0, newValue, frameBuffer.size() - 1);

                VideoTimer->stop();
                isPlaying = false;
                playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

                // 更新畫面
                QImage img;
                {
                    QMutexLocker locker(&bufferMutex);
                    img = frameBuffer[newValue];
                }

                if (!img.isNull()) {
                    QPixmap pix = QPixmap::fromImage(img);
                    imageItem->setPixmap(pix);
                    imageItem->setPos(0, 0);
                    rotateAndFitImage(imageItem, scene);
                }

                progressSlider->setValue(newValue);
                currentFrameIndex = newValue;

                // === 顯示時間浮窗 ===
                showSeekOverlay(true, newValue, 60);

                qDebug() << "➡️ 調整進度到：" << newValue;
            }
            accumulatedMove = 0; // 防止累積過多
        }

        lastMousePos = mouseEvent->pos();
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            isDragging = false;
            accumulatedMove = 0;
            dragMode = None; // Reset 模式

            // 👉 放開時隱藏浮窗
            showSeekOverlay(false, 0, 60);
//            hideSpeedOverlay();

            // 判斷是否為輕點
            int dist = (mouseEvent->pos() - pressPos).manhattanLength();
            qint64 releaseTime = QDateTime::currentMSecsSinceEpoch();
            qint64 duration = releaseTime - pressTime;

            if (dist < 5 && duration < 300) {
                qDebug() << "輕點觸發 play/pause";
                if (playPauseButton) {
                    playPauseButton->click();
                }
            }
        }
        return true;
    }
}
return QMainWindow::eventFilter(obj, event);
}

int MainWindow::updateTimerInterval() {
    double fps = 60.0; // 建議改成從影片資訊取得
    int interval = static_cast<int>((1000.0 / fps) / playbackSpeed);

    if (interval < 1) interval = 1; // 避免小於 1ms

    qDebug() << "更新 tmpFPS:" << interval << "ms (速度:" << playbackSpeed << "x)";
    return interval;
}

// 外面可以加一個 helper（放在 cpp 上方或匿名 namespace 裡）
static QIcon makeHamburgerIcon(int size = 24, const QColor &color = Qt::black) {
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(color);
    pen.setWidth(2);
    p.setPen(pen);

    int margin = size / 6;
    int spacing = (size - 2 * margin) / 4; // 3 lines => 4 gaps
    int y1 = margin;
    int y2 = margin + spacing + 2;
    int y3 = margin + 2 * (spacing + 2);

    int lineLen = size - 2 * margin;
    int x = margin;

    p.drawLine(x, y1, x + lineLen, y1);
    p.drawLine(x, y2, x + lineLen, y2);
    p.drawLine(x, y3, x + lineLen, y3);
    p.end();

    return QIcon(pix);
}

void MainWindow::setupCornerButton()
{
    cornerButton = new QPushButton(ui->graphicsView->viewport());
    cornerButton->setFixedSize(80, 80);  // ⬆ 再放大
    cornerButton->move(ui->graphicsView->width() - cornerButton->width() - 16, 16);
    cornerButton->raise();
    cornerButton->show();
    cornerButton->setStyleSheet("background-color: rgba(255,255,255,80); border-radius: 5px;");
    cornerButton->setToolTip("Menu");

    // 設漢堡圖示（用深色方便在淡背景看得見）
    cornerButton->setIcon(makeHamburgerIcon(48, Qt::black));
    cornerButton->setIconSize(QSize(48, 48));

    connect(cornerButton, &QPushButton::clicked, this, [this]() {
        ui->re_bt->setVisible(!ui->re_bt->isVisible());
        ui->live_bt->setVisible(!ui->live_bt->isVisible());
        ui->listWidget->setVisible(!ui->listWidget->isVisible());
        QTimer::singleShot(0, this, [this]() {
            if (imageItem && !imageItem->pixmap().isNull()) {
//                scene->setSceneRect(imageItem->boundingRect());
//                ui->graphicsView->fitInView(imageItem->boundingRect(), Qt::KeepAspectRatio);
//                ui->graphicsView->viewport()->update();
                rotateAndFitImage(imageItem, scene);
            }
            if (cornerButton) {
                int x = ui->graphicsView->viewport()->width() - cornerButton->width() - 8;
                int y = 8;
                cornerButton->move(x, y);
            }
            adjustUIPositions();
        });
        qDebug() << "[UI] Button clicked!";
    });
}

void MainWindow::setupPlayPauseButton()
{
    playPauseButton = new QPushButton(this);
    playPauseButton->setFixedSize(64, 64); // ⬆ 放大
    playPauseButton->setFlat(true);
    playPauseButton->setFocusPolicy(Qt::NoFocus);
    playPauseButton->setStyleSheet(R"(
        QPushButton {
            background: rgba(150,150,150,150);
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: rgba(180,180,180,250);
        }
    )");

    // 初始為播放中，因此顯示暫停圖示
    isPlaying = true;
    auto setIconForState = [this]() {
        if (isPlaying) {
            playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        } else {
            playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        }
        playPauseButton->setIconSize(QSize(36, 36)); // ⬆ 更大圖示
    };
    setIconForState();

    playPauseButton->raise();
    playPauseButton->show();

    connect(playPauseButton, &QPushButton::clicked, this, [this, setIconForState]() mutable {
        if(frameBuffer.isEmpty())
        {
            return ;
        }
        if (isPlaying) {
//            playThread->pause();
            VideoTimer->stop();
            isPlaying = false;
        } else {
            VideoTimer->start(updateTimerInterval());

//            playThread->resume();
            isPlaying = true;
        }
        setIconForState();
    });
}

void MainWindow::setupProgressSlider()
{
    progressSlider = new ClickableSlider(Qt::Horizontal, ui->graphicsView->viewport());
    progressSlider->installEventFilter(this);
    progressSlider->setRange(0, 100);
    progressSlider->setFixedHeight(80); // ⬆ 高度加大
    if (progressSlider) {
        int x = 0;
        int y = ui->graphicsView->viewport()->height() - progressSlider->height() - 10;
        int w = ui->graphicsView->viewport()->width();
        progressSlider->setGeometry(x, y, w, progressSlider->height());
    }
    progressSlider->show();

    progressSlider->setStyleSheet(R"(
           QSlider::groove:horizontal {
               background: gray;
               height: 20px;  /* ⬆ 加粗 */
               border-radius: 7px;
           }
           QSlider::handle:horizontal {
               background: green;
               width: 40px;   /* ⬆ 滑塊更大 */
               height: 40px;
               margin: -8px 0;
               border-radius: 14px;
               border: 2px solid #555;
           }
           QSlider::handle:horizontal:pressed {
               background: darkgreen;
               border: 2px solid #222;
           }
       )");

    progressSlider->raise();
    progressSlider->show();

    // local helper to sync play/pause icon
    auto updatePlayPauseIcon = [this]() {
        if (!playPauseButton) return;
        if (isPlaying) {
            playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        } else {
            playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        }
        playPauseButton->setIconSize(QSize(36, 36));
    };

    connect(progressSlider, &ClickableSlider::sliderPressed, this, [this, updatePlayPauseIcon]() mutable {
        qDebug() << "======(pressed)=======";
        wasPlayingBeforeDrag = isPlaying;
//        playThread->pause();
        VideoTimer->stop();
        isPlaying = false;
        updatePlayPauseIcon();
    });

    connect(progressSlider, &ClickableSlider::sliderMoved, this, [this](int value){
        QImage img;
        {
            QMutexLocker locker(&bufferMutex);
            if (frameBuffer.isEmpty()) return;
            // bounds check
            if (value < 0 || value >= frameBuffer.size())
            {
                return;
            }
            img = frameBuffer[value];
        }

        if (img.isNull()) return;  // 保險

        qDebug() << "👆 拖曳到：" << value;

        QPixmap pix = QPixmap::fromImage(img);
        imageItem->setPixmap(pix);
        imageItem->setPos(0, 0);
        rotateAndFitImage(imageItem, scene);

        // 只有當尺寸變了才重新 fitInView（避免每次拖動都重置比例導致閃爍/消失）
        static QSizeF lastSize;
        QSizeF pixSize = pix.size();
        if (lastSize != pixSize) {

//            scene->setSceneRect(0, 0, pixSize.width(), pixSize.height());
//            ui->graphicsView->fitInView(imageItem->boundingRect(), Qt::KeepAspectRatio);
            lastSize = pixSize;
        }
    });

    connect(progressSlider, &ClickableSlider::sliderReleased, this, [this, updatePlayPauseIcon]() mutable {
        int value = progressSlider->value();
        int target = -1;
        {
            QMutexLocker locker(&bufferMutex);
            if (frameBuffer.isEmpty()) return;
            int lastBuffered = frameBuffer.size() - 1;
            target = qMin(value, lastBuffered);
        }

        if (target != value) {
            progressSlider->blockSignals(true);
            progressSlider->setValue(target);
            progressSlider->blockSignals(false);
        }

        qDebug() << "▶️ 播放 (released) 到：" << target;

        QImage img;
        {
            QMutexLocker locker(&bufferMutex);
            img = frameBuffer[target];
        }
        if (img.isNull()) return;

        QPixmap pix = QPixmap::fromImage(img);
        imageItem->setPixmap(pix);
        imageItem->setPos(0, 0);

        // 正確設定 scene 大小並縮放
//        scene->setSceneRect(0, 0, pix.width(), pix.height());
//        ui->graphicsView->fitInView(imageItem->boundingRect(), Qt::KeepAspectRatio);
        rotateAndFitImage(imageItem, scene);

        currentFrameIndex = target;

        if (wasPlayingBeforeDrag) {
//            playThread->resume();
            VideoTimer->start(updateTimerInterval());
            isPlaying = true;
        } else {
            isPlaying = false;
        }
        updatePlayPauseIcon();
    });
}


void MainWindow::adjustUIPositions() {
    // 取 viewport 的 geometry（相對於 MainWindow）
    QRect vpGeom = ui->graphicsView->viewport()->geometry();
    QPoint vpTopLeft = ui->graphicsView->viewport()->mapTo(this, QPoint(0,0));

    // 按鈕：左下角
    int btnMargin = 8;
    int btnX = vpTopLeft.x() + btnMargin;
    int btnY = vpTopLeft.y() + vpGeom.height() - playPauseButton->height() - btnMargin;
    playPauseButton->move(btnX, btnY);

    // Slider：從按鈕右邊開始，留間距，不蓋到按鈕，靠 bottom 對齊
    int spacing = 12;
    int sliderX = btnX + playPauseButton->width() + spacing;
    int sliderY = vpTopLeft.y() + vpGeom.height() - progressSlider->height() - btnMargin;

    int availableW = vpGeom.width() - (sliderX - vpTopLeft.x()) - btnMargin;
    if (availableW < 0) availableW = 0;
    progressSlider->setFixedWidth(availableW);
    progressSlider->move(sliderX, sliderY);

    if (loadingLabel) {
        QWidget* vp = ui->graphicsView->viewport();
        if (loadingLabel->parentWidget() != vp) {
            loadingLabel->setParent(vp);      // 萬一你有換過 viewport
        }
        loadingLabel->setGeometry(vp->rect()); // 直接覆蓋整個 viewport
        loadingLabel->raise();                 // 防止被其他覆蓋控件蓋住
    }
}

bool MainWindow::DecoderAndShow(QString path)
{
    // 停止舊解碼器（若存在）
    if (decodeThread && decodeThread->isRunning()) {
        decodeThread->requestInterruption();
        decodeThread->quit();
        decodeThread->wait();
//        delete decodeThread;
        decodeThread = nullptr;
    }

    // 刪除舊 decoder（釋放 ffmpeg 資源）
    if (decoder) {
//        delete decoder;
        decoder = nullptr;
    }

    if(VideoTimer->isActive())
    {
        VideoTimer->stop();
        qDebug()<<"[video stop!!!!!!!!]";
    }
    decoder = nullptr;
    decodeThread = nullptr;

    frameBuffer.clear();
    currentFrameIndex = 0;
    decodingFinished = false;

    // thread 解碼
    // 啟動解碼執行緒
    decoder = new VideoDecoder();
    bool bl_openfile = decoder->openFile(path);
    if(!bl_openfile)
    {
        return false;
    }
    qDebug()<<bl_openfile;
    FrameData = decoder->getFrameData();

    progressSlider->setRange(0, FrameData.FrameCount-1);  // 設定區間為 100～500

    int num = 1000 / FrameData.fps;
    connect(VideoTimer, &QTimer::timeout, this, &MainWindow::showNextFrame, Qt::UniqueConnection);
    showLoading(true,"影片載入中...");
    isLoading = true;

    // 延遲 1 秒再啟動播放 timer（buffer 已開始累積）
    QTimer::singleShot(4000, this, [this]() {
        showLoading(false,"影片載入中...");
        isLoading = false;

        VideoTimer->start(updateTimerInterval());
        decodingFinished = true;
        ui->listWidget->setEnabled(true);
    });

    // 先啟動解碼 thread（馬上填 buffer）
    decodeThread = QThread::create([this, path]() {
        while (true) {
            QImage img = decoder->getNextFrame();
//            if (!img.isNull()) {
//                img = img.convertToFormat(QImage::Format_RGBA8888)
//                                 .scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//                // out 就是 1920x1080
//            }
            if (img.isNull()) break;

            {
                QMutexLocker locker(&bufferMutex);
                frameBuffer.append(img);
                frameReady.wakeAll();
            }
        }
        decoder->release();
        {
            QMutexLocker locker(&bufferMutex);
            decodingFinished = true;
        }
    });
    decodeThread->start();

    return true;
}

//按鍵事件初始化
void MainWindow::setupPressEvent()
{
    // 安裝事件處理器
    eventHandler = new PressEvent(this);

    connect(eventHandler, &PressEvent::clickedOutside, this, [this]() {
        qDebug() << "[MainWindow] Clicked outside MainWindow!";
    });

    connect(eventHandler, &PressEvent::keyPressed, this, [this](int key) {
        qDebug() << "[MainWindow] Key pressed:" << key;
    });

    connect(eventHandler, &PressEvent::keyTextPressed, this, [this](const QString &text) {
        qDebug() << "[MainWindow] Key text:" << text;
    });

    connect(eventHandler, &PressEvent::keyCombinationPressed, this, [this](Qt::KeyboardModifiers mods, int key) {
        if ((mods & Qt::ControlModifier) && key == Qt::Key_O) {
            settingDialog->show(); // 顯示設定視窗
            settingDialog->initCam(settingJson);
        }
    });
    connect(eventHandler, &PressEvent::keyCombinationPressed, this, [this](Qt::KeyboardModifiers mods, int key) {
        if ((mods & Qt::ControlModifier) && key == Qt::Key_R) {
            rotateAndFitImage(imageItem, scene);
        }
    });
    connect(eventHandler, &PressEvent::keyCombinationPressed, this, [this](Qt::KeyboardModifiers mods, int key) {
        if (key == Qt::Key_F9) {
            if(isLoading)
            {
                return;
            }
            if(!isStream)
            {
                showOverlayText("相機斷線，無法錄影",48,Qt::red);
                qDebug()<<"not live!";
                return;
            }
            m_viewRotated = false;
            startCountdown(3,10);

//            QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
//            QString path = "C:/Users/user/Desktop/FreeviewLive-win32-x64/www/video/" + timeStr + ".mp4";

//            if(isStream)
//            {
//                if(!isRecord)
//                {
//                    qDebug()<<"startRec";
//                    Cam_Fun.open_file(path.toStdString());

//                    isRecord = true;

//                    // 15 秒後自動停止
//                       QTimer::singleShot(15000, this, [this]() {
//                           if (isRecord) {
//                               qDebug() << "stopRec (auto after 15s)";
//                               Cam_Fun.close_file();
//                               isRecord = false;
//                           }
//                       });
//                }else
//                {
//                    qDebug()<<"stopRec";
//                    Cam_Fun.close_file();
//                    isRecord = false;
//                }
//            }
        }
    });
}

// showNextFrame 實作
void MainWindow::showNextFrame() {
    QElapsedTimer t_total; t_total.start();

    if (!decodingFinished || frameBuffer.isEmpty()) return;
    if (currentFrameIndex >= FrameData.FrameCount) { VideoTimer->stop(); return; }
    if (currentFrameIndex >= frameBuffer.size())    return;

    const QImage& img = frameBuffer[currentFrameIndex];

    QElapsedTimer t_pix; t_pix.start();
    QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);  // 減少格式轉換
    const double ms_pix = t_pix.nsecsElapsed() / 1e6;

    QElapsedTimer t_set; t_set.start();
    imageItem->setPixmap(pix);
    const double ms_set = t_set.nsecsElapsed() / 1e6;

    double ms_fit = -1.0;
    if (!m_viewRotated) {
        QElapsedTimer t_fit; t_fit.start();
        rotateAndFitImage(imageItem, scene);
        ms_fit = t_fit.nsecsElapsed() / 1e6;
        m_viewRotated = true;
    }
    rotateAndFitImage(imageItem, scene);

    progressSlider->setValue(currentFrameIndex);
    currentFrameIndex++;

    const double ms_total = t_total.nsecsElapsed() / 1e6;

    // 輸出到 debug
    qDebug().noquote() << QString("[Perf] frame %1 | total %2 ms | fromImage %3 ms | setPixmap %4 ms%5")
                          .arg(currentFrameIndex-1)
                          .arg(ms_total, 0, 'f', 3)
                          .arg(ms_pix,   0, 'f', 3)
                          .arg(ms_set,   0, 'f', 3)
                          .arg(ms_fit >= 0 ? QString(" | rotate+fit %1 ms").arg(ms_fit, 0, 'f', 3) : QString());
}

//void MainWindow::showNextFrame() {
//    QMutexLocker locker(&bufferMutex);
//    if (!decodingFinished || frameBuffer.isEmpty()) {
//        return;
//    }
//    ui->listWidget->setEnabled(true);

//    if (currentFrameIndex >= FrameData.FrameCount) {
//        stopPlayback();  // 停止播放執行緒
//        qDebug() << "[Player] Playback finished.";
//        return;
//    }
//    if (currentFrameIndex >= frameBuffer.size()) {
//        qDebug() << "frameBuffer" << frameBuffer.size();
//        return;
//    }

//    const QImage& img = frameBuffer[currentFrameIndex];
//    QPixmap pix = QPixmap::fromImage(img);

//    imageItem->setPixmap(pix);
//    imageItem->setTransformOriginPoint(pix.width() / 2.0, pix.height() / 2.0);
//    imageItem->setPos(0, 0);
//    scene->setSceneRect(0, 0, pix.width(), pix.height());

//    if (currentFrameIndex == 0) {
//        ui->graphicsView->resetTransform();
//        ui->graphicsView->fitInView(imageItem, Qt::KeepAspectRatio);
//    }

//    progressSlider->setValue(currentFrameIndex);
//    currentFrameIndex++;
//}

void MainWindow::startMonitoring() {

    if (monitorThread) {
        qWarning() << "[Monitor] Already running!";
        return;
    }

    monitorThread = QThread::create([this]() {
        QStringList previousList;
        bool newRecFile = false;
        QString newRecName = "";

        while (!QThread::currentThread()->isInterruptionRequested()) {
            QDir dir(monitoredPath);
            dir.setSorting(QDir::Time);

            QStringList filters = {"*.mp4", "*.avi", "*.mov"};
            QStringList files = dir.entryList(filters, QDir::Files);
//            qWarning() << "[Monitor]:"<<files;

            if (files != previousList) {
                if (files.size() == previousList.size() + 1)
                {
                    // 找出新增的檔案
                    QString newFile;
                    for (const QString &f : files) {
                        if (!previousList.contains(f)) {
                            newFile = f;
                            break;
                        }
                    }

                    if (!newFile.isEmpty()) {
                        QFileInfo info(newFile);
                        QString baseName = info.completeBaseName(); // 去掉副檔名

                        // 跟 timeStr 比對
                        if (baseName == timeStr) {
                            newRecFile = true;
                            newRecName = baseName;
                        }
                    }
                }

                previousList = files;

                // 去除副檔名
                QStringList fileNamesNoExt;
                for (const QString& f : files) {
                    QFileInfo info(f);
                    fileNamesNoExt << info.completeBaseName();  // 去掉副檔名
                }

                // 切回 UI thread 更新列表
                QMetaObject::invokeMethod(this, [this, fileNamesNoExt]() {
                    updateVideoList(fileNamesNoExt);  // 傳的是不含副檔名的清單
                }, Qt::QueuedConnection);
            }

            if(newRecFile)
            {
                QMetaObject::invokeMethod(this, [this, newRecName]() {
                    // 在 listWidget 裡找到該項目
                    QList<QListWidgetItem *> items = ui->listWidget->findItems(newRecName, Qt::MatchExactly);
                    if (!items.isEmpty()) {
                        ui->listWidget->setCurrentItem(items.first());
                        emit ui->listWidget->itemClicked(items.first());
                        qDebug() << "[Monitor] Auto-selected new file:" << newRecName;
                    }
                }, Qt::QueuedConnection);
                newRecFile = false;
            }
            QThread::sleep(3);
        }

        qDebug() << "[Monitor] Stopped.";
    });

    monitorThread->start();
//    qDebug() << "[Monitor] Started watching:" << monitoredPath;
}

void MainWindow::updateVideoList(const QStringList& videos) {
    ui->listWidget->clear();
    ui->listWidget->addItems(videos);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    if (!item)
    {
        qDebug()<<"item:"<<item;
        return;
    }

    QString fileName = item->text();
    QString fullPath = monitoredPath + "/" + fileName + ".mp4";

    if (fullPath == currentVideoPath || openingVideo) {
        qDebug() << "[UI] Same video clicked or opening in progress, ignored:" << fullPath;
        return;
    }

    openingVideo = true;                // 設旗標，避免重入
    ui->listWidget->setEnabled(false);  //（可選）載入時先鎖住列表

    imageItem->setPixmap(QPixmap());  // 設成空 pixmap

    if(DecoderAndShow(fullPath))
    {
        ui->listWidget->setEnabled(false);

        setVideoTool(true);

        currentVideoPath = fullPath;
        qDebug() << "[UI] Selected video:" << fullPath;

        LiveTimer->stop();

        currentFrameIndex = 0;
        frameBuffer.clear();
        decodingFinished = false;
//        isStream = false
        isVideo = true;
    }
    else
    {
        setVideoTool(false);
        VideoTimer->stop();
        isVideo = false;
        decoder->stop();
        progressSlider->setValue(0);
        ui->listWidget->clearSelection();   // 取消選取
        showOverlayText("影像錯誤，無法讀取",48,Qt::red);
    }
    openingVideo = false;
}

// cam_fun
void MainWindow::Cam_Fun_init()
{
    Cam_Fun.init(settingDialog->get_decode_param(),settingDialog->get_encode_param());
}

void MainWindow::setCamList(QList<QString> IPs)
{
    SPID_list.clear();
    camList.clear();
    Cam_Fun.DeleteAllCam();
    for(int i = 0; i <= IPs.size()-1; i++)
    {
        CamInfo tmpInfo;
        std::string* ipAddressStdString = new std::string(IPs[i].toStdString());
        tmpInfo.id = Cam_Fun.AddCam(ipAddressStdString->c_str());
        tmpInfo.ip = IPs[i];
        qDebug()<<"tmpInfo.ip:"<<tmpInfo.ip;
        qDebug()<<"tmpInfo.id:"<<tmpInfo.id;

        camList.append(tmpInfo);
    }

    QList<QString> ipList;

    if (settingJson.contains("camList") && settingJson["camList"].isArray()) {
        QJsonArray ipArray = settingJson["camList"].toArray();
        for (const QJsonValue &val : ipArray) {
            ipList.append(val.toString());
        }
    }

    bool found = false;

    // 判斷並取出 id
    for (const QString &ip : ipList) {
        for (const CamInfo &cam : camList) {
            if (cam.ip == ip) {
                qDebug() << "IP:" << ip << " 對應的 ID:" << cam.id;
                SPID_list.push_back(cam.id);
            }
        }
    }

    show_mode = Show_Mode::Show_splice;
    SP_mode = SP_MODE::sp_4k_2x2;
    Cam_Fun.set_sp_Mode(SP_mode);
    Cam_Fun.set_view_sp_num(SPID_list);
    Cam_Fun.set_Show_Mode(show_mode);

    std::vector<SP_Rot> rot_id;

    QJsonArray rotArray = settingJson["rotList"].toArray();
    qDebug()<<"array:"<<rotArray[0].toInt();
    qDebug()<<"array:"<<rotArray[1].toInt();
    qDebug()<<"array:"<<rotArray[2].toInt();
    qDebug()<<"array:"<<rotArray[3].toInt();

    rot_id.push_back(static_cast<SP_Rot>(rotArray[0].toInt()));
    rot_id.push_back(static_cast<SP_Rot>(rotArray[1].toInt()));
    rot_id.push_back(static_cast<SP_Rot>(rotArray[2].toInt()));
    rot_id.push_back(static_cast<SP_Rot>(rotArray[3].toInt()));
    Cam_Fun.set_view_sp_rot(rot_id);

}

void MainWindow::startCamList()
{
    Cam_Fun.stop_all();
    Cam_Fun.start_all();
    ui->live_bt->setEnabled(true);
    on_live_bt_clicked();
    isStream = true;
}

void MainWindow::on_live_bt_clicked()
{
    if (VideoTimer && VideoTimer->isActive()) {
        qDebug() << "Timer stopped.";
        VideoTimer->stop();
        isVideo = false;
        decoder->stop();
        progressSlider->setValue(0);
        ui->listWidget->clearSelection();   // 取消選取
    }
//    if(playThread)
//    {
//        qDebug() << "Timer stopped.";
//        playThread->pause();
//    }
    setVideoTool(false);
    currentVideoPath = "";

    qDebug()<<"Start";
    LiveTimer->stop();
    connect(LiveTimer, &QTimer::timeout, this, &MainWindow::show_live_frame);
    // 延遲 1 秒再啟動播放 timer（buffer 已開始累積）
    QTimer::singleShot(500, this, [this]() {
        qDebug()<<"test1234";
        rotateAndFitImage(imageItem, scene);
        LiveTimer->start(15);
    });
}

void MainWindow::show_live_frame()
{
    const QImage& img = Cam_Fun.get_LiveView();

    // 如果影像是空的就直接 return
    if (img.isNull()) {
        return;
    }

    QPixmap pix = QPixmap::fromImage(img);

    // 更新 imageItem 的內容
    imageItem->setPixmap(pix);

    // 將圖片 item 移到左上角
    imageItem->setPos(0, 0);

    // 更新 scene 大小（必要）
    QSizeF pixSize = pix.size();
    scene->setSceneRect(0, 0, pixSize.width(), pixSize.height());

    // 使 graphicsView 自動縮放以符合圖片大小
//    ui->graphicsView->fitInView(imageItem->boundingRect(), Qt::KeepAspectRatio);
    rotateAndFitImage(imageItem, scene);
}

void MainWindow::setVideoTool(bool visable)
{
    playPauseButton->setVisible(visable);
    progressSlider->setVisible(visable);
}

void MainWindow::rotateAndFitImage(QGraphicsPixmapItem* imageItem, QGraphicsScene* scene)
{
    if (!imageItem || !scene) return;

    // 設定旋轉中心
    imageItem->setTransformOriginPoint(imageItem->boundingRect().center());
    imageItem->setRotation(90);

    // 用 pixmap().size() 取得真實尺寸
    QSizeF pixSize = imageItem->pixmap().size();

    // 旋轉後的 bounding rect
    QRectF rotatedRect = imageItem->mapToScene(QRectF(QPointF(0,0), pixSize)).boundingRect();

    // 將 sceneRect 以中心對齊
    scene->setSceneRect(-rotatedRect.width()/2.0, -rotatedRect.height()/2.0,
                        rotatedRect.width(), rotatedRect.height());

    // 將 imageItem 移到 scene 中心
    imageItem->setPos(-pixSize.width()/2.0, -pixSize.height()/2.0);

    // fitInView + 保證居中
    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    ui->graphicsView->centerOn(0,0);
}

void MainWindow::createCountdownOverlay()
{
    // 半透明黑色遮罩
    overlayMask = new QGraphicsRectItem();
    overlayMask->setBrush(QColor(0, 0, 0, 150));
    overlayMask->setPen(Qt::NoPen);
    overlayMask->setZValue(1000);
    scene->addItem(overlayMask);

    // 倒數 / 完成文字
    countdownText = new QGraphicsTextItem();
    countdownText->setFont(QFont("Arial", 300, QFont::Bold));
    countdownText->setDefaultTextColor(Qt::white);
    countdownText->setZValue(1001);
    scene->addItem(countdownText);

    // 紅色錄影邊框
    recordingBorder = new QGraphicsRectItem();
    recordingBorder->setPen(QPen(Qt::red, 5));
    recordingBorder->setBrush(Qt::NoBrush);
    recordingBorder->setZValue(1002);
    scene->addItem(recordingBorder);

    overlayMask->hide();
    countdownText->hide();
    recordingBorder->hide();
}

void MainWindow::updateOverlayPosition()
{
    QRectF viewRect = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();

    overlayMask->setRect(viewRect);
    recordingBorder->setRect(viewRect.adjusted(7.5, 7.5, -7.5, -7.5)); // 邊框往內縮半筆寬

    QRectF textRect = countdownText->boundingRect();
    countdownText->setPos(
        viewRect.center().x() - textRect.width() / 2,
        viewRect.center().y() - textRect.height() / 2
    );
}

void MainWindow::startCountdown(int seconds, int recordingSeconds)
{
    countdownValue = seconds;
    recordingDuration = recordingSeconds;

    on_live_bt_clicked();
    isLoading = true;

    overlayMask->show();
    countdownText->show();
    recordingBorder->hide();
    countdownText->setPlainText(QString::number(countdownValue));
    updateOverlayPosition();

    if (!countdownTimer) {
        countdownTimer = new QTimer(this);
        connect(countdownTimer, &QTimer::timeout, this, &MainWindow::updateCountdown);
    }
    countdownTimer->start(1000);
}

void MainWindow::updateCountdown()
{
    countdownValue--;
    if (countdownValue > 0) {
        countdownText->setPlainText(QString::number(countdownValue));
    } else {
        countdownTimer->stop();
        overlayMask->hide();
        countdownText->hide();

        // 開始錄影
        startRec();

        // 顯示紅色邊框
        recordingBorder->show();
        updateOverlayPosition();

        // N 秒後結束錄影
        QTimer::singleShot(recordingDuration * 1000, this, [this]() {
            recordingBorder->hide();
            // 顯示 Done
            countdownText->setFont(QFont("Arial", 200, QFont::Bold));

            countdownText->setPlainText("Done");
            countdownText->setDefaultTextColor(Qt::white);
            countdownText->show();
            updateOverlayPosition();
            ui->graphicsView->scene()->setBackgroundBrush(Qt::black);
            QTimer::singleShot(1500, this, [this]() {
                countdownText->hide();
            });
        });
    }
}

void MainWindow::showOverlayText(const QString& text, int fontSize, QColor color)
{
    if (!scene) return;

    // 如果之前已經有 overlayLabel，先刪掉
    if (overlayLabel) {
        overlayLabel->deleteLater();
        overlayLabel = nullptr;
    }

    // 建立 QLabel 疊在 graphicsView 上
    overlayLabel = new QLabel(ui->graphicsView->viewport());
    overlayLabel->setStyleSheet(QString("color: %1; "

                                        "font: bold %2px 'Microsoft JhengHei';")
                                        .arg(color.name())
                                        .arg(fontSize));
    overlayLabel->setAlignment(Qt::AlignCenter);

    // === 設定在畫面「中間靠上」 ===
    QRect viewRect = ui->graphicsView->viewport()->rect();
    int labelWidth  = viewRect.width() * 0.6;   // 寬度 60% 畫面
    int labelHeight = 100;                       // 固定高度
    int x = (viewRect.width() - labelWidth) / 2; // 水平置中
    int y = viewRect.height() * 0.35;            // 垂直 20% 高度（中間偏上）

    overlayLabel->setGeometry(x, y, labelWidth, labelHeight);
    overlayLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 不擋滑鼠
    overlayLabel->setText(text);
    overlayLabel->show();

    // 1.5 秒後自動清除
    QTimer::singleShot(1500, this, [this]() {
        if (overlayLabel) {
            overlayLabel->hide();
            overlayLabel->deleteLater();
            overlayLabel = nullptr;
        }
    });
}

QString formatSpeed(double speed) {
    if (speed == 1.0)   return "1x";
    if (speed == 0.5)   return "0.5x";
    if (speed == 0.25)  return "0.25x";
    if (speed == 0.125) return "0.125x";
    return QString::number(speed) + "x"; // fallback
}

void MainWindow::showSpeedOverlay(double speed)
{
    if (!scene) return;

    // 如果之前已經有文字，先刪掉
    if (speedOverlayText) {
        scene->removeItem(speedOverlayText);
        delete speedOverlayText;
        speedOverlayText = nullptr;
    }

    // 建立新的文字
    speedOverlayText = new QGraphicsTextItem(formatSpeed(speed));
    speedOverlayText->setFont(QFont("Arial", 150, QFont::Bold));
    speedOverlayText->setDefaultTextColor(Qt::white);
    speedOverlayText->setZValue(2000);
    scene->addItem(speedOverlayText);

    // 放到畫面正中間
    QRect viewRect = ui->graphicsView->viewport()->rect();
    QPoint centerPoint = viewRect.center();  // 這裡是 QPoint
    QPointF center = ui->graphicsView->mapToScene(centerPoint);
    QRectF textRect = speedOverlayText->boundingRect();
    speedOverlayText->setPos(center.x() - textRect.width() / 2,
                             center.y() - textRect.height() / 2);

    // 動畫淡入淡出
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    QWidget* textWidget = new QWidget();
    textWidget->setGraphicsEffect(effect);

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(1200); // 總長度 1.2 秒
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.7, 1.0); // 前 70% 保持不透明
    anim->setEndValue(0.0);        // 最後淡出
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 動畫結束時刪除文字
    connect(anim, &QPropertyAnimation::finished, [this]() {
        if (speedOverlayText) {
            scene->removeItem(speedOverlayText);
            delete speedOverlayText;
            speedOverlayText = nullptr;
        }
    });
}

void MainWindow::startRec()
{

    timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    QString path = "C:/Users/user/Desktop/FreeviewLive-win32-x64/www/video/tmp/" + timeStr + ".mp4";
    QString finalPath = "C:/Users/user/Desktop/FreeviewLive-win32-x64/www/video/" + timeStr + ".mp4";

    if(isStream)
    {
        if(!isRecord)
        {
            qDebug()<<"startRec";
            Cam_Fun.open_file(path.toStdString());

            isRecord = true;

            // 10 秒後自動停止
               QTimer::singleShot(10000, this, [this, path, finalPath]() {
                   if (isRecord) {
                       qDebug() << "stopRec (auto after 15s)";
                       Cam_Fun.close_file();
                       isRecord = false;

                       // 延遲 1 秒後移動檔案
                       QTimer::singleShot(1000, this, [path, finalPath]() {
                           if (QFile::exists(path)){
                               if (QFile::rename(path, finalPath)) {
                                   qDebug() << "Moved file to:" << finalPath;
                                   // 🔹 這裡可以呼叫更新 UI 列表
                                   // addVideoToList(finalPath);
                               } else {
                                   qWarning() << "❌ Failed to move file!";
                               }
                           } else {
                               qWarning() << "❌ Temp file not found:" << path;
                           }
                       });
                   }
               });
        }else
        {
            qDebug()<<"stopRec";
            Cam_Fun.close_file();
            isRecord = false;

            // 延遲 1 秒再拉檔案
            QTimer::singleShot(1000, this, [path, finalPath]() {
                if (QFile::exists(path)){
                    if (QFile::rename(path, finalPath)) {
                        qDebug() << "Moved file to:" << finalPath;
                        // 🔹 這裡可以呼叫更新 UI 列表
                        // addVideoToList(finalPath);
                    } else {
                        qWarning() << "❌ Failed to move file!";
                    }
                } else {
                    qWarning() << "❌ Temp file not found:" << path;
                }
            });
        }
    }else
    {
        qDebug()<<"not live";
    }
}

void MainWindow::on_re_bt_clicked()
{
    qDebug()<<"isPlaying:"<<isPlaying;
    if(isPlaying)
    {
        VideoTimer->stop();
        isPlaying = false;
        if (imageItem) {
            imageItem->setPixmap(QPixmap());  // 設成空 pixmap
        }
    }
    Cam_Fun.stop_all();
    QString targetIP = "172.16.0.1"; // 請根據實際 IP 設定

    // 設定 JSON 與網址
    QJsonObject data;

    QUrl url("http://" + targetIP + "/ctrl/info");

    showLoading(true, "相機連線中");

    // 發送請求
    apiClient->postJson(url, data, "info");

    ui->listWidget->setEnabled(true);
    ui->re_bt->setEnabled(true);
    ui->live_bt->setEnabled(true);
}

void MainWindow::setRefreshIcon(QPushButton* btn) {
    if (!btn) return;

    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::black, 2);
    p.setPen(pen);

    // 畫圓弧
    QRectF rect(4, 4, 24, 24);
    p.drawArc(rect, 30 * 16, 300 * 16);

    // 畫箭頭
    QPolygon arrow;
    arrow << QPoint(24, 8) << QPoint(28, 12) << QPoint(20, 12);
    p.setBrush(Qt::black);
    p.drawPolygon(arrow);

    btn->setIcon(QIcon(pix));
    btn->setIconSize(QSize(32, 32));
}

void MainWindow::apiResController(QString apiName, bool success, const QJsonObject& response)
{
    QString targetIP = "172.16.0.1"; // 請根據實際 IP 設定
    qDebug()<<"apiName:"<<apiName;
    if(!success)
    {
        showLoading(false);

        showOverlayText("相機無法連線",48,Qt::red);
        ui->listWidget->setEnabled(true);
        ui->re_bt->setEnabled(true);
        ui->live_bt->setEnabled(false);
        return;
    }
    if(apiName == "info")
    {
        if(!success)
        {
            showLoading(false);

            showOverlayText("相機無法連線",48,Qt::red);
            ui->listWidget->setEnabled(true);
            ui->re_bt->setEnabled(true);
            ui->live_bt->setEnabled(false);
        }else
        {
            if (response.contains("IP") && response["IP"].isArray()) {
                ipList.clear();
                QJsonArray ipArray = response["IP"].toArray();
                for (const QJsonValue &val : ipArray) {
                    ipList.append(val.toString());
                }
                settingJson["IP"] = ipArray;  // ✅ 加入 IP 陣列到 JSON 中
                qDebug() << "[TEST]" << settingJson;

                JsonFileManager::writeConfig(settingJson);
                settingDialog->MicComboboxInit(ipList);



                if (settingJson.contains("image") && settingJson["image"].isObject()) {
                    imageObj = settingJson["image"].toObject();
                    QJsonObject tmpObj;
                    tmpObj["iso"] = 100;
                    showLoading(true, "相機設定中");
                    apiClient->camSet(targetIP, tmpObj);

                    // 呼叫 ApiClient 中的 camSet 函式傳送設定
                    apiClient->camSet(targetIP, imageObj);           
                } else {
                    qWarning() << "[image] object not found in config.";
                }
            }
        }
    }
    else if(apiName == "camSet_wdr")
    {
        apiClient->timeSet(targetIP);
    }
    else if(apiName == "timeset")
    {
        qDebug() << "[ipList]" << ipList;

        showLoading(false);

        setCamList(ipList);
        startCamList();

        ui->listWidget->setEnabled(true);
        ui->re_bt->setEnabled(true);
        ui->live_bt->setEnabled(true);
    }
}

void MainWindow::startPlayback() {
    if (playThread) return;

    int fps = 60;   // 你原本的影片 FPS
    playThread = new PlayThread(fps, this);

    connect(playThread, &PlayThread::nextFrame,
            this, &MainWindow::showNextFrame,
            Qt::QueuedConnection);

    playThread->start();
}

void MainWindow::stopPlayback() {
    if (playThread) {
        playThread->stop();
        playThread->wait();
        delete playThread;
        playThread = nullptr;
    }
}

// MainWindow.cpp
void MainWindow::showLoading(bool show, const QString& text)
{
    if (!ui->graphicsView) return;

    if (show) {
        if (!loadingLabel) {
            // 建立 QLabel 疊在 graphicsView 上
            loadingLabel = new QLabel(ui->graphicsView->viewport());
            loadingLabel->setStyleSheet("color: white; "
                                        "background-color: rgba(0,0,0,150); "
                                        "font: bold 48px 'Microsoft JhengHei';");
            loadingLabel->setAlignment(Qt::AlignCenter);
            loadingLabel->setGeometry(ui->graphicsView->viewport()->rect());

            loadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 不擋滑鼠
        }
        loadingLabel->setText(text);
        loadingLabel->show();
    } else {
        if (loadingLabel) {
            loadingLabel->hide();
        }
    }
}

// MainWindow.cpp
// MainWindow.cpp
void MainWindow::showSeekOverlay(bool show, int frameIndex, double fps)
{
    if (!ui->graphicsView) return;

    if (show) {
        if (!seekLabel) {
            seekLabel = new QLabel(ui->graphicsView->viewport());
            seekLabel->setStyleSheet("color: yellow; "
                                     "background-color: rgba(0,0,0,180); "
                                     "font: bold 40px 'Microsoft JhengHei'; "
                                     "border-radius: 10px; "
                                     "padding: 10px;");
            seekLabel->setAlignment(Qt::AlignCenter);
            seekLabel->setFixedSize(360, 100);  // 寬高
            seekLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        }

        // === 計算時間字串 (分:秒:毫秒) ===
        double seconds = (fps > 0) ? (frameIndex / fps) : 0.0;
        int mm = static_cast<int>(seconds / 60);
        int ss = static_cast<int>(seconds) % 60;
        int ms = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

        QString timeStr = QString("%1:%2:%3")
                            .arg(mm, 2, 10, QChar('0'))
                            .arg(ss, 2, 10, QChar('0'))
                            .arg(ms, 3, 10, QChar('0'));

        seekLabel->setText(timeStr);

        // === 顯示在畫面中央 ===
        QRect viewRect = ui->graphicsView->viewport()->rect();
        int x = (viewRect.width() - seekLabel->width()) / 2;
        int y = (viewRect.height() - seekLabel->height()) / 2;
        seekLabel->move(x, y);

        seekLabel->show();
    } else {
        if (seekLabel) {
            seekLabel->hide();
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    this->hide();
    Cam_Fun.DeleteAllCam();
    Login dlg;
    dlg.show();
    if (dlg.exec() == QDialog::Accepted) {
        this->show();
        imageItem->setPixmap(QPixmap());

        ui->graphicsView->scene()->setBackgroundBrush(Qt::black);

        QString targetIP = "172.16.0.1"; // 請根據實際 IP 設定

        // 設定 JSON 與網址
        QJsonObject data;

        QUrl url("http://" + targetIP + "/ctrl/info");

        showLoading(true, "相機連線中");

        // 發送請求
        apiClient->postJson(url, data, "info");

//        auto *nw = new MainWindow();   // 重新宣告一個新的 MainWindow
//        nw->showFullScreen();          // 或 nw->show();
//        this->deleteLater();           // 再把舊的安全丟掉
    } else {
        this->show();

    }
}

