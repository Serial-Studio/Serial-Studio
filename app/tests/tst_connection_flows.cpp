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

#include <memory>
#include <QAbstractSocket>
#include <QByteArray>
#include <QCoreApplication>
#include <QHash>
#include <QHostAddress>
#include <QIODevice>
#include <QList>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QVariant>
#include <utility>

#include "Async/RetryPolicy.h"
#include "Async/TaskTree.h"
#include "IO/ConnectionFlows.h"
#include "IO/HAL_Driver.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// The shared open ceiling a driver that declares none falls back to, mirrored from
// ConnectionFlows.cpp so a change to it has to be made in two places on purpose.
static constexpr int kDefaultOpenTimeoutMsec = 15000;

//--------------------------------------------------------------------------------------------------
// Test doubles
//--------------------------------------------------------------------------------------------------

/**
 * @brief AsyncClock that never touches wall time, so an open deadline and a recovery backoff are
 *        asserted in microseconds instead of the seconds they would really take.
 */
class VirtualClock final : public Async::AsyncClock {
public:
  VirtualClock() : m_now(0), m_nextId(0) {}

  void cancel(Async::TimerId id) override { m_entries.remove(id); }

  [[nodiscard]] Async::TimerId schedule(int msec, Callback cb) override
  {
    Entry entry;
    entry.due      = m_now + msec;
    entry.callback = std::move(cb);

    const Async::TimerId id = ++m_nextId;
    m_entries.insert(id, entry);
    m_delays.append(msec);
    return id;
  }

  [[nodiscard]] int pendingCount() const { return static_cast<int>(m_entries.size()); }

  [[nodiscard]] const QList<int>& delays() const { return m_delays; }

  void advance(qint64 msec)
  {
    m_now += msec;
    for (int guard = 0; guard < 4096; ++guard) {
      const Async::TimerId next = dueEntry();
      if (next == Async::kInvalidTimerId)
        return;

      const Entry entry = m_entries.take(next);
      entry.callback();
    }
  }

private:
  struct Entry {
    qint64 due;
    Callback callback;
  };

  [[nodiscard]] Async::TimerId dueEntry() const
  {
    Async::TimerId winner = Async::kInvalidTimerId;
    qint64 best           = 0;
    for (auto it = m_entries.cbegin(); it != m_entries.cend(); ++it) {
      if (it.value().due > m_now)
        continue;

      if (winner == Async::kInvalidTimerId || it.value().due < best) {
        winner = it.key();
        best   = it.value().due;
      }
    }

    return winner;
  }

  qint64 m_now;
  Async::TimerId m_nextId;
  QList<int> m_delays;
  QHash<Async::TimerId, Entry> m_entries;
};

/**
 * @brief Driver whose open outcome the test reports by hand, counting every call the flow makes so
 *        the abort-on-cancel, deadline and re-open claims are checkable.
 */
class FakeDriver final : public IO::HAL_Driver {
  Q_OBJECT

public:
  FakeDriver()
    : m_open(false)
    , m_result(true)
    , m_synchronous(false)
    , m_aborts(0)
    , m_begins(0)
    , m_timeout(0)
    , m_mode(QIODevice::NotOpen)
  {}

  [[nodiscard]] int aborts() const { return m_aborts; }

  [[nodiscard]] int begins() const { return m_begins; }

  [[nodiscard]] QIODevice::OpenMode lastMode() const { return m_mode; }

  void dropLink() { Q_EMIT linkDropped(); }

  void setOpen(bool open) { m_open = open; }

  void setOpenTimeoutMsec(int msec) { m_timeout = msec; }

  void setSynchronous(bool synchronous, bool result)
  {
    m_result      = result;
    m_synchronous = synchronous;
  }

  void finishOpen(bool ok, const QString& reason)
  {
    m_open = ok;
    Q_EMIT openFinished(ok, reason);
  }

  void close() override { m_open = false; }

  [[nodiscard]] bool isOpen() const noexcept override { return m_open; }

  [[nodiscard]] bool isReadable() const noexcept override { return m_open; }

  [[nodiscard]] bool isWritable() const noexcept override { return m_open; }

  [[nodiscard]] bool configurationOk() const noexcept override { return true; }

  [[nodiscard]] qint64 write(const QByteArray& data) override { return data.size(); }

  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override
  {
    m_mode = mode;
    m_open = m_result;
    return m_result;
  }

  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override { return {}; }

  [[nodiscard]] int openTimeoutMsec() const noexcept override { return m_timeout; }

  void beginOpen(const QIODevice::OpenMode mode) override
  {
    ++m_begins;
    m_mode = mode;
    if (m_synchronous)
      IO::HAL_Driver::beginOpen(mode);
  }

  void abortOpen() override
  {
    ++m_aborts;
    m_open = false;
  }

public slots:

  void setDriverProperty(const QString& key, const QVariant& value) override
  {
    Q_UNUSED(key);
    Q_UNUSED(value);
  }

private:
  bool m_open;
  bool m_result;
  bool m_synchronous;
  int m_aborts;
  int m_begins;
  int m_timeout;
  QIODevice::OpenMode m_mode;
};

/**
 * @brief Socket whose dial never reaches the network, so the connect step's state machine can be
 *        driven exactly, including the ConnectedState-without-a-peer phantom it must reject.
 */
class ScriptedSocket final : public QAbstractSocket {
  Q_OBJECT

public:
  ScriptedSocket() : QAbstractSocket(QAbstractSocket::TcpSocket, nullptr), m_dials(0) {}

  [[nodiscard]] int dials() const { return m_dials; }

  void connectToHost(const QString& host,
                     quint16 port,
                     OpenMode mode                 = ReadWrite,
                     NetworkLayerProtocol protocol = AnyIPProtocol) override
  {
    Q_UNUSED(host);
    Q_UNUSED(port);
    Q_UNUSED(mode);
    Q_UNUSED(protocol);

    ++m_dials;
    setPeerPort(0);
    setSocketState(QAbstractSocket::ConnectingState);
  }

  void disconnectFromHost() override
  {
    setPeerPort(0);
    setSocketState(QAbstractSocket::UnconnectedState);
  }

  void reportPhantomConnected()
  {
    setPeerPort(0);
    setSocketState(QAbstractSocket::ConnectedState);
    Q_EMIT connected();
  }

  void reportConnected(quint16 peer)
  {
    setPeerPort(peer);
    setSocketState(QAbstractSocket::ConnectedState);
    Q_EMIT connected();
  }

  void reportError(const QString& text)
  {
    setSocketError(QAbstractSocket::ConnectionRefusedError);
    setErrorString(text);
    Q_EMIT errorOccurred(QAbstractSocket::ConnectionRefusedError);
  }

private:
  int m_dials;
};

/**
 * @brief Task whose completion the test drives by hand, recording how often it was started and
 *        cancelled so the supervisor's re-run and cancel claims are checkable.
 */
class FakeStep final : public Async::Task {
public:
  explicit FakeStep(const QString& name) : Async::Task(name), m_starts(0), m_cancels(0) {}

  [[nodiscard]] int starts() const { return m_starts; }

  [[nodiscard]] int cancels() const { return m_cancels; }

  void complete(Async::Outcome outcome, const QString& reason = QStringLiteral("boom"))
  {
    if (!isRunning())
      return;

    if (outcome == Async::Outcome::Success)
      reportFinished(outcome, Async::StepError());
    else
      reportFinished(outcome, errorHere(reason));
  }

protected:
  void doStart() override { ++m_starts; }

  void doCancel() override { ++m_cancels; }

private:
  int m_starts;
  int m_cancels;
};

/**
 * @brief Records the single terminal outcome of a task, plus how many times it was emitted.
 */
struct Watcher {
  int count;
  Async::Outcome outcome;
  Async::StepError error;

  Watcher() : count(0), outcome(Async::Outcome::Cancelled) {}

  void attach(Async::Task* task)
  {
    QObject::connect(task,
                     &Async::Task::finished,
                     task,
                     [this](Async::Outcome result, const Async::StepError& step) {
                       ++count;
                       outcome = result;
                       error   = step;
                     });
  }
};

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Covers the spec-0034 connection flows: the driver-open step, the socket-connect step, the
 *        link supervisor, and the three composers, on fakes and a virtual clock wherever the real
 *        transport would only add wall time.
 */
class ConnectionFlowsTests : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void driverOpenReportsTheDriverSuccess();
  void driverOpenObservesASynchronousOpen();
  void driverOpenCarriesTheDriverFailureReason();
  void driverOpenSubstitutesAnEmptyFailureReason();
  void driverOpenCancelAbortsTheDriverAndDropsLateOutcomes();
  void driverOpenFailsWhenTheDriverIsGone();

  void openFlowHonoursTheDriverDeclaredTimeout();
  void openFlowFallsBackToTheSharedTimeout();

  void socketConnectSucceedsAgainstALiveServer();
  void socketConnectFailsWithTheSocketErrorString();
  void socketConnectRejectsAPhantomConnectedState();
  void socketConnectReportsTheScriptedErrorString();
  void socketConnectCancelDropsTheQueuedDial();
  void socketConnectRestartDropsTheSupersededTurn();
  void socketConnectRestartIgnoresTheEarlierArming();
  void socketConnectFlowBoundsTheDialAndAbortsOnExpiry();
  void socketConnectFlowRejectsANonPositiveTimeout();

  void supervisorIgnoresADropWhileTheDriverIsOpen();
  void supervisorRerunsTheFlowOnTheRecoveryPolicy();
  void supervisorCancelStopsTheChildAndTheDropWatch();
  void supervisorForwardsANonSuccessOutcome_data();
  void supervisorForwardsANonSuccessOutcome();
  void supervisorFailsWhenTheDriverIsGoneAtSuccess();
  void supervisedFlowWithoutAChildFailsUpFront();

  void openDropRecoverCycleRunsOnTheVirtualClock();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes the recovery branch of a violated SS_ASSERT instead of aborting, which is the only
 *        way the two composer preconditions (a null flow, a non-positive timeout) can be asserted
 *        in a debug build; the flag is latched on first use, so it has to be set before any test.
 */
void ConnectionFlowsTests::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", QByteArrayLiteral("1"));
}

//--------------------------------------------------------------------------------------------------
// Driver open step
//--------------------------------------------------------------------------------------------------

/**
 * @brief The step reports the driver's own success, and asks it to open exactly once.
 */
void ConnectionFlowsTests::driverOpenReportsTheDriverSuccess()
{
  Watcher watcher;
  FakeDriver driver;
  auto task = std::make_unique<IO::DriverOpenTask>(&driver, QIODevice::ReadWrite);
  watcher.attach(task.get());

  task->start();
  QCOMPARE(driver.begins(), 1);
  QCOMPARE(driver.lastMode(), QIODevice::ReadWrite);
  QCOMPARE(watcher.count, 0);

  driver.finishOpen(true, QString());

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
  QVERIFY(watcher.error.reason.isEmpty());
  QVERIFY(!task->isRunning());
}

/**
 * @brief A driver that keeps the base synchronous beginOpen() finishes inside start(), which only
 *        holds because the step subscribes to openFinished before it asks the driver to open.
 */
void ConnectionFlowsTests::driverOpenObservesASynchronousOpen()
{
  Watcher watcher;
  FakeDriver driver;
  driver.setSynchronous(true, true);
  auto task = std::make_unique<IO::DriverOpenTask>(&driver, QIODevice::ReadOnly);
  watcher.attach(task.get());

  task->start();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
  QVERIFY(driver.isOpen());
}

/**
 * @brief A failure carries the driver's own reason, not a bare deadline.
 */
void ConnectionFlowsTests::driverOpenCarriesTheDriverFailureReason()
{
  Watcher watcher;
  FakeDriver driver;
  auto task = std::make_unique<IO::DriverOpenTask>(&driver, QIODevice::ReadWrite);
  watcher.attach(task.get());

  task->start();
  driver.finishOpen(false, QStringLiteral("port busy"));

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("driver-open"));
  QCOMPARE(watcher.error.reason, QStringLiteral("port busy"));
}

/**
 * @brief A driver that reports failure without a reason still yields a failure with provenance.
 */
void ConnectionFlowsTests::driverOpenSubstitutesAnEmptyFailureReason()
{
  Watcher watcher;
  FakeDriver driver;
  auto task = std::make_unique<IO::DriverOpenTask>(&driver, QIODevice::ReadWrite);
  watcher.attach(task.get());

  task->start();
  driver.finishOpen(false, QString());

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("driver-open"));
  QCOMPARE(watcher.error.reason, QStringLiteral("open failed"));
}

/**
 * @brief Cancel is real, not merely ignored: the driver is asked to abandon the attempt, and the
 *        outcome it reports afterwards can no longer complete the step.
 */
void ConnectionFlowsTests::driverOpenCancelAbortsTheDriverAndDropsLateOutcomes()
{
  Watcher watcher;
  FakeDriver driver;
  auto task = std::make_unique<IO::DriverOpenTask>(&driver, QIODevice::ReadWrite);
  watcher.attach(task.get());

  task->start();
  task->cancel();

  QCOMPARE(driver.aborts(), 1);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
  QCOMPARE(watcher.error.step, QStringLiteral("driver-open"));

  driver.finishOpen(true, QString());

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
}

/**
 * @brief A driver destroyed between composition and execution fails the step instead of crashing.
 */
void ConnectionFlowsTests::driverOpenFailsWhenTheDriverIsGone()
{
  Watcher watcher;
  auto* driver = new FakeDriver();
  auto task    = std::make_unique<IO::DriverOpenTask>(driver, QIODevice::ReadWrite);
  watcher.attach(task.get());

  delete driver;
  task->start();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.reason, QStringLiteral("driver is gone"));
}

//--------------------------------------------------------------------------------------------------
// Open flow composition
//--------------------------------------------------------------------------------------------------

/**
 * @brief A driver whose handshake outlasts a socket dial names its own ceiling, and the flow bounds
 *        the attempt at exactly that, releasing the driver when it expires.
 */
void ConnectionFlowsTests::openFlowHonoursTheDriverDeclaredTimeout()
{
  Watcher watcher;
  VirtualClock clock;
  FakeDriver driver;
  driver.setOpenTimeoutMsec(2500);

  std::unique_ptr<Async::Task> flow(IO::Flows::makeOpenFlow(&driver, QIODevice::ReadWrite, clock));
  const auto* bounded = qobject_cast<const Async::TimeoutTask*>(flow.get());
  QVERIFY(bounded != nullptr);
  QCOMPARE(bounded->timeoutMsec(), 2500);

  watcher.attach(flow.get());
  flow->start();
  QCOMPARE(driver.begins(), 1);

  clock.advance(2499);
  QCOMPARE(watcher.count, 0);

  clock.advance(1);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(watcher.error.step, QStringLiteral("driver-open"));
  QCOMPARE(driver.aborts(), 1);
  QCOMPARE(clock.pendingCount(), 0);
}

/**
 * @brief Zero means the driver declares nothing, and the shared 15 s ceiling applies.
 */
void ConnectionFlowsTests::openFlowFallsBackToTheSharedTimeout()
{
  Watcher watcher;
  VirtualClock clock;
  FakeDriver driver;
  QCOMPARE(driver.openTimeoutMsec(), 0);

  std::unique_ptr<Async::Task> flow(IO::Flows::makeOpenFlow(&driver, QIODevice::ReadWrite, clock));
  const auto* bounded = qobject_cast<const Async::TimeoutTask*>(flow.get());
  QVERIFY(bounded != nullptr);
  QCOMPARE(bounded->timeoutMsec(), kDefaultOpenTimeoutMsec);

  watcher.attach(flow.get());
  flow->start();

  clock.advance(kDefaultOpenTimeoutMsec - 1);
  QCOMPARE(watcher.count, 0);

  clock.advance(1);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(driver.aborts(), 1);
}

//--------------------------------------------------------------------------------------------------
// Socket connect step
//--------------------------------------------------------------------------------------------------

/**
 * @brief Against a real listener the step succeeds, and the peer port is already live at the moment
 *        it reports: ConnectedState alone never carries the outcome.
 */
void ConnectionFlowsTests::socketConnectSucceedsAgainstALiveServer()
{
  QTcpServer server;
  QVERIFY(server.listen(QHostAddress::LocalHost, 0));

  Watcher watcher;
  QTcpSocket socket;
  quint16 peer_at_finish = 0;
  auto task              = std::make_unique<IO::SocketConnectTask>(
    &socket, QStringLiteral("127.0.0.1"), server.serverPort());
  QObject::connect(task.get(),
                   &Async::Task::finished,
                   task.get(),
                   [&socket, &peer_at_finish](Async::Outcome, const Async::StepError&) {
                     peer_at_finish = socket.peerPort();
                   });
  watcher.attach(task.get());

  task->start();
  QTRY_COMPARE_WITH_TIMEOUT(watcher.count, 1, 10000);

  QCOMPARE(watcher.outcome, Async::Outcome::Success);
  QCOMPARE(peer_at_finish, server.serverPort());
  QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
}

/**
 * @brief A refused dial fails with the transport's own text, so the reason names what happened.
 */
void ConnectionFlowsTests::socketConnectFailsWithTheSocketErrorString()
{
  QTcpServer probe;
  QVERIFY(probe.listen(QHostAddress::LocalHost, 0));
  const quint16 port = probe.serverPort();
  probe.close();

  Watcher watcher;
  QTcpSocket socket;
  QString reported;
  QObject::connect(&socket,
                   &QAbstractSocket::errorOccurred,
                   &socket,
                   [&socket, &reported](QAbstractSocket::SocketError) {
                     if (reported.isEmpty())
                       reported = socket.errorString();
                   });

  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), port);
  watcher.attach(task.get());

  task->start();
  QTRY_COMPARE_WITH_TIMEOUT(watcher.count, 1, 10000);

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("socket-connect"));
  QVERIFY(!watcher.error.reason.isEmpty());
  QCOMPARE(watcher.error.reason, reported);
}

/**
 * @brief ConnectedState with no peer behind it is the reused-socket phantom, and it must never
 *        finish the step; the real connection that follows still does.
 */
void ConnectionFlowsTests::socketConnectRejectsAPhantomConnectedState()
{
  Watcher watcher;
  ScriptedSocket socket;
  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), 4242);
  watcher.attach(task.get());

  task->start();
  QCoreApplication::processEvents();
  QCOMPARE(socket.dials(), 1);

  socket.reportPhantomConnected();
  QCOMPARE(watcher.count, 0);
  QVERIFY(task->isRunning());

  socket.reportConnected(4242);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

/**
 * @brief The reason is the socket's errorString verbatim, taken at the moment the error fires.
 */
void ConnectionFlowsTests::socketConnectReportsTheScriptedErrorString()
{
  Watcher watcher;
  ScriptedSocket socket;
  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), 4242);
  watcher.attach(task.get());

  task->start();
  QCoreApplication::processEvents();
  socket.reportError(QStringLiteral("connection refused by test"));

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("socket-connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("connection refused by test"));
}

/**
 * @brief Cancelling before the deferred dial runs supersedes it: no dial is ever issued, and the
 *        queued turn that arrives afterwards finishes nothing.
 */
void ConnectionFlowsTests::socketConnectCancelDropsTheQueuedDial()
{
  Watcher watcher;
  ScriptedSocket socket;
  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), 4242);
  watcher.attach(task.get());

  task->start();
  task->cancel();
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);

  QCoreApplication::processEvents();

  QCOMPARE(socket.dials(), 0);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
}

/**
 * @brief Two turns are queued across a restart and the task is running when both arrive: only the
 *        current generation dials, so the superseded turn cannot open a second connection.
 */
void ConnectionFlowsTests::socketConnectRestartDropsTheSupersededTurn()
{
  Watcher watcher;
  ScriptedSocket socket;
  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), 4242);
  watcher.attach(task.get());

  task->start();
  task->cancel();
  task->start();
  QVERIFY(task->isRunning());

  QCoreApplication::processEvents();

  QCOMPARE(socket.dials(), 1);
  QCOMPARE(watcher.count, 1);

  socket.reportConnected(4242);

  QCOMPARE(watcher.count, 2);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

/**
 * @brief A restart never inherits the previous attempt's outcome: signals armed by the first dial
 *        arrive after the second start and must finish nothing.
 */
void ConnectionFlowsTests::socketConnectRestartIgnoresTheEarlierArming()
{
  Watcher watcher;
  ScriptedSocket socket;
  auto task = std::make_unique<IO::SocketConnectTask>(&socket, QStringLiteral("127.0.0.1"), 4242);
  watcher.attach(task.get());

  task->start();
  QCoreApplication::processEvents();
  QCOMPARE(socket.dials(), 1);

  task->cancel();
  QCOMPARE(watcher.count, 1);

  task->start();
  socket.reportConnected(4242);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(socket.dials(), 1);

  QCoreApplication::processEvents();
  QCOMPARE(socket.dials(), 2);

  socket.reportConnected(4242);

  QCOMPARE(watcher.count, 2);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

/**
 * @brief The composer bounds the dial, and the expiry abandons it rather than leaving a half-open
 *        socket behind.
 */
void ConnectionFlowsTests::socketConnectFlowBoundsTheDialAndAbortsOnExpiry()
{
  Watcher watcher;
  VirtualClock clock;
  ScriptedSocket socket;

  std::unique_ptr<Async::Task> flow(
    IO::Flows::makeSocketConnect(&socket, QStringLiteral("127.0.0.1"), 4242, 1200, clock));
  const auto* bounded = qobject_cast<const Async::TimeoutTask*>(flow.get());
  QVERIFY(bounded != nullptr);
  QCOMPARE(bounded->timeoutMsec(), 1200);

  watcher.attach(flow.get());
  flow->start();
  QCoreApplication::processEvents();
  QCOMPARE(socket.dials(), 1);

  clock.advance(1200);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(watcher.error.step, QStringLiteral("socket-connect"));
  QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
}

/**
 * @brief A non-positive deadline is a violated precondition: the composer hands back a step that
 *        reports it, and the socket is never touched.
 */
void ConnectionFlowsTests::socketConnectFlowRejectsANonPositiveTimeout()
{
  Watcher watcher;
  VirtualClock clock;
  ScriptedSocket socket;

  std::unique_ptr<Async::Task> flow(
    IO::Flows::makeSocketConnect(&socket, QStringLiteral("127.0.0.1"), 4242, 0, clock));
  QVERIFY(flow != nullptr);
  QVERIFY(qobject_cast<const Async::TimeoutTask*>(flow.get()) == nullptr);

  watcher.attach(flow.get());
  flow->start();
  QCoreApplication::processEvents();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("socket-connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("invalid connect timeout"));
  QCOMPARE(socket.dials(), 0);
  QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
  QCOMPARE(clock.pendingCount(), 0);
}

//--------------------------------------------------------------------------------------------------
// Link supervisor
//--------------------------------------------------------------------------------------------------

/**
 * @brief A drop reported while the driver is still open is a stale event from a recycled socket:
 *        acting on it would tear down a healthy link, so the flow is not re-run.
 */
void ConnectionFlowsTests::supervisorIgnoresADropWhileTheDriverIsOpen()
{
  Watcher watcher;
  FakeDriver driver;
  auto* child = new FakeStep(QStringLiteral("open"));
  auto supervisor =
    std::make_unique<IO::SupervisorTask>(&driver, child, Async::RetryPolicy::autoReconnect());
  watcher.attach(supervisor.get());

  supervisor->start();
  QCOMPARE(child->starts(), 1);

  driver.setOpen(true);
  child->complete(Async::Outcome::Success);
  QCOMPARE(watcher.count, 0);

  driver.dropLink();

  QCOMPARE(child->starts(), 1);
  QCOMPARE(watcher.count, 0);
  QVERIFY(supervisor->isRunning());
}

/**
 * @brief A genuine drop re-runs the flow silently, on the recovery schedule rather than on the one
 *        a user-initiated connect waited behind.
 */
void ConnectionFlowsTests::supervisorRerunsTheFlowOnTheRecoveryPolicy()
{
  Watcher watcher;
  VirtualClock clock;
  FakeDriver driver;
  const Async::RetryPolicy initial(4, 100, 400, 2.0);
  const Async::RetryPolicy recovery(6, 250, 1000, 2.0);

  auto* step      = new FakeStep(QStringLiteral("open"));
  auto* child     = new Async::RetryTask(step, initial, clock);
  auto supervisor = std::make_unique<IO::SupervisorTask>(&driver, child, recovery);
  watcher.attach(supervisor.get());

  supervisor->start();
  driver.setOpen(true);
  step->complete(Async::Outcome::Success);
  QCOMPARE(child->policy().initialDelayMsec(), 100);
  QCOMPARE(supervisor->attempt(), 0);

  driver.setOpen(false);
  driver.dropLink();

  QCOMPARE(step->starts(), 2);
  QCOMPARE(child->policy().maxAttempts(), 6);
  QCOMPARE(child->policy().initialDelayMsec(), 250);
  QCOMPARE(supervisor->attempt(), 1);
  QCOMPARE(watcher.count, 0);
}

/**
 * @brief Cancelling stops the flow in flight and releases the drop watch, so a later drop cannot
 *        resurrect a link the owner asked to stop.
 */
void ConnectionFlowsTests::supervisorCancelStopsTheChildAndTheDropWatch()
{
  Watcher watcher;
  FakeDriver driver;
  auto* child = new FakeStep(QStringLiteral("open"));
  auto supervisor =
    std::make_unique<IO::SupervisorTask>(&driver, child, Async::RetryPolicy::autoReconnect());
  watcher.attach(supervisor.get());

  supervisor->start();
  supervisor->cancel();

  QCOMPARE(child->cancels(), 1);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);

  driver.dropLink();

  QCOMPARE(child->starts(), 1);
  QCOMPARE(watcher.count, 1);
}

void ConnectionFlowsTests::supervisorForwardsANonSuccessOutcome_data()
{
  QTest::addColumn<Async::Outcome>("outcome");

  QTest::newRow("failure") << Async::Outcome::Failure;
  QTest::newRow("timed out") << Async::Outcome::TimedOut;
  QTest::newRow("cancelled") << Async::Outcome::Cancelled;
}

/**
 * @brief Anything but success is the flow's final word: the supervisor forwards it with the child's
 *        own provenance instead of restating it.
 */
void ConnectionFlowsTests::supervisorForwardsANonSuccessOutcome()
{
  QFETCH(Async::Outcome, outcome);

  Watcher watcher;
  FakeDriver driver;
  auto* child = new FakeStep(QStringLiteral("open"));
  auto supervisor =
    std::make_unique<IO::SupervisorTask>(&driver, child, Async::RetryPolicy::autoReconnect());
  watcher.attach(supervisor.get());

  supervisor->start();
  child->complete(outcome, QStringLiteral("no route to host"));

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, outcome);
  QCOMPARE(watcher.error.step, QStringLiteral("open"));
  QCOMPARE(watcher.error.reason, QStringLiteral("no route to host"));
  QVERIFY(!supervisor->isRunning());
  QCOMPARE(supervisor->attempt(), 0);
}

/**
 * @brief A driver destroyed while the flow was in flight fails the supervisor instead of leaving it
 *        watching a dangling link.
 */
void ConnectionFlowsTests::supervisorFailsWhenTheDriverIsGoneAtSuccess()
{
  Watcher watcher;
  auto* driver = new FakeDriver();
  auto* child  = new FakeStep(QStringLiteral("open"));
  auto supervisor =
    std::make_unique<IO::SupervisorTask>(driver, child, Async::RetryPolicy::autoReconnect());
  watcher.attach(supervisor.get());

  supervisor->start();
  delete driver;
  child->complete(Async::Outcome::Success);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("link-supervisor"));
  QCOMPARE(watcher.error.reason, QStringLiteral("driver is gone"));
}

/**
 * @brief A composer whose precondition was violated hands back a step that reports, never a null
 *        pointer its caller would run blind.
 */
void ConnectionFlowsTests::supervisedFlowWithoutAChildFailsUpFront()
{
  Watcher watcher;
  VirtualClock clock;
  FakeDriver driver;

  std::unique_ptr<Async::Task> flow(IO::Flows::makeSupervised(&driver,
                                                              nullptr,
                                                              Async::RetryPolicy::initialConnect(),
                                                              Async::RetryPolicy::autoReconnect(),
                                                              clock));
  QVERIFY(flow != nullptr);
  QVERIFY(qobject_cast<const IO::SupervisorTask*>(flow.get()) == nullptr);

  watcher.attach(flow.get());
  flow->start();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("link-supervisor"));
  QCOMPARE(watcher.error.reason, QStringLiteral("no flow to supervise"));
  QCOMPARE(clock.pendingCount(), 0);
}

//--------------------------------------------------------------------------------------------------
// End to end
//--------------------------------------------------------------------------------------------------

/**
 * @brief The whole composed link: a first attempt fails, the retry brings it up on the backoff, a
 *        genuine drop re-opens it, and nothing between attempts is ever reported to the owner.
 */
void ConnectionFlowsTests::openDropRecoverCycleRunsOnTheVirtualClock()
{
  Watcher watcher;
  VirtualClock clock;
  FakeDriver driver;
  const Async::RetryPolicy initial(3, 100, 400, 2.0);
  const Async::RetryPolicy recovery(4, 250, 1000, 2.0);

  auto* flow = IO::Flows::makeOpenFlow(&driver, QIODevice::ReadWrite, clock);
  std::unique_ptr<Async::Task> link(
    IO::Flows::makeSupervised(&driver, flow, initial, recovery, clock));
  auto* supervisor = qobject_cast<IO::SupervisorTask*>(link.get());
  QVERIFY(supervisor != nullptr);
  watcher.attach(link.get());

  link->start();
  QCOMPARE(driver.begins(), 1);

  driver.finishOpen(false, QStringLiteral("port busy"));
  QCOMPARE(watcher.count, 0);
  QCOMPARE(driver.begins(), 1);

  clock.advance(100);
  QCOMPARE(driver.begins(), 2);
  QCOMPARE(supervisor->attempt(), 2);

  driver.finishOpen(true, QString());
  QCOMPARE(watcher.count, 0);
  QCOMPARE(supervisor->attempt(), 0);
  QCOMPARE(clock.pendingCount(), 0);

  driver.setOpen(false);
  driver.dropLink();
  QCOMPARE(driver.begins(), 3);

  driver.finishOpen(true, QString());

  QCOMPARE(watcher.count, 0);
  QVERIFY(link->isRunning());
  QCOMPARE(driver.aborts(), 0);
}

QTEST_GUILESS_MAIN(ConnectionFlowsTests)

#include "tst_connection_flows.moc"
