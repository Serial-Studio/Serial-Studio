/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <memory>
#include <QByteArray>
#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "Console/BusMonitor.h"

// The bus monitor's message store (spec 0127 R11, R15-R17, AC8): nothing is ingested without a
// view, Messages mode splits received bytes by the idle gap and keeps sends in order, the store
// is capped while its counters are not, pause and clear behave, a Modbus timeout lands on the
// row of its request whether that row is still staged or already published, and the table
// filter and timeline read the same rows. Settings go to a temporary INI folder.

using Console::BusMonitor;
using Console::BusRole;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief @p body with its Modbus CRC appended, low byte first.
 */
[[nodiscard]] static QByteArray seal(QByteArray body)
{
  quint16 crc = 0xFFFF;
  for (const char c : std::as_const(body)) {
    crc ^= static_cast<quint8>(c);
    for (int bit = 0; bit < 8; ++bit)
      crc =
        (crc & 1) != 0 ? static_cast<quint16>((crc >> 1) ^ 0xA001) : static_cast<quint16>(crc >> 1);
  }

  body.append(static_cast<char>(crc & 0xFF));
  body.append(static_cast<char>(crc >> 8));
  return body;
}

/**
 * @brief A read-holding-registers request for one register of @p slave.
 */
[[nodiscard]] static QByteArray request(const int slave)
{
  QByteArray body;
  for (const int value : {slave, 3, 0, 0, 0, 1})
    body.append(static_cast<char>(value));

  return seal(body);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class BusMonitorTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void unwatchedIngestsNothing();
  void receivedBytesSplitByTheIdleGap();
  void sendIsOneMessageAndKeepsItsPlace();
  void storeIsCappedButCountersAreNot();
  void pauseDiscardsAndClearResets();
  void timeoutLandsOnAPublishedRow();
  void timeoutLandsOnAStagedRow();
  void filterKeepsOnlyIssues();
  void timelineBarsFollowTheRows();
  void settingsSurviveARestart();
  void destroyedViewerStopsIngestion();

private:
  QTemporaryDir m_settingsDir;
  QObject m_viewer;
};

/**
 * @brief Routes QSettings to a temporary INI folder and takes the SS_ASSERT recovery branch.
 */
void BusMonitorTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
  QVERIFY(m_settingsDir.isValid());
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_bus_monitor"));
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
}

/**
 * @brief Every case starts with default settings.
 */
void BusMonitorTest::init()
{
  QSettings settings;
  settings.clear();
  settings.sync();
}

/**
 * @brief With no panel on screen the monitor keeps nothing, in either mode.
 */
void BusMonitorTest::unwatchedIngestsNothing()
{
  BusMonitor monitor;
  QVERIFY(!monitor.watched());
  monitor.feedReceived(QByteArray("hello"), 0);
  monitor.feedSent(QByteArray("world"), 5);
  monitor.setMode(BusMonitor::ModbusRtu);
  monitor.feedReceived(request(1), 10);
  monitor.commitPending(1000);

  QCOMPARE(monitor.count(), 0);
  QCOMPARE(monitor.frameTotal(), 0);
}

/**
 * @brief Chunks closer together than the gap are one message, stamped with its first byte; a
 *        chunk after the gap starts the next, which is published once the line has been idle.
 */
void BusMonitorTest::receivedBytesSplitByTheIdleGap()
{
  BusMonitor monitor;
  monitor.setViewerActive(&m_viewer, true);
  monitor.setGapMs(10);

  monitor.feedReceived(QByteArray("AB"), 100);
  monitor.feedReceived(QByteArray("CD"), 105);
  monitor.feedReceived(QByteArray("EF"), 130);
  monitor.commitPending(131);
  QCOMPARE(monitor.count(), 1);
  QCOMPARE(monitor.at(0).bytes, QByteArray("ABCD"));
  QCOMPARE(monitor.at(0).role, BusRole::Received);
  QCOMPARE(monitor.at(0).timeMs, qint64(100));

  monitor.commitPending(145);
  QCOMPARE(monitor.count(), 2);
  QCOMPARE(monitor.at(1).bytes, QByteArray("EF"));
  QCOMPARE(monitor.at(1).timeMs, qint64(130));
  QCOMPARE(monitor.frameTotal(), 2);
  QCOMPARE(monitor.errorTotal(), 0);
  QCOMPARE(monitor.lastTimeMs(), qint64(130));
  QCOMPARE(monitor.averageIntervalMs(), 30.0);
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief A send closes the received message before it, so the table reads in wire order.
 */
void BusMonitorTest::sendIsOneMessageAndKeepsItsPlace()
{
  BusMonitor monitor;
  monitor.setViewerActive(&m_viewer, true);

  monitor.feedReceived(QByteArray("READY"), 0);
  monitor.feedSent(QByteArray("GO\n"), 3);
  monitor.feedReceived(QByteArray("OK"), 4);
  monitor.commitPending(100);

  QCOMPARE(monitor.count(), 3);
  QCOMPARE(monitor.at(0).role, BusRole::Received);
  QCOMPARE(monitor.at(0).bytes, QByteArray("READY"));
  QCOMPARE(monitor.at(1).role, BusRole::Sent);
  QCOMPARE(monitor.at(1).bytes, QByteArray("GO\n"));
  QCOMPARE(monitor.at(2).role, BusRole::Received);
  QVERIFY(monitor.at(0).sequence < monitor.at(1).sequence);
  QVERIFY(monitor.at(1).sequence < monitor.at(2).sequence);
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief The store keeps the newest 5000 messages; the totals keep counting past it.
 */
void BusMonitorTest::storeIsCappedButCountersAreNot()
{
  BusMonitor monitor;
  monitor.setViewerActive(&m_viewer, true);

  const int total = Console::kBusMonitorMaxMessages + 100;
  for (int i = 0; i < total; ++i) {
    monitor.feedSent(QByteArray::number(i), i);
    if (i % 500 == 0)
      monitor.commitPending(i);
  }

  monitor.commitPending(total);
  QCOMPARE(monitor.count(), Console::kBusMonitorMaxMessages);
  QCOMPARE(monitor.rowCount(), Console::kBusMonitorMaxMessages);
  QCOMPARE(monitor.frameTotal(), total);
  QCOMPARE(monitor.at(0).bytes, QByteArray::number(100));
  QCOMPARE(monitor.at(monitor.count() - 1).bytes, QByteArray::number(total - 1));
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief Paused, data is dropped and not counted; clear empties the table and the totals.
 */
void BusMonitorTest::pauseDiscardsAndClearResets()
{
  BusMonitor monitor;
  monitor.setViewerActive(&m_viewer, true);

  monitor.feedSent(QByteArray("one"), 0);
  monitor.setPaused(true);
  monitor.feedSent(QByteArray("two"), 10);
  monitor.feedReceived(QByteArray("three"), 20);
  monitor.setPaused(false);
  monitor.feedSent(QByteArray("four"), 30);
  monitor.commitPending(100);

  QCOMPARE(monitor.count(), 2);
  QCOMPARE(monitor.frameTotal(), 2);
  QCOMPARE(monitor.at(1).bytes, QByteArray("four"));

  QSignalSpy counted(&monitor, &BusMonitor::countChanged);
  monitor.clear();
  QCOMPARE(monitor.count(), 0);
  QCOMPARE(monitor.frameTotal(), 0);
  QCOMPARE(counted.count(), 1);
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief A request that is already in the table gets its "no response" verdict in place: the
 *        row turns into an error and the error total goes up, with no new row.
 */
void BusMonitorTest::timeoutLandsOnAPublishedRow()
{
  BusMonitor monitor;
  monitor.setMode(BusMonitor::ModbusRtu);
  monitor.setViewerActive(&m_viewer, true);

  monitor.feedReceived(request(7), 0);
  monitor.commitPending(10);
  QCOMPARE(monitor.count(), 1);
  QCOMPARE(monitor.at(0).role, BusRole::Master);
  QCOMPARE(monitor.errorTotal(), 0);

  QSignalSpy changed(&monitor, &BusMonitor::dataChanged);
  monitor.commitPending(Console::kModbusTimeoutDefaultMs);
  QCOMPARE(monitor.count(), 1);
  QCOMPARE(changed.count(), 1);
  QCOMPARE(monitor.errorTotal(), 1);
  QCOMPARE(monitor.frameTotal(), 1);

  const auto severity = monitor.data(monitor.index(0, 0), BusMonitor::SeverityRole).toInt();
  QCOMPARE(severity, static_cast<int>(Console::BusSeverity::Error));
  const auto cell = [&monitor](const int column) {
    return monitor.data(monitor.index(0, column), Qt::DisplayRole).toString();
  };

  QVERIFY(cell(BusMonitor::DiagnosisColumn).contains(QLatin1Char('7')));
  QCOMPARE(cell(BusMonitor::SlaveColumn), QStringLiteral("7"));
  QCOMPARE(cell(BusMonitor::CrcColumn), QStringLiteral("OK"));
  QCOMPARE(cell(BusMonitor::LengthColumn), QStringLiteral("8"));
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief Two requests in one chunk: the first is closed as unanswered while it is still staged,
 *        and a burst of them keeps each verdict on its own row.
 */
void BusMonitorTest::timeoutLandsOnAStagedRow()
{
  BusMonitor monitor;
  monitor.setMode(BusMonitor::ModbusRtu);
  monitor.setViewerActive(&m_viewer, true);

  monitor.feedReceived(request(1) + request(2) + request(3), 0);
  monitor.commitPending(10);
  QCOMPARE(monitor.count(), 3);
  QCOMPARE(monitor.errorTotal(), 2);
  QVERIFY((monitor.at(0).findings & Console::FindingNoResponse) != 0);
  QVERIFY((monitor.at(1).findings & Console::FindingNoResponse) != 0);
  QCOMPARE(monitor.at(2).findings, quint16(Console::FindingNone));

  monitor.commitPending(Console::kModbusTimeoutDefaultMs + 10);
  QCOMPARE(monitor.errorTotal(), 3);
  QVERIFY((monitor.at(2).findings & Console::FindingNoResponse) != 0);
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief With "issues only" the filter shows the rows with a warning or an error, and a row that
 *        gains a finding later joins them.
 */
void BusMonitorTest::filterKeepsOnlyIssues()
{
  BusMonitor monitor;
  Console::BusMonitorFilter filter(&monitor);
  monitor.setMode(BusMonitor::ModbusRtu);
  monitor.setViewerActive(&m_viewer, true);

  QByteArray reply;
  for (const int value : {1, 3, 2, 0, 9})
    reply.append(static_cast<char>(value));

  monitor.feedReceived(request(1), 0);
  monitor.feedReceived(seal(reply), 20);
  monitor.feedReceived(request(2), 100);
  monitor.commitPending(110);
  QCOMPARE(monitor.count(), 3);
  QCOMPARE(filter.rowCount(), 3);

  monitor.setIssuesOnly(true);
  QCOMPARE(filter.rowCount(), 0);

  monitor.commitPending(100 + Console::kModbusTimeoutDefaultMs);
  QCOMPARE(filter.rowCount(), 1);

  monitor.setIssuesOnly(false);
  QCOMPARE(filter.rowCount(), 3);
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief The timeline returns four numbers per message inside the window: x scaled to the
 *        canvas, lane 0 for sends and requests, the severity and the row.
 */
void BusMonitorTest::timelineBarsFollowTheRows()
{
  BusMonitor monitor;
  monitor.setViewerActive(&m_viewer, true);

  monitor.feedSent(QByteArray("a"), 0);
  monitor.feedReceived(QByteArray("b"), 500);
  monitor.feedSent(QByteArray("c"), 1000);
  monitor.commitPending(2000);
  QCOMPARE(monitor.count(), 3);

  const auto bars = monitor.timelineBars(0, 1000, 200.0);
  QCOMPARE(bars.size(), 12);
  QCOMPARE(bars.at(0).toDouble(), 0.0);
  QCOMPARE(bars.at(1).toInt(), 0);
  QCOMPARE(bars.at(3).toInt(), 0);
  QCOMPARE(bars.at(4).toDouble(), 100.0);
  QCOMPARE(bars.at(5).toInt(), 1);
  QCOMPARE(bars.at(7).toInt(), 1);
  QCOMPARE(bars.at(8).toDouble(), 200.0);

  QCOMPARE(monitor.timelineBars(400, 600, 100.0).size(), 4);
  QVERIFY(monitor.timelineBars(600, 400, 100.0).isEmpty());
  QVERIFY(monitor.timelineBars(0, 1000, 0.0).isEmpty());

  const auto info = monitor.messageInfo(1);
  QCOMPARE(info.value(QStringLiteral("length")).toInt(), 1);
  QCOMPARE(info.value(QStringLiteral("raw")).toString(), QStringLiteral("62"));
  QVERIFY(monitor.messageInfo(99).isEmpty());
  monitor.setViewerActive(&m_viewer, false);
}

/**
 * @brief Mode, timeout, gap, formats and the filter come back in a new monitor; pause does not.
 */
void BusMonitorTest::settingsSurviveARestart()
{
  {
    BusMonitor first;
    first.setMode(BusMonitor::ModbusRtu);
    first.setTimeoutMs(250);
    first.setGapMs(40);
    first.setFormats(Console::FormatS16 | Console::FormatF32);
    first.setIssuesOnly(true);
    first.setPaused(true);
  }

  BusMonitor second;
  QCOMPARE(second.mode(), static_cast<int>(BusMonitor::ModbusRtu));
  QCOMPARE(second.timeoutMs(), 250);
  QCOMPARE(second.gapMs(), 40);
  QCOMPARE(second.formats(), static_cast<int>(Console::FormatS16 | Console::FormatF32));
  QVERIFY(second.issuesOnly());
  QVERIFY(!second.paused());

  second.setGapMs(0);
  QCOMPARE(second.gapMs(), Console::kBusMonitorGapMinMs);
  second.setTimeoutMs(99999);
  QCOMPARE(second.timeoutMs(), Console::kModbusTimeoutMaxMs);
}

/**
 * @brief A view destroyed while registered stops counting as a viewer, so the monitor goes back
 *        to ingesting nothing.
 */
void BusMonitorTest::destroyedViewerStopsIngestion()
{
  BusMonitor monitor;
  auto viewer = std::make_unique<QObject>();
  monitor.setViewerActive(viewer.get(), true);
  monitor.setViewerActive(viewer.get(), true);
  QVERIFY(monitor.watched());

  viewer.reset();
  QVERIFY(!monitor.watched());
  monitor.feedSent(QByteArray("late"), 0);
  monitor.commitPending(100);
  QCOMPARE(monitor.count(), 0);

  monitor.setViewerActive(nullptr, true);
  QVERIFY(!monitor.watched());
}

QTEST_GUILESS_MAIN(BusMonitorTest)

#include "tst_bus_monitor.moc"
