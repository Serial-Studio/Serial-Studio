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

#include "Core/Async/RetryPolicy.h"
#include "Core/Async/TaskTree.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. This suite covers the free-function
// combinator vocabulary (Async::parallel/sequential/timeout/retry/invoke/awaitSignal/onDone), the
// RetryPolicy geometric schedule and step cap, RetryTask::setPolicy, a pre-start dead-sender
// SignalTask, and a deep nested tree -- gaps tst_async_engine.cpp leaves in TaskTree.cpp and
// RetryPolicy.cpp.

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
 * @brief Builds sequential(parallel(a,b), timeout(c), retry(d)) via the free-function vocabulary,
 *        so the three nested-tree tests share one tree shape instead of re-deriving it each time.
 */
static Async::Task* buildNestedTree(
  Async::AsyncClock& clock, FakeStep*& a, FakeStep*& b, FakeStep*& c, FakeStep*& d)
{
  a = new FakeStep(QStringLiteral("a"), nullptr);
  b = new FakeStep(QStringLiteral("b"), nullptr);
  c = new FakeStep(QStringLiteral("c"), nullptr);
  d = new FakeStep(QStringLiteral("d"), nullptr);

  auto* par = Async::parallel(QStringLiteral("par"));
  par->addChild(a)->addChild(b);

  auto* root = Async::sequential(QStringLiteral("root"));
  root->addChild(par)
    ->addChild(Async::timeout(c, 500, clock))
    ->addChild(Async::retry(d, Async::RetryPolicy(3, 100, 400, 2.0), clock));

  return root;
}

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Covers the Async:: free-function combinator vocabulary, the RetryPolicy geometric
 *        schedule and step cap, RetryTask::setPolicy, a pre-start dead-sender SignalTask, and a
 *        nested sequential(parallel, timeout, retry) tree, with fake steps and a virtual clock
 *        only.
 */
class AsyncCombinatorTests : public QObject {
  Q_OBJECT

private slots:
  void sequentialFactoryOrdersChildrenAndReportsFailureStep();
  void parallelFactoryCancelsSiblingsOnFirstFailure();
  void invokeFactoryMapsCallableFailureReason();
  void timeoutFactoryReportsTimedOutAndCancelsChild();
  void retryFactoryReachesSuccessAfterPolicyBackoffs();
  void awaitSignalFactoryReportsFailureReason();
  void onDoneReturnsSameTaskAndReceivesOutcome();

  void retryPolicyAutoReconnectGeometricSchedule_data();
  void retryPolicyAutoReconnectGeometricSchedule();
  void retryPolicyBackoffStepCapPlateausPastTwentyFourSteps();

  void retryTaskSetPolicySwapsScheduleBeforeNextRun();

  void signalTaskFailsWhenSenderDestroyedBeforeStart();

  void nestedTreeAggregatesSuccessThroughEveryCombinator();
  void nestedTreeCancelDuringParallelStagePropagatesToActiveChildren();
  void nestedTreeCancelDuringRetryStagePropagatesToActiveChild();
};

//--------------------------------------------------------------------------------------------------
// Combinator factories
//--------------------------------------------------------------------------------------------------

void AsyncCombinatorTests::sequentialFactoryOrdersChildrenAndReportsFailureStep()
{
  Watcher watcher;
  auto* group = Async::sequential(QStringLiteral("open"));
  auto* one   = new FakeStep(QStringLiteral("resolve"), nullptr);
  auto* two   = new FakeStep(QStringLiteral("connect"), nullptr);
  group->addChild(one)->addChild(two);
  std::unique_ptr<Async::Task> task(group);
  watcher.attach(task.get());

  task->start();
  QCOMPARE(one->starts(), 1);
  QCOMPARE(two->starts(), 0);

  one->complete(Async::Outcome::Success);
  QCOMPARE(two->starts(), 1);

  two->complete(Async::Outcome::Failure, QStringLiteral("connection refused"));

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("connection refused"));
}

void AsyncCombinatorTests::parallelFactoryCancelsSiblingsOnFirstFailure()
{
  Watcher watcher;
  auto* group = Async::parallel(QStringLiteral("probe"));
  auto* a     = new FakeStep(QStringLiteral("a"), nullptr);
  auto* b     = new FakeStep(QStringLiteral("b"), nullptr);
  auto* c     = new FakeStep(QStringLiteral("c"), nullptr);
  group->addChild(a)->addChild(b)->addChild(c);
  std::unique_ptr<Async::Task> task(group);
  watcher.attach(task.get());

  task->start();
  b->complete(Async::Outcome::Failure, QStringLiteral("bad handshake"));

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("b"));
  QCOMPARE(a->cancels(), 1);
  QCOMPARE(c->cancels(), 1);
}

void AsyncCombinatorTests::invokeFactoryMapsCallableFailureReason()
{
  Watcher watcher;
  std::unique_ptr<Async::Task> task(Async::invoke(QStringLiteral("apply"), [](QString& reason) {
    reason = QStringLiteral("port busy");
    return false;
  }));
  watcher.attach(task.get());

  task->start();

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.reason, QStringLiteral("port busy"));
}

void AsyncCombinatorTests::timeoutFactoryReportsTimedOutAndCancelsChild()
{
  VirtualClock clock;
  Watcher watcher;
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  std::unique_ptr<Async::Task> task(Async::timeout(child, 500, clock));
  watcher.attach(task.get());

  task->start();
  clock.advance(500);

  QCOMPARE(watcher.outcome, Async::Outcome::TimedOut);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(child->cancels(), 1);
}

void AsyncCombinatorTests::retryFactoryReachesSuccessAfterPolicyBackoffs()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy policy(3, 100, 400, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  std::unique_ptr<Async::Task> task(Async::retry(child, policy, clock));
  watcher.attach(task.get());

  task->start();
  child->complete(Async::Outcome::Failure);
  clock.advance(policy.maxDelayMsec());
  child->complete(Async::Outcome::Failure);
  clock.advance(policy.maxDelayMsec());
  child->complete(Async::Outcome::Success);

  QCOMPARE(child->starts(), 3);
  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncCombinatorTests::awaitSignalFactoryReportsFailureReason()
{
  FakeSender sender;
  Watcher watcher;
  auto* signal = Async::awaitSignal(QStringLiteral("connect"));
  signal->onFailure(&sender, &FakeSender::failed, QStringLiteral("socket error"));
  std::unique_ptr<Async::Task> task(signal);
  watcher.attach(task.get());

  task->start();
  Q_EMIT sender.failed();

  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("socket error"));
}

void AsyncCombinatorTests::onDoneReturnsSameTaskAndReceivesOutcome()
{
  int calls               = 0;
  Async::Outcome captured = Async::Outcome::Cancelled;
  Async::StepError capturedError;

  auto* task = Async::invoke(QStringLiteral("apply"), [](QString&) { return true; });
  Async::Task* returned =
    Async::onDone(task, [&](Async::Outcome outcome, const Async::StepError& error) {
      ++calls;
      captured      = outcome;
      capturedError = error;
    });

  QCOMPARE(returned, static_cast<Async::Task*>(task));

  std::unique_ptr<Async::Task> guard(task);
  guard->start();

  QCOMPARE(calls, 1);
  QCOMPARE(captured, Async::Outcome::Success);
  QVERIFY(capturedError.step.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// RetryPolicy schedule and step cap
//--------------------------------------------------------------------------------------------------

void AsyncCombinatorTests::retryPolicyAutoReconnectGeometricSchedule_data()
{
  QTest::addColumn<int>("attempt");
  QTest::addColumn<int>("delay");

  QTest::newRow("first") << 1 << 500;
  QTest::newRow("second") << 2 << 1000;
  QTest::newRow("third") << 3 << 2000;
  QTest::newRow("fourth") << 4 << 4000;
  QTest::newRow("fifth, capped") << 5 << 5000;
  QTest::newRow("sixth, still capped") << 6 << 5000;
}

void AsyncCombinatorTests::retryPolicyAutoReconnectGeometricSchedule()
{
  QFETCH(int, attempt);
  QFETCH(int, delay);

  const Async::RetryPolicy policy = Async::RetryPolicy::autoReconnect();
  QCOMPARE(policy.delayForAttempt(attempt), delay);
}

void AsyncCombinatorTests::retryPolicyBackoffStepCapPlateausPastTwentyFourSteps()
{
  const Async::RetryPolicy policy(100, 1, 100000000, 2.0);

  const int atCap      = policy.delayForAttempt(25);
  const int wayPastCap = policy.delayForAttempt(100000);

  QCOMPARE(atCap, 16777216);
  QCOMPARE(wayPastCap, atCap);
  QVERIFY(atCap > 0);
  QVERIFY(atCap < policy.maxDelayMsec());
}

//--------------------------------------------------------------------------------------------------
// RetryTask::setPolicy
//--------------------------------------------------------------------------------------------------

void AsyncCombinatorTests::retryTaskSetPolicySwapsScheduleBeforeNextRun()
{
  VirtualClock clock;
  Watcher watcher;
  const Async::RetryPolicy original(5, 300, 1200, 2.0);
  const Async::RetryPolicy swapped(5, 50, 200, 2.0);
  auto* child = new FakeStep(QStringLiteral("connect"), nullptr);
  auto task   = std::make_unique<Async::RetryTask>(child, original, clock);
  watcher.attach(task.get());

  task->setPolicy(swapped);
  QCOMPARE(task->policy().initialDelayMsec(), 50);

  task->start();
  child->complete(Async::Outcome::Failure);

  QCOMPARE(clock.delays(), QList<int>({50}));
  QCOMPARE(watcher.count, 0);
}

//--------------------------------------------------------------------------------------------------
// SignalTask pre-start dead sender
//--------------------------------------------------------------------------------------------------

void AsyncCombinatorTests::signalTaskFailsWhenSenderDestroyedBeforeStart()
{
  Watcher watcher;
  auto sender = std::make_unique<FakeSender>();
  auto task   = std::make_unique<Async::SignalTask>(QStringLiteral("connect"));
  task->onSuccess(sender.get(), &FakeSender::connected);
  watcher.attach(task.get());

  sender.reset();
  task->start();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Failure);
  QCOMPARE(watcher.error.step, QStringLiteral("connect"));
  QCOMPARE(watcher.error.reason, QStringLiteral("signal source is gone"));
}

//--------------------------------------------------------------------------------------------------
// Nested combinator composition
//--------------------------------------------------------------------------------------------------

void AsyncCombinatorTests::nestedTreeAggregatesSuccessThroughEveryCombinator()
{
  VirtualClock clock;
  Watcher watcher;
  FakeStep* a = nullptr;
  FakeStep* b = nullptr;
  FakeStep* c = nullptr;
  FakeStep* d = nullptr;
  std::unique_ptr<Async::Task> root(buildNestedTree(clock, a, b, c, d));
  watcher.attach(root.get());

  root->start();
  QCOMPARE(a->starts(), 1);
  QCOMPARE(b->starts(), 1);
  QCOMPARE(c->starts(), 0);
  QCOMPARE(d->starts(), 0);

  a->complete(Async::Outcome::Success);
  b->complete(Async::Outcome::Success);
  QCOMPARE(c->starts(), 1);
  QCOMPARE(clock.pendingCount(), 1);

  c->complete(Async::Outcome::Success);
  QCOMPARE(clock.pendingCount(), 0);
  QCOMPARE(d->starts(), 1);

  d->complete(Async::Outcome::Success);

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Success);
}

void AsyncCombinatorTests::nestedTreeCancelDuringParallelStagePropagatesToActiveChildren()
{
  VirtualClock clock;
  Watcher watcher;
  FakeStep* a = nullptr;
  FakeStep* b = nullptr;
  FakeStep* c = nullptr;
  FakeStep* d = nullptr;
  std::unique_ptr<Async::Task> root(buildNestedTree(clock, a, b, c, d));
  watcher.attach(root.get());

  root->start();
  root->cancel();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
  QCOMPARE(a->cancels(), 1);
  QCOMPARE(b->cancels(), 1);
  QCOMPARE(c->starts(), 0);
  QCOMPARE(d->starts(), 0);
}

void AsyncCombinatorTests::nestedTreeCancelDuringRetryStagePropagatesToActiveChild()
{
  VirtualClock clock;
  Watcher watcher;
  FakeStep* a = nullptr;
  FakeStep* b = nullptr;
  FakeStep* c = nullptr;
  FakeStep* d = nullptr;
  std::unique_ptr<Async::Task> root(buildNestedTree(clock, a, b, c, d));
  watcher.attach(root.get());

  root->start();
  a->complete(Async::Outcome::Success);
  b->complete(Async::Outcome::Success);
  c->complete(Async::Outcome::Success);
  QCOMPARE(d->starts(), 1);

  root->cancel();

  QCOMPARE(watcher.count, 1);
  QCOMPARE(watcher.outcome, Async::Outcome::Cancelled);
  QCOMPARE(d->cancels(), 1);
  QCOMPARE(a->cancels(), 0);
  QCOMPARE(b->cancels(), 0);
  QCOMPARE(c->cancels(), 0);
}

QTEST_GUILESS_MAIN(AsyncCombinatorTests)

#include "tst_async_combinators.moc"
