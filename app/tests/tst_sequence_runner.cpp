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

#include <QHash>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTest>

#include "Core/Async/AsyncClock.h"
#include "Core/IO/ConnectionSequence.h"
#include "IO/Drivers/UART/SequencePort.h"
#include "IO/Drivers/UART/SequenceRunner.h"

/**
 * @brief Deterministic clock: callbacks fire only when the test advances time, in due order.
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
    return id;
  }

  [[nodiscard]] qint64 now() const { return m_now; }

  [[nodiscard]] int pendingCount() const { return static_cast<int>(m_entries.size()); }

  void advance(qint64 msec)
  {
    const qint64 target = m_now + msec;
    for (int guard = 0; guard < 4096; ++guard) {
      const Async::TimerId next = dueEntry(target);
      if (next == Async::kInvalidTimerId)
        break;

      const Entry entry = m_entries.take(next);
      m_now             = entry.due;
      entry.callback();
    }

    m_now = target;
  }

private:
  struct Entry {
    qint64 due;
    Callback callback;
  };

  [[nodiscard]] Async::TimerId dueEntry(qint64 target) const
  {
    Async::TimerId winner = Async::kInvalidTimerId;
    qint64 best           = 0;
    for (auto it = m_entries.cbegin(); it != m_entries.cend(); ++it) {
      if (it.value().due > target)
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
  QHash<Async::TimerId, Entry> m_entries;
};

/**
 * @brief Scripted port: records every call with the virtual time it happened at, and refuses
 *        whichever operation the test names.
 */
class FakePort final : public IO::Drivers::ISequencePort {
public:
  explicit FakePort(const VirtualClock& clock)
    : refuseWrite(false), refuseBreak(false), refuseBaud(false), breakOn(false), m_clock(clock)
  {}

  bool clearInput() override
  {
    calls.append(QStringLiteral("clear"));
    return true;
  }

  bool setBaud(qint32 rate) override
  {
    calls.append(QStringLiteral("baud %1").arg(rate));
    return !refuseBaud;
  }

  bool setBreak(bool enabled) override
  {
    if (refuseBreak)
      return false;

    breakOn = enabled;
    calls.append(QStringLiteral("break %1 @%2")
                   .arg(enabled ? QStringLiteral("on") : QStringLiteral("off"),
                        QString::number(m_clock.now())));
    return true;
  }

  QString errorString() const override { return QStringLiteral("device gone"); }

  qint64 write(const QByteArray& data) override
  {
    if (refuseWrite)
      return -1;

    written.append(data);
    calls.append(QStringLiteral("write %1").arg(QString::fromLatin1(data.toHex())));
    return data.size();
  }

  QSerialPort::PinoutSignals lineStates() const override { return QSerialPort::NoSignal; }

  bool setFlowControl(QSerialPort::FlowControl mode) override
  {
    calls.append(QStringLiteral("flow %1").arg(static_cast<int>(mode)));
    return true;
  }

  bool setLine(IO::SequenceStep::Line line, bool high) override
  {
    calls.append(
      QStringLiteral("line %1 %2")
        .arg(line == IO::SequenceStep::Line::Rts ? QStringLiteral("rts") : QStringLiteral("dtr"),
             QString::number(high ? 1 : 0)));
    return true;
  }

  bool refuseWrite;
  bool refuseBreak;
  bool refuseBaud;
  bool breakOn;
  QStringList calls;
  QByteArray written;

private:
  const VirtualClock& m_clock;
};

/**
 * @brief The Connection Sequence runner of spec 0103 (AC4's logic half, AC2's discard rule, AC5's
 *        ordering): one verdict per run and only after the last step, each failure kind naming
 *        its step, the optional flag, the budget, silent cancel and BREAK timing.
 */
class TstSequenceRunner : public QObject {
  Q_OBJECT

private slots:
  void sixStepSuccess();
  void expectTimeoutQuotesReply();
  void writeRefusalFailsTheStep();
  void portRefusalFailsTheStep();
  void optionalExpectContinues();
  void budgetExpires();
  void cancelMidStepReportsNothing();
  void cancelAfterFinishIsNoop();
  void cancelReleasesBreak();
  void breakHoldsRequestedTime();
  void bytesDuringBreakAreDropped();
  void invalidSequenceFailsAtOnce();

private:
  [[nodiscard]] static IO::ConnectionSequence sequence(const char* json);
};

/**
 * @brief Parses a sequence from JSON text.
 */
IO::ConnectionSequence TstSequenceRunner::sequence(const char* json)
{
  return IO::ConnectionSequence::fromJson(QJsonDocument::fromJson(json).object());
}

/**
 * @brief BREAK, wait, send, expect, baud, send: the verdict arrives once, after the last step.
 */
void TstSequenceRunner::sixStepSuccess()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"steps":[
    {"kind":"break","ms":55},
    {"kind":"wait","ms":50},
    {"kind":"send","data":"15 15","hex":true},
    {"kind":"expect","patterns":["55","5D 5?"],"timeoutMs":100},
    {"kind":"baud","rate":115200},
    {"kind":"send","data":"AA","hex":true}]})"),
                                     port,
                                     clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  QSignalSpy started(&runner, &IO::Drivers::SequenceRunner::stepStarted);
  QSignalSpy sent(&runner, &IO::Drivers::SequenceRunner::bytesSent);

  runner.start();
  QVERIFY(runner.isRunning());
  QVERIFY(port.breakOn);

  clock.advance(55);
  QVERIFY(!port.breakOn);
  clock.advance(50);
  QCOMPARE(port.written, QByteArray::fromHex("1515"));
  QCOMPARE(finished.count(), 0);

  runner.feed(QByteArray::fromHex("00"));
  QCOMPARE(finished.count(), 0);
  runner.feed(QByteArray::fromHex("5D5A"));

  QCOMPARE(finished.count(), 1);
  QVERIFY(finished.at(0).at(0).toBool());
  QVERIFY(!runner.isRunning());
  QCOMPARE(started.count(), 6);
  QCOMPARE(sent.count(), 2);
  QCOMPARE(port.written, QByteArray::fromHex("1515AA"));
  QVERIFY(port.calls.contains(QStringLiteral("baud 115200")));
  QCOMPARE(clock.pendingCount(), 0);
}

/**
 * @brief An expect that times out fails its own step and quotes what the device did send.
 */
void TstSequenceRunner::expectTimeoutQuotesReply()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"steps":[{"kind":"send","data":"AT"},
                          {"kind":"expect","patterns":["4F 4B"],"timeoutMs":100}]})"),
                                     port,
                                     clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  runner.feed(QByteArrayLiteral("ER"));
  clock.advance(99);
  QCOMPARE(finished.count(), 0);

  clock.advance(1);
  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 1);

  const auto reason = finished.at(0).at(1).toString();
  QVERIFY(reason.contains(QStringLiteral("45 52")));
  QVERIFY(reason.contains(QStringLiteral("100")));
}

/**
 * @brief A write the port refuses fails the send step with a reason.
 */
void TstSequenceRunner::writeRefusalFailsTheStep()
{
  VirtualClock clock;
  FakePort port(clock);
  port.refuseWrite = true;
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"discard"},{"kind":"send","data":"AT"}]})"), port, clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();

  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 1);
  QVERIFY(!finished.at(0).at(1).toString().isEmpty());
}

/**
 * @brief An operation the port refuses (the device is gone) fails that step with the port's own
 *        error text.
 */
void TstSequenceRunner::portRefusalFailsTheStep()
{
  VirtualClock clock;
  FakePort port(clock);
  port.refuseBaud = true;
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"wait","ms":10},{"kind":"baud","rate":115200}]})"), port, clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  clock.advance(10);

  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 1);
  QVERIFY(finished.at(0).at(1).toString().contains(QStringLiteral("device gone")));
}

/**
 * @brief An optional expect that times out leaves a note and the sequence carries on.
 */
void TstSequenceRunner::optionalExpectContinues()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"expect","patterns":["55"],"timeoutMs":50,"optional":true},
                          {"kind":"wait","ms":10}]})"),
    port,
    clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  QSignalSpy notes(&runner, &IO::Drivers::SequenceRunner::stepNote);
  runner.start();

  clock.advance(50);
  QCOMPARE(notes.count(), 1);
  QCOMPARE(notes.at(0).at(0).toInt(), 0);
  QCOMPARE(finished.count(), 0);

  clock.advance(10);
  QCOMPARE(finished.count(), 1);
  QVERIFY(finished.at(0).at(0).toBool());
}

/**
 * @brief The whole-sequence budget ends a run whose step would outlast it, naming that step.
 */
void TstSequenceRunner::budgetExpires()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"budgetMs":100,
                 "steps":[{"kind":"expect","patterns":["55"],"timeoutMs":500}]})"),
                                     port,
                                     clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  clock.advance(100);

  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 0);
  QVERIFY(finished.at(0).at(1).toString().contains(QStringLiteral("100")));
  QCOMPARE(clock.pendingCount(), 0);
}

/**
 * @brief A cancel mid-step is a user disconnect, not a failed step: no verdict, no timer left.
 */
void TstSequenceRunner::cancelMidStepReportsNothing()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"wait","ms":100},{"kind":"send","data":"AT"}]})"), port, clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  clock.advance(50);
  runner.cancel();
  clock.advance(1000);

  QCOMPARE(finished.count(), 0);
  QVERIFY(!runner.isRunning());
  QVERIFY(port.written.isEmpty());
  QCOMPARE(clock.pendingCount(), 0);
}

/**
 * @brief cancel() after the verdict changes nothing; this is the re-entrant close() that the
 *        connection manager performs from inside a failure verdict.
 */
void TstSequenceRunner::cancelAfterFinishIsNoop()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"steps":[{"kind":"discard"}]})"), port, clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  QCOMPARE(finished.count(), 1);

  runner.cancel();
  QCOMPARE(finished.count(), 1);
  QVERIFY(!runner.isRunning());
}

/**
 * @brief A cancel while BREAK is held releases the line.
 */
void TstSequenceRunner::cancelReleasesBreak()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"break","ms":100}]})"), port, clock);

  runner.start();
  QVERIFY(port.breakOn);
  clock.advance(10);
  runner.cancel();
  QVERIFY(!port.breakOn);
  QCOMPARE(clock.pendingCount(), 0);
}

/**
 * @brief BREAK is asserted at the step's start and released exactly its duration later.
 */
void TstSequenceRunner::breakHoldsRequestedTime()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"steps":[{"kind":"wait","ms":7},
                                                           {"kind":"break","ms":55}]})"),
                                     port,
                                     clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  clock.advance(100);

  QCOMPARE(finished.count(), 1);
  QVERIFY(port.calls.contains(QStringLiteral("break on @7")));
  QVERIFY(port.calls.contains(QStringLiteral("break off @62")));
}

/**
 * @brief Bytes the device sends while BREAK is held never reach an expect step.
 */
void TstSequenceRunner::bytesDuringBreakAreDropped()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(sequence(R"({"steps":[{"kind":"break","ms":50},
                          {"kind":"expect","patterns":["55"],"timeoutMs":100}]})"),
                                     port,
                                     clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();
  QVERIFY(runner.breakHeld());
  runner.feed(QByteArray::fromHex("55"));

  clock.advance(50);
  QVERIFY(!runner.breakHeld());
  QCOMPARE(finished.count(), 0);

  clock.advance(100);
  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 1);
}

/**
 * @brief A sequence that does not validate fails at start, naming the step, before touching the
 *        port.
 */
void TstSequenceRunner::invalidSequenceFailsAtOnce()
{
  VirtualClock clock;
  FakePort port(clock);
  IO::Drivers::SequenceRunner runner(
    sequence(R"({"steps":[{"kind":"discard"},{"kind":"send","data":""}]})"), port, clock);

  QSignalSpy finished(&runner, &IO::Drivers::SequenceRunner::finished);
  runner.start();

  QCOMPARE(finished.count(), 1);
  QVERIFY(!finished.at(0).at(0).toBool());
  QCOMPARE(finished.at(0).at(2).toInt(), 1);
  QVERIFY(port.calls.isEmpty());
}

QTEST_GUILESS_MAIN(TstSequenceRunner)

#include "tst_sequence_runner.moc"
