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

#pragma once

#include <QMainWindow>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QFileInfoList>
#include <QTimer>

#include "generalsetuparguments.h"


QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(SlideWindow)
QT_FORWARD_DECLARE_CLASS(BtClient)


class ScoreController : public QMainWindow
{
    Q_OBJECT

public:
    ScoreController(QFile *myLogFile, QWidget *parent = nullptr);
    ~ScoreController();

protected slots:
    void onButtonSpotLoopClicked();
    void onButtonSlideShowClicked();
    void onButtonSetupClicked();
    void onButtonShutdownClicked();
    void onTextMessageReceived(QString sMessage);
    void onBinaryMessageReceived(QByteArray baMessage);
    void closeEvent(QCloseEvent*) override;

private slots:
    void onConnectionTimeExipred();
    void onPanelServerConnected();
    void onPanelServerDisconnected();
    void onPanelServerSocketError(QString sError);
    void onTimeToRefreshStatus();

protected:
    bool            eventFilter(QObject *obj, QEvent *event) override;
    bool            prepareLogFile();
    QHBoxLayout*    CreateSpotButtons();
    void            connectButtonSignals();
    virtual void    SendStatus();
    virtual void    SaveStatus();
    virtual void    GeneralSetup();
    int             sendMessage(const QString& sMessage);
    int             sendBinary(const QByteArray& bMessage);

protected:
    GeneralSetupArguments gsArgs;
    QFile*                pLogFile;
    QSettings*            pSettings;
    QPushButton*          pSpotButton{};
    QPushButton*          pSlideShowButton{};
    QPushButton*          pGeneralSetupButton{};
    QPushButton*          pShutdownButton{};
    QHBoxLayout*          pSpotButtonsLayout;
    enum status {
        showPanel,
        showSpots,
        showSlides,
        showCamera
    };
    status             myStatus;
    QString            serverUrl;
    BtClient*          pPanelServerSocket;
    SlideWindow*       pMySlideWindow;


private:
    bool               bStillConnected;
    QTimer             refreshTimer;
    QTimer             connectionTimer;
};
