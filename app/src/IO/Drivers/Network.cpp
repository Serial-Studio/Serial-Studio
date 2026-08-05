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

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

static constexpr int kTcpConnectAttempts  = 10;
static constexpr int kTcpConnectTimeoutMs = 15000;
static constexpr int kTcpConnectBackoffMs = 300;
static constexpr int kReopenDebounceMs    = 500;

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
IO::Drivers::Network::Network()
  : m_hostExists(false)
  , m_dialInProgress(false)
  , m_udpMulticast(false)
  , m_lookupActive(false)
  , m_dialAttempts(0)
  , m_dialMode(QIODevice::ReadWrite)
{
  m_reopenTimer.setSingleShot(true);
  m_reopenTimer.setInterval(kReopenDebounceMs);
  m_dialRetryTimer.setSingleShot(true);
  m_dialRetryTimer.setInterval(kTcpConnectBackoffMs);
  m_dialTimeoutTimer.setSingleShot(true);
  m_dialTimeoutTimer.setInterval(kTcpConnectTimeoutMs);
  connect(&m_reopenTimer, &QTimer::timeout, this, &IO::Drivers::Network::reopenAfterConfigChange);
  connect(&m_dialRetryTimer, &QTimer::timeout, this, &IO::Drivers::Network::startTcpDialAttempt);
  connect(&m_dialTimeoutTimer, &QTimer::timeout, this, &IO::Drivers::Network::onDialTimeout);

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

  connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);
  connect(&m_udpSocket, &QUdpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);

  connect(this,
          &IO::Drivers::Network::addressChanged,
          this,
          &IO::Drivers::Network::scheduleReopenIfActive);
  connect(
    this, &IO::Drivers::Network::portChanged, this, &IO::Drivers::Network::scheduleReopenIfActive);
  connect(this,
          &IO::Drivers::Network::socketTypeChanged,
          this,
          &IO::Drivers::Network::scheduleReopenIfActive);
  connect(this,
          &IO::Drivers::Network::udpMulticastChanged,
          this,
          &IO::Drivers::Network::scheduleReopenIfActive);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current network connection and cancels an in-flight TCP dial. Stopping the
 *        dial state and its timers first is what makes a user's disconnect final: nothing may
 *        redial after close() returns.
 */
void IO::Drivers::Network::close()
{
  m_dialInProgress = false;
  m_reopenTimer.stop();
  m_dialRetryTimer.stop();
  m_dialTimeoutTimer.stop();

  disconnect(&m_tcpSocket, &QTcpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);
  disconnect(&m_udpSocket, &QUdpSocket::readyRead, this, &IO::Drivers::Network::onReadyRead);

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
 * @brief Returns true while a TCP dial started by open() has not yet connected or failed.
 */
bool IO::Drivers::Network::isConnecting() const noexcept
{
  return m_dialInProgress;
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
 * @brief Opens a network connection with the specified mode. TCP dials asynchronously: the true
 *        verdict means the attempt started, the socket's own state change reports the outcome
 *        (io.md "connectDevice(int) reports the outcome itself"). UDP binds synchronously.
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  close();
  m_dialMode = mode;

  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  if (socketType() == QAbstractSocket::TcpSocket) {
    m_dialHost       = hostAddr;
    m_dialAttempts   = 0;
    m_dialInProgress = true;
    connect(&m_tcpSocket,
            &QTcpSocket::readyRead,
            this,
            &IO::Drivers::Network::onReadyRead,
            Qt::UniqueConnection);
    startTcpDialAttempt();
    return true;
  }

  QIODevice* socket = nullptr;

  if (socketType() == QAbstractSocket::UdpSocket) {
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
 * @brief Starts (or retries) one TCP connect attempt without blocking the GUI thread. The retry
 *        backoff exists so a server a control-script onConnect() just launched has time to start
 *        listening; onErrorOccurred() decides between another attempt and reporting the failure.
 */
void IO::Drivers::Network::startTcpDialAttempt()
{
  if (!m_dialInProgress)
    return;

  ++m_dialAttempts;
  m_tcpSocket.abort();
  m_tcpSocket.connectToHost(m_dialHost, tcpPort(), m_dialMode);
  m_dialTimeoutTimer.start();
}

/**
 * @brief Fails a dial that neither connected nor errored within the timeout window: aborts the
 *        socket, reports once, and tears the device down so the connect button stays truthful.
 */
void IO::Drivers::Network::onDialTimeout()
{
  if (!m_dialInProgress)
    return;

  m_dialInProgress = false;
  m_dialRetryTimer.stop();
  m_tcpSocket.abort();

  static auto& connectionManager = ConnectionManager::instance();
  queueErrorBox(&connectionManager,
                tr("Network socket error"),
                tr("Connection to %1:%2 timed out.").arg(m_dialHost, QString::number(tcpPort())));
  connectionManager.disconnectDevice(this);
}

/**
 * @brief Debounces a reopen while the link is live so an endpoint edit (address, port, socket
 *        type, multicast) takes effect without a manual disconnect/reconnect cycle. A closed
 *        driver stays closed: configuration edits never dial on their own.
 */
void IO::Drivers::Network::scheduleReopenIfActive()
{
  if (!isOpen() && !isConnecting())
    return;

  m_reopenTimer.start();
}

/**
 * @brief Applies a debounced endpoint change by redialing with the mode the link was opened in.
 */
void IO::Drivers::Network::reopenAfterConfigChange()
{
  if (!isOpen() && !isConnecting())
    return;

  const auto mode = m_dialMode;
  close();
  (void)open(mode);
}

/**
 * @brief Tracks the TCP socket's state: a successful connect ends the dial and stops its timers,
 *        and every transition is forwarded so ConnectionManager can refresh the connected state.
 */
void IO::Drivers::Network::onTcpStateChanged()
{
  if (m_tcpSocket.state() == QAbstractSocket::ConnectedState) {
    m_dialInProgress = false;
    m_dialRetryTimer.stop();
    m_dialTimeoutTimer.stop();
  }

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
 * @brief Sets the IPv4/IPv6 literal or host name, resolving names such as "localhost" through DNS.
 */
void IO::Drivers::Network::setRemoteAddress(const QString& address)
{
  if (!address.isEmpty() && QHostAddress(address).isNull()) {
    m_hostExists = false;
    m_resolvedAddress.clear();
    lookup(address);
  }

  else {
    m_pendingLookup.clear();
    m_hostExists      = true;
    m_resolvedAddress = QHostAddress(address);
  }

  m_address = address;
  m_settings.setValue("NetworkDriver/address", address);
  Q_EMIT addressChanged();
}

/**
 * @brief Starts a DNS lookup, recording the host it belongs to so an out-of-order result for a
 *        host the user has already replaced cannot validate the current one.
 */
void IO::Drivers::Network::lookup(const QString& host)
{
  m_pendingLookup = host.simplified();
  m_lookupActive  = true;
  Q_EMIT lookupActiveChanged();
  QHostInfo::lookupHost(m_pendingLookup, this, &IO::Drivers::Network::lookupFinished);
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
 * @brief Stores the resolved address when the lookup finishes. UDP sends to the resolved literal,
 *        so a name left unresolved here makes every datagram fail; a failed or superseded lookup
 *        clears it and still reports, otherwise the UI keeps offering a host that cannot be dialed.
 */
void IO::Drivers::Network::lookupFinished(const QHostInfo& info)
{
  if (!m_pendingLookup.isEmpty() && info.hostName() != m_pendingLookup)
    return;

  m_pendingLookup.clear();
  m_lookupActive = false;
  Q_EMIT lookupActiveChanged();

  const auto resolved = preferredAddress(info.addresses());
  if (info.error() != QHostInfo::NoError || resolved.isNull()) {
    m_hostExists = false;
    m_resolvedAddress.clear();
    Q_EMIT addressChanged();
    return;
  }

  m_hostExists      = true;
  m_resolvedAddress = resolved;
  Q_EMIT addressChanged();
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
 * @brief Handles socket errors by disconnecting and reporting. A refused dial retries with a
 *        short backoff before failing for real; the teardown destroys this driver, so the box is
 *        queued on the connection manager: raising it here would outlive the object it was raised
 *        from and block the very disconnect it is reporting.
 */
void IO::Drivers::Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  if (m_dialInProgress && socketType() == QAbstractSocket::TcpSocket) {
    m_dialTimeoutTimer.stop();
    if (socketError == QAbstractSocket::ConnectionRefusedError
        && m_dialAttempts < kTcpConnectAttempts) {
      m_dialRetryTimer.start();
      return;
    }

    m_dialInProgress = false;
  }

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
