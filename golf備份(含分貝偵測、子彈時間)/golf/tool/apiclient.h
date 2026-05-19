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

signals:
    void requestSucceeded(const QString &apiName, const QByteArray &responseData);
    void requestSucceededJson(const QString &apiName, const QJsonObject &responseObject);
    void requestFailed(const QString &apiName, const QString &errorString);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager *networkManager;
    QString IP = nullptr;
};

#endif // APICLIENT_H
