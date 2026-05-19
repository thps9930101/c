#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);
    void postJson(const QUrl &url, const QJsonObject &jsonData, const QString&);
    void camSet(const QString &ip, const QJsonObject &jsonData);
    void setIP(QString);
    QString getCurrentDateTime();
    void timeSet(QString);
    void getStatusById(const QString&, QVariantMap);
    void startRec(const QList<QString> &);
    void stopRec(const QList<QString> &);
    void clearVideo(const QList<QString> &);
    void listFiles(const QList<QString> &);
    void downloadFiles(const QList<QString> &ipList, const QString &remotePath, const QString &saveDir);
    void downloadFile(const QString &ip, const QString &remotePath, const QString &savePath);
    void getStatus(const QList<QString> &, QString);

signals:
    void requestSucceeded(const QString &apiName, const QByteArray &responseData);
    void requestSucceededJson(const QString &apiName, const QJsonObject &responseObject);
    void requestFailed(const QString &apiName, const QString &errorString);
    void downloadProgress(QString ip, int percent);
    void downloadFinished(QString ip, QString savePath);
    void downloadFailed(QString ip, QString error);

private slots:
    void onReplyFinished();

private:
    static bool createFolder(const QString &);

    QNetworkAccessManager *networkManager;
    QString IP = nullptr;
};

#endif // APICLIENT_H
