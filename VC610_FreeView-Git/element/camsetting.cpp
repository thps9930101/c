#include "camsetting.h"
#include "ui_camsetting.h"


CamSetting::CamSetting(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CamSetting)
{
    ui->setupUi(this);
    checkDir();
//    setting = readParam();
    InitalizeUI();
    connectSignals();
}

void CamSetting::InitalizeUI()
{
    QList<QWidget *> allWidgets = this->findChildren<QWidget *>();

    // 設定所有元件的字體大小為18
    QFont font;
    font.setPointSize(18);

    for (QWidget *widget : allWidgets) {
        widget->setFont(font);
    }

    ui->videoGroup->setLayout(ui->videoForm);
    ui->imageGroup->setLayout(ui->imageForm);

    /// 新增UI選項
    Cam_UI_Setting set;
    //video
    ui->cb_Resolution->addItems(set.resolutionList);
    ui->cb_FrameRate->addItems(set.frameRateList);
    ui->sb_GoP->setRange(set.gopRange.min,set.gopRange.max);
    ui->cb_Quality->addItems(set.qualityList);
    //image
    ui->cb_ExposureTime->addItems(set.exposureTimeList);
    ui->cb_ISO->addItems(set.isoList);
    ui->cb_EV->addItems(set.evList);
    ui->cb_AEArea->addItems(set.aeAreaList);
    ui->cb_WDR->addItems(set.wdrList);
    ui->cb_WB->addItems(set.wbList);
    ui->sb_wbCT->setRange(set.wbCT.min,set.wbCT.max);


    /// 帶入參數值
    setUIValue(setting);
    ///
    content = new QWidget(this);
    content->setLayout(ui->mainLayout);
    setCentralWidget(content);
    //showMaximized();

}
void CamSetting::connectSignals()
{

    connect(ui->actionExport_Param, &QAction::triggered, this, [=](){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.camset)"));
        if (fileName.isEmpty()) return;

        wirteParamFile(get_UI_set().toObj(), fileName);
    });

    connect(ui->actionImport_Param, &QAction::triggered, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open txt"), "", "cam setting (*.camset )");
        if (fileName.isEmpty()) return;

        QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
        setUIValue(Cam_Set::toStruct(JDoc.object()), false);
    });

    connect(ui->actionDefault_Param, &QAction::triggered, this, [=]() {
        QMessageBox::StandardButton result;
        result = QMessageBox::question(this, "Confirmation", "Are you sure you want to revert to default?", QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
            setUIValue(Cam_Set(), false);
    });

    connect(this, &CamSetting::cantGetResult, [=](){
        // 創建一個 QMessageBox
        showMessage("Error", "Can't set this camera.");
    });

    connect(ui->cb_WB, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        // 判斷是否選擇了最後一個選項
        needSetWBCT = (index == ui->cb_WB->count() - 1);
        ui->sb_wbCT->setDisabled(!needSetWBCT);
    });
}

Cam_Set CamSetting::get_UI_set()
{
    Cam_Set set;
    set.IP = packIP();
    //video
    set.videoResolution = ui->cb_Resolution->currentText();
    set.videoFrameRate = ui->cb_FrameRate->currentText();
    set.videoGop = ui->sb_GoP->value();
    set.videoBitRate = ui->cb_Quality->currentText();

    //Image
    set.exposureTime = ui->cb_ExposureTime->currentText();
    set.iso = ui->cb_ISO->currentText();
    set.ev = ui->cb_EV->currentText();
    set.ae = ui->cb_AEArea->currentText();
    set.wdr = ui->cb_WDR->currentText();
    set.wb = ui->cb_WB->currentText();
    set.wbCT = ui->sb_wbCT->value();
    return set;
}

void CamSetting::setUIValue(Cam_Set set, bool setIP)
{
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
    ui->cb_AEArea->setCurrentText(setting.ae);
    ui->cb_WDR->setCurrentText(setting.wdr);
    ui->cb_WB->setCurrentText(setting.wb);
    ui->sb_wbCT->setValue(setting.wbCT);

    setDisableByIP(setting.IP);
//    set_VideoSettings_Disable(isStreaming);
//    set_ImageSettings_Disable(true);
}

void CamSetting::set_VideoSettings_Disable(bool isStream){
    ui->cb_Resolution->setDisabled(isStream);
    ui->cb_FrameRate->setDisabled(isStream);
    ui->sb_GoP->setDisabled(isStream);
    ui->cb_Quality->setDisabled(isStream);
}

void CamSetting::set_ImageSettings_Disable(bool isHide){
    ui->cb_ExposureTime->setDisabled(isHide);
    ui->cb_ISO->setDisabled(isHide);
    ui->cb_EV->setDisabled(isHide);
    ui->cb_AEArea->setDisabled(isHide);
    ui->cb_WDR->setDisabled(isHide);
    ui->cb_WB->setDisabled(isHide);

    if(isHide)
    {
        ui->sb_wbCT->setDisabled(isHide);
    }else
    {
        needSetWBCT = (ui->cb_WB->currentIndex() == ui->cb_WB->count() - 1);
        ui->sb_wbCT->setDisabled(!needSetWBCT);
    }
}

bool CamSetting::setDisableByIP(QString IP)
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


/// Param Func
bool CamSetting::isValidIPv4(const QString &ip)
{
    QRegularExpression ipRegex(
        "^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$"
    );

    return ipRegex.match(ip).hasMatch();
}

QStringList CamSetting::splitIPv4(const QString &ip)
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

void CamSetting::checkDir()
{
    QDir dir(ResourceDir);
    if(!dir.exists())
        dir.mkpath(".");

}

QString CamSetting::packIP()
{
    IPAddress.clear();
    IPAddress.append(QString::number(ui->sb_IPA->value()));
    IPAddress.append(QString::number(ui->sb_IPB->value()));
    IPAddress.append(QString::number(ui->sb_IPC->value()));
    IPAddress.append(QString::number(ui->sb_IPD->value()));

    return IPAddress.join('.');
}

void CamSetting::showMessage(QString Title, QString msg)
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(Title);
    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton("OK", QMessageBox::AcceptRole);
    msgBox->exec();
}

CamSetting::~CamSetting()
{
    delete ui;
}

/// 元件
void CamSetting::on_bt_getSet_clicked()
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

void CamSetting::on_bt_cs_clicked()
{
    //取消
    this->close();
}

void CamSetting::on_bt_save_clicked()
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



//    qDebug()<<wabAPI->test2();
}

void CamSetting::wirteParamFile(QJsonObject content, QString fileName)
{
    QFile file(fileName);
    QJsonDocument json_doc(content);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << json_doc.toJson() << endl;
    file.close();
}

QString CamSetting::readFileToString(QString name) {
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QTextStream in(&file);
    QString JsonString = in.readAll();//error
    file.close();
    return JsonString;
}


