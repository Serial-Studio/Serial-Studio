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

#pragma once

#include <functional>
#include <memory>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <utility>

#include "Core/Async/AsyncClock.h"
#include "Core/Async/RetryPolicy.h"
#include "Core/SSAssert.h"

class QThread;

namespace Async {
/**
 * @brief Terminal result of one task run. Every non-Success value stops the enclosing group.
 */
enum class Outcome {
  Success,
  Failure,
  Cancelled,
  TimedOut
};

/**
 * @brief Provenance of a failure: which step failed, and why, in a form the connection
 *        diagnostics and problem-center specs can consume without parsing prose.
 */
struct StepError {
  QString step;
  QString reason;
};

/**
 * @brief Base of every node in an async task tree. A task is thread-affine, never blocks the
 *        thread it runs on, and emits finished() exactly once per run.
 */
class Task : public QObject {
  Q_OBJECT

signals:
  void finished(Async::Outcome outcome, const Async::StepError& error);

public:
  explicit Task(QString name, QObject* parent = nullptr);
  Task(Task&&)                 = delete;
  Task(const Task&)            = delete;
  Task& operator=(Task&&)      = delete;
  Task& operator=(const Task&) = delete;
  ~Task() override             = default;

  [[nodiscard]] bool isRunning() const noexcept;
  [[nodiscard]] bool isFinished() const noexcept;
  [[nodiscard]] const QString& name() const noexcept;

public slots:
  void start();
  void cancel();

protected:
  virtual void doStart() = 0;
  virtual void doCancel();

  [[nodiscard]] StepError errorHere(const QString& reason) const;
  void reportFinished(Outcome outcome, const StepError& error);

private:
  bool m_running;
  bool m_finished;
  QString m_name;
};

/**
 * @brief Ownership, cancellation, and child bookkeeping shared by the composite tasks.
 */
class Group : public Task {
  Q_OBJECT

public:
  explicit Group(QString name, QObject* parent = nullptr);
  ~Group() override = default;

  Group* addChild(Task* child);
  [[nodiscard]] int childCount() const noexcept;
  [[nodiscard]] bool isCancelling() const noexcept;
  [[nodiscard]] Task* childAt(int index) const;

protected:
  void doCancel() override;
  void cancelChildren();
  virtual void onChildFinished(Task* child, Outcome outcome, const StepError& error) = 0;

private:
  bool m_cancelling;
  QList<Task*> m_children;
};

/**
 * @brief Runs its children one after another; the first non-Success outcome finishes the group
 *        with that outcome and that child's error, and no later child is started.
 */
class SequentialGroup final : public Group {
  Q_OBJECT

public:
  explicit SequentialGroup(QString name, QObject* parent = nullptr);
  ~SequentialGroup() override = default;

protected:
  void doStart() override;
  void onChildFinished(Task* child, Outcome outcome, const StepError& error) override;

private:
  int m_index;
};

/**
 * @brief Starts every child at once; the first non-Success outcome cancels the siblings and
 *        finishes the group.
 */
class ParallelGroup final : public Group {
  Q_OBJECT

public:
  explicit ParallelGroup(QString name, QObject* parent = nullptr);
  ~ParallelGroup() override = default;

protected:
  void doStart() override;
  void onChildFinished(Task* child, Outcome outcome, const StepError& error) override;

private:
  int m_pending;
};

/**
 * @brief Bounds one child's runtime. On expiry the child is cancelled and the wrapper finishes
 *        TimedOut, which the enclosing group treats exactly like a failure.
 */
class TimeoutTask final : public Task {
  Q_OBJECT

public:
  TimeoutTask(Task* child, int msec, AsyncClock& clock, QObject* parent = nullptr);
  ~TimeoutTask() override;

  [[nodiscard]] int timeoutMsec() const noexcept;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void stopTimer();
  void onExpired();
  void onChildFinished(Outcome outcome, const StepError& error);

private:
  bool m_expired;
  int m_msec;
  TimerId m_timer;
  Task* m_child;
  AsyncClock* m_clock;
};

/**
 * @brief Re-runs one child under a RetryPolicy. Attempts are internal: the wrapper emits nothing
 *        until the child finally succeeds, the cap is reached, or the flow is cancelled, so a
 *        recovering link cannot amplify connection-state churn.
 */
class RetryTask final : public Task {
  Q_OBJECT

public:
  RetryTask(Task* child, const RetryPolicy& policy, AsyncClock& clock, QObject* parent = nullptr);
  ~RetryTask() override;

  void resetAttempts() noexcept;
  void setPolicy(const RetryPolicy& policy);
  [[nodiscard]] int attempt() const noexcept;
  [[nodiscard]] const RetryPolicy& policy() const noexcept;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void stopTimer();
  void beginAttempt();
  void onBackoffElapsed();
  void onChildFinished(Outcome outcome, const StepError& error);

private:
  int m_attempt;
  TimerId m_timer;
  Task* m_child;
  AsyncClock* m_clock;
  RetryPolicy m_policy;
};

/**
 * @brief Waits for the first of one success signal or N failure signals to fire. Senders are
 *        QPointer-guarded and every connection is torn down on every exit path, which is what
 *        makes a superseded continuation impossible rather than merely unlikely.
 */
class SignalTask final : public Task {
  Q_OBJECT

public:
  using AbortHandler      = std::function<void()>;
  using ConnectionFactory = std::function<QMetaObject::Connection()>;

  explicit SignalTask(QString name, QObject* parent = nullptr);
  ~SignalTask() override;

  void setAbortHandler(AbortHandler handler);

  template<typename Sender, typename Signal>
  void onSuccess(Sender* sender, Signal signal);

  template<typename Sender, typename Signal>
  void onFailure(Sender* sender, Signal signal, const QString& reason);

protected:
  void doStart() override;
  void doCancel() override;

private:
  void succeed();
  void releaseConnections();
  void fail(const QString& reason);
  void trackSender(QObject* sender);

private:
  AbortHandler m_abort;
  QList<ConnectionFactory> m_factories;
  QList<QMetaObject::Connection> m_connections;
  QList<QPointer<QObject>> m_senders;
};

/**
 * @brief Runs a synchronous callable and maps its bool result onto an Outcome. This is the shape
 *        an unmigrated driver participates in a tree with, so its semantics stay today's.
 */
class InvokeTask final : public Task {
  Q_OBJECT

public:
  using Callable = std::function<bool(QString&)>;

  InvokeTask(QString name, Callable callable, QObject* parent = nullptr);
  ~InvokeTask() override = default;

protected:
  void doStart() override;

private:
  Callable m_callable;
};

/**
 * @brief The only handle a caller holds. Owns the root, cancels any previous root before running
 *        a new one, and cancels silently on destruction so teardown cannot re-enter its owner.
 */
class TaskRunner final : public QObject {
  Q_OBJECT

signals:
  void finished(Async::Outcome outcome, const Async::StepError& error);

public:
  explicit TaskRunner(QObject* parent = nullptr);
  TaskRunner(AsyncClock* clock, QObject* parent);
  TaskRunner(TaskRunner&&)                 = delete;
  TaskRunner(const TaskRunner&)            = delete;
  TaskRunner& operator=(TaskRunner&&)      = delete;
  TaskRunner& operator=(const TaskRunner&) = delete;
  ~TaskRunner() override;

  [[nodiscard]] Task* root() const noexcept;
  [[nodiscard]] AsyncClock& clock() const noexcept;
  [[nodiscard]] bool isRunning() const noexcept;

public slots:
  void cancel();
  void run(Task* root);

private:
  void stopRoot();
  void onRootFinished(Outcome outcome, const StepError& error);

private:
  QThread* m_thread;
  AsyncClock* m_clock;
  std::unique_ptr<SystemClock> m_ownedClock;
  std::unique_ptr<Task> m_root;
  QMetaObject::Connection m_rootConnection;
};

[[nodiscard]] ParallelGroup* parallel(QString name);
[[nodiscard]] SignalTask* awaitSignal(QString name);
[[nodiscard]] SequentialGroup* sequential(QString name);
[[nodiscard]] InvokeTask* invoke(QString name, InvokeTask::Callable callable);
[[nodiscard]] TimeoutTask* timeout(Task* child, int msec, AsyncClock& clock);
[[nodiscard]] RetryTask* retry(Task* child, const RetryPolicy& policy, AsyncClock& clock);
Task* onDone(Task* task, std::function<void(Outcome, const StepError&)> handler);

/**
 * @brief Registers a success signal, deferring the connection to start() so a signal that fires
 *        before this step is reached can never complete it.
 */
template<typename Sender, typename Signal>
void SignalTask::onSuccess(Sender* sender, Signal signal)
{
  SS_ASSERT(sender != nullptr, return);
  SS_ASSERT(!isRunning(), return);

  trackSender(sender);
  const QPointer<Sender> guard(sender);
  m_factories.append([this, guard, signal]() -> QMetaObject::Connection {
    if (guard.isNull())
      return QMetaObject::Connection();

    return QObject::connect(guard.data(), signal, this, [this]() { succeed(); });
  });
}

/**
 * @brief Registers a failure signal and the reason it reports, deferred to start() for the same
 *        staleness reason as onSuccess().
 */
template<typename Sender, typename Signal>
void SignalTask::onFailure(Sender* sender, Signal signal, const QString& reason)
{
  SS_ASSERT(sender != nullptr, return);
  SS_ASSERT(!isRunning(), return);

  trackSender(sender);
  const QPointer<Sender> guard(sender);
  m_factories.append([this, guard, signal, reason]() -> QMetaObject::Connection {
    if (guard.isNull())
      return QMetaObject::Connection();

    return QObject::connect(guard.data(), signal, this, [this, reason]() { fail(reason); });
  });
}
}  // namespace Async

Q_DECLARE_METATYPE(Async::Outcome)
Q_DECLARE_METATYPE(Async::StepError)
