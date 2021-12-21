/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <DataTypes.h>

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
/**
 * @brief The Network class
 *
 * Serial Studio "driver" class to interact with UDP/TCP network ports.
 */
class Network : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString remoteAddress
               READ remoteAddress
               WRITE setRemoteAddress
               NOTIFY addressChanged)
    Q_PROPERTY(quint16 tcpPort
               READ tcpPort
               WRITE setTcpPort
               NOTIFY portChanged)
    Q_PROPERTY(quint16 udpLocalPort
               READ udpLocalPort
               WRITE setUdpLocalPort
               NOTIFY portChanged)
    Q_PROPERTY(quint16 udpRemotePort
               READ udpRemotePort
               WRITE setUdpRemotePort
               NOTIFY portChanged)
    Q_PROPERTY(QAbstractSocket::SocketType socketType
               READ socketType
               WRITE setSocketType
               NOTIFY socketTypeChanged)
    Q_PROPERTY(int socketTypeIndex
               READ socketTypeIndex
               WRITE setSocketTypeIndex
               NOTIFY socketTypeChanged)
    Q_PROPERTY(StringList socketTypes
               READ socketTypes
               CONSTANT)
    Q_PROPERTY(QString defaultAddress
               READ defaultAddress
               CONSTANT)
    Q_PROPERTY(quint16 defaultTcpPort
               READ defaultTcpPort
               CONSTANT)
    Q_PROPERTY(quint16 defaultUdpLocalPort
               READ defaultUdpLocalPort
               CONSTANT)
    Q_PROPERTY(quint16 defaultUdpRemotePort
               READ defaultUdpRemotePort
               CONSTANT)
    Q_PROPERTY(bool lookupActive
               READ lookupActive
               NOTIFY lookupActiveChanged)
    Q_PROPERTY(bool udpMulticast
               READ udpMulticast
               WRITE setUdpMulticast
               NOTIFY udpMulticastChanged)
    // clang-format on

Q_SIGNALS:
    void portChanged();
    void addressChanged();
    void socketTypeChanged();
    void udpMulticastChanged();
    void lookupActiveChanged();

private:
    explicit Network();
    Network(Network &&) = delete;
    Network(const Network &) = delete;
    Network &operator=(Network &&) = delete;
    Network &operator=(const Network &) = delete;

    ~Network();

public:
    static Network &instance();

    QString remoteAddress() const;

    quint16 tcpPort() const;
    quint16 udpLocalPort() const;
    quint16 udpRemotePort() const;

    bool udpMulticast() const;
    bool lookupActive() const;
    int socketTypeIndex() const;
    bool configurationOk() const;
    StringList socketTypes() const;
    QAbstractSocket::SocketType socketType() const;

    QTcpSocket *tcpSocket() { return &m_tcpSocket; }
    QUdpSocket *udpSocket() { return &m_udpSocket; }

    static QString defaultAddress() { return "127.0.0.1"; }
    static quint16 defaultTcpPort() { return 23; }
    static quint16 defaultUdpLocalPort() { return 0; }
    static quint16 defaultUdpRemotePort() { return 53; }

    QIODevice *openNetworkPort();

public Q_SLOTS:
    void setTcpSocket();
    void setUdpSocket();
    void disconnectDevice();
    void lookup(const QString &host);
    void setTcpPort(const quint16 port);
    void setUdpLocalPort(const quint16 port);
    void setUdpMulticast(const bool enabled);
    void setSocketTypeIndex(const int index);
    void setUdpRemotePort(const quint16 port);
    void setRemoteAddress(const QString &address);
    void setSocketType(const QAbstractSocket::SocketType type);

private Q_SLOTS:
    void lookupFinished(const QHostInfo &info);
    void onErrorOccurred(const QAbstractSocket::SocketError socketError);

private:
    QString m_address;
    quint16 m_tcpPort;
    bool m_hostExists;
    bool m_udpMulticast;
    bool m_lookupActive;
    quint16 m_udpLocalPort;
    quint16 m_udpRemotePort;
    QAbstractSocket::SocketType m_socketType;

    QTcpSocket m_tcpSocket;
    QUdpSocket m_udpSocket;
};
}
}
