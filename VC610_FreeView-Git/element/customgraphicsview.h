#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QWidget>
#include <QMenu>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QOpenGLWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QCoreApplication>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
//#include <QOpenGLContext>
//#include <QGraphicsSceneEvent>
//#include <QGraphicsColorizeEffect>
#include "GraphicsViewEventFilter.h"


//#include "mode/imageoverlayer.h"

#include <QDebug>
#include <thread>
//#include <QGraphicsTextItem>
//#include <QPainter>
//#include <QFontMetrics>



class CustomGraphicsView : public QWidget
{
    Q_OBJECT
public:
    CustomGraphicsView(QGraphicsView* view, QWidget* parent = nullptr) ;
    ~CustomGraphicsView();

    void selectItemChange(QGraphicsItem* item);

    void setGraphicView(QGraphicsView* &view);

    void setAllowAddItem(const bool& bln);

    void setCamlist(QStringList camlist);
    void onAddStrikeZone();
    void onAddMask();
    void MaskComplite()
    {
        inMode_addMask = false;
    }
    void newFullScreen(int);
    void QueryDisplay();

signals:
    void cameraSwitch(int);
    void fullScreen();
    void Record_signal(bool);
    void newScreen();
    void newScreenMenu(int);
public:
    void timerEvent(QTimerEvent *);
    void setCenter()
    {
        qreal scaleFactor = qMin(static_cast<qreal>(this->view->width()) / m_pixmap->pixmap().width(),
                                  static_cast<qreal>(this->view->height()) / m_pixmap->pixmap().height());

        m_pixmap->setScale(scaleFactor);
        QPointF center = this->view->mapToScene(this->view->viewport()->rect().center());
        QPointF pixmapCenter = m_pixmap->boundingRect().center();

        pixmapCenter.setX(pixmapCenter.x() * scaleFactor);
        pixmapCenter.setY(pixmapCenter.y() * scaleFactor);

        m_pixmap->setPos(center - pixmapCenter);
        infoItem->setPos(m_pixmap->pos());
//        background->setRect(infoItem->boundingRect());
        view->setSceneRect(0, 0, view->size().width(), view->size().height());

    }

    void setInfoVisible(bool visible)
    {
        infoItem->setVisible(visible);
//        background->setVisible(visible);
    }

    void setInfo(QString info)
    {
        infoItem->setPlainText(info);
    }

    void setScale(const double& scale)
    {
        this->scale = scale;
    }


    int CustomGraphicsView::getRow()
    {
        return row;
    }

    bool inQWidget(QPoint, QPointF, QPointF);
    bool inQWidget(QPoint event, QGraphicsItem * widget);

public:
    QGraphicsView* view;
    QGraphicsPixmapItem* m_pixmap;

    double scale = 1;                           /*!< 縮放比例 */
    bool TimerReSize = false;                      /*!< 使否變更大小 */
    bool isReSize = false;                      /*!< 使否變更大小 */
    bool isAllowAddItem = false;                /*!< 是否允許新增子項目 */

    bool allow_addMask = false;
    bool inMode_addMask = false;
public slots:
    void onResize(QResizeEvent*);

    void onKeyPress(QKeyEvent * event);

    void onMouseRelease(QGraphicsSceneMouseEvent *);
    void onMousePress(QGraphicsSceneMouseEvent *event);
    void onMouseMove(QGraphicsSceneMouseEvent *event);
    void set_onKeyPress(int);

private:

    QGraphicsScene* m_scene;
    QOpenGLWidget* m_openGLWidget;

    QGraphicsTextItem* infoItem;
//    QGraphicsRectItem* background;


    QStringList camlist;
    QPoint MousePressPos;                       /*!< 滑鼠點擊場景相對位置 */
    int row = 0;                                /*!< 目前選取相機編號 */
    int RowCount = 0;                           /*!< 相機清單比數 */

    float mouse_Old_X;
    bool changeCamera = false;
    bool start_record = false;

    int tmpInt = 0;
    QList<QString> screenSelect;
    QList<QAction *> screenList;
    QMenu *screenMenu;
};

#endif // CUSTOMGRAPHICSVIEW_H
