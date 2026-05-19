#ifndef GRAPHICSVIEWEVENTFILTER_H
#define GRAPHICSVIEWEVENTFILTER_H

#include <QGraphicsSceneEvent>
#include <QMouseEvent>

class GraphicsViewEventFilter : public QObject
{
    Q_OBJECT
signals:
    void onMousePress(QGraphicsSceneMouseEvent*);
    void onMouseMove(QGraphicsSceneMouseEvent*);
    void onMouseRelease(QGraphicsSceneMouseEvent*);
    void onKeyPress(QKeyEvent*);
    void onResize(QResizeEvent*);
public:
    explicit GraphicsViewEventFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // GRAPHICSVIEWEVENTFILTER_H
