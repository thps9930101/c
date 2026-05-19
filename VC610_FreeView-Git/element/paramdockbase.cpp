#include "paramdockbase.h"
#include "ui_paramdockbase.h"

ParamDockBase::ParamDockBase(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ParamDockBase)
{
    ui->setupUi(this);


    setWidget(viewer);


    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, [=] () {
        if (getObjFunc)
        {
            auto obj = getObjFunc();
            viewer->updateJsonObject(getObjFunc());
//            displayJsonObject(obj);
        }
    });
    updateTimer->start(100);
}

void ParamDockBase::setObj(std::function<QJsonObject()> getObj)
{
    getObjFunc = getObj;
    viewer->setJsonObject(getObjFunc());
}

void ParamDockBase::displayJsonObject(const QJsonObject &jsonObject)
{
    // 获取所有键
    QStringList keys = jsonObject.keys();

    // 遍历键并显示对应的值
    for (const QString &key : keys)
    {
        QJsonValue value = jsonObject.value(key);

        // 判断值的类型
        if (value.isObject())
        {
            qDebug() << "Object key:" << key;
            displayJsonObject(value.toObject());
        } else if (value.isArray())
        {
            qDebug() << "Array key:" << key;
            displayJsonArray(value.toArray());
        } else
        {
            if (value.isBool())
                qDebug() << "Key:" << key << ", Value:" << value.toBool();
            else if (value.isString())
                qDebug() << "Key:" << key << ", Value:" << value.toString();
            else if (value.isDouble())
                qDebug() << "Key:" << key << ", Value:" << value.toInt();
        }
    }

}

void ParamDockBase::displayJsonArray(QJsonArray jsonArray)
{
    // 遍历数组并显示对应的值
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonValue value = jsonArray.at(i);

        // 判断值的类型
        if (value.isObject()) {
            qDebug() << "Object in array at index" << i;
            displayJsonObject(value.toObject());
        } else if (value.isArray()) {
            qDebug() << "Array in array at index" << i;
            displayJsonArray(value.toArray());
        } else {
            qDebug() << "Value in array at index" << i << ":" << value.toString();
        }
    }

}

ParamDockBase::~ParamDockBase()
{
    delete ui;
}
