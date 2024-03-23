#include "volleyapplication.h"

#include <QApplication>

// https://doc.qt.io/qt-6/deployment-android.html

int
main(int argc, char *argv[]) {
    VolleyApplication a(argc, argv);
    QString sVersion = QString("3.00");
    a.setApplicationVersion(sVersion);
    int iResult = a.exec();
    return iResult;
}

