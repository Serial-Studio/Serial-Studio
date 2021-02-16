/*
 * Copyright (C) MATUSICA S.A. de C.V. - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Written by Alex Spataru <https://alex-spataru.com/>, February 2021
 */

#ifndef IO_DATA_SOURCES_NETWORK_H
#define IO_DATA_SOURCES_NETWORK_H

#include <QHostInfo>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QAbstractSocket>

namespace IO
{
namespace DataSources
{
class Network : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString host
               READ host
               WRITE setHost
               NOTIFY hostChanged)
    Q_PROPERTY(quint16 port
               READ port
               WRITE setPort
               NOTIFY portChanged)
    Q_PROPERTY(QAbstractSocket::SocketType socketType
               READ socketType
               WRITE setSocketType
               NOTIFY socketTypeChanged)
    Q_PROPERTY(int socketTypeIndex
               READ socketTypeIndex
               WRITE setSocketTypeIndex
               NOTIFY socketTypeChanged)
    Q_PROPERTY(QStringList socketTypes
               READ socketTypes
               CONSTANT)
    Q_PROPERTY(QString defaultHost
               READ defaultHost
               CONSTANT)
    Q_PROPERTY(quint16 defaultPort
               READ defaultPort
               CONSTANT)
    // clang-format on

signals:
    void hostChanged();
    void portChanged();
    void socketTypeChanged();

public:
    static Network *getInstance();

    QString host() const;
    quint16 port() const;
    int socketTypeIndex() const;
    bool configurationOk() const;
    QStringList socketTypes() const;
    QAbstractSocket::SocketType socketType() const;

    static QString defaultHost() { return "127.0.0.1"; }

    static quint16 defaultPort() { return 23; }

    QIODevice *openNetworkPort();

public slots:
    void setTcpSocket();
    void setUdpSocket();
    void disconnectDevice();
    void setPort(const quint16 port);
    void setHost(const QString &host);
    void findIp(const QString &host);
    void setSocketTypeIndex(const int index);
    void setSocketType(const QAbstractSocket::SocketType type);

private slots:
    void lookupFinished(const QHostInfo &info);
    void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
    Network();
    ~Network();

private:
    QString m_host;
    quint16 m_port;
    QIODevice *m_device;
    QTcpSocket m_tcpSocket;
    QUdpSocket m_udpSocket;
    QAbstractSocket::SocketType m_socketType;
};
}
}

#endif
