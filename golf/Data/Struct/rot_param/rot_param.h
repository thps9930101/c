#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "Data/Enum/SP_Rot.h"
#include <vector>                      // 加入 vector


struct rot_param {
    SP_Rot sp_Rot;
    std::vector<SP_Rot> sp_rot;       // 改為 std::vector

    static QJsonObject toObj(rot_param _rot_param);
    static rot_param toStruct(QJsonObject obj);
    QJsonObject toObj();
    void setStruct(QJsonObject obj);
    static bool isMatch(QJsonObject obj);
};
