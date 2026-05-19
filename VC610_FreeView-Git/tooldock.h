#ifndef TOOLDOCK_H
#define TOOLDOCK_H

#include <QDockWidget>

namespace Ui {
class ToolDock;
}

class ToolDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ToolDock(QWidget *parent = nullptr);
    ~ToolDock();

private:
    Ui::ToolDock *ui;
};

#endif // TOOLDOCK_H
