#include <QDir>
#include <QTime>
#include <QMessageBox>

#include "volleyapplication.h"
#include "volleycontroller.h"
#include "utility.h"


VolleyApplication::VolleyApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , pLogFile(nullptr)
    , pScoreController(nullptr)
{
    (void)START_GRADIENT; // Just to Silent a Warning
    pSettings = new QSettings("Gabriele Salvato", "Volley Score Controller");

#ifndef Q_OS_ANDROID
    QString sBaseDir;
    sBaseDir = QDir::homePath();
    if(!sBaseDir.endsWith(QString("/"))) sBaseDir+= QString("/");
    logFileName = QString("%1volley_controller.txt").arg(sBaseDir);
    PrepareLogFile();
#endif

#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Initial Language: %1").arg(sLanguage));
#endif


    // Initialize the random number generator
    QTime time(QTime::currentTime());
    srand(uint(time.msecsSinceStartOfDay()));

    pScoreController = new VolleyController(pLogFile);
#ifdef Q_OS_ANDROID
    pScoreController->showFullScreen();
#else
    pScoreController->showNormal();
#endif
}


VolleyApplication::~VolleyApplication() {
}


bool
VolleyApplication::PrepareLogFile() {
#ifdef LOG_MESG
    QFileInfo checkFile(logFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(logFileName+QString(".bkp"));
        renamed.rename(logFileName, logFileName+QString(".bkp"));
    }
    pLogFile = new QFile(logFileName);
    if (!pLogFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(Q_NULLPTR, "Segnapunti Volley",
                                 QString("Impossibile aprire il file %1: %2.")
                                 .arg(logFileName, pLogFile->errorString()));
        delete pLogFile;
        pLogFile = nullptr;
    }
#endif
    return true;
}
