#include "allsetting.h"
#include "ui_allsetting.h"
#include <QStringListModel>
#include <QDebug>
#include <QMessageBox>
#include <QDir>

Allsetting::Allsetting(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Allsetting)
{
    ui->setupUi(this);

    qRegisterMetaType<AVPixelFormat>("AVPixelFormat");

    ui->cb_camera->addItem("-1");

    selectView_init();

    InitalizeUI();
    connectSignals();

    Ao_Ui_init();

    setComponentItems();
    setStreamItem();
    setRTSPItem();
    hotKeyUI();
}

Allsetting::~Allsetting()
{
    delete ui;
}

//========================== ViewSetting ==========================
void Allsetting::selectView_init()
{
    QStringList list;

    list.append("Camera");
    list.append("Encode");
    list.append("Multi Screen");
    //list.append("RTSP");
    list.append("Output");
    list.append("Hotkeys");

    listmodel = new QStringListModel(list);

    ui->SelectView->setModel(listmodel);                   //设置模型到listview上
    ui->SelectView->setSpacing(2);                         //设置間距
    QModelIndex cameraIndex = listmodel->index(list.indexOf("Camera"), 0);
    ui->SelectView->setCurrentIndex(cameraIndex);
    ui->stackedWidget->setCurrentIndex(0);

    QObject::connect(ui->SelectView, &QListView::clicked, this, &Allsetting::selectView);
}

//========================= selectViewSetting =========================
void Allsetting::selectView(const QModelIndex &index)
{
    std::string selectedItem = listmodel->data(index, Qt::DisplayRole).toString().toStdString();
    changeWidget(selectedItem);
}

void Allsetting::changeWidget(std::string settingName)
{
    if(settingName == "Camera")
    {
        ui->stackedWidget->setCurrentIndex(0);
        widgetNum = 0;
    }
    else if(settingName == "Encode")
    {
        ui->stackedWidget->setCurrentIndex(1);
        widgetNum = 1;
    }
    else if(settingName == "Multi Screen")
    {
        ui->stackedWidget->setCurrentIndex(2);
        widgetNum = 2;
    }
    else if(settingName == "RTSP")
    {
        ui->stackedWidget->setCurrentIndex(3);
        widgetNum = 3;
    }
    else if(settingName == "Output")
    {
        ui->stackedWidget->setCurrentIndex(4);
        widgetNum = 4;
    }
    else if(settingName == "Hotkeys")
    {
        ui->stackedWidget->setCurrentIndex(5);
        widgetNum = 5;
    }
}

//========================== CameraSetting ==========================
void Allsetting::InitalizeUI()
{
    QList<QWidget *> allWidgets = this->findChildren<QWidget *>();

    // 設定所有元件的字體大小為18
    QFont font;
    font.setPointSize(18);

    for (QWidget *widget : allWidgets) {
        widget->setFont(font);
    }

//    ui->videoGroup->setLayout(ui->videoForm);
//    ui->imageGroup->setLayout(ui->imageForm);

    /// 新增UI選項
    all_Cam_UI_Setting set;
    //video
    ui->cb_Resolution->addItems(set.resolutionList);
    ui->cb_FrameRate->addItems(set.frameRateList);
    ui->sb_GoP->setRange(set.gopRange.min,set.gopRange.max);
    ui->cb_Quality->addItems(set.qualityList);
    //image
    ui->cb_ExposureTime->addItems(set.exposureTimeList);
    ui->cb_ISO->addItems(set.isoList);
    ui->cb_EV->addItems(set.evList);
    ui->cb_ACAREA->addItems(set.aeAreaList);
    ui->cb_WDR->addItems(set.wdrList);
    ui->cb_WB->addItems(set.wbList);
    ui->sb_WBCT->setRange(set.wbCT.min,set.wbCT.max);


    /// 帶入參數值
    setUIValue(setting);
    ///

    setWindowModality(Qt::WindowModal);
    //showMaximized();

}

void Allsetting::setUIValue(Cam_Set set, bool setIP)
{
//    Cam_Set tmp_set;
//    tmp_set = wabAPI->getParam();



    setting = set;

    if (setIP)
    {
        IPAddress = splitIPv4(setting.IP);
        if(IPAddress.size()==4)
        {
            ui->sb_IPA->setValue(IPAddress[0].toInt());
            ui->sb_IPB->setValue(IPAddress[1].toInt());
            ui->sb_IPC->setValue(IPAddress[2].toInt());
            ui->sb_IPD->setValue(IPAddress[3].toInt());
        }
    }

    //video
    ui->cb_Resolution->setCurrentText(setting.videoResolution);
    ui->cb_FrameRate->setCurrentText(setting.videoFrameRate);
    ui->sb_GoP->setValue(setting.videoGop);
    ui->cb_Quality->setCurrentText(setting.videoBitRate);
    //image
    ui->cb_ExposureTime->setCurrentText(setting.exposureTime);
    ui->cb_ISO->setCurrentText(setting.iso);
    ui->cb_EV->setCurrentText(setting.ev);
    ui->cb_ACAREA->setCurrentText(setting.ae);
    ui->cb_WDR->setCurrentText(setting.wdr);
    ui->cb_WB->setCurrentText(setting.wb);
    ui->sb_WBCT->setValue(setting.wbCT);
    setDisableByIP(setting.IP);

    ui->cb_mic->clear();
    ui->cb_mic->addItem("No Sound");


    for(int i = 0 ;i<camList.size();i++)
    {
        ui->cb_mic->addItem(camList[i].ip);
    }

    setMicSelect();

    //qDebug()<< "substring：" << substring;

    for(int i = 0 ;i<camList.size();i++)
    {
        int tmpNum;
        tmpNum = camList[i].ip.section(".",-1).toInt();

        int lastDotIndex = camList[i].ip.lastIndexOf('.');
        QString substring = camList[i].ip.left(lastDotIndex);

        qDebug()<<"tmp："<< ((tmpNum +1) - masterLast);
        qDebug()<<"setting.micSelect："<<setting.micSelect;
        qDebug()<<"setting"<< setting.micSelect.section(".",-1);

        if(setting.micSelect.section(".",-1) == QString::number((tmpNum +1) - masterLast))
        {
            qDebug()<< "substring：" << (tmpNum + 1) - masterLast;
            int tmpnum = (tmpNum + 1) - masterLast;
            QString tmpStr = substring + "." + QString::number(tmpnum);
            qDebug()<< "substring：" <<tmpStr;

//            if(tmp_set.micSelect != setting.micSelect)
//            {
//                ui->cb_mic->setCurrentIndex(0);
//                cam_set_save();
//                qDebug()<<"111111111";
//            }else
//            {
//                qDebug()<<"222222222";
//            }

            ui->cb_mic->setCurrentText(tmpStr);

        }
    }



//    set_VideoSettings_Disable(isStreaming);
//    set_ImageSettings_Disable(true);
}

void Allsetting::on_cb_mic_currentIndexChanged(int index)
{
//    if(index == 0)
//    {
//        setting.micSelect = "0";
//    }else
//    {
//        setting.micSelect = QString::number(ui->cb_mic->currentText().section(".",-1).toInt() - masterLast);
//    }
}

QStringList Allsetting::splitIPv4(const QString &ip)
{
    QStringList IP_List;
    if (isValidIPv4(ip)) {
        QStringList parts = ip.split('.');
        for (const QString &part : parts) {
            IP_List << part;
        }
    }
    return IP_List;
}

bool Allsetting::isValidIPv4(const QString &ip)
{
    QRegularExpression ipRegex(
        "^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$"
    );

    return ipRegex.match(ip).hasMatch();
}

void Allsetting::on_bt_cs_clicked()
{
    //取消
    this->close();
}

Cam_Set Allsetting::get_UI_set()
{
    Cam_Set set;
    set.IP = packIP();
    //video
    set.videoResolution = ui->cb_Resolution->currentText();
    set.videoFrameRate = ui->cb_FrameRate->currentText();
    set.videoGop = ui->sb_GoP->value();
    set.videoBitRate = ui->cb_Quality->currentText();

    //Audio
    if(ui->cb_mic->currentIndex() == 0)
    {
        setting.micSelect = "0";
    }else
    {
        set.micSelect = QString::number(ui->cb_mic->currentText().section(".",-1).toInt() - masterLast + 1);
    }

    //Image
    set.exposureTime = ui->cb_ExposureTime->currentText();
    set.iso = ui->cb_ISO->currentText();
    set.ev = ui->cb_EV->currentText();
    set.ae = ui->cb_ACAREA->currentText();
    set.wdr = ui->cb_WDR->currentText();
    set.wb = ui->cb_WB->currentText();
    set.wbCT = ui->sb_WBCT->value();
    return set;
}

QString Allsetting::packIP()
{
    IPAddress.clear();
    IPAddress.append(QString::number(ui->sb_IPA->value()));
    IPAddress.append(QString::number(ui->sb_IPB->value()));
    IPAddress.append(QString::number(ui->sb_IPC->value()));
    IPAddress.append(QString::number(ui->sb_IPD->value()));

    return IPAddress.join('.');
}

void Allsetting::connectSignals()
{

    connect(ui->actionExport_Param, &QAction::triggered, this, [=](){
        switch (widgetNum)
        {
        case 0:
        {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.camset)"));
            if (fileName.isEmpty()) return;
            wirteParamFile(get_UI_set().toObj(), fileName);
            break;
        }
        case 1:
        {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.enc)"));
            if (fileName.isEmpty()) return;

            set_param();
            wirteParamFile(encode_param.toObj(), fileName);
            break;
        }
        case 2:
        {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.aoset)"));
            if (fileName.isEmpty()) return;

            wirteParamFile(get_UI_set().toObj(), fileName);
            break;
        }
        default:
        showMessage("Error","save error.");
        break;
        }

    });

    connect(ui->actionImport_Param, &QAction::triggered, this, [=]() {
        switch (widgetNum)
        {
        case 0:
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open txt"), "", "cam setting (*.camset )");
            if (fileName.isEmpty()) return;

            QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
            setUIValue(Cam_Set::toStruct(JDoc.object()), false);
            break;
        }
        case 1:
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open txt"), "", "cam focus (*.enc )");
            if (fileName.isEmpty()) return;

            QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
            encode_param = Encoder_param::toStruct(JDoc.object());
            set_init_param(encode_param, 0, true);
            break;
        }
        case 2:
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open .aoset"), "", "cam setting (*.aoset )");
            if (fileName.isEmpty()) return;

            QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
            AO_setUIValue(AoFunSetting::toStruct(JDoc.object()));
            break;
        }
        default:
        showMessage("Error","open error.");
        break;
        }
    });

    connect(ui->actionDefault_Param, &QAction::triggered, this, [=]() {
        switch (widgetNum)
        {
        case 0:
        {
            QMessageBox::StandardButton result;
            result = QMessageBox::question(this, "Confirmation", "Are you sure you want to revert to default?", QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
                setUIValue(Cam_Set(), false);
            break;
        }
        case 1:
        {
            QMessageBox::StandardButton result;
            result = QMessageBox::question(this, "Confirmation", "Are you sure you want to revert to default?", QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
            {
                encode_param = Encoder_param();
                set_init_param(encode_param, 0, true);
            }
            break;
        }
        case 2:
        {
            QMessageBox::StandardButton result;
            result = QMessageBox::question(this, "Confirmation", "Are you sure you want to revert to default?", QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes)
                AO_setUIValue(AoFunSetting());
            break;
        }
        default:
        showMessage("Error","open error.");
        break;
        }

    });

    connect(this, &Allsetting::cantGetResult, [=](){
        // 創建一個 QMessageBox
        showMessage("Error", "Can't set this camera.");
    });

    connect(ui->cb_WB, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        // 判斷是否選擇了最後一個選項
        needSetWBCT = (index == ui->cb_WB->count() - 1);
        ui->sb_WBCT->setDisabled(!needSetWBCT);
    });
}

void Allsetting::cam_set_save()
{
    Cam_Set set = get_UI_set();

    //check IP is legal.
    if(!isValidIPv4(set.IP))
    {
        showMessage("Error", "Can't set this camera.");
        ui->bt_save->setDisabled(false);
        return;
    }

    //check state's Stream or unStream.
    if(!setDisableByIP(set.IP))
    {
        showMessage("Error", "Can't set this camera.");
        return;
    }

    QJsonObject obj = set.toObj(isStreaming);
    ui->bt_save->setDisabled(true);

    wabAPI->setIP(set.IP);

    if(wabAPI->scanCamIP(set.IP) != 1)
    {
        QMetaObject::invokeMethod(this, [this]() {
            ui->bt_save->setDisabled(false);
        });
        return;
    }

//    writeParam(set);

    auto analyze = [=](int ret, QString key){
        if (ret == state::done)
        {
            qDebug() << key << ": Set.";
        }else if (ret == state::inProgress)
        {
            int id = wabAPI->get_ProgressID();
            int timeCount = 0;
            while(!wabAPI->checkStatus(id))
            {
                timeCount++;
                if(timeCount>=5)
                {
                    qDebug() << "error: " + key + " - timeOut.";
                    return;
                }
                Sleep(1000);
            }

            qDebug() << key << ": Set.";
        }else
        {
            qDebug() << "error:" + QString::number(ret);
        }
    };

    //waitting for check (using ststusID)
    std::thread([=]()
    {
        QStringList keys = obj.keys();
        for (const QString& key : keys)
        {
            if(key == "IP")
                continue;
            if(!needSetWBCT && key == "wbCT")
                continue;

            QJsonValue value = obj.value(key);
            if (value.isDouble())
                analyze(wabAPI->settingParam2(key, value.toInt()), key);
            else if (value.isString())
                analyze(wabAPI->settingParam1(key, value.toString()), key);
            else {
                qDebug() << key << "(unknown type)";
            }
            Sleep(100);
        }

        int i = wabAPI->saveParam();

        if(i == state::done)
        {
            qDebug() << "success save.";
            emit returnParam(set);
            QMetaObject::invokeMethod(this, [this]() {
                ui->bt_save->setDisabled(false);
            });
        }
        else
        {
            qDebug() << "fail save.";
            qDebug()<< "error: setting fail.";
            QMetaObject::invokeMethod(this, [this]() {
                ui->bt_save->setDisabled(false);
            });
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    showMessage("Save","Save completed");

//    qDebug()<<wabAPI->test2();
}

void Allsetting::on_bt_save_clicked()
{
    cam_set_save();

}

bool Allsetting::setDisableByIP(QString IP)
{
    auto setSaveDisable = [=](bool bl){
        ui->bt_save->setDisabled(bl);
    };
    switch (wabAPI->checkState(IP)) {
    case 0: // unstream
        isStreaming = false;
        set_VideoSettings_Disable(false);
        set_ImageSettings_Disable(false);
        setSaveDisable(false);
        return true;
    case 1: // stream
        isStreaming = true;
        set_VideoSettings_Disable(true);
        set_ImageSettings_Disable(false);
        setSaveDisable(false);
        return true;
    default: // else
        isStreaming = false;
        set_VideoSettings_Disable(true);
        set_ImageSettings_Disable(true);
        setSaveDisable(true);
        return false;
    }

}

void Allsetting::set_VideoSettings_Disable(bool isStream){
    ui->cb_Resolution->setDisabled(isStream);
    ui->cb_FrameRate->setDisabled(isStream);
    ui->sb_GoP->setDisabled(isStream);
    ui->cb_Quality->setDisabled(isStream);
}

void Allsetting::set_ImageSettings_Disable(bool isHide){
    ui->cb_ExposureTime->setDisabled(isHide);
    ui->cb_ISO->setDisabled(isHide);
    ui->cb_EV->setDisabled(isHide);
    ui->cb_ACAREA->setDisabled(isHide);
    ui->cb_WDR->setDisabled(isHide);
    ui->cb_WB->setDisabled(isHide);

    if(isHide)
    {
        ui->sb_WBCT->setDisabled(isHide);
    }else
    {
        needSetWBCT = (ui->cb_WB->currentIndex() == ui->cb_WB->count() - 1);
        ui->sb_WBCT->setDisabled(!needSetWBCT);
    }
}

void Allsetting::setMicSelect()
{
    if(masterCam == "")
    {
        qDebug()<<" masterCam= null";
        return;
    }
    QString lastNum = masterCam.section('.', -1);
    masterLast = lastNum.toInt();
    qDebug()<< "lastNum："<< lastNum;
}
//========================= EncoderSetting ==========================
void Allsetting::setComponentItems()
{
    ui->cb_Encoder->addItem("h264_nvenc", Encoder_class_format::Encoder_class_h264_nvenc);
    ui->cb_Encoder->addItem("hevc_nvenc", Encoder_class_format::Encoder_class_hevc_nvenc);

    ui->cb_preset->addItem(tr("Max Quality"), Encoder_class_preset::Encoder_class_mq);
    ui->cb_preset->addItem(tr("Quality"), Encoder_class_preset::Encoder_class_hq);
    ui->cb_preset->addItem(tr("Max Performance"), Encoder_class_preset::Encoder_class_hp);
    ui->cb_preset->addItem(tr("Performance"), Encoder_class_preset::Encoder_class_default);
    ui->cb_preset->addItem(tr("Low Latency"), Encoder_class_preset::Encoder_class_ll);
    ui->cb_preset->addItem(tr("Low Latency Quality"), Encoder_class_preset::Encoder_class_llhq);
    ui->cb_preset->addItem(tr("Low Latency Performance"), Encoder_class_preset::Encoder_class_llhp);

    ui->cb_profile->addItem(tr("Highest Image Quality"), Encoder_class_profile::Encoder_class_high);
    ui->cb_profile->addItem(tr("Mainstream Image Quality"), Encoder_class_profile::Encoder_class_main);
    ui->cb_profile->addItem(tr("Basic Image Quality"), Encoder_class_profile::Encoder_class_baseline);


    ui->sb_ImageW->setRange(1920, 3840);
    ui->sb_ImageH->setRange(1080, 2160);
    ui->sb_FPS->setRange(25, 120);
    ui->sb_GOP->setRange(30, 250);
    ui->sb_bitrate->setRange(2500, 100000);

    ui->cb_pixel->addItem("NV12", AVPixelFormat::AV_PIX_FMT_NV12);
//    ui->pixelFormatComboBox->addItem("YUV420P", AVPixelFormat::AV_PIX_FMT_YUV420P);

    setDefault();

    // disable
    ui->cb_Encoder->setDisabled(true);
    ui->sb_ImageW->setDisabled(true);
    ui->sb_ImageH->setDisabled(true);
    ui->cb_pixel->setDisabled(true);
}

void Allsetting::on_bt_en_bt_clicked()
{
    //取消
    this->close();
}

void Allsetting::on_bt_getSet_clicked()
{
    getset();
}

void Allsetting::on_bt_en_ss_clicked()
{
    if(!set_param())
        return;

    showMessage("Save","Save completed");

    emit sentEncoderSetting(encode_param);
    close();
}

void Allsetting::setEnable_RTSP(bool bl)
{
    ui->cb_preset->setDisabled(bl);
    ui->cb_profile->setDisabled(bl);
    ui->sb_FPS->setDisabled(bl);
    ui->sb_GOP->setDisabled(bl);
    ui->sb_maxbframe->setDisabled(bl);
    ui->sb_bitrate->setDisabled(bl);
}

void Allsetting::getset()
{
    ui->bt_getSet->setDisabled(true);
    std::thread([this](){
        Cam_Set set = get_UI_set();
        QJsonObject obj = set.toObj();
        wabAPI->setIP(set.IP);

        if(wabAPI->scanCamIP(packIP()) != 1)
        {
            QMetaObject::invokeMethod(this, [this]() {
                showMessage("Error", "Can't set this camera.");
                ui->bt_getSet->setDisabled(false);
            });
            return;
        }
        wabAPI->checkState(packIP());

        setting = wabAPI->getParam();

        setting.IP = set.IP;
    //    setting
        QMetaObject::invokeMethod(this, [this]() {
            setUIValue(setting);
            ui->bt_getSet->setDisabled(false);
        });
    }).detach();
}

//========================= En設定預設參數 ============================
void Allsetting::set_init_param(Encoder_param param, int setFrameCount, bool isEnable)
{
    int codecComboBoxCount = ui->cb_Encoder->count();

    int pixelFormatComboBoxCount = ui->cb_pixel->count();
    for (int i = 0; i < pixelFormatComboBoxCount; i ++) {
        if(param.pix_format == qvariant_cast<AVPixelFormat>(ui->cb_pixel->itemData(i))){
            ui->cb_pixel->setCurrentIndex(i);
            break;
        }
    }
    //TODO setby param
    ui->sb_ImageW->setValue(param.width);
    ui->sb_ImageH->setValue(param.height);
    ui->sb_FPS->setValue(param.fps);
    ui->sb_GOP->setValue(param.gop);
    ui->sb_bitrate->setValue(param.bitrate / 1000);
    ui->cb_preset->setCurrentIndex(param.preset);
    ui->cb_profile->setCurrentIndex(param.profile);
    ui->sb_maxbframe->setValue(param.max_b_frame);
}

void Allsetting::setDefault()
{
    ui->cb_Encoder->setCurrentIndex(0);

    ui->sb_ImageW->setValue(1920);
    ui->sb_ImageH->setValue(1080);
    ui->sb_FPS->setValue(60);
    ui->sb_GOP->setValue(60);
    ui->sb_bitrate->setValue(3000);
    ui->cb_pixel->setCurrentIndex(0);
}

bool Allsetting::set_param()
{
    encode_param.width = ui->sb_ImageW->text().toInt();
    encode_param.height = ui->sb_ImageH->text().toInt();
    encode_param.fps = ui->sb_FPS->text().toInt();
    encode_param.gop = ui->sb_GOP->text().toInt();
    encode_param.bitrate = ui->sb_bitrate->text().toInt() * 1000;
    encode_param.pix_format = qvariant_cast<AVPixelFormat>(ui->cb_pixel->currentData());
    return true;
}

//========================= AoSetting =========================
void Allsetting::Ao_Ui_init()
{
    ui->cb_SP->addItem("4K_4×3");
    ui->cb_SP->addItem("4K_3×3");
    ui->cb_SP->addItem("4K_3×2");
    ui->cb_SP->addItem("4K_2×2");

    ui->cb_rot->addItem("0", -1);
    ui->cb_rot->addItem("180", 0);
    ui->cb_rot->addItem("垂直翻轉", 1);
    ui->cb_rot->addItem("水平翻轉", 2);

    QStringList tmpList = ui->cb_SP->currentText().split("_");

    rowcolList = tmpList[1].split("×");
    add_rowcol_combo();

    customImageWidget = new CustomImageWidget(rowcolList[1].toInt(),rowcolList[0].toInt());
     //customImageWidget->setGeometry(0, 0, 470, 300);
//    customImageWidget->setLayout(ui->verticalLayout_5);
    ui->verticalLayout_5->addWidget(customImageWidget);
//    ui->widget_2 = customImageWidget;
    QObject::connect(customImageWidget, &CustomImageWidget::onSelect, [&](int row, int col) {
        ui->cb_row->setCurrentText(QString::number(col+1));
        ui->cb_col->setCurrentText(QString::number(row+1));

    });

}

void Allsetting::wirteParamFile(QJsonObject content, QString fileName)
{
    QFile file(fileName);
    QJsonDocument json_doc(content);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << json_doc.toJson() << endl;
    file.close();
}

void Allsetting::on_bt_AO_cs_clicked()
{
    //取消
    this->close();
}

//ReadFileToString
QString Allsetting::readFileToString(QString name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QTextStream in(&file);
    QString JsonString = in.readAll();//error
    file.close();
    return JsonString;
}

void Allsetting::AO_setUIValue(AoFunSetting set)
{
    aoSetting = set;
    //ui->le_RTSP_2->setText(aoSetting.rtsp_path);
    ui->cb_SP->setCurrentIndex(aoSetting.SP_mode);

    qDebug()<<"----";
    qDebug()<<aoSetting.SPCamList;
}

AoFunSetting Allsetting::Ao_get_UI_set()
{
    AoFunSetting set;
    //set.rtsp_path = ui->le_RTSP_2->text();

    return set;
}

void Allsetting::showMessage(QString Title, QString msg)
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(Title);
    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton("OK", QMessageBox::AcceptRole);
    msgBox->exec();
}

void Allsetting::on_bt_AO_ss_clicked()
{
    aoSetting.SP_mode = static_cast<SP_MODE>(ui->cb_SP->currentIndex());


    aoSetting.SPCamList = c_ip;

    showMessage("Save","Save completed");

    emit c_id_signal(c_id);
    emit confirmBtn_signal(aoSetting);
}

void Allsetting::add_rowcol_combo()
{
    ui->cb_row->setCurrentText("");
    ui->cb_col->setCurrentText("");
    ui->cb_row->clear();
    ui->cb_col->clear();


    for(int i=1 ;i<=rowcolList[0].toInt();i++)
    {
        ui->cb_row->addItem(QString::number(i));
    }

    for(int i=1 ;i<=rowcolList[1].toInt();i++)
    {
        ui->cb_col->addItem(QString::number(i));
    }
}

void Allsetting::setSPID_list(std::vector<uint32_t> SPID_list, QVector<CamInfo> tmpList)
{
    for(int i=0 ; i<SPID_list.size() ; i++)
    {
        qDebug()<<SPID_list[i];
    }
    for(int i=0 ; i<tmpList.size() ; i++)
    {
        qDebug()<<tmpList[i].ip;
    }
    c_ip.clear();
    c_id.clear();
    camList.clear();

    c_id = SPID_list;
    camList = tmpList;

    ui->cb_camera->clear();
    ui->cb_camera->addItem("-1");

    for(int i=0 ; i<camList.size() ; i++)
    {
        ui->cb_camera->addItem(tmpList[i].ip);
    }

    int tmpNum = rowcolList[0].toInt()*rowcolList[1].toInt()-c_id.size();
    for(int i =0 ;i<tmpNum;i++)
    {
        c_id.push_back(-1);
    }

    int tmpbl;
    for(int i =0;i<c_id.size();i++)
    {
        tmpbl = false;
        for(int j=0;j<camList.size();j++)
        {
            if(c_id[i]==camList[j].id)
            {
                c_ip.push_back(camList[j].ip);

                tmpbl = true;
                break;
            }
        }
        if(!tmpbl)
        {
            c_ip.push_back("-1");
        }
    }

    set_cb_Camera();
}

void Allsetting::on_cb_camera_currentTextChanged(const QString &arg1)
{
    if(c_id.size() <= 0 || c_ip.size() <= 0)
    {
        return;
    }
    int row = ui->cb_row->currentText().toInt();
    int col = ui->cb_col->currentText().toInt();

    int num = (col-1)*rowcolList[0].toInt()+row-1;

    bool removeCam =false;

    for(int i =0;i<camList.size();i++)
    {
        if(ui->cb_camera->currentText()==camList[i].ip)
        {
            c_id[num] = camList[i].id;
            c_ip[num] = camList[i].ip;
            removeCam = true;
        }
    }

    if(removeCam == false)
    {
        c_id[num] = -1;
        c_ip[num] = "-1";
    }
    qDebug()<<c_id;
    qDebug()<<c_ip;
}

void Allsetting::on_cb_row_currentTextChanged(const QString &arg1)
{
    set_cb_Camera();
}

void Allsetting::on_cb_col_currentTextChanged(const QString &arg1)
{
    set_cb_Camera();
}

void Allsetting::on_cb_SP_currentTextChanged(const QString &arg1)
{
    QStringList tmpList = ui->cb_SP->currentText().split("_");
    rowcolList = tmpList[1].split("×");

    c_id.resize(rowcolList[0].toInt()*rowcolList[1].toInt(),-1);
    c_ip.resize(rowcolList[0].toInt()*rowcolList[1].toInt(),"-1");
    add_rowcol_combo();

    if (customImageWidget)
        customImageWidget->setRowCol(rowcolList[1].toInt(),rowcolList[0].toInt());

}

void Allsetting::set_cb_Camera()
{
    int row = ui->cb_row->currentText().toInt();
    int col = ui->cb_col->currentText().toInt();

    if(customImageWidget)
        customImageWidget->selectScreen(col-1,row-1);

    bool tmpbl = false;
    if(row>0 && row<= rowcolList[0] && col>0 && col <= rowcolList[1])
    {
        int num = (col-1)*rowcolList[0].toInt()+row-1;
        tmpbl = false;
        for(int i =0;i<camList.size();i++)
        {
            if(c_id[num] == camList[i].id)
            {
                ui->cb_camera->setCurrentText(camList[i].ip);
                tmpbl = true;
                break;
            }
        }
        if(tmpbl == false)
        {
            ui->cb_camera->setCurrentText("-1");
        }
    }
}

void Allsetting::on_bt_ao_clean_clicked()
{
    for(int i =0;i<c_id.size();i++)
    {
        c_id[i]=-1;
        c_ip[i]="-1";
    }

    ui->cb_row->setCurrentText("1");
    ui->cb_col->setCurrentText("1");

    customImageWidget->selectScreen(0,0);

    //aoSetting.rtsp_path = ui->le_RTSP_2->text();
    aoSetting.SP_mode = static_cast<SP_MODE>(ui->cb_SP->currentIndex());
    aoSetting.SPCamList = c_ip;

    showMessage("cleam","Clean all Screen!");

    emit c_id_signal(c_id);
    emit confirmBtn_signal(aoSetting);
}
//========================= tool ============================
QObject* Allsetting::selectUIElement(UI_allSetting allSetting)
{
    switch (allSetting) {

    case bt_cam_Resolution:
        return ui->cb_Resolution;

    case bt_cam_FrameRate:
        return ui->cb_FrameRate;

    case bt_cam_GoP:
        return ui->sb_GoP;

    case bt_cam_Quality:
        return ui->cb_Quality;

    case bt_cam_ISO:
        return ui->cb_ISO;

    case bt_cam_EV:
        return  ui->cb_ISO;

    case bt_cam_AEAREA:
        return  ui->cb_ACAREA;

    case bt_cam_WDR:
        return  ui->cb_WDR;

    case bt_cam_WB:
        return  ui->cb_WB;

    case bt_cam_WBCT:
        return  ui->sb_WBCT;

    case bt_enc_Encoder:
        return  ui->cb_Encoder;

    case bt_enc_preset:
        return ui->cb_preset;

    case bt_enc_profile:
        return  ui->cb_profile;

    case bt_enc_ImageW:
        return  ui->sb_ImageW;

    case bt_enc_ImageH:
        return  ui->sb_ImageH;

    case bt_enc_FPS:
        return  ui->sb_FPS;

    case bt_enc_GOP:
        return  ui->sb_GOP;

    case bt_enc_Bitrate:
        return  ui->sb_bitrate;

    case bt_enc_Pixel:
        return  ui->cb_pixel;

    case bt_enc_maxbframe:
        return  ui->sb_maxbframe;

    case bt_ao_RTSP:
        return  ui->le_RTSP_2;

    case bt_ao_SPmode:
        return  ui->cb_SP;

    case bt_ao_row:
        return  ui->cb_row;

    case bt_ao_col:
        return  ui->cb_col;

    case bt_ao_camera:
        return  ui->cb_camera;
    }
}

void Allsetting::setButton_Disable(UI_allSetting allSetting, bool bl)
{
    QPushButton* temp = qobject_cast<QPushButton*>(selectUIElement(allSetting));
    if (temp) {
        temp->setDisabled(bl);
    }
}

void Allsetting::on_cb_preset_currentTextChanged(const QString &arg1)
{
    switch (ui->cb_preset->currentIndex()) {
    case 0:
        encode_param.preset=Encoder_class_preset::Encoder_class_mq;
        break;
    case 1:
        encode_param.preset=Encoder_class_preset::Encoder_class_hq;
        break;
    case 2:
        encode_param.preset=Encoder_class_preset::Encoder_class_hp;
        break;
    case 3:
        encode_param.preset=Encoder_class_preset::Encoder_class_default;
        break;
    case 4:
        encode_param.preset=Encoder_class_preset:: Encoder_class_ll;
        break;
    case 5:
        encode_param.preset=Encoder_class_preset::Encoder_class_llhq;
        break;
    case 6:
        encode_param.preset=Encoder_class_preset::Encoder_class_llhp;
        break;
    }
}

void Allsetting::on_cb_profile_currentTextChanged(const QString &arg1)
{
    switch (ui->cb_profile->currentIndex()) {
    case 0:
        encode_param.profile=Encoder_class_profile::Encoder_class_baseline;
        break;
    case 1:
        encode_param.profile=Encoder_class_profile::Encoder_class_main;
        break;
    case 2:
        encode_param.profile=Encoder_class_profile::Encoder_class_high;
        break;

    }
}

void Allsetting::clearLayout(QLayout *layout) {
    QLayoutItem *item;
    if (layout)
    {
        while ((item = layout->takeAt(0)) != nullptr) {
            if (QWidget *widget = item->widget()) {
                // 如果是 widget，刪除 widget
                widget->deleteLater();
            } else if (QLayout *childLayout = item->layout()) {
                // 如果是子佈局，遞迴刪除子佈局中的元件
                clearLayout(childLayout);
            }
            delete item;
        }
    }
}

//========================= Stream =========================
void Allsetting::setStreamItem()
{
    ui->cb_time->addItem("limited");
    ui->cb_time->addItem("unlimited");
}

void Allsetting::on_pb_Browse_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder");

    if(folderPath == "")
    {
        return;
    }

    ui->le_path->setText(folderPath);
    qDebug()<<folderPath;
}

void Allsetting::setRecordUI(Record_param set)
{
    recordSetting = set;
    QStringList tmpList = (recordSetting.record_path).split("/");
    path = "";
    fileName = "";

    for(int i =0;i<tmpList.size();i++)
    {
        path += tmpList[i];
        if(i!=tmpList.size()-1)
        {
            path += "/";
        }
    }

//    fileName = recordSetting.frontName.toStdString() +
//            QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss").toStdString() +
//            recordSetting.backName.toStdString()+".mp4";

    ui->le_path->setText(path);
    ui->le_n1->setText(recordSetting.frontName);
    ui->le_n2->setText(recordSetting.backName);
    ui->sb_time->setValue(recordSetting.sec);
    ui->cb_time->setCurrentIndex(recordSetting.record_type);
    QRegularExpression re("\\d+");
    QRegularExpressionMatch match = re.match(setting.videoFrameRate);

    if (match.hasMatch()) {
        QString numberStr = match.captured(0);
        bool ok;
        int number = numberStr.toInt(&ok);
        if (ok) {
            ui->spinBox_FPS->setValue(number);
        } else {
            qDebug() << "無法轉換為整數";
        }
    } else {
        qDebug() << "未找到匹配的數字";
    }
}

void Allsetting::on_pb_record_ss_clicked()
{
    QStringList tmpstr;

    for(int i =0;i< ui->le_n1->text().length();++i)
    {
        tmpstr << ui->le_n1->text().mid(i,1);
    }

    for(int i =0;i<tmpstr.size();i++)
    {
        for(int j=0;j<error_file_name.size();j++)
        {
            if(tmpstr[i]==error_file_name[j])
            {
                qDebug()<<"error file name";
                return;
            }
        }
    }

    tmpstr.clear();
    for(int i =0;i< ui->le_n2->text().length();++i)
    {
        tmpstr << ui->le_n2->text().mid(i,1);
    }

    for(int i =0;i<tmpstr.size();i++)
    {
        for(int j=0;j<error_file_name.size();j++)
        {
            if(tmpstr[i]==error_file_name[j])
            {
                qDebug()<<"error file name";
                return;
            }
        }
    }
    recordSetting.record_path = ui->le_path->text();
    recordSetting.frontName = ui->le_n1->text();
    recordSetting.backName = ui->le_n2->text();
    recordSetting.sec = ui->sb_time->value();
    recordSetting.record_type = ui->cb_time->currentIndex();
    //RTSP

    if(ui->cb_service->currentIndex()==0)
    {
        RTSPSetting.rtsp_path = ui->le_RTSP_2->text();
    }else
    {
        RTSPSetting.rtmp_path = ui->le_RTSP_2->text();
    }

    RTSPSetting.service = ui->cb_service->currentIndex();

    qDebug()<<"ui->cb_time->currentIndex()："<<ui->cb_time->currentIndex();
    qDebug()<<"RTSPSetting.service："<<RTSPSetting.service;

    emit RTSP_signal(RTSPSetting);
    emit record_signal(recordSetting);
    emit onFPSChange(stream_FPS);

    showMessage("Save","Save completed");

    //取消
    this->close();
}

void Allsetting::on_pb_record_cancel_clicked()
{
    //取消
    this->close();
}

void Allsetting::on_spinBox_FPS_valueChanged(int arg1)
{
    stream_FPS = arg1;
}
//========================= RTSP =========================
void Allsetting::setRTSPItem()
{
    ui->cb_service->addItem("RTSP");
    ui->cb_service->addItem("RTMP");
}

void Allsetting::set_RTSP_value(RTSP_param set)
{
    RTSPSetting = set;
    ui->cb_service->setCurrentIndex(set.service);
    if(set.service==0)
    {
        ui->le_RTSP_2->setText(RTSPSetting.rtsp_path);
    }else
    {
        ui->le_RTSP_2->setText(RTSPSetting.rtmp_path);
    }
}

void Allsetting::on_bt_RTSP_save_clicked()
{
    if(ui->cb_service->currentIndex()==0)
    {
        RTSPSetting.rtsp_path = ui->le_RTSP_2->text();
    }else
    {
        RTSPSetting.rtmp_path = ui->le_RTSP_2->text();
    }

    RTSPSetting.service = ui->cb_service->currentIndex();
    qDebug()<<"RTSPSetting.service："<<RTSPSetting.service;
    showMessage("Save","Save completed");

    emit RTSP_signal(RTSPSetting);
    this->close();
}

void Allsetting::on_bt_RTSP_cancel_clicked()
{
    this->close();
}

void Allsetting::on_cb_time_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0:
        {
            ui->sb_time->setValue(recordSetting.sec);
            ui->sb_time->setEnabled(true);
            ui->sb_time->setVisible(true);
            ui->label_4->setVisible(true);
            ui->label_5->setVisible(true);
            break;
        }
        case 1:
        {
            ui->sb_time->setValue(0);
            ui->sb_time->setEnabled(false);
            ui->sb_time->setVisible(false);
            ui->label_4->setVisible(false);
            ui->label_5->setVisible(false);
            break;
        }
    }
}

void Allsetting::on_cb_service_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0:
        {
            ui->le_RTSP_2->setText(RTSPSetting.rtsp_path);
            break;
        }
        case 1:
        {
            ui->le_RTSP_2->setText(RTSPSetting.rtmp_path);
            break;
        }
    }
}

void Allsetting::on_bt_Reset_clicked()
{
    if(camList.size() > c_id.size())
    {
        for(int i=0;i<camList.size();i++)
        {
            if(i < c_id.size())
            {
                c_id[i] = camList[i].id;
                c_ip[i] = camList[i].ip;
            }
        }
    }else
    {
        for(int i =0 ;i<c_id.size();i++)
        {
            if(i < camList.size())
            {
                c_id[i] = camList[i].id;
                c_ip[i] = camList[i].ip;
            }else
            {
                c_id[i] = -1;
                c_ip[i] = "-1";
            }
        }
    }
    set_cb_Camera();

    aoSetting.SP_mode = static_cast<SP_MODE>(ui->cb_SP->currentIndex());


    aoSetting.SPCamList = c_ip;

    showMessage("Save","Reset completed");

    emit c_id_signal(c_id);
    emit confirmBtn_signal(aoSetting);
}

//========================= Hotkey =========================
void Allsetting::hotKeyUI()
{
    ui->le_startRecord->setText("F9");
    ui->le_stopRecord->setText("F9");
    ui->le_fullPro->setText("F10");
    ui->le_fullscreen->setText("F11");

    ui->le_startRecord->setEnabled(false);
    ui->le_stopRecord->setEnabled(false);
    ui->le_fullPro->setEnabled(false);
    ui->le_fullscreen->setEnabled(false);
}
