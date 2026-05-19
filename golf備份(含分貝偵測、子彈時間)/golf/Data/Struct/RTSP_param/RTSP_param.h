#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


struct RTSP_param{
    QString rtsp_path;
    QString rtmp_path;
    int service;
    RTSP_param()
    {
        service = 0;
        rtsp_path = "rtsp://127.0.0.1:8554/stream";
        rtmp_path = "rtmp://127.0.0.1:1935/stream";
    }

    static QJsonObject toObj(RTSP_param _rtsp_param);
    static RTSP_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};
