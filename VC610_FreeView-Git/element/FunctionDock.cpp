#include "FunctionDock.h"
#include "ui_FunctionDock.h"

FunctionDock::FunctionDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::FunctionDock)
{
    this->parent = parent;
    ui->setupUi(this);
    InitalizeUI();
//    on_bt_scan_clicked();

}

void FunctionDock::InitalizeUI()
{
    ui->widget->setLayout(ui->verticalLayout);
    setWidget(ui->widget);

    ui->combo_mode->addItem("Single Mode");
    ui->combo_mode->addItem("Multi-View Mode");

    ui->tableView->verticalHeader()->setVisible(false);

    ui->bt_RTSP->setVisible(false);
    ui->bt_rotate->setVisible(false);
    ui->bt_scan->setVisible(false);
    ui->bt_stream->setVisible(false);
    ui->pb_StartRecord->setVisible(false);
    ui->combo_mode->setVisible(false);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    model = new QStandardItemModel(0, 4, this);
    //chinese
    /*model->setHeaderData(0, Qt::Horizontal, "編號");
    model->setHeaderData(1, Qt::Horizontal, "IP");
    model->setHeaderData(2, Qt::Horizontal, "相機狀態");
    model->setHeaderData(3, Qt::Horizontal, "主從模式");*/

    //English

    model->setHeaderData(0, Qt::Horizontal, "No");
    model->setHeaderData(1, Qt::Horizontal, "IP");
    model->setHeaderData(2, Qt::Horizontal, "mode");
    model->setHeaderData(3, Qt::Horizontal, "status");


    ui->tableView->setModel(model);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &FunctionDock::showtableView);

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FunctionDock::tableViewIndexOnChange);

    // start stop enable;
    ui->verticalLayout->setStretch(0,0);
    ui->bt_Start->setVisible(false);
    ui->bt_Stop->setVisible(false);

    ui->spinBox_FPS->setMinimum(1);
    ui->spinBox_FPS->setMaximum(120);    

    recordMode();
}

void FunctionDock::setAppSetting(AppSettings* set)
{
    appSettings = set;
    mode = appSettings->aoFunSetting.mode;
    ui->combo_mode->setCurrentIndex(mode);

    ui->spinBox_FPS->setVisible(false);
    ui->label->setVisible(false);

    QRegularExpression re("\\d+");
    QRegularExpressionMatch match = re.match(appSettings->camSetting.videoFrameRate);

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

void FunctionDock::tableViewIndexOnChange(const QModelIndex &current, const QModelIndex &)
{
    if (!(camCount > 0))
        return;
    emit onLiveCamNo_change(current.row());
}

void FunctionDock::showtableView(const QPoint& pos)
{
    if (!(camCount > 0))
        return;

    QModelIndex index = ui->tableView->indexAt(pos);

    // 如果索引無效，即右鍵點擊的是空白處，則不顯示菜單
    if (!index.isValid())
        return;

    QMenu *menu = new QMenu(this);
//    QMenu *menu = new QMenu(ui->tableView);
    // 在這裡可以根據需要添加其他右鍵選單項目

    QAction *startItem = menu->addAction("start");
    QAction *stopItem = menu->addAction("stop");
    QAction *deleteItem = menu->addAction("delete");
    QAction *deleteAllItem = menu->addAction("delete all");

    connect(startItem, &QAction::triggered, this, &FunctionDock::startCam);
    connect(stopItem, &QAction::triggered, this, &FunctionDock::stopCam);
    connect(deleteItem, &QAction::triggered, this, &FunctionDock::deleteCam);
    connect(deleteAllItem, &QAction::triggered, this, &FunctionDock::deleteAllCam);

    menu->addAction(startItem);
    menu->addAction(stopItem);
    menu->addAction(deleteItem);
    menu->addAction(deleteAllItem);

    // 顯示右鍵選單
    menu->exec(ui->tableView->mapToGlobal(pos));
    delete menu;
}

FunctionDock::~FunctionDock()
{
    delete ui;
}

void FunctionDock::addCam(int num, const QString& camIP, const QString& status, const QString& mode)
{
    QList<QStandardItem *> items;
    QStandardItem *column_Num = new QStandardItem(QString::number(num + 1));
    QStandardItem *column_IP = new QStandardItem(camIP);
    QStandardItem *column_Status = new QStandardItem(status);
    QStandardItem *column_mode = new QStandardItem(mode);

    column_Num->setTextAlignment(Qt::AlignCenter);
    column_IP->setTextAlignment(Qt::AlignCenter);
    column_Status->setTextAlignment(Qt::AlignCenter);
    column_mode->setTextAlignment(Qt::AlignCenter);

    items   << column_Num
            << column_IP
            << column_Status
            << column_mode;

    model->appendRow(items);
}

// delete all single
void FunctionDock::deleteAllCam()
{
    // 發送刪除訊號
    emit ondeleteAllCam();
}

// delete single
void FunctionDock::deleteCam()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    // 遍歷選擇的每一行
    for (const QModelIndex& index : selectedRows) {
        int rowToRemove = index.row();

        // 獲取模型
        // QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());

        // 發送刪除訊號
        emit ondeleteCam(rowToRemove);
    }
}

// stop single
void FunctionDock::stopCam()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    // 遍歷選擇的每一行
    for (const QModelIndex& index : selectedRows) {
        int rowToRemove = index.row();

        // 獲取模型
        // QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());

        // 發送刪除訊號
        emit onstopCam(rowToRemove);
    }
}

void FunctionDock::startCam()
{
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();

    // 遍歷選擇的每一行
    for (const QModelIndex& index : selectedRows) {
        int rowToRemove = index.row();

        emit onstartCam(rowToRemove);
    }
}
void FunctionDock::tableViewChange(int index)
{
    QModelIndex row = ui->tableView->model()->index(index, 0, QModelIndex());
    ui->tableView->setCurrentIndex(row);
}


void FunctionDock::setTableView(QVector<CamInfo> list)
{
    model->removeRows(0, model->rowCount());

    int i= 0;
    foreach(auto item, list)
    {
        addCam(i,item.ip,item.isConnect?"ON":"OFF",item.mode);
        i++;
    }
    camCount = list.size();
}

void FunctionDock::RTSP_Change(bool isSusccess, bool isOpen) {
    ui->bt_RTSP->setEnabled(true);
    if (!isSusccess)
        return;

    if (isOpen)
        ui->bt_RTSP->setText("RTSP Stop");
    else
        ui->bt_RTSP->setText("RTSP Start");
}


QObject* FunctionDock::selectUIElement(UI_FunctionDock funDuck)
{
    switch (funDuck) {
    case allDown:
    case init:
        break;

    case bt_scan:
        return ui->bt_scan;

    case bt_stream:
        return ui->bt_stream;

    case bt_startRecord:
        return ui->bt_Start;

    case bt_stopRecord:
        return ui->bt_Stop;

    case bt_RTSP:
        return ui->bt_RTSP;

    case bt_record:
        return  ui->pb_StartRecord;

    case combo_mode:
        return  ui->combo_mode;

    case bt_rotate:
        return  ui->bt_rotate;
    }
}


void FunctionDock::setButton_Disable(UI_FunctionDock funDuck, bool bl)
{
    switch (funDuck) {
    case allDown:
        ui->bt_scan->setDisabled(true);
        ui->bt_stream->setDisabled(true);
        ui->bt_Start->setDisabled(true);
        ui->bt_Stop->setDisabled(true);
        ui->bt_RTSP->setDisabled(true);
        ui->pb_StartRecord->setDisabled(true);
        ui->bt_rotate->setDisabled(true);
        return;
    case init:
        ui->bt_scan->setDisabled(false);
        ui->bt_stream->setDisabled(true);
        ui->bt_Start->setDisabled(true);
        ui->bt_Stop->setDisabled(true);
        ui->bt_RTSP->setDisabled(true);
        ui->pb_StartRecord ->setDisabled(true);
        ui->bt_rotate->setDisabled(true);
        return;
    }

    QPushButton* temp = qobject_cast<QPushButton*>(selectUIElement(funDuck));
    if (temp) {
        temp->setDisabled(bl);
    }
    QComboBox* cotemp = qobject_cast<QComboBox*>(selectUIElement(funDuck));
    if (cotemp) {
        cotemp->setDisabled(bl);
    }
}

void FunctionDock::setButton_Text(UI_FunctionDock funDuck, QString text)
{
    switch (funDuck) {
    case allDown:
        ui->bt_scan->setDisabled(true);
        ui->bt_stream->setDisabled(true);
        ui->bt_Start->setDisabled(true);
        ui->bt_Stop->setDisabled(true);
        return;
    case init:
        ui->bt_scan->setDisabled(false);
        ui->bt_stream->setDisabled(true);
        ui->bt_Start->setDisabled(true);
        ui->bt_Stop->setDisabled(true);
        return;
    case bt_record:
        start_record =true;
        ui->pb_StartRecord->setText(text);
        return;
    }

    QPushButton* temp = qobject_cast<QPushButton*>(selectUIElement(funDuck));
    if (temp) {
        temp->setText(text);
    }
}

/// element
void FunctionDock::setBt_streamText(QString str)
{
    ui->bt_stream->setText(str);
}

void FunctionDock::on_bt_scan_clicked()
{
    emit scanButtonClick();
}

void FunctionDock::on_bt_Start_clicked()
{
    emit startButtonClick();
}

void FunctionDock::on_bt_Stop_clicked()
{
    emit stopButtonClick();
}

void FunctionDock::on_bt_stream_clicked()
{
    click_StreamButton();
}

void FunctionDock::click_StreamButton()
{
    int rowCount = model->rowCount();
    int columnCount = model->columnCount();
    QVector<CamInfo> camList;

    for (int row = 0; row < rowCount; ++row)
    {
        CamInfo cam;
        cam.ip = model->index(row, 1).data().toString();
        cam.isConnect = (model->index(row, 2).data().toString() == "OFF")? false: true;
        cam.mode = model->index(row, 3).data().toString();
        camList.append(cam);
    }

    emit StreamSignal(camList);
}

void FunctionDock::on_combo_mode_currentIndexChanged(int index)
{
    change_mode(index);
}

void FunctionDock::change_mode(int index)
{
    Show_Mode show_mode;
    //qDebug()<<"index:"<<index;
    if(index == 0)
    {
      show_mode = Show_Mode::Show_Single;
    }else
    {
       show_mode = Show_Mode::Show_splice;
    }

    mode = index;

    if (appSettings)
        appSettings->aoFunSetting.mode = mode;
    emit onStreamModeChange(show_mode);
}

void FunctionDock::on_bt_RTSP_clicked()
{
    ui->bt_RTSP->setEnabled(false);
    emit rtsp_Signal();
}

void FunctionDock::on_spinBox_FPS_valueChanged(int arg1)
{
    emit onFPSChange(arg1);
}

void FunctionDock::on_pb_StartRecord_clicked()
{
    StartRecord();
}

void FunctionDock::StartRecord()
{
    int ret = 0;

    if(start_record)
    {
        ui->pb_StartRecord->setText("Start Recording");
    }else
    {
        ui->pb_StartRecord->setText("Stop Recording");
    }

    emit record_Signal(ret,start_record);
    start_record = !start_record;
}

void FunctionDock::recordMode()
{
//    if(ui->combo_mode->currentIndex() == 0)
//    {
//        ui->pb_StartRecord->setEnabled(false);
//    }else
//    {
//        ui->pb_StartRecord->setEnabled(true);
//    }

    ui->pb_StartRecord->setEnabled(true);
}

void FunctionDock::on_bt_rotate_clicked()
{
    emit rotate_Signal();
}

void FunctionDock::on_spinBox_FPS_textChanged(const QString &arg1)
{

}
