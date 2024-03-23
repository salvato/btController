#pragma once

#include "qbluetoothserviceinfo.h"
#include <QMainWindow>
#include <QFile>
#include <QSettings>
#include <QTimer>

#include "generalsetuparguments.h"


QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(BtClient)
QT_FORWARD_DECLARE_CLASS(QBluetoothHostInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)
QT_FORWARD_DECLARE_CLASS(QBluetoothLocalDevice)


class BtScoreController : public QMainWindow
{
    Q_OBJECT

public:
    BtScoreController(QFile *myLogFile, QWidget *parent = nullptr);
    ~BtScoreController();

protected slots:
    void onButtonSpotLoopClicked();
    void onButtonSlideShowClicked();
    void onButtonSetupClicked();
    void onOffButtonClicked();
    void onTextMessageReceived(QString sSource, QString sMessage);
    void closeEvent(QCloseEvent*) override;

private slots:
    void onPanelClientConnected(QString sName);
    void onPanelClientDisconnected();
    void onPanelClientSocketError(QString sError);
    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();
    void discoveryCancelled();

protected:
    void            initBluetooth();
    void            try2ConnectBt();
    bool            prepareLogFile();
    QHBoxLayout*    CreateSpotButtons();
    void            connectButtonSignals();
    virtual void    SaveStatus();
    virtual void    GeneralSetup();
    int             sendMessage(const QString& sMessage);
    void            startBtDiscovery(const QBluetoothUuid &uuid);
    void            stopBtDiscovery();
    virtual void    processTextMessage(QString sMessage);
    void            processGeneralMessages(QString sMessage);

protected:
    GeneralSetupArguments gsArgs;
    QFile*                pLogFile;
    QSettings*            pSettings;
    QPushButton*          pSpotButton{};
    QPushButton*          pSlideShowButton{};
    QPushButton*          pGeneralSetupButton{};
    QPushButton*          pSwitchOffButton{};
    QHBoxLayout*          pSpotButtonsLayout;
    enum status {
        showPanel,
        showSpots,
        showSlides,
        showCamera
    };
    status             myStatus;
    BtClient*          pTempClient;
    BtClient*          pPanelClient;
    QString            sLocalName;
    QList<QBluetoothHostInfo> localAdapters;
    QBluetoothServiceDiscoveryAgent* pBtDiscoveryAgent;
    QBluetoothLocalDevice* pAdapter;

private:
    QBluetoothServiceInfo service;
};
