#ifndef SECWINDOW_H
#define SECWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "element/customgraphicsview.h"

namespace Ui {
class secWindow;
}

class secWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit secWindow(QWidget *parent = nullptr);
    ~secWindow();

    CustomGraphicsView *secScreen;
private:
    Ui::secWindow *ui;
};

#endif // SECWINDOW_H
