#ifndef ENCODERSETTING_H
#define ENCODERSETTING_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QMetaType>
#include <QTextStream>
#include <QDebug>
#include "Data/Struct/Encoder_param/encoder_param.h"

//Q_DECLARE_METATYPE(AVPixelFormat)

namespace Ui {
class EncoderSetting;
}

class EncoderSetting : public QMainWindow
{
    Q_OBJECT

signals:
    void sentEncoderSetting(Encoder_param param);

public:
    explicit EncoderSetting(QWidget *parent = nullptr);
    ~EncoderSetting();

    void setLang();


    void closeEvent(QCloseEvent *);

    ///
    /// \brief set_init_param   畫面初始設定預設值
    /// \param BT_param         預設子彈時間參數
    /// \param setFrameCount    預設影片前後撥放秒數
    ///
    void set_init_param(Encoder_param param, int setFrameCount, bool isEnable);

private slots:
    void on_cancelBtn_clicked();

    void on_confirmBtn_clicked();

    void on_codecComboBox_currentIndexChanged(int index);

private:
    Ui::EncoderSetting *ui;
    Encoder_param encode_param;
    int VideoFrameCount;                    /*!< [參數]子彈時間影片前後撥放秒數 */

    ///
    /// \brief setDefault           畫面初始設定預設值
    ///
    void setDefault();


    bool set_param();

    ///
    /// \brief setComponentItems    設定元件範圍、選項
    ///
    void setComponentItems();
};

#endif // ENCODERSETTING_H
