#ifndef AOCAMSETTING_H
#define AOCAMSETTING_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include "Data/Struct/AoFunSetting/AoFunSetting.h"

namespace Ui {
class AoCamSetting;
}


class AoCamSetting : public QMainWindow
{
    Q_OBJECT
signals:
//    void cantGetResult();
    void confirmBtn_signal(AoFunSetting);

public:
    explicit AoCamSetting(QWidget *parent = nullptr);
    ~AoCamSetting();

    void setUIValue(AoFunSetting);
    AoFunSetting get_UI_set();

    void wirteParamFile(QJsonObject content, QString fileName);
    QString readFileToString(QString name);
private slots:
    void on_confirmBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::AoCamSetting *ui;
    AoFunSetting aoSetting;
};

#endif // AOCAMSETTING_H
