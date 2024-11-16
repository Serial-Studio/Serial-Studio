/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#include "IO/Manager.h"
#include "IO/Drivers/Network.h"

#include "Misc/Utilities.h"

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * Constructor function
 */
IO::Drivers::Network::Network()
  : m_hostExists(false)
  , m_udpMulticast(false)
  , m_lookupActive(false)
{
  // Set initial configuration
  setRemoteAddress("");
  setTcpPort(defaultTcpPort());
  setUdpLocalPort(defaultUdpLocalPort());
  setUdpRemotePort(defaultUdpRemotePort());
  setSocketType(QAbstractSocket::TcpSocket);

  // Update connect button status when the configuration is changed
  connect(this, &IO::Drivers::Network::addressChanged, this,
          &IO::Drivers::Network::configurationChanged);
  connect(this, &IO::Drivers::Network::socketTypeChanged, this,
          &IO::Drivers::Network::configurationChanged);
  connect(this, &IO::Drivers::Network::portChanged, this,
          &IO::Drivers::Network::configurationChanged);

  // Report socket errors
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
  connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(onErrorOccurred(QAbstractSocket::SocketError)));
  connect(&m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(onErrorOccurred(QAbstractSocket::SocketError)));
#else
  connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this,
          &IO::Drivers::Network::onErrorOccurred);
  connect(&m_udpSocket, &QUdpSocket::errorOccurred, this,
          &IO::Drivers::Network::onErrorOccurred);
#endif
}

/**
 * Returns the only instance of this class
 */
IO::Drivers::Network &IO::Drivers::Network::instance()
{
  static Network singleton;
  return singleton;
}

//------------------------------------------------------------------------------
// HAL driver implementation
//------------------------------------------------------------------------------

/**
 * Closes the current network connection and discards signals/slots with the
 * TCP/UDP socket in use by the network module.
 */
void IO::Drivers::Network::close()
{
  // Abort network connections
  m_tcpSocket.abort();
  m_udpSocket.abort();
  m_tcpSocket.disconnectFromHost();
  m_udpSocket.disconnectFromHost();

  // Disconnect signals/slots
  if (socketType() == QAbstractSocket::TcpSocket)
    disconnect(&m_tcpSocket, &QTcpSocket::readyRead, this,
               &IO::Drivers::Network::onReadyRead);
  else if (socketType() == QAbstractSocket::UdpSocket)
    disconnect(&m_udpSocket, &QUdpSocket::readyRead, this,
               &IO::Drivers::Network::onReadyRead);
}

/**
 * Returns @c true if a network connection is currently open
 */
bool IO::Drivers::Network::isOpen() const
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return m_udpSocket.isOpen();
  else if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isOpen();

  return false;
}

/**
 * Returns @c true if the current network device is readable
 */
bool IO::Drivers::Network::isReadable() const
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return m_udpSocket.isReadable();
  else if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isReadable();

  return false;
}

/**
 * Returns @c true if the current network device is writable
 */
bool IO::Drivers::Network::isWritable() const
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return m_udpSocket.isWritable();
  else if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isWritable();

  return false;
}

/**
 * Returns @c true if the port is greater than 0 and the host address is valid.
 */
bool IO::Drivers::Network::configurationOk() const
{
  return tcpPort() > 0 && m_hostExists;
}

/**
 * @brief Writes data to the network socket.
 *
 * Sends the provided data to the network socket if it is writable.
 *
 * @param data The data to be written to the port.
 * @return The number of bytes written on success, or `-1` if the socket is not
 *         writable.
 */
quint64 IO::Drivers::Network::write(const QByteArray &data)
{
  if (isWritable())
  {
    if (socketType() == QAbstractSocket::UdpSocket)
      return m_udpSocket.write(data);
    else if (socketType() == QAbstractSocket::TcpSocket)
      return m_tcpSocket.write(data);
  }

  return -1;
}

/**
 * @brief Opens a network connection with the specified mode.
 *
 * Initializes and configures the network socket based on the selected socket
 * type (TCP or UDP). For TCP, it connects to the remote host, while for UDP,
 * it binds to the specified local port and joins a multicast group if required.
 *
 * @param mode The mode in which to open the network connection.
 * @return `true` if the connection is successfully opened, `false` otherwise.
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  // Disconnect all sockets
  close();

  // Get host & port
  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  // Init socket pointer
  QIODevice *socket = nullptr;

  // TCP connection, assign socket pointer & connect to host
  if (socketType() == QAbstractSocket::TcpSocket)
  {
    socket = static_cast<QIODevice *>(&m_tcpSocket);
    m_tcpSocket.connectToHost(hostAddr, tcpPort());
  }

  // UDP connection, assign socket pointer & bind to host
  else if (socketType() == QAbstractSocket::UdpSocket)
  {
    // Bind the UDP socket
    m_udpSocket.bind(udpLocalPort(), QAbstractSocket::ShareAddress
                                         | QAbstractSocket::ReuseAddressHint);

    // Join the multicast group (if required)
    if (udpMulticast())
      m_udpSocket.joinMulticastGroup(
          QHostAddress(QHostAddress(m_address).toIPv6Address()));

    // Set socket pointer
    socket = static_cast<QIODevice *>(&m_udpSocket);
  }

  // Open network socket
  if (socket)
  {
    if (socket->open(mode))
    {
      connect(socket, &QIODevice::readyRead, this,
              &IO::Drivers::Network::onReadyRead);
      return true;
    }
  }

  // Open failure, abort connection
  close();
  return false;
}

//------------------------------------------------------------------------------
// Driver specifics
//------------------------------------------------------------------------------

/**
 * Returns the TCP port number
 */
quint16 IO::Drivers::Network::tcpPort() const
{
  return m_tcpPort;
}

/**
 * Returns the UDP local port number
 */
quint16 IO::Drivers::Network::udpLocalPort() const
{
  return m_udpLocalPort;
}

/**
 * Returns the UDP remote port number
 */
quint16 IO::Drivers::Network::udpRemotePort() const
{
  return m_udpRemotePort;
}

/**
 * Returns @c true if the UDP socket is managing a multicasted
 * connection.
 */
bool IO::Drivers::Network::udpMulticast() const
{
  return m_udpMulticast;
}

/**
 * Returns @c true if we are currently performing a DNS lookup
 */
bool IO::Drivers::Network::lookupActive() const
{
  return m_lookupActive;
}

/**
 * Returns the current socket type as an index of the list returned by the @c
 * socketType function.
 */
int IO::Drivers::Network::socketTypeIndex() const
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
 * Returns the host address
 */
const QString &IO::Drivers::Network::remoteAddress() const
{
  return m_address;
}

/**
 * Returns a list with the available socket types
 */
QStringList IO::Drivers::Network::socketTypes() const
{
  QStringList list;
  list.append(QStringLiteral("TCP"));
  list.append(QStringLiteral("UDP"));
  return list;
}

/**
 * Returns the socket type. Valid return values are:
 *
 * @c QAbstractSocket::TcpSocket
 * @c QAbstractSocket::UdpSocket
 * @c QAbstractSocket::SctpSocket
 * @c QAbstractSocket::UnknownSocketType
 */
QAbstractSocket::SocketType IO::Drivers::Network::socketType() const
{
  return m_socketType;
}

/**
 * Instructs the module to communicate via a TCP socket.
 */
void IO::Drivers::Network::setTcpSocket()
{
  setSocketType(QAbstractSocket::TcpSocket);
}

/**
 * Instructs the module to communicate via an UDP socket.
 */
void IO::Drivers::Network::setUdpSocket()
{
  setSocketType(QAbstractSocket::UdpSocket);
}

/**
 * Changes the TCP socket's @c port number
 */
void IO::Drivers::Network::setTcpPort(const quint16 port)
{
  m_tcpPort = port;
  Q_EMIT portChanged();
}

/**
 * Changes the UDP socket's local @c port number
 */
void IO::Drivers::Network::setUdpLocalPort(const quint16 port)
{
  m_udpLocalPort = port;
  Q_EMIT portChanged();
}

/**
 * Changes the UDP socket's remote @c port number
 */
void IO::Drivers::Network::setUdpRemotePort(const quint16 port)
{
  m_udpRemotePort = port;
  Q_EMIT portChanged();
}

/**
 * Sets the IPv4 or IPv6 address specified by the input string representation
 */
void IO::Drivers::Network::setRemoteAddress(const QString &address)
{
  // Check if host name exists
  if (QHostAddress(address).isNull())
  {
    m_hostExists = false;
    lookup(address);
  }

  // Host is an IP address, host should exist
  else
    m_hostExists = true;

  // Change host
  m_address = address;
  Q_EMIT addressChanged();
}

/**
 * Performs a DNS lookup for the given @a host name
 */
void IO::Drivers::Network::lookup(const QString &host)
{
  m_lookupActive = true;
  Q_EMIT lookupActiveChanged();
  QHostInfo::lookupHost(host.simplified(), this,
                        &IO::Drivers::Network::lookupFinished);
}

/**
 * Enables/Disables multicast connections with the UDP socket.
 */
void IO::Drivers::Network::setUdpMulticast(const bool enabled)
{
  m_udpMulticast = enabled;
  Q_EMIT udpMulticastChanged();
}

/**
 * Changes the current socket type given an index of the list returned by the
 * @c socketType() function.
 */
void IO::Drivers::Network::setSocketTypeIndex(const int index)
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
 * @c QAbstractSocket::UnknownSocketType
 */
void IO::Drivers::Network::setSocketType(const QAbstractSocket::SocketType type)
{
  m_socketType = type;
  Q_EMIT socketTypeChanged();
}

/**
 * Reads incoming data from the UDP/TCP ports
 */
void IO::Drivers::Network::onReadyRead()
{
  // Check if we need to use UDP socket functions
  if (socketType() == QAbstractSocket::UdpSocket)
  {
    while (udpSocket()->hasPendingDatagrams())
    {
      QByteArray datagram;
      datagram.resize(int(udpSocket()->pendingDatagramSize()));
      udpSocket()->readDatagram(datagram.data(), datagram.size());
      processData(datagram);
    }
  }

  // We are using the TCP socket...
  else if (socketType() == QAbstractSocket::TcpSocket)
    processData(tcpSocket()->readAll());
}

/**
 * Sets the host IP address when the lookup finishes.
 * If the lookup fails, the error code/string shall be shown to the user in a
 * messagebox.
 */
void IO::Drivers::Network::lookupFinished(const QHostInfo &info)
{
  m_lookupActive = false;

  if (info.error() == QHostInfo::NoError)
  {
    auto addresses = info.addresses();
    if (addresses.count() >= 1)
    {
      m_hostExists = true;
      Q_EMIT addressChanged();
      return;
    }
  }

  Q_EMIT lookupActiveChanged();
}

/**
 * This function is called whenever a socket error occurs, it disconnects the
 * socket from the host and displays the error in a message box.
 */
void IO::Drivers::Network::onErrorOccurred(
    const QAbstractSocket::SocketError socketError)
{
  QString error;
  if (socketType() == QAbstractSocket::TcpSocket)
    error = m_tcpSocket.errorString();
  else if (socketType() == QAbstractSocket::UdpSocket)
    error = m_udpSocket.errorString();
  else
    error = QString::number(socketError);

  Manager::instance().disconnectDevice();
  Misc::Utilities::showMessageBox(tr("Network socket error"), error);
}
