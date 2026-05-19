#ifndef PARAMFILEMANAGER_H
#define PARAMFILEMANAGER_H


#include "Data/Define/DefineData.h"
#include "Data/Struct/AppSettings/appSettings.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QApplication>



/**
 * @brief 參數檔類型枚舉
 */
enum ParamFile
{
    camSetting,     /*!< [相機API參數檔] */
    BT,             /*!< [子彈時間參數檔] */
    OutputFile,     /*!< [影片輸出參數檔] */
    AppSetting
};

/**
 * @brief 參數檔管理器
 */
class ParamFileManager
{

public:
    /**
     * @brief 參數檔管理器
     */
    ParamFileManager();

    QJsonObject getObjectFromFile(QString fileName);

    QString AppSetting2Json(const QString lang, const QString fontSize, const int buffSec, QList<QString> IpRangeList);

    /**
     * @brief [影片輸出參數]轉為[Json字串]
     * 
     * @param dir 目錄位置
     * @param prefix 檔案前綴
     * @param suffix 檔案後綴
     * 
     * @return Json字串
     */
    QString Output_Param2Json(const QString dir, const QString prefix, const QString suffix);


    /**
     * @brief [Json字串]轉為[影片輸出參數]
     * 
     * @param jsonString Json格式字串
     * 
     * @return 影片輸出參數
     */
    QStringList Json2Output_Param(const QString jsonString);

    /**
     * @brief 從參數檔取得[影片輸出參數]
     * 
     * @return 影片輸出參數
     */
    QStringList getOutput_ParamFromFile();


    /**
     * @brief [子彈時間參數]轉為[Json字串]
     * 
     * @param BT_Param 子彈時間參數
     * 
     * @return Json字串
     */
//    QString BT_Param2Json(const bullet_time_param& BT_Param);

    /**
     * @brief [子彈時間參數]轉為[Json字串]
     * 
     * @param BT_Param 子彈時間參數
     * @param VideoFrameCount 子彈時間前後撥放時間
     * 
     * @return Json字串
     */
//    QString BT_Param2Json(const bullet_time_param& BT_Param, const int VideoFrameCount, bool isEnable = true);

    /**
     * @brief 從參數檔取得[子彈時間前後撥放時間]
     */
    int getVideoFrameCountFromFile();

    bool getIsAllowFilterFromFile();

    /**
     * @brief 建立參數檔，並寫入[Json字串]
     * 
     * @param path 建立參數檔位置
     * @param context Json字串
     */
    void createFile(QString path, QString context);

    /**
     * @brief 將[Json字串]寫入參數檔
     * 
     * @param FILE 寫入哪個參數檔
     * @param context Json字串
     */
    void writeToFile(ParamFile FILE, QString context);
    void writeToFile(QString path, QString context);

    void writeToPath(QString path, QString context);

    /**
     * @brief 確保參數檔全部建立完成
     */
    int checkResource();

    QString getFileString(QString fileName);

    std::string extractMainFileName(const std::string& path);
    QString getDefultTrajFile();
private:

    const QString template_bt_argFile = "templateArg_bt.txt";                       /*!< 預設[子彈時間參數]參數檔檔名 */
    const QString template_outputParam_argFile = "templateArg_outputFile.txt";      /*!< 預設[影片輸出參數]參數檔檔名 */
    const QString template_camSetting_argFile = "templateArg_camSetting.txt";       /*!< 預設[相機API參數]參數檔檔名 */
    QString TrajectoryFilePath  = "\\TrajectorySet_arg.txt";    /*!< [軌跡]參數檔檔名 */

    QString CamSettingFilePath;     /*!< [相機API參數]參數檔檔名 */
    QString BtFilePath;             /*!< [子彈時間參數]參數檔檔名 */
    QString OutputFilePath;         /*!< [影片輸出參數]參數檔檔名 */
    QString AppSettingFilePath;     /*!< [相機API參數]參數檔檔名 */
};

#endif // PARAMFILEMANAGER_H
