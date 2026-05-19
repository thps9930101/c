#include "customqthread.h"



FileThread::FileThread(QWidget* parent, QString title, QString filter)
{
    this->parent = parent;
    this->title = title;
    this->filter = filter;
};

FileThread::~FileThread()
{

};

void FileThread::run()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(parent, title, "", filter);
    emit onOpen(fileNames);
    this->deleteLater();
};


FrameThread::FrameThread(int FPS)
{
    this->sleepMs = 1000 / FPS;
};

FrameThread::~FrameThread()
{

};

void FrameThread::run()
{
    while (isStart) {
        msleep(sleepMs);
        if (!isStart)
            break;

        emit refreash();
    }

    this->deleteLater();
};

void FrameThread::setStart(bool bln)
{
    isStart = bln;
}

void FrameThread::setFPS(int fps)
{
    this->sleepMs = 1000 / fps;
}
//---------------

//-----------------

RotatingWidget::RotatingWidget(QWidget *parent): QWidget(parent)
{
    // 设置背景颜色为白色
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::transparent);
    setPalette(palette);

    // 启动定时器
    startTimer(30);
}

void RotatingWidget::setEnable(bool enable)
{
    isEnable = enable;
}

void RotatingWidget::paintEvent(QPaintEvent *)
{
    // 创建一个QPainter对象，用于绘制旋转后的widget
    QPainter painter(this);

    // 将坐标系原点移到widget的中心
    painter.translate(width() / 2, height() / 2);

    // 旋转坐标系
    painter.rotate(m_rotationAngle);

    // 绘制一个3/4的圆
    int penWidth;

    if(width() >= height())
        penWidth = height() / 5;
    else
        penWidth = width() / 5;

    painter.setPen(QPen(Qt::cyan, penWidth));
    int radius = qMin(width(), height()) * 3 / 8;
    QRectF rect(-radius, -radius, 2 * radius, 2 * radius);
    painter.drawArc(rect, 0, 270 * 16);
}

void RotatingWidget::timerEvent(QTimerEvent *)
{
    if (!isEnable)
        return;

    // 更新旋转角度
    m_rotationAngle += 20;

    // 防止旋转角度溢出
    if (m_rotationAngle >= 360) {
        m_rotationAngle -= 360;
    }

    // 触发重新绘制
    update();
}



