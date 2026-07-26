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

#include "Misc/Diagnostics/NetworkChecks.h"

#include <QHostAddress>
#include <QHostInfo>
#include <utility>

#include "IO/ConnectionManager.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Misc::Diagnostics::Bus;
using Misc::Diagnostics::makeResult;
using Misc::Diagnostics::Result;
using Misc::Diagnostics::trDiag;
using Misc::Diagnostics::Verdict;

static constexpr int kInvalidLookup    = -1;
static constexpr int kLookupTimeoutMs  = 2000;
static constexpr int kConnectTimeoutMs = 3000;

//--------------------------------------------------------------------------------------------------
// Host lookup task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a lookup step bound to @p host.
 */
Misc::Diagnostics::NetworkChecks::HostLookupTask::HostLookupTask(QString host, QObject* parent)
  : Async::Task(QString::fromLatin1(kHostLookupStep), parent)
  , m_lookupId(kInvalidLookup)
  , m_host(std::move(host))
{
  SS_ASSERT_LOG(!m_host.isEmpty());
}

/**
 * @brief Aborts any pending lookup so a destroyed step cannot deliver a continuation.
 */
Misc::Diagnostics::NetworkChecks::HostLookupTask::~HostLookupTask()
{
  abortLookup();
}

/**
 * @brief Resolves the host name, short-circuiting a literal address so a run against 127.0.0.1
 *        never touches the resolver.
 */
void Misc::Diagnostics::NetworkChecks::HostLookupTask::doStart()
{
  SS_ASSERT(!m_host.isEmpty(), {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("no host configured")));
    return;
  });
  SS_ASSERT_LOG(m_lookupId == kInvalidLookup);

  QHostAddress literal;
  if (literal.setAddress(m_host)) {
    reportFinished(Async::Outcome::Success, Async::StepError());
    return;
  }

  m_lookupId =
    QHostInfo::lookupHost(m_host, this, [this](const QHostInfo& info) { onLookupFinished(info); });
}

/**
 * @brief Cancels the pending resolver request.
 */
void Misc::Diagnostics::NetworkChecks::HostLookupTask::doCancel()
{
  abortLookup();
}

/**
 * @brief Releases the resolver request handle, if one is outstanding.
 */
void Misc::Diagnostics::NetworkChecks::HostLookupTask::abortLookup()
{
  if (m_lookupId == kInvalidLookup)
    return;

  QHostInfo::abortHostLookup(m_lookupId);
  m_lookupId = kInvalidLookup;
}

/**
 * @brief Finishes the step on the resolver's answer; an empty address list is a failure even when
 *        the resolver itself reported no error.
 */
void Misc::Diagnostics::NetworkChecks::HostLookupTask::onLookupFinished(const QHostInfo& info)
{
  m_lookupId = kInvalidLookup;
  if (!isRunning())
    return;

  if (info.error() == QHostInfo::NoError && !info.addresses().isEmpty()) {
    reportFinished(Async::Outcome::Success, Async::StepError());
    return;
  }

  auto reason = info.errorString();
  if (reason.isEmpty())
    reason = QStringLiteral("host not found");

  reportFinished(Async::Outcome::Failure, errorHere(reason));
}

//--------------------------------------------------------------------------------------------------
// TCP probe task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a connect step bound to @p host and @p port.
 */
Misc::Diagnostics::NetworkChecks::TcpProbeTask::TcpProbeTask(QString host,
                                                             quint16 port,
                                                             QObject* parent)
  : Async::Task(QString::fromLatin1(kTcpProbeStep), parent), m_port(port), m_host(std::move(host))
{
  SS_ASSERT_LOG(!m_host.isEmpty());
  SS_ASSERT_LOG(m_port != 0);
}

/**
 * @brief Tears the socket down so a destroyed step cannot deliver a continuation.
 */
Misc::Diagnostics::NetworkChecks::TcpProbeTask::~TcpProbeTask()
{
  releaseConnections();
  m_socket.abort();
}

/**
 * @brief Starts the connection attempt. Nothing is written and nothing is read: the socket exists
 *        only to learn whether the endpoint accepts a connection.
 */
void Misc::Diagnostics::NetworkChecks::TcpProbeTask::doStart()
{
  SS_ASSERT(!m_host.isEmpty() && m_port != 0, {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("no endpoint configured")));
    return;
  });

  m_connectedConnection =
    connect(&m_socket, &QTcpSocket::connected, this, &TcpProbeTask::onConnected);
  m_errorConnection =
    connect(&m_socket, &QAbstractSocket::errorOccurred, this, &TcpProbeTask::onErrorOccurred);

  m_socket.connectToHost(m_host, m_port, QIODevice::ReadOnly);
}

/**
 * @brief Drops the socket without reporting, so the wrapper's cancellation stays the outcome.
 */
void Misc::Diagnostics::NetworkChecks::TcpProbeTask::doCancel()
{
  releaseConnections();
  m_socket.abort();
}

/**
 * @brief Disconnects both socket handlers, which is what makes the abort() calls below unable to
 *        re-enter this task while it is finishing.
 */
void Misc::Diagnostics::NetworkChecks::TcpProbeTask::releaseConnections()
{
  QObject::disconnect(m_errorConnection);
  QObject::disconnect(m_connectedConnection);
  m_errorConnection     = QMetaObject::Connection();
  m_connectedConnection = QMetaObject::Connection();
}

/**
 * @brief Closes the connection the instant it is established and reports the endpoint reachable.
 */
void Misc::Diagnostics::NetworkChecks::TcpProbeTask::onConnected()
{
  if (!isRunning())
    return;

  releaseConnections();
  m_socket.abort();
  reportFinished(Async::Outcome::Success, Async::StepError());
}

/**
 * @brief Reports the endpoint unreachable, carrying the socket's own reason.
 */
void Misc::Diagnostics::NetworkChecks::TcpProbeTask::onErrorOccurred(
  QAbstractSocket::SocketError error)
{
  Q_UNUSED(error)

  if (!isRunning())
    return;

  auto reason = m_socket.errorString();
  if (reason.isEmpty())
    reason = QStringLiteral("connection failed");

  releaseConnections();
  m_socket.abort();
  reportFinished(Async::Outcome::Failure, errorHere(reason));
}

//--------------------------------------------------------------------------------------------------
// Configuration reads
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the network source's TCP endpoint; UDP carries no connection to probe.
 */
[[nodiscard]] static bool networkEndpoint(QString& host, quint16& port)
{
  static auto& manager = IO::ConnectionManager::instance();

  auto* network = manager.network();
  if (network == nullptr || network->socketType() != QAbstractSocket::TcpSocket)
    return false;

  host = network->remoteAddress();
  port = network->tcpPort();
  return true;
}

/**
 * @brief Reads the MQTT source's broker endpoint, absent outside commercial builds.
 */
[[nodiscard]] static bool brokerEndpoint(QString& host, quint16& port)
{
#ifdef BUILD_COMMERCIAL
  static auto& manager = IO::ConnectionManager::instance();

  auto* mqtt = manager.mqtt();
  if (mqtt == nullptr)
    return false;

  host = mqtt->hostname();
  port = mqtt->port();
  return true;
#else
  Q_UNUSED(host)
  Q_UNUSED(port)
  return false;
#endif
}

/**
 * @brief Reads the endpoint configured for @p bus, or reports that the bus has none.
 */
bool Misc::Diagnostics::NetworkChecks::endpoint(Bus bus, QString& host, quint16& port)
{
  if (bus == Bus::Network)
    return networkEndpoint(host, port);

  if (bus == Bus::Broker)
    return brokerEndpoint(host, port);

  return false;
}

/**
 * @brief Returns the declared worst case of one probe: the lookup budget plus the connect budget.
 */
int Misc::Diagnostics::NetworkChecks::probeBudgetMsec()
{
  return kLookupTimeoutMs + kConnectTimeoutMs;
}

//--------------------------------------------------------------------------------------------------
// Instant checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports an endpoint that cannot be probed because it was never fully configured.
 */
void Misc::Diagnostics::NetworkChecks::collectInstant(Bus bus, QList<Result>& out)
{
  QString host;
  quint16 port = 0;
  if (!endpoint(bus, host, port))
    return;

  if (host.isEmpty()) {
    out.append(makeResult(bus,
                          Verdict::Warning,
                          "host-not-configured",
                          trDiag("No host is configured"),
                          trDiag("This source has no host name or address, so there is nothing "
                                 "to connect to."),
                          trDiag("Enter the host name or IP address of the device in the device "
                                 "setup pane.")));
    return;
  }

  if (port == 0)
    out.append(
      makeResult(bus,
                 Verdict::Warning,
                 "port-not-configured",
                 trDiag("No port is configured"),
                 trDiag("Host %1 has no port number, so no connection can be opened.").arg(host),
                 trDiag("Enter the port the device listens on in the device setup "
                        "pane.")));
}

//--------------------------------------------------------------------------------------------------
// Probing flow
//--------------------------------------------------------------------------------------------------

/**
 * @brief Composes the bounded lookup and the bounded connect into one sequential flow.
 */
Async::Task* Misc::Diagnostics::NetworkChecks::makeReachabilityFlow(const QString& host,
                                                                    quint16 port,
                                                                    Async::AsyncClock& clock)
{
  SS_ASSERT_LOG(!host.isEmpty());
  SS_ASSERT_LOG(port != 0);

  auto* flow = Async::sequential(QStringLiteral("reachability"));
  flow->addChild(Async::timeout(new HostLookupTask(host), kLookupTimeoutMs, clock));
  flow->addChild(Async::timeout(new TcpProbeTask(host, port), kConnectTimeoutMs, clock));
  return flow;
}

/**
 * @brief Names the endpoint the way a user reads it back.
 */
[[nodiscard]] static QString endpointLabel(const QString& host, quint16 port)
{
  return QStringLiteral("%1:%2").arg(host, QString::number(port));
}

/**
 * @brief Builds the verdict for a lookup that did not resolve.
 */
[[nodiscard]] static Result unresolvedResult(Bus bus, const QString& host)
{
  return makeResult(bus,
                    Verdict::Failure,
                    "host-not-resolved",
                    trDiag("The host name did not resolve"),
                    trDiag("The name %1 could not be resolved to an address.").arg(host),
                    trDiag("Check the host name for a typo, or use the device's IP address "
                           "instead."));
}

/**
 * @brief Builds the verdict for an endpoint that refused the connection.
 */
[[nodiscard]] static Result refusedResult(Bus bus, const QString& host, quint16 port)
{
  return makeResult(bus,
                    Verdict::Failure,
                    "connection-refused",
                    trDiag("The connection was refused"),
                    trDiag("%1 resolved and answered, and refused a connection on that port.")
                      .arg(endpointLabel(host, port)),
                    trDiag("Confirm the port number, and that the service on the device is "
                           "running and accepting connections."));
}

/**
 * @brief Builds the verdict for an endpoint that never answered inside the budget.
 */
[[nodiscard]] static Result timedOutResult(Bus bus, const QString& host, quint16 port)
{
  return makeResult(
    bus,
    Verdict::Failure,
    "endpoint-timed-out",
    trDiag("The connection timed out"),
    trDiag("%1 did not answer within the time allowed for the check.")
      .arg(endpointLabel(host, port)),
    trDiag("Check that the device is powered and on the same network, and that no firewall is "
           "dropping the connection."));
}

/**
 * @brief Maps the flow's outcome and failing step onto one of the three reachability verdicts; a
 *        successful probe produces a Pass, which the caller stores nothing for.
 */
Result Misc::Diagnostics::NetworkChecks::reachabilityResult(
  Bus bus, const QString& host, quint16 port, Async::Outcome outcome, const Async::StepError& error)
{
  if (outcome == Async::Outcome::TimedOut)
    return timedOutResult(bus, host, port);

  if (outcome != Async::Outcome::Failure) {
    Result pass;
    pass.bus = bus;
    return pass;
  }

  if (error.step == QLatin1String(kHostLookupStep))
    return unresolvedResult(bus, host);

  return refusedResult(bus, host, port);
}
