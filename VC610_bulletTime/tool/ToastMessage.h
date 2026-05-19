#ifndef TOASTMESSAGE_H
#define TOASTMESSAGE_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ToastMessage : public QDialog
{
    Q_OBJECT
public:
    explicit ToastMessage(QWidget *parent = nullptr, const QString &title = "", const QString &message = "");

    static void showMessage(QWidget *parent, const QString &title, const QString &message);

private:
    QLabel *label;
    QPushButton *okButton;
};

#endif // TOASTMESSAGE_H
