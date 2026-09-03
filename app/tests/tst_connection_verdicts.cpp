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

#include <QSignalSpy>
#include <QTest>

#include "IO/ConnectionManager/ConnectFanOut.h"
#include "support/FakeDriver.h"

// IO::ConnectionManager itself cannot be linked into a unit suite -- its link set is the whole
// application -- so the Manager below reproduces the five lines of
// connectDevice()/disconnectDevice() that decide a verdict, driven against the same ConnectFanOut
// and HAL_Driver latch the production path uses. What is pinned here is the composition: exactly
// ONE verdict per attempt, on both outcomes, and never one for an attempt the user cancelled.

/**
 * @brief The manager side of the spec-0050 verdict contract, reduced to what it actually does:
 *        arm the latch, open, either settle now or record a pending dial, and report exactly once.
 */
class Manager : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Wires the driver's openFinished() the way ConnectionManager wires a live driver.
   */
  explicit Manager(Test::FakeDriver& driver, QObject* parent = nullptr)
    : QObject(parent), m_driver(driver), m_verdicts(0), m_lastOk(false), m_paused(false)
  {
    connect(&m_driver, &IO::HAL_Driver::openFinished, this, &Manager::onOpenFinished);
  }

  /**
   * @brief The user's connect: resumes the session pause, arms the latch, opens, and either notes
   *        a pending dial or settles the verdict on the spot.
   */
  void connectDevice() { open(true); }

  /**
   * @brief A driver's own auto-reconnect: identical, except that the pause survives it.
   */
  void reconnectDevice() { open(false); }

  /**
   * @brief The user's disconnect: drops the pending verdict (a cancel is not a failure), disarms
   *        the latch so a late driver report cannot masquerade as one, and closes.
   */
  void disconnectDevice()
  {
    (void)m_fanOut.takePendingDial(0);
    m_driver.disarmOpenReport();
    m_driver.close();
  }

  [[nodiscard]] int verdicts() const noexcept { return m_verdicts; }

  [[nodiscard]] bool lastOk() const noexcept { return m_lastOk; }

  [[nodiscard]] bool paused() const noexcept { return m_paused; }

  void setPaused(bool paused) noexcept { m_paused = paused; }

signals:
  void settled(bool ok);

private slots:

  /**
   * @brief The driver reported: the pending id is claimed first, so the report can settle the
   *        attempt exactly once and a report with no pending id (already cancelled) is ignored.
   */
  void onOpenFinished(bool ok, const QString& reason)
  {
    Q_UNUSED(reason)

    if (!m_fanOut.takePendingDial(0))
      return;

    settle(ok);
  }

private:
  /**
   * @brief Shared open path; @p resume is the ResumePolicy the production manager carries.
   */
  void open(bool resume)
  {
    m_driver.armOpenReport();
    const bool started = m_driver.open(QIODevice::ReadWrite);
    if (resume)
      m_paused = false;

    if (started && m_driver.isConnecting()) {
      m_fanOut.notePendingDial(0);
      return;
    }

    m_driver.disarmOpenReport();
    settle(started);
  }

  /**
   * @brief Records one verdict.
   */
  void settle(bool ok)
  {
    ++m_verdicts;
    m_lastOk = ok;
    Q_EMIT settled(ok);
  }

private:
  Test::FakeDriver& m_driver;
  IO::ConnectFanOut m_fanOut;
  int m_verdicts;
  bool m_lastOk;
  bool m_paused;
};

/**
 * @brief The verdict matrix: synchronous success and failure, asynchronous success and failure,
 *        cancel mid-dial, a drop with a pending dial, and a reconnect that keeps the pause.
 */
class TstConnectionVerdicts : public QObject {
  Q_OBJECT

private slots:
  void syncSuccessSettlesInsideOpen();
  void syncFailureSettlesInsideOpen();
  void asyncSuccessSettlesOnce();
  void asyncFailureSettlesOnce();
  void cancelMidDialReportsNothing();
  void dropWithPendingDialReportsOnce();
  void reconnectKeepsThePause();
  void userConnectResumesThePause();
};

//--------------------------------------------------------------------------------------------------
// Synchronous verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief A driver that settles inside open() is reported by the return value, and the latch is
 *        disarmed so nothing else can report the same attempt.
 */
void TstConnectionVerdicts::syncSuccessSettlesInsideOpen()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncOk);

  Manager manager(driver);
  manager.connectDevice();

  QCOMPARE(manager.verdicts(), 1);
  QVERIFY(manager.lastOk());
  QVERIFY(!driver.openReportArmed());
  QCOMPARE(driver.openCalls(), 1);
}

/**
 * @brief The same for a refusal: one verdict, reported false, no pending dial left behind.
 */
void TstConnectionVerdicts::syncFailureSettlesInsideOpen()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncFail);

  Manager manager(driver);
  manager.connectDevice();

  QCOMPARE(manager.verdicts(), 1);
  QVERIFY(!manager.lastOk());
  QVERIFY(!driver.openReportArmed());
}

//--------------------------------------------------------------------------------------------------
// Asynchronous verdicts
//--------------------------------------------------------------------------------------------------

/**
 * @brief An asynchronous dial reports through openFinished() exactly once; open() returning true
 *        means the attempt started, not that the link is up.
 */
void TstConnectionVerdicts::asyncSuccessSettlesOnce()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::AsyncOk);
  driver.setAsyncDelayMs(10);

  Manager manager(driver);
  QSignalSpy spy(&manager, &Manager::settled);

  manager.connectDevice();
  QCOMPARE(manager.verdicts(), 0);
  QVERIFY(driver.isConnecting());

  QVERIFY(spy.wait(2000));
  QCOMPARE(manager.verdicts(), 1);
  QVERIFY(manager.lastOk());

  QTest::qWait(50);
  QCOMPARE(manager.verdicts(), 1);
}

/**
 * @brief A failed asynchronous dial is the bug class spec 0050 exists to kill: it must report,
 *        once, instead of leaving the connect button wedged on "Connecting".
 */
void TstConnectionVerdicts::asyncFailureSettlesOnce()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::AsyncFail);
  driver.setAsyncDelayMs(10);

  Manager manager(driver);
  QSignalSpy spy(&manager, &Manager::settled);

  manager.connectDevice();
  QVERIFY(driver.isConnecting());

  QVERIFY(spy.wait(2000));
  QCOMPARE(manager.verdicts(), 1);
  QVERIFY(!manager.lastOk());
  QVERIFY(!driver.isConnecting());
}

//--------------------------------------------------------------------------------------------------
// Cancellation and drops
//--------------------------------------------------------------------------------------------------

/**
 * @brief A user who cancels mid-dial gets no verdict at all: the pending id is dropped and the
 *        latch disarmed, so a driver report landing afterwards is ignored.
 */
void TstConnectionVerdicts::cancelMidDialReportsNothing()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::AsyncOk);
  driver.setAsyncDelayMs(40);

  Manager manager(driver);
  manager.connectDevice();
  QVERIFY(driver.isConnecting());

  manager.disconnectDevice();

  QTest::qWait(200);
  QCOMPARE(manager.verdicts(), 0);
}

/**
 * @brief A link that drops while a dial is pending settles that dial as failed, and only once.
 */
void TstConnectionVerdicts::dropWithPendingDialReportsOnce()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::AsyncFail);
  driver.setAsyncDelayMs(10);

  Manager manager(driver);
  QSignalSpy spy(&manager, &Manager::settled);

  manager.connectDevice();
  driver.dropLink();

  QVERIFY(spy.wait(2000) || manager.verdicts() == 1);
  QCOMPARE(manager.verdicts(), 1);
  QVERIFY(!manager.lastOk());
}

//--------------------------------------------------------------------------------------------------
// Pause policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief A driver's auto-reconnect keeps the session pause: an adapter blip silently resuming a
 *        paused multi-source session is what the ResumePolicy split exists to prevent.
 */
void TstConnectionVerdicts::reconnectKeepsThePause()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncOk);

  Manager manager(driver);
  manager.setPaused(true);
  manager.reconnectDevice();

  QVERIFY(manager.paused());
  QCOMPARE(manager.verdicts(), 1);
}

/**
 * @brief The user's own connect does resume it, which is the half of the split that must not
 *        regress while fixing the other.
 */
void TstConnectionVerdicts::userConnectResumesThePause()
{
  Test::FakeDriver driver;
  driver.setOpenOutcome(Test::FakeDriver::Outcome::SyncOk);

  Manager manager(driver);
  manager.setPaused(true);
  manager.connectDevice();

  QVERIFY(!manager.paused());
}

QTEST_MAIN(TstConnectionVerdicts)

#include "tst_connection_verdicts.moc"
