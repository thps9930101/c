#include "GraphicsViewEventFilter.h"

GraphicsViewEventFilter::GraphicsViewEventFilter(QObject *){

}

bool GraphicsViewEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    //qDebug() << event->type();
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        emit onMousePress(mouseEvent);
    }
    else if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        emit onMouseMove(mouseEvent);
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        emit onMouseRelease(mouseEvent);
    }
    else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        emit onKeyPress(keyEvent);
    }
    else if (event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = dynamic_cast<QResizeEvent*>(event);
        emit onResize(resizeEvent);
    }
    return QObject::eventFilter(obj, event);
}
