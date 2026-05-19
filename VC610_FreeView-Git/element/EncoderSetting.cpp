#include "EncoderSetting.h"
#include "ui_EncoderSetting.h"

void wirteParamFile(QJsonObject content, QString fileName)
{
    QFile file(fileName);
    QJsonDocument json_doc(content);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << json_doc.toJson() << endl;
    file.close();
}


QString readFileToString(QString name) {
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QTextStream in(&file);
    QString JsonString = in.readAll();//error
    file.close();
    return JsonString;
}

EncoderSetting::EncoderSetting(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EncoderSetting)
{
    ui->setupUi(this);

    QWidget* content = new QWidget(this);
    content->setLayout(ui->MainLayout);
    setCentralWidget(content);
    setWindowModality(Qt::WindowModal);

    setComponentItems();
}

EncoderSetting::~EncoderSetting()
{
    delete ui;
}


void EncoderSetting::closeEvent(QCloseEvent *)
{
    emit destroyed();
}


void EncoderSetting::setLang()
{
    ui->retranslateUi(this);
}

void EncoderSetting::setComponentItems()
{
    // Hide
    ui->label_2->setVisible(false);
    ui->presetComboBox->setVisible(false);
    ui->label_3->setVisible(false);
    ui->profileComboBox->setVisible(false);



    ui->codecComboBox->addItem("h264_nvenc", Encoder_class_format::Encoder_class_h264_nvenc);
    ui->codecComboBox->addItem("hevc_nvenc", Encoder_class_format::Encoder_class_hevc_nvenc);

//    ui->presetComboBox->addItem(tr("品質最高"), Encoder_class_preset::Encoder_class_hp);
//    ui->presetComboBox->addItem(tr("畫質"), Encoder_class_preset::Encoder_class_hq);
//    ui->presetComboBox->addItem(tr("效能"), Encoder_class_preset::Encoder_class_default);
//    ui->presetComboBox->addItem(tr("效能最高"), Encoder_class_preset::Encoder_class_hp);
//    ui->presetComboBox->addItem(tr("低延遲"), Encoder_class_preset::Encoder_class_ll);
//    ui->presetComboBox->addItem(tr("低延遲品質"), Encoder_class_preset::Encoder_class_llhq);
//    ui->presetComboBox->addItem(tr("低延遲效能"), Encoder_class_preset::Encoder_class_llhp);

//    ui->profileComboBox->addItem(tr("最高畫質"), Encoder_class_profile::Encoder_class_high);
//    ui->profileComboBox->addItem(tr("主流畫質"), Encoder_class_profile::Encoder_class_main);
//    ui->profileComboBox->addItem(tr("基本畫質"), Encoder_class_profile::Encoder_class_baseline);

    ui->presetComboBox->addItem(tr("Highest Quality"), Encoder_class_preset::Encoder_class_hp);
//    ui->presetComboBox->addItem(tr("Image Quality"), Encoder_class_preset::Encoder_class_hq);
//    ui->presetComboBox->addItem(tr("Default Performance"), Encoder_class_preset::Encoder_class_default);
//    ui->presetComboBox->addItem(tr("Highest Performance"), Encoder_class_preset::Encoder_class_hp);
//    ui->presetComboBox->addItem(tr("Low Latency"), Encoder_class_preset::Encoder_class_ll);
//    ui->presetComboBox->addItem(tr("Low Latency Image Quality"), Encoder_class_preset::Encoder_class_llhq);
//    ui->presetComboBox->addItem(tr("Low Latency Performance"), Encoder_class_preset::Encoder_class_llhp);

    ui->profileComboBox->addItem(tr("Highest Image Quality"), Encoder_class_profile::Encoder_class_high);
//    ui->profileComboBox->addItem(tr("Mainstream Image Quality"), Encoder_class_profile::Encoder_class_main);
//    ui->profileComboBox->addItem(tr("Basic Image Quality"), Encoder_class_profile::Encoder_class_baseline);

    ui->widthSpinBox->setRange(1920, 3840);
    ui->heightSpinBox->setRange(1080, 2160);
    ui->fpsSpinBox->setRange(25, 120);
    ui->gopSpinBox->setRange(30, 250);
    ui->bitrateSpinBox->setRange(2500, 100000);

    ui->pixelFormatComboBox->addItem("NV12", AVPixelFormat::AV_PIX_FMT_NV12);
//    ui->pixelFormatComboBox->addItem("YUV420P", AVPixelFormat::AV_PIX_FMT_YUV420P);

    setDefault();

    connect(ui->actionExport_Param, &QAction::triggered, this, [=](){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.enc)"));
        if (fileName.isEmpty()) return;

        set_param();
        wirteParamFile(encode_param.toObj(), fileName);
    });

    connect(ui->actionImport_Param, &QAction::triggered, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open txt"), "", "cam focus (*.enc )");
        if (fileName.isEmpty()) return;

        QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
        encode_param = Encoder_param::toStruct(JDoc.object());
        set_init_param(encode_param, 0, true);
    });

    connect(ui->actionDefault_Param, &QAction::triggered, this, [=]() {
        QMessageBox::StandardButton result;
        result = QMessageBox::question(this, "Confirmation", "是否確定回到預設?", QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            encode_param = Encoder_param();
            set_init_param(encode_param, 0, true);
        }
    });

    // disable
    ui->codecComboBox->setDisabled(true);
    ui->widthSpinBox->setDisabled(true);
    ui->heightSpinBox->setDisabled(true);
    ui->pixelFormatComboBox->setDisabled(true);
}

//========================= 設定預設參數 ============================
void EncoderSetting::set_init_param(Encoder_param param, int setFrameCount, bool isEnable)
{
    int codecComboBoxCount = ui->codecComboBox->count();

    int pixelFormatComboBoxCount = ui->pixelFormatComboBox->count();
    for (int i = 0; i < pixelFormatComboBoxCount; i ++) {
        if(param.pix_format == qvariant_cast<AVPixelFormat>(ui->pixelFormatComboBox->itemData(i))){
            ui->pixelFormatComboBox->setCurrentIndex(i);
            break;
        }
    }
    //TODO setby param
    ui->presetComboBox->setCurrentIndex(0);
    ui->profileComboBox->setCurrentIndex(0);

    ui->widthSpinBox->setValue(param.width);
    ui->heightSpinBox->setValue(param.height);
    ui->fpsSpinBox->setValue(param.fps);
    ui->gopSpinBox->setValue(param.gop);
    ui->bitrateSpinBox->setValue(param.bitrate / 1000);
}

void EncoderSetting::setDefault()
{
    ui->codecComboBox->setCurrentIndex(0);
    ui->presetComboBox->setCurrentIndex(0);
    ui->profileComboBox->setCurrentIndex(0);

    ui->widthSpinBox->setValue(1920);
    ui->heightSpinBox->setValue(1080);
    ui->fpsSpinBox->setValue(60);
    ui->gopSpinBox->setValue(60);
    ui->bitrateSpinBox->setValue(3000);

    ui->pixelFormatComboBox->setCurrentIndex(0);
}

void EncoderSetting::on_codecComboBox_currentIndexChanged(int index)
{
    bool isH264;

    if(index == 0)
        isH264 = true;
    else
        isH264 = false;

    ui->presetComboBox->setEnabled(isH264);
    ui->profileComboBox->setEnabled(isH264);
    ui->pixelFormatComboBox->setEnabled(isH264);

    ui->widthSpinBox->setEnabled(isH264);
    ui->heightSpinBox->setEnabled(isH264);
    ui->fpsSpinBox->setEnabled(isH264);
    ui->gopSpinBox->setEnabled(isH264);
    ui->bitrateSpinBox->setEnabled(isH264);
}

//========================= 設定參數 ============================

bool EncoderSetting::set_param()
{
    encode_param.width = ui->widthSpinBox->text().toInt();
    encode_param.height = ui->heightSpinBox->text().toInt();
    encode_param.fps = ui->fpsSpinBox->text().toInt();
    encode_param.gop = ui->gopSpinBox->text().toInt();
    encode_param.bitrate = ui->bitrateSpinBox->text().toInt() * 1000;
    encode_param.pix_format = qvariant_cast<AVPixelFormat>(ui->pixelFormatComboBox->currentData());

    return true;
}



//========================= 傳送參數 ============================

void EncoderSetting::on_confirmBtn_clicked()
{
    if(!set_param())
        return;

    emit sentEncoderSetting(encode_param);
    close();
}

void EncoderSetting::on_cancelBtn_clicked()
{
    close();
}
