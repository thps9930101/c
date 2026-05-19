#include "aocamsetting.h"
#include "ui_aocamsetting.h"

AoCamSetting::AoCamSetting(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AoCamSetting)
{
    ui->setupUi(this);
    QWidget* content = new QWidget(this);
    content->setLayout(ui->MainLayout);
    setCentralWidget(content);


    ui->cb_spmode->addItem("4×3");
    ui->cb_spmode->addItem("3×3");
    ui->cb_spmode->addItem("3×2");
    ui->cb_spmode->addItem("2×2");


    connect(ui->actionExport_Param, &QAction::triggered, this, [=](){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), tr("save project (*.aoset)"));
        if (fileName.isEmpty()) return;

        wirteParamFile(get_UI_set().toObj(), fileName);
    });

    connect(ui->actionImport_Param, &QAction::triggered, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open .aoset"), "", "cam setting (*.aoset )");
        if (fileName.isEmpty()) return;

        QJsonDocument JDoc = QJsonDocument::fromJson(readFileToString(fileName).toUtf8());
        setUIValue(AoFunSetting::toStruct(JDoc.object()));
    });

    connect(ui->actionDefault_Param, &QAction::triggered, this, [=]() {
        QMessageBox::StandardButton result;
        result = QMessageBox::question(this, "Confirmation", "Are you sure you want to revert to default?", QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
            setUIValue(AoFunSetting());
    });
}

void AoCamSetting::setUIValue(AoFunSetting set)
{
    aoSetting = set;
    //ui->lineEdit->setText(aoSetting.rtsp_path);
}

AoFunSetting AoCamSetting::get_UI_set()
{
    AoFunSetting set;
    //set.rtsp_path = ui->lineEdit->text();
    //video

    return set;
}

AoCamSetting::~AoCamSetting()
{
    delete ui;
}

void AoCamSetting::on_cancelBtn_clicked()
{
    close();
}

void AoCamSetting::on_confirmBtn_clicked()
{
    //aoSetting.rtsp_path = ui->lineEdit->text();
    aoSetting.SP_mode = static_cast<SP_MODE>(ui->cb_spmode->currentIndex());

    emit confirmBtn_signal(aoSetting);
    close();
}


void AoCamSetting::wirteParamFile(QJsonObject content, QString fileName)
{
    QFile file(fileName);
    QJsonDocument json_doc(content);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << json_doc.toJson() << endl;
    file.close();
}

QString AoCamSetting::readFileToString(QString name) {
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QTextStream in(&file);
    QString JsonString = in.readAll();//error
    file.close();
    return JsonString;
}
