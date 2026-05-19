#include "jsonfilemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

QString JsonFileManager::configFilePath = "config.json"; // 預設路徑

void JsonFileManager::setConfigFilePath(const QString &filePath)
{
    configFilePath = filePath;
}

bool JsonFileManager::writeConfig(const QJsonObject &jsonObject)
{
    QFile file(configFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "[JsonFileManager] Failed to open file for writing:" << configFilePath;
        return false;
    }

    QJsonDocument doc(jsonObject);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonObject JsonFileManager::readConfig()
{
    QJsonObject jsonObject;
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[JsonFileManager] Failed to open file for reading:" << configFilePath;
        return jsonObject;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[JsonFileManager] Failed to parse JSON:" << error.errorString();
        return QJsonObject();
    }

    return doc.object();
}
