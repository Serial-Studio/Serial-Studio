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

#include <QByteArray>
#include <QElapsedTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/JsWatchdog.h"
#include "DataModel/Scripting/JsWatchdogThread.h"

// Every test function here is self-contained but the declaration order IS load-bearing for the last
// slot: JsWatchdogThread::shutdown() latches m_stopped for the life of the process, so once
// shutdownStopsTheWorkerThread() has run no later slot could ever be interrupted. It is declared
// last for that reason and nothing may be added below it.
//
// setInterrupted(true) is JsWatchdogThread.cpp's exclusive privilege, so every timeout below is
// produced by arming a real watchdog and letting the real worker thread expire it; the test never
// forces the flag itself.

//--------------------------------------------------------------------------------------------------
// Script fixtures and timing constants
//--------------------------------------------------------------------------------------------------

// The worker polls every 20 ms, so a trip budget has to be several poll intervals wide before an
// "it timed out" claim is about the budget rather than about scheduler jitter. Every assertion is
// on the outcome plus a one-sided lower bound; no test asserts an upper bound on wall time.
static constexpr int kTripBudgetMs = 250;

static constexpr int kGenerousBudgetMs = 30000;

static constexpr int kLoopIterations = 2000000;

// The modulo keeps the body live under both the interpreter and the baseline JIT, so the loop
// cannot be folded away before the interrupt check at the back edge is reached.
static const QString kInfiniteLoop =
  QStringLiteral("(function() { var i = 0; while (true) { i = (i + 1) % 1000003; } return i; })");

static const QString kBoundedLoop = QStringLiteral(
  "(function(n) { var s = 0; for (var i = 0; i < n; ++i) s = (s + i) % 65521; return s; })");

/**
 * @brief Builds the exact diagnostic finishCall() emits, so a timeout test consumes its own warning
 *        instead of leaving it loose in the log.
 */
static QByteArray timeoutWarning(const QString& tag, int budgetMs)
{
  return QStringLiteral("[JsWatchdog] %1 timed out after %2 ms -- interrupted")
    .arg(tag)
    .arg(budgetMs)
    .toUtf8();
}

/**
 * @brief Runtime contract of JsWatchdog and the singleton worker thread that expires it: value
 *        pass-through, interruption of a runaway script, engine reuse afterwards, budget handling,
 *        per-instance isolation, and the shutdown path.
 */
class JsWatchdogTests : public QObject {
  Q_OBJECT

private slots:
  void fastCallReturnsItsValue();
  void accessorsReportEngineBudgetAndRetiredDeadline();

  void infiniteLoopIsInterruptedWithinBudget();
  void engineStaysUsableAfterAnInterrupt();
  void tinyBudgetTripsWhereGenerousBudgetDoesNot();

  void callWithInstancePassesThisAndArguments();
  void thrownExceptionIsAnErrorNotATimeout();

  void onlyTheExpiredWatchdogIsInterrupted();
  void armFollowedByDisarmNeverInterrupts();

  void shutdownStopsTheWorkerThread();
};

//--------------------------------------------------------------------------------------------------
// Value pass-through
//--------------------------------------------------------------------------------------------------

/**
 * @brief A call that finishes well inside its budget returns the script's own value and reports no
 *        timeout, and the deadline is retired before call() returns.
 */
void JsWatchdogTests::fastCallReturnsItsValue()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kGenerousBudgetMs, QStringLiteral("fast"));

  QJSValue fn = engine.evaluate(QStringLiteral("(function(a, b) { return a * b; })"));
  QVERIFY(fn.isCallable());

  const QJSValue result = watchdog.call(fn, QJSValueList{QJSValue(6), QJSValue(7)});

  QVERIFY(!result.isError());
  QCOMPARE(result.toInt(), 42);
  QVERIFY(!watchdog.lastCallTimedOut());
  QVERIFY(!engine.isInterrupted());
  QCOMPARE(watchdog.deadlineNs(), qint64(0));
}

/**
 * @brief The watchdog-thread accessors and the budget round-trip: a fresh instance is disarmed,
 *        reports no timeout, and hands back the engine it was built with.
 */
void JsWatchdogTests::accessorsReportEngineBudgetAndRetiredDeadline()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, 750, QStringLiteral("accessors"));

  QCOMPARE(watchdog.engine(), &engine);
  QCOMPARE(watchdog.budgetMs(), 750);
  QCOMPARE(watchdog.deadlineNs(), qint64(0));
  QVERIFY(!watchdog.lastCallTimedOut());

  watchdog.setBudgetMs(kTripBudgetMs);
  QCOMPARE(watchdog.budgetMs(), kTripBudgetMs);

  watchdog.arm();
  QVERIFY(watchdog.deadlineNs() > 0);

  watchdog.disarm();
  QCOMPARE(watchdog.deadlineNs(), qint64(0));
}

//--------------------------------------------------------------------------------------------------
// Interruption
//--------------------------------------------------------------------------------------------------

/**
 * @brief A script that never returns is aborted by the worker thread: call() comes back, the result
 *        is an error value, the timeout is reported, and the engine interrupt flag is cleared again
 *        so the next call starts from a clean slate.
 */
void JsWatchdogTests::infiniteLoopIsInterruptedWithinBudget()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kTripBudgetMs, QStringLiteral("runaway"));

  QJSValue fn = engine.evaluate(kInfiniteLoop);
  QVERIFY(fn.isCallable());

  const QByteArray warning = timeoutWarning(QStringLiteral("runaway"), kTripBudgetMs);
  QTest::ignoreMessage(QtWarningMsg, warning.constData());

  QElapsedTimer timer;
  timer.start();
  const QJSValue result  = watchdog.call(fn, QJSValueList());
  const qint64 elapsedMs = timer.elapsed();

  QVERIFY(watchdog.lastCallTimedOut());
  QVERIFY(elapsedMs >= kTripBudgetMs);
  QVERIFY(result.isError());
  QVERIFY(!engine.isInterrupted());
  QCOMPARE(watchdog.deadlineNs(), qint64(0));
}

/**
 * @brief An interrupt is not terminal for the engine: the same watchdog and the same engine run a
 *        normal call straight afterwards, and lastCallTimedOut() falls back to false.
 */
void JsWatchdogTests::engineStaysUsableAfterAnInterrupt()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kTripBudgetMs, QStringLiteral("recover"));

  QJSValue runaway = engine.evaluate(kInfiniteLoop);
  QVERIFY(runaway.isCallable());

  const QByteArray warning = timeoutWarning(QStringLiteral("recover"), kTripBudgetMs);
  QTest::ignoreMessage(QtWarningMsg, warning.constData());
  const QJSValue stuck = watchdog.call(runaway, QJSValueList());
  QVERIFY(watchdog.lastCallTimedOut());
  QVERIFY(stuck.isError());

  watchdog.setBudgetMs(kGenerousBudgetMs);
  QJSValue fn = engine.evaluate(QStringLiteral("(function(a) { return a + 1; })"));
  QVERIFY(fn.isCallable());

  const QJSValue result = watchdog.call(fn, QJSValueList{QJSValue(41)});

  QVERIFY(!result.isError());
  QCOMPARE(result.toInt(), 42);
  QVERIFY(!watchdog.lastCallTimedOut());
  QVERIFY(!engine.isInterrupted());
}

/**
 * @brief The budget is what decides: a bounded loop completes untouched under a generous budget,
 *        and the same watchdog trips on an unbounded one once the budget is narrowed.
 */
void JsWatchdogTests::tinyBudgetTripsWhereGenerousBudgetDoesNot()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kGenerousBudgetMs, QStringLiteral("budget"));

  QJSValue bounded = engine.evaluate(kBoundedLoop);
  QVERIFY(bounded.isCallable());

  const QJSValue sum = watchdog.call(bounded, QJSValueList{QJSValue(kLoopIterations)});
  QVERIFY(!sum.isError());
  QVERIFY(!watchdog.lastCallTimedOut());

  watchdog.setBudgetMs(kTripBudgetMs);
  QJSValue runaway = engine.evaluate(kInfiniteLoop);
  QVERIFY(runaway.isCallable());

  const QByteArray warning = timeoutWarning(QStringLiteral("budget"), kTripBudgetMs);
  QTest::ignoreMessage(QtWarningMsg, warning.constData());
  const QJSValue stuck = watchdog.call(runaway, QJSValueList());

  QVERIFY(watchdog.lastCallTimedOut());
  QVERIFY(stuck.isError());
}

//--------------------------------------------------------------------------------------------------
// Call surface
//--------------------------------------------------------------------------------------------------

/**
 * @brief The three-argument overload forwards both the receiver and the argument list, so a script
 *        that reads `this` sees the object the caller passed.
 */
void JsWatchdogTests::callWithInstancePassesThisAndArguments()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kGenerousBudgetMs, QStringLiteral("instance"));

  QJSValue fn = engine.evaluate(QStringLiteral("(function(a, b) { return this.base + a * b; })"));
  QVERIFY(fn.isCallable());

  QJSValue self = engine.newObject();
  self.setProperty(QStringLiteral("base"), QJSValue(100));

  const QJSValue result = watchdog.call(fn, self, QJSValueList{QJSValue(3), QJSValue(4)});

  QVERIFY(!result.isError());
  QCOMPARE(result.toInt(), 112);
  QVERIFY(!watchdog.lastCallTimedOut());
  QCOMPARE(watchdog.deadlineNs(), qint64(0));
}

/**
 * @brief A script that throws is a script error, never a timeout: the throw travels back as the
 *        result value, the host survives, and the engine runs the next call normally.
 */
void JsWatchdogTests::thrownExceptionIsAnErrorNotATimeout()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kGenerousBudgetMs, QStringLiteral("throws"));

  QJSValue fn = engine.evaluate(QStringLiteral("(function() { throw new Error('boom'); })"));
  QVERIFY(fn.isCallable());

  const QJSValue result = watchdog.call(fn, QJSValueList());

  QVERIFY(result.isError());
  QCOMPARE(result.property(QStringLiteral("message")).toString(), QStringLiteral("boom"));
  QVERIFY(!watchdog.lastCallTimedOut());
  QVERIFY(!engine.isInterrupted());

  QJSValue ok = engine.evaluate(QStringLiteral("(function() { return 9; })"));
  QVERIFY(ok.isCallable());
  QCOMPARE(watchdog.call(ok, QJSValueList()).toInt(), 9);
}

//--------------------------------------------------------------------------------------------------
// Isolation between instances
//--------------------------------------------------------------------------------------------------

/**
 * @brief The scan set is per watchdog: with two engines registered at once, expiring the runaway
 *        one leaves the other's interrupt flag clear and its engine fully usable.
 */
void JsWatchdogTests::onlyTheExpiredWatchdogIsInterrupted()
{
  QJSEngine runawayEngine;
  QJSEngine calmEngine;
  DataModel::JsWatchdog runawayDog(&runawayEngine, kTripBudgetMs, QStringLiteral("runaway"));
  DataModel::JsWatchdog calmDog(&calmEngine, kGenerousBudgetMs, QStringLiteral("calm"));

  QJSValue loop = runawayEngine.evaluate(kInfiniteLoop);
  QJSValue add  = calmEngine.evaluate(QStringLiteral("(function(a) { return a + 1; })"));
  QVERIFY(loop.isCallable());
  QVERIFY(add.isCallable());

  const QJSValue before = calmDog.call(add, QJSValueList{QJSValue(1)});
  QCOMPARE(before.toInt(), 2);

  const QByteArray warning = timeoutWarning(QStringLiteral("runaway"), kTripBudgetMs);
  QTest::ignoreMessage(QtWarningMsg, warning.constData());
  const QJSValue stuck = runawayDog.call(loop, QJSValueList());

  QVERIFY(runawayDog.lastCallTimedOut());
  QVERIFY(stuck.isError());
  QVERIFY(!calmDog.lastCallTimedOut());
  QVERIFY(!calmEngine.isInterrupted());

  const QJSValue after = calmDog.call(add, QJSValueList{QJSValue(41)});
  QCOMPARE(after.toInt(), 42);
  QVERIFY(!calmDog.lastCallTimedOut());
}

/**
 * @brief arm() followed by disarm() with no call in between retires the deadline, so the worker
 *        never interrupts an engine that is not executing anything.
 */
void JsWatchdogTests::armFollowedByDisarmNeverInterrupts()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kTripBudgetMs, QStringLiteral("idle"));

  watchdog.disarm();
  QCOMPARE(watchdog.deadlineNs(), qint64(0));

  watchdog.arm();
  watchdog.disarm();
  QCOMPARE(watchdog.deadlineNs(), qint64(0));

  QTest::qWait(kTripBudgetMs * 3);
  QVERIFY(!engine.isInterrupted());

  QJSValue fn = engine.evaluate(QStringLiteral("(function() { return 5; })"));
  QVERIFY(fn.isCallable());

  const QJSValue result = watchdog.call(fn, QJSValueList());

  QCOMPARE(result.toInt(), 5);
  QVERIFY(!watchdog.lastCallTimedOut());
}

//--------------------------------------------------------------------------------------------------
// Shutdown -- MUST STAY LAST
//--------------------------------------------------------------------------------------------------

/**
 * @brief shutdown() joins the worker thread and latches the singleton stopped for the rest of the
 *        process: an armed deadline that the live worker demonstrably expires stops being expired
 *        once shutdown() has run, and a second shutdown() is a no-op rather than a double join.
 *        Because that latch is permanent, this slot must remain the last one declared -- Qt Test
 *        runs slots in declaration order, and any slot after it would run without a watchdog.
 */
void JsWatchdogTests::shutdownStopsTheWorkerThread()
{
  QJSEngine engine;
  DataModel::JsWatchdog watchdog(&engine, kTripBudgetMs, QStringLiteral("shutdown"));

  watchdog.arm();
  QTRY_VERIFY_WITH_TIMEOUT(engine.isInterrupted(), kTripBudgetMs * 20);
  watchdog.disarm();

  DataModel::JsWatchdogThread::instance().shutdown();
  DataModel::JsWatchdogThread::instance().shutdown();

  watchdog.arm();
  QTest::qWait(kTripBudgetMs * 4);

  QVERIFY(!engine.isInterrupted());
  watchdog.disarm();
}

QTEST_GUILESS_MAIN(JsWatchdogTests)

#include "tst_js_watchdog.moc"
