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

#include "IO/Drivers/UART/SequenceRunner.h"

#include "Core/SSAssert.h"
#include "IO/Drivers/UART/SequencePort.h"
#include "IO/Drivers/UART/UartPolicy.h"

//--------------------------------------------------------------------------------------------------
// Step tasks
//--------------------------------------------------------------------------------------------------

namespace IO::Drivers::detail {
/**
 * @brief Waits a fixed time on the runner's clock. Cancelling releases the timer.
 */
class DelayTask final : public Async::Task {
public:
  DelayTask(QString name, int ms, Async::AsyncClock& clock);
  ~DelayTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void stopTimer();
  void onElapsed();

private:
  int m_ms;
  Async::TimerId m_timer;
  Async::AsyncClock& m_clock;
};

/**
 * @brief Holds the line in BREAK for a fixed time, then releases it. A refused BREAK fails the
 *        step; a granted one is trusted for its duration (the port reports success, not timing).
 *        Cancelling mid-BREAK releases the line, so a disconnect never leaves it held.
 */
class BreakTask final : public Async::Task {
public:
  using Setter = std::function<bool(bool)>;

  BreakTask(QString name, int ms, Async::AsyncClock& clock, Setter setBreak);
  ~BreakTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void stopTimer();
  void onElapsed();

private:
  int m_ms;
  bool m_held;
  Setter m_setBreak;
  Async::TimerId m_timer;
  Async::AsyncClock& m_clock;
};

/**
 * @brief Waits for the runner's receive buffer to match one of the step's patterns, consuming
 *        through the earliest match. The runner polls it on every fed chunk through @c pollSlot,
 *        which the task owns only while it runs. An optional step that times out reports the
 *        skip through @c skipped and succeeds.
 */
class ExpectTask final : public Async::Task {
public:
  using Poll = std::function<void()>;
  using Note = std::function<void(const QString&)>;

  ExpectTask(QString name,
             const IO::SequenceStep& step,
             Async::AsyncClock& clock,
             QByteArray& rx,
             Poll& pollSlot,
             Note skipped);
  ~ExpectTask() override;

protected:
  void doStart() override;
  void doCancel() override;

private:
  void check();
  void release();
  void onExpired();

private:
  bool m_armed;
  bool m_optional;
  int m_timeoutMs;
  Note m_skipped;
  Poll& m_pollSlot;
  QByteArray& m_rx;
  Async::TimerId m_timer;
  Async::AsyncClock& m_clock;
  QList<IO::ExpectPattern> m_patterns;
};
}  // namespace IO::Drivers::detail

using IO::Drivers::detail::BreakTask;
using IO::Drivers::detail::DelayTask;
using IO::Drivers::detail::ExpectTask;

/**
 * @brief The QSerialPort mode a flow-control step selects.
 */
static QSerialPort::FlowControl flowControlFor(IO::SequenceStep::Flow flow)
{
  if (flow == IO::SequenceStep::Flow::RtsCts)
    return QSerialPort::HardwareControl;

  if (flow == IO::SequenceStep::Flow::XonXoff)
    return QSerialPort::SoftwareControl;

  return QSerialPort::NoFlowControl;
}

//--------------------------------------------------------------------------------------------------
// Delay task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a wait of @p ms milliseconds.
 */
DelayTask::DelayTask(QString name, int ms, Async::AsyncClock& clock)
  : Async::Task(std::move(name)), m_ms(ms), m_timer(Async::kInvalidTimerId), m_clock(clock)
{
  SS_ASSERT(m_ms >= 0, m_ms = 0);
}

/**
 * @brief Releases a pending timer.
 */
DelayTask::~DelayTask()
{
  stopTimer();
}

/**
 * @brief Arms the timer.
 */
void DelayTask::doStart()
{
  SS_ASSERT_LOG(isRunning());

  m_timer = m_clock.schedule(m_ms, [this]() { onElapsed(); });
}

/**
 * @brief Releases the timer; the base reports Cancelled.
 */
void DelayTask::doCancel()
{
  stopTimer();
}

/**
 * @brief Cancels the pending timer, if any.
 */
void DelayTask::stopTimer()
{
  if (m_timer == Async::kInvalidTimerId)
    return;

  m_clock.cancel(m_timer);
  m_timer = Async::kInvalidTimerId;
}

/**
 * @brief Finishes the wait.
 */
void DelayTask::onElapsed()
{
  m_timer = Async::kInvalidTimerId;
  if (isRunning())
    reportFinished(Async::Outcome::Success, Async::StepError());
}

//--------------------------------------------------------------------------------------------------
// Break task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a BREAK of @p ms milliseconds driven through @p setBreak.
 */
BreakTask::BreakTask(QString name, int ms, Async::AsyncClock& clock, Setter setBreak)
  : Async::Task(std::move(name))
  , m_ms(ms)
  , m_held(false)
  , m_setBreak(std::move(setBreak))
  , m_timer(Async::kInvalidTimerId)
  , m_clock(clock)
{
  SS_ASSERT(m_ms >= 0, m_ms = 0);
  SS_ASSERT_LOG(static_cast<bool>(m_setBreak));
}

/**
 * @brief Releases a pending timer. The line itself is released by cancel(), never here: the port
 *        may already be gone when the tree is destroyed.
 */
BreakTask::~BreakTask()
{
  stopTimer();
}

/**
 * @brief Asserts BREAK and arms the release timer, or fails when the port refuses BREAK.
 */
void BreakTask::doStart()
{
  SS_ASSERT_LOG(isRunning());

  if (!m_setBreak(true)) {
    reportFinished(Async::Outcome::Failure,
                   errorHere(IO::Drivers::SequenceRunner::tr("the port refused BREAK")));
    return;
  }

  m_held  = true;
  m_timer = m_clock.schedule(m_ms, [this]() { onElapsed(); });
}

/**
 * @brief Stops the timer and releases the line.
 */
void BreakTask::doCancel()
{
  stopTimer();
  if (!m_held)
    return;

  m_held = false;
  (void)m_setBreak(false);
}

/**
 * @brief Cancels the pending timer, if any.
 */
void BreakTask::stopTimer()
{
  if (m_timer == Async::kInvalidTimerId)
    return;

  m_clock.cancel(m_timer);
  m_timer = Async::kInvalidTimerId;
}

/**
 * @brief Releases the line once the BREAK lasted its duration.
 */
void BreakTask::onElapsed()
{
  m_timer = Async::kInvalidTimerId;
  if (!isRunning())
    return;

  m_held = false;
  if (!m_setBreak(false)) {
    reportFinished(Async::Outcome::Failure,
                   errorHere(IO::Drivers::SequenceRunner::tr("the port did not release BREAK")));
    return;
  }

  reportFinished(Async::Outcome::Success, Async::StepError());
}

//--------------------------------------------------------------------------------------------------
// Expect task
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an expect step matching @p step's patterns against the runner's @p rx.
 */
ExpectTask::ExpectTask(QString name,
                       const IO::SequenceStep& step,
                       Async::AsyncClock& clock,
                       QByteArray& rx,
                       Poll& pollSlot,
                       Note skipped)
  : Async::Task(std::move(name))
  , m_armed(false)
  , m_optional(step.optional)
  , m_timeoutMs(step.timeoutMs)
  , m_skipped(std::move(skipped))
  , m_pollSlot(pollSlot)
  , m_rx(rx)
  , m_timer(Async::kInvalidTimerId)
  , m_clock(clock)
  , m_patterns(step.expectPatterns())
{
  SS_ASSERT_LOG(step.kind == IO::SequenceStep::Kind::Expect);
  SS_ASSERT_LOG(!m_patterns.isEmpty());
}

/**
 * @brief Releases the poll slot and the timer.
 */
ExpectTask::~ExpectTask()
{
  release();
}

/**
 * @brief Arms the timeout before the first check, so a reply already buffered still finds a
 *        timer to release.
 */
void ExpectTask::doStart()
{
  SS_ASSERT_LOG(isRunning());

  m_armed    = true;
  m_pollSlot = [this]() {
    check();
  };
  m_timer = m_clock.schedule(m_timeoutMs, [this]() { onExpired(); });
  check();
}

/**
 * @brief Releases the poll slot and the timer; the base reports Cancelled.
 */
void ExpectTask::doCancel()
{
  release();
}

/**
 * @brief Finishes on the earliest match of any pattern, consuming the buffer through it.
 */
void ExpectTask::check()
{
  if (!isRunning())
    return;

  SS_ASSERT_LOG(m_rx.size() <= IO::kExpectBufferBytes);

  qsizetype best = -1;
  for (const auto& pattern : std::as_const(m_patterns)) {
    const auto end = pattern.matchIn(m_rx);
    if (end > 0 && (best < 0 || end < best))
      best = end;
  }

  if (best < 0)
    return;

  SS_ASSERT(best <= m_rx.size(), return);

  m_rx.remove(0, best);
  release();
  reportFinished(Async::Outcome::Success, Async::StepError());
}

/**
 * @brief Hands the poll slot back and cancels the timeout.
 */
void ExpectTask::release()
{
  if (m_armed) {
    m_armed    = false;
    m_pollSlot = nullptr;
  }

  if (m_timer == Async::kInvalidTimerId)
    return;

  m_clock.cancel(m_timer);
  m_timer = Async::kInvalidTimerId;
}

/**
 * @brief Fails the step when no pattern matched in time, or reports the skip and succeeds when
 *        the step is optional.
 */
void ExpectTask::onExpired()
{
  m_timer = Async::kInvalidTimerId;
  if (!isRunning())
    return;

  release();
  const auto reason =
    IO::Drivers::SequenceRunner::tr("no reply matched within %1 ms").arg(m_timeoutMs);
  if (m_optional) {
    m_skipped(reason);
    reportFinished(Async::Outcome::Success, Async::StepError());
    return;
  }

  reportFinished(Async::Outcome::Failure, errorHere(reason));
}

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the sequence, the port it drives and the clock every step's timer comes from.
 */
IO::Drivers::SequenceRunner::SequenceRunner(const IO::ConnectionSequence& sequence,
                                            ISequencePort& port,
                                            Async::AsyncClock& clock,
                                            QObject* parent)
  : QObject(parent)
  , m_running(false)
  , m_cancelled(false)
  , m_breakHeld(false)
  , m_current(-1)
  , m_port(port)
  , m_clock(clock)
  , m_sequence(sequence)
  , m_runner(&clock, this)
{
  connect(&m_runner, &Async::TaskRunner::finished, this, &SequenceRunner::onTreeFinished);
}

/**
 * @brief Drops the tree silently; a run still in flight reports nothing.
 */
IO::Drivers::SequenceRunner::~SequenceRunner()
{
  cancel();
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a BREAK step currently holds the line.
 */
bool IO::Drivers::SequenceRunner::breakHeld() const noexcept
{
  return m_breakHeld;
}

/**
 * @brief Whether a run is in flight.
 */
bool IO::Drivers::SequenceRunner::isRunning() const noexcept
{
  return m_running;
}

/**
 * @brief The zero-based index of the step running now, -1 before the first.
 */
int IO::Drivers::SequenceRunner::currentStep() const noexcept
{
  return m_current;
}

//--------------------------------------------------------------------------------------------------
// Run control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates the sequence and runs it; an invalid sequence fails at once, naming the step.
 *        A runner cancelled before it started never runs.
 */
void IO::Drivers::SequenceRunner::start()
{
  SS_ASSERT(!m_running, return);

  if (m_cancelled)
    return;

  m_rx.clear();
  m_current   = -1;
  m_running   = true;
  m_breakHeld = false;

  const auto error = m_sequence.validate();
  if (!error.message.isEmpty()) {
    const auto text = error.step < 0 ? tr("Connection Sequence is not valid: %1").arg(error.message)
                                     : failureText(error.step, error.message);
    finishWith(false, text, error.step);
    return;
  }

  m_runner.run(buildTree());
}

/**
 * @brief Cancels the run without a verdict: a user disconnect is not a failed step. Releases a
 *        held BREAK through the step's own cancel path.
 */
void IO::Drivers::SequenceRunner::cancel()
{
  SS_ASSERT_LOG(m_running || !m_breakHeld);

  m_cancelled = true;
  if (!m_running)
    return;

  m_running = false;
  m_runner.cancel();
  SS_ASSERT_LOG(!m_runner.isRunning());

  m_poll      = nullptr;
  m_breakHeld = false;
}

/**
 * @brief Adds bytes the port received to the bounded receive buffer and lets a running expect
 *        step look at them. Bytes received while BREAK is held are dropped.
 */
void IO::Drivers::SequenceRunner::feed(const QByteArray& bytes)
{
  if (!m_running || m_breakHeld || bytes.isEmpty())
    return;

  m_rx.append(bytes);
  if (m_rx.size() > IO::kExpectBufferBytes)
    m_rx.remove(0, m_rx.size() - IO::kExpectBufferBytes);

  SS_ASSERT_LOG(m_rx.size() <= IO::kExpectBufferBytes);

  const auto poll = m_poll;
  if (poll)
    poll();
}

//--------------------------------------------------------------------------------------------------
// Tree construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds TimeoutTask(budget) { SequentialGroup { announce, step }... }: each step is
 *        preceded by an instant task that names it, so the console line lands before its bytes.
 */
Async::Task* IO::Drivers::SequenceRunner::buildTree()
{
  SS_ASSERT_LOG(m_sequence.steps.size() <= IO::kMaxSequenceSteps);

  auto* group = Async::sequential(QStringLiteral("connection-sequence"));
  for (int i = 0; i < m_sequence.steps.size(); ++i) {
    group->addChild(Async::invoke(QString::number(i), [this, i](QString&) {
      beginStep(i);
      return true;
    }));
    group->addChild(makeStep(i));
  }

  return Async::timeout(group, m_sequence.budgetMs, m_clock);
}

/**
 * @brief Builds the task for step @p index: the three timed kinds get their own task, the
 *        instant kinds run as one InvokeTask each.
 */
Async::Task* IO::Drivers::SequenceRunner::makeStep(int index)
{
  SS_ASSERT(index >= 0 && index < m_sequence.steps.size(), return nullptr);

  const auto& step = m_sequence.steps.at(index);
  const auto name  = QString::number(index);
  switch (step.kind) {
    case IO::SequenceStep::Kind::Break:
      return new BreakTask(name, step.ms, m_clock, [this](bool on) { return holdBreak(on); });
    case IO::SequenceStep::Kind::Wait:
      return new DelayTask(name, step.ms, m_clock);
    case IO::SequenceStep::Kind::Expect:
      return new ExpectTask(name, step, m_clock, m_rx, m_poll, [this, index](const QString& why) {
        Q_EMIT stepNote(
          index,
          tr("Step %1 got no reply and was skipped: %2").arg(QString::number(index + 1), why));
      });
    case IO::SequenceStep::Kind::SetLine:
    case IO::SequenceStep::Kind::Send:
    case IO::SequenceStep::Kind::Discard:
    case IO::SequenceStep::Kind::SetBaud:
    case IO::SequenceStep::Kind::SetFlowControl:
    case IO::SequenceStep::Kind::Unknown:
      break;
  }

  return Async::invoke(name, [this, index](QString& reason) { return runInstant(index, reason); });
}

//--------------------------------------------------------------------------------------------------
// Step execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks step @p index as running and announces it.
 */
void IO::Drivers::SequenceRunner::beginStep(int index)
{
  SS_ASSERT(index >= 0 && index < m_sequence.steps.size(), return);

  m_current = index;
  Q_EMIT stepStarted(index, m_sequence.stepText(index));
}

/**
 * @brief Drives BREAK on the port. Releasing it also discards what the port buffered while it was
 *        held, since a device's reaction to BREAK is noise rather than a reply.
 */
bool IO::Drivers::SequenceRunner::holdBreak(bool enabled)
{
  SS_ASSERT_LOG(!enabled || m_running);
  SS_ASSERT_LOG(!enabled || !m_breakHeld);

  const bool ok = m_port.setBreak(enabled);
  m_breakHeld   = enabled && ok;
  if (enabled)
    return ok;

  (void)m_port.clearInput();
  m_rx.clear();
  return ok;
}

/**
 * @brief Runs one instant step: a line, a send, a discard, a baud or a flow-control change.
 */
bool IO::Drivers::SequenceRunner::runInstant(int index, QString& reason)
{
  SS_ASSERT(index >= 0 && index < m_sequence.steps.size(), return false);

  using Kind       = IO::SequenceStep::Kind;
  const auto& step = m_sequence.steps.at(index);
  bool ok          = false;
  switch (step.kind) {
    case Kind::Send:
      return runSend(step, reason);
    case Kind::SetLine:
      ok = m_port.setLine(step.line, step.high);
      break;
    case Kind::Discard:
      ok = m_port.clearInput();
      m_rx.clear();
      break;
    case Kind::SetBaud:
      ok = m_port.setBaud(step.rate);
      break;
    case Kind::SetFlowControl:
      ok = m_port.setFlowControl(flowControlFor(step.flow));
      break;
    case Kind::Break:
    case Kind::Wait:
    case Kind::Expect:
    case Kind::Unknown:
      reason = tr("this step cannot run here");
      return false;
  }

  if (!ok)
    reason = tr("the port refused: %1").arg(m_port.errorString());

  return ok;
}

/**
 * @brief Writes a send step's bytes; anything short of the whole payload fails the step.
 */
bool IO::Drivers::SequenceRunner::runSend(const IO::SequenceStep& step, QString& reason)
{
  SS_ASSERT_LOG(step.kind == IO::SequenceStep::Kind::Send);

  const auto bytes   = step.sendBytes();
  const auto written = m_port.write(bytes);
  if (written != bytes.size()) {
    reason = tr("the port accepted %1 of %2 bytes: %3")
               .arg(QString::number(qMax<qint64>(written, 0)),
                    QString::number(bytes.size()),
                    m_port.errorString());
    return false;
  }

  Q_EMIT bytesSent(bytes);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Verdict
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps the tree's outcome onto the one verdict: success, the budget running out, or the
 *        failing step. A cancelled tree reports nothing.
 */
void IO::Drivers::SequenceRunner::onTreeFinished(Async::Outcome outcome,
                                                 const Async::StepError& error)
{
  if (!m_running || outcome == Async::Outcome::Cancelled)
    return;

  SS_ASSERT_LOG(m_current >= -1 && m_current < m_sequence.steps.size());
  SS_ASSERT_LOG(outcome != Async::Outcome::Success || m_current == m_sequence.steps.size() - 1);

  if (outcome == Async::Outcome::Success) {
    finishWith(true, QString(), m_current);
    return;
  }

  const auto reason =
    outcome == Async::Outcome::TimedOut
      ? tr("the whole sequence ran past its %1 ms time limit").arg(m_sequence.budgetMs)
      : error.reason;

  finishWith(false, failureText(qMax(m_current, 0), reason), m_current);
}

/**
 * @brief The verdict text of a failure at step @p index; an expect step quotes the receive
 *        buffer, which still holds everything since the last consumed reply.
 */
QString IO::Drivers::SequenceRunner::failureText(int index, const QString& reason) const
{
  SS_ASSERT(index >= 0 && index < m_sequence.steps.size(), return reason);

  const auto kind = m_sequence.steps.at(index).kind;
  const auto quote =
    kind == IO::SequenceStep::Kind::Expect ? IO::ConnectionSequence::hexText(m_rx) : QString();
  return UartPolicy::sequenceFailureText(
    IO::ConnectionSequence::kindLabel(kind), index, reason, quote);
}

/**
 * @brief Ends the run and emits its single verdict.
 */
void IO::Drivers::SequenceRunner::finishWith(bool ok, const QString& reason, int index)
{
  SS_ASSERT(m_running, return);

  m_running   = false;
  m_poll      = nullptr;
  m_breakHeld = false;
  Q_EMIT finished(ok, reason, index);
}
