#include "pressevent.h"
#include <QApplication>
#include <QDebug>

PressEvent::PressEvent(QWidget *target, QObject *parent)
    : QObject(parent), m_targetWidget(target)
{
    if (m_targetWidget)
        m_targetWidget->installEventFilter(this);
}

void PressEvent::installOn(QWidget *target)
{
    new PressEvent(target, target); // 父物件設為 target，隨之銷毀
}

bool PressEvent::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_targetWidget)
        return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPos();

        if (!m_targetWidget->geometry().contains(globalPos)) {
            emit clickedOutside();
            qDebug() << "[PressEvent] Clicked outside target widget.";
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        emit keyPressed(keyEvent->key());
        emit keyTextPressed(keyEvent->text());
        emit keyCombinationPressed(keyEvent->modifiers(), keyEvent->key());

        qDebug() << "[PressEvent] Key pressed:" << keyEvent->key();
        qDebug() << "[PressEvent] Key Text:" << keyEvent->text();
        qDebug() << "[PressEvent] Modifiers:" << keyEvent->modifiers();
    }

    return false;
}
