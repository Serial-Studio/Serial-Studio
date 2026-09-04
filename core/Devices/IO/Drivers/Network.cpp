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

static constexpr int kHttpMinIntervalMs = 10;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Network driver and restores persisted socket settings.
 */
IO::Drivers::Network::Network()
  : m_udpMulticast(false)
  , m_lookupActive(false)
  , m_lookupId(-1)
  , m_socketType(SocketType::Tcp)
  , m_webSocketFormat(0)
  , m_ignoreTlsErrors(false)
  , m_dialPending(false)
  , m_httpMethod(0)
  , m_httpInterval(defaultHttpInterval())
  , m_httpActive(false)
  , m_httpFailureLogged(false)
  , m_httpTruncationLogged(false)
  , m_pollsOk(0)
  , m_pollsFailed(0)
  , m_pollsSkipped(0)
  , m_consecutiveFailures(0)
  , m_tcpSocket(new QTcpSocket)
  , m_udpSocket(new QUdpSocket)
  , m_webSocket(new QWebSocket)
  , m_httpManager(new QNetworkAccessManager)
{
  // clang-format off
  auto socketType = m_settings.value("NetworkDriver/socketType", 0).toInt();
  auto remoteAddress = m_settings.value("NetworkDriver/address", "").toString();
  auto tcpPort = m_settings.value("NetworkDriver/tcpPort", defaultTcpPort()).toInt();
  auto udpMulticastEnabled = m_settings.value("NetworkDriver/udpMulticastEnabled", false).toBool();
  auto udpLocalPort = m_settings.value("NetworkDriver/udpLocalPort", defaultUdpLocalPort()).toInt();
  auto udpRemotePort = m_settings.value("NetworkDriver/udpRemotePort", defaultUdpRemotePort()).toInt();
  auto ignoreTlsErrors = m_settings.value("NetworkDriver/ignoreTlsErrors", false).toBool();
  auto webSocketFormat = m_settings.value("NetworkDriver/webSocketFormat", 0).toInt();
  auto webSocketUrl = m_settings.value("NetworkDriver/webSocketUrl", defaultWebSocketUrl()).toString();
  auto httpBody = m_settings.value("NetworkDriver/httpBody", "").toString();
  auto httpMethod = m_settings.value("NetworkDriver/httpMethod", 0).toInt();
  auto httpHeaders = m_settings.value("NetworkDriver/httpHeaders", "").toString();
  auto httpUrl = m_settings.value("NetworkDriver/httpUrl", defaultHttpUrl()).toString();
  auto httpInterval = m_settings.value("NetworkDriver/httpInterval", defaultHttpInterval()).toInt();
  // clang-format on

  setTcpPort(tcpPort);
  setUdpLocalPort(udpLocalPort);
  setUdpRemotePort(udpRemotePort);
  setRemoteAddress(remoteAddress);
  setWebSocketUrl(webSocketUrl);
  setIgnoreTlsErrors(ignoreTlsErrors);
  setUdpMulticast(udpMulticastEnabled);
  setWebSocketFormatIndex(webSocketFormat);
  setHttpUrl(httpUrl);
  setHttpBody(httpBody);
  setHttpHeaders(httpHeaders);
  setHttpInterval(httpInterval);
  setHttpMethodIndex(httpMethod);
  setSocketTypeIndex(socketType);

  connect(
    this, &IO::Drivers::Network::addressChanged, this, &IO::Drivers::Network::configurationChanged);
  connect(this,
          &IO::Drivers::Network::socketTypeChanged,
          this,
          &IO::Drivers::Network::configurationChanged);
  connect(
    this, &IO::Drivers::Network::portChanged, this, &IO::Drivers::Network::configurationChanged);
  connect(this,
          &IO::Drivers::Network::webSocketChanged,
          this,
          &IO::Drivers::Network::configurationChanged);
  connect(
    this, &IO::Drivers::Network::httpChanged, this, &IO::Drivers::Network::configurationChanged);

  m_pollTimer.setSingleShot(false);
  connect(&m_pollTimer, &QTimer::timeout, this, &IO::Drivers::Network::onPollTimeout);

  connect(&m_tcpDial, &IO::AsyncTcpDial::finished, this, &IO::Drivers::Network::onTcpDialFinished);

  connect(
    m_tcpSocket, &QAbstractSocket::stateChanged, this, &IO::Drivers::Network::onTcpStateChanged);
  connect(m_udpSocket, &QAbstractSocket::stateChanged, this, [=, this] {
    Q_EMIT configurationChanged();
  });

  connect(m_udpSocket, &QUdpSocket::errorOccurred, this, &IO::Drivers::Network::onUdpError);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the link and retires the sockets. They are deleteLater()'d rather than destroyed
 *        with the driver: a run-loop source scheduled for a socket still fires after close(), and
 *        it must find a live, aborted socket instead of a freed one (macOS, 2026-08-16).
 */
IO::Drivers::Network::~Network()
{
  close();

  m_tcpSocket->deleteLater();
  m_udpSocket->deleteLater();
  m_webSocket->deleteLater();
  m_httpManager->deleteLater();
}

/**
 * @brief Closes every transport rather than the selected one: a socket type changed while a link
 *        was up would otherwise strand the socket that is actually open. Nothing may redial after
 *        close() returns: there are no dial or reopen timers left to cancel.
 */
void IO::Drivers::Network::close()
{
  closeTcp();
  closeUdp();
  closeHttp();
  closeWebSocket();

  m_dialPending = false;
}

/**
 * @brief Returns true when the active socket is connected or bound.
 */
bool IO::Drivers::Network::isOpen() const noexcept
{
  if (socketType() == SocketType::Tcp)
    return tcpOpen();

  if (socketType() == SocketType::Udp)
    return udpOpen();

  if (socketType() == SocketType::WebSocket)
    return webSocketOpen();

  if (socketType() == SocketType::Http)
    return httpOpen();

  return false;
}

/**
 * @brief Returns true while an asynchronous dial is in flight. TCP, WebSocket and HTTP all dial
 *        asynchronously and report through openFinished(); UDP alone settles inside open(), where
 *        the return value is its whole verdict.
 */
bool IO::Drivers::Network::isConnecting() const noexcept
{
  return m_dialPending;
}

/**
 * @brief Returns true when the active socket can be read.
 */
bool IO::Drivers::Network::isReadable() const noexcept
{
  if (socketType() == SocketType::Tcp)
    return tcpReadable();

  if (socketType() == SocketType::Udp)
    return udpReadable();

  if (socketType() == SocketType::WebSocket)
    return webSocketOpen();

  if (socketType() == SocketType::Http)
    return httpOpen();

  return false;
}

/**
 * @brief Returns true when the active socket can be written.
 */
bool IO::Drivers::Network::isWritable() const noexcept
{
  if (socketType() == SocketType::Tcp)
    return tcpWritable();

  if (socketType() == SocketType::Udp)
    return udpWritable();

  if (socketType() == SocketType::WebSocket)
    return webSocketWritable();

  if (socketType() == SocketType::Http)
    return httpOpen();

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
  if (socketType() == SocketType::Udp)
    return udpConfigured();

  if (socketType() == SocketType::WebSocket)
    return webSocketConfigured();

  if (socketType() == SocketType::Http)
    return httpConfigured();

  return tcpConfigured();
}

/**
 * @brief Writes data to the network socket. A TCP write issued while the dial is still in flight
 *        is held rather than dropped: a control script's io.connect() + writeData() sequence must
 *        keep working now that the dial no longer blocks inside open() (spec 0050).
 */
qint64 IO::Drivers::Network::write(const QByteArray& data)
{
  if (socketType() == SocketType::Tcp && m_dialPending)
    return queueTcpWrite(data);

  if (!isWritable())
    return 0;

  if (socketType() == SocketType::Tcp)
    return writeTcp(data);

  if (socketType() == SocketType::Udp)
    return writeUdp(data);

  if (socketType() == SocketType::WebSocket)
    return writeWebSocket(data);

  if (socketType() == SocketType::Http)
    return writeHttp(data);

  return 0;
}

/**
 * @brief Opens a network connection with the specified mode. Only UDP finishes synchronously; the
 *        other three return "attempt started" and settle through openFinished().
 */
bool IO::Drivers::Network::open(const QIODevice::OpenMode mode)
{
  close();

  if (socketType() == SocketType::Tcp)
    return openTcp(mode);

  if (socketType() == SocketType::Udp)
    return openUdp(mode);

  if (socketType() == SocketType::WebSocket)
    return openWebSocket(mode);

  if (socketType() == SocketType::Http)
    return openHttp(mode);

  return false;
}

/**
 * @brief Resolves the URL the active transport dials, reporting why an unusable one is unusable.
 *        configurationOk() and open() both come through here: a second validation rule is how the
 *        Connect button and the open path drifted apart for DNS once already.
 */
bool IO::Drivers::Network::urlForCurrentMode(QUrl& url, QString& reason) const
{
  QString raw;
  QStringList schemes;

  if (socketType() == SocketType::WebSocket) {
    raw     = m_webSocketUrl;
    schemes = {QStringLiteral("ws"), QStringLiteral("wss")};
  }

  else if (socketType() == SocketType::Http) {
    raw     = m_httpUrl;
    schemes = {QStringLiteral("http"), QStringLiteral("https")};
  }

  else {
    reason = tr("This socket type does not use a URL");
    return false;
  }

  const QString trimmed = raw.trimmed();
  if (trimmed.isEmpty()) {
    reason = tr("Enter a URL first");
    return false;
  }

  const QUrl parsed(trimmed, QUrl::StrictMode);
  if (!parsed.isValid() || parsed.host().isEmpty()) {
    reason = tr("\"%1\" is not a valid URL").arg(trimmed);
    return false;
  }

  if (!schemes.contains(parsed.scheme(), Qt::CaseInsensitive)) {
    reason =
      tr("\"%1\" must start with %2://").arg(trimmed, schemes.join(QStringLiteral(":// or ")));
    return false;
  }

  url = parsed;
  return true;
}

/**
 * @brief Settles an asynchronous dial as successful. The base-class latch makes a second report
 *        for the same attempt a no-op, so an established-link event cannot masquerade as one.
 */
void IO::Drivers::Network::succeedDial()
{
  m_dialPending = false;
  reportOpenFinished(true);
  Q_EMIT configurationChanged();
}

/**
 * @brief Settles an asynchronous dial as failed. It only reports: ConnectionManager closes the
 *        device itself on a failed verdict, so tearing down here as well would double-close.
 */
void IO::Drivers::Network::failDial(const QString& reason)
{
  m_dialPending = false;
  logDriverError(tr("Network socket error"), reason);
  reportOpenFinished(false, reason);
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
 * @brief Returns the current socket type as an index of the socketTypes() list. The enumerator
 *        numbering is that index, so this is a cast rather than a mapping.
 */
int IO::Drivers::Network::socketTypeIndex() const
{
  return static_cast<int>(socketType());
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
  list.append(QStringLiteral("WebSocket"));
  list.append(QStringLiteral("HTTP"));
  return list;
}

/**
 * @brief Returns the HTTP methods, indexed as the property stores them.
 */
QStringList IO::Drivers::Network::httpMethods() const
{
  QStringList list;
  list.append(QStringLiteral("GET"));
  list.append(QStringLiteral("POST"));
  list.append(QStringLiteral("PUT"));
  list.append(QStringLiteral("PATCH"));
  list.append(QStringLiteral("DELETE"));
  return list;
}

/**
 * @brief Returns the labels of the WebSocket send formats, indexed as the property stores them.
 */
QStringList IO::Drivers::Network::webSocketFormats() const
{
  QStringList list;
  list.append(tr("Automatic"));
  list.append(tr("Text"));
  list.append(tr("Binary"));
  return list;
}

/**
 * @brief Returns the configured WebSocket endpoint.
 */
const QString& IO::Drivers::Network::webSocketUrl() const
{
  return m_webSocketUrl;
}

/**
 * @brief Returns the WebSocket send format as an index of webSocketFormats().
 */
int IO::Drivers::Network::webSocketFormatIndex() const
{
  return m_webSocketFormat;
}

/**
 * @brief Returns true when certificate verification is bypassed for the URL transports.
 */
bool IO::Drivers::Network::ignoreTlsErrors() const
{
  return m_ignoreTlsErrors;
}

/**
 * @brief Returns the configured REST endpoint.
 */
const QString& IO::Drivers::Network::httpUrl() const
{
  return m_httpUrl;
}

/**
 * @brief Returns the request body sent with every poll.
 */
const QString& IO::Drivers::Network::httpBody() const
{
  return m_httpBody;
}

/**
 * @brief Returns the custom request headers, one @c Name: @c Value pair per line.
 */
const QString& IO::Drivers::Network::httpHeaders() const
{
  return m_httpHeaders;
}

/**
 * @brief Returns the HTTP method as an index of httpMethods().
 */
int IO::Drivers::Network::httpMethodIndex() const
{
  return m_httpMethod;
}

/**
 * @brief Returns the poll interval in milliseconds; 0 means requests are sent only on a write.
 */
int IO::Drivers::Network::httpInterval() const
{
  return m_httpInterval;
}

/**
 * @brief Returns how many polls of this run returned a usable response.
 */
quint64 IO::Drivers::Network::pollsOk() const
{
  return m_pollsOk;
}

/**
 * @brief Returns how many polls of this run failed.
 */
quint64 IO::Drivers::Network::pollsFailed() const
{
  return m_pollsFailed;
}

/**
 * @brief Returns how many polls were skipped because the previous response had not arrived.
 */
quint64 IO::Drivers::Network::pollsSkipped() const
{
  return m_pollsSkipped;
}

/**
 * @brief Returns the length of the current run of consecutive failures; a success resets it.
 */
quint64 IO::Drivers::Network::consecutiveFailures() const
{
  return m_consecutiveFailures;
}

/**
 * @brief Returns the active socket type (TCP or UDP).
 */
IO::Drivers::Network::SocketType IO::Drivers::Network::socketType() const
{
  return m_socketType;
}

/**
 * @brief Instructs the module to communicate via a TCP socket.
 */
void IO::Drivers::Network::setTcpSocket()
{
  setSocketType(SocketType::Tcp);
}

/**
 * @brief Instructs the module to communicate via a UDP socket.
 */
void IO::Drivers::Network::setUdpSocket()
{
  setSocketType(SocketType::Udp);
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
 * @brief Changes the socket type given an index of the socketTypes() list. An index this build
 *        does not know is ignored rather than applied, so a project written by a newer build
 *        leaves this one on its previous transport instead of an unusable one.
 */
void IO::Drivers::Network::setSocketTypeIndex(const int index)
{
  if (index < 0 || index >= socketTypes().count())
    return;

  setSocketType(static_cast<SocketType>(index));
}

/**
 * @brief Sets the WebSocket endpoint. The URL is stored as typed and validated at connect time,
 *        so a half-typed address does not spend the edit fighting the field.
 */
void IO::Drivers::Network::setWebSocketUrl(const QString& url)
{
  if (m_webSocketUrl == url)
    return;

  m_webSocketUrl = url;
  m_settings.setValue("NetworkDriver/webSocketUrl", url);
  Q_EMIT webSocketChanged();
}

/**
 * @brief Chooses how written payloads are framed: automatic, text, or binary.
 */
void IO::Drivers::Network::setWebSocketFormatIndex(const int index)
{
  if (m_webSocketFormat == index || index < 0 || index >= webSocketFormats().count())
    return;

  m_webSocketFormat = index;
  m_settings.setValue("NetworkDriver/webSocketFormat", index);
  Q_EMIT webSocketChanged();
}

/**
 * @brief Enables or disables the certificate-verification bypass used by wss:// and https://.
 */
void IO::Drivers::Network::setIgnoreTlsErrors(const bool enabled)
{
  if (m_ignoreTlsErrors == enabled)
    return;

  m_ignoreTlsErrors = enabled;
  m_settings.setValue("NetworkDriver/ignoreTlsErrors", enabled);
  Q_EMIT ignoreTlsErrorsChanged();
}

/**
 * @brief Sets the REST endpoint. Stored as typed and validated at connect time, so a half-typed
 *        address does not spend the edit fighting the field.
 */
void IO::Drivers::Network::setHttpUrl(const QString& url)
{
  if (m_httpUrl == url)
    return;

  m_httpUrl = url;
  m_settings.setValue("NetworkDriver/httpUrl", url);
  Q_EMIT httpChanged();
}

/**
 * @brief Sets the body sent with every request.
 */
void IO::Drivers::Network::setHttpBody(const QString& body)
{
  if (m_httpBody == body)
    return;

  m_httpBody = body;
  m_settings.setValue("NetworkDriver/httpBody", body);
  Q_EMIT httpChanged();
}

/**
 * @brief Sets the custom request headers, one @c Name: @c Value pair per line.
 */
void IO::Drivers::Network::setHttpHeaders(const QString& headers)
{
  if (m_httpHeaders == headers)
    return;

  m_httpHeaders = headers;
  m_settings.setValue("NetworkDriver/httpHeaders", headers);
  Q_EMIT httpChanged();
}

/**
 * @brief Selects the HTTP method by index of httpMethods().
 */
void IO::Drivers::Network::setHttpMethodIndex(const int index)
{
  if (m_httpMethod == index || index < 0 || index >= httpMethods().count())
    return;

  m_httpMethod = index;
  m_settings.setValue("NetworkDriver/httpMethod", index);
  Q_EMIT httpChanged();
}

/**
 * @brief Sets the poll interval. Zero keeps the source silent until something is written; any
 *        other value is floored, because a handful of microseconds between requests is a way to
 *        hammer a public API by accident rather than a usable sample rate.
 */
void IO::Drivers::Network::setHttpInterval(const int interval)
{
  const int bounded = interval <= 0 ? 0 : qMax(interval, kHttpMinIntervalMs);
  if (m_httpInterval == bounded)
    return;

  m_httpInterval = bounded;
  m_settings.setValue("NetworkDriver/httpInterval", bounded);
  Q_EMIT httpChanged();
}

/**
 * @brief Changes the socket type.
 */
void IO::Drivers::Network::setSocketType(const SocketType type)
{
  if (m_socketType == type)
    return;

  m_socketType = type;
  m_settings.setValue("NetworkDriver/socketType", static_cast<int>(type));
  Q_EMIT socketTypeChanged();
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
 * @brief Handles an error on an established link: report once, tear down, stay down. Dial
 *        failures never reach here, because the dial's own verdict owns them and the error
 *        handler is wired only after it succeeds; the teardown is queued on the connection
 *        manager because it destroys this driver.
 */
void IO::Drivers::Network::reportLinkError(const QString& error)
{
  static auto& connectionManager = ConnectionManager::instance();
  logDriverError(tr("Network socket error"), error);
  connectionManager.disconnectDevice(this);
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Network configuration as a flat list of editable properties: every
 *        transport's rows, not just the active one's. The list IS what a project persists, and
 *        setDriverProperty() already offers every key to every transport, so gating the rows on
 *        the current type silently dropped the other three transports' settings on save.
 */
QList<IO::DriverProperty> IO::Drivers::Network::driverProperties() const
{
  QList<IO::DriverProperty> props;
  appendSocketTypeProperty(props);
  appendAddressProperty(props);
  appendTcpProperties(props);
  appendUdpProperties(props);
  appendWebSocketProperties(props);
  appendHttpProperties(props);
  appendTlsProperty(props);

  return props;
}

/**
 * @brief Appends the TLS bypass row shared by the URL transports.
 */
void IO::Drivers::Network::appendTlsProperty(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty tls;
  tls.key         = QStringLiteral("ignoreTlsErrors");
  tls.label       = tr("Ignore TLS Errors");
  tls.description = tr("Accept self-signed or mismatched certificates");
  tls.type        = IO::DriverProperty::CheckBox;
  tls.value       = m_ignoreTlsErrors;
  props.append(tls);
}

/**
 * @brief Applies the shared TLS property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyTlsProperty(const QString& key, const QVariant& value)
{
  if (key != QLatin1String("ignoreTlsErrors"))
    return false;

  setIgnoreTlsErrors(value.toBool());
  return true;
}

/**
 * @brief Appends the transport selector, the one row every socket type shows.
 */
void IO::Drivers::Network::appendSocketTypeProperty(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty socketTypeProp;
  socketTypeProp.key     = QStringLiteral("socketTypeIndex");
  socketTypeProp.label   = tr("Socket Type");
  socketTypeProp.type    = IO::DriverProperty::ComboBox;
  socketTypeProp.value   = socketTypeIndex();
  socketTypeProp.options = socketTypes();
  props.append(socketTypeProp);
}

/**
 * @brief Appends the remote address row shared by the host-and-port transports.
 */
void IO::Drivers::Network::appendAddressProperty(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty addr;
  addr.key   = QStringLiteral("address");
  addr.label = tr("Remote Address");
  addr.type  = IO::DriverProperty::Text;
  addr.value = m_address;
  props.append(addr);
}

/**
 * @brief Applies the shared address property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyAddressProperty(const QString& key, const QVariant& value)
{
  if (key != QLatin1String("address"))
    return false;

  setRemoteAddress(value.toString());
  return true;
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

  if (applyAddressProperty(key, value))
    return;

  if (applyTcpProperty(key, value))
    return;

  if (applyUdpProperty(key, value))
    return;

  if (applyTlsProperty(key, value))
    return;

  if (applyWebSocketProperty(key, value))
    return;

  (void)applyHttpProperty(key, value);
}
