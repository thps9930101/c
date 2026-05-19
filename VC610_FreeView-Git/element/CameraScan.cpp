#include "CameraScan.h"
#include "ui_CameraScan.h"


QStandardItemModel* createModel(){

    // 建立QStandardItemModel
    QStandardItemModel *model = new QStandardItemModel(0, 3); // 建立3行3列的表格

    // 設定表格欄位名稱
//    model->setHeaderData(0, Qt::Horizontal, QObject::tr("No"));
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("IP"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Camera Mode"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("select"));



    return model;
}

void CameraScan::addData(QStandardItemModel* &model, QString IP, QString name)
{
    CheckBoxDelegate delegate;
    QStandardItem *checkBoxItem = new QStandardItem();
    checkBoxItem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    checkBoxItem->setData(Qt::Unchecked, Qt::CheckStateRole); // 设置默认状态
    //checkBoxItem->setEditable(false); // 禁止编辑
    checkBoxItem->setCheckable(true); // 设置可选中

    checkBoxItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable); // 设置可选中和可用
    //checkBoxItem->setDelegate(&delegate); // 设置委托对象



    QList<QStandardItem *> items;

    items   << new QStandardItem(IP)
            << new QStandardItem(name)
            <<  checkBoxItem;

    model->appendRow(items);
}

void clearData(QAbstractItemModel* model)
{
    int row = model->rowCount();
    for (int i = 0; i < row; ++i)
    {
        model->removeRow(0);
    }
}

//====================================================================================

IP_Widget::IP_Widget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;

    layout->setSpacing(0);
    for (int i = 0; i < 4; i++)
    {
        spinBoxes[i] = spinTemplate();
        layout->addWidget(spinBoxes[i]);

        if (i < 3)
        {
            labels[i] = labelTemplate(".");
            layout->addWidget(labels[i]);
        }
    }

    QLabel *lab = labelTemplate("~");
    spin = spinTemplate();
    layout->addWidget(lab);
    layout->addWidget(spin);

    setLayout(layout);
};

IP_Widget::~IP_Widget()
{
    delete spin;

    for(int i=0; i < sizeof (spinBoxes)/ sizeof(QSpinBox); i++){
        delete spinBoxes[i];
    }

    for(int i=0; i < sizeof (labels)/ sizeof(QLabel); i++){
        delete labels[i];
    }
}

QString IP_Widget::base()
{
    QString ip = "";

    for(int i = 0; i < 3; i++){
        ip += QString::number(spinBoxes[i]->value());
        ip += ".";
    }

    return ip;
}

//搜尋IP範圍起點; 回傳IP字串
QString IP_Widget::from()
{
    QString ip = "";

//    for(int i = 0; i < 4; i++){
//        ip += QString::number(spinBoxes[i]->value());
//        if(i != 3)
//            ip += ".";
//    }

    ip += QString::number(spinBoxes[3]->value());

    return ip;
}

//搜尋IP範圍終點; 回傳IP字串
QString IP_Widget::to()
{
    QString ip = "";

//    for(int i = 0; i < 3; i++){
//        ip += QString::number(spinBoxes[i]->value());
//        ip += ".";
//    }

    ip += QString::number(spin->value());

    return ip;
}

//字串設定IP例如: 127.0.0.1
void IP_Widget::setIP(QString ip)
{
    int i = 0;
    QStringList list = ip.split(".");

    foreach(QString s , list)
    {
        spinBoxes[i]->setValue(s.toInt());

        i++;

        if(i == list.size())
            spin->setValue(s.toInt());
    }
}

//字串設定IP例如: 127.0.0.1
void IP_Widget::setIP(QString ipFrom, QString ipTo)
{
    int i = 0;
    QStringList list = ipFrom.split(".");

    foreach(QString s , list)
    {
        spinBoxes[i]->setValue(s.toInt());
        i++;
    }

    spin->setValue(ipTo.split(".").last().toInt());
}

//設定IP元件可設定部f分
void IP_Widget::setEnable(bool bln1, bool bln2, bool bln3, bool bln4)
{
    spinBoxes[0]->setEnabled(bln1);
    spinBoxes[1]->setEnabled(bln2);
    spinBoxes[2]->setEnabled(bln3);
    spinBoxes[3]->setEnabled(bln4);
    spin->setEnabled(bln4);
}

//生成畫面-spinBox模板
QSpinBox* IP_Widget::spinTemplate()
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(0, 255);
    spin->setAlignment(Qt::AlignCenter);
    spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spin->setFixedWidth(100);

    return spin;
}

//生成畫面-label模板
QLabel* IP_Widget::labelTemplate(QString str)
{
    QLabel *lab = new QLabel(str, this);
    lab->setFixedWidth(40);
    lab->setAlignment(Qt::AlignCenter);
    lab->setBackgroundRole(QPalette::Base);

    return lab;
}

//====================================================================================

CameraScan::CameraScan(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CameraScan)
{
    setupApp();

}

CameraScan::CameraScan(QVector<CamInfo> list, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CameraScan)
{
    this->camlist = list;
    setupApp();
}

void CameraScan::setCamlist(QVector<CamInfo> list)
{
    this->camlist = list;
}

void CameraScan::setSelect()
{
    selectRows.clear();
    clearData(ui->tableView->model());
}

void CameraScan::setScanRangelist(QList<QString> list)
{
    ip->setIP(list[0], list[1]);

}

QList<QString> CameraScan::getIP_Range()
{
    QList<QString> list = QList<QString>();
    list.append(ip->base() + ip->from());
    list.append(ip->base() + ip->to());
    return list;
}

void CameraScan::setupApp()
{
    ui->setupUi(this);

    QWidget* content = new QWidget(this);
    content->setLayout(ui->MainLayout);
    setCentralWidget(content);
    setWindowModality(Qt::WindowModal);
    setComponentItems();

    ui->pushButton->setVisible(false);
    ui->pushButton_2->setVisible(false);
    ui->tableViewSelect->setVisible(false);
    ui->checkBox_2->setVisible(false);
    ui->label_4->setVisible(false);
    ui->clearAllBtn->setVisible(false);
    ui->selectAllBtn->setVisible(false);

}

CameraScan::~CameraScan()
{
    delete ip;
    delete loader;
    delete ui;
}

void CameraScan::closeEvent(QCloseEvent  *event)
{
//    deleteLater();
    qDebug()<<"hi";
    if(StartScan)
    {
        event->ignore();
    }else
    {
        event->accept();
    }
}

void CameraScan::setLang()
{
    ui->retranslateUi(this);
}

void CameraScan::setTableSize(int index)
{
    tablesize = index;
    QHeaderView* header=ui->tableView->verticalHeader();
    header->setDefaultSectionSize(tablesize); // 20 px height
    header->sectionResizeMode(QHeaderView::Fixed);



    QHeaderView* headerSelect=ui->tableViewSelect->verticalHeader();
    headerSelect->setDefaultSectionSize(tablesize); // 20 px height
    headerSelect->sectionResizeMode(QHeaderView::Fixed);
}

void CameraScan::setComponentItems()
{
    tableView = ui->tableView;

    model = createModel();
    modelSelect = createModel();

    QObject::connect(model, &QStandardItemModel::itemChanged, [&](QStandardItem *item) {
        if (item->checkState() == Qt::Checked) {
            selectRows.append(item);
            qDebug() << selectRows.size();
            qDebug() << "Checkbox checked at row:" << item->row();
        }else
        {
            for(int i=0;i<selectRows.size();i++)
            {
                if(selectRows[i]->row()==item->row())
                {
                    selectRows.removeAt(i);
                }
            }
        }
    });

    ui->tableView->setModel(model);
    ui->tableView->verticalHeader()->setVisible(false);

//    QHeaderView* header=ui->tableView->horizontalHeader();
//    header->resizeSection(0, 100);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    //model->item(0,0)->setTextAlignment(Qt::AlignHCenter);



    ui->tableView->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Make sure the content is fully visible


    ui->tableViewSelect->setModel(modelSelect);
    ui->tableViewSelect->verticalHeader()->setVisible(false);
    ui->tableViewSelect->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableViewSelect->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->tableViewSelect->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);

    ui->tableViewSelect->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ip = new IP_Widget(this);
    ip->setEnable(true, true, true, true);

    loader = new RotatingWidget();
    loader->setVisible(false);

    // ===========
    ui->clearAllBtn->setVisible(true);
    ui->selectAllBtn->setVisible(true);
    // ===========

    ui->IPWidgetLayout->addWidget(ip);
    ui->horizontalLayout_8->insertWidget(0, loader);

    ui->scanModeComboBox->addItem(tr("Auto"));
    ui->scanModeComboBox->addItem(tr("Manual"));
    connect(camAPI, &CamVC610_WebAPI_Ctrl::scanFinish, this, &CameraScan::scanFinished);

}

void CameraScan::on_scanCameraBtn_clicked()
{
    clearData(ui->tableView->model());
    selectRows.clear();
    StartScan = true;

    ui->tableView->setEnabled(false);
    ui->scanCameraBtn->setEnabled(false);
    ui->scanModeComboBox->setEnabled(false);
    QString ipBase = ip->base();
    qDebug()<<"ipBase:"<<ipBase;
//    ScanCameraThread *th;
    if(ui->scanModeComboBox->currentIndex() == 0)
    {
        camAPI->autoScanCamIP();
    }
    else
    {
        std::string to = ip->to().toStdString().c_str();

        camAPI->scanCamIP_Range(ipBase, ip->from().toInt(),ip->to().toInt());
    }
    loader->setEnable(true);
    loader->setVisible(true);

//    qRegisterMetaType<std::vector<std::pair<std::string, Cam_cfg_param>>>("CamIP_param_list");

//    th->start();
}

void CameraScan::scanFinished(QVector<CamInfo> list)
{
    cam_param_list = list;

    qDebug()<<"list size:"<<list.size();

    if(list.size()==0)
    {
        showMessage("Message","Unable to find camera");
    }


    QStringList selectedCam;
    for (int i = 0; i < modelSelect->rowCount(); ++i)
    {
        selectedCam.append(modelSelect->data(modelSelect->index(i, 0)).toString());
    }

    for (int i = 0; i < (int)list.size(); i++)
    {
        qDebug()<<"listIP："<<list[i].ip;
        bool duplicate = false;
        for (auto cam : camlist) {
            if (cam.ip == list[i].ip)
                duplicate = true;
        }

        if(duplicate)
            continue;

        if(selectedCam.contains(list[i].ip))
            continue;

        QString IP = list[i].ip;
        QString mode = list[i].mode;

        if(mode == "master")
        {
            masterCam = list[i].ip;
            qDebug()<<"masterCam："<<masterCam;
        }
        addData(model, IP, mode);
    }


    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
    ui->tableView->setEnabled(true);
    ui->scanCameraBtn->setEnabled(true);
    ui->scanModeComboBox->setEnabled(true);
    loader->setEnable(false);
    loader->setVisible(false);
    StartScan = false;
}

void CameraScan::on_scanModeComboBox_currentIndexChanged(int index)
{
    if(index == 0)
        ip->setVisible(false);
    else
        ip->setVisible(true);
}

void CameraScan::on_selectAllBtn_clicked()
{
    ui->tableView->selectAll();
}

void CameraScan::on_clearAllBtn_clicked()
{
    ui->tableView->clearSelection();
}

void CameraScan::moveTableItem(QTableView* srcTable, QTableView* dstTable, QStandardItemModel *dstModel)
{
    QModelIndexList selectedIndexes = srcTable->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
        return;
    }
    for (const QModelIndex &index : selectedIndexes)
    {
        int row = index.row();
        QString data = srcTable->model()->data(srcTable->model()->index(row, 0)).toString();
        QString mode = srcTable->model()->data(srcTable->model()->index(row, 1)).toString();
        addData(dstModel, data, mode);
    }

    std::reverse(selectedIndexes.begin(), selectedIndexes.end());
    for (const QModelIndex &index : selectedIndexes)
    {
        int row = index.row();
        srcTable->model()->removeRow(row);
    }

    srcTable->clearSelection();
    dstTable->clearSelection();
}

void CameraScan::on_pushButton_clicked()
{
    moveTableItem(ui->tableView, ui->tableViewSelect, modelSelect);
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
}

void CameraScan::on_pushButton_2_clicked()
{
    moveTableItem(ui->tableViewSelect, ui->tableView, model);
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
}

void CameraScan::on_confirmBtn_clicked() //confirm
{
    ui->tableViewSelect->selectAll();

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();

    if(!model)
        return;

    if(selectRows.size() == 0)
    {
        QMessageBox::information(this, tr("Warning"), tr("No camera selected!"));
        return;
    }

    QVector<CamInfo> cam_result_list;
    // Multiple rows can be selected
//    for(int i = 0; i < selection.count(); ++i)
//    {
//        QModelIndex index = selection.at(i);
//        QString ip = model->data(model->index(index.row(), 0)).toString();
//        QString mode = model->data(model->index(index.row(), 1)).toString();
////        camlist.append(model->data(model->index(index.row(), 0)).toString());

//        CamInfo cam = CamInfo();
//        cam.setCamInfo(ip, true, mode);
//        cam_result_list.push_back(cam);
//    }

    for(int i = 0; i < selectRows.size(); i++)
    {
        //QModelIndex index = selection.at(i);
        QString ip = model->data(model->index(selectRows[i]->row(), 0)).toString();
        QString mode = model->data(model->index(selectRows[i]->row(), 1)).toString();
//        camlist.append(model->data(model->index(index.row(), 0)).toString());

        qDebug()<<"select IP："<<ip;
        qDebug()<<"select mode："<<mode;

        CamInfo cam = CamInfo();
        cam.setCamInfo(ip, true, mode);
        cam_result_list.push_back(cam);
    }

//    foreach(auto item, cam_param_list)
//    {
//        if (camlist.contains(QString::fromStdString(item.ip.toStdString())))
//            cam_result_list.push_back(item);
//    }



    clearData(ui->tableView->model());
    clearData(ui->tableViewSelect->model());
    selectRows.clear();

    if(masterCam !=  "")
    {
        emit masterCam_signal(masterCam);
    }

    emit scanConfirm(cam_result_list);
    emit setIP_Range(getIP_Range());
    close();
//    deleteLater();
}

void CameraScan::on_checkBox_toggled(bool checked)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());

    if (checked)
    {
        for(int i =0;i<model->rowCount();i++)
        {
            QModelIndex index = model->index(i,2);
            model->setData(index, Qt::Checked, Qt::CheckStateRole);
        }
    }else
    {
        for(int i =0;i<model->rowCount();i++)
        {
            QModelIndex index = model->index(i,2);
            model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void CameraScan::on_checkBox_2_toggled(bool checked)
{
    if (checked)
        ui->tableViewSelect->selectAll();
}

void CameraScan::showMessage(QString Title, QString msg)
{
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(Title);
    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton("OK", QMessageBox::AcceptRole);
    msgBox->exec();
}
