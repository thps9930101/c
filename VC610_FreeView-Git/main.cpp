#include "mainwindow.h"
#include <Windows.h>
#include <QApplication>

//全域key
HHOOK keyboardHook = nullptr;
MainWindow* Main = nullptr;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            KBDLLHOOKSTRUCT* pKey = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            int key = pKey->vkCode;
            Main->getkeyboard(key);
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString theme = "DarkMaroon";
    QFile theme_file("./qss/template.qss");
    theme_file.open(QFile::ReadOnly);   //open theme file

    if(theme_file.isOpen())
    {
        a.setStyleSheet(theme_file.readAll());
        theme_file.close();
    }
    else
    {
        qDebug("File couldn't be opened!");
    }

    qRegisterMetaType<Decoder_class_param>("Decoder_class_param");
    qRegisterMetaType<Encoder_param>("Encoder_param");
    qRegisterMetaType<QVector<CamInfo>>("QVector<CamInfo>");
    qRegisterMetaType<Cam_Set>("Cam_Set");
    qRegisterMetaType<AoFunSetting>("AoFunSetting");
    qRegisterMetaType<App>("App");
    qRegisterMetaType<SP_MODE>("SP_MODE");



    MainWindow w;


//    if(screens.size()>0)
//    {

//        QScreen *secondScreen = screens.at(0);
//        QRect secondScreenGeometry = secondScreen->geometry();

//        // 将窗口移动到第二个屏幕的中心
//        w.setGeometry(secondScreenGeometry);
//    }


    w.show();
    Main = &w;
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0 );

    if (!keyboardHook)
    {
      return 1;
    }

//    QList<int> resolutionList = QList<int>();
//    int max = [=](){
//        int m = -1;
//        for (int i = 0; i < resolutionList.size(); ++i) {
//            m = qMax(m, resolutionList[i]);
//        }
//        return m;
//    }();

    return a.exec();
}
