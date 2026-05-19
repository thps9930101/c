#ifndef JSONFILEMANAGER_H
#define JSONFILEMANAGER_H

#include <QString>
#include <QJsonObject>

class JsonFileManager
{
public:
    // 設定設定檔路徑
    static void setConfigFilePath(const QString &filePath);

    // 寫入設定 JSON
    static bool writeConfig(const QJsonObject &jsonObject);

    // 讀取設定 JSON
    static QJsonObject readConfig();

private:
    static QString configFilePath;
};

#endif // JSONFILEMANAGER_H
