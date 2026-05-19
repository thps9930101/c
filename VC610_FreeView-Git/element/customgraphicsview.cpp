#include "customgraphicsview.h"

void setOpenGL_widget(QOpenGLWidget* &widget)
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    format.setVersion(3, 3);

    widget = new QOpenGLWidget();
    widget->setFormat(format);
}

//===============

CustomGraphicsView::CustomGraphicsView(QGraphicsView* view, QWidget* parent)
{
    setOpenGL_widget(m_openGLWidget);
    setStyleSheet(parent->styleSheet());
    setGraphicView(view);

    m_scene = new QGraphicsScene();
    m_pixmap = new QGraphicsPixmapItem();
    m_pixmap->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    m_scene->addItem(m_pixmap);

    view->setScene(m_scene);
    view->setViewport(m_openGLWidget);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(view->scene(), &QGraphicsScene::focusItemChanged, this, &CustomGraphicsView::selectItemChange);


    GraphicsViewEventFilter *filter = new GraphicsViewEventFilter(this);
    connect(filter, &GraphicsViewEventFilter::onMousePress, this , &CustomGraphicsView::onMousePress);
    connect(filter, &GraphicsViewEventFilter::onMouseMove, this , &CustomGraphicsView::onMouseMove);
    connect(filter, &GraphicsViewEventFilter::onMouseRelease, this , &CustomGraphicsView::onMouseRelease);
    connect(filter, &GraphicsViewEventFilter::onKeyPress, this , &CustomGraphicsView::onKeyPress);
    view->scene()->installEventFilter(filter);

    infoItem = new QGraphicsTextItem();
    infoItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    infoItem->setDefaultTextColor(Qt::cyan);
    infoItem->setPlainText("");
    infoItem->setScale(1.5);
    infoItem->setVisible(false);

//    background = new QGraphicsRectItem(0,0,0,0);
//    background->setFlag(QGraphicsItem::ItemIgnoresTransformations);
//    background->setBrush(QBrush(QColor(200, 200, 200, 100))); // Set the background color
//    background->setVisible(false);

//    m_scene->addItem(background);
    m_scene->addItem(infoItem);
    GraphicsViewEventFilter *filter_view = new GraphicsViewEventFilter(this);
    connect(filter_view, &GraphicsViewEventFilter::onResize, this , &CustomGraphicsView::onResize);
    view->installEventFilter(filter_view);


    this->view = view;
    startTimer(20);
}

CustomGraphicsView::~CustomGraphicsView()
{

}

void CustomGraphicsView::setAllowAddItem(const bool& bln)
{
    isAllowAddItem = bln;
}

//void CustomGraphicsView::setOpenGL_widget(QOpenGLWidget* &widget)
//{
//    QSurfaceFormat format;
//    format.setRenderableType(QSurfaceFormat::OpenGL);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
//    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
//    format.setVersion(3, 3);

//    widget = new QOpenGLWidget();
//    widget->setFormat(format);
//}

void CustomGraphicsView::setGraphicView(QGraphicsView* &view)
{
    view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setBackgroundBrush(QColor(25, 25, 30));
}

// public Slot:
void CustomGraphicsView::timerEvent(QTimerEvent *)
{
    TimerReSize = isReSize;
}


void CustomGraphicsView::selectItemChange(QGraphicsItem* item)
{

}

void CustomGraphicsView::newFullScreen(int screenID)
{
    emit newScreenMenu(screenID);
}

void CustomGraphicsView::QueryDisplay()
{
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    UINT32 flags = QDC_ONLY_ACTIVE_PATHS;
    LONG isError = ERROR_INSUFFICIENT_BUFFER;

    UINT32 pathCount, modeCount;
    isError = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

    if( isError )
    {
        qDebug()<<"ERROR";
        return;
    }


    // Allocate the path and mode arrays
    paths.resize(pathCount);
    modes.resize(modeCount);

    // Get all active paths and their modes
    isError = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);


    if ( isError )
    {
        qDebug()<<"ERROR2";
        return;
    }

    // For each active path
    int len = paths.size();
    screenSelect.clear();

    for( int i=0 ; i<len ; i++ )
    {
        // Find the target (monitor) friendly name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);



        isError = DisplayConfigGetDeviceInfo(&targetName.header);

        if( isError )
        {
            qDebug()<<"ERROR3";
            return;
        }
        QString mon_name = "Display"+QString::number(len);

        if( targetName.flags.friendlyNameFromEdid )
        {
            mon_name = QString::fromStdWString(
                       targetName.monitorFriendlyDeviceName);

        }
        screenSelect.append(mon_name);
    }
}

void CustomGraphicsView::onMousePress(QGraphicsSceneMouseEvent *event)
{
    MousePressPos = this->view->mapFromScene(event->scenePos());
    QueryDisplay();
    if (event->button() == Qt::RightButton)
    {
        QueryDisplay();
        screenList.clear();
        QMenu *menu = new QMenu(this);

        QAction *FullscreenItem = menu->addAction("Fullscreen Projector");
        menu->addAction(FullscreenItem);

        screenMenu = new QMenu("Open Recent", this);
        FullscreenItem->setMenu(screenMenu);
        menu->addAction(FullscreenItem);

        for(int i =0;i<screenSelect.size();i++)
        {
            screenList.append(new QAction(screenSelect[i], this));
            screenMenu->addAction(screenList[i]);
            tmpInt = i;

            connect(screenList[i], &QAction::triggered, this, [this]() {
                QAction *action = qobject_cast<QAction*>(sender());
                QList<QAction*> actions = screenMenu->actions();
                int index = actions.indexOf(action);

                if (index != -1) {
                    qDebug() << "Index of QAction in QMenu:" << index;
                } else {
                    qDebug() << "QAction not found in QMenu.";
                }

                newFullScreen(index);
            });

        }

//        QAction *fileNewAction = new QAction("&New", this);
//        screenMenu->addAction(fileNewAction);

//        connect(fileNewAction, &QAction::triggered, this, &CustomGraphicsView::newFullScreen);

        MousePressPos.setX(MousePressPos.x()+5);
        MousePressPos.setY(MousePressPos.y()+5);
        // 顯示右鍵選單
        menu->exec(this->view->mapToGlobal(MousePressPos));
        delete menu;
    }
}


void CustomGraphicsView::onMouseMove(QGraphicsSceneMouseEvent *event)
{

}



void CustomGraphicsView::onResize(QResizeEvent* event)
{
    isReSize = true;

    if (isAllowAddItem)
        this->view->setSceneRect(0, 0, event->size().width(), event->size().height());
}

void CustomGraphicsView::onKeyPress(QKeyEvent* event)
{
    //qDebug() << event->key();
//    if(event->key() == Qt::Key_F11)
//    {
//        emit fullScreen();
//    }else if(event->key() == Qt::Key_R)
//    {
//        emit Record_signal(start_record);
//        start_record = !start_record;
//    }else if(event->key() == Qt::Key_F10)
//    {
//        emit newScreen();
//    }


//    if (!(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete))
//        return;

//    int i = -1;
}

void CustomGraphicsView::onMouseRelease(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        changeCamera = false;
    }
}

void CustomGraphicsView::setCamlist(QStringList camlist)
{
    this->camlist = camlist;
    this->RowCount = this->camlist.size();
}

bool CustomGraphicsView::inQWidget(QPoint event, QPointF TopLeft, QPointF BottomRight)
{
    QRectF widgetRect(TopLeft, BottomRight);
    return widgetRect.contains(event);
}

bool CustomGraphicsView::inQWidget(QPoint event, QGraphicsItem * widget)
{
    QPointF pos = this->view->mapFromScene(widget->pos());
    QRectF rect = widget->boundingRect();
    QPointF widgetTopLeft = pos;
    QPointF widgetBottomRight = pos + QPointF(rect.width(), rect.height());
    QRectF widgetRect(widgetTopLeft, widgetBottomRight);
    return widgetRect.contains(event);
}

void CustomGraphicsView::set_onKeyPress(int key)
{
    //qDebug() << event->key();
    if(key == 122)
    {
        emit fullScreen();
    }else if(key == 120)
    {
        emit Record_signal(start_record);
        start_record = !start_record;
    }else if(key == 121)
    {
        emit newScreen();
    }


    if (!(key == 32 || key == 46))
        return;

    int i = -1;
}

