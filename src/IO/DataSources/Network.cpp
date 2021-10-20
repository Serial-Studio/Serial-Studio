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

#include <IO/Manager.h>
#include <Misc/Utilities.h>

using namespace IO::DataSources;
static Network *INSTANCE = nullptr;

/**
 * Constructor function
 */
Network::Network()
    : m_hostExists(false)
    , m_udpMulticast(false)
    , m_lookupActive(false)
{
    setHost("");
    setPort(defaultPort());
    setSocketType(QAbstractSocket::TcpSocket);
    connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this, &Network::onErrorOccurred);
    connect(&m_udpSocket, &QUdpSocket::errorOccurred, this, &Network::onErrorOccurred);
}

/**
 * Destructor function
 */
Network::~Network()
{
    disconnectDevice();
}

/**
 * Returns the only instance of this class
 */
Network *Network::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Network;

    return INSTANCE;
}

/**
 * Returns the host address
 */
QString Network::host() const
{
    return m_host;
}

/**
 * Returns the network port number
 */
quint16 Network::port() const
{
    return m_port;
}

/**
 * Returns @c true if the UDP socket is managing a multicasted
 * connection.
 */
bool Network::udpMulticast() const
{
    return m_udpMulticast;
}

/**
 * Returns @c true if we are currently performing a DNS lookup
 */
bool Network::lookupActive() const
{
    return m_lookupActive;
}

/**
 * Returns the current socket type as an index of the list returned by the @c socketType
 * function.
 */
int Network::socketTypeIndex() const
{
    switch (socketType())
    {
        case QAbstractSocket::TcpSocket:
            return 0;
            break;
        case QAbstractSocket::UdpSocket:
            return 1;
            break;
        default:
            return -1;
            break;
    }
}

/**
 * Returns @c true if the port is greater than 0 and the host address is valid.
 */
bool Network::configurationOk() const
{
    return port() > 0 && m_hostExists;
}

/**
 * Returns a list with the available socket types
 */
QVector<QString> Network::socketTypes() const
{
    return QVector<QString> { "TCP", "UDP" };
}

/**
 * Returns the socket type. Valid return values are:
 *
 * @c QAbstractSocket::TcpSocket
 * @c QAbstractSocket::UdpSocket
 * @c QAbstractSocket::SctpSocket
 * @c QAbstractSocket::UnknownSocketType
 */
QAbstractSocket::SocketType Network::socketType() const
{
    return m_socketType;
}

/**
 * Attempts to make a connection to the given host, port and TCP/UDP socket type.
 */
QIODevice *Network::openNetworkPort()
{
    // Disconnect all sockets
    disconnectDevice();

    // Init socket pointer
    QAbstractSocket *socket = nullptr;

    // Get host & port
    auto hostAddr = host();
    auto portAddr = port();
    if (hostAddr.isEmpty())
        hostAddr = defaultHost();
    if (portAddr <= 0)
        portAddr = defaultPort();

    // TCP connection, assign socket pointer & connect to host
    if (socketType() == QAbstractSocket::TcpSocket)
    {
        socket = &m_tcpSocket;
        m_tcpSocket.connectToHost(hostAddr, portAddr);
    }

    // UDP connection, assign socket pointer & bind to host
    else if (socketType() == QAbstractSocket::UdpSocket)
    {
        // Bind the UDP socket to a multicast group
        if (udpMulticast())
        {
            QHostAddress address(hostAddr);
            if (address.protocol() == QAbstractSocket::IPv4Protocol)
                m_udpSocket.bind(QHostAddress::AnyIPv4, portAddr,
                                 QUdpSocket::ShareAddress);
            else if (address.protocol() == QAbstractSocket::IPv6Protocol)
                m_udpSocket.bind(QHostAddress::AnyIPv6, portAddr,
                                 QUdpSocket::ShareAddress);
            else
                m_udpSocket.bind(QHostAddress::Any, portAddr, QUdpSocket::ShareAddress);

            m_udpSocket.joinMulticastGroup(address);
        }

        // Bind the UDP socket to an individual host
        else
            m_udpSocket.connectToHost(hostAddr, portAddr);

        // Update socket pointer
        socket = &m_udpSocket;
    }

    // Convert socket to IO device pointer
    return static_cast<QIODevice *>(socket);
}

/**
 * Instructs the module to communicate via a TCP socket.
 */
void Network::setTcpSocket()
{
    setSocketType(QAbstractSocket::TcpSocket);
}

/**
 * Instructs the module to communicate via an UDP socket.
 */
void Network::setUdpSocket()
{
    setSocketType(QAbstractSocket::UdpSocket);
}

/**
 * Disconnects the TCP/UDP sockets from the host
 */
void Network::disconnectDevice()
{
    m_tcpSocket.disconnectFromHost();
    m_udpSocket.disconnectFromHost();
}

/**
 * Sets the @c port number
 */
void Network::setPort(const quint16 port)
{
    m_port = port;
    emit portChanged();
}

/**
 * Sets the IPv4 or IPv6 address specified by the input string representation
 */
void Network::setHost(const QString &host)
{
    // Check if host name exists
    if (QHostAddress(host).isNull())
    {
        m_hostExists = false;
        lookup(host);
    }

    // Host is an IP address, host should exist
    else
        m_hostExists = true;

    // Change host
    m_host = host;
    emit hostChanged();
}

/**
 * Performs a DNS lookup for the given @a host name
 */
void Network::lookup(const QString &host)
{
    m_lookupActive = true;
    emit lookupActiveChanged();
    QHostInfo::lookupHost(host.simplified(), this, &Network::lookupFinished);
}

/**
 * Enables/Disables multicast connections with the UDP socket.
 */
void Network::setUdpMulticast(const bool enabled)
{
    m_udpMulticast = enabled;
    emit udpMulticastChanged();
}

/**
 * Changes the current socket type given an index of the list returned by the
 * @c socketType() function.
 */
void Network::setSocketTypeIndex(const int index)
{
    switch (index)
    {
        case 0:
            setTcpSocket();
            break;
        case 1:
            setUdpSocket();
            break;
        default:
            break;
    }
}

/**
 * Changes the socket type. Valid input values are:
 *
 * @c QAbstractSocket::TcpSocket
 * @c QAbstractSocket::UdpSocket
 * @c QAbstractSocket::SctpSocket
 * @c QAbstractSocket::UnknownSocketType
 */
void Network::setSocketType(const QAbstractSocket::SocketType type)
{
    m_socketType = type;
    emit socketTypeChanged();
}

/**
 * Sets the host IP address when the lookup finishes.
 * If the lookup fails, the error code/string shall be shown to the user in a messagebox.
 */
void Network::lookupFinished(const QHostInfo &info)
{
    m_lookupActive = false;
    emit lookupActiveChanged();

    if (info.error() == QHostInfo::NoError)
    {
        auto addresses = info.addresses();
        if (addresses.count() >= 1)
        {
            m_hostExists = true;
            emit hostChanged();
            return;
        }
    }
}

/**
 * This function is called whenever a socket error occurs, it disconnects the socket
 * from the host and displays the error in a message box.
 */
void Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
    QString error;
    if (socketType() == QAbstractSocket::TcpSocket)
        error = m_tcpSocket.errorString();
    else if (socketType() == QAbstractSocket::UdpSocket)
        error = m_udpSocket.errorString();
    else
        error = QString::number(socketError);

    Manager::getInstance()->disconnectDevice();
    Misc::Utilities::showMessageBox(tr("Network socket error"), error);
}
