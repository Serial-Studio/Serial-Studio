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

#include <limits>
#include <QTest>

#include "UI/AlarmDeadband.h"

//--------------------------------------------------------------------------------------------------
// Fixture: a Warning band [80, 90] with severity 2, a Critical band above it with severity 3,
// nothing below 80 (candidate severity -1).
//--------------------------------------------------------------------------------------------------

static constexpr double kMin      = 80.0;
static constexpr double kMax      = 90.0;
static constexpr int kWarning     = 2;
static constexpr int kCritical    = 3;
static constexpr int kNoBand      = -1;
static constexpr double kDeadband = 1.0;
static constexpr double kNaN      = std::numeric_limits<double>::quiet_NaN();

class AlarmDeadbandTests : public QObject {
  Q_OBJECT

private slots:
  void rule_data();
  void rule();
  void ruleIsConstexpr();
};

void AlarmDeadbandTests::rule_data()
{
  QTest::addColumn<double>("value");
  QTest::addColumn<int>("candidateSeverity");
  QTest::addColumn<double>("deadband");
  QTest::addColumn<bool>("holds");

  QTest::newRow("no deadband: below edge releases") << 79.5 << kNoBand << 0.0 << false;
  QTest::newRow("no deadband: above edge releases") << 90.5 << kCritical << 0.0 << false;
  QTest::newRow("negative deadband is none") << 79.5 << kNoBand << -1.0 << false;
  QTest::newRow("NaN deadband is none") << 79.5 << kNoBand << kNaN << false;

  QTest::newRow("return to normal inside deadband holds") << 79.5 << kNoBand << kDeadband << true;
  QTest::newRow("return to normal at deadband edge holds") << 79.0 << kNoBand << kDeadband << true;
  QTest::newRow("return to normal past deadband releases") << 78.9 << kNoBand << kDeadband << false;
  QTest::newRow("far below releases") << 0.0 << kNoBand << kDeadband << false;

  QTest::newRow("escalation inside deadband never holds")
    << 90.5 << kCritical << kDeadband << false;
  QTest::newRow("escalation past deadband never holds") << 95.0 << kCritical << kDeadband << false;

  QTest::newRow("sibling of equal severity inside deadband holds")
    << 90.5 << kWarning << kDeadband << true;
  QTest::newRow("sibling of equal severity past deadband releases")
    << 91.5 << kWarning << kDeadband << false;

  QTest::newRow("de-escalation inside deadband holds") << 79.5 << 1 << kDeadband << true;
  QTest::newRow("de-escalation past deadband releases") << 78.0 << 1 << kDeadband << false;

  QTest::newRow("still inside the band holds") << 85.0 << kWarning << kDeadband << true;
  QTest::newRow("NaN value never holds") << kNaN << kNoBand << kDeadband << false;
}

void AlarmDeadbandTests::rule()
{
  QFETCH(double, value);
  QFETCH(int, candidateSeverity);
  QFETCH(double, deadband);
  QFETCH(bool, holds);

  QCOMPARE(
    UI::AlarmDeadband::holdsActiveBand(value, kMin, kMax, kWarning, candidateSeverity, deadband),
    holds);
}

void AlarmDeadbandTests::ruleIsConstexpr()
{
  static_assert(UI::AlarmDeadband::holdsActiveBand(79.5, kMin, kMax, kWarning, kNoBand, kDeadband));
  static_assert(
    !UI::AlarmDeadband::holdsActiveBand(90.5, kMin, kMax, kWarning, kCritical, kDeadband));
  static_assert(!UI::AlarmDeadband::holdsActiveBand(79.5, kMin, kMax, kWarning, kNoBand, 0.0));
  QVERIFY(true);
}

QTEST_APPLESS_MAIN(AlarmDeadbandTests)
#include "tst_alarm_deadband.moc"
