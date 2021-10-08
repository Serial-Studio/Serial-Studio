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
/**
 * @brief The Network class
 *
 * Serial Studio "driver" class to interact with UDP/TCP network ports.
 */
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
    Q_PROPERTY(QVector<QString> socketTypes
               READ socketTypes
               CONSTANT)
    Q_PROPERTY(QString defaultHost
               READ defaultHost
               CONSTANT)
    Q_PROPERTY(quint16 defaultPort
               READ defaultPort
               CONSTANT)
    Q_PROPERTY(bool lookupActive
               READ lookupActive
               NOTIFY lookupActiveChanged)
    Q_PROPERTY(bool udpMulticast
               READ udpMulticast
               WRITE setUdpMulticast
               NOTIFY udpMulticastChanged)
    // clang-format on

signals:
    void hostChanged();
    void portChanged();
    void socketTypeChanged();
    void udpMulticastChanged();
    void lookupActiveChanged();

public:
    static Network *getInstance();

    QString host() const;
    quint16 port() const;
    bool udpMulticast() const;
    bool lookupActive() const;
    int socketTypeIndex() const;
    bool configurationOk() const;
    QVector<QString> socketTypes() const;
    QAbstractSocket::SocketType socketType() const;

    QTcpSocket *tcpSocket() { return &m_tcpSocket; }
    QUdpSocket *udpSocket() { return &m_udpSocket; }
    static QString defaultHost() { return "127.0.0.1"; }
    static quint16 defaultPort() { return 23; }

    QIODevice *openNetworkPort();

public slots:
    void setTcpSocket();
    void setUdpSocket();
    void disconnectDevice();
    void setPort(const quint16 port);
    void setHost(const QString &host);
    void lookup(const QString &host);
    void setUdpMulticast(const bool enabled);
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
    bool m_hostExists;
    bool m_udpMulticast;
    bool m_lookupActive;
    QIODevice *m_device;
    QTcpSocket m_tcpSocket;
    QUdpSocket m_udpSocket;
    QAbstractSocket::SocketType m_socketType;
};
}
}

#endif
