/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QTest>

#include "UI/Widgets/Terminal/TerminalSearch.h"

/**
 * @brief Pins Widgets::TerminalSearch: query/match bookkeeping over caller-supplied lines,
 *        wrap-around navigation, and index clamping after the buffer shrinks.
 */
class TstTerminalSearch : public QObject {
  Q_OBJECT

private slots:
  void idleStateHasNothingSelected();
  void setQueryReportsChange();
  void refreshFindsMatches_data();
  void refreshFindsMatches();
  void overlappingMatchesAdvancePastEachHit();
  void caseSensitivitySelectsMatchSet();
  void navigationWrapsBothWays();
  void refreshClampsCurrentAfterShrink();
  void emptyQueryRefreshClearsSelection();
  void clearReportsWhetherAnythingDropped();
  void markDirtyIsStickyUntilRefresh();
};

/**
 * @brief A default-built search is inactive, matchless, and points at nothing.
 */
void TstTerminalSearch::idleStateHasNothingSelected()
{
  Widgets::TerminalSearch search;
  QVERIFY(!search.active());
  QVERIFY(!search.dirty());
  QCOMPARE(search.matchCount(), 0);
  QCOMPARE(search.currentIndex(), -1);
  QCOMPARE(search.currentRow(), -1);
  QCOMPARE(search.currentMatchNumber(), 0);
}

/**
 * @brief setQuery() returns true only when the query or case mode actually changed.
 */
void TstTerminalSearch::setQueryReportsChange()
{
  Widgets::TerminalSearch search;
  QVERIFY(search.setQuery(QStringLiteral("abc"), false));
  QVERIFY(!search.setQuery(QStringLiteral("abc"), false));
  QVERIFY(search.setQuery(QStringLiteral("abc"), true));
  QVERIFY(search.setQuery(QStringLiteral("abcd"), true));
  QVERIFY(search.active());
}

void TstTerminalSearch::refreshFindsMatches_data()
{
  QTest::addColumn<QString>("query");
  QTest::addColumn<QStringList>("lines");
  QTest::addColumn<int>("expectedCount");
  QTest::addColumn<int>("firstRow");

  QTest::newRow("single hit") << QStringLiteral("err")
                              << QStringList{QStringLiteral("no error here")} << 1 << 0;
  QTest::newRow("hit per line") << QStringLiteral(
    "ok") << QStringList{QStringLiteral("ok"), QStringLiteral("still ok"), QStringLiteral("nope")}
                                << 2 << 0;
  QTest::newRow("multiple hits one line")
    << QStringLiteral("ab") << QStringList{QStringLiteral("ab ab ab")} << 3 << 0;
  QTest::newRow("no hits") << QStringLiteral("xyz")
                           << QStringList{QStringLiteral("abc"), QStringLiteral("def")} << 0 << -1;
  QTest::newRow("empty lines") << QStringLiteral("a") << QStringList{QString(), QString()} << 0
                               << -1;
  QTest::newRow("unicode") << QStringLiteral("°C")
                           << QStringList{QStringLiteral("21.5 °C"), QStringLiteral("22 °C")} << 2
                           << 0;
}

/**
 * @brief refresh() scans the supplied lines and orders matches by row.
 */
void TstTerminalSearch::refreshFindsMatches()
{
  QFETCH(QString, query);
  QFETCH(QStringList, lines);
  QFETCH(int, expectedCount);
  QFETCH(int, firstRow);

  Widgets::TerminalSearch search;
  (void)search.setQuery(query, false);
  search.refresh(lines);

  QVERIFY(!search.dirty());
  QCOMPARE(search.matchCount(), expectedCount);
  QCOMPARE(search.currentRow() >= 0 ? search.matches().first().y() : -1, firstRow);
}

/**
 * @brief The scan advances past each hit by the query length, so "aaaa" holds two "aa"
 *        matches, not three overlapping ones.
 */
void TstTerminalSearch::overlappingMatchesAdvancePastEachHit()
{
  Widgets::TerminalSearch search;
  (void)search.setQuery(QStringLiteral("aa"), false);
  search.refresh(QStringList{QStringLiteral("aaaa")});
  QCOMPARE(search.matchCount(), 2);
  QCOMPARE(search.matches().at(0), QPoint(0, 0));
  QCOMPARE(search.matches().at(1), QPoint(2, 0));
}

/**
 * @brief Case-sensitive and insensitive modes select different match sets over the same
 *        buffer.
 */
void TstTerminalSearch::caseSensitivitySelectsMatchSet()
{
  const QStringList lines{QStringLiteral("Error error ERROR")};

  Widgets::TerminalSearch search;
  (void)search.setQuery(QStringLiteral("error"), false);
  search.refresh(lines);
  QCOMPARE(search.matchCount(), 3);

  (void)search.setQuery(QStringLiteral("error"), true);
  search.refresh(lines);
  QCOMPARE(search.matchCount(), 1);
  QCOMPARE(search.matches().first(), QPoint(6, 0));
}

/**
 * @brief next()/previous() wrap around both ends and report false with no matches.
 */
void TstTerminalSearch::navigationWrapsBothWays()
{
  Widgets::TerminalSearch search;
  QVERIFY(!search.next());
  QVERIFY(!search.previous());

  (void)search.setQuery(QStringLiteral("x"), false);
  search.refresh(QStringList{QStringLiteral("x x x")});
  QCOMPARE(search.matchCount(), 3);
  QCOMPARE(search.currentIndex(), 0);

  QVERIFY(search.next());
  QCOMPARE(search.currentIndex(), 1);
  QVERIFY(search.next());
  QVERIFY(search.next());
  QCOMPARE(search.currentIndex(), 0);

  QVERIFY(search.previous());
  QCOMPARE(search.currentIndex(), 2);
  QCOMPARE(search.currentMatchNumber(), 3);
}

/**
 * @brief A rescan over a shrunken buffer clamps the current index instead of leaving it
 *        past the end.
 */
void TstTerminalSearch::refreshClampsCurrentAfterShrink()
{
  Widgets::TerminalSearch search;
  (void)search.setQuery(QStringLiteral("x"), false);
  search.refresh(QStringList{QStringLiteral("x"), QStringLiteral("x"), QStringLiteral("x")});
  QVERIFY(search.next());
  QVERIFY(search.next());
  QCOMPARE(search.currentIndex(), 2);

  search.refresh(QStringList{QStringLiteral("x")});
  QCOMPARE(search.matchCount(), 1);
  QCOMPARE(search.currentIndex(), 0);
  QCOMPARE(search.currentRow(), 0);
}

/**
 * @brief Refreshing with no query active drops the selection but keeps the object clean.
 */
void TstTerminalSearch::emptyQueryRefreshClearsSelection()
{
  Widgets::TerminalSearch search;
  search.markDirty();
  search.refresh(QStringList{QStringLiteral("anything")});
  QVERIFY(!search.dirty());
  QCOMPARE(search.matchCount(), 0);
  QCOMPARE(search.currentIndex(), -1);
}

/**
 * @brief clear() reports whether a query or match list actually existed.
 */
void TstTerminalSearch::clearReportsWhetherAnythingDropped()
{
  Widgets::TerminalSearch search;
  QVERIFY(!search.clear());

  (void)search.setQuery(QStringLiteral("q"), false);
  search.refresh(QStringList{QStringLiteral("q")});
  QVERIFY(search.clear());
  QVERIFY(!search.active());
  QCOMPARE(search.matchCount(), 0);
  QVERIFY(!search.clear());
}

/**
 * @brief markDirty() stays set until the next refresh().
 */
void TstTerminalSearch::markDirtyIsStickyUntilRefresh()
{
  Widgets::TerminalSearch search;
  (void)search.setQuery(QStringLiteral("q"), false);
  search.markDirty();
  QVERIFY(search.dirty());
  search.markDirty();
  QVERIFY(search.dirty());
  search.refresh(QStringList{});
  QVERIFY(!search.dirty());
}

QTEST_APPLESS_MAIN(TstTerminalSearch)

#include "tst_terminal_search.moc"
