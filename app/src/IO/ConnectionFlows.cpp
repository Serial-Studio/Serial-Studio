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

#include "IO/ConnectionFlows.h"

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants: the ceiling on a single open attempt, past which the step is failed and released
//--------------------------------------------------------------------------------------------------

static constexpr int kOpenTimeoutMsec = 15000;

//--------------------------------------------------------------------------------------------------
// Composition helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a step that fails immediately with @p reason, so a composer whose precondition
 *        was violated hands back a tree that reports rather than a null pointer its caller would
 *        run blind.
 */
static Async::Task* failedStep(const QString& name, const QString& reason)
{
  return Async::invoke(name, [reason](QString& out) {
    out = reason;
    return false;
  });
}

//--------------------------------------------------------------------------------------------------
// Driver open step
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an open step bound to one driver and open mode.
 */
IO::DriverOpenTask::DriverOpenTask(HAL_Driver* driver, QIODevice::OpenMode mode, QObject* parent)
  : Async::Task(QStringLiteral("driver-open"), parent), m_mode(mode), m_driver(driver)
{
  SS_ASSERT_LOG(driver != nullptr);
  SS_ASSERT_LOG(mode != QIODevice::NotOpen);
}

/**
 * @brief Destroys the step, releasing the outcome connection it may still hold.
 */
IO::DriverOpenTask::~DriverOpenTask()
{
  releaseConnection();
}

/**
 * @brief Subscribes to the driver's outcome before asking it to open, so an open that completes
 *        synchronously is still observed.
 */
void IO::DriverOpenTask::doStart()
{
  SS_ASSERT(isRunning(), return);
  SS_ASSERT(m_mode != QIODevice::NotOpen, {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("invalid open mode")));
    return;
  });

  if (m_driver.isNull()) {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("driver is gone")));
    return;
  }

  m_connection =
    connect(m_driver.data(), &HAL_Driver::openFinished, this, &IO::DriverOpenTask::onOpenFinished);

  m_driver->beginOpen(m_mode);
}

/**
 * @brief Releases the outcome connection and asks the driver to abandon the attempt, which is
 *        what makes a cancel real instead of merely ignored.
 */
void IO::DriverOpenTask::doCancel()
{
  SS_ASSERT_LOG(isRunning());

  releaseConnection();
  if (!m_driver.isNull())
    m_driver->abortOpen();
}

/**
 * @brief Tears down the outcome connection; a stale handle is a no-op.
 */
void IO::DriverOpenTask::releaseConnection()
{
  QObject::disconnect(m_connection);
  m_connection = QMetaObject::Connection();
}

/**
 * @brief Maps the driver's reported outcome onto the step's, substituting a reason when the
 *        driver left one out so no failure reaches the owner without provenance.
 */
void IO::DriverOpenTask::onOpenFinished(bool ok, const QString& reason)
{
  SS_ASSERT_LOG(!m_driver.isNull());

  if (!isRunning())
    return;

  releaseConnection();
  if (ok) {
    reportFinished(Async::Outcome::Success, Async::StepError());
    return;
  }

  const QString why = reason.isEmpty() ? QStringLiteral("open failed") : reason;
  reportFinished(Async::Outcome::Failure, errorHere(why));
}

//--------------------------------------------------------------------------------------------------
// Socket connect step
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a connect step bound to one socket and endpoint.
 */
IO::SocketConnectTask::SocketConnectTask(QAbstractSocket* socket,
                                         QString host,
                                         quint16 port,
                                         QObject* parent)
  : Async::Task(QStringLiteral("socket-connect"), parent)
  , m_port(port)
  , m_generation(0)
  , m_host(std::move(host))
  , m_socket(socket)
{
  SS_ASSERT_LOG(port > 0);
  SS_ASSERT_LOG(socket != nullptr);
}

/**
 * @brief Destroys the step, releasing the outcome connections it may still hold.
 */
IO::SocketConnectTask::~SocketConnectTask()
{
  releaseConnections();
}

/**
 * @brief Aborts any prior use of the socket, then defers the dial one event-loop turn: signals
 *        still queued from an earlier attempt on the same reused socket drain while nothing is
 *        armed, so a stale connected/error event can never masquerade as this attempt's outcome.
 */
void IO::SocketConnectTask::doStart()
{
  SS_ASSERT(isRunning(), return);
  SS_ASSERT(!m_host.isEmpty(), {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("no host to connect to")));
    return;
  });

  if (m_socket.isNull()) {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("socket is gone")));
    return;
  }

  m_socket->abort();

  const int generation = ++m_generation;
  QMetaObject::invokeMethod(
    this, [this, generation] { beginDial(generation); }, Qt::QueuedConnection);
}

/**
 * @brief Arms the outcome signals and dials, on a fresh event-loop turn. The generation check
 *        drops a dial that a restart of this task has superseded; the trailing state check covers
 *        a connect the platform completed before any signal could be delivered, validated against
 *        a live peer so a desynced socket state machine cannot fake it.
 */
void IO::SocketConnectTask::beginDial(int generation)
{
  if (!isRunning() || generation != m_generation)
    return;

  if (m_socket.isNull()) {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("socket is gone")));
    return;
  }

  m_okConnection = connect(
    m_socket.data(), &QAbstractSocket::connected, this, &IO::SocketConnectTask::onConnected);
  m_errorConnection = connect(
    m_socket.data(), &QAbstractSocket::errorOccurred, this, &IO::SocketConnectTask::onSocketError);

  m_socket->connectToHost(m_host, m_port);

  if (isRunning() && !m_socket.isNull() && hasLiveConnection())
    onConnected();
}

/**
 * @brief Returns whether the socket holds a kernel-backed live connection: ConnectedState alone
 *        is not trusted, because a reused QAbstractSocket can report it with no descriptor behind
 *        it (observed as ConnectedState with peerPort 0), and that phantom must never count.
 */
bool IO::SocketConnectTask::hasLiveConnection() const
{
  return !m_socket.isNull() && m_socket->state() == QAbstractSocket::ConnectedState
      && m_socket->peerPort() != 0;
}

/**
 * @brief Releases the outcome connections and abandons the dial, which is what makes a cancel
 *        during a connect attempt immediate instead of merely ignored.
 */
void IO::SocketConnectTask::doCancel()
{
  SS_ASSERT_LOG(isRunning());

  ++m_generation;
  releaseConnections();
  if (!m_socket.isNull())
    m_socket->abort();
}

/**
 * @brief Finishes the step once the socket reports a live, kernel-backed connection. A connected
 *        signal without a live peer is a stale or phantom event: it is ignored rather than
 *        trusted, and the attempt's timeout bounds the wait for the real outcome.
 */
void IO::SocketConnectTask::onConnected()
{
  if (!isRunning())
    return;

  if (!hasLiveConnection())
    return;

  releaseConnections();
  reportFinished(Async::Outcome::Success, Async::StepError());
}

/**
 * @brief Finishes the step with the socket's own error text, so the failure names what the
 *        transport reported rather than a bare deadline.
 */
void IO::SocketConnectTask::onSocketError()
{
  if (!isRunning())
    return;

  QString why = m_socket.isNull() ? QString() : m_socket->errorString();
  if (why.isEmpty())
    why = QStringLiteral("socket connect failed");

  releaseConnections();
  reportFinished(Async::Outcome::Failure, errorHere(why));
}

/**
 * @brief Tears down both outcome connections; stale handles are a no-op.
 */
void IO::SocketConnectTask::releaseConnections()
{
  QObject::disconnect(m_okConnection);
  QObject::disconnect(m_errorConnection);
  m_okConnection    = QMetaObject::Connection();
  m_errorConnection = QMetaObject::Connection();
}

//--------------------------------------------------------------------------------------------------
// Link supervisor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a supervisor around the open flow it re-runs, adopting it. The recovery
 *        policy is the schedule a drop is brought back on, which is deliberately not the one a
 *        user waited behind for the first connect.
 */
IO::SupervisorTask::SupervisorTask(HAL_Driver* driver,
                                   Async::Task* child,
                                   const Async::RetryPolicy& recovery,
                                   QObject* parent)
  : Async::Task(QStringLiteral("link-supervisor"), parent)
  , m_child(child)
  , m_driver(driver)
  , m_recovery(recovery)
{
  SS_ASSERT_LOG(driver != nullptr);
  SS_ASSERT(child != nullptr, return);

  m_child->setParent(this);
  connect(m_child, &Async::Task::finished, this, &IO::SupervisorTask::onChildFinished);
}

/**
 * @brief Destroys the supervisor, releasing the drop watch it may still hold.
 */
IO::SupervisorTask::~SupervisorTask()
{
  releaseConnection();
}

/**
 * @brief Returns how many open attempts the current sequence has made, which is zero while the
 *        link is idle or up: the retry wrapper clears it the moment an attempt succeeds, and a
 *        supervisor that already gave up reports nothing rather than its last count forever.
 */
int IO::SupervisorTask::attempt() const
{
  if (!isRunning())
    return 0;

  const auto* retry = qobject_cast<const Async::RetryTask*>(m_child);
  return retry ? retry->attempt() : 0;
}

/**
 * @brief Arms the drop watch, then runs the open flow. The watch is wired before the first
 *        attempt rather than on its success, so a drop whose events were already queued when the
 *        flow reported success cannot slip past an unarmed listener and strand the link down.
 */
void IO::SupervisorTask::doStart()
{
  SS_ASSERT(isRunning(), return);
  SS_ASSERT(m_child != nullptr, {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("no flow to supervise")));
    return;
  });

  if (m_driver.isNull()) {
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("driver is gone")));
    return;
  }

  releaseConnection();
  m_connection =
    connect(m_driver.data(), &HAL_Driver::linkDropped, this, &IO::SupervisorTask::onLinkDropped);

  m_child->start();
}

/**
 * @brief Stops supervising and stops the flow, whichever of the two is in flight.
 */
void IO::SupervisorTask::doCancel()
{
  SS_ASSERT_LOG(isRunning());

  releaseConnection();
  SS_ASSERT(m_child != nullptr, return);

  if (m_child->isRunning())
    m_child->cancel();
}

/**
 * @brief Re-runs the open flow on the recovery schedule after an unsolicited drop, silently. A
 *        drop reported while the driver is still open is a stale event from a recycled socket;
 *        restarting on it would tear down a healthy link, so it is ignored. The watch armed at
 *        doStart() stays live for the supervisor's whole lifetime.
 */
void IO::SupervisorTask::onLinkDropped()
{
  SS_ASSERT(m_child != nullptr, return);

  if (!isRunning() || m_child->isRunning())
    return;

  if (!m_driver.isNull() && m_driver->isOpen())
    return;

  auto* retry = qobject_cast<Async::RetryTask*>(m_child);
  if (retry)
    retry->setPolicy(m_recovery);

  m_child->start();
}

/**
 * @brief Tears down the drop watch; a stale handle is a no-op.
 */
void IO::SupervisorTask::releaseConnection()
{
  QObject::disconnect(m_connection);
  m_connection = QMetaObject::Connection();
}

/**
 * @brief Keeps running on success (the drop watch armed at start stays live); any other outcome
 *        is the flow's final word and finishes the supervisor with it.
 */
void IO::SupervisorTask::onChildFinished(Async::Outcome outcome, const Async::StepError& error)
{
  SS_ASSERT_LOG(m_child != nullptr);

  if (!isRunning())
    return;

  if (outcome != Async::Outcome::Success) {
    releaseConnection();
    reportFinished(outcome, error);
    return;
  }

  if (m_driver.isNull()) {
    releaseConnection();
    reportFinished(Async::Outcome::Failure, errorHere(QStringLiteral("driver is gone")));
    return;
  }
}

//--------------------------------------------------------------------------------------------------
// Flow composition
//--------------------------------------------------------------------------------------------------

/**
 * @brief Composes the open flow of a driver that opted into orchestration: one bounded open
 *        attempt, whose deadline releases the attempt instead of leaving it pending forever. The
 *        driver may name a wider ceiling than the shared default; zero means it does not.
 */
Async::Task* IO::Flows::makeOpenFlow(HAL_Driver* driver,
                                     QIODevice::OpenMode mode,
                                     Async::AsyncClock& clock)
{
  SS_ASSERT_LOG(driver != nullptr);
  SS_ASSERT_LOG(mode != QIODevice::NotOpen);

  const int declared = driver->openTimeoutMsec();
  const int msec     = declared > 0 ? declared : kOpenTimeoutMsec;
  return Async::timeout(new DriverOpenTask(driver, mode), msec, clock);
}

/**
 * @brief Wraps a flow in the shared retry policy and the drop watch, which together are what
 *        makes a link come back on its own without the owner writing a state machine. The first
 *        sequence runs on @p policy; a drop that follows runs on @p recovery.
 */
Async::Task* IO::Flows::makeSupervised(HAL_Driver* driver,
                                       Async::Task* flow,
                                       const Async::RetryPolicy& policy,
                                       const Async::RetryPolicy& recovery,
                                       Async::AsyncClock& clock)
{
  SS_ASSERT_LOG(driver != nullptr);
  SS_ASSERT(
    flow != nullptr,
    return failedStep(QStringLiteral("link-supervisor"), QStringLiteral("no flow to supervise")));

  return new SupervisorTask(driver, Async::retry(flow, policy, clock), recovery);
}

/**
 * @brief Composes one bounded socket connect attempt. The retry around it belongs to the flow's
 *        owner, so the schedule stays the shared policy's instead of a per-driver loop.
 */
Async::Task* IO::Flows::makeSocketConnect(QAbstractSocket* socket,
                                          const QString& host,
                                          quint16 port,
                                          int timeout_msec,
                                          Async::AsyncClock& clock)
{
  SS_ASSERT_LOG(port > 0);
  SS_ASSERT_LOG(socket != nullptr);
  SS_ASSERT(
    timeout_msec > 0,
    return failedStep(QStringLiteral("socket-connect"), QStringLiteral("invalid connect timeout")));

  return Async::timeout(new SocketConnectTask(socket, host, port), timeout_msec, clock);
}
