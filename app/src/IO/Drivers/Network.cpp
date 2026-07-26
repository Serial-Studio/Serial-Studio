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

#include <QDebug>
#include <QMetaMethod>

#include "IO/ConnectionFlows.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants: per-attempt windows only. The attempt count and the backoff between attempts are the
// shared Async::RetryPolicy's, so the whole application retries on one schedule.
//--------------------------------------------------------------------------------------------------

static constexpr int kLookupTimeoutMs     = 5000;
static constexpr int kTcpConnectTimeoutMs = 600;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Network driver and restores persisted socket settings.
 */
IO::Drivers::Network::Network()
  : m_lookupId(-1)
  , m_hostExists(false)
  , m_connecting(false)
  , m_udpMulticast(false)
  , m_lookupActive(false)
  , m_userWantsOpen(false)
  , m_runner(this)
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

  connect(&m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);
  connect(&m_udpSocket, &QUdpSocket::errorOccurred, this, &IO::Drivers::Network::onErrorOccurred);

  connect(
    &m_tcpSocket, &QTcpSocket::disconnected, this, &IO::Drivers::Network::onSocketDisconnected);

  connect(&m_runner, &Async::TaskRunner::finished, this, &IO::Drivers::Network::onOpenFlowFinished);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the current network connection and discards the socket signals/slots. The open
 *        flow is cancelled first so a pending attempt cannot re-arm the sockets behind the close.
 */
void IO::Drivers::Network::close()
{
  m_connecting    = false;
  m_userWantsOpen = false;
  m_runner.cancel();

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
    open  = m_tcpSocket.isOpen() && m_tcpSocket.peerPort() != 0;
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
 * @brief Returns true; the driver opens through an orchestrated, non-blocking flow.
 */
bool IO::Drivers::Network::supportsAsyncOpen() const noexcept
{
  return true;
}

/**
 * @brief Starts an open attempt and reports whether one is now in flight. The outcome itself
 *        arrives through openFinished(), which is what callers of an async driver read.
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  beginOpen(mode);
  return isOpen() || m_runner.isRunning();
}

/**
 * @brief Starts the open sequence and returns immediately. Nothing here waits on the socket, so
 *        an unreachable address costs the interface nothing and a disconnect lands at once.
 */
void IO::Drivers::Network::beginOpen(const QIODevice::OpenMode mode)
{
  SS_ASSERT(mode != QIODevice::NotOpen, {
    Q_EMIT openFinished(false, QStringLiteral("invalid open mode"));
    return;
  });

  close();

  m_connecting    = true;
  m_userWantsOpen = true;
  m_runner.run(buildOpenFlow(mode));
}

/**
 * @brief Abandons an attempt in flight, releasing the sockets it half-opened.
 */
void IO::Drivers::Network::abortOpen()
{
  close();
}

/**
 * @brief Composes the open sequence: resolve the host, reach the endpoint, then activate the
 *        socket. One attempt per run; the attempt count and the wait between attempts belong to
 *        the shared retry policy that wraps this flow.
 */
Async::Task* IO::Drivers::Network::buildOpenFlow(const QIODevice::OpenMode mode)
{
  SS_ASSERT_LOG(mode != QIODevice::NotOpen);

  auto* group = Async::sequential(QStringLiteral("network-open"));
  group->addChild(makeLookupStep());

  if (socketType() == QAbstractSocket::TcpSocket) {
    auto host = remoteAddress();
    if (host.isEmpty())
      host = defaultAddress();

    group->addChild(Flows::makeSocketConnect(
      &m_tcpSocket, host, tcpPort(), kTcpConnectTimeoutMs, m_runner.clock()));
  }

  else
    group->addChild(Async::invoke(QStringLiteral("udp-bind"),
                                  [this](QString& reason) { return bindUdpSocket(reason); }));

  group->addChild(Async::invoke(QStringLiteral("socket-activate"), [this, mode](QString& reason) {
    return activateSocket(mode, reason);
  }));

  return group;
}

/**
 * @brief Builds the address-resolution step: a wait on the lookup already in flight, whose
 *        cancellation aborts the real lookup, or an immediate pass when nothing is pending.
 */
Async::Task* IO::Drivers::Network::makeLookupStep()
{
  if (!m_lookupActive)
    return Async::invoke(QStringLiteral("dns-lookup"), [](QString& reason) {
      Q_UNUSED(reason);
      return true;
    });

  auto* step = Async::awaitSignal(QStringLiteral("dns-lookup"));
  step->onSuccess(this, &IO::Drivers::Network::lookupActiveChanged);
  step->setAbortHandler([this]() { abortLookup(); });

  return Async::timeout(step, kLookupTimeoutMs, m_runner.clock());
}

/**
 * @brief Binds the UDP socket, enlarges its receive buffer and joins the multicast group.
 */
bool IO::Drivers::Network::bindUdpSocket(QString& reason)
{
  if (!m_udpSocket.bind(udpLocalPort(),
                        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
    qWarning() << "UDP bind failed on port" << udpLocalPort() << ":" << m_udpSocket.errorString();
    reason = m_udpSocket.errorString();
    return false;
  }

  enlargeUdpReceiveBuffer();
  if (!udpMulticast())
    return true;

  const QHostAddress group(m_address);
  if (group.isNull() || !m_udpSocket.joinMulticastGroup(group)) {
    qWarning() << "UDP multicast join failed for" << m_address << ":" << m_udpSocket.errorString();
    reason = m_udpSocket.errorString();
    return false;
  }

  return true;
}

/**
 * @brief Opens the connected socket in the requested mode and wires it to the read path.
 */
bool IO::Drivers::Network::activateSocket(const QIODevice::OpenMode mode, QString& reason)
{
  SS_ASSERT(mode != QIODevice::NotOpen, {
    reason = QStringLiteral("invalid open mode");
    return false;
  });

  QIODevice* socket = nullptr;
  if (socketType() == QAbstractSocket::TcpSocket)
    socket = static_cast<QIODevice*>(&m_tcpSocket);

  else if (socketType() == QAbstractSocket::UdpSocket)
    socket = static_cast<QIODevice*>(&m_udpSocket);

  if (!socket || !socket->open(mode)) {
    reason = QStringLiteral("the socket could not be opened");
    return false;
  }

  connect(
    socket, &QIODevice::readyRead, this, &IO::Drivers::Network::onReadyRead, Qt::UniqueConnection);

  return true;
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
 * @brief Sets the IPv4 or IPv6 address, performing a DNS lookup if needed.
 */
void IO::Drivers::Network::setRemoteAddress(const QString& address)
{
  if (!address.isEmpty() && QHostAddress(address).isNull()) {
    m_hostExists = false;
    m_resolvedAddress.clear();
    lookup(address);
  }

  else {
    m_hostExists      = true;
    m_resolvedAddress = QHostAddress(address);
  }

  m_address = address;
  m_settings.setValue("NetworkDriver/address", address);
  Q_EMIT addressChanged();
}

/**
 * @brief Performs a DNS lookup for the given host name, superseding any lookup still in flight so
 *        an older answer cannot land after a newer address was typed.
 */
void IO::Drivers::Network::lookup(const QString& host)
{
  abortLookup();

  m_lookupActive = true;
  Q_EMIT lookupActiveChanged();
  m_lookupId =
    QHostInfo::lookupHost(host.simplified(), this, &IO::Drivers::Network::lookupFinished);
}

/**
 * @brief Aborts the lookup in flight, if any. Stopping the real lookup is what makes a cancelled
 *        resolution step release its resources instead of merely ignoring the answer.
 */
void IO::Drivers::Network::abortLookup()
{
  if (m_lookupId < 0)
    return;

  QHostInfo::abortHostLookup(m_lookupId);
  m_lookupId     = -1;
  m_lookupActive = false;
  Q_EMIT lookupActiveChanged();
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
 * @brief Sets the host IP address when the lookup finishes. An answer that belongs to a
 *        superseded lookup is dropped rather than allowed to stomp the current address state.
 */
void IO::Drivers::Network::lookupFinished(const QHostInfo& info)
{
  if (m_lookupId >= 0 && info.lookupId() != m_lookupId)
    return;

  m_lookupId     = -1;
  m_lookupActive = false;
  Q_EMIT lookupActiveChanged();

  if (info.error() == QHostInfo::NoError) {
    auto addresses = info.addresses();
    if (addresses.count() >= 1) {
      m_hostExists = true;
      Q_EMIT addressChanged();
    }
  }
}

/**
 * @brief Reports a link that went down without the application asking, which is what lets a
 *        supervised flow bring it back. A disconnect during an attempt is the attempt's own
 *        business and is not a drop.
 */
void IO::Drivers::Network::onSocketDisconnected()
{
  if (!m_userWantsOpen || m_connecting)
    return;

  Q_EMIT linkDropped();
}

/**
 * @brief Reports the attempt's outcome. A cancel is a close the owner asked for and says nothing;
 *        a failure closes the sockets first, which is what the blocking path did on its way out.
 */
void IO::Drivers::Network::onOpenFlowFinished(Async::Outcome outcome, const Async::StepError& error)
{
  if (outcome == Async::Outcome::Cancelled)
    return;

  m_connecting = false;
  if (outcome == Async::Outcome::Success) {
    Q_EMIT openFinished(true, QString());
    return;
  }

  const QString reason = error.reason.isEmpty() ? QStringLiteral("open failed") : error.reason;
  close();
  Q_EMIT openFinished(false, reason);
}

/**
 * @brief Handles socket errors. While a supervised flow holds a TCP link that came up, the error
 *        is the drop itself: it feeds linkDropped() so the flow retries on the recovery schedule,
 *        and the teardown below is deferred to the supervisor's final give-up. UDP, and a link no
 *        flow is watching, keep the immediate teardown and its message box.
 */
void IO::Drivers::Network::onErrorOccurred(const QAbstractSocket::SocketError socketError)
{
  if (m_connecting)
    return;

  if (socketType() == QAbstractSocket::UdpSocket
      && socketError == QAbstractSocket::ConnectionRefusedError) [[unlikely]]
    return;

  const auto dropped = QMetaMethod::fromSignal(&IO::HAL_Driver::linkDropped);
  if (socketType() == QAbstractSocket::TcpSocket && m_userWantsOpen && isSignalConnected(dropped)) {
    Q_EMIT linkDropped();
    return;
  }

  QString error;
  if (socketType() == QAbstractSocket::TcpSocket)
    error = m_tcpSocket.errorString();
  else if (socketType() == QAbstractSocket::UdpSocket)
    error = m_udpSocket.errorString();
  else
    error = QString::number(socketError);

  static auto& connectionManager = ConnectionManager::instance();
  connectionManager.disconnectDevice(this);
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
