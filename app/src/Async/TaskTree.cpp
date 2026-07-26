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

#include "Async/TaskTree.h"

#include <QThread>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Task base
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a task identified by the step name its failures report.
 */
Async::Task::Task(QString name, QObject* parent)
  : QObject(parent), m_running(false), m_finished(false), m_name(std::move(name))
{
  SS_ASSERT(!m_name.isEmpty(), m_name = QStringLiteral("task"));
  SS_ASSERT_LOG(!m_running);
}

/**
 * @brief Returns whether an attempt is in flight.
 */
bool Async::Task::isRunning() const noexcept
{
  return m_running;
}

/**
 * @brief Returns whether the last run reached a terminal outcome.
 */
bool Async::Task::isFinished() const noexcept
{
  return m_finished;
}

/**
 * @brief Returns the machine-readable step identity carried by this task's failures.
 */
const QString& Async::Task::name() const noexcept
{
  return m_name;
}

/**
 * @brief Begins a run. A task is restartable so RetryTask can re-attempt its child, but it may
 *        never be started while a previous attempt is still in flight.
 */
void Async::Task::start()
{
  SS_ASSERT(!m_running, return);
  SS_ASSERT(thread() == QThread::currentThread(), return);

  m_running  = true;
  m_finished = false;
  doStart();
}

/**
 * @brief Stops the run and reports Cancelled unless the subclass already reported an outcome
 *        while unwinding, which is how a cancelled child's error keeps its provenance.
 */
void Async::Task::cancel()
{
  SS_ASSERT(thread() == QThread::currentThread(), return);

  if (!m_running)
    return;

  doCancel();

  if (m_running)
    reportFinished(Outcome::Cancelled, errorHere(QStringLiteral("cancelled")));
}

/**
 * @brief Default cancellation hook for tasks that hold nothing to release.
 */
void Async::Task::doCancel()
{
  SS_ASSERT_LOG(m_running);
}

/**
 * @brief Builds a StepError attributed to this task.
 */
Async::StepError Async::Task::errorHere(const QString& reason) const
{
  SS_ASSERT_LOG(!m_name.isEmpty());
  SS_ASSERT_LOG(!reason.isEmpty());

  StepError error;
  error.step   = m_name;
  error.reason = reason;
  return error;
}

/**
 * @brief Emits the single terminal outcome of the current run. The two assertions are the
 *        emit-once discipline the whole engine relies on.
 */
void Async::Task::reportFinished(Outcome outcome, const StepError& error)
{
  SS_ASSERT(m_running, return);
  SS_ASSERT(!m_finished, return);

  m_running  = false;
  m_finished = true;
  Q_EMIT finished(outcome, error);
}

//--------------------------------------------------------------------------------------------------
// Group base
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty composite task.
 */
Async::Group::Group(QString name, QObject* parent)
  : Task(std::move(name), parent), m_cancelling(false)
{
  SS_ASSERT_LOG(!m_cancelling);
  SS_ASSERT_LOG(m_children.isEmpty());
}

/**
 * @brief Adopts a child and returns the group so trees can be composed in one expression.
 */
Async::Group* Async::Group::addChild(Task* child)
{
  SS_ASSERT(child != nullptr, return this);
  SS_ASSERT_LOG(!child->isRunning());

  child->setParent(this);
  m_children.append(child);
  connect(child, &Task::finished, this, [this, child](Outcome outcome, const StepError& error) {
    onChildFinished(child, outcome, error);
  });

  return this;
}

/**
 * @brief Returns how many children the group holds.
 */
int Async::Group::childCount() const noexcept
{
  return static_cast<int>(m_children.size());
}

/**
 * @brief Returns whether the group is currently unwinding its children, during which a child's
 *        terminal outcome must not be mistaken for the group's own.
 */
bool Async::Group::isCancelling() const noexcept
{
  return m_cancelling;
}

/**
 * @brief Returns the child at the given index.
 */
Async::Task* Async::Group::childAt(int index) const
{
  SS_ASSERT(index >= 0, return nullptr);
  SS_ASSERT(index < m_children.size(), return nullptr);

  return m_children.at(index);
}

/**
 * @brief Cancels every started child.
 */
void Async::Group::doCancel()
{
  SS_ASSERT_LOG(isRunning());

  cancelChildren();
}

/**
 * @brief Cancels every running child exactly once, fencing the sweep so re-entrant child
 *        outcomes cannot finish the group from inside it.
 */
void Async::Group::cancelChildren()
{
  SS_ASSERT(!m_cancelling, return);

  m_cancelling = true;
  for (int i = 0; i < m_children.size(); ++i) {
    Task* child = m_children.at(i);
    if (child->isRunning())
      child->cancel();
  }

  m_cancelling = false;
}

//--------------------------------------------------------------------------------------------------
// Sequential group
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a sequential group.
 */
Async::SequentialGroup::SequentialGroup(QString name, QObject* parent)
  : Group(std::move(name), parent), m_index(0)
{
  SS_ASSERT_LOG(m_index == 0);
  SS_ASSERT_LOG(childCount() == 0);
}

/**
 * @brief Starts the first child, or succeeds immediately when the group is empty.
 */
void Async::SequentialGroup::doStart()
{
  SS_ASSERT_LOG(isRunning());

  m_index = 0;
  if (childCount() == 0) {
    reportFinished(Outcome::Success, StepError());
    return;
  }

  childAt(m_index)->start();
}

/**
 * @brief Advances to the next child on Success; any other outcome finishes the group with that
 *        child's error and starts nothing further.
 */
void Async::SequentialGroup::onChildFinished(Task* child, Outcome outcome, const StepError& error)
{
  SS_ASSERT_LOG(child != nullptr);

  if (!isRunning() || isCancelling())
    return;

  SS_ASSERT_LOG(m_index < childCount());
  if (outcome != Outcome::Success) {
    reportFinished(outcome, error);
    return;
  }

  ++m_index;
  if (m_index >= childCount()) {
    reportFinished(Outcome::Success, StepError());
    return;
  }

  childAt(m_index)->start();
}

//--------------------------------------------------------------------------------------------------
// Parallel group
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a parallel group.
 */
Async::ParallelGroup::ParallelGroup(QString name, QObject* parent)
  : Group(std::move(name), parent), m_pending(0)
{
  SS_ASSERT_LOG(m_pending == 0);
  SS_ASSERT_LOG(childCount() == 0);
}

/**
 * @brief Starts every child, stopping early if a synchronous child already finished the group.
 */
void Async::ParallelGroup::doStart()
{
  SS_ASSERT_LOG(isRunning());

  m_pending = childCount();
  if (m_pending == 0) {
    reportFinished(Outcome::Success, StepError());
    return;
  }

  for (int i = 0; i < childCount() && isRunning(); ++i)
    childAt(i)->start();
}

/**
 * @brief Finishes on the first non-Success child after cancelling its siblings, or on the last
 *        Success.
 */
void Async::ParallelGroup::onChildFinished(Task* child, Outcome outcome, const StepError& error)
{
  SS_ASSERT_LOG(child != nullptr);

  if (!isRunning() || isCancelling())
    return;

  SS_ASSERT_LOG(m_pending > 0);
  --m_pending;
  if (outcome != Outcome::Success) {
    cancelChildren();
    reportFinished(outcome, error);
    return;
  }

  if (m_pending <= 0)
    reportFinished(Outcome::Success, StepError());
}

//--------------------------------------------------------------------------------------------------
// Timeout task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a deadline around one child, adopting it.
 */
Async::TimeoutTask::TimeoutTask(Task* child, int msec, AsyncClock& clock, QObject* parent)
  : Task(child != nullptr ? child->name() : QStringLiteral("timeout"), parent)
  , m_expired(false)
  , m_msec(msec)
  , m_timer(kInvalidTimerId)
  , m_child(child)
  , m_clock(&clock)
{
  SS_ASSERT(child != nullptr, return);
  SS_ASSERT(msec >= 0, m_msec = 0);

  m_child->setParent(this);
  connect(m_child, &Task::finished, this, &TimeoutTask::onChildFinished);
}

/**
 * @brief Destroys the deadline and releases any pending timer.
 */
Async::TimeoutTask::~TimeoutTask()
{
  stopTimer();
}

/**
 * @brief Returns the deadline in milliseconds.
 */
int Async::TimeoutTask::timeoutMsec() const noexcept
{
  return m_msec;
}

/**
 * @brief Arms the deadline before starting the child, so a child that completes synchronously
 *        still finds a timer to release.
 */
void Async::TimeoutTask::doStart()
{
  SS_ASSERT(m_child != nullptr, {
    reportFinished(Outcome::Failure, errorHere(QStringLiteral("no child")));
    return;
  });
  SS_ASSERT(m_timer == kInvalidTimerId, stopTimer());

  m_expired = false;
  m_timer   = m_clock->schedule(m_msec, [this]() { onExpired(); });
  m_child->start();
}

/**
 * @brief Releases the deadline and cancels the child.
 */
void Async::TimeoutTask::doCancel()
{
  SS_ASSERT(m_child != nullptr, {
    stopTimer();
    return;
  });
  SS_ASSERT_LOG(isRunning());

  stopTimer();
  if (m_child->isRunning())
    m_child->cancel();
}

/**
 * @brief Releases a pending deadline; a stale handle is a no-op.
 */
void Async::TimeoutTask::stopTimer()
{
  SS_ASSERT(m_clock != nullptr, {
    m_timer = kInvalidTimerId;
    return;
  });

  if (m_timer == kInvalidTimerId)
    return;

  m_clock->cancel(m_timer);
  m_timer = kInvalidTimerId;
}

/**
 * @brief Cancels the child and reports TimedOut, which the enclosing group reads as a failure.
 */
void Async::TimeoutTask::onExpired()
{
  SS_ASSERT(m_child != nullptr, {
    m_timer = kInvalidTimerId;
    return;
  });

  m_timer = kInvalidTimerId;
  if (!isRunning())
    return;

  m_expired = true;
  if (m_child->isRunning())
    m_child->cancel();

  const QString reason = QStringLiteral("timed out after %1 ms").arg(m_msec);
  reportFinished(Outcome::TimedOut, errorHere(reason));
}

/**
 * @brief Passes the child's outcome through, unless the deadline already claimed the run.
 */
void Async::TimeoutTask::onChildFinished(Outcome outcome, const StepError& error)
{
  SS_ASSERT_LOG(m_child != nullptr);

  if (!isRunning() || m_expired)
    return;

  stopTimer();
  reportFinished(outcome, error);
}

//--------------------------------------------------------------------------------------------------
// Retry task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a retry wrapper around one child, adopting it.
 */
Async::RetryTask::RetryTask(Task* child,
                            const RetryPolicy& policy,
                            AsyncClock& clock,
                            QObject* parent)
  : Task(child != nullptr ? child->name() : QStringLiteral("retry"), parent)
  , m_attempt(0)
  , m_timer(kInvalidTimerId)
  , m_child(child)
  , m_clock(&clock)
  , m_policy(policy)
{
  SS_ASSERT(child != nullptr, return);
  SS_ASSERT_LOG(policy.maxAttempts() >= 1);

  m_child->setParent(this);
  connect(m_child, &Task::finished, this, &RetryTask::onChildFinished);
}

/**
 * @brief Destroys the wrapper and releases any pending backoff.
 */
Async::RetryTask::~RetryTask()
{
  stopTimer();
}

/**
 * @brief Clears the attempt counter, which is what a success or a user-initiated connect does.
 */
void Async::RetryTask::resetAttempts() noexcept
{
  m_attempt = 0;
}

/**
 * @brief Swaps the schedule the next run will follow. A link that was already up retries on the
 *        recovery policy rather than on the one a user-initiated connect waited behind, so the
 *        two cases keep their own budgets without a second wrapper.
 */
void Async::RetryTask::setPolicy(const RetryPolicy& policy)
{
  SS_ASSERT(!isRunning(), return);
  SS_ASSERT(policy.maxAttempts() >= 1, return);

  m_policy = policy;
}

/**
 * @brief Returns the number of attempts made in the current run, zero when idle or connected.
 */
int Async::RetryTask::attempt() const noexcept
{
  return m_attempt;
}

/**
 * @brief Returns the policy driving this wrapper.
 */
const Async::RetryPolicy& Async::RetryTask::policy() const noexcept
{
  return m_policy;
}

/**
 * @brief Starts the first attempt of a fresh run.
 */
void Async::RetryTask::doStart()
{
  SS_ASSERT(m_child != nullptr, {
    reportFinished(Outcome::Failure, errorHere(QStringLiteral("no child")));
    return;
  });
  SS_ASSERT(m_timer == kInvalidTimerId, stopTimer());

  m_attempt = 0;
  beginAttempt();
}

/**
 * @brief Stops the run whether it is mid-attempt or mid-backoff.
 */
void Async::RetryTask::doCancel()
{
  SS_ASSERT(m_child != nullptr, {
    stopTimer();
    return;
  });
  SS_ASSERT_LOG(isRunning());

  stopTimer();
  if (m_child->isRunning())
    m_child->cancel();
}

/**
 * @brief Releases a pending backoff; a stale handle is a no-op.
 */
void Async::RetryTask::stopTimer()
{
  SS_ASSERT(m_clock != nullptr, {
    m_timer = kInvalidTimerId;
    return;
  });

  if (m_timer == kInvalidTimerId)
    return;

  m_clock->cancel(m_timer);
  m_timer = kInvalidTimerId;
}

/**
 * @brief Counts and starts one attempt.
 */
void Async::RetryTask::beginAttempt()
{
  SS_ASSERT(m_child != nullptr, return);
  SS_ASSERT(!m_child->isRunning(), return);

  ++m_attempt;
  m_child->start();
}

/**
 * @brief Starts the next attempt once the backoff elapsed, unless the run was cancelled while
 *        the wait was pending.
 */
void Async::RetryTask::onBackoffElapsed()
{
  SS_ASSERT(m_child != nullptr, {
    m_timer = kInvalidTimerId;
    return;
  });

  m_timer = kInvalidTimerId;
  if (!isRunning())
    return;

  beginAttempt();
}

/**
 * @brief Maps an attempt's outcome onto success, another attempt after the policy's delay, or
 *        the final failure. No signal is emitted between attempts.
 */
void Async::RetryTask::onChildFinished(Outcome outcome, const StepError& error)
{
  SS_ASSERT_LOG(m_child != nullptr);

  if (!isRunning())
    return;

  SS_ASSERT(m_attempt >= 1, m_attempt = 1);
  if (outcome == Outcome::Success) {
    resetAttempts();
    reportFinished(Outcome::Success, StepError());
    return;
  }

  if (outcome == Outcome::Cancelled || !m_policy.shouldRetry(m_attempt)) {
    reportFinished(outcome, error);
    return;
  }

  const int delay = m_policy.delayForAttempt(m_attempt);
  m_timer         = m_clock->schedule(delay, [this]() { onBackoffElapsed(); });
}

//--------------------------------------------------------------------------------------------------
// Signal task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a signal-wait step with no sources yet registered.
 */
Async::SignalTask::SignalTask(QString name, QObject* parent) : Task(std::move(name), parent)
{
  SS_ASSERT_LOG(m_factories.isEmpty());
  SS_ASSERT_LOG(m_connections.isEmpty());
}

/**
 * @brief Destroys the step, releasing every connection it still holds.
 */
Async::SignalTask::~SignalTask()
{
  releaseConnections();
}

/**
 * @brief Installs the callable that aborts the underlying operation on cancel, which is how a
 *        real host lookup or socket connect is stopped rather than merely ignored.
 */
void Async::SignalTask::setAbortHandler(AbortHandler handler)
{
  SS_ASSERT(!isRunning(), return);
  SS_ASSERT(static_cast<bool>(handler), return);

  m_abort = std::move(handler);
}

/**
 * @brief Establishes every registered connection at start time and fails immediately when a
 *        source died between composition and execution.
 */
void Async::SignalTask::doStart()
{
  SS_ASSERT(!m_factories.isEmpty(), {
    reportFinished(Outcome::Failure, errorHere(QStringLiteral("no source")));
    return;
  });
  SS_ASSERT(m_connections.isEmpty(), releaseConnections());

  for (int i = 0; i < m_factories.size(); ++i) {
    const QMetaObject::Connection link = m_factories.at(i)();
    if (link)
      m_connections.append(link);
  }

  if (m_connections.size() != m_factories.size()) {
    releaseConnections();
    reportFinished(Outcome::Failure, errorHere(QStringLiteral("signal source is gone")));
    return;
  }

  for (int i = 0; i < m_senders.size(); ++i) {
    QObject* sender = m_senders.at(i).data();
    if (sender == nullptr)
      continue;

    m_connections.append(connect(sender, &QObject::destroyed, this, [this]() {
      fail(QStringLiteral("signal source destroyed"));
    }));
  }
}

/**
 * @brief Releases every connection and aborts the underlying operation.
 */
void Async::SignalTask::doCancel()
{
  SS_ASSERT_LOG(isRunning());

  releaseConnections();
  if (m_abort)
    m_abort();
}

/**
 * @brief Finishes the step successfully.
 */
void Async::SignalTask::succeed()
{
  SS_ASSERT_LOG(!m_factories.isEmpty());

  if (!isRunning())
    return;

  releaseConnections();
  reportFinished(Outcome::Success, StepError());
}

/**
 * @brief Tears down every connection this step made, on every exit path.
 */
void Async::SignalTask::releaseConnections()
{
  for (int i = 0; i < m_connections.size(); ++i)
    QObject::disconnect(m_connections.at(i));

  m_connections.clear();
  SS_ASSERT_LOG(m_connections.isEmpty());
}

/**
 * @brief Finishes the step with the reason registered for the failure signal that fired.
 */
void Async::SignalTask::fail(const QString& reason)
{
  SS_ASSERT_LOG(!reason.isEmpty());

  if (!isRunning())
    return;

  releaseConnections();
  reportFinished(Outcome::Failure, errorHere(reason));
}

/**
 * @brief Records a sender so its destruction can fail the step instead of hanging it.
 */
void Async::SignalTask::trackSender(QObject* sender)
{
  SS_ASSERT(sender != nullptr, return);
  SS_ASSERT(!isRunning(), return);

  const QPointer<QObject> guard(sender);
  if (!m_senders.contains(guard))
    m_senders.append(guard);
}

//--------------------------------------------------------------------------------------------------
// Invoke task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a synchronous step from a callable returning success plus a reason.
 */
Async::InvokeTask::InvokeTask(QString name, Callable callable, QObject* parent)
  : Task(std::move(name), parent), m_callable(std::move(callable))
{
  SS_ASSERT_LOG(static_cast<bool>(m_callable));
  SS_ASSERT_LOG(!isRunning());
}

/**
 * @brief Runs the callable and maps its result, substituting a reason when the caller left one
 *        out so no failure reaches the owner without provenance.
 */
void Async::InvokeTask::doStart()
{
  SS_ASSERT_LOG(isRunning());
  SS_ASSERT(static_cast<bool>(m_callable), {
    reportFinished(Outcome::Failure, errorHere(QStringLiteral("no action")));
    return;
  });

  QString reason;
  const bool ok = m_callable(reason);
  if (ok) {
    reportFinished(Outcome::Success, StepError());
    return;
  }

  if (reason.isEmpty())
    reason = QStringLiteral("step failed");

  reportFinished(Outcome::Failure, errorHere(reason));
}

//--------------------------------------------------------------------------------------------------
// Task runner
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a runner owning a clock bound to the calling thread.
 */
Async::TaskRunner::TaskRunner(QObject* parent) : TaskRunner(nullptr, parent) {}

/**
 * @brief Constructs a runner, adopting an injected clock when one is supplied. The affinity
 *        assertion makes a wrong-thread construction fail loudly in debug rather than race.
 */
Async::TaskRunner::TaskRunner(AsyncClock* clock, QObject* parent)
  : QObject(parent)
  , m_thread(QThread::currentThread())
  , m_clock(clock)
  , m_ownedClock(clock != nullptr ? nullptr : std::make_unique<SystemClock>())
{
  SS_ASSERT_LOG(m_thread != nullptr);
  SS_ASSERT_LOG(m_thread == thread());

  if (m_clock == nullptr)
    m_clock = m_ownedClock.get();
}

/**
 * @brief Cancels the running tree without emitting, so teardown cannot re-enter the owner.
 */
Async::TaskRunner::~TaskRunner()
{
  stopRoot();
}

/**
 * @brief Returns the running root, or nullptr when the runner is idle.
 */
Async::Task* Async::TaskRunner::root() const noexcept
{
  return m_root.get();
}

/**
 * @brief Returns the clock every timed task in this runner's trees must use.
 */
Async::AsyncClock& Async::TaskRunner::clock() const noexcept
{
  SS_ASSERT_LOG(m_clock != nullptr);

  return *m_clock;
}

/**
 * @brief Returns whether a tree is currently in flight.
 */
bool Async::TaskRunner::isRunning() const noexcept
{
  return m_root != nullptr && m_root->isRunning();
}

/**
 * @brief Cancels the running tree and reports Cancelled to the owner exactly once.
 */
void Async::TaskRunner::cancel()
{
  SS_ASSERT(m_thread == QThread::currentThread(), return);

  if (m_root == nullptr || !m_root->isRunning())
    return;

  m_root->cancel();
}

/**
 * @brief Adopts and starts a tree, cancelling and dropping any previous root first so a
 *        superseded flow can never deliver a continuation.
 */
void Async::TaskRunner::run(Task* root)
{
  SS_ASSERT(root != nullptr, return);
  SS_ASSERT(m_thread == QThread::currentThread(), return);

  stopRoot();
  m_root.reset(root);
  m_rootConnection = connect(m_root.get(), &Task::finished, this, &TaskRunner::onRootFinished);
  m_root->start();
}

/**
 * @brief Silently drops the current root: the outcome connection goes first, so the cancel that
 *        follows notifies nobody.
 */
void Async::TaskRunner::stopRoot()
{
  if (m_root == nullptr)
    return;

  QObject::disconnect(m_rootConnection);
  m_rootConnection = QMetaObject::Connection();

  if (m_root->isRunning())
    m_root->cancel();

  m_root.reset();
}

/**
 * @brief Forwards the root's terminal outcome to the owner.
 */
void Async::TaskRunner::onRootFinished(Outcome outcome, const StepError& error)
{
  SS_ASSERT_LOG(m_root != nullptr);
  SS_ASSERT_LOG(m_thread == QThread::currentThread());

  Q_EMIT finished(outcome, error);
}

//--------------------------------------------------------------------------------------------------
// Combinator vocabulary
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates an empty parallel group.
 */
Async::ParallelGroup* Async::parallel(QString name)
{
  return new ParallelGroup(std::move(name));
}

/**
 * @brief Creates an empty signal-wait step.
 */
Async::SignalTask* Async::awaitSignal(QString name)
{
  return new SignalTask(std::move(name));
}

/**
 * @brief Creates an empty sequential group.
 */
Async::SequentialGroup* Async::sequential(QString name)
{
  return new SequentialGroup(std::move(name));
}

/**
 * @brief Creates a synchronous step from a callable.
 */
Async::InvokeTask* Async::invoke(QString name, InvokeTask::Callable callable)
{
  return new InvokeTask(std::move(name), std::move(callable));
}

/**
 * @brief Wraps a child in a deadline.
 */
Async::TimeoutTask* Async::timeout(Task* child, int msec, AsyncClock& clock)
{
  return new TimeoutTask(child, msec, clock);
}

/**
 * @brief Wraps a child in the shared retry policy.
 */
Async::RetryTask* Async::retry(Task* child, const RetryPolicy& policy, AsyncClock& clock)
{
  return new RetryTask(child, policy, clock);
}

/**
 * @brief Attaches a completion handler to a task and returns it, so a tree reads as the sequence
 *        it describes instead of as a set of separate connect calls.
 */
Async::Task* Async::onDone(Task* task, std::function<void(Outcome, const StepError&)> handler)
{
  SS_ASSERT(task != nullptr, return nullptr);
  SS_ASSERT(static_cast<bool>(handler), return task);

  QObject::connect(task, &Task::finished, task, std::move(handler));
  return task;
}
