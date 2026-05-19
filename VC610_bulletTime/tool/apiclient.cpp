#include "apiclient.h"
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QDir>

ApiClient::ApiClient(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
}

void ApiClient::setIP(QString setIP)
{
    IP = setIP;
}

void ApiClient::postJson(const QUrl &url, const QJsonObject &jsonData, const QString &apiName)
{
    int timeoutMs = 5000;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument jsonDoc(jsonData);
    QByteArray jsonDataBytes = jsonDoc.toJson();

    QNetworkReply *reply = networkManager->post(request, jsonDataBytes);

    // 存原始請求資料（方便重發）
    QVariantMap reqInfo;
    reqInfo["url"] = url.toString();
    reqInfo["json"] = QString(jsonDataBytes);
    reqInfo["apiName"] = apiName;
    reply->setProperty("reqInfo", reqInfo);

//    reply->setProperty("apiName", apiName);
    QTimer *timer = new QTimer(reply); // 設 parent → reply 結束會自動清掉
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply]() {
        if (reply->isRunning()) {
            qWarning() << "⚠️ API Timeout:" << reply->property("reqInfo").toMap()["apiName"].toString();
            reply->abort();  // 強制中斷
//            emit requestFailed(reply->property("reqInfo").toMap()["apiName"].toString(), "Timeout");
        }
    });
    timer->start(timeoutMs);

    connect(reply, &QNetworkReply::finished, this, &ApiClient::onReplyFinished);
}

void ApiClient::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    QVariantMap reqInfo = reply->property("reqInfo").toMap();
    QString apiName = reqInfo["apiName"].toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);
        qDebug().noquote() << "回傳訊息:" << jsonDoc.toJson(QJsonDocument::Indented);

        if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            QString stateValue = jsonObj.value("state").toString();

            // ✅ 如果有 id → 呼叫新的 API 查詢
            if (jsonObj.contains("id") && jsonObj["id"].isString()) {
                QString idValue = jsonObj["id"].toString();

                if (stateValue != "done") {
                    qDebug() << "偵測到 ID:" << idValue << " 狀態:" << stateValue << " → 重新呼叫 API";
                    getStatusById(idValue, reqInfo); // 重新查詢
                    reply->deleteLater();
                    return; // 直接結束，不進入後續處理
                } else {
                    qDebug() << "偵測到 ID:" << idValue << " 狀態已完成 → 正常回傳";
                    emit requestSucceededJson(apiName, jsonObj);
                    emit requestSucceeded(apiName, response);
                    reply->deleteLater();
                    return;
                }
            }

            // ✅ 如果沒有 id → 視為成功（不判斷 state）
            emit requestSucceededJson(apiName, jsonObj);
            emit requestSucceeded(apiName, response);

        } else {
            emit requestFailed(apiName, "JSON parse error: " + parseError.errorString());
        }
    } else {
        emit requestFailed(apiName, reply->errorString());
    }

    reply->deleteLater();
}

// 新增一個 API 呼叫函數
void ApiClient::getStatusById(const QString& id, QVariantMap reqInfo)
{
    QJsonObject optionsObj;
    optionsObj.insert("id", id);

    QString fullUrl = reqInfo["url"].toString();
    QUrl qurl(fullUrl);
    QString targetIP = qurl.host();
    QUrl url("http://" + targetIP + "/ctrl/status");

    QJsonDocument jsonDoc(optionsObj);
    QByteArray jsonDataBytes = jsonDoc.toJson();

    // 建立請求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 發送 GET
    QNetworkReply *reply = networkManager->post(request, jsonDataBytes);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "解析狀態回應失敗:" << parseError.errorString();
        } else {
            QJsonObject obj = jsonDoc.object();
            QString stateValue = obj.value("state").toString();
            qDebug() << "[Status API]" << stateValue;

            if (stateValue != "done") {
                // 500ms 後重試
                if(stateValue == "error")
                {
                    emit requestFailed(reqInfo["apiName"].toString(), "JSON parse error: " + parseError.errorString());
                    return;
                }

                QTimer::singleShot(500, this, [=]() {
                    getStatusById(id, reqInfo);
                });
            } else {
                qDebug() << "狀態已完成";
                emit requestSucceededJson(reqInfo["apiName"].toString(), obj);
            }
        }

        reply->deleteLater();
    });
}

//void ApiClient::camSet(const QString &ip, const QJsonObject &jsonData)
//{
//    int delayMs = 500; // 每次 API 呼叫的間隔時間（毫秒），可改成參數傳入

//    // 取得所有 JSON 的 key，例如：exposureTime、iso、ev...
//    QStringList keys = jsonData.keys();

//    for (int i = 0; i < keys.size(); ++i) {
//        const QString &key = keys[i];

//        QJsonObject optionsObj;
//        optionsObj.insert(key, jsonData.value(key));

//        QJsonObject parametersObj;
//        parametersObj.insert("options", optionsObj);

//        QJsonObject finalObj;
//        finalObj.insert("name", "setOption");
//        finalObj.insert("parameters", parametersObj);

//        // 建立 URL
//        QUrl url(QString("http://%1/ctrl/execute").arg(ip));
//        QNetworkRequest request(url);
//        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

//        // 轉換為 JSON 串
//        QJsonDocument jsonDoc(finalObj);
//        QByteArray jsonDataBytes = jsonDoc.toJson();

//        // 發送 POST 請求
//        QNetworkReply *reply = networkManager->post(request, jsonDataBytes);

//        connect(reply, &QNetworkReply::finished, this, [reply, key]() {
//            if (reply->error() == QNetworkReply::NoError) {
//                QByteArray response = reply->readAll();
//                qDebug() << QString("[camSet] '%1' Success:").arg(key) << response;
//            } else {
//                qWarning() << QString("[camSet] '%1' Error:").arg(key) << reply->errorString();
//            }
//            reply->deleteLater();
//        });

//        // 延遲 delayMs 毫秒再送下一個
//        QEventLoop loop;
//        QTimer::singleShot(delayMs, &loop, &QEventLoop::quit);
//        loop.exec();
//    }
//}

void ApiClient::camSet(const QString &ip, const QJsonObject &jsonData)
{
    int delayMs = 500; // 每次 API 呼叫的間隔時間（毫秒），可改成參數傳入

    // 取得所有 JSON 的 key，例如：exposureTime、iso、ev...
    QStringList keys = jsonData.keys();

    for (int i = 0; i < keys.size(); ++i) {
        const QString &key = keys[i];

        QJsonObject optionsObj;
        optionsObj.insert(key, jsonData.value(key));

        QJsonObject parametersObj;
        parametersObj.insert("options", optionsObj);

        QJsonObject finalObj;
        finalObj.insert("name", "setOption");
        finalObj.insert("parameters", parametersObj);

        // API endpoint
        QUrl url(QString("http://%1/ctrl/execute").arg(ip));

        // ✅ 改成用 postJson 傳送，apiName 就放 key
        postJson(url, finalObj, QString("camSet_%1").arg(key));

        // 延遲 delayMs 毫秒再送下一個
        QEventLoop loop;
        QTimer::singleShot(delayMs, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

void ApiClient::timeSet(QString targetIP)
{
//    std::string time = getCurrentDateTime().toStdString();
    QJsonObject optionsObj;
    optionsObj.insert("dateTime", getCurrentDateTime());

    QJsonObject parametersObj;
    parametersObj.insert("options", optionsObj);

    QJsonObject finalObj;
    finalObj.insert("name", "setOption");
    finalObj.insert("parameters", parametersObj);
    QUrl url("http://" + targetIP + "/ctrl/execute");

    postJson(url, finalObj, "timeset");
}

QString ApiClient::getCurrentDateTime()
{
    // 取得當前的日期和時間
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 將日期和時間格式化為指定的字串格式
    QString formattedDateTime = currentDateTime.toString("yyyy/MM/dd hh:mm:ss");

    return formattedDateTime;
}

void ApiClient::startRec(const QList<QString> &ipList)
{
    // 包裝成 parameters
    QJsonObject parametersObj;
    parametersObj.insert("name", "startRec");

    // 對每個 IP 各發送一次 POST
    for (const QString &ip : ipList)
    {
        QUrl url("http://" + ip + "/ctrl/execute");
        postJson(url, parametersObj, "startRec");
    }
}

void ApiClient::stopRec(const QList<QString> &ipList)
{
    // 包裝成 parameters
    QJsonObject parametersObj;
    parametersObj.insert("name", "stopRec");

    // 對每個 IP 各發送一次 POST
    for (const QString &ip : ipList)
    {
        QUrl url("http://" + ip + "/ctrl/execute");
        postJson(url, parametersObj, "stopRec");
    }
}

void ApiClient::clearVideo(const QList<QString> &ipList)
{
    // 包裝成 parameters
    QJsonObject parametersObj;
    parametersObj.insert("name", "deleteDcim");

    // 對每個 IP 各發送一次 POST
    for (const QString &ip : ipList)
    {
        QUrl url("http://" + ip + "/ctrl/execute");
        postJson(url, parametersObj, "deleteDcim");
    }
}

void ApiClient::listFiles(const QList<QString> &ipList)
{
    QJsonObject optionsObj;
    optionsObj.insert("fileType", "all");

    // 包裝成 parameters
    QJsonObject parametersObj;
    parametersObj.insert("name", "listFiles");
    parametersObj.insert("parameters", optionsObj);


    // 對每個 IP 各發送一次 POST
    for (const QString &ip : ipList)
    {
        QUrl url("http://" + ip + "/ctrl/execute");
        postJson(url, parametersObj, "listFiles");
    }
}

void ApiClient::getStatus(const QList<QString> &ipList, QString id)
{
    // 包裝成 parameters
    QJsonObject parametersObj;
    qDebug()<<"id:"<<id;
    parametersObj.insert("id", id);


    // 對每個 IP 各發送一次 POST
    for (const QString &ip : ipList)
    {
        QUrl url("http://" + ip + "/ctrl/status");
        postJson(url, parametersObj, "getStatus");
    }
}

void ApiClient::downloadFiles(const QList<QString> &ipList, const QString &remotePath, const QString &saveDir)
{
    for (const QString &ip : ipList)
    {
//        QString fileName = remotePath.split("/").last();
//        QString savePath = saveDir + "/" + ip + "_" + fileName;
        qDebug()<<saveDir;
        downloadFile(ip, remotePath, saveDir);
    }
}

void ApiClient::downloadFile(const QString &ip, const QString &remotePath, const QString &savePath)
{
    // -----------------------------
    // ① 建立資料夾（若存在則略過）
    // -----------------------------
    QFileInfo info(savePath);
    QString folder = info.absolutePath();

    QDir dir;
    if (!dir.exists(folder)) {
        if (!dir.mkpath(folder)) {
            emit downloadFailed(ip, "無法建立資料夾：" + folder);
            return;
        } else {
            qDebug() << "📁 已建立資料夾:" << folder;
        }
    }

    // -----------------------------
    // ② 準備下載
    // -----------------------------
    QUrl url(remotePath);
    qDebug() << "下載 URL:" << url;

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    // 保存下載資訊
    QVariantMap reqInfo;
    reqInfo["ip"] = ip;
    reqInfo["savePath"] = savePath;
    reply->setProperty("reqInfo", reqInfo);

    // 建立檔案
    QFile *file = new QFile(savePath);
    if (!file->open(QIODevice::WriteOnly))
    {
        qDebug()<<"test1234:"<<savePath;
        emit downloadFailed(ip, "無法寫入檔案：" + savePath);
        delete file;
        reply->abort();
        reply->deleteLater();
        return;
    }

    reply->setProperty("filePtr", QVariant::fromValue<void*>(file));

    // -----------------------------
    // ③ 下載逾時處理
    // -----------------------------
    int timeoutMs = 15000;
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply]() {
        QVariantMap info = reply->property("reqInfo").toMap();
        QString ip = info["ip"].toString();
        emit downloadFailed(ip, "下載逾時");
        reply->abort();
    });
    timer->start(timeoutMs);

    // -----------------------------
    // ④ 寫入資料片段
    // -----------------------------
    connect(reply, &QNetworkReply::readyRead, this, [reply]() {
        QFile *file = static_cast<QFile*>(reply->property("filePtr").value<void*>());
        if (file)
            file->write(reply->readAll());
    });

    // -----------------------------
    // ⑤ 下載進度
    // -----------------------------
    connect(reply, &QNetworkReply::downloadProgress,
            this, [this, reply](qint64 received, qint64 total) {
        if (total > 0)
        {
            QVariantMap info = reply->property("reqInfo").toMap();
            QString ip = info["ip"].toString();
            int percent = received * 100 / total;
            emit downloadProgress(ip, percent);
        }
    });

    // -----------------------------
    // ⑥ 下載完成
    // -----------------------------
    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() {
        QVariantMap info = reply->property("reqInfo").toMap();
        QString ip = info["ip"].toString();
        QString savePath = info["savePath"].toString();

        QFile *file = static_cast<QFile*>(reply->property("filePtr").value<void*>());

        if (file)
        {
            file->write(reply->readAll());
            file->close();
            delete file;
        }

        if (reply->error() == QNetworkReply::NoError)
            emit downloadFinished(ip, savePath);
        else
            emit downloadFailed(ip, reply->errorString());

        reply->deleteLater();
    });
}


bool createFolder(const QString &path)
{
    QDir dir;

    // 如果資料夾已存在 → 回傳 true
    if (dir.exists(path))
        return true;

    // 嘗試建立多層資料夾
    bool ok = dir.mkpath(path);

    if (!ok) {
        qWarning() << "❌ 無法建立資料夾:" << path;
    } else {
        qDebug() << "📁 建立資料夾成功:" << path;
    }

    return ok;
}
