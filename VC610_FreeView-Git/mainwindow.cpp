#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>

#define PASSWORD "pass"


void decStringToFile(const char *encrypted_text, const char *pwd, const char *output_filename) {
    int j0 = 0;
    int pwd_len = strlen(pwd);
    int text_len = strlen(encrypted_text);

    std::ofstream outputFile(output_filename); // 打开要写入的文件
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open output file" << std::endl;
        return;
    }

    /* 進行解密 */
    for (int i = 0; i < text_len; ++i) {
        char ch = encrypted_text[i];
        ch -= pwd[j0++ % pwd_len]; // 循環使用密碼
        outputFile << ch; // 将解密后的字符写入文件
    }

    outputFile.close(); // 关闭文件
}

QString decString3(const char *encrypted_text, const char *pwd) {
    int j0 = 0;
    int pwd_len = strlen(pwd);
    int text_len = strlen(encrypted_text);

    QByteArray decrypted_bytes;

    /* 進行解密 */
    for (int i = 0; i < text_len; ++i) {
        char ch = encrypted_text[i];
        ch -= pwd[j0++ % pwd_len]; // 循環使用密碼
        decrypted_bytes.append(ch);
    }

    // 使用 fromUtf8 將 QByteArray 轉換為 QString
    return QString::fromUtf8(decrypted_bytes);
}

std::wstring decryptString(const char* encrypted_text, const char* pwd) {
    int j0 = 0;
    int pwd_len = strlen(pwd);
    int text_len = strlen(encrypted_text);

    std::wstring decrypted_text;

    /* 进行解密 */
    for (int i = 0; i < text_len; ++i) {
        char ch = encrypted_text[i];
        ch -= pwd[j0++ % pwd_len]; // 循环使用密码
        decrypted_text += static_cast<wchar_t>(ch); // 将字符添加到 std::wstring 中
    }

    return decrypted_text;
}

std::string encryptString(const std::string& input, const std::string& key) {
    std::string encryptedText = input; // 将输入的字符串复制到加密字符串中

    // 对每个字符进行加密
    for (size_t i = 0; i < input.size(); ++i) {
        encryptedText[i] += key[i % key.size()]; // 使用密钥对字符进行加密
    }

    return encryptedText;
}

// 将 std::wstring 转换为 const char*
const char* wstringToChar(const std::wstring& wstr) {
    // 创建一个本地转换对象，将宽字符转换为多字节字符
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    // 使用转换器将宽字符字符串转换为多字节字符字符串
    std::string convertedString = converter.to_bytes(wstr);
    // 返回转换后的 C 字符串（注意：返回的指针会在函数结束时失效）
    return convertedString.c_str();
}

std::string decfile(QString filepath)
{

    std::string JsonString;
    std::string currentSettingsFileStr = filepath.toStdString();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_str = converter.from_bytes(currentSettingsFileStr);
    std::cout<<"currentSettingsFileStr"<<currentSettingsFileStr<<endl;

    std::ifstream inputFile(wide_str);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return "error";
    }

    // 读取加密文件的内容
    std::getline(inputFile, JsonString);

    // 关闭文件
    inputFile.close();
    const char * jsonCharArray  = JsonString.c_str();
    QString tmpString;
    tmpString = decString3(jsonCharArray,PASSWORD);
    return JsonString;
}


QString MainWindow::getLastProject() {
    QFile file(QString::fromUtf8(BASE_RESOURCE) + QString::fromUtf8(APP_READ));
    if (!file.exists())
    {
        QJsonObject appReadObj;
        appReadObj["lastProject"] = QString::fromUtf8(BASE_RESOURCE) + QString::fromUtf8(APPSET);
        QString fileName = QString::fromUtf8(BASE_RESOURCE) + QString::fromUtf8(APPSET);
        qDebug()<<"fileName:"<<fileName;

        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QJsonDocument json_doc(appReadObj);
//        QTextStream stream(&file);
//        stream << json_doc.toJson() << endl;
        file.close();

        std::string tmp3 = encryptString(json_doc.toJson().toStdString(), PASSWORD);
        const char* encryptedTextChar = tmp3.c_str();


        QByteArray byteArray = fileName.toUtf8();

        // 将 QByteArray 转换为 const char*
        const char* charPtr = byteArray.constData();
        qDebug()<<"charPtr123："<<charPtr;
        std::ofstream outputFile(charPtr);
        if (!outputFile.is_open()) {
            std::cerr << "Error opening file1!!" << std::endl;
            return "ERROR";
        }

        //将加密后的字符串写入文件
        outputFile << encryptedTextChar;
        outputFile.close();
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening file:" << file.errorString();
        return "";
    }
    QTextStream in(&file);
    //QString JsonString = in.readAll();//error
    file.close();

    std::string JsonString;
    QString tmp1 = BASE_RESOURCE;
    QString tmp2 = APP_READ;
    QString fileName = tmp1 + tmp2;
    // 打开加密文件
    qDebug()<<"test";

    std::string currentSettingsFileStr = fileName.toStdString();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_str = converter.from_bytes(currentSettingsFileStr);


    std::ifstream inputFile(wide_str);
    if (!inputFile.is_open()) {
        std::wcout << "Error opening file2:"<<fileName.toStdWString() << std::endl;
        return "Error2";
    }

    // 读取加密文件的内容
    std::getline(inputFile, JsonString);
    // 关闭文件
    inputFile.close();


    const char * jsonCharArray  = JsonString.c_str();

    qDebug()<<"====test====";


    QString test = "";
    std::string test2;
    std::wstring test3;

    test = decString3(jsonCharArray,PASSWORD);

    QByteArray byteArray = test.toUtf8();

    // 将 QByteArray 转换为 const char*
    const char* charPtr = byteArray.constData();
    // 使用 std::wstring_convert 進行字符集轉換
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter2;
    std::wstring utf16Str = converter2.from_bytes(charPtr);
    qDebug()<<"charPtr554444："<<utf16Str;

    // 使用 std::wstring_convert 進行字符集轉換

    QString qString = test;
    qDebug()<<"qString"<<qString;

    QJsonDocument doc = QJsonDocument::fromJson(byteArray);
//    JsonString = decString(JsonString.toUtf8().constData(),password);
//    QJsonDocument doc = QJsonDocument::fromJson(JsonString.toUtf8());


    QJsonObject obj = QJsonDocument::fromJson(qString.toUtf8()).object();

    return obj["lastProject"].toString();
}

QWidget* MainWindow::createRedDot() {
        redDot = new QLabel();
        redDot->setFixedSize(12, 12); // 設置紅色點的大小
        redDot->setStyleSheet("background-color: red; border-radius: 5px;"); // 設置紅色背景和圓角
        ui->statusbar->addPermanentWidget(redDot); // 將紅色點添加到狀態欄的最右邊

        return redDot;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Syncshot");
    installEventFilter(this);

//    AudioWaveformWidget* waveformWidget = new AudioWaveformWidget(this);

    // UI 佈局
//    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->addWidget(waveformWidget);
//    setLayout(layout);

    //    std::thread([this, waveformWidget]() {
    //        while (true) {
    //            if (QThread::currentThread()->isInterruptionRequested())
    //                break;

    //            float db = Cam_Fun.returnDB();
    ////            qDebug()<<"DB:"<<db;
    //            QMetaObject::invokeMethod(waveformWidget, [waveformWidget, db]() {
    //                waveformWidget->addSample(db);  // 在主執行緒更新 UI
    //            });

    //            std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 控制更新頻率
    //        }
    //    }).detach();

    multiple_vid_ctx_param Cam_Video_param;
    Cam_Video_param.fps = appSettings.vidSetting.fps;
    Cam_Video_param.width = appSettings.vidSetting.width;
    Cam_Video_param.height = appSettings.vidSetting.height;
    Cam_Video_param.codec_id = appSettings.vidSetting.codec_id;
    Cam_Fun.set_CamVideo_param(Cam_Video_param);

    // 创建一个 QSettings 对象，指定注册表路径
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", QSettings::NativeFormat);

    // 使用 value() 方法获取注册表中的值
    QVariant value = settings.value("MachineGuid");
    if (value.isValid()) {
        qDebug() << "Program Files Directory:" << value.toString();
    } else {
        qDebug() << "Failed to read value from registry.";
    }

    statusBar()->addWidget(createRedDot()); // 將紅色點添加到狀態欄
    redDot->setVisible(false);


    // 讀取參數
    readParamFile();
    // 初始化變數
    Initalize();
    // 初始化UI
    InitalizeUI();
    // 連結信號
    connectSignals();
    // 是否自動啟動
    if (appSettings.app.AUTOSTART)
        autoStart();

    QProcess q1;
    q1.start("sh", QStringList() << "-c" << "htop");

    angle = 0;

//    ParamDockBase* dock = new ParamDockBase();
//    addDockWidget(Qt::RightDockWidgetArea, dock);
//    dock->setObj([this](){ return appSettings.toObj(); });

    //QRcode
//    QrCodeGenerator generator;
//    QString data = "https://molly1024.medium.com/%E6%96%B0%E7%89%88-react-router-%E6%80%8E%E9%BA%BC%E7%94%A8-react-router-dom-v6-8c0624642fce";
//    QImage qrCodeImage = generator.generateQr(data);
//    qrCodeImage.save("aaa.png");

//    ui->statusbar->setVisible(fullScreen);
//    ui->menubar->setVisible(fullScreen);
//    functionDock->setVisible(fullScreen);
//    fullScreen = !fullScreen;

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_S)
        {
            ui->actionsave->trigger();
            statusBar()->showMessage("saved!");
            return true;
        }
        else if (keyEvent->key() == Qt::Key_L) {
            Cam_Fun.log();
        }
        else if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_O) {
            appSettings.app.AUTOSTART = true;
            autoStart();
        }
        else if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_R) {
            isShowTime = !isShowTime;
        }
    }

//    if(event->type() == QEvent::MouseButtonPress)
//    {


//        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
//        if(mouseEvent->button() == Qt::RightButton)
//        {
//            QGraphicsItem *item = Screen->m_pixmap;
//            QPoint scenePos = mouseEvent->pos();
//            qDebug()<<Screen->inQWidget(scenePos,item);
//        }

//    }

    return QObject::eventFilter(obj, event);
};

void MainWindow::autoStart()
{
    ui->actionAuto_Start->setChecked(appSettings.app.AUTOSTART);
//    ui->actionEnc_Set->setEnabled(false);
    functionDock->setButton_Disable(UI_FunctionDock::bt_scan,true);
    toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,true);
    QVector<CamInfo> cam_param_list;

    //掃描自動開始的相機清單
    for (int i = 0; i < appSettings.app.autoCamList.size(); i++) {
        if(apiCtrl.scanCamIP(appSettings.app.autoCamList[i].ip) != -1)
            cam_param_list.append(appSettings.app.autoCamList[i]);
    }

    if (cam_param_list.isEmpty())
    {
        functionDock->setButton_Disable(UI_FunctionDock::bt_scan,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,false);
        return;
    }

    scanFinished(cam_param_list);

    //if(cam_param_list)

    CamVC610_WebAPI_Ctrl *wabAPI = new CamVC610_WebAPI_Ctrl();

    connect(wabAPI, &CamVC610_WebAPI_Ctrl::masterCam_signal,this, [=](QString tmpString){
        masterCam = tmpString;
        allSetting->masterCam = masterCam;
    });


    wabAPI->autoScanCamIP();


    connect(wabAPI, &CamVC610_WebAPI_Ctrl::setListDateTimeFinish, [=](){
        functionDock->setButton_Disable(UI_FunctionDock::bt_scan, true);
        functionDock->setButton_Disable(UI_FunctionDock::bt_stream, false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan, true);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream, false);

        if(streamStart(cam_param_list))
        {
            status = AppStatus::Stream;
        }else
        {
//            ui->actionEnc_Set->setEnabled(true);
            functionDock->setButton_Disable(UI_FunctionDock::bt_scan,false);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,false);
        }
    });
    wabAPI->setListDatetime(cam_param_list);
}

QString MainWindow::readFileToString(QString name) {
    QFile file(name);

    if (!file.exists())
        wirteParamFile(AppSettings());

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error opening file:" << file.errorString();
    }

//    QTextStream in(&file);
//    QString JsonString = in.readAll();//error
    file.close();

    std::string JsonString;
    std::string tmp1 = BASE_RESOURCE;
    std::string tmp2 = APP_READ;
    std::string fileName = tmp1 + tmp2;

    // 打开加密文件
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file2" << std::endl;
        return "Error";
    }

    // 读取加密文件的内容
    std::getline(inputFile, JsonString);

    // 关闭文件
    inputFile.close();

    const char* jsonCharArray  = JsonString.c_str();

    QString tmp;
    tmp = decString3(jsonCharArray,PASSWORD);
    qDebug()<<"tmp:"<<tmp;
    QByteArray byteArray = tmp.toUtf8();
    QString qString = tmp;

    QJsonDocument doc = QJsonDocument::fromJson(byteArray);
//    JsonString = decString(JsonString.toUtf8().constData(),password);
//    QJsonDocument doc = QJsonDocument::fromJson(JsonString.toUtf8());
    QJsonObject obj = doc.object();


    return qString;
}

void MainWindow::setCurrentSelect(QString currentFile) {

    QString fileName = QString::fromUtf8(BASE_RESOURCE) + QString::fromUtf8(APP_READ);

    //fileName = "D:/qt/project/build-VC610_FreeView-Desktop_Qt_5_14_1_MSVC2017_64bit-Debug/tmp2.txt";
    qDebug()<<"fileName："<<fileName;
    QFile file(fileName);

    QJsonObject appReadObj;
    appReadObj["lastProject"] = currentFile;

    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QJsonDocument json_doc(appReadObj);
    //QTextStream stream(&file);
    //stream << json_doc.toJson() << endl;
    file.close();

    std::string tmp3 = encryptString(json_doc.toJson().toStdString(), PASSWORD);
    const char* encryptedTextChar = tmp3.c_str();

//    std::cout << "Original string: " << tmp3 << std::endl;
//    qDebug() << "---------------";

    QByteArray byteArray = fileName.toUtf8();

    // 将 QByteArray 转换为 const char*
    const char* charPtr = byteArray.constData();

    std::ofstream outputFile(charPtr);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file4" << std::endl;
        return ;
    }

    //将加密后的字符串写入文件
    outputFile << encryptedTextChar;
    outputFile.close();


    currentSettingsFile = currentFile;
    setWindowTitle("Syncshot - " + currentSettingsFile);
}

// TODO readfile
void MainWindow::readParamFile()
{
    logger.log(LogLevel::INFO, "===============");

    QDir dir(BASE_RESOURCE);

    if(!dir.exists())
        dir.mkpath(".");

    currentSettingsFile = getLastProject();
    qDebug() <<"currentSettingsFile123："<< currentSettingsFile;

//    QString test = "D:/qt/project/build-VC610_FreeView-Desktop_Qt_5_14_1_MSVC2017_64bit-Debug/tmp0.txt";
//    currentSettingsFile = test;
//    qDebug() << currentSettingsFile;

    QFile file(currentSettingsFile);

    if (!file.exists()) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("New project"), QDir::currentPath(), tr("Text Files (*.txt)"));

        //QString fileName = QFileDialog::getSaveFileName(this, tr("Open project"), QDir::currentPath(), tr("Open project (*.txt)"));
        if (fileName.isEmpty())
        {
            int count = 0;
            QString tmp = "tmp" + QString::number(count) + ".txt";
            while (QFile::exists(tmp)) {
                ++ count;
                tmp = "tmp" + QString::number(count) + ".txt";
            }
            fileName = tmp;

        }

    //    qDebug()<<"TMP："<<tmp;

    //    stream << json_doc.toJson() << endl;
        wirteParamFile(AppSettings(), fileName);
        setCurrentSelect(fileName);
        file.setFileName(fileName);
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        showMessage("file ERROR","Error opening file:"+file.errorString());
        qDebug() << "Error opening file:" << file.errorString();
    }

    QTextStream in(&file);

    //QString JsonString = in.readAll();//error
    //qDebug() << "JsonString:" << JsonString;

    file.close();

    /*!!!!!*/
    std::string JsonString;
    std::string currentSettingsFileStr = currentSettingsFile.toStdString();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_str = converter.from_bytes(currentSettingsFileStr);
    /*!!!!!*/

    // 打开加密文件
    std::ifstream inputFile(wide_str);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file23" << std::endl;
        return;
    }

    // 读取加密文件的内容
    std::getline(inputFile, JsonString);

    // 关闭文件
    inputFile.close();

    const char* jsonCharArray  = JsonString.c_str();

    QString tmp;
    tmp = decString3(jsonCharArray,PASSWORD);
    QByteArray byteArray = tmp.toUtf8();
    QString qString = tmp;


    QJsonDocument doc = QJsonDocument::fromJson(byteArray);
//    JsonString = decString(JsonString.toUtf8().constData(),password);
//    QJsonDocument doc = QJsonDocument::fromJson(JsonString.toUtf8());
    QJsonObject obj = doc.object();

    appSettings = AppSettings::toStruct(obj);

    isReadParamFile = true;
    logger.log(LogLevel::INFO, "readParamFile");
    setWindowTitle("Syncshot - " + currentSettingsFile);
}

void MainWindow::Initalize()
{
    Cam_Fun.init(appSettings.Decoder, appSettings.Encoder);
    status = AppStatus::CamList_Empty;
    logger.log(LogLevel::INFO, "Initalize");
}

void MainWindow::InitalizeUI()
{
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QSize screenSize = primaryScreen->size();

    if (screenSize.width() == 1920)
    {
        setStyleSheet(QString(FONTFAMILY) + FONTSIZE_18);
    }
    else if (screenSize.width() < 3840 && screenSize.width() > 1920)
    {
        setStyleSheet(QString(FONTFAMILY) + FONTSIZE_24);
    }
    else if (screenSize.width() == 3840)
    {
        setStyleSheet(QString(FONTFAMILY) + FONTSIZE_36);
        table_line_height = 60;
    }
    //=======graphicsView
    Screen = new CustomGraphicsView(ui->graphicsView, this);
    setCentralWidget(ui->graphicsView);

    //=======FunctionDock
    functionDock = new FunctionDock(this);
    functionDock->setMinimumHeight(200);
    functionDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, functionDock);
    functionDock->setButton_Disable(UI_FunctionDock::init);
    functionDock->setAppSetting(&appSettings);

    //=======toolDock
    toolDock = new ToolDock(this);
    toolDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    toolDock->setButton_Disable(UI_ToolDock::tool_init);
    toolDock->setAppSetting(&appSettings);
    addDockWidget(Qt::BottomDockWidgetArea, toolDock);


    //=======Setting
    allSetting  = new Allsetting(this);
    camSetting  = new CamSetting(this);
    camScan     = new CameraScan(this);
    enc_set     = new EncoderSetting(this);
    aoCamSetting= new AoCamSetting(this);


    logger.log(LogLevel::INFO, "InitalizeUI");
}

void MainWindow::connectSignals()
{
    connect(toolDock, &ToolDock::StreamSignal,this, [this](){
       functionDock->click_StreamButton();
    });
    connect(toolDock, &ToolDock::tool_scanButtonClick, this, &MainWindow::scanButtonClick);
    connect(toolDock, &ToolDock::tool_rtsp_Signal, this, &MainWindow::RTSP_Clicked);
    connect(toolDock, &ToolDock::tool_rotate_Signal, this, &MainWindow::Screen_Rotate);
    connect(toolDock, &ToolDock::record_Signal, this, &MainWindow::Screen_Record);
    connect(toolDock, &ToolDock::tool_combo_change, this, [this] (int index){
        functionDock->change_mode(index);
    });


    connect(functionDock, &FunctionDock::scanButtonClick, this, &MainWindow::scanButtonClick);
//    connect(functionDock, &FunctionDock::startButtonClick, this, &MainWindow::record_Start);
//    connect(functionDock, &FunctionDock::stopButtonClick, this, &MainWindow::record_Stop);
    connect(functionDock, &FunctionDock::StreamSignal, this, &MainWindow::bt_stream_Click);
//    connect(functionDock, &FunctionDock::changeCam, this, &MainWindow::bt_stream_Click);
    connect(functionDock, &FunctionDock::onLiveCamNo_change, this, &MainWindow::onCameraNumberChange);
    connect(functionDock, &FunctionDock::ondeleteAllCam, this, &MainWindow::onCameraAllDelete);
    connect(functionDock, &FunctionDock::ondeleteCam, this, &MainWindow::onCameraDelete);
    connect(functionDock, &FunctionDock::onstopCam, this, &MainWindow::onCameraStop);
    connect(functionDock, &FunctionDock::onstartCam, this, &MainWindow::onCameraStart);
    connect(functionDock, &FunctionDock::onStreamModeChange, this, &MainWindow::onModeChange);
    connect(functionDock, &FunctionDock::rtsp_Signal, this, &MainWindow::RTSP_Clicked);
    connect(functionDock, &FunctionDock::record_Signal, this, &MainWindow::Screen_Record);
    connect(functionDock, &FunctionDock::rotate_Signal, this, &MainWindow::Screen_Rotate);
    connect(functionDock, &FunctionDock::onFPSChange, this, [this] (int fps) {
        appSettings.app.FPS = fps;
        if (setth)
            setth->setFPS(appSettings.app.FPS);
    });
    connect(Screen, &CustomGraphicsView::fullScreen, this, [this] (){
        if(fullScreen)
        {
//            bool ok;
//            QString password = QInputDialog::getText(nullptr, "Login", "Password:", QLineEdit::Password, "", &ok);
//            if (!ok)
//                return 0;

//            if (password == "password") {
//                QMessageBox::information(nullptr, "Login", "Login successful!");
//            } else {
//                QMessageBox::critical(nullptr, "Login", "Invalid password!");
//                return 0;
//            }
            showNormal();
        }else{
            showFullScreen();
        }
        ui->statusbar->setVisible(fullScreen);
        ui->menubar->setVisible(fullScreen);
        functionDock->setVisible(fullScreen);
        toolDock->setVisible(fullScreen);

        fullScreen = !fullScreen;
    });
    connect(Screen, &CustomGraphicsView::newScreen,this, [this](){


        SecondScreen.append(new secWindow(nullptr));

        SecondScreen[SecondScreen.size()-1]->showFullScreen();

        add_secScreen = !add_secScreen;
    });

    connect(Screen, &CustomGraphicsView::newScreenMenu,this, [this](int screenID){

        QList<QScreen*> screens = QGuiApplication::screens();


        SecondScreen.append(new secWindow(nullptr));

        if(screens.size()>0)
        {
            qDebug()<<"screenNum："<<screenID;
            QScreen *secondScreen = screens.at(screenID);
            QRect secondScreenGeometry = secondScreen->geometry();

            // 将窗口移动到第二个屏幕的中心
            SecondScreen[SecondScreen.size()-1]->setGeometry(secondScreenGeometry);
        }

        SecondScreen[SecondScreen.size()-1]->showFullScreen();
        SecondScreen.size();

        add_secScreen = !add_secScreen;
    });

    connect(Screen, &CustomGraphicsView::Record_signal, this, [this] (bool start){
       if(status == AppStatus::Stream)
       {
           //functionDock->StartRecord();
           toolDock->StartRecord();
        }
    });

    connect(Screen, &CustomGraphicsView::cameraSwitch, this, [this] (int UpDown) {
        if(ScreenNum >= camList.size())
        {
            showMessage("ScreenNum ERROR","ScreenNum out of range.");
            qDebug()<<"ScreenNum out of range";
            return;
        }

        if(ScreenNum+UpDown>=camList.size())
        {
            ScreenNum = 0;
        }else if(ScreenNum+UpDown < 0)
        {
            ScreenNum = camList.size()-1;
        }else
        {
            ScreenNum = ScreenNum + UpDown;
        }

        onCameraNumberChange(ScreenNum);
        //qDebug()<<UpDown;
    });
//    connect(camSetting, &CamSetting::returnParam, this, [=](Cam_Set set){
//        appSettings.camSetting = set;
//        wirteParamFile(appSettings, currentSettingsFile);
//    });


    connect(camScan, &CameraScan::masterCam_signal,this, [=](QString tmpString){
       masterCam = tmpString;
       allSetting->masterCam = masterCam;
    });
    connect(camScan, &CameraScan::scanConfirm, this, &MainWindow::scanFinished);
    connect(camScan, &CameraScan::setIP_Range, this, [=](QList<QString> list){
        appSettings.app.IpRangeList = list;
        wirteParamFile(appSettings, currentSettingsFile);
    });
    connect(allSetting, &Allsetting::returnParam, this, [=](Cam_Set set){
        appSettings.camSetting = set;
        wirteParamFile(appSettings, currentSettingsFile);
    });
    connect(allSetting, &Allsetting::sentEncoderSetting, this, [=](Encoder_param param){
        appSettings.Encoder = param;

        Cam_Fun.set_Encoder_param(appSettings.Encoder);
        wirteParamFile(appSettings, currentSettingsFile);
    });
    connect(allSetting, &Allsetting::confirmBtn_signal, this, [=](AoFunSetting param){
        appSettings.aoFunSetting = param;
        Cam_Fun.set_sp_Mode(appSettings.aoFunSetting.SP_mode);
        wirteParamFile(appSettings, currentSettingsFile);
    });
    connect(allSetting, &Allsetting::c_id_signal, this, [=](std::vector<uint32_t> c_id){
        SPID_list = c_id;
        Cam_Fun.set_view_sp_num(c_id);
    });
    connect(allSetting, &Allsetting::record_signal, this, [=](Record_param recordSetting){
        appSettings.recordSetting = recordSetting;
        wirteParamFile(appSettings,currentSettingsFile);
    });
    connect(allSetting, &Allsetting::RTSP_signal, this, [=](RTSP_param RTSPset)
    {
        appSettings.RTSPSetting = RTSPset;
        wirteParamFile(appSettings,currentSettingsFile);
    });

    //Menu Action
    connect(ui->actionSetting, &QAction::triggered, this,[=](){
        qDebug() << currentSettingsFile;

        int tmpbl;

        for(int i =0;i<SPID_list.size();i++)
        {
            tmpbl = false;
            for(int j = 0; j<camList.size();j++)
            {
                if(SPID_list[i]==camList[j].id)
                {
                    tmpbl = true;
                    break;
                }
            }
            if(!tmpbl)
            {
                SPID_list[i] = -1;
            }
        }

        allSetting->AO_setUIValue(appSettings.aoFunSetting);
        allSetting->set_init_param(appSettings.Encoder, 0, true);//sentEncoderSetting
        allSetting->setSPID_list(SPID_list,camList);
        allSetting->setUIValue(appSettings.camSetting);
        allSetting->setRecordUI(appSettings.recordSetting);
        qDebug()<<"=================";
        qDebug()<<appSettings.RTSPSetting.rtsp_path;
        allSetting->set_RTSP_value(appSettings.RTSPSetting);
        allSetting->show();

    });
//    connect(ui->actionCam_Set, &QAction::triggered, this,[=](){
//        qDebug() << currentSettingsFile;
//        camSetting->show();
////        camSetting->setIsStreaming(status == AppStatus::Stream);
//        camSetting->setUIValue(appSettings.camSetting);
//    });
//    connect(ui->actionEnc_Set, &QAction::triggered, this,[=](){
//        enc_set->set_init_param(appSettings.Encoder, 0, true);//sentEncoderSetting
//        enc_set->show();
//    });
//    connect(ui->actionAo_Setting, &QAction::triggered, this,[=](){
//        aoCamSetting->setUIValue(appSettings.aoFunSetting);
//        aoCamSetting->show();
//    });
    connect(ui->actionAuto_Start, &QAction::triggered, this,[=](bool checked){
        if(status == AppStatus::Stream)
        {
            showMessage("Error","can't add camera in stream.");
            ui->actionAuto_Start->setChecked(false);
            return;
        }

        appSettings.app.AUTOSTART = checked;
        if (checked)
            autoStart();
    });
    connect(ui->actionsave, &QAction::triggered, this, [=] () {
        wirteParamFile(appSettings, currentSettingsFile);
    });
    connect(ui->actionsave_as, &QAction::triggered, this, [=](){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.txt)"));
        if (fileName.isEmpty()) return;

        setCurrentSelect(fileName);
        wirteParamFile(appSettings, fileName);
    });
    connect(ui->actionopen, &QAction::triggered, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open txt"), "", "cam focus (*.txt )");
        if (fileName.isEmpty()) return;

        QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
        auto a = AppSettings::toStruct(JDoc.object());

        setCurrentSelect(fileName);

        // TODO Cam_Fun 全部清空再重新開始

        if(status == AppStatus::Stream)
        {
            streamStop();
            functionDock->deleteAllCam();
        }
        Sleep(500);

        // 刪除 & 重新開

        Initalize();
        autoStart();
    });

    connect(timer, &QTimer::timeout, [&]() {

        time++;

        qDebug()<<"time:"<<time;
         if(time == appSettings.recordSetting.sec)
         {
             //functionDock->StartRecord();
             toolDock->StartRecord();

             timer->stop();
             qDebug()<<"timeout!";
         }
    });

    logger.log(LogLevel::INFO, "connectSignals");
}

void MainWindow::RTSP_Clicked()
{
    int ret;

    if (!isRTSP_OPEN)
    {
        if(appSettings.RTSPSetting.service == 0)
        {
            ret = Cam_Fun.open_rtsp(appSettings.RTSPSetting.rtsp_path.toStdString());
        }else
        {
            ret = Cam_Fun.open_rtmp(appSettings.RTSPSetting.rtmp_path.toStdString());
        }
    }
    else
    {
        if(appSettings.RTSPSetting.service == 0)
        {
            ret = Cam_Fun.close_rtsp();
        }else
        {
            ret = Cam_Fun.close_rtmp();
        }
    }
    if (ret == 0)
    {
        isRTSP_OPEN = !isRTSP_OPEN;
        ui->actionAo_Setting->setEnabled(!isRTSP_OPEN);
        //functionDock->RTSP_Change(true, isRTSP_OPEN);
        toolDock->RTSP_Change(true, isRTSP_OPEN);

        ui->actionEnc_Set->setEnabled(!isRTSP_OPEN);

        allSetting->setEnable_RTSP(isRTSP_OPEN);


        qDebug()<<"RTSP Click";
    }else if(ret == -1) //出現錯誤
    {
        qDebug()<<"RTSP false";

        if(!isRTSP_OPEN)//想開RTSP
        {
            showMessage("Error", "Can't open stream.");
            //functionDock->RTSP_Change(true, false);
            toolDock->RTSP_Change(true, false);

        }else//想關RTSP
        {
            //functionDock->RTSP_Change(true, false);
            toolDock->RTSP_Change(true, false);

        }

    }
}

void MainWindow::wirteParamFile(AppSettings set, QString fileName)
{
    if (fileName == "")
        fileName = QString::fromUtf8(BASE_RESOURCE) + QString::fromUtf8(APPSET);

//    fileName = "D:/qt/project/build-VC610_FreeView-Desktop_Qt_5_14_1_MSVC2017_64bit-Debug/tmp2.txt";
//    qDebug()<<"fileName："<<fileName;
    QFile file(fileName);

    QJsonObject setting_obj = set.toObj();
    QJsonDocument json_doc(setting_obj);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);

    QString tmp = QString(json_doc.toJson());

//    qDebug()<<"TMP："<<tmp;

//    stream << json_doc.toJson() << endl;
    //qDebug()<<QString(json_doc.toJson());
    //qDebug()<<decString(tmp,password);
    file.close();
//    logger.log(LogLevel::INFO, "Write File Success");


    std::string tmp3 = encryptString(json_doc.toJson().toStdString(), PASSWORD);
    const char* encryptedTextChar = tmp3.c_str();
    decStringToFile(encryptedTextChar,PASSWORD,"tmp3.txt");

//    std::cout << "Original string: " << tmp3 << std::endl;
//    qDebug() << "---------------";

    // 打开一个文件来写入加密后的字符串
    // 将 QString 转换为 QByteArray
    QByteArray byteArray = fileName.toUtf8();

    // 将 QByteArray 转换为 const char*
    const char* charPtr = byteArray.constData();
    // 使用 std::wstring_convert 進行字符集轉換
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring utf16Str = converter.from_bytes(charPtr);

    qDebug()<<"charPtr555："<<utf16Str;
    std::wofstream outputFile(utf16Str);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file!:" <<charPtr<< std::endl;

        return ;
    }

    //将加密后的字符串写入文件
    outputFile << encryptedTextChar;
    outputFile.close();
    //qDebug()<<decString(encryptedTextChar,password);


}

// init camera
void MainWindow::camera_Init()
{

}

// TODO setting camera
void MainWindow::camera_Setting()
{

}

// TODO start streaming
void MainWindow::bt_stream_Click(QVector<CamInfo> List)
{
    Lock.lock();
    switch(status)
    {
        case AppStatus::unStream:
        functionDock->setButton_Disable(UI_FunctionDock::bt_scan,true);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,true);
        if(streamStart(List))
        {

        }else
        {
            functionDock->setButton_Disable(UI_FunctionDock::bt_scan,false);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,false);
            //functionDock->recordMode();

            //toolDock->recordMode();
        }

        break;
        case AppStatus::Stream:
        if(streamStop())
        {
            functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,true);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,true);
            if(isRTSP_OPEN)
                RTSP_Clicked();
        }

        break;
        default:
        showMessage("Error","status is not non-executable.");
        break;
    }
    Lock.unlock();
}

void MainWindow::scanButtonClick()
{
    camScan->setScanRangelist(appSettings.app.IpRangeList);
    camScan->setCamlist(camList);
    camScan->setSelect();
//    connect(camScan, &CameraScan::setIP_Range, this, [=](QList<QString> newList){
//        emit setIP_Range(newList);
//    });
    camScan->setTableSize(table_line_height);
    camScan->show();
}

bool MainWindow::streamStart(QVector<CamInfo> list)
{
    if(list.size() ==0)
    {
        qDebug() << "empty List.";
        showMessage("Stream Error","No camera selected!");
        return false;
    }

    Cam_Fun.start_all();
    Cam_Fun.set_Live_num(0);
    ScreenNum = 0;
    Sleep(1000);

    for(int i = 0; i<camList.size();i++)
    {
        camList[i].isConnect = true;
    }

    if(functionDock->mode == 0)
    {
       show_mode = Show_Mode::Show_Single;
    }else
    {
       show_mode = Show_Mode::Show_splice;
    }

    set_SPID(appSettings.aoFunSetting.SPCamList);

    allSetting->setSPID_list(SPID_list,camList);


    Cam_Fun.set_sp_Mode(appSettings.aoFunSetting.SP_mode);
    Cam_Fun.set_view_sp_num(SPID_list);
    Cam_Fun.set_Show_Mode(show_mode);

    functionDock->setTableView(camList);

    allSetting->setDisableByIP(appSettings.camSetting.IP);
    functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,false);
    functionDock->setButton_Disable(UI_FunctionDock::combo_mode,false);

    toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,false);
    toolDock->setButton_Disable(UI_ToolDock::tool_combo_mode,false);
    toolDock->setButton_Disable(UI_ToolDock::tool_bt_record,false);

    if (!setth)
    {
        int FPS = checkFPS_string(appSettings.camSetting.videoFrameRate);
        if(FPS<0)
            return false;

        Stream_first = false;
        //functionDock->setButton_Disable(UI_FunctionDock::bt_stream,true);
        functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,false);
        functionDock->recordMode();
        toolDock->recordMode();
        setth = new FrameThread(appSettings.app.FPS);
        connect(setth, &FrameThread::refreash, this, &MainWindow::updateScreen);
        connect(setth, &FrameThread::destroyed, [this]{ setth = nullptr; });
        setth->start();
    }
    return true;
}

void MainWindow::onCameraNumberChange(int index)
{
    if (index < 0)
        return;


    //todo 改成根據index找 cam ID
    ScreenNum = index;
    Cam_Fun.set_Live_num(camList[index].id);
}

void MainWindow::onCameraDelete(int index)
{

    Cam_Fun.DeleteCam(camList[index].id);
    camList.remove(index);
    functionDock->setTableView(camList);
    if(camList.size() > 0)
    {
        //切換回第一個相機畫面
        functionDock->tableViewChange(0);
        onCameraNumberChange(0);
    }else
    {
        if(status == AppStatus::Stream)
        {
            if(streamStop())
            {
                functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,true);
                functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,true);

                toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,true);
                toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,true);
                if(isRTSP_OPEN)
                    RTSP_Clicked();
            }
            Screen->m_pixmap->setPixmap(QPixmap());
            //SecondScreen->secScreen->m_pixmap->setPixmap(QPixmap());
        }
    }
}

void MainWindow::onCameraAllDelete()
{

    Cam_Fun.DeleteAllCam();
    camList.clear();
    functionDock->setTableView(camList);

    if(status == AppStatus::Stream)
    {
        if(streamStop())
        {
            functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,true);
            functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,true);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,true);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,true);
            toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,true);
            if(isRTSP_OPEN)
                RTSP_Clicked();
        }
        Screen->m_pixmap->setPixmap(QPixmap());

        for(int i =0; i<SecondScreen.size();i++)
        {
            SecondScreen[i]->secScreen->m_pixmap->setPixmap(QPixmap());
        }
    }else if(status == AppStatus::unStream)
    {
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,true);
    }

}
void MainWindow::onCameraStart(int index)
{
    camList[index].isConnect = true;
    functionDock->setTableView(camList);
    Cam_Fun.start_cam_num(camList[index].id);
    qDebug()<<"iiiiiiiiiiiiiiiiiiiiindex:"<<index;
}

void MainWindow::onCameraStop(int index)
{
    camList[index].isConnect = false;
    functionDock->setTableView(camList);
    Cam_Fun.stop_cam_num(camList[index].id);
}

void MainWindow::onModeChange(const Show_Mode mode)
{
    if(status == AppStatus::Stream)
    {
        qDebug()<<"mode："<<mode;
        qDebug()<<"SPMODE:"<<SPID_list;
        functionDock->recordMode();
        toolDock->recordMode();
        Cam_Fun.set_Show_Mode(mode);
        //Cam_Fun.set_view_sp_num(SPID_list);

    }
}

bool MainWindow::streamStop()
{
    qDebug() << "streamStop";

    setth->setStart(false);
    setth->wait();
    qDebug() << "th.wait()";

    Cam_Fun.stop_all();
    qDebug() << "Cam_Fun.stop()";

    for(int i = 0; i<camList.size();i++)
    {
        camList[i].isConnect = false;
    }

    allSetting->setDisableByIP(appSettings.camSetting.IP);
    status = AppStatus::unStream;
    functionDock->setTableView(camList);

    Screen->m_pixmap->setPixmap(QPixmap::fromImage(QImage()));
    for(int i =0; i<SecondScreen.size();i++)
    {
        SecondScreen[i]->secScreen->m_pixmap->setPixmap(QPixmap::fromImage(QImage()));
    }
    functionDock->setBt_streamText("stream");
    functionDock->setButton_Disable(UI_FunctionDock::bt_scan,false);
    functionDock->setButton_Disable(UI_FunctionDock::bt_record,true);
    functionDock->setButton_Disable(UI_FunctionDock::combo_mode,true);
    functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,true);;

    toolDock->setButton_Text(UI_ToolDock::tool_bt_stream,"Camera Start");
    toolDock->setButton_Disable(UI_ToolDock::tool_bt_scan,false);
    toolDock->setButton_Disable(UI_ToolDock::tool_bt_record,true);
    toolDock->setButton_Disable(UI_ToolDock::tool_combo_mode,true);
    toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,true);
    return true;
}

void MainWindow::updateScreen()
{
    Cam_Fun.get_LiveView(img);
    if (img->isNull()){
//        qDebug() << "img null";
        return;
    }

    if(!Stream_first)
    {
        status = AppStatus::Stream;

//        ui->actionEnc_Set->setEnabled(false);
        functionDock->setButton_Disable(UI_FunctionDock::bt_stream,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,false);

        functionDock->setBt_streamText("stop Stream");
        toolDock->setButton_Text(UI_ToolDock::tool_bt_stream,"Camera Stop");
        Stream_first = true;
    }
    //!!!時間
    start_time = clock_type::now();
    QPixmap test = QPixmap::fromImage(*img);
    Screen->m_pixmap->setPixmap(test.transformed(QTransform().rotate(angle)));
    //Screen->m_pixmap->setPixmap(test.transformed(QTransform()));
    Screen->setCenter();
    for(int i =0; i<SecondScreen.size();i++)
    {
        SecondScreen[i]->secScreen->m_pixmap->setPixmap(test.transformed(QTransform().rotate(angle)));
        SecondScreen[i]->secScreen->setCenter();
    }
    end_time = clock_type::now();
    DC_cost_time = end_time - start_time;
    if (isShowTime)
        printf("preview cost time %lf ms\n", DC_cost_time.count());
}

void MainWindow::setScale(CustomGraphicsView* CustomView, QGraphicsView* graphicsView)
{
    QImage snapshotFrame = Cam_Fun.get_LiveView();
    double scale = snapshotFrame.width();
    snapshotFrame = snapshotFrame.scaled(graphicsView->size(), Qt::KeepAspectRatio);
    scale = (double)snapshotFrame.width() / scale;
    CustomView->setScale(scale);
}

void MainWindow::setView_SP(std::vector<uint32_t> idArray)
{
    Cam_Fun.set_view_sp_num(idArray);
}

void MainWindow::scanFinished(QVector<CamInfo> list)
{
    if(status == AppStatus::Stream)
    {
        showMessage("Error","can't add camera in stream.");
        logger.log(LogLevel::ERR, "can't add camera in stream.");
        return;
    }

    if (list.size() == 0)
        return;

    status = AppStatus::unStream;

    addAllCam(list);
    auto statusToUnstream = [=](){
        status = AppStatus::unStream;
        functionDock->setTableView(camList);
        appSettings.app.autoCamList = list;

        std::vector<SP_Rot> rot_id;

        rot_id.push_back(static_cast<SP_Rot>(-1));
        rot_id.push_back(static_cast<SP_Rot>(-1));
        rot_id.push_back(static_cast<SP_Rot>(0));
        rot_id.push_back(static_cast<SP_Rot>(0));
        rot_id.push_back(static_cast<SP_Rot>(-1));
//        appSettings.rotSetting.sp_rot = rot_id;

        Cam_Fun.set_view_sp_rot(rot_id);

        wirteParamFile(appSettings, currentSettingsFile);

        functionDock->setButton_Disable(UI_FunctionDock::bt_stream, false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,false);
    };


    if (appSettings.app.AUTOSTART)
    {
        statusToUnstream();
    }
    else
    {
        CamVC610_WebAPI_Ctrl *wabAPI = new CamVC610_WebAPI_Ctrl();
        connect(wabAPI, &CamVC610_WebAPI_Ctrl::setListDateTimeFinish, [=](){
            statusToUnstream();
        });
        wabAPI->setListDatetime(list);

    }

//    camScan->scanFinished(list);
}

void MainWindow::addAllCam(QVector<CamInfo> list)
{
    for (int i = 0; i<list.size(); i++)
    {
        addCam(list[i]);
    }
    functionDock->setTableView(camList);
}

void MainWindow::addCam(CamInfo caminfo)
{
    std::string* ipAddressStdString = new std::string(caminfo.ip.toStdString());
    caminfo.id = Cam_Fun.AddCam(ipAddressStdString->c_str());

    camList.append(caminfo);
    functionDock->setTableView(camList);
}

void MainWindow::deleteAllCam()
{
    Cam_Fun.DeleteAllCam();
    status = AppStatus::CamList_Empty;

    camList.clear();
    functionDock->setTableView(camList);
}

void MainWindow::deleteCam(CamInfo caminfo)
{
    Cam_Fun.DeleteCam(caminfo.id);

    for (int i = 0; i < camList.size(); ++i)
        if(caminfo.id == camList[i].id)
        {
            camList.remove(i);
            break;
        }
    if (camList.size()==0)
    {
        status = AppStatus::CamList_Empty;
    }

    functionDock->setTableView(camList);
}

void MainWindow::camrea_release()
{

}

void MainWindow::showMessage(QString Title, QString msg)
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(Title);
    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton("OK", QMessageBox::AcceptRole);
    msgBox->exec();
}

// release camera
MainWindow::~MainWindow()
{

    if(status == AppStatus::Stream)
        streamStop();
    Sleep(500);
    wirteParamFile(appSettings, currentSettingsFile);
//    wirteParamFile(appSettings);
    logger.log(LogLevel::INFO, "===============");
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
    {
        qDebug()<<"close";
        qDebug()<<SecondScreen.size();
        for(int i =SecondScreen.size()-1 ; i >= 0 ; i--)
        {
            SecondScreen[i]->close();
            delete SecondScreen[i];
        }
        SecondScreen.clear();
        //SecondScreen->close();
        // 繼續處理關閉事件
        event->accept();
    }

bool folderExists(const std::string& folderPath) {
    DWORD dwAttrib = GetFileAttributesA(folderPath.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void createFolder(const std::wstring& folderPath) {
    std::wstring stemp = std::wstring(folderPath.begin(), folderPath.end());
    LPCWSTR folderPathW = stemp.c_str();

    if (!CreateDirectoryW(folderPathW, NULL)) {

        //std::cerr << "Failed to create folder: " << folderPath << std::endl;
        exit(1);
    }
}

//record
void MainWindow::Screen_Record(int ret1 , bool start_record)
{
    int ret;

    QRadioButton* tmpRadioButton = qobject_cast<QRadioButton*>(toolDock->selectUIElement(UI_ToolDock::tool_rb_single));
    QPushButton* tmpPushButton = qobject_cast<QPushButton*>(toolDock->selectUIElement(UI_ToolDock::tool_bt_record));

    if(!start_record)
    {
        record_initialPath = appSettings.recordSetting.record_path;

        QDir mDir(record_initialPath);

        if (!mDir.exists() || record_initialPath == "")
        {
            toolDock->start_record = !toolDock->start_record;
            showMessage("Path ERROR","Unable to access the folder path. Please check the path or select again.");
            return;
        }
        QString filePath;
        if (tmpRadioButton) {
            if(tmpRadioButton->isChecked() == true)
            {
                filePath = record_initialPath + "/"+
                        appSettings.recordSetting.frontName +
                        QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss") +
                        appSettings.recordSetting.backName;

            }else
            {
                filePath = record_initialPath + "/" +
                               appSettings.recordSetting.frontName +
                               QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss") +
                               appSettings.recordSetting.backName + ".mp4";
            }
        }else
        {
            return;
        }
        if (filePath.isEmpty())
        {
            showMessage("Path ERROR","Unable to access the folder path. Please check the path or select again.");
            functionDock->setButton_Text(UI_FunctionDock::bt_record,"Start Recording");
            toolDock->setButton_Text(UI_ToolDock::tool_bt_record,"Start Record");
            toolDock->start_record = !toolDock->start_record;
            return;
        }

        start_Timer();

        functionDock->setButton_Disable(UI_FunctionDock::combo_mode,true);
        functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,true);
        functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,true);
        functionDock->setButton_Disable(UI_FunctionDock::bt_stream,true);

        toolDock->setButton_Disable(UI_ToolDock::tool_combo_mode,true);
        //toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,true);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,true);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,true);
        toolDock->setButton_Disable(UI_ToolDock::tool_rb_multi,true);
        toolDock->setButton_Disable(UI_ToolDock::tool_rb_single,true);

        tmpPushButton->setText("Stop Recording");
        redDot->setVisible(true);

        allSetting->setEnable_RTSP(!start_record);

        if (tmpRadioButton) {
            if(tmpRadioButton->isChecked() == true)
            {
                qDebug()<<"filePath："+filePath;
                std::wcout  << filePath.toStdWString() << std::endl;
                if (!folderExists(filePath.toStdString())) {
                   std::cout << "Folder does not exist. Creating folder..." << std::endl;
                   createFolder(filePath.toStdWString());
                   std::cout << "Folder created." << std::endl;
               } else {
                   std::cout << "Folder exists." << std::endl;
               }

                multiple_vid_ctx_param Cam_Video_param;
                Cam_Video_param.fps = appSettings.vidSetting.fps;
                Cam_Video_param.width = appSettings.vidSetting.width;
                Cam_Video_param.height = appSettings.vidSetting.height;
                Cam_Video_param.codec_id = appSettings.vidSetting.codec_id;

                Cam_Fun.set_Camera_videoPath(filePath.toStdString());
                Cam_Fun.set_Camera_videoSave(true);
            }else
            {
                ret = Cam_Fun.open_file(filePath.toStdString());
            }

        } else {
            // 轉換失敗，處理錯誤
        }


    }
    else
    {
        functionDock->setButton_Disable(UI_FunctionDock::combo_mode,false);
        functionDock->setButton_Disable(UI_FunctionDock::bt_RTSP,false);
        functionDock->setButton_Disable(UI_FunctionDock::bt_rotate,false);
        functionDock->setButton_Disable(UI_FunctionDock::bt_stream,false);

        toolDock->setButton_Disable(UI_ToolDock::tool_combo_mode,false);
        //toolDock->setButton_Disable(UI_ToolDock::tool_bt_RTSP,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_rotate,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_bt_stream,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_rb_multi,false);
        toolDock->setButton_Disable(UI_ToolDock::tool_rb_single,false);

        tmpPushButton->setText("Start Recording");
        redDot->setVisible(false);

        allSetting->setEnable_RTSP(!start_record);
        timer->stop();
        //Cam_Fun.set_Camera_videoSave(false);

        if (tmpRadioButton) {
            if(tmpRadioButton->isChecked () == true)
            {
                Cam_Fun.set_Camera_videoSave(false);
            }else
            {
                ret = Cam_Fun.close_file();
            }

        } else {
            // 轉換失敗，處理錯誤
        }
    }

    if(ret < 0)
    {
        printf("file error \n");
        return;
    }

    printf("file %d \n",start_record);
}

void MainWindow::Screen_Rotate()
{
    //Screen->m_pixmap->setRotation(90);

    if(status == AppStatus::Stream)
    {
        angle +=90;
        if(Stream_first)
        {
            if(angle == 360)
            {
                angle = 0;
            }
        }
    }
}

void MainWindow::set_SPID(std::vector<QString> ID_list)
{
    SPID_list.clear();
    bool tmpbl;
    qDebug()<<appSettings.aoFunSetting.SPCamList.size();
    if(ID_list.size()>0)
    {
        for(int i = 0 ; i<ID_list.size() ; i++)
        {
            for(int j =0 ; j<camList.size();j++)
            {
                tmpbl = false;
                if(ID_list[i]==camList[j].ip)
                {
                    SPID_list.push_back(camList[j].id);
                    tmpbl = true;
                    break;
                }
            }

            if(!tmpbl)
            {
                SPID_list.push_back(-1);
                tmpbl = false;
            }

        }
    }else
    {
        for(int i = 0 ; i<camList.size() ; i++)
        {
            SPID_list.push_back(camList[i].id);
        }

    }
}

void MainWindow::start_Timer()
{
    time = 0;

   timer->start(1000);
}

void MainWindow::getkeyboard(int getkey)
{
    key = getkey;
    Screen->set_onKeyPress(key);
    qDebug()<<key;
}
