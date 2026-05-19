/********************************************************************************
** Form generated from reading UI file 'setting.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTING_H
#define UI_SETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_setting
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QListView *SelectView;
    QStackedWidget *stackedWidget;
    QWidget *cameraPage;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *audioGroup;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QComboBox *comboBox;
    QGroupBox *imageGroup;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *etHLayout;
    QLabel *lb_ExposureTime;
    QComboBox *cb_ExposureTime;
    QHBoxLayout *isoHLayout;
    QLabel *lb_ISO;
    QComboBox *cb_ISO;
    QHBoxLayout *evHLayout;
    QLabel *lb_EV;
    QComboBox *cb_EV;
    QHBoxLayout *aeHLayout;
    QLabel *lb_AEAREA;
    QComboBox *cb_AEAREA;
    QHBoxLayout *wdrHLayout;
    QLabel *lb_WDR;
    QComboBox *cb_WDR;
    QHBoxLayout *wbLayout;
    QLabel *lb_WB;
    QComboBox *cb_WB;
    QHBoxLayout *wbctLayout;
    QLabel *lb_WBCT;
    QSpinBox *sb_WBCT;
    QHBoxLayout *camerabtLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QPushButton *save_btn;
    QWidget *encodePage;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *encoderHLayout;
    QLabel *lb_Encoder;
    QComboBox *cb_Encoder;
    QHBoxLayout *horizontalLayout_5;
    QLabel *lb_preset;
    QComboBox *cb_preset;
    QHBoxLayout *horizontalLayout_6;
    QLabel *lb_profile;
    QComboBox *cb_profile;
    QHBoxLayout *horizontalLayout_7;
    QLabel *lb_ImageW;
    QSpinBox *sb_ImageW;
    QHBoxLayout *horizontalLayout_8;
    QLabel *lb_ImageH;
    QSpinBox *sb_ImageH;
    QHBoxLayout *horizontalLayout_9;
    QLabel *lb_FPS;
    QSpinBox *sb_FPS;
    QHBoxLayout *horizontalLayout_13;
    QLabel *lb_GOP;
    QSpinBox *sb_GOP;
    QHBoxLayout *horizontalLayout_14;
    QLabel *lb_bitrate;
    QSpinBox *sb_bitrate;
    QHBoxLayout *horizontalLayout_15;
    QLabel *lb_pixel;
    QComboBox *cb_pixel;
    QHBoxLayout *horizontalLayout_16;
    QLabel *lb_maxbframe;
    QSpinBox *sb_maxbframe;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *bt_en_bt;
    QPushButton *bt_en_ss;
    QWidget *aoPage;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *SPHLayout;
    QLabel *lb_SP;
    QComboBox *cb_SP;
    QHBoxLayout *RowLayout;
    QLabel *lb_row;
    QComboBox *cb_row;
    QHBoxLayout *columnLayout;
    QLabel *label_4;
    QComboBox *comboBox_4;
    QHBoxLayout *CameraLayout;
    QLabel *label_5;
    QComboBox *comboBox_5;
    QHBoxLayout *horizontalLayout_10;
    QHBoxLayout *horizontalLayout_11;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_12;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *setting)
    {
        if (setting->objectName().isEmpty())
            setting->setObjectName(QString::fromUtf8("setting"));
        setting->resize(878, 678);
        centralwidget = new QWidget(setting);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        SelectView = new QListView(centralwidget);
        SelectView->setObjectName(QString::fromUtf8("SelectView"));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font.setPointSize(16);
        SelectView->setFont(font);

        horizontalLayout->addWidget(SelectView);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        cameraPage = new QWidget();
        cameraPage->setObjectName(QString::fromUtf8("cameraPage"));
        verticalLayout_2 = new QVBoxLayout(cameraPage);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        audioGroup = new QGroupBox(cameraPage);
        audioGroup->setObjectName(QString::fromUtf8("audioGroup"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\273\237\346\255\243\351\273\221\351\253\224"));
        font1.setPointSize(12);
        audioGroup->setFont(font1);
        horizontalLayout_3 = new QHBoxLayout(audioGroup);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(audioGroup);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font1);

        horizontalLayout_2->addWidget(label);

        comboBox = new QComboBox(audioGroup);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setFont(font1);

        horizontalLayout_2->addWidget(comboBox);


        horizontalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout_2->addWidget(audioGroup);

        imageGroup = new QGroupBox(cameraPage);
        imageGroup->setObjectName(QString::fromUtf8("imageGroup"));
        imageGroup->setFont(font1);
        verticalLayout_3 = new QVBoxLayout(imageGroup);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        etHLayout = new QHBoxLayout();
        etHLayout->setObjectName(QString::fromUtf8("etHLayout"));
        lb_ExposureTime = new QLabel(imageGroup);
        lb_ExposureTime->setObjectName(QString::fromUtf8("lb_ExposureTime"));
        lb_ExposureTime->setFont(font1);

        etHLayout->addWidget(lb_ExposureTime);

        cb_ExposureTime = new QComboBox(imageGroup);
        cb_ExposureTime->setObjectName(QString::fromUtf8("cb_ExposureTime"));
        cb_ExposureTime->setFont(font1);

        etHLayout->addWidget(cb_ExposureTime);


        verticalLayout_3->addLayout(etHLayout);

        isoHLayout = new QHBoxLayout();
        isoHLayout->setObjectName(QString::fromUtf8("isoHLayout"));
        lb_ISO = new QLabel(imageGroup);
        lb_ISO->setObjectName(QString::fromUtf8("lb_ISO"));
        lb_ISO->setFont(font1);

        isoHLayout->addWidget(lb_ISO);

        cb_ISO = new QComboBox(imageGroup);
        cb_ISO->setObjectName(QString::fromUtf8("cb_ISO"));
        cb_ISO->setFont(font1);

        isoHLayout->addWidget(cb_ISO);


        verticalLayout_3->addLayout(isoHLayout);

        evHLayout = new QHBoxLayout();
        evHLayout->setObjectName(QString::fromUtf8("evHLayout"));
        lb_EV = new QLabel(imageGroup);
        lb_EV->setObjectName(QString::fromUtf8("lb_EV"));
        lb_EV->setFont(font1);

        evHLayout->addWidget(lb_EV);

        cb_EV = new QComboBox(imageGroup);
        cb_EV->setObjectName(QString::fromUtf8("cb_EV"));
        cb_EV->setFont(font1);

        evHLayout->addWidget(cb_EV);


        verticalLayout_3->addLayout(evHLayout);

        aeHLayout = new QHBoxLayout();
        aeHLayout->setObjectName(QString::fromUtf8("aeHLayout"));
        lb_AEAREA = new QLabel(imageGroup);
        lb_AEAREA->setObjectName(QString::fromUtf8("lb_AEAREA"));
        lb_AEAREA->setFont(font1);

        aeHLayout->addWidget(lb_AEAREA);

        cb_AEAREA = new QComboBox(imageGroup);
        cb_AEAREA->setObjectName(QString::fromUtf8("cb_AEAREA"));
        cb_AEAREA->setFont(font1);

        aeHLayout->addWidget(cb_AEAREA);


        verticalLayout_3->addLayout(aeHLayout);

        wdrHLayout = new QHBoxLayout();
        wdrHLayout->setObjectName(QString::fromUtf8("wdrHLayout"));
        lb_WDR = new QLabel(imageGroup);
        lb_WDR->setObjectName(QString::fromUtf8("lb_WDR"));
        lb_WDR->setFont(font1);

        wdrHLayout->addWidget(lb_WDR);

        cb_WDR = new QComboBox(imageGroup);
        cb_WDR->setObjectName(QString::fromUtf8("cb_WDR"));
        cb_WDR->setFont(font1);

        wdrHLayout->addWidget(cb_WDR);


        verticalLayout_3->addLayout(wdrHLayout);

        wbLayout = new QHBoxLayout();
        wbLayout->setObjectName(QString::fromUtf8("wbLayout"));
        lb_WB = new QLabel(imageGroup);
        lb_WB->setObjectName(QString::fromUtf8("lb_WB"));
        lb_WB->setFont(font1);

        wbLayout->addWidget(lb_WB);

        cb_WB = new QComboBox(imageGroup);
        cb_WB->setObjectName(QString::fromUtf8("cb_WB"));
        cb_WB->setFont(font1);

        wbLayout->addWidget(cb_WB);


        verticalLayout_3->addLayout(wbLayout);

        wbctLayout = new QHBoxLayout();
        wbctLayout->setObjectName(QString::fromUtf8("wbctLayout"));
        lb_WBCT = new QLabel(imageGroup);
        lb_WBCT->setObjectName(QString::fromUtf8("lb_WBCT"));
        lb_WBCT->setFont(font1);

        wbctLayout->addWidget(lb_WBCT);

        sb_WBCT = new QSpinBox(imageGroup);
        sb_WBCT->setObjectName(QString::fromUtf8("sb_WBCT"));
        sb_WBCT->setFont(font1);

        wbctLayout->addWidget(sb_WBCT);


        verticalLayout_3->addLayout(wbctLayout);


        verticalLayout_2->addWidget(imageGroup);

        camerabtLayout = new QHBoxLayout();
        camerabtLayout->setObjectName(QString::fromUtf8("camerabtLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        camerabtLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(cameraPage);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMinimumSize(QSize(0, 40));
        pushButton->setFont(font1);

        camerabtLayout->addWidget(pushButton);

        save_btn = new QPushButton(cameraPage);
        save_btn->setObjectName(QString::fromUtf8("save_btn"));
        save_btn->setMinimumSize(QSize(0, 40));
        save_btn->setFont(font1);

        camerabtLayout->addWidget(save_btn);


        verticalLayout_2->addLayout(camerabtLayout);

        verticalLayout_2->setStretch(1, 5);
        stackedWidget->addWidget(cameraPage);
        encodePage = new QWidget();
        encodePage->setObjectName(QString::fromUtf8("encodePage"));
        verticalLayout_5 = new QVBoxLayout(encodePage);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        encoderHLayout = new QHBoxLayout();
        encoderHLayout->setObjectName(QString::fromUtf8("encoderHLayout"));
        lb_Encoder = new QLabel(encodePage);
        lb_Encoder->setObjectName(QString::fromUtf8("lb_Encoder"));
        lb_Encoder->setFont(font1);

        encoderHLayout->addWidget(lb_Encoder);

        cb_Encoder = new QComboBox(encodePage);
        cb_Encoder->setObjectName(QString::fromUtf8("cb_Encoder"));
        cb_Encoder->setFont(font1);

        encoderHLayout->addWidget(cb_Encoder);


        verticalLayout_5->addLayout(encoderHLayout);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        lb_preset = new QLabel(encodePage);
        lb_preset->setObjectName(QString::fromUtf8("lb_preset"));
        lb_preset->setFont(font1);

        horizontalLayout_5->addWidget(lb_preset);

        cb_preset = new QComboBox(encodePage);
        cb_preset->setObjectName(QString::fromUtf8("cb_preset"));
        cb_preset->setFont(font1);

        horizontalLayout_5->addWidget(cb_preset);


        verticalLayout_5->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        lb_profile = new QLabel(encodePage);
        lb_profile->setObjectName(QString::fromUtf8("lb_profile"));
        lb_profile->setFont(font1);

        horizontalLayout_6->addWidget(lb_profile);

        cb_profile = new QComboBox(encodePage);
        cb_profile->setObjectName(QString::fromUtf8("cb_profile"));
        cb_profile->setFont(font1);

        horizontalLayout_6->addWidget(cb_profile);


        verticalLayout_5->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        lb_ImageW = new QLabel(encodePage);
        lb_ImageW->setObjectName(QString::fromUtf8("lb_ImageW"));
        lb_ImageW->setFont(font1);

        horizontalLayout_7->addWidget(lb_ImageW);

        sb_ImageW = new QSpinBox(encodePage);
        sb_ImageW->setObjectName(QString::fromUtf8("sb_ImageW"));
        sb_ImageW->setFont(font1);

        horizontalLayout_7->addWidget(sb_ImageW);


        verticalLayout_5->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        lb_ImageH = new QLabel(encodePage);
        lb_ImageH->setObjectName(QString::fromUtf8("lb_ImageH"));
        lb_ImageH->setFont(font1);

        horizontalLayout_8->addWidget(lb_ImageH);

        sb_ImageH = new QSpinBox(encodePage);
        sb_ImageH->setObjectName(QString::fromUtf8("sb_ImageH"));
        sb_ImageH->setFont(font1);

        horizontalLayout_8->addWidget(sb_ImageH);


        verticalLayout_5->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        lb_FPS = new QLabel(encodePage);
        lb_FPS->setObjectName(QString::fromUtf8("lb_FPS"));
        lb_FPS->setFont(font1);

        horizontalLayout_9->addWidget(lb_FPS);

        sb_FPS = new QSpinBox(encodePage);
        sb_FPS->setObjectName(QString::fromUtf8("sb_FPS"));
        sb_FPS->setFont(font1);

        horizontalLayout_9->addWidget(sb_FPS);


        verticalLayout_5->addLayout(horizontalLayout_9);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        lb_GOP = new QLabel(encodePage);
        lb_GOP->setObjectName(QString::fromUtf8("lb_GOP"));
        lb_GOP->setFont(font1);

        horizontalLayout_13->addWidget(lb_GOP);

        sb_GOP = new QSpinBox(encodePage);
        sb_GOP->setObjectName(QString::fromUtf8("sb_GOP"));
        sb_GOP->setFont(font1);

        horizontalLayout_13->addWidget(sb_GOP);


        verticalLayout_5->addLayout(horizontalLayout_13);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        lb_bitrate = new QLabel(encodePage);
        lb_bitrate->setObjectName(QString::fromUtf8("lb_bitrate"));
        lb_bitrate->setFont(font1);

        horizontalLayout_14->addWidget(lb_bitrate);

        sb_bitrate = new QSpinBox(encodePage);
        sb_bitrate->setObjectName(QString::fromUtf8("sb_bitrate"));
        sb_bitrate->setFont(font1);

        horizontalLayout_14->addWidget(sb_bitrate);


        verticalLayout_5->addLayout(horizontalLayout_14);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        lb_pixel = new QLabel(encodePage);
        lb_pixel->setObjectName(QString::fromUtf8("lb_pixel"));
        lb_pixel->setFont(font1);

        horizontalLayout_15->addWidget(lb_pixel);

        cb_pixel = new QComboBox(encodePage);
        cb_pixel->setObjectName(QString::fromUtf8("cb_pixel"));
        cb_pixel->setFont(font1);

        horizontalLayout_15->addWidget(cb_pixel);


        verticalLayout_5->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        lb_maxbframe = new QLabel(encodePage);
        lb_maxbframe->setObjectName(QString::fromUtf8("lb_maxbframe"));
        lb_maxbframe->setFont(font1);

        horizontalLayout_16->addWidget(lb_maxbframe);

        sb_maxbframe = new QSpinBox(encodePage);
        sb_maxbframe->setObjectName(QString::fromUtf8("sb_maxbframe"));
        sb_maxbframe->setEnabled(false);
        sb_maxbframe->setFont(font1);

        horizontalLayout_16->addWidget(sb_maxbframe);


        verticalLayout_5->addLayout(horizontalLayout_16);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        bt_en_bt = new QPushButton(encodePage);
        bt_en_bt->setObjectName(QString::fromUtf8("bt_en_bt"));
        bt_en_bt->setMinimumSize(QSize(0, 40));
        bt_en_bt->setFont(font1);

        horizontalLayout_4->addWidget(bt_en_bt);

        bt_en_ss = new QPushButton(encodePage);
        bt_en_ss->setObjectName(QString::fromUtf8("bt_en_ss"));
        bt_en_ss->setMinimumSize(QSize(0, 40));
        bt_en_ss->setFont(font1);

        horizontalLayout_4->addWidget(bt_en_ss);


        verticalLayout_5->addLayout(horizontalLayout_4);

        stackedWidget->addWidget(encodePage);
        aoPage = new QWidget();
        aoPage->setObjectName(QString::fromUtf8("aoPage"));
        verticalLayout_4 = new QVBoxLayout(aoPage);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        SPHLayout = new QHBoxLayout();
        SPHLayout->setObjectName(QString::fromUtf8("SPHLayout"));
        lb_SP = new QLabel(aoPage);
        lb_SP->setObjectName(QString::fromUtf8("lb_SP"));
        lb_SP->setFont(font1);

        SPHLayout->addWidget(lb_SP);

        cb_SP = new QComboBox(aoPage);
        cb_SP->setObjectName(QString::fromUtf8("cb_SP"));
        cb_SP->setFont(font1);

        SPHLayout->addWidget(cb_SP);


        verticalLayout_4->addLayout(SPHLayout);

        RowLayout = new QHBoxLayout();
        RowLayout->setObjectName(QString::fromUtf8("RowLayout"));
        lb_row = new QLabel(aoPage);
        lb_row->setObjectName(QString::fromUtf8("lb_row"));
        lb_row->setFont(font1);

        RowLayout->addWidget(lb_row);

        cb_row = new QComboBox(aoPage);
        cb_row->setObjectName(QString::fromUtf8("cb_row"));
        cb_row->setFont(font1);

        RowLayout->addWidget(cb_row);


        verticalLayout_4->addLayout(RowLayout);

        columnLayout = new QHBoxLayout();
        columnLayout->setObjectName(QString::fromUtf8("columnLayout"));
        label_4 = new QLabel(aoPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setFont(font1);

        columnLayout->addWidget(label_4);

        comboBox_4 = new QComboBox(aoPage);
        comboBox_4->setObjectName(QString::fromUtf8("comboBox_4"));
        comboBox_4->setFont(font1);

        columnLayout->addWidget(comboBox_4);


        verticalLayout_4->addLayout(columnLayout);

        CameraLayout = new QHBoxLayout();
        CameraLayout->setObjectName(QString::fromUtf8("CameraLayout"));
        label_5 = new QLabel(aoPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setFont(font1);

        CameraLayout->addWidget(label_5);

        comboBox_5 = new QComboBox(aoPage);
        comboBox_5->setObjectName(QString::fromUtf8("comboBox_5"));
        comboBox_5->setFont(font1);

        CameraLayout->addWidget(comboBox_5);


        verticalLayout_4->addLayout(CameraLayout);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));

        verticalLayout_4->addLayout(horizontalLayout_10);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        widget = new QWidget(aoPage);
        widget->setObjectName(QString::fromUtf8("widget"));

        horizontalLayout_11->addWidget(widget);


        verticalLayout_4->addLayout(horizontalLayout_11);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        pushButton_3 = new QPushButton(aoPage);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_3->sizePolicy().hasHeightForWidth());
        pushButton_3->setSizePolicy(sizePolicy);
        pushButton_3->setMinimumSize(QSize(0, 40));
        pushButton_3->setFont(font1);

        horizontalLayout_12->addWidget(pushButton_3);

        pushButton_4 = new QPushButton(aoPage);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setMinimumSize(QSize(0, 40));
        pushButton_4->setFont(font1);

        horizontalLayout_12->addWidget(pushButton_4);

        pushButton_5 = new QPushButton(aoPage);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));
        pushButton_5->setMinimumSize(QSize(0, 40));
        pushButton_5->setFont(font1);

        horizontalLayout_12->addWidget(pushButton_5);

        pushButton_6 = new QPushButton(aoPage);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));
        pushButton_6->setMinimumSize(QSize(0, 40));
        pushButton_6->setFont(font1);

        horizontalLayout_12->addWidget(pushButton_6);


        verticalLayout_4->addLayout(horizontalLayout_12);

        stackedWidget->addWidget(aoPage);

        horizontalLayout->addWidget(stackedWidget);

        horizontalLayout->setStretch(0, 2);
        horizontalLayout->setStretch(1, 4);

        verticalLayout->addLayout(horizontalLayout);

        setting->setCentralWidget(centralwidget);
        menubar = new QMenuBar(setting);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 878, 21));
        setting->setMenuBar(menubar);
        statusbar = new QStatusBar(setting);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        setting->setStatusBar(statusbar);

        retranslateUi(setting);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(setting);
    } // setupUi

    void retranslateUi(QMainWindow *setting)
    {
        setting->setWindowTitle(QCoreApplication::translate("setting", "MainWindow", nullptr));
        audioGroup->setTitle(QCoreApplication::translate("setting", "Audio", nullptr));
        label->setText(QCoreApplication::translate("setting", "Mic Select", nullptr));
        imageGroup->setTitle(QCoreApplication::translate("setting", "Image", nullptr));
        lb_ExposureTime->setText(QCoreApplication::translate("setting", "Exposure Time", nullptr));
        lb_ISO->setText(QCoreApplication::translate("setting", "ISO", nullptr));
        lb_EV->setText(QCoreApplication::translate("setting", "EV", nullptr));
        lb_AEAREA->setText(QCoreApplication::translate("setting", "AE AREA", nullptr));
        lb_WDR->setText(QCoreApplication::translate("setting", "WDR", nullptr));
        lb_WB->setText(QCoreApplication::translate("setting", "WB", nullptr));
        lb_WBCT->setText(QCoreApplication::translate("setting", "WBCT", nullptr));
        pushButton->setText(QCoreApplication::translate("setting", "Cancel", nullptr));
        save_btn->setText(QCoreApplication::translate("setting", "Save", nullptr));
        lb_Encoder->setText(QCoreApplication::translate("setting", "Encoder\357\274\232", nullptr));
        lb_preset->setText(QCoreApplication::translate("setting", "Preset\357\274\232", nullptr));
        lb_profile->setText(QCoreApplication::translate("setting", "Profile\357\274\232", nullptr));
        lb_ImageW->setText(QCoreApplication::translate("setting", "Image Width (1920~3840)\357\274\232", nullptr));
        lb_ImageH->setText(QCoreApplication::translate("setting", "Image Height(1080~2160)\357\274\232", nullptr));
        lb_FPS->setText(QCoreApplication::translate("setting", "FPS (25~120)\357\274\232", nullptr));
        lb_GOP->setText(QCoreApplication::translate("setting", "GOP (30~250)\357\274\232", nullptr));
        lb_bitrate->setText(QCoreApplication::translate("setting", "Bitrate kbpst(2500 ~ 100000)\357\274\232", nullptr));
        lb_pixel->setText(QCoreApplication::translate("setting", "Pixel Format\357\274\232", nullptr));
        lb_maxbframe->setText(QCoreApplication::translate("setting", "max_b_frame\357\274\232", nullptr));
        bt_en_bt->setText(QCoreApplication::translate("setting", "Cancel", nullptr));
        bt_en_ss->setText(QCoreApplication::translate("setting", "Save", nullptr));
        lb_SP->setText(QCoreApplication::translate("setting", "Splice Model\357\274\232", nullptr));
        lb_row->setText(QCoreApplication::translate("setting", "row\357\274\232", nullptr));
        label_4->setText(QCoreApplication::translate("setting", "column\357\274\232", nullptr));
        label_5->setText(QCoreApplication::translate("setting", "Camera\357\274\232", nullptr));
        pushButton_3->setText(QCoreApplication::translate("setting", "Reset", nullptr));
        pushButton_4->setText(QCoreApplication::translate("setting", "Clean", nullptr));
        pushButton_5->setText(QCoreApplication::translate("setting", "Cancel", nullptr));
        pushButton_6->setText(QCoreApplication::translate("setting", "Save", nullptr));
    } // retranslateUi

};

namespace Ui {
    class setting: public Ui_setting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTING_H
