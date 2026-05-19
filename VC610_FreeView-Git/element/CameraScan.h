#ifndef CAMERASCAN_H
#define CAMERASCAN_H

#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QMessageBox>
#include <QProxyStyle>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QCloseEvent>

#include "pra/camvc610_webapi_ctrl.h"
#include "utils/customqthread.h"


//Q_DECLARE_METATYPE(CamIP_param_list)

class CheckBoxDelegate : public QStyledItemDelegate {
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(option);
        Q_UNUSED(index);

        QCheckBox *checkBox = new QCheckBox(parent);
        return checkBox;
    }


    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
            QStyledItemDelegate::initStyleOption(option, index);
            option->displayAlignment = Qt::AlignHCenter;
     }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        bool value = index.model()->data(index, Qt::EditRole).toBool();
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        checkBox->setChecked(value);

    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        bool value = checkBox->isChecked();
        model->setData(index, value, Qt::EditRole);

    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};

/**
 * @brief IP元件
 */
class IP_Widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int age)

public:

    /**
     * @brief 初始化IP元件
     * @param parent 父控制項
     */
    IP_Widget(QWidget *parent = nullptr);

    /**
     * @brief IP元件解構式
     */
    ~IP_Widget();

    /**
     * @brief 搜尋IP範圍，前三碼
     * @return 前三碼IP字串(ex:192.168.0.)
     */
    QString base();

    /**
     * @brief 搜尋IP範圍起點
     * @return 起始IP字串
     */
    QString from();

    /**
     * @brief 搜尋IP範圍終點
     * @return 終點IP字串
     */
    QString to();

    /**
     * @brief 字串設定IP例如: 127.0.0.1
     */
    void setIP(QString ip);

    /**
     * @brief 字串設定IP例如: 127.0.0.1
     */
    void setIP(QString ipFrom, QString ipTo);

    /**
     * @brief 設定IP是否可輸入
     */
    void setEnable(bool, bool, bool, bool);

private:

    QSpinBox *spinBoxes[4];     /*!< [數值元件]IP輸入框(0~255) */
    QSpinBox *spin;             /*!< [數值元件]IP輸入框(0~255) */
    QLabel *labels[3];          /*!< [文字]IP輸入框分隔逗點 */

    /**
     * @brief [生成畫面]spinBox模板
     */
    QSpinBox* spinTemplate();

    /**
     * @brief [生成畫面]label模板
     */
    QLabel* labelTemplate(QString str = "");
};


namespace Ui {
class CameraScan;
}

class CameraScan : public QMainWindow
{
    Q_OBJECT

signals:

    /**
     * @brief 確認[CameraScan]掃描結果，傳送信號
     * @param camlist 已選擇相機清單
     */
//    void CameraScan_send_param(QStringList camlist, QStringList);
    void scanConfirm(QVector<CamInfo>);
    void setIP_Range(QList<QString>);
    void masterCam_signal(QString);

public:
    explicit CameraScan(QWidget *parent = nullptr);
    explicit CameraScan(QVector<CamInfo> list = QVector<CamInfo>(), QWidget *parent = nullptr);
    ~CameraScan();

    void setLang();

    /**
     * @brief [emit]關閉並傳送信號
     */

    void setCamlist(QVector<CamInfo> list);

    void setScanRangelist(QList<QString>);

    QList<QString> getIP_Range();

    QTableView *tableView;
    QTableView *tableViewSelect;
    int tablesize = 24;
    void testCURL();

    void sreachRange();

    void moveTableItem(QTableView* srcTable, QTableView* dstTable, QStandardItemModel *dstModel);
    void scanFinished(QVector<CamInfo>);
    void setTableSize(int);
    void setSelect();
    void addData(QStandardItemModel* &model, QString IP, QString name);
    void showMessage(QString , QString );

protected:
    void closeEvent(QCloseEvent *event);


private slots:
    /**
     * @brief 非同步取得掃描結果
     */


    void on_scanModeComboBox_currentIndexChanged(int index);

    void on_scanCameraBtn_clicked();

    void on_selectAllBtn_clicked();

    void on_clearAllBtn_clicked();

    void on_confirmBtn_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_checkBox_toggled(bool checked);

    void on_checkBox_2_toggled(bool checked);

private:
    Ui::CameraScan *ui;
    IP_Widget *ip = nullptr;                    /*!< [IP元件]輸入IP範圍 */
    RotatingWidget *loader = nullptr;           /*!< [loading元件]顯示載入中動畫 */
    QStandardItemModel *model = nullptr;        /*!< [表格model]掃描列表的model */
    QStandardItemModel *modelSelect = nullptr;        /*!< [表格model]掃描列表的model */
    CamVC610_WebAPI_Ctrl *camAPI = new CamVC610_WebAPI_Ctrl();
    QVector<CamInfo> camlist;                        /*!< [字串清單]已選取的相機IP清單 */
    QVector<CamInfo> cam_param_list;            /*!< [相機參數清單]掃描回傳相機參數資訊清單 */
    QString masterCam = "";
    QList<QStandardItem*> selectRows;
    void setComponentItems();
    void setupApp();
    bool StartScan = false;
};

#endif // CAMERASCAN_H
