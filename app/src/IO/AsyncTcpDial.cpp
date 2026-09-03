/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "IO/AsyncTcpDial.h"

#include "SSAssert.h"

static constexpr int kDefaultDeadlineMs = 5000;
static constexpr int kRefusalPaceMs     = 250;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an idle dialer. Both timers are single-shot: the deadline bounds the whole attempt
 *        and the pace timer spaces one refusal round from the next.
 */
IO::AsyncTcpDial::AsyncTcpDial(QObject* parent)
  : QObject(parent)
  , m_active(false)
  , m_probeEnabled(true)
  , m_refusedRound(false)
  , m_lookupId(-1)
  , m_deadlineMs(kDefaultDeadlineMs)
  , m_addressIndex(0)
  , m_port(0)
  , m_mode(QIODevice::ReadWrite)
  , m_probe(nullptr)
{
  m_paceTimer.setSingleShot(true);
  m_deadlineTimer.setSingleShot(true);

  connect(&m_paceTimer, &QTimer::timeout, this, &IO::AsyncTcpDial::onPaceElapsed);
  connect(&m_deadlineTimer, &QTimer::timeout, this, &IO::AsyncTcpDial::onDeadlineReached);
}

/**
 * @brief Ends any attempt still in flight without reporting a verdict; a destroyed dialer has no
 *        caller left to hear one.
 */
IO::AsyncTcpDial::~AsyncTcpDial()
{
  cancel();
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while an attempt is in flight; the drivers publish this as isConnecting().
 */
bool IO::AsyncTcpDial::active() const noexcept
{
  return m_active;
}

/**
 * @brief Returns the deadline that bounds a whole attempt, in milliseconds.
 */
int IO::AsyncTcpDial::deadline() const noexcept
{
  return m_deadlineMs;
}

/**
 * @brief Returns the address the probe accepted, so the caller can dial the literal instead of
 *        re-resolving the name inside connectToHost().
 */
QHostAddress IO::AsyncTcpDial::resolvedAddress() const noexcept
{
  return m_target;
}

//--------------------------------------------------------------------------------------------------
// Attempt lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the deadline covering resolution, probing and the connect. Ignored while an attempt
 *        is in flight, so a live attempt keeps the bound it started with.
 */
void IO::AsyncTcpDial::setDeadline(int milliseconds)
{
  SS_ASSERT(milliseconds > 0, return);

  if (!m_active)
    m_deadlineMs = milliseconds;
}

/**
 * @brief Turns the refusal probe off for start(), so the attempt resolves and then connects the
 *        caller's socket directly. A station that permits ONE client counts a probe as a second
 *        one, which is why IEC 104 dials without it; the retry window goes with the probe.
 */
void IO::AsyncTcpDial::setProbeEnabled(bool enabled)
{
  SS_ASSERT_LOG(!m_active);

  if (!m_active)
    m_probeEnabled = enabled;
}

/**
 * @brief Waits for @p host : @p port to accept, reporting the verdict without dialing a caller
 *        socket. Backs the drivers whose own stack owns the connect (Modbus, MQTT).
 */
void IO::AsyncTcpDial::startProbe(const QString& host, quint16 port)
{
  m_socket       = nullptr;
  m_probeEnabled = true;
  beginAttempt(host, port);
}

/**
 * @brief Resolves @p host and reports, touching no socket at all; resolvedAddress() then carries
 *        the literal the caller hands to a stack that would otherwise resolve synchronously.
 */
void IO::AsyncTcpDial::startResolve(const QString& host, quint16 port)
{
  m_socket       = nullptr;
  m_probeEnabled = false;
  beginAttempt(host, port);
}

/**
 * @brief Resolves @p host, waits for @p port to accept, then connects @p socket exactly once. The
 *        socket is never aborted here: a failed verdict leaves teardown to the caller's close(),
 *        which is the only place that owns the socket's lifetime.
 */
void IO::AsyncTcpDial::start(const QString& host,
                             quint16 port,
                             QTcpSocket* socket,
                             QIODevice::OpenMode mode)
{
  SS_ASSERT(socket != nullptr, return);

  m_mode   = mode;
  m_socket = socket;
  beginAttempt(host, port);
}

/**
 * @brief Starts an attempt: cancels whatever came before, arms the deadline, and either resolves
 *        the name or goes straight to the probe when the host is already a literal.
 */
void IO::AsyncTcpDial::beginAttempt(const QString& host, quint16 port)
{
  SS_ASSERT(port != 0, return);
  SS_ASSERT_LOG(!host.isEmpty());

  const QPointer<QTcpSocket> socket = m_socket;
  cancel();
  m_socket = socket;

  m_host         = host.simplified();
  m_port         = port;
  m_active       = true;
  m_addressIndex = 0;
  m_refusedRound = false;
  m_target       = QHostAddress();
  m_addresses.clear();

  m_deadlineTimer.start(m_deadlineMs);

  const QHostAddress literal(m_host);
  if (!literal.isNull()) {
    applyResolution({literal});
    return;
  }

  m_lookupId = QHostInfo::lookupHost(m_host, this, &IO::AsyncTcpDial::onLookupFinished);
}

/**
 * @brief Ends the attempt with no verdict: a cancel is not an open failure, and the manager drops
 *        the pending dial rather than reporting one. Idempotent.
 */
void IO::AsyncTcpDial::cancel()
{
  m_active = false;
  m_paceTimer.stop();
  m_deadlineTimer.stop();

  if (m_lookupId >= 0) {
    QHostInfo::abortHostLookup(m_lookupId);
    m_lookupId = -1;
  }

  dropProbe();
  releaseSocket();
}

/**
 * @brief Settles the attempt exactly once. The state is torn down before the emission so a caller
 *        that restarts a dial from its own handler runs against an idle object.
 */
void IO::AsyncTcpDial::report(bool ok, const QString& reason)
{
  SS_ASSERT_LOG(ok || !reason.isEmpty());

  if (!m_active)
    return;

  cancel();
  Q_EMIT finished(ok, reason);
}

//--------------------------------------------------------------------------------------------------
// Resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consumes the asynchronous name resolution. A lookup that lands after a cancel or for a
 *        superseded host is ignored, so a stale answer can never validate the current attempt.
 */
void IO::AsyncTcpDial::onLookupFinished(const QHostInfo& info)
{
  m_lookupId = -1;
  if (!m_active)
    return;

  if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
    report(false, tr("Host not found"));
    return;
  }

  applyResolution(info.addresses());
}

/**
 * @brief Orders the resolved addresses IPv4 first and starts probing them. A dual-stack name such
 *        as "localhost" resolves ::1 ahead of an IPv4-only listener, which is a dial that fails
 *        against a server that is plainly there.
 */
void IO::AsyncTcpDial::applyResolution(const QList<QHostAddress>& addresses)
{
  SS_ASSERT(!addresses.isEmpty(), return report(false, tr("Host not found")));

  m_addresses.clear();
  for (const auto& address : addresses)
    if (address.protocol() == QAbstractSocket::IPv4Protocol)
      m_addresses.append(address);

  for (const auto& address : addresses)
    if (address.protocol() != QAbstractSocket::IPv4Protocol)
      m_addresses.append(address);

  m_addressIndex = 0;
  m_refusedRound = false;

  if (!m_probeEnabled) {
    m_target = m_addresses.first();
    dialSocket();
    return;
  }

  probeNextAddress();
}

//--------------------------------------------------------------------------------------------------
// Refusal probe
//--------------------------------------------------------------------------------------------------

/**
 * @brief Probes the next resolved address on a throwaway socket. A round that ends with at least
 *        one active refusal is re-paced (a helper process may still be binding); a round where
 *        nothing refused means nothing is coming, and the attempt fails at once.
 */
void IO::AsyncTcpDial::probeNextAddress()
{
  if (!m_active)
    return;

  dropProbe();

  if (m_addressIndex >= m_addresses.count()) {
    if (!m_refusedRound) {
      report(false, tr("Connection failed"));
      return;
    }

    m_addressIndex = 0;
    m_refusedRound = false;
    m_paceTimer.start(kRefusalPaceMs);
    return;
  }

  m_probe = new QTcpSocket(this);
  connect(m_probe, &QTcpSocket::connected, this, &IO::AsyncTcpDial::onProbeConnected);
  connect(m_probe, &QTcpSocket::errorOccurred, this, &IO::AsyncTcpDial::onProbeError);
  m_probe->connectToHost(m_addresses.at(m_addressIndex), m_port, QIODevice::ReadOnly);
}

/**
 * @brief Restarts the address sweep after a paced refusal round.
 */
void IO::AsyncTcpDial::onPaceElapsed()
{
  probeNextAddress();
}

/**
 * @brief The endpoint accepted: keep the address that worked and hand the dial to the caller's
 *        socket. The probe is retired first so no run-loop source of it outlives the attempt.
 */
void IO::AsyncTcpDial::onProbeConnected()
{
  SS_ASSERT(m_probe != nullptr, return);
  SS_ASSERT(m_addressIndex < m_addresses.count(), return report(false, tr("Connection failed")));

  m_target = m_addresses.at(m_addressIndex);
  dropProbe();
  dialSocket();
}

/**
 * @brief A refusal keeps the round alive (something is listening, just not yet ready); any other
 *        error retires this address for the round.
 */
void IO::AsyncTcpDial::onProbeError(QAbstractSocket::SocketError error)
{
  if (!m_active)
    return;

  m_refusedRound = m_refusedRound || (error == QAbstractSocket::ConnectionRefusedError);
  ++m_addressIndex;
  probeNextAddress();
}

/**
 * @brief Retires the throwaway probe. deleteLater() rather than delete: a run-loop source
 *        scheduled for the socket still fires after abort() and must find a live object.
 */
void IO::AsyncTcpDial::dropProbe()
{
  if (m_probe == nullptr)
    return;

  m_probe->disconnect(this);
  m_probe->abort();
  m_probe->deleteLater();
  m_probe = nullptr;
}

//--------------------------------------------------------------------------------------------------
// Caller socket
//--------------------------------------------------------------------------------------------------

/**
 * @brief Connects the caller's socket to the address the probe accepted -- once, and to a literal,
 *        so connectToHost() never runs a synchronous resolver of its own. A probe-only attempt has
 *        no socket and settles here.
 */
void IO::AsyncTcpDial::dialSocket()
{
  SS_ASSERT_LOG(!m_target.isNull());

  if (m_socket.isNull()) {
    report(true, QString());
    return;
  }

  m_socketConnected =
    connect(m_socket.data(), &QTcpSocket::connected, this, &IO::AsyncTcpDial::onSocketConnected);
  m_socketFailed =
    connect(m_socket.data(), &QTcpSocket::errorOccurred, this, &IO::AsyncTcpDial::onSocketError);

  m_socket->connectToHost(m_target, m_port, m_mode);
}

/**
 * @brief The caller's socket is up: report success once and let go of it.
 */
void IO::AsyncTcpDial::onSocketConnected()
{
  report(true, QString());
}

/**
 * @brief The caller's socket failed. Only the verdict is reported: the socket belongs to the
 *        driver, whose close() owns the teardown, and closing it here would double-close.
 */
void IO::AsyncTcpDial::onSocketError(QAbstractSocket::SocketError error)
{
  Q_UNUSED(error);

  if (!m_active)
    return;

  const QString reason = m_socket.isNull() ? tr("Connection failed") : m_socket->errorString();
  report(false, reason.isEmpty() ? tr("Connection failed") : reason);
}

/**
 * @brief Drops this object's handlers on the caller's socket, leaving the socket itself untouched.
 */
void IO::AsyncTcpDial::releaseSocket()
{
  QObject::disconnect(m_socketConnected);
  QObject::disconnect(m_socketFailed);
  m_socketConnected = QMetaObject::Connection();
  m_socketFailed    = QMetaObject::Connection();
}

//--------------------------------------------------------------------------------------------------
// Deadline
//--------------------------------------------------------------------------------------------------

/**
 * @brief One deadline covers resolution, probing and the connect, so an attempt can neither hang
 *        on a dead resolver nor outlive the bound the driver advertised.
 */
void IO::AsyncTcpDial::onDeadlineReached()
{
  report(false, tr("Connection timed out"));
}
