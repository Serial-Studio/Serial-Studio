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

// Winsock must precede anything that can reach <windows.h>, hence above the Qt includes
#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <fcntl.h>
#  include <netdb.h>
#  include <poll.h>
#  include <unistd.h>

#  include <cerrno>
#  include <sys/socket.h>
#endif

#include <QElapsedTimer>
#include <QThread>

#include "IO/Drivers/Network.h"
#include "SSAssert.h"

static constexpr int kNetworkDialDeadlineMs = 5000;
static constexpr int kNetworkDialPaceMs     = 250;

#ifdef Q_OS_WIN
using ProbeSocket                           = SOCKET;
using ProbeLen                              = int;
using ProbePoll                             = WSAPOLLFD;
static constexpr ProbeSocket kNoProbeSocket = INVALID_SOCKET;
static constexpr int kProbeRefused          = WSAECONNREFUSED;
#else
using ProbeSocket                           = int;
using ProbeLen                              = socklen_t;
using ProbePoll                             = pollfd;
static constexpr ProbeSocket kNoProbeSocket = -1;
static constexpr int kProbeRefused          = ECONNREFUSED;
#endif

/**
 * @brief Waits for the probe socket to become writable; a wrapper because clang-cl refuses the
 *        dllimport'ed WSAPoll address as a constant expression.
 */
static inline int probePoll(ProbePoll* fds, int timeoutMs)
{
#ifdef Q_OS_WIN
  return ::WSAPoll(fds, 1, timeoutMs);
#else
  return ::poll(fds, 1, timeoutMs);
#endif
}

/**
 * @brief Whether the last connect() merely started an asynchronous dial rather than failing.
 */
[[nodiscard]] static bool probeConnectPending()
{
#ifdef Q_OS_WIN
  return ::WSAGetLastError() == WSAEWOULDBLOCK;
#else
  return errno == EINPROGRESS;
#endif
}

/**
 * @brief Whether the last connect() failed outright because the endpoint refused the connection.
 */
[[nodiscard]] static bool probeLastErrorRefused()
{
#ifdef Q_OS_WIN
  return ::WSAGetLastError() == WSAECONNREFUSED;
#else
  return errno == ECONNREFUSED;
#endif
}

/**
 * @brief Closes a probe descriptor on either platform.
 */
static void closeProbeSocket(ProbeSocket handle)
{
#ifdef Q_OS_WIN
  ::closesocket(handle);
#else
  ::close(handle);
#endif
}

/**
 * @brief Puts a probe descriptor in non-blocking mode so the connect can be bounded by select().
 */
[[nodiscard]] static bool markProbeNonBlocking(ProbeSocket handle)
{
#ifdef Q_OS_WIN
  u_long mode = 1;
  return ::ioctlsocket(handle, FIONBIO, &mode) == 0;
#else
  const int flags = ::fcntl(handle, F_GETFL, 0);
  return flags != -1 && ::fcntl(handle, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

/**
 * @brief Waits inside the deadline for @p host:@p port to accept, re-pacing refused attempts (a
 *        script-launched helper needs a moment to bind). Each attempt uses a throwaway socket
 *        destroyed inside the blocked section: abort-and-redial churn on a run-loop-registered
 *        socket leaves stale CFSocket sources that crashed readFromSocket (2026-08-10, macOS).
 */
/**
 * @brief Runs one bounded non-blocking connect against @p address, reporting an active refusal
 *        separately so the caller retries only while something is listening but not yet ready.
 *        poll() rather than select(): a descriptor past FD_SETSIZE overruns an fd_set, and the
 *        app holds session-DB, export and device descriptors alongside this one.
 */
[[nodiscard]] static bool probeTcpOnce(const addrinfo* address, int timeoutMs, bool& refused)
{
  SS_ASSERT(address != nullptr, return false);
  SS_ASSERT(timeoutMs >= 0, timeoutMs = 0);

  refused = false;

  const ProbeSocket handle =
    ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  if (handle == kNoProbeSocket)
    return false;

  if (!markProbeNonBlocking(handle)) {
    closeProbeSocket(handle);
    return false;
  }

  const int dialed =
    ::connect(handle, address->ai_addr, static_cast<ProbeLen>(address->ai_addrlen));
  if (dialed == 0) {
    closeProbeSocket(handle);
    return true;
  }

  if (!probeConnectPending()) {
    refused = probeLastErrorRefused();
    closeProbeSocket(handle);
    return false;
  }

  ProbePoll pending{};
  pending.fd     = handle;
  pending.events = POLLOUT;

  const int ready = probePoll(&pending, timeoutMs);
  if (ready <= 0) {
    closeProbeSocket(handle);
    return false;
  }

  int error          = 0;
  ProbeLen errorSize = sizeof(error);
  const int queried =
    ::getsockopt(handle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &errorSize);
  closeProbeSocket(handle);

  refused = (queried == 0 && error == kProbeRefused);
  return queried == 0 && error == 0;
}

/**
 * @brief Waits for @p host:@p port to accept connections, retrying only while the endpoint
 *        actively refuses. Every resolved address is tried, as QTcpSocket::connectToHost() does:
 *        probing only the first record makes this stricter than the dial it gates, and a
 *        dual-stack host commonly resolves ::1 ahead of an IPv4-only listener.
 */
static bool waitForTcpEndpoint(const QString& host, quint16 port, QString& reason)
{
  SS_ASSERT(port != 0, return false);
  SS_ASSERT_LOG(!host.isEmpty());

  addrinfo hints{};
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_ADDRCONFIG;

  const QByteArray node    = host.toUtf8();
  const QByteArray service = QByteArray::number(port);

  addrinfo* resolved = nullptr;
  if (::getaddrinfo(node.constData(), service.constData(), &hints, &resolved) != 0 || !resolved) {
    reason = QObject::tr("Host not found");
    return false;
  }

  QElapsedTimer deadline;
  deadline.start();

  while (deadline.elapsed() < kNetworkDialDeadlineMs) {
    bool refusedAny = false;
    for (const addrinfo* it = resolved; it != nullptr; it = it->ai_next) {
      const int remaining = kNetworkDialDeadlineMs - static_cast<int>(deadline.elapsed());
      if (remaining <= 0)
        break;

      bool refused = false;
      if (probeTcpOnce(it, remaining, refused)) {
        ::freeaddrinfo(resolved);
        return true;
      }

      refusedAny = refusedAny || refused;
    }

    if (!refusedAny) {
      reason = QObject::tr("Connection failed");
      ::freeaddrinfo(resolved);
      return false;
    }

    QThread::msleep(kNetworkDialPaceMs);
  }

  reason = QObject::tr("Connection timed out");
  ::freeaddrinfo(resolved);
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
  return m_tcpSocket->state() == QAbstractSocket::ConnectedState;
}

/**
 * @brief Dials TCP synchronously under the connect wait cursor and returns the final verdict.
 *        A throwaway probe absorbs the retry churn; the driver's own socket then connects
 *        exactly once per open(), never seeing an abort-and-redial cycle. The readyRead and
 *        errorOccurred handlers are wired only on success: dial failures are owned here.
 */
bool IO::Drivers::Network::dialTcpBlocking(const QString& host, const QIODevice::OpenMode mode)
{
  QString reason;
  if (!waitForTcpEndpoint(host, tcpPort(), reason)) {
    logDriverError(
      tr("Network socket error"),
      tr("Cannot connect to %1:%2 (%3)").arg(host, QString::number(tcpPort()), reason));
    return false;
  }

  m_tcpSocket->connectToHost(host, tcpPort(), mode);
  if (!m_tcpSocket->waitForConnected(kNetworkDialDeadlineMs)) {
    const QString finalReason = m_tcpSocket->errorString();
    m_tcpSocket->abort();
    logDriverError(
      tr("Network socket error"),
      tr("Cannot connect to %1:%2 (%3)").arg(host, QString::number(tcpPort()), finalReason));
    return false;
  }

  connect(m_tcpSocket,
          &QTcpSocket::readyRead,
          this,
          &IO::Drivers::Network::onTcpReadyRead,
          Qt::UniqueConnection);
  connect(m_tcpSocket,
          &QTcpSocket::errorOccurred,
          this,
          &IO::Drivers::Network::onTcpError,
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
 * @brief Dials the configured TCP endpoint, falling back to the default address when the user
 *        left the field empty. The blocking dial returns the final verdict.
 */
bool IO::Drivers::Network::openTcp(const QIODevice::OpenMode mode)
{
  auto hostAddr = remoteAddress();
  if (hostAddr.isEmpty())
    hostAddr = defaultAddress();

  return dialTcpBlocking(hostAddr, mode);
}

/**
 * @brief Retires the TCP link and its handlers. Safe on an idle socket, which is what lets
 *        close() tear down every transport unconditionally.
 */
void IO::Drivers::Network::closeTcp()
{
  disconnect(m_tcpSocket, &QTcpSocket::readyRead, this, &IO::Drivers::Network::onTcpReadyRead);
  disconnect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Network::onTcpError);

  m_tcpSocket->abort();
  m_tcpSocket->close();
  m_tcpSocket->disconnectFromHost();
}

/**
 * @brief Writes @p data to the TCP stream.
 */
qint64 IO::Drivers::Network::writeTcp(const QByteArray& data)
{
  return m_tcpSocket->write(data);
}

/**
 * @brief Returns true for an open socket carrying an established link.
 */
bool IO::Drivers::Network::tcpOpen() const
{
  return m_tcpSocket->isOpen() && tcpLinkUp();
}

/**
 * @brief Returns true when the TCP socket can be read.
 */
bool IO::Drivers::Network::tcpReadable() const
{
  return m_tcpSocket->isReadable();
}

/**
 * @brief Returns true when the TCP socket can be written.
 */
bool IO::Drivers::Network::tcpWritable() const
{
  return m_tcpSocket->isWritable();
}

/**
 * @brief Returns true when the TCP port is usable. Host validity is deliberately not checked
 *        here: open() falls back to the default address and connectToHost() resolves names.
 */
bool IO::Drivers::Network::tcpConfigured() const
{
  return tcpPort() > 0;
}

/**
 * @brief Publishes everything buffered on the TCP stream.
 */
void IO::Drivers::Network::onTcpReadyRead()
{
  publishReceivedData(m_tcpSocket->readAll());
}

/**
 * @brief Reports an error on an established TCP link.
 */
void IO::Drivers::Network::onTcpError()
{
  reportLinkError(m_tcpSocket->errorString());
}

/**
 * @brief Appends the TCP rows of the driver property model.
 */
void IO::Drivers::Network::appendTcpProperties(QList<IO::DriverProperty>& props) const
{
  appendAddressProperty(props);

  IO::DriverProperty tcp;
  tcp.key   = QStringLiteral("tcpPort");
  tcp.label = tr("TCP Port");
  tcp.type  = IO::DriverProperty::IntField;
  tcp.value = m_tcpPort;
  tcp.min   = 1;
  tcp.max   = 65535;
  props.append(tcp);
}

/**
 * @brief Applies a TCP property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyTcpProperty(const QString& key, const QVariant& value)
{
  if (key != QLatin1String("tcpPort"))
    return false;

  setTcpPort(static_cast<quint16>(value.toInt()));
  return true;
}
