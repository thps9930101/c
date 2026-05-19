#ifndef PRESSEVENT_H
#define PRESSEVENT_H

#include <QObject>
#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>

class PressEvent : public QObject
{
    Q_OBJECT
public:
    explicit PressEvent(QWidget *target, QObject *parent = nullptr);
    static void installOn(QWidget *target);

signals:
    void clickedOutside();
    void keyPressed(int key);
    void keyTextPressed(const QString &text);
    void keyCombinationPressed(Qt::KeyboardModifiers modifiers, int key);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QWidget *m_targetWidget;
};

#endif // PRESSEVENT_H
