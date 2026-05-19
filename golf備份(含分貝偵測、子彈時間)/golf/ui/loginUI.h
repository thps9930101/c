#ifndef LOIGN_H
#define LOIGN_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QFrame;

class Loign
{
public:
    Loign();
};
namespace LoginUI {
    void setupLoginUI(QDialog *dialog, QLineEdit *&username, QLineEdit *&password, QPushButton *&loginButton);
}
#endif // LOIGN_H
