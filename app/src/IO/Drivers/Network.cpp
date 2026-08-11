/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/Network.h"

#include <QElapsedTimer>
#include <QThread>

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

static constexpr int kDialDeadlineMs = 5000;
static constexpr int kDialPaceMs     = 250;

/**
 * @brief Queues an error box so it opens once the current stack has returned: a modal spins the
 *        event loop, and raising one from a connect or socket-error stack leaves the connection
 *        stuck behind a dialog the user cannot disconnect from.
 */
static void queueErrorBox(QObject* context, const QString& title, const QString& text)
{
  QMetaObject::invokeMethod(
    context,
    [title, text] { Misc::Utilities::showMessageBox(title, text, QMessageBox::Critical); },
    Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Network driver and restores persisted socket settings.
 */
IO::Drivers::Network::Network() : m_udpMulticast(false), m_lookupActive(false), m_lookupId(-1)
{
  // clang-format off
  auto socketType = m_settings.value("NetworkDriver/socketType", 0).toInt();
  auto remoteAddress = m_settings.value("NetworkDriver/address", "").toString();
  auto tcpPort = m_settings.value("NetworkDriver/tcpPort", defaultTcpPort()).toInt();
  auto udpMulticastEnabled = m_settings.value("NetworkDriver/udpMulticastEnabled", false).toBool();
  auto udpLocalPort = m_settings.value("NetworkDriver/udpLocalPort", defaultUdpLocalPort()).toInt();
  auto udpRemotePort = m_settings.value("NetworkDriver/udpRemotePort", defaultUdpRemotePort()).toInt();
  // clang-format on

  setTcpPort(tcpPort);
  setUdpLocalPort(udpLocalPort);
  setUdpRemotePort(udpRemotePort);
  setRemoteAddress(remoteAddress);
  setUdpMulticast(udpMulticastEnabled);
  setSocketType(static_cast<QAbstractSocket::SocketType>(socketType));

  connect(
    this, &IO::Drivers::Network::addressChanged, this, &IO::Drivers::Network::configurationChanged);
  connect(this,
          &IO::Drivers::Network::socketTypeChanged,
          this,
          &IO::Drivers::Network::configurationChanged);
  connect(
    this, &IO::Drivers::Network::portChanged, this, &IO::Drivers::Network::configurationChanged);

  connect(
    &m_tcpSocket, &QAbstractSocket::stateChanged, this, &IO::Drivers::Network::onTcpStateChanged);
  connect(&m_udpSocket, &QAbstractSocket::stateChanged, this, [=, this] {
    Q_EMIT configurationChanged();
  });

  connect(&m_udpSocket, &QUdpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current network connection. Nothing may redial after close() returns: there
 *        are no dial or reopen timers left to cancel.
 */
void IO::Drivers::Network::close()
{
  disconnect(&m_tcpSocket, &QTcpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);
  disconnect(&m_udpSocket, &QUdpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);
  disconnect(
    &m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);

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
  if (socketType() == QAbstractSocket::TcpSocket)
    return m_tcpSocket.isOpen() && tcpLinkUp();

  if (socketType() == QAbstractSocket::UdpSocket) {
    const auto state = m_udpSocket.state();
    return m_udpSocket.isOpen()
        && (state == QUdpSocket::ConnectedState || state == QUdpSocket::BoundState);
  }

  return false;
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
 * @brief Returns @c true if the relevant port is greater than 0. Host validity is not checked
 *        here: open() falls back to the default address when empty, TCP resolves names inside
 *        connectToHost(), and a bad host surfaces as a dial error. Gating on an async DNS
 *        verdict left the connect button and open path disagreeing about stale state.
 */
bool IO::Drivers::Network::configurationOk() const noexcept
{
  if (socketType() == QAbstractSocket::UdpSocket)
    return udpRemotePort() > 0;

  return tcpPort() > 0;
}

/**
 * @brief Writes data to the network socket.
 */
qint64 IO::Drivers::Network::write(const QByteArray& data)
{
  if (isWritable()) {
    if (socketType() == QAbstractSocket::UdpSocket) {
      const QHostAddress dest =
        m_resolvedAddress.isNull() ? QHostAddress(m_address) : m_resolvedAddress;
      if (dest.isNull())
        return -1;

      return m_udpSocket.writeDatagram(data, dest, udpRemotePort());
    }

    else if (socketType() == QAbstractSocket::TcpSocket)
      return m_tcpSocket.write(data);
  }

  return 0;
}

/**
 * @brief Opens a network connection with the specified mode. Both TCP and UDP finish
 *        synchronously: the return value is the final verdict, nothing happens after.
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  close();

  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  if (socketType() == QAbstractSocket::TcpSocket)
    return dialTcpBlocking(hostAddr, mode);

  QIODevice* socket = nullptr;

  if (socketType() == QAbstractSocket::UdpSocket) {
    if (!m_address.isEmpty() && m_resolvedAddress.isNull() && !m_lookupActive)
      lookup(m_address);

    if (!m_udpSocket.bind(udpLocalPort(),
                          QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
      qWarning() << "UDP bind failed on port" << udpLocalPort() << ":" << m_udpSocket.errorString();
      close();
      return false;
    }

    enlargeUdpReceiveBuffer();
    socket = static_cast<QIODevice*>(&m_udpSocket);
  }

  if (socketType() == QAbstractSocket::UdpSocket && udpMulticast()) {
    const QHostAddress literal(m_address);
    const QHostAddress group = literal.isNull() ? m_resolvedAddress : literal;
    if (group.isNull() || !m_udpSocket.joinMulticastGroup(group)) {
      qWarning() << "UDP multicast join failed for" << m_address << ":"
                 << m_udpSocket.errorString();
      close();
      return false;
    }
  }

  if (socket && socket->open(mode)) {
    connect(socket, &QIODevice::readyRead, this, &IO::Drivers::Network::onReadyRead);
    return true;
  }

  close();
  return false;
}

/**
 * @brief Returns true for an established TCP link. The state alone is trustworthy because the
 *        blocking dial's waitForConnected() verdict is authoritative and every retry aborts a
 *        non-idle socket before redialing (the 2026-07 phantom-connect trigger was an abort and
 *        a dial racing in the same event-loop turn with the outcome read from signals).
 */
bool IO::Drivers::Network::tcpLinkUp() const
{
  return m_tcpSocket.state() == QAbstractSocket::ConnectedState;
}

/**
 * @brief Waits inside the deadline for @p host:@p port to accept, re-pacing refused attempts (a
 *        script-launched helper needs a moment to bind). Each attempt uses a throwaway socket
 *        destroyed inside the blocked section: abort-and-redial churn on a run-loop-registered
 *        socket leaves stale CFSocket sources that crashed readFromSocket (2026-08-10, macOS).
 */
static bool waitForTcpEndpoint(const QString& host, quint16 port, QString& reason)
{
  QElapsedTimer deadline;
  deadline.start();

  while (deadline.elapsed() < kDialDeadlineMs) {
    QTcpSocket probe;
    probe.connectToHost(host, port);
    const bool up = probe.waitForConnected(kDialDeadlineMs - int(deadline.elapsed()));
    const QAbstractSocket::SocketError err = probe.error();
    reason                                 = probe.errorString();
    probe.abort();

    if (up)
      return true;

    if (err != QAbstractSocket::ConnectionRefusedError)
      return false;

    QThread::msleep(kDialPaceMs);
  }

  return false;
}

/**
 * @brief Dials TCP synchronously under the connect wait cursor and returns the final verdict.
 *        A throwaway probe absorbs the retry churn; the driver's own socket then connects
 *        exactly once per open(), never seeing an abort-and-redial cycle. The readyRead and
 *        errorOccurred handlers are wired only on success: dial failures are owned here.
 */
bool IO::Drivers::Network::dialTcpBlocking(const QString& host, const QIODevice::OpenMode mode)
{
  static auto& connectionManager = ConnectionManager::instance();

  QString reason;
  if (!waitForTcpEndpoint(host, tcpPort(), reason)) {
    queueErrorBox(&connectionManager,
                  tr("Network socket error"),
                  tr("Cannot connect to %1:%2 (%3)").arg(host, QString::number(tcpPort()), reason));
    return false;
  }

  m_tcpSocket.connectToHost(host, tcpPort(), mode);
  if (!m_tcpSocket.waitForConnected(kDialDeadlineMs)) {
    const QString finalReason = m_tcpSocket.errorString();
    m_tcpSocket.abort();
    queueErrorBox(
      &connectionManager,
      tr("Network socket error"),
      tr("Cannot connect to %1:%2 (%3)").arg(host, QString::number(tcpPort()), finalReason));
    return false;
  }

  connect(&m_tcpSocket,
          &QTcpSocket::readyRead,
          this,
          &IO::Drivers::Network::onReadyRead,
          Qt::UniqueConnection);
  connect(&m_tcpSocket,
          &QTcpSocket::errorOccurred,
          this,
          &IO::Drivers::Network::onErrorOccurred,
          Qt::UniqueConnection);
  return true;
}

/**
 * @brief Forwards every TCP state transition so ConnectionManager can refresh connected state.
 */
void IO::Drivers::Network::onTcpStateChanged()
{
  Q_EMIT configurationChanged();
}

/**
 * @brief Requests a large kernel receive buffer on the bound UDP socket so a bursty high-rate
 *        sender is not dropped before the event loop drains it. Windows' default UDP SO_RCVBUF
 *        is only tens of KB and overflows under loopback floods (Linux/macOS default much
 *        higher); the OS caps the request to its allowed maximum.
 */
void IO::Drivers::Network::enlargeUdpReceiveBuffer()
{
  constexpr int kUdpReceiveBufferBytes = 8 * 1024 * 1024;
  m_udpSocket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                              kUdpReceiveBufferBytes);
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
 * @brief Sets the IPv4/IPv6 literal or host name. TCP resolves names inside connectToHost(); the
 *        async lookup only feeds the resolved literal that UDP datagrams are sent to. Re-applying
 *        the current address is a no-op so the UI/live/project sync layers cannot restart lookups
 *        or bounce a live connection.
 */
void IO::Drivers::Network::setRemoteAddress(const QString& address)
{
  if (m_address == address)
    return;

  m_address         = address;
  m_resolvedAddress = QHostAddress(address);
  m_pendingLookup.clear();
  if (m_lookupActive) {
    m_lookupActive = false;
    Q_EMIT lookupActiveChanged();
  }

  if (!address.isEmpty() && m_resolvedAddress.isNull())
    lookup(address);

  m_settings.setValue("NetworkDriver/address", address);
  Q_EMIT addressChanged();
}

/**
 * @brief Starts a DNS lookup, recording the host it belongs to so an out-of-order result for a
 *        host the user has already replaced cannot validate the current one.
 */
void IO::Drivers::Network::lookup(const QString& host)
{
  if (m_lookupActive)
    QHostInfo::abortHostLookup(m_lookupId);

  m_pendingLookup = host.simplified();
  m_lookupActive  = true;
  Q_EMIT lookupActiveChanged();
  m_lookupId = QHostInfo::lookupHost(m_pendingLookup, this, &IO::Drivers::Network::lookupFinished);
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
  if (socketType() == QAbstractSocket::UdpSocket) {
    constexpr int kMaxDatagramsPerRead = 256;
    for (int n = 0; n < kMaxDatagramsPerRead && udpSocket()->hasPendingDatagrams(); ++n) {
      const qint64 size = udpSocket()->pendingDatagramSize();
      m_udpBuffer.resize(size);
      udpSocket()->readDatagram(m_udpBuffer.data(), m_udpBuffer.size());
      publishReceivedData(m_udpBuffer);
    }
  }

  else if (socketType() == QAbstractSocket::TcpSocket)
    publishReceivedData(tcpSocket()->readAll());
}

/**
 * @brief Stores the resolved address for UDP sends when the lookup finishes; a failed or
 *        superseded lookup leaves it cleared. Resolution never gates or reopens the connection:
 *        TCP resolves inside connectToHost(), so a lookup landing after a dial must not touch a
 *        live link (emitting change signals here used to bounce healthy connections).
 */
void IO::Drivers::Network::lookupFinished(const QHostInfo& info)
{
  if (m_pendingLookup.isEmpty() || info.hostName() != m_pendingLookup)
    return;

  m_pendingLookup.clear();
  m_lookupActive = false;
  Q_EMIT lookupActiveChanged();

  const auto resolved = preferredAddress(info.addresses());
  if (info.error() == QHostInfo::NoError && !resolved.isNull())
    m_resolvedAddress = resolved;
}

/**
 * @brief Picks the address to dial from a DNS result, preferring IPv4 because a dual-stack name
 *        such as "localhost" resolves to ::1 first and most local test servers bind IPv4 only.
 */
QHostAddress IO::Drivers::Network::preferredAddress(const QList<QHostAddress>& addresses)
{
  for (const auto& address : addresses)
    if (address.protocol() == QAbstractSocket::IPv4Protocol)
      return address;

  return addresses.isEmpty() ? QHostAddress() : addresses.first();
}

/**
 * @brief Handles errors on an established link: report once, tear down, stay down. The TCP
 *        handler is wired only after a successful dial (dialTcpBlocking() owns dial failures);
 *        the box is queued on the connection manager because the teardown destroys this driver.
 */
void IO::Drivers::Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  if (socketType() == QAbstractSocket::UdpSocket
      && socketError == QAbstractSocket::ConnectionRefusedError) [[unlikely]]
    return;

  QString error;
  if (socketType() == QAbstractSocket::TcpSocket)
    error = m_tcpSocket.errorString();
  else if (socketType() == QAbstractSocket::UdpSocket)
    error = m_udpSocket.errorString();
  else
    error = QString::number(socketError);

  static auto& connectionManager = ConnectionManager::instance();
  queueErrorBox(&connectionManager, tr("Network socket error"), error);
  connectionManager.disconnectDevice(this);
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Network configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::Network::driverProperties() const
{
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
  if (key == QLatin1String("socketTypeIndex")) {
    setSocketTypeIndex(value.toInt());
    return;
  }

  if (key == QLatin1String("address")) {
    setRemoteAddress(value.toString());
    return;
  }

  if (key == QLatin1String("tcpPort")) {
    setTcpPort(static_cast<quint16>(value.toInt()));
    return;
  }

  if (key == QLatin1String("udpLocalPort")) {
    setUdpLocalPort(static_cast<quint16>(value.toInt()));
    return;
  }

  if (key == QLatin1String("udpRemotePort")) {
    setUdpRemotePort(static_cast<quint16>(value.toInt()));
    return;
  }

  if (key == QLatin1String("udpMulticast"))
    setUdpMulticast(value.toBool());
}
