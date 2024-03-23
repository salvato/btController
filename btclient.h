#pragma once

#include <QObject>
#include <QtBluetooth/qbluetoothsocket.h>

QT_FORWARD_DECLARE_CLASS(QBluetoothServiceInfo)


class BtClient : public QObject
{
    Q_OBJECT
public:
    explicit BtClient(QObject *parent = nullptr);
    ~BtClient();

    void startClient(const QBluetoothAddress& address, const QBluetoothUuid uuid);
    void startClient(const QBluetoothServiceInfo &remoteService);
    void stopClient();

public slots:
    void sendMessage(const QString &message);

signals:
    void messageReceived(const QString &sender, const QString &message);
    void connected(const QString &name);
    void disconnected();
    void socketErrorOccurred(const QString &errorString);

private slots:
    void readSocket();
    void connected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError);

private:
    QBluetoothSocket* pSocket = nullptr;
};
