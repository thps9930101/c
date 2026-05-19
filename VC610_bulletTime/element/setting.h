#ifndef SETTING_H
#define SETTING_H

#include <QMainWindow>
#include <QStringListModel>
#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QList>
#include <QString>

#include "jsonfilemanager.h"
#include "apiclient.h"
#include <libavutil/pixfmt.h>
#include "Data/Struct/Encoder_param/encoder_param.h"
#include "Data/Struct/Decoder_param/decoder_class_mode.h"

namespace Ui {
class setting;
}

class PressEvent;

class setting : public QMainWindow
{
    Q_OBJECT

public:
    explicit setting(QWidget *parent = nullptr);
    ~setting();

    void initCam(QJsonObject);
    void MicComboboxInit(QList<QString>);
    Encoder_param get_encode_param();
    Decoder_class_param get_decode_param();

private slots:
    void on_save_btn_clicked();

    void on_bt_en_ss_clicked();

private:
    Ui::setting *ui;

    QStringListModel *listmodel;
    QJsonObject settingJson;

    int widgetNum = 0;

    void selectView_init();
    void UI_setting();
    void selectView(const QModelIndex &index);
    void changeWidget(std::string);
    void applyImageConfigToUI(const QJsonObject&);
    void setComponentItems();
    void setDefault();
    bool set_enocde_param();

    QJsonObject collectImageConfigFromUI();
    ApiClient *apiPost; // 或你的成員變數
    Encoder_param encode_param;
    Decoder_class_param decode_param;

    QJsonObject encodeObj;

};

struct all_Cam_UI_Setting
{
    //Video
    QStringList resolutionList;
    QStringList frameRateList;
    struct Range{
        int min;
        int max;
    };
    Range gopRange;
    QStringList qualityList;

    //Image
    QStringList exposureTimeList;
    QStringList isoList;
    QStringList evList;
    QStringList aeAreaList;
    QStringList wdrList;
    QStringList wbList;
    Range wbCT;

    all_Cam_UI_Setting(){//QStringList({});
        //Video
        resolutionList = QStringList({"1920x1080","3840x2160"});
        frameRateList = QStringList({"FPS30","FPS60"});
        gopRange = Range({2,60});
        qualityList = QStringList({"Low","Middle","High"});
        //Image
        exposureTimeList = QStringList({
            "auto", "1/6400", "1/5000", "1/4000", "1/3200", "1/2500", "1/2000", "1/1600",
            "1/1250", "1/1200", "1/1000", "1/800", "1/640", "1/600", "1/500", "1/400",
            "1/320", "1/250", "1/240", "1/200", "1/160", "1/125", "1/120", "1/110",
            "1/100", "1/80", "1/60", "1/55", "1/50", "1/48", "1/40", "1/30"
        });
        isoList = QStringList({
            "auto", "100", "200", "400", "800", "1600", "3200", "6400"
        });
        evList = QStringList({
            "-3.0", "-2.7", "-2.3", "-2.0", "-1.7", "-1.3", "-1.0",
            "-0.7", "-0.3", "0.0", "+0.3", "+0.7", "+1.0", "+1.3", "+1.7",
            "+2.0", "+2.3", "+3.0"
        });
        aeAreaList = QStringList({
            "center", "average"
        });
        wdrList = QStringList({
            "off", "on"
        });
        wbList = QStringList({
            "auto", "daylight", "cloudy", "fluorescent", "incandescent", "fluorescentCWF", "customer"
        });
        wbCT = Range({2800, 9000});
    }
};
#endif // SETTING_H
