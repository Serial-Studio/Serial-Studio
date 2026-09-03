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

#include <QDateTime>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

#include "Licensing/MonotonicClock.h"
#include "Licensing/SimpleCrypt.h"

// Every case drives the injected seam (nowFloored) against its own INI file, so no case can see
// another's floor and the developer's real settings are never touched.

/**
 * @brief Pins the anti-rewind clock the grace period and the offline expiry both read: the floor
 *        never moves backwards, and the store is written at most once a minute. The write rate is
 *        the point of the second half -- now() used to write on every call, and the trial getters
 *        that call it are Q_PROPERTY reads bound in seventeen places (spec 0075, K10).
 */
class TstMonotonicClock : public QObject {
  Q_OBJECT

private slots:
  void returnsWallClockOnAFreshStore();
  void storedFutureFloorWins();
  void repeatedCallsDoNotRewriteTheStore();
  void floorSurvivesAClockRewind();

private:
  QTemporaryDir m_dir;
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the cipher the clock's caller configures, keyed like the real one.
 */
static Licensing::SimpleCrypt makeCrypt()
{
  Licensing::SimpleCrypt crypt(0x0BADC0DE0BADC0DEULL);
  crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
  return crypt;
}

/**
 * @brief Returns the raw stored floor, so a case can tell a rewrite from a skipped write.
 */
static QString storedFloor(QSettings& settings)
{
  settings.beginGroup(QStringLiteral("licensing"));
  const auto value = settings.value(QStringLiteral("lastSeen"), QString()).toString();
  settings.endGroup();
  return value;
}

//--------------------------------------------------------------------------------------------------
// Flooring
//--------------------------------------------------------------------------------------------------

/**
 * @brief With nothing stored, the clock is the wall clock.
 */
void TstMonotonicClock::returnsWallClockOnAFreshStore()
{
  QVERIFY(m_dir.isValid());
  QSettings settings(m_dir.path() + QStringLiteral("/fresh.ini"), QSettings::IniFormat);
  auto crypt = makeCrypt();

  const auto before = QDateTime::currentDateTime().addSecs(-2);
  const auto now    = Licensing::MonotonicClock::nowFloored(settings, crypt);

  QVERIFY(now >= before);
  QVERIFY(now <= QDateTime::currentDateTime().addSecs(2));
}

/**
 * @brief A stored floor in the future wins, which is what stops a rewind from reviving a lapsed
 *        grace period.
 */
void TstMonotonicClock::storedFutureFloorWins()
{
  QVERIFY(m_dir.isValid());
  QSettings settings(m_dir.path() + QStringLiteral("/future.ini"), QSettings::IniFormat);
  auto crypt = makeCrypt();

  const auto future = QDateTime::currentDateTime().addDays(30);
  settings.beginGroup(QStringLiteral("licensing"));
  settings.setValue(QStringLiteral("lastSeen"),
                    crypt.encryptToString(future.toString(Qt::RFC2822Date)));
  settings.endGroup();

  const auto now = Licensing::MonotonicClock::nowFloored(settings, crypt);
  QVERIFY(now >= future.addSecs(-60));
}

/**
 * @brief A rewound wall clock still reads at least the previously recorded floor.
 */
void TstMonotonicClock::floorSurvivesAClockRewind()
{
  QVERIFY(m_dir.isValid());
  QSettings settings(m_dir.path() + QStringLiteral("/rewind.ini"), QSettings::IniFormat);
  auto crypt = makeCrypt();

  const auto seen = QDateTime::currentDateTime().addDays(3);
  settings.beginGroup(QStringLiteral("licensing"));
  settings.setValue(QStringLiteral("lastSeen"),
                    crypt.encryptToString(seen.toString(Qt::RFC2822Date)));
  settings.endGroup();

  const auto first  = Licensing::MonotonicClock::nowFloored(settings, crypt);
  const auto second = Licensing::MonotonicClock::nowFloored(settings, crypt);
  QVERIFY(first >= seen.addSecs(-60));
  QVERIFY(second >= first.addSecs(-60));
}

//--------------------------------------------------------------------------------------------------
// Write rate
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two calls inside the same minute leave the store untouched. The cipher prepends a random
 *        byte, so an unchanged value can only mean the second call skipped the write -- which is
 *        the property that keeps a QML binding evaluation off the settings file (K10).
 */
void TstMonotonicClock::repeatedCallsDoNotRewriteTheStore()
{
  QVERIFY(m_dir.isValid());
  QSettings settings(m_dir.path() + QStringLiteral("/throttle.ini"), QSettings::IniFormat);
  auto crypt = makeCrypt();

  (void)Licensing::MonotonicClock::nowFloored(settings, crypt);
  const auto first = storedFloor(settings);

  (void)Licensing::MonotonicClock::nowFloored(settings, crypt);
  (void)Licensing::MonotonicClock::nowFloored(settings, crypt);
  const auto second = storedFloor(settings);

  QCOMPARE(second, first);
}

QTEST_GUILESS_MAIN(TstMonotonicClock)

#include "tst_monotonic_clock.moc"
