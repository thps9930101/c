#ifndef ALLSETTING_H
#define ALLSETTING_H

#include <QMainWindow>
#include <QStringListModel>
#include <QJsonDocument>
#include <QFileDialog>

#include "Data/Struct/AoFunSetting/AoFunSetting.h"
#include "Data/Struct/Encoder_param/encoder_param.h"
#include "Data/Struct/AppSettings/appSettings.h"

#include "../pra/camvc610_webapi_ctrl.h"
#include <QPainter>
#include <QMouseEvent>

class CustomImageWidget : public QWidget
{
    Q_OBJECT

signals:
    void onSelect(int row, int col);

public:
    CustomImageWidget(int rows, int columns, QWidget *parent = nullptr)
        : QWidget(parent), rows(rows), columns(columns), selectedRow(-1), selectedColumn(-1)
    {
        createImage();
    }

    void setRowCol(int newRows,int newCol) {
       columns = newCol;
       rows = newRows;
       createImage();
       update();
    }

    void selectScreen(int row,int col)
    {
        selectedRow = row;
        selectedColumn = col;

        qDebug() << "Clicked at Row:" << selectedRow << "Column:" << selectedColumn;
        emit onSelect(selectedRow, selectedColumn);
        update();
    }
protected:
    void resizeEvent(QResizeEvent *event) override
    {
        createImage();
    }

    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.drawImage(0, 0, image);

        QPen pen;
        painter.setPen(pen);

        for (int i = 0; i <= rows; ++i)
        {
            painter.drawLine(0, i * cellHeight, width() - 2, i * cellHeight);
        }

        for (int j = 0; j <= columns; ++j)
        {
            painter.drawLine(j * cellWidth, 0, j * cellWidth, height());
        }

        if (selectedRow != -1 && selectedColumn != -1)
        {
            painter.fillRect(selectedColumn * cellWidth, selectedRow * cellHeight, cellWidth, cellHeight, Qt::black);

            //pen.setColor(Qt::red);
            painter.setPen(pen);
            painter.drawRect(selectedColumn * cellWidth, selectedRow * cellHeight, cellWidth, cellHeight);
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        QPoint clickedPos = event->pos();

        selectedRow = clickedPos.y() / cellHeight;
        selectedColumn = clickedPos.x() / cellWidth;

        qDebug() << "Clicked at Row:" << selectedRow << "Column:" << selectedColumn;
        emit onSelect(selectedRow, selectedColumn);
        update();
    }

private:
    void createImage()
    {
        cellWidth = width() / columns;
        cellHeight = height() / rows;
        image.fill(Qt::transparent);
        image = QImage(columns * cellWidth, rows * cellHeight, QImage::Format_RGB32);
        image.fill(Qt::darkGray);
    }

    QImage image;
    int rows;
    int columns;
    int cellWidth;
    int cellHeight;
    int selectedRow;
    int selectedColumn;
};


enum UI_allSetting{
    bt_cam_Resolution,
    bt_cam_FrameRate,
    bt_cam_GoP,
    bt_cam_Quality,
    bt_cam_Exposure,
    bt_cam_ISO,
    bt_cam_EV,
    bt_cam_AEAREA,
    bt_cam_WDR,
    bt_cam_WB,
    bt_cam_WBCT,

    bt_enc_Encoder,
    bt_enc_preset,
    bt_enc_profile,
    bt_enc_ImageW,
    bt_enc_ImageH,
    bt_enc_FPS,
    bt_enc_GOP,
    bt_enc_Bitrate,
    bt_enc_Pixel,
    bt_enc_maxbframe,
    bt_ao_RTSP,
    bt_ao_SPmode,
    bt_ao_row,
    bt_ao_col,
    bt_ao_camera
};

QT_BEGIN_NAMESPACE
namespace Ui {
class Allsetting;
}
QT_END_NAMESPACE

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


class Allsetting : public QMainWindow
{
    Q_OBJECT

public:
    explicit Allsetting(QWidget *parent = nullptr);
    ~Allsetting();
    void changeWidget(std::string);
    void selectView_init();
    void selectView(const QModelIndex &index);

    /** @brief 取得UI設定 */
    Cam_Set get_UI_set();
    /** @brief 設定camUI畫面 */
    void setUIValue(Cam_Set set, bool setIP = true);
    void set_VideoSettings_Disable(bool);
    void set_ImageSettings_Disable(bool);
    void showMessage(QString , QString);
    void connectSignals();
    void getset();
    QString masterCam = "";
    void setMicSelect();
    void cam_set_save();

    /** @brief 設定Enc_UI畫面 */
    bool setDisableByIP(QString);
    QString packIP();

    QStringList splitIPv4(const QString &ip);
    bool isValidIPv4(const QString &ip);
    QString readFileToString(QString name);
    void InitalizeUI();
    void setComponentItems();
    void setDefault();
    bool set_param();
    void set_init_param(Encoder_param, int , bool);
    void setEnable_RTSP(bool);



    /** @brief AoSetting */
    void Ao_Ui_init();
    void wirteParamFile(QJsonObject , QString);
    void AO_setUIValue(AoFunSetting);
    AoFunSetting Ao_get_UI_set();
    void setSPID_list(std::vector<uint32_t>,QVector<CamInfo>);
    void set_cb_Camera();

    void setButton_Disable(UI_allSetting , bool );
    QObject* selectUIElement(UI_allSetting);
    void clearLayout(QLayout *);

    /** @brief StreamSetting */
    void setRecordUI(Record_param);
    void setStreamItem();

    /** @brief RTSPSetting */
    void set_RTSP_value(RTSP_param);
    void setRTSPItem();

    /** @brief HotkeySetting */
    void hotKeyUI();

signals:
    void returnParam(Cam_Set set);
    void sentEncoderSetting(Encoder_param param);
    void confirmBtn_signal(AoFunSetting);
    void cantGetResult();
    void c_id_signal(std::vector<uint32_t>);
    void record_signal(Record_param);
    void RTSP_signal(RTSP_param);
    void onFPSChange(int);

private slots:
    void on_bt_cs_clicked();
    void on_bt_en_bt_clicked();
    void on_bt_AO_cs_clicked();
    void on_bt_save_clicked();
    void on_bt_getSet_clicked();
    void on_bt_en_ss_clicked();
    void on_bt_AO_ss_clicked();

    void add_rowcol_combo();

    void on_cb_camera_currentTextChanged(const QString &arg1);
    void on_cb_row_currentTextChanged(const QString &arg1);
    void on_cb_col_currentTextChanged(const QString &arg1);
    void on_cb_SP_currentTextChanged(const QString &arg1);

    void on_cb_preset_currentTextChanged(const QString &arg1);

    void on_cb_profile_currentTextChanged(const QString &arg1);

    void on_pb_Browse_clicked();

    void on_pb_record_ss_clicked();

    void on_pb_record_cancel_clicked();

    void on_bt_ao_clean_clicked();

    void on_bt_RTSP_save_clicked();

    void on_bt_RTSP_cancel_clicked();

    void on_cb_time_currentIndexChanged(int index);

    void on_spinBox_FPS_valueChanged(int arg1);

    void on_cb_service_currentIndexChanged(int index);

    void on_bt_Reset_clicked();

    void on_cb_mic_currentIndexChanged(int index);

private:
    Ui::Allsetting *ui;
    QStringListModel *listmodel;

    QStringList IPAddress;
    Encoder_param encode_param;

    int widgetNum = 0;
    //cam
     bool needSetWBCT = false;
     bool isStreaming = false;
     Cam_Set setting;

     CamVC610_WebAPI_Ctrl *wabAPI = new CamVC610_WebAPI_Ctrl();

    //ao
    QStringList rowcolList;
    AoFunSetting aoSetting;
    std::vector<uint32_t > c_id;
    std::vector<QString> c_ip;
    QVector<CamInfo> camList;
    Record_param recordSetting;
    std::string fileName;
    QString path;
    CustomImageWidget * customImageWidget = nullptr;

    //record
    QStringList error_file_name = {"<",">","：","“","/","\\","|","？","*"};
    int stream_FPS = 0;
    //RTSP
    RTSP_param RTSPSetting;
    int masterLast = -1;
    int micNum;
};

#endif // ALLSETTING_H
