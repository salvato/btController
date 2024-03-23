/*
 *
Copyright (C) 2024  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QWebSocket>
#include <QHostInfo>

#include "scorecontroller.h"
#include "utility.h"

#define CONNECTION_TIMEOUT 500

#define SERVER_PORT 45454


ScoreController::ScoreController(QFile *myLogFile, QWidget *parent)
    : QMainWindow(parent)
    , pLogFile(myLogFile)
    , pSettings(new QSettings("Gabriele Salvato", "Volley Controller"))
    , serverUrl(QString("ws://panel.local:45454"))
    , pPanelServerSocket(new QWebSocket())
    , pMySlideWindow(nullptr)
{
    qApp->installEventFilter(this);

    setWindowTitle("Score Controller - ©Gabriele Salvato (2024)");
    setWindowIcon(QIcon(":/Logo.ico"));

    pSpotButtonsLayout = CreateSpotButtons();
    connectButtonSignals();

    // We are Ready to Connect to the Panel Server
    pPanelServerSocket->ignoreSslErrors(); // To silent some warnings

    // Events from the Panel Server WebSocket
    connect(pPanelServerSocket, SIGNAL(connected()),
            this, SLOT(onPanelServerConnected()));
    connect(pPanelServerSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onPanelServerSocketError(QAbstractSocket::SocketError)));

    // Let's Start to try to connect to Panel Server
    connect(&connectionTimer, SIGNAL(timeout()),
            this, SLOT(onConnectionTimeExipred()));
    connectionTimer.start(CONNECTION_TIMEOUT);

    setDisabled(true);
    myStatus = showPanel;
}


ScoreController::~ScoreController() {
    refreshTimer.disconnect();
    refreshTimer.stop();
    if(pPanelServerSocket)
        pPanelServerSocket->disconnect();
    if(pSettings) delete pSettings;
    pSettings = Q_NULLPTR;

    if(pPanelServerSocket)
        delete pPanelServerSocket;
    pPanelServerSocket = nullptr;
}


void
ScoreController::closeEvent(QCloseEvent*) {
    refreshTimer.disconnect();
    refreshTimer.stop();
    if(pPanelServerSocket)
        pPanelServerSocket->disconnect();
    if(pSettings) delete pSettings;
    pSettings = Q_NULLPTR;

    if(pPanelServerSocket)
        delete pPanelServerSocket;
    pPanelServerSocket = nullptr;
}


bool
ScoreController::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(((keyEvent->key() == Qt::Key_Left)   ||
            (keyEvent->key() == Qt::Key_Right)) &&
           (keyEvent->modifiers() == Qt::AltModifier))
        {
//            qSwap(iPrimaryScreen, iSecondaryScreen);
//            changeScreen();
//            qCritical() << "key " << keyEvent->key() << "from" << obj;
        }
    }
    return QObject::eventFilter(obj, event);
}


void
ScoreController::onConnectionTimeExipred() {
    // Try to (Re)Open the Server socket to talk to
    pPanelServerSocket->open(serverUrl);
}


void
ScoreController::onPanelServerConnected() {
    connectionTimer.stop();
    connect(pPanelServerSocket, SIGNAL(disconnected()),
            this, SLOT(onPanelServerDisconnected()));
    setEnabled(true);
    SendStatus();
    bStillConnected = false;
    refreshTimer.start(rand()%2000+3000);
}


void
ScoreController::SendStatus() {

}


void
ScoreController::onTimeToRefreshStatus() {
    if(!bStillConnected) {
#ifdef LOG_VERBOSE
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Panel Server Disconnected"));
#endif
        if(pPanelServerSocket)
            pPanelServerSocket->deleteLater();
        pPanelServerSocket =Q_NULLPTR;
        close();
        return;
    }
    SendStatus();
    bStillConnected = false;
}


void
ScoreController::onPanelServerDisconnected() {
    refreshTimer.stop();
    disconnect(pPanelServerSocket, SIGNAL(disconnected()),
               this, SLOT(onPanelServerDisconnected()));
    connectionTimer.start(CONNECTION_TIMEOUT);
    if(myStatus == showSpots) {
        QPixmap pixmap;
        QIcon ButtonIcon;
        pixmap.load(":/resources/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        pShutdownButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
    else if(myStatus == showSlides) {
        QPixmap pixmap;
        QIcon ButtonIcon;
        pixmap.load(":/resources/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setEnabled(true);
        pShutdownButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
    setDisabled(true);
}


void
ScoreController::onPanelServerSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    refreshTimer.stop();
#ifdef LOG_VERBOSE
    QString sError = QString(Q_FUNC_INFO)+
                     QString(" Error %1").arg(error);
    qDebug() << sError;
#endif
    if(pPanelServerSocket->isValid())
        pPanelServerSocket->close();
    connectionTimer.start(CONNECTION_TIMEOUT);
}


QHBoxLayout*
ScoreController::CreateSpotButtons() {
    auto* spotButtonLayout = new QHBoxLayout();

    QPixmap pixmap(":/resources/buttonIcons/PlaySpots.png");
    QIcon ButtonIcon(pixmap);
    pSpotButton = new QPushButton(ButtonIcon, "");
    pSpotButton->setIconSize(pixmap.rect().size());
    pSpotButton->setFlat(true);
    pSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/resources/buttonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    pSlideShowButton = new QPushButton(ButtonIcon, "");
    pSlideShowButton->setIconSize(pixmap.rect().size());
    pSlideShowButton->setFlat(true);
    pSlideShowButton->setToolTip("Start/Stop Slide Show");

    pixmap.load(":/resources/buttonIcons/PanelSetup.png");
    ButtonIcon.addPixmap(pixmap);
    pGeneralSetupButton = new QPushButton(ButtonIcon, "");
    pGeneralSetupButton->setIconSize(pixmap.rect().size());
    pGeneralSetupButton->setFlat(true);
    pGeneralSetupButton->setToolTip("General Setup");

    pixmap.load(":/resources/buttonIcons/video-display.png");
    ButtonIcon.addPixmap(pixmap);
    pShutdownButton = new QPushButton(ButtonIcon, "");
    pShutdownButton->setIconSize(pixmap.rect().size());
    pShutdownButton->setFlat(true);
    pShutdownButton->setToolTip("Shutdown System");

    spotButtonLayout->addWidget(pSpotButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pSlideShowButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pGeneralSetupButton);
    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(pShutdownButton);

    return spotButtonLayout;
}


void
ScoreController::connectButtonSignals() {
    connect(pSpotButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSpotLoopClicked()));

    connect(pSlideShowButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSlideShowClicked()));
    connect(pGeneralSetupButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonSetupClicked()));

    connect(pShutdownButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonShutdownClicked()));
}


int
ScoreController::sendBinary(const QByteArray& bMessage) {
    qint64 written = pPanelServerSocket->sendBinaryMessage(bMessage);
    if(written != bMessage.length()) {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Error writing %1").arg(bMessage));
    }
#ifdef LOG_VERBOSE
    else {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Sent Logo to: %1")
                       .arg(pClient->peerAddress().toString()));
    }
#endif
    return 0;
}


int
ScoreController::sendMessage(const QString& sMessage) {
    qint64 written = pPanelServerSocket->sendTextMessage(sMessage);
    if(written != sMessage.length()) {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Error writing %1").arg(sMessage));
    }
#ifdef LOG_VERBOSE
    else {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Sent %1 to: %2")
                       .arg(sMessage, pClient->peerAddress().toString()));
    }
#endif
    return 0;
}


void
ScoreController::onButtonSetupClicked() {
    GeneralSetup();
}


void
ScoreController::onBinaryMessageReceived(QByteArray baMessage) {
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Received %1 bytes").arg(baMessage.size()));
}


void
ScoreController::onTextMessageReceived(QString sMessage) {
    (void)sMessage;
}


// Dummy... see Volley Panel
void
ScoreController::GeneralSetup() {
}


// Dummy... see Volley Panel
void
ScoreController::SaveStatus() {
}


void
ScoreController::onButtonSpotLoopClicked() {
    QString sMessage;
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        sMessage = QString("<spotloop>1</spotloop>");
        sendMessage(sMessage);
        pixmap.load(":/resources/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        pShutdownButton->setDisabled(true);
        myStatus = showSpots;
    }
    else {
        sMessage = "<endspotloop>1</endspotloop>";
        sendMessage(sMessage);
        pixmap.load(":/resources/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        pSpotButton->setIcon(ButtonIcon);
        pSpotButton->setIconSize(pixmap.rect().size());
        pSlideShowButton->setEnabled(true);
        pShutdownButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonSlideShowClicked() {
    QString sMessage;
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(myStatus == showPanel) {
        sMessage = "<slideshow>1</slideshow>";
        sendMessage(sMessage);
        pixmap.load(":/resources/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setDisabled(true);
        pGeneralSetupButton->setDisabled(true);
        pShutdownButton->setDisabled(true);
        myStatus = showSlides;
    }
    else {
        sMessage = "<endslideshow>1</endslideshow>";
        sendMessage(sMessage);
        pixmap.load(":/resources/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        pSlideShowButton->setIcon(ButtonIcon);
        pSlideShowButton->setIconSize(pixmap.rect().size());
        pSpotButton->setEnabled(true);
        pShutdownButton->setEnabled(true);
        pGeneralSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonShutdownClicked() {
    QMessageBox msgBox;
    msgBox.setText("Sei Sicuro di Volere Spegnere");
    msgBox.setInformativeText("i Tabelloni ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int answer = msgBox.exec();
    if(answer != QMessageBox::Yes) return;
    QString sMessage = "<kill>1</kill>";
    sendMessage(sMessage);
}

