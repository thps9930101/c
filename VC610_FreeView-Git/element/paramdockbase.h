#ifndef PARAMDOCKBASE_H
#define PARAMDOCKBASE_H


#include <QJsonObject>
#include <QJsonArray>
#include <QDockWidget>
#include <QtDebug>
#include <QTimer>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QLabel>
#include <QLineEdit>

class JsonObjectViewer : public QWidget {
    Q_OBJECT

public:
    JsonObjectViewer(QWidget* parent = nullptr) : QWidget(parent) {
        layout = new QVBoxLayout(this);
        setLayout(layout);
    }

    void setJsonObject(const QJsonObject& jsonObject) {
        clear();
        addJsonObjectToLayout(jsonObject);
    }

    void updateJsonObject(const QJsonObject& jsonObject) {
        updateJsonObjectInLayout(jsonObject);
    }

private:
    QVBoxLayout* layout;
    QMap<QLabel*, QWidget*> keyToWidgetMap;

    void addJsonObjectToLayout(const QJsonObject& jsonObject) {
        for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
            QLabel* keyLabel = new QLabel(it.key() + ":", this);
            QWidget* valueWidget;

            if (it.value().isObject()) {
                JsonObjectViewer* nestedViewer = new JsonObjectViewer(this);
                nestedViewer->setJsonObject(it.value().toObject());
                valueWidget = nestedViewer;
            } else {
                QLabel* valueLabel = new QLabel(valueToString(it.value()), this);
                valueWidget = valueLabel;
            }

            QHBoxLayout* lay = new QHBoxLayout();
            lay->addWidget(keyLabel);
            lay->addWidget(valueWidget);

            keyToWidgetMap[keyLabel] = valueWidget;

            layout->addLayout(lay);
        }
    }

    void updateJsonObjectInLayout(const QJsonObject& jsonObject) {
        for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
            QLabel* keyLabel = getKeyLabel(it.key());
            if (keyLabel) {
                QWidget* valueWidget = keyToWidgetMap[keyLabel];
                if (valueWidget) {
                    if (auto nestedViewer = qobject_cast<JsonObjectViewer*>(valueWidget)) {
                        // Update nested JsonObjectViewer
                        nestedViewer->updateJsonObject(it.value().toObject());
                    } else if (auto valueLabel = qobject_cast<QLabel*>(valueWidget)) {
                        // Update QLabel
                        valueLabel->setText(valueToString(it.value()));
                    }
                }
            }

            if (it.value().isObject()) {
                updateJsonObjectInLayout(it.value().toObject());
            }
        }
    }

    void clear() {
        // Clear existing widgets from layout
        QLayoutItem* child;
        while ((child = layout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        keyToWidgetMap.clear();
    }

    QLabel* getKeyLabel(const QString& key) {
        for (auto label : keyToWidgetMap.keys()) {
            if (label->text().startsWith(key + ":")) {
                return label;
            }
        }
        return nullptr;
    }

    QString valueToString(const QJsonValue& value) {
        switch (value.type()) {
        case QJsonValue::String:
            return value.toString();
        case QJsonValue::Double:
            return QString::number(value.toDouble());
        case QJsonValue::Bool:
            return value.toBool() ? "true" : "false";
        case QJsonValue::Null:
            return "null";
        default:
            return ""; // Handle other types as needed
        }
    }
};


namespace Ui {
class ParamDockBase;
}

class ParamDockBase : public QDockWidget
{
    Q_OBJECT

public:
    explicit ParamDockBase(QWidget *parent = nullptr);
    ~ParamDockBase();

    void setObj(std::function<QJsonObject()> getObj);
    void displayJsonObject(const QJsonObject &jsonObject);
    void displayJsonArray(QJsonArray jsonArray);

private:
    Ui::ParamDockBase *ui;
    QTimer* updateTimer;
    QJsonObject obj;
    std::function<QJsonObject()> getObjFunc;
    JsonObjectViewer* viewer = new JsonObjectViewer(this);
};
#endif // PARAMDOCKBASE_H
