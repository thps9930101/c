#include "setting.h"
#include "ui_setting.h"
#include "apiclient.h"

setting::setting(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::setting)
{
    ui->setupUi(this);
    UI_setting();
    selectView_init();

    apiPost = new ApiClient(this); // 或你的成員變數

    setComponentItems();

    decode_param = Decoder_class_param();

}

setting::~setting()
{
    delete ui;
}

void setting::UI_setting()
{
    /// 新增UI選項
    all_Cam_UI_Setting set;

    //image
    ui->cb_ExposureTime->addItems(set.exposureTimeList);
    ui->cb_ISO->addItems(set.isoList);
    ui->cb_EV->addItems(set.evList);
    ui->cb_AEAREA->addItems(set.aeAreaList);
    ui->cb_WDR->addItems(set.wdrList);
    ui->cb_WB->addItems(set.wbList);
    ui->sb_WBCT->setRange(set.wbCT.min,set.wbCT.max);
}

void setting::selectView_init()
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

    QObject::connect(ui->SelectView, &QListView::clicked, this, &setting::selectView);
}

void setting::selectView(const QModelIndex &index)
{
    std::string selectedItem = listmodel->data(index, Qt::DisplayRole).toString().toStdString();
    changeWidget(selectedItem);
}

void setting::changeWidget(std::string settingName)
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
}

void setting::initCam(QJsonObject Json)
{
    settingJson = Json;
    QJsonObject image;
    if (settingJson.contains("image") && settingJson["image"].isObject()) {
        image = settingJson["image"].toObject();
    }
    qDebug()<<image["wbCT"].toString().toInt();
    applyImageConfigToUI(image);
}

//========================= EncoderSetting ==========================
void setting::setComponentItems()
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
    ui->sb_GOP->setRange(4, 60);
    ui->sb_bitrate->setRange(2500, 100000);

    ui->cb_pixel->addItem("NV12", AVPixelFormat::AV_PIX_FMT_NV12);
//    ui->pixelFormatComboBox->addItem("YUV420P", AVPixelFormat::AV_PIX_FMT_YUV420P);

    setDefault();

    // disable
    ui->cb_Encoder->setDisabled(false);
    ui->sb_ImageW->setDisabled(true);
    ui->sb_ImageH->setDisabled(true);
    ui->cb_pixel->setDisabled(true);
}

void setting::setDefault()
{
    encode_param = Encoder_param();
    qDebug()<<"encode_param.en_format"<<encode_param.en_format;
    ui->cb_Encoder->setCurrentIndex(encode_param.en_format);
    ui->sb_ImageW->setValue(encode_param.width);
    ui->sb_ImageH->setValue(encode_param.height);
    ui->sb_FPS->setValue(encode_param.fps);
    ui->sb_GOP->setValue(encode_param.gop);
    ui->sb_bitrate->setValue(encode_param.bitrate / 1000);  // 還原為 kbps

    // 設定 pixel format 到 combo box（假設 combo 的每個 item 存的是 AVPixelFormat）
    for (int i = 0; i < ui->cb_pixel->count(); ++i) {
        if (ui->cb_pixel->itemData(i).toInt() == encode_param.pix_format) {
            ui->cb_pixel->setCurrentIndex(i);
            break;
        }
    }
}

bool setting::set_enocde_param()
{
    encode_param.width = ui->sb_ImageW->text().toInt();
    encode_param.height = ui->sb_ImageH->text().toInt();
    encode_param.fps = ui->sb_FPS->text().toInt();
    encode_param.gop = ui->sb_GOP->text().toInt();
    encode_param.bitrate = ui->sb_bitrate->text().toInt() * 1000;
    encode_param.pix_format = qvariant_cast<AVPixelFormat>(ui->cb_pixel->currentData());

    encodeObj["width"] = ui->sb_ImageW->value();      // 整數值，不需轉成文字後再 toInt()
    encodeObj["height"] = ui->sb_ImageH->value();
    encodeObj["fps"] = ui->sb_FPS->value();
    encodeObj["gop"] = ui->sb_GOP->value();
    encodeObj["bitrate"] = ui->sb_bitrate->value() * 1000;

    encodeObj["pix_format"] = static_cast<int>(encode_param.pix_format);
    encodeObj["en_format"] = static_cast<int>(encode_param.en_format);
    encodeObj["preset"] = static_cast<int>(encode_param.preset);
    encodeObj["profile"] = static_cast<int>(encode_param.profile);
    encodeObj["max_b_frame"] = encode_param.max_b_frame;

    return true;
}

Encoder_param setting::get_encode_param(){
    return encode_param;
}
Decoder_class_param setting::get_decode_param(){
    return decode_param;
}
void setting::applyImageConfigToUI(const QJsonObject& imageObj) {
    auto setComboBoxValue = [](QComboBox* box, const QString& value) {
        int index = box->findText(value);
        if (index != -1)
            box->setCurrentIndex(index);
    };

    if (imageObj.contains("exposureTime"))
        setComboBoxValue(ui->cb_ExposureTime, imageObj["exposureTime"].toString());

    if (imageObj.contains("iso"))
        setComboBoxValue(ui->cb_ISO, imageObj["iso"].toString());

    if (imageObj.contains("ev"))
        setComboBoxValue(ui->cb_EV, imageObj["ev"].toString());

    if (imageObj.contains("ae"))
        setComboBoxValue(ui->cb_AEAREA, imageObj["ae"].toString());

    if (imageObj.contains("wdr"))
        setComboBoxValue(ui->cb_WDR, imageObj["wdr"].toString());

    if (imageObj.contains("wb"))
        setComboBoxValue(ui->cb_WB, imageObj["wb"].toString());

    if (imageObj.contains("wbCT")) {
        QJsonValue val = imageObj["wbCT"];
        int wbCT = val.isDouble() ? val.toInt() : val.toString().toInt();
        ui->sb_WBCT->setValue(wbCT);
    }
}

QJsonObject setting::collectImageConfigFromUI() {
    QJsonObject imageObj;

    imageObj["exposureTime"] = ui->cb_ExposureTime->currentText();
    imageObj["iso"] = ui->cb_ISO->currentText();
    imageObj["ev"] = ui->cb_EV->currentText();
    imageObj["ae"] = ui->cb_AEAREA->currentText();
    imageObj["wdr"] = ui->cb_WDR->currentText();
    imageObj["wb"] = ui->cb_WB->currentText();
    imageObj["wbCT"] = ui->sb_WBCT->value();  // 儲存為 int

    return imageObj;
}

void setting::MicComboboxInit(QList<QString> IPList)
{
    for (int i = 0; i < IPList.size(); i++) {
        ui->comboBox->addItem(IPList[i], i+1);  // 第二個參數是 userData
    }
}

void setting::on_save_btn_clicked()
{
    ui->save_btn->setEnabled(false);
    QJsonObject UImage = collectImageConfigFromUI();

    settingJson["image"] = UImage;
    JsonFileManager::writeConfig(settingJson);
    apiPost->camSet("172.16.0.1",UImage);

    ui->save_btn->setEnabled(true);
}

void setting::on_bt_en_ss_clicked()
{
    ui->bt_en_ss->setEnabled(false);

    set_enocde_param();

    settingJson["encode"] = encodeObj;
    JsonFileManager::writeConfig(settingJson);


    ui->bt_en_ss->setEnabled(true);
}
