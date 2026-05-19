#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


struct Record_param{
    QString record_path;
    QString frontName;
    QString backName;
    int sec;
    int record_type;

    Record_param()
    {
        record_type = 1;
    }

    static QJsonObject toObj(Record_param _record_param);
    static Record_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};
