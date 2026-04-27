/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/Network.h"

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Network driver and restores persisted socket settings.
 */
IO::Drivers::Network::Network() : m_hostExists(false), m_udpMulticast(false), m_lookupActive(false)
{
  // Restore persisted settings
  // clang-format off
  auto socketType = m_settings.value("NetworkDriver/socketType", 0).toInt();
  auto remoteAddress = m_settings.value("NetworkDriver/address", "").toString();
  auto tcpPort = m_settings.value("NetworkDriver/tcpPort", defaultTcpPort()).toInt();
  auto udpMulticastEnabled = m_settings.value("NetworkDriver/udpMulticastEnabled", false).toBool();
  auto udpLocalPort = m_settings.value("NetworkDriver/udpLocalPort", defaultUdpLocalPort()).toInt();
  auto udpRemotePort = m_settings.value("NetworkDriver/udpRemotePort", defaultUdpRemotePort()).toInt();
  // clang-format on

  // Apply restored settings
  setTcpPort(tcpPort);
  setUdpLocalPort(udpLocalPort);
  setUdpRemotePort(udpRemotePort);
  setRemoteAddress(remoteAddress);
  setUdpMulticast(udpMulticastEnabled);
  setSocketType(static_cast<QAbstractSocket::SocketType>(socketType));

  // Propagate configuration changes
  connect(
    this, &IO::Drivers::Network::addressChanged, this, &IO::Drivers::Network::configurationChanged);
  connect(this,
          &IO::Drivers::Network::socketTypeChanged,
          this,
          &IO::Drivers::Network::configurationChanged);
  connect(
    this, &IO::Drivers::Network::portChanged, this, &IO::Drivers::Network::configurationChanged);

  // Update state when sockets change
  connect(
    &m_tcpSocket, &QUdpSocket::stateChanged, this, [=, this] { Q_EMIT configurationChanged(); });
  connect(
    &m_udpSocket, &QUdpSocket::stateChanged, this, [=, this] { Q_EMIT configurationChanged(); });

  // Handle socket errors
  connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);
  connect(&m_udpSocket, &QUdpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current network connection and discards the socket signals/slots.
 */
void IO::Drivers::Network::close()
{
  // Disconnect both socket types — avoids duplicate readyRead after a
  // socketType flip between open() and close().
  disconnect(&m_tcpSocket, &QTcpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);
  disconnect(&m_udpSocket, &QUdpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);

  // Abort and close both sockets
  m_tcpSocket.abort();
  m_udpSocket.abort();
  m_tcpSocket.close();
  m_udpSocket.close();
  m_tcpSocket.disconnectFromHost();
  m_udpSocket.disconnectFromHost();
}

/**
 * @brief Returns true when the active socket is connected or bound.
 */
bool IO::Drivers::Network::isOpen() const noexcept
{
  // Query the active socket for its connection state
  bool open  = false;
  auto state = QAbstractSocket::UnconnectedState;

  if (socketType() == QAbstractSocket::UdpSocket) {
    open  = m_udpSocket.isOpen();
    state = m_udpSocket.state();
  }

  else if (socketType() == QAbstractSocket::TcpSocket) {
    open  = m_tcpSocket.isOpen();
    state = m_tcpSocket.state();
  }

  return open && (state == QUdpSocket::ConnectedState || state == QUdpSocket::BoundState);
}

/**
 * @brief Returns true when the active socket can be read.
 */
bool IO::Drivers::Network::isReadable() const noexcept
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return m_udpSocket.isReadable();
  else if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isReadable();

  return false;
}

/**
 * @brief Returns true when the active socket can be written.
 */
bool IO::Drivers::Network::isWritable() const noexcept
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return m_udpSocket.isWritable();
  else if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isWritable();

  return false;
}

/**
 * @brief Returns @c true if the port is greater than 0 and the host address is valid.
 */
bool IO::Drivers::Network::configurationOk() const noexcept
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return udpRemotePort() > 0 && m_hostExists;

  return tcpPort() > 0 && m_hostExists;
}

/**
 * @brief Writes data to the network socket.
 */
qint64 IO::Drivers::Network::write(const QByteArray& data)
{
  if (isWritable()) {
    if (socketType() == QAbstractSocket::UdpSocket)
      return m_udpSocket.write(data);
    else if (socketType() == QAbstractSocket::TcpSocket)
      return m_tcpSocket.write(data);
  }

  return 0;
}

/**
 * @brief Opens a network connection with the specified mode.
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  // Close any existing connection
  close();

  // Resolve host address
  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  QIODevice* socket = nullptr;

  // TCP: connect to remote host
  if (socketType() == QAbstractSocket::TcpSocket) {
    socket = static_cast<QIODevice*>(&m_tcpSocket);
    m_tcpSocket.connectToHost(hostAddr, tcpPort());
  }

  // UDP: bind to local port and optionally join multicast group
  else if (socketType() == QAbstractSocket::UdpSocket) {
    m_udpSocket.bind(udpLocalPort(),
                     QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if (udpMulticast())
      m_udpSocket.joinMulticastGroup(QHostAddress(QHostAddress(m_address).toIPv6Address()));

    socket = static_cast<QIODevice*>(&m_udpSocket);
  }

  // Open the socket and connect readyRead
  if (socket) {
    if (socket->open(mode)) {
      connect(socket, &QIODevice::readyRead, this, &IO::Drivers::Network::onReadyRead);
      return true;
    }
  }

  // Failed to open, clean up
  close();
  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the configured TCP port.
 */
quint16 IO::Drivers::Network::tcpPort() const
{
  return m_tcpPort;
}

/**
 * @brief Returns the UDP local bind port.
 */
quint16 IO::Drivers::Network::udpLocalPort() const
{
  return m_udpLocalPort;
}

/**
 * @brief Returns the UDP remote port.
 */
quint16 IO::Drivers::Network::udpRemotePort() const
{
  return m_udpRemotePort;
}

/**
 * @brief Returns true when UDP multicast is enabled.
 */
bool IO::Drivers::Network::udpMulticast() const
{
  return m_udpMulticast;
}

/**
 * @brief Returns true when a DNS lookup is currently in flight.
 */
bool IO::Drivers::Network::lookupActive() const
{
  return m_lookupActive;
}

/**
 * @brief Returns the current socket type as an index of the list returned by socketType().
 */
int IO::Drivers::Network::socketTypeIndex() const
{
  switch (socketType()) {
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
 * @brief Returns the configured remote host address or hostname.
 */
const QString& IO::Drivers::Network::remoteAddress() const
{
  return m_address;
}

/**
 * @brief Returns a list with the available socket types.
 */
QStringList IO::Drivers::Network::socketTypes() const
{
  QStringList list;
  list.append(QStringLiteral("TCP"));
  list.append(QStringLiteral("UDP"));
  return list;
}

/**
 * @brief Returns the active socket type (TCP or UDP).
 */
QAbstractSocket::SocketType IO::Drivers::Network::socketType() const
{
  return m_socketType;
}

/**
 * @brief Instructs the module to communicate via a TCP socket.
 */
void IO::Drivers::Network::setTcpSocket()
{
  setSocketType(QAbstractSocket::TcpSocket);
}

/**
 * @brief Instructs the module to communicate via a UDP socket.
 */
void IO::Drivers::Network::setUdpSocket()
{
  setSocketType(QAbstractSocket::UdpSocket);
}

/**
 * @brief Changes the TCP socket's port number.
 */
void IO::Drivers::Network::setTcpPort(const quint16 port)
{
  if (m_tcpPort == port)
    return;

  m_tcpPort = port;
  m_settings.setValue("NetworkDriver/tcpPort", port);
  Q_EMIT portChanged();
}

/**
 * @brief Changes the UDP socket's local port number.
 */
void IO::Drivers::Network::setUdpLocalPort(const quint16 port)
{
  if (m_udpLocalPort == port)
    return;

  m_udpLocalPort = port;
  m_settings.setValue("NetworkDriver/udpLocalPort", port);
  Q_EMIT portChanged();
}

/**
 * @brief Changes the UDP socket's remote port number.
 */
void IO::Drivers::Network::setUdpRemotePort(const quint16 port)
{
  if (m_udpRemotePort == port)
    return;

  m_udpRemotePort = port;
  m_settings.setValue("NetworkDriver/udpRemotePort", port);
  Q_EMIT portChanged();
}

/**
 * @brief Sets the IPv4 or IPv6 address, performing a DNS lookup if needed.
 */
void IO::Drivers::Network::setRemoteAddress(const QString& address)
{
  // Perform DNS lookup if address is not a direct IP
  if (!address.isEmpty() && QHostAddress(address).isNull()) {
    m_hostExists = false;
    lookup(address);
  }

  // Direct IP address, mark host as valid
  else
    m_hostExists = true;

  // Store and persist the address
  m_address = address;
  m_settings.setValue("NetworkDriver/address", address);
  Q_EMIT addressChanged();
}

/**
 * @brief Performs a DNS lookup for the given host name.
 */
void IO::Drivers::Network::lookup(const QString& host)
{
  m_lookupActive = true;
  Q_EMIT lookupActiveChanged();
  QHostInfo::lookupHost(host.simplified(), this, &IO::Drivers::Network::lookupFinished);
}

/**
 * @brief Enables/disables multicast connections with the UDP socket.
 */
void IO::Drivers::Network::setUdpMulticast(const bool enabled)
{
  if (m_udpMulticast == enabled)
    return;

  m_udpMulticast = enabled;
  m_settings.setValue("NetworkDriver/udpMulticastEnabled", enabled);
  Q_EMIT udpMulticastChanged();
}

/**
 * @brief Changes the current socket type given an index of the socketTypes() list.
 */
void IO::Drivers::Network::setSocketTypeIndex(const int index)
{
  switch (index) {
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
 * @brief Changes the socket type.
 */
void IO::Drivers::Network::setSocketType(const QAbstractSocket::SocketType type)
{
  if (m_socketType == type)
    return;

  m_socketType = type;
  m_settings.setValue("NetworkDriver/socketType", static_cast<int>(type));
  Q_EMIT socketTypeChanged();
}

/**
 * @brief Reads incoming data from the UDP/TCP ports.
 */
void IO::Drivers::Network::onReadyRead()
{
  // Read from UDP socket (bounded to prevent event loop starvation)
  if (socketType() == QAbstractSocket::UdpSocket) {
    constexpr int kMaxDatagramsPerRead = 256;
    for (int n = 0; n < kMaxDatagramsPerRead && udpSocket()->hasPendingDatagrams(); ++n) {
      const qint64 size = udpSocket()->pendingDatagramSize();
      m_udpBuffer.resize(size);
      udpSocket()->readDatagram(m_udpBuffer.data(), m_udpBuffer.size());
      publishReceivedData(m_udpBuffer);
    }
  }

  // Read from TCP socket
  else if (socketType() == QAbstractSocket::TcpSocket)
    publishReceivedData(tcpSocket()->readAll());
}

/**
 * @brief Sets the host IP address when the lookup finishes.
 */
void IO::Drivers::Network::lookupFinished(const QHostInfo& info)
{
  // Mark lookup as finished and check results
  m_lookupActive = false;

  if (info.error() == QHostInfo::NoError) {
    auto addresses = info.addresses();
    if (addresses.count() >= 1) {
      m_hostExists = true;
      Q_EMIT addressChanged();
      return;
    }
  }

  Q_EMIT lookupActiveChanged();
}

/**
 * @brief Handles socket errors by disconnecting and showing a message box.
 */
void IO::Drivers::Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  // Ignore UDP "port unreachable" — normal for fire-and-forget datagrams.
  if (socketType() == QAbstractSocket::UdpSocket
      && socketError == QAbstractSocket::ConnectionRefusedError) [[unlikely]]
    return;

  // Retrieve the error string from the active socket
  QString error;
  if (socketType() == QAbstractSocket::TcpSocket)
    error = m_tcpSocket.errorString();
  else if (socketType() == QAbstractSocket::UdpSocket)
    error = m_udpSocket.errorString();
  else
    error = QString::number(socketError);

  ConnectionManager::instance().disconnectDevice();
  Misc::Utilities::showMessageBox(tr("Network socket error"), error, QMessageBox::Critical);
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Network configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::Network::driverProperties() const
{
  // Build property list with socket type selector
  QList<IO::DriverProperty> props;

  IO::DriverProperty socketTypeProp;
  socketTypeProp.key     = QStringLiteral("socketTypeIndex");
  socketTypeProp.label   = tr("Socket Type");
  socketTypeProp.type    = IO::DriverProperty::ComboBox;
  socketTypeProp.value   = socketTypeIndex();
  socketTypeProp.options = socketTypes();
  props.append(socketTypeProp);

  IO::DriverProperty addr;
  addr.key   = QStringLiteral("address");
  addr.label = tr("Remote Address");
  addr.type  = IO::DriverProperty::Text;
  addr.value = m_address;
  props.append(addr);

  if (m_socketType == QAbstractSocket::TcpSocket) {
    IO::DriverProperty tcp;
    tcp.key   = QStringLiteral("tcpPort");
    tcp.label = tr("TCP Port");
    tcp.type  = IO::DriverProperty::IntField;
    tcp.value = m_tcpPort;
    tcp.min   = 1;
    tcp.max   = 65535;
    props.append(tcp);
  } else {
    IO::DriverProperty udpLocal;
    udpLocal.key   = QStringLiteral("udpLocalPort");
    udpLocal.label = tr("UDP Local Port");
    udpLocal.type  = IO::DriverProperty::IntField;
    udpLocal.value = m_udpLocalPort;
    udpLocal.min   = 0;
    udpLocal.max   = 65535;
    props.append(udpLocal);

    IO::DriverProperty udpRemote;
    udpRemote.key   = QStringLiteral("udpRemotePort");
    udpRemote.label = tr("UDP Remote Port");
    udpRemote.type  = IO::DriverProperty::IntField;
    udpRemote.value = m_udpRemotePort;
    udpRemote.min   = 1;
    udpRemote.max   = 65535;
    props.append(udpRemote);

    IO::DriverProperty multicast;
    multicast.key   = QStringLiteral("udpMulticast");
    multicast.label = tr("UDP Multicast");
    multicast.type  = IO::DriverProperty::CheckBox;
    multicast.value = m_udpMulticast;
    props.append(multicast);
  }

  return props;
}

/**
 * @brief Applies a single Network configuration change by key.
 */
void IO::Drivers::Network::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("socketTypeIndex"))
    setSocketTypeIndex(value.toInt());

  else if (key == QLatin1String("address"))
    setRemoteAddress(value.toString());

  else if (key == QLatin1String("tcpPort"))
    setTcpPort(static_cast<quint16>(value.toInt()));

  else if (key == QLatin1String("udpLocalPort"))
    setUdpLocalPort(static_cast<quint16>(value.toInt()));

  else if (key == QLatin1String("udpRemotePort"))
    setUdpRemotePort(static_cast<quint16>(value.toInt()));

  else if (key == QLatin1String("udpMulticast"))
    setUdpMulticast(value.toBool());
}
