#include "btclient.h"

#include <QtCore/qmetaobject.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>

using namespace  Qt::StringLiterals;


BtClient::BtClient(QObject *parent)
    : QObject{parent}
{}


BtClient::~BtClient() {
    stopClient();
}


void
BtClient::startClient(const QBluetoothAddress& address, const QBluetoothUuid uuid) {
    if (pSocket)
        return;
    // Connect to service
    pSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    // qDebug() << "Create socket";
    pSocket->connectToService(address, uuid, QBluetoothSocket::ReadWrite);
    // qDebug() << "ConnectToService done";

    connect(pSocket, &QBluetoothSocket::readyRead,
            this, &BtClient::readSocket);
    connect(pSocket, &QBluetoothSocket::connected, this,
            QOverload<>::of(&BtClient::connected));
    connect(pSocket, &QBluetoothSocket::disconnected, this,
            &BtClient::disconnected);
    connect(pSocket, &QBluetoothSocket::errorOccurred, this,
            &BtClient::onSocketErrorOccurred);
}


void
BtClient::startClient(const QBluetoothServiceInfo &remoteService) {
    if (pSocket)
        return;

    // Connect to service
    pSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    // qDebug() << "Create socket";
    pSocket->connectToService(remoteService);
    // qDebug() << "ConnectToService done";

    connect(pSocket, &QBluetoothSocket::readyRead,
            this, &BtClient::readSocket);
    connect(pSocket, &QBluetoothSocket::connected, this,
            QOverload<>::of(&BtClient::connected));
    connect(pSocket, &QBluetoothSocket::disconnected, this,
            &BtClient::disconnected);
    connect(pSocket, &QBluetoothSocket::errorOccurred, this,
            &BtClient::onSocketErrorOccurred);
}


void
BtClient::stopClient() {
    delete pSocket;
    pSocket = nullptr;
}


void
BtClient::readSocket() {
    if (!pSocket)
        return;

    while (pSocket->canReadLine()) {
        QByteArray line = pSocket->readLine().trimmed();
        // qDebug() << "Received:" << line;
        emit messageReceived(pSocket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}


void
BtClient::sendMessage(const QString &message) {
    QByteArray text = message.toUtf8() + '\n';
    pSocket->write(text);
}


void
BtClient::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    if (error == QBluetoothSocket::SocketError::NoSocketError)
        return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    QString errorString = pSocket->peerName() + ' '_L1
                          + metaEnum.valueToKey(static_cast<int>(error)) + " occurred"_L1;

    emit socketErrorOccurred(errorString);
}


void
BtClient::connected() {
    emit connected(pSocket->peerName());
}
