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

#include <QList>
#include <QRect>
#include <QTest>

#include "Platform/KioskSession.h"

// Every test function here is self-contained: the placement rule is a pure function of the
// screen list, so Qt Test's declaration-order execution is never load-bearing.

/**
 * @brief Known-answer coverage of the kiosk placement rule (spec 0125): which screen counts as
 *        leftmost, and that the reserved strip is cut from that screen only. The zero-reserve
 *        rows are the guard for a plain --kiosk, which must get its screen back untouched.
 */
class TstKioskSession : public QObject {
  Q_OBJECT

private slots:
  void leftmostIndex_data();
  void leftmostIndex();

  void insetRect_data();
  void insetRect();
};

//--------------------------------------------------------------------------------------------------
// Leftmost screen
//--------------------------------------------------------------------------------------------------

/**
 * @brief Layouts for the leftmost rule: side by side in both orders, stacked with either screen
 *        primary, a tie the primary is not part of, and no screens at all.
 */
void TstKioskSession::leftmostIndex_data()
{
  QTest::addColumn<QList<QRect>>("screens");
  QTest::addColumn<int>("primary");
  QTest::addColumn<int>("expected");

  const QRect left(0, 0, 1920, 1080);
  const QRect right(1920, 0, 2560, 1440);
  const QRect negative(-2560, 0, 2560, 1440);
  const QRect below(0, 1080, 1920, 1080);

  QTest::newRow("none") << QList<QRect>{} << -1 << -1;
  QTest::newRow("single") << QList<QRect>{left} << 0 << 0;
  QTest::newRow("side by side") << QList<QRect>{left, right} << 0 << 0;
  QTest::newRow("primary on the right") << QList<QRect>{right, left} << 0 << 1;
  QTest::newRow("secondary at negative x") << QList<QRect>{left, negative} << 0 << 1;
  QTest::newRow("stacked, first primary") << QList<QRect>{left, below} << 0 << 0;
  QTest::newRow("stacked, second primary") << QList<QRect>{left, below} << 1 << 1;
  QTest::newRow("tie without the primary") << QList<QRect>{right, left, below} << 0 << 1;
}

/**
 * @brief The smallest left coordinate wins; among equals the primary, else the first listed.
 */
void TstKioskSession::leftmostIndex()
{
  QFETCH(QList<QRect>, screens);
  QFETCH(int, primary);
  QFETCH(int, expected);

  QCOMPARE(Platform::KioskSession::leftmostIndex(screens, primary), expected);
}

//--------------------------------------------------------------------------------------------------
// Reserved strip
//--------------------------------------------------------------------------------------------------

/**
 * @brief Targets on and off the leftmost screen, with no strip, a normal one, a negative one and
 *        one wider than half the screen.
 */
void TstKioskSession::insetRect_data()
{
  QTest::addColumn<QList<QRect>>("screens");
  QTest::addColumn<int>("primary");
  QTest::addColumn<QRect>("target");
  QTest::addColumn<int>("reserve");
  QTest::addColumn<QRect>("expected");

  const QRect left(0, 0, 1920, 1080);
  const QRect right(1920, 0, 2560, 1440);
  const QRect below(0, 1080, 1920, 1080);
  const QList<QRect> pair{left, right};
  const QList<QRect> stacked{left, below};

  QTest::newRow("no strip, leftmost") << pair << 0 << left << 0 << left;
  QTest::newRow("no strip, other") << pair << 0 << right << 0 << right;
  QTest::newRow("negative strip") << pair << 0 << left << -64 << left;
  QTest::newRow("strip on the leftmost") << pair << 0 << left << 64 << QRect(64, 0, 1856, 1080);
  QTest::newRow("strip ignored elsewhere") << pair << 0 << right << 64 << right;
  QTest::newRow("strip capped at half") << pair << 0 << left << 5000 << QRect(960, 0, 960, 1080);
  QTest::newRow("stacked, primary gets it")
    << stacked << 1 << below << 64 << QRect(64, 1080, 1856, 1080);
  QTest::newRow("stacked, the other does not") << stacked << 1 << left << 64 << left;
  QTest::newRow("no screens") << QList<QRect>{} << -1 << left << 64 << left;
}

/**
 * @brief The strip moves the left edge of the leftmost screen only and keeps the right edge.
 */
void TstKioskSession::insetRect()
{
  QFETCH(QList<QRect>, screens);
  QFETCH(int, primary);
  QFETCH(QRect, target);
  QFETCH(int, reserve);
  QFETCH(QRect, expected);

  QCOMPARE(Platform::KioskSession::insetRect(screens, primary, target, reserve), expected);
}

QTEST_APPLESS_MAIN(TstKioskSession)

#include "tst_kiosk_session.moc"
