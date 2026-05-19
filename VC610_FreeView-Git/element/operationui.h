#ifndef OPERATIONUI_H
#define OPERATIONUI_H

#include <QDockWidget>

namespace Ui {
class operationUI;
}

class operationUI : public QDockWidget
{
    Q_OBJECT

public:
    explicit operationUI(QWidget *parent = nullptr);
    ~operationUI();

private:
    Ui::operationUI *ui;
};

#endif // OPERATIONUI_H
