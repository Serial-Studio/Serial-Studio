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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <memory>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTest>
#include <utility>

#include "Async/RetryPolicy.h"
#include "Async/TaskTree.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Test doubles
//--------------------------------------------------------------------------------------------------

/**
 * @brief AsyncClock that never touches wall time, so a multi-attempt backoff schedule is
 *        asserted in microseconds instead of the seconds it would really take.
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
 * @brief Task whose completion the test drives by hand, recording how often it was started and
 *        cancelled so sibling-cancellation and no-further-step claims are checkable.
 */
class FakeStep final : public Async::Task {
public:
  FakeStep(const QString& name, QStringList* log)
    : Async::Task(name), m_starts(0), m_cancels(0), m_log(log)
  {}

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
  void doStart() override
  {
    ++m_starts;
    if (m_log != nullptr)
      m_log->append(name() + QStringLiteral("/start"));
  }

  void doCancel() override
  {
    ++m_cancels;
    if (m_log != nullptr)
      m_log->append(name() + QStringLiteral("/cancel"));
  }

private:
  int m_starts;
  int m_cancels;
  QStringList* m_log;
};

/**
 * @brief Emitter standing in for a socket or a broker client.
 */
class FakeSender : public QObject {
  Q_OBJECT

signals:
  void connected();
  void failed();
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

/**
 * @brief Builds one task of each composite shape around the same child, so the emit-once claim
 *        can be asserted per shape from a data table.
 */
static Async::Task* makeShape(int shape, FakeStep* child, Async::AsyncClock& clock)
{
  if (shape == 0) {
    auto* group = new Async::SequentialGroup(QStringLiteral("seq"));
    group->addChild(child);
    return group;
  }

  if (shape == 1) {
    auto* group = new Async::ParallelGroup(QStringLiteral("par"));
    group->addChild(child);
    return group;
  }

  if (shape == 2)
    return new Async::TimeoutTask(child, 500, clock);

  return new Async::RetryTask(child, Async::RetryPolicy(), clock);
}

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Covers AC1-AC3 of spec 0034: sequencing, parallel completion, timeout, retry backoff,
 *        cancellation, and error provenance, with fake steps and a virtual clock only.
 */
class AsyncEngineTests : public QObject {
  Q_OBJECT

private slots:
  void sequentialRunsChildrenInOrder();
  void sequentialEmptyGroupSucceeds();
  void sequentialStopsAtFirstFailure();
  void sequentialPropagatesTimedOutOutcome();

  void parallelStartsEveryChildImmediately();
  void parallelFinishesOnLastSuccess();
  void parallelFirstFailureCancelsSiblings();

  void timeoutExpiryCancelsChildAndReportsTimedOut();
  void timeoutReleasesTimerOnChildSuccess();
  void timeoutCancelPropagatesToChild();

  void retryPolicySchedule_data();
  void retryPolicySchedule();
  void retryPolicyNamedPoliciesAreBounded();
  void retryFollowsPolicyBackoffSchedule();
  void retryStopsAtAttemptCap();
  void retryResetsAttemptCountOnSuccess();
  void retryCancelDuringBackoffNeverReattempts();
  void retryEmitsNothingBetweenAttempts();

  void cancelMidStepRunsNoFurtherStep();
  void finishedEmittedExactlyOnce_data();
  void finishedEmittedExactlyOnce();

  void invokeTaskMapsCallableResult();
  void invokeTaskSubstitutesMissingReason();

  void signalTaskFinishesOnSuccessSignal();
  void signalTaskFinishesOnFailureSignalWithReason();
  void signalTaskIgnoresSignalsFiredBeforeStart();
  void signalTaskRunsAbortHandlerOnCancel();
  void signalTaskFailsWhenSenderDies();

  void runnerSupersedesPreviousRoot();
  void runnerCancelsRootOnDestruction();
};

//--------------------------------------------------------------------------------------------------
// Sequential group
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::sequentialRunsChildrenInOrder()
{
  QStringList log;
  Watcher watcher;

  auto group  = std::make_unique<Async::SequentialGroup>(QStringLiteral("open"));
  auto* one   = new FakeStep(QStringLiteral("s1"), &log);
  auto* two   = new FakeStep(QStringLiteral("s2"), &log);
  auto* three = new FakeStep(QStringLiteral("s3"), &log);
  group->addChild(one)->addChild(two)->addChild(three);
  watcher.attach(group.get());

  group->start();
  QCOMPARE(log, QStringList{QStringLiteral("s1/start")});
  QCOMPARE(two->starts(), 0);

  one->complete(Async::Outcome::Success);
  QCOMPARE(two->starts(), 1);
  QCOMPARE(three->starts(), 0);

  two->complete(Async::Outcome::Success);
  three->complete(Async::Outcome::Success);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
  QVERIFY(watcher.error.step.isEmpty());
}

void AsyncEngineTests::sequentialEmptyGroupSucceeds()
{
  Watcher watcher;
  auto group = std::make_unique<Async::SequentialGroup>(QStringLiteral("empty"));
  watcher.attach(group.get());

  group->start();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncEngineTests::sequentialStopsAtFirstFailure()
{
  QStringList log;
  Watcher watcher;

  auto group  = std::make_unique<Async::SequentialGroup>(QStringLiteral("open"));
  auto* one   = new FakeStep(QStringLiteral("resolve"), &log);
  auto* two   = new FakeStep(QStringLiteral("connect"), &log);
  auto* three = new FakeStep(QStringLiteral("subscribe"), &log);
  group->addChild(one)->addChild(two)->addChild(three);
  watcher.attach(group.get());

  group->start();
  one->complete(Async::Outcome::Success);
  two->complete(Async::Outcome::Failure, QStringLiteral("connection refused"));

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("connection refused"));
  QVERIFY(!watcher.error.reason.isEmpty());
  QCOMPARE(three->starts(), 0);
}

void AsyncEngineTests::sequentialPropagatesTimedOutOutcome()
{
  Watcher watcher;
  auto group = std::make_unique<Async::SequentialGroup>(QStringLiteral("open"));
  auto* one  = new FakeStep(QStringLiteral("connect"), nullptr);
  auto* two  = new FakeStep(QStringLiteral("subscribe"), nullptr);
  group->addChild(one)->addChild(two);
  watcher.attach(group.get());

  group->start();
  one->complete(Async::Outcome::TimedOut, QStringLiteral("no answer"));

  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(two->starts(), 0);
}

//--------------------------------------------------------------------------------------------------
// Parallel group
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::parallelStartsEveryChildImmediately()
{
  QStringList log;
  auto group = std::make_unique<Async::ParallelGroup>(QStringLiteral("probe"));
  auto* one  = new FakeStep(QStringLiteral("a"), &log);
  auto* two  = new FakeStep(QStringLiteral("b"), &log);
  group->addChild(one)->addChild(two);

  group->start();

  QCOMPARE(one->starts(), 1);
  QCOMPARE(two->starts(), 1);
}

void AsyncEngineTests::parallelFinishesOnLastSuccess()
{
  Watcher watcher;
  auto group = std::make_unique<Async::ParallelGroup>(QStringLiteral("probe"));
  auto* one  = new FakeStep(QStringLiteral("a"), nullptr);
  auto* two  = new FakeStep(QStringLiteral("b"), nullptr);
  group->addChild(one)->addChild(two);
  watcher.attach(group.get());

  group->start();
  one->complete(Async::Outcome::Success);
  QCOMPARE(watcher.count, 0);

  two->complete(Async::Outcome::Success);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncEngineTests::parallelFirstFailureCancelsSiblings()
{
  Watcher watcher;
  auto group  = std::make_unique<Async::ParallelGroup>(QStringLiteral("probe"));
  auto* one   = new FakeStep(QStringLiteral("a"), nullptr);
  auto* two   = new FakeStep(QStringLiteral("b"), nullptr);
  auto* three = new FakeStep(QStringLiteral("c"), nullptr);
  group->addChild(one)->addChild(two)->addChild(three);
  watcher.attach(group.get());

  group->start();
  two->complete(Async::Outcome::Failure, QStringLiteral("bad handshake"));

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("b"));
  QCOMPARE(one->cancels(), 1);
  QCOMPARE(three->cancels(), 1);
}

//--------------------------------------------------------------------------------------------------
// Timeout
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::timeoutExpiryCancelsChildAndReportsTimedOut()
{
  VirtualClock clock;
  Watcher watcher;
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::TimeoutTask>(child, 800, clock);
  watcher.attach(task.get());

  task->start();
  QCOMPARE(clock.pendingCount(), 1);

  clock.advance(799);
  QCOMPARE(watcher.count, 0);

  clock.advance(1);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QVERIFY(!watcher.error.reason.isEmpty());
  QCOMPARE(child->cancels(), 1);
  QCOMPARE(clock.pendingCount(), 0);
}

void AsyncEngineTests::timeoutReleasesTimerOnChildSuccess()
{
  VirtualClock clock;
  Watcher watcher;
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::TimeoutTask>(child, 500, clock);
  watcher.attach(task.get());

  task->start();
  child->complete(Async::Outcome::Success);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
  QCOMPARE(clock.pendingCount(), 0);

  clock.advance(10000);
  QCOMPARE(watcher.count, 1);
}

void AsyncEngineTests::timeoutCancelPropagatesToChild()
{
  VirtualClock clock;
  Watcher watcher;
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::TimeoutTask>(child, 500, clock);
  watcher.attach(task.get());

  task->start();
  task->cancel();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
  QCOMPARE(child->cancels(), 1);
  QCOMPARE(clock.pendingCount(), 0);
}

//--------------------------------------------------------------------------------------------------
// Retry policy and RetryTask
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::retryPolicySchedule_data()
{
  QTest::addColumn<int>("attempt");
  QTest::addColumn<int>("delay");

  QTest::newRow("first") << 1 << 300;
  QTest::newRow("second") << 2 << 300;
  QTest::newRow("third") << 3 << 300;
  QTest::newRow("fourth") << 4 << 300;
  QTest::newRow("past the attempt cap") << 9 << 300;
}

void AsyncEngineTests::retryPolicySchedule()
{
  QFETCH(int, attempt);
  QFETCH(int, delay);

  const Async::RetryPolicy policy = Async::RetryPolicy::initialConnect();
  QCOMPARE(policy.delayForAttempt(attempt), delay);
}

void AsyncEngineTests::retryPolicyNamedPoliciesAreBounded()
{
  const Async::RetryPolicy connect   = Async::RetryPolicy::initialConnect();
  const Async::RetryPolicy reconnect = Async::RetryPolicy::autoReconnect();

  QVERIFY(connect.maxAttempts() >= 1);
  QVERIFY(reconnect.maxAttempts() >= 1);
  QVERIFY(connect.delayForAttempt(64) <= connect.maxDelayMsec());
  QVERIFY(reconnect.delayForAttempt(64) <= reconnect.maxDelayMsec());
  QVERIFY(!connect.shouldRetry(connect.maxAttempts()));
  QVERIFY(!reconnect.shouldRetry(reconnect.maxAttempts()));
  QVERIFY(connect.shouldRetry(1));

  const Async::RetryPolicy none;
  QVERIFY(!none.shouldRetry(1));
}

void AsyncEngineTests::retryFollowsPolicyBackoffSchedule()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(4, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, policy, clock);
  watcher.attach(task.get());

  task->start();
  for (int i = 0; i < 3; ++i) {
    child->complete(Async::Outcome::Failure);
    clock.advance(policy.maxDelayMsec());
  }

  QCOMPARE(clock.delays(), QList<int>({100, 200, 400}));
  QCOMPARE(child->starts(), 4);
  QCOMPARE(watcher.count, 0);
}

void AsyncEngineTests::retryStopsAtAttemptCap()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(3, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, policy, clock);
  watcher.attach(task.get());

  task->start();
  for (int i = 0; i < 3; ++i) {
    child->complete(Async::Outcome::Failure, QStringLiteral("refused"));
    clock.advance(policy.maxDelayMsec());
  }

  QCOMPARE(child->starts(), 3);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("refused"));
  QCOMPARE(clock.pendingCount(), 0);
}

void AsyncEngineTests::retryResetsAttemptCountOnSuccess()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(5, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, policy, clock);
  watcher.attach(task.get());

  task->start();
  child->complete(Async::Outcome::Failure);
  clock.advance(policy.maxDelayMsec());
  QCOMPARE(task->attempt(), 2);

  child->complete(Async::Outcome::Success);

  QCOMPARE(task->attempt(), 0);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncEngineTests::retryCancelDuringBackoffNeverReattempts()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(5, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, policy, clock);
  watcher.attach(task.get());

  task->start();
  child->complete(Async::Outcome::Failure);
  QCOMPARE(clock.pendingCount(), 1);

  task->cancel();
  QCOMPARE(clock.pendingCount(), 0);

  clock.advance(100000);

  QCOMPARE(child->starts(), 1);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
}

void AsyncEngineTests::retryEmitsNothingBetweenAttempts()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(6, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, policy, clock);
  watcher.attach(task.get());

  task->start();
  for (int i = 0; i < 4; ++i) {
    child->complete(Async::Outcome::Failure);
    clock.advance(policy.maxDelayMsec());
    QCOMPARE(watcher.count, 0);
  }

  child->complete(Async::Outcome::Success);
  QCOMPARE(watcher.count, 1);
}

//--------------------------------------------------------------------------------------------------
// Cancellation and emit-once discipline
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::cancelMidStepRunsNoFurtherStep()
{
  QStringList log;
  Watcher watcher;
  auto group = std::make_unique<Async::SequentialGroup>(QStringLiteral("open"));
  auto* one  = new FakeStep(QStringLiteral("s1"), &log);
  auto* two  = new FakeStep(QStringLiteral("s2"), &log);
  group->addChild(one)->addChild(two);
  watcher.attach(group.get());

  group->start();
  group->cancel();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
  QCOMPARE(one->cancels(), 1);
  QCOMPARE(two->starts(), 0);
  QCOMPARE(two->cancels(), 0);
}

void AsyncEngineTests::finishedEmittedExactlyOnce_data()
{
  QTest::addColumn<int>("shape");
  QTest::addColumn<bool>("cancelled");

  QTest::newRow("sequential success") << 0 << false;
  QTest::newRow("sequential cancelled") << 0 << true;
  QTest::newRow("parallel success") << 1 << false;
  QTest::newRow("parallel cancelled") << 1 << true;
  QTest::newRow("timeout success") << 2 << false;
  QTest::newRow("timeout cancelled") << 2 << true;
  QTest::newRow("retry success") << 3 << false;
  QTest::newRow("retry cancelled") << 3 << true;
}

void AsyncEngineTests::finishedEmittedExactlyOnce()
{
  QFETCH(int, shape);
  QFETCH(bool, cancelled);

  VirtualClock clock;
  Watcher watcher;
  auto* child = new FakeStep(QStringLiteral("step"), nullptr);
  std::unique_ptr<Async::Task> task(makeShape(shape, child, clock));

  watcher.attach(task.get());
  task->start();

  if (cancelled) {
    task->cancel();
    task->cancel();
  } else {
    child->complete(Async::Outcome::Success);
    child->complete(Async::Outcome::Success);
  }

  QCOMPARE(watcher.count, 1);
  QVERIFY(task->isFinished());
  QVERIFY(!task->isRunning());
}

//--------------------------------------------------------------------------------------------------
// Invoke
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::invokeTaskMapsCallableResult()
{
  Watcher ok;
  Watcher bad;

  auto good =
    std::make_unique<Async::InvokeTask>(QStringLiteral("apply"), [](QString&) { return true; });
  ok.attach(good.get());
  good->start();

  auto fail = std::make_unique<Async::InvokeTask>(QStringLiteral("apply"), [](QString& reason) {
    reason = QStringLiteral("port busy");
    return false;
  });
  bad.attach(fail.get());
  fail->start();

  QCOMPARE(ok.count, 1);
  QCOMPARE(ok.outcome, Async::Outcome::Success);
  QCOMPARE(bad.count, 1);
  QCOMPARE(bad.outcome, Async::Outcome::Failure);
  QCOMPARE(bad.error.step, QStringLiteral("apply"));
  QCOMPARE(bad.error.reason, QStringLiteral("port busy"));
}

void AsyncEngineTests::invokeTaskSubstitutesMissingReason()
{
  Watcher watcher;
  auto task =
    std::make_unique<Async::InvokeTask>(QStringLiteral("apply"), [](QString&) { return false; });
  watcher.attach(task.get());

  task->start();

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QVERIFY(!watcher.error.reason.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Signal wait
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::signalTaskFinishesOnSuccessSignal()
{
  FakeSender sender;
  Watcher watcher;
  auto task = std::make_unique<Async::SignalTask>(QStringLiteral("connect"));
  task->onSuccess(&sender, &FakeSender::connected);
  task->onFailure(&sender, &FakeSender::failed, QStringLiteral("socket error"));
  watcher.attach(task.get());

  task->start();
  Q_EMIT sender.connected();
  Q_EMIT sender.failed();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncEngineTests::signalTaskFinishesOnFailureSignalWithReason()
{
  FakeSender sender;
  Watcher watcher;
  auto task = std::make_unique<Async::SignalTask>(QStringLiteral("connect"));
  task->onSuccess(&sender, &FakeSender::connected);
  task->onFailure(&sender, &FakeSender::failed, QStringLiteral("socket error"));
  watcher.attach(task.get());

  task->start();
  Q_EMIT sender.failed();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("socket error"));
}

void AsyncEngineTests::signalTaskIgnoresSignalsFiredBeforeStart()
{
  FakeSender sender;
  Watcher watcher;
  auto task = std::make_unique<Async::SignalTask>(QStringLiteral("connect"));
  task->onSuccess(&sender, &FakeSender::connected);
  watcher.attach(task.get());

  Q_EMIT sender.connected();
  QCOMPARE(watcher.count, 0);

  task->start();
  Q_EMIT sender.connected();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncEngineTests::signalTaskRunsAbortHandlerOnCancel()
{
  FakeSender sender;
  Watcher watcher;
  int aborts = 0;
  auto task  = std::make_unique<Async::SignalTask>(QStringLiteral("resolve"));
  task->onSuccess(&sender, &FakeSender::connected);
  task->setAbortHandler([&aborts]() { ++aborts; });
  watcher.attach(task.get());

  task->start();
  task->cancel();
  Q_EMIT sender.connected();

  QCOMPARE(aborts, 1);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
}

void AsyncEngineTests::signalTaskFailsWhenSenderDies()
{
  Watcher watcher;
  auto* sender = new FakeSender();
  auto task    = std::make_unique<Async::SignalTask>(QStringLiteral("connect"));
  task->onSuccess(sender, &FakeSender::connected);
  watcher.attach(task.get());

  task->start();
  delete sender;

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QVERIFY(!watcher.error.reason.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Runner
//--------------------------------------------------------------------------------------------------

void AsyncEngineTests::runnerSupersedesPreviousRoot()
{
  int emissions = 0;
  Async::TaskRunner runner;
  auto* first  = new FakeStep(QStringLiteral("first"), nullptr);
  auto* second = new FakeStep(QStringLiteral("second"), nullptr);

  QObject::connect(&runner,
                   &Async::TaskRunner::finished,
                   &runner,
                   [&emissions](Async::Outcome, const Async::StepError&) { ++emissions; });

  runner.run(first);
  QVERIFY(runner.isRunning());

  runner.run(second);

  QCOMPARE(emissions, 0);
  QCOMPARE(runner.root(), second);
  QVERIFY(runner.isRunning());

  second->complete(Async::Outcome::Success);
  QCOMPARE(emissions, 1);
  QVERIFY(!runner.isRunning());
}

void AsyncEngineTests::runnerCancelsRootOnDestruction()
{
  int cancels = 0;
  {
    Async::TaskRunner runner;
    auto* step = new FakeStep(QStringLiteral("connect"), nullptr);
    QObject::connect(step,
                     &Async::Task::finished,
                     step,
                     [&cancels](Async::Outcome outcome, const Async::StepError&) {
                       if (outcome == Async::Outcome::Cancelled)
                         ++cancels;
                     });
    runner.run(step);
  }

  QCOMPARE(cancels, 1);
}

QTEST_GUILESS_MAIN(AsyncEngineTests)

#include "tst_async_engine.moc"
