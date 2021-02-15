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

#include "Network.h"

using namespace IO::DataSources;

static Network *INSTANCE = nullptr;

Network::Network()
{
    setPort(0);
    setHost("");
    setSocketType(QAbstractSocket::TcpSocket);
    connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this, &Network::onErrorOccurred);
    connect(&m_udpSocket, &QTcpSocket::errorOccurred, this, &Network::onErrorOccurred);
}

Network::~Network()
{
    disconnectDevice();
}

Network *Network::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Network;

    return INSTANCE;
}

QString Network::host() const
{
    return m_host;
}

quint16 Network::port() const
{
    return m_port;
}

bool Network::configurationOk() const
{
    return port() >= 0 && !QHostAddress(host()).isNull();
}

QAbstractSocket::SocketType Network::socketType() const
{
    return m_socketType;
}

QIODevice *Network::openNetworkPort()
{
    // Disconnect all sockets
    disconnectDevice();

    // Init socket pointer
    QAbstractSocket *socket = nullptr;

    // TCP connection, assign socket pointer & connect to host
    if (socketType() == QAbstractSocket::TcpSocket)
    {
        socket = &m_tcpSocket;
        m_tcpSocket.connectToHost(host(), port());
    }

    // UDP connection, assign socket pointer & connect to host
    else if (socketType() == QAbstractSocket::UdpSocket)
    {
        socket = &m_udpSocket;
        m_udpSocket.connectToHost(host(), port());
    }

    // Convert socket to IO device pointer
    return static_cast<QIODevice *>(socket);
}

void Network::setTcpSocket()
{
    setSocketType(QAbstractSocket::TcpSocket);
}

void Network::setUdpSocket()
{
    setSocketType(QAbstractSocket::UdpSocket);
}

void Network::disconnectDevice()
{
    m_tcpSocket.disconnectFromHost();
    m_udpSocket.disconnectFromHost();
}

void Network::setPort(const quint16 port)
{
    m_port = port;
    emit portChanged();
}

void Network::setHost(const QString &host)
{
    m_host = host;
    emit hostChanged();
}

void Network::setSocketType(const QAbstractSocket::SocketType type)
{
    m_socketType = type;
    emit socketTypeChanged();
}

void Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
}
