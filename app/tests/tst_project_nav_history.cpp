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

#include <QSet>
#include <QString>
#include <QTest>

#include "DataModel/Project/ProjectNavHistory.h"

// Every test function here builds its own ProjectNavHistory: no state is carried between slots, so
// Qt Test's declaration-order execution is never load-bearing. Out-of-range setCursor()/entryAt()
// paths are deliberately untested: they are SS_ASSERT guards, which abort in a debug build.

using Entry = DataModel::ProjectNavHistory::Entry;

Q_DECLARE_METATYPE(DataModel::ProjectNavHistory::Entry)

/**
 * @brief Builds an entity entry (kind/id/parentId identity).
 */
static Entry entity(int kind, int id, int parentId = -1, const QString& key = QString())
{
  Entry entry;
  entry.valid    = true;
  entry.kind     = kind;
  entry.id       = id;
  entry.parentId = parentId;
  entry.key      = key;
  return entry;
}

/**
 * @brief Builds a container entry (view identity).
 */
static Entry container(int view)
{
  Entry entry;
  entry.valid     = true;
  entry.container = true;
  entry.view      = view;
  return entry;
}

/**
 * @brief Ring/cursor semantics of the Project Editor's back/forward history.
 */
class TstProjectNavHistory : public QObject {
  Q_OBJECT

private slots:
  void emptyHistoryHasNoCursor();
  void pushRecordsAndAdvancesCursor();
  void pushRejectsInvalidEntries();
  void pushDedupesAgainstCursor();
  void pushAllowsRepeatAfterAnotherTarget();
  void pushTruncatesForwardTail();
  void pushCapsAtMaxEntries();
  void clearResetsAndReportsChange();
  void sameTargetSeparatesKinds_data();
  void sameTargetSeparatesKinds();
  void previousResolvableSkipsDeadEntries();
  void nextResolvableSkipsDeadEntries();
  void resolvableWalksReportMissWithoutMovingCursor();
  void cursorMoveFlipsCanGoBackAndForward();
  void directionAndNavigatingAreLatches();
};

//--------------------------------------------------------------------------------------------------
// Empty state
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh history is parked before the first slot and offers no navigation.
 */
void TstProjectNavHistory::emptyHistoryHasNoCursor()
{
  DataModel::ProjectNavHistory history;

  QCOMPARE(history.size(), 0);
  QCOMPARE(history.cursor(), -1);
  QVERIFY(!history.canGoBack());
  QVERIFY(!history.canGoForward());
  QCOMPARE(history.direction(), 0);
  QVERIFY(!history.navigating());
}

//--------------------------------------------------------------------------------------------------
// push()
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each accepted push appends one entry and parks the cursor on it.
 */
void TstProjectNavHistory::pushRecordsAndAdvancesCursor()
{
  DataModel::ProjectNavHistory history;

  QVERIFY(history.push(entity(1, 10)));
  QCOMPARE(history.size(), 1);
  QCOMPARE(history.cursor(), 0);

  QVERIFY(history.push(entity(1, 11)));
  QCOMPARE(history.size(), 2);
  QCOMPARE(history.cursor(), 1);
  QCOMPARE(history.entryAt(1).id, 11);
}

/**
 * @brief An entry the editor could not identify is dropped, not recorded as a dead slot.
 */
void TstProjectNavHistory::pushRejectsInvalidEntries()
{
  DataModel::ProjectNavHistory history;

  Entry invalid;
  QVERIFY(!history.push(invalid));
  QCOMPARE(history.size(), 0);
  QCOMPARE(history.cursor(), -1);
}

/**
 * @brief One click reaches the editor twice (currentChanged plus selectionChanged), so a repeat of
 *        the entry under the cursor must be swallowed rather than doubling the history.
 */
void TstProjectNavHistory::pushDedupesAgainstCursor()
{
  DataModel::ProjectNavHistory history;

  QVERIFY(history.push(entity(2, 7, 3)));
  QVERIFY(!history.push(entity(2, 7, 3)));
  QCOMPARE(history.size(), 1);
  QCOMPARE(history.cursor(), 0);

  QVERIFY(history.push(container(4)));
  QCOMPARE(history.size(), 2);
}

/**
 * @brief The dedup is against the cursor only, so revisiting a node after visiting another one is
 *        a genuine new step.
 */
void TstProjectNavHistory::pushAllowsRepeatAfterAnotherTarget()
{
  DataModel::ProjectNavHistory history;

  QVERIFY(history.push(entity(1, 10)));
  QVERIFY(history.push(entity(1, 11)));
  QVERIFY(history.push(entity(1, 10)));

  QCOMPARE(history.size(), 3);
  QCOMPARE(history.cursor(), 2);
  QCOMPARE(history.entryAt(2).id, 10);
}

/**
 * @brief Visiting a new node after stepping back discards the forward tail, exactly like a browser.
 */
void TstProjectNavHistory::pushTruncatesForwardTail()
{
  DataModel::ProjectNavHistory history;

  QVERIFY(history.push(entity(1, 10)));
  QVERIFY(history.push(entity(1, 11)));
  QVERIFY(history.push(entity(1, 12)));
  QCOMPARE(history.size(), 3);

  history.setCursor(0);
  QVERIFY(history.push(entity(1, 99)));

  QCOMPARE(history.size(), 2);
  QCOMPARE(history.cursor(), 1);
  QCOMPARE(history.entryAt(0).id, 10);
  QCOMPARE(history.entryAt(1).id, 99);
  QVERIFY(!history.canGoForward());
}

/**
 * @brief The ring is bounded: overflow drops from the front and drags the cursor with it, so the
 *        cursor keeps pointing at the entry that was just pushed.
 */
void TstProjectNavHistory::pushCapsAtMaxEntries()
{
  DataModel::ProjectNavHistory history;

  constexpr int kOverflow = 200;
  for (int i = 0; i < kOverflow; ++i)
    QVERIFY(history.push(entity(1, i)));

  QCOMPARE(history.size(), 128);
  QCOMPARE(history.cursor(), 127);
  QCOMPARE(history.entryAt(127).id, kOverflow - 1);
  QCOMPARE(history.entryAt(0).id, kOverflow - 128);
}

//--------------------------------------------------------------------------------------------------
// clear()
//--------------------------------------------------------------------------------------------------

/**
 * @brief clear() reports whether anything was discarded so the caller only emits on a real change.
 */
void TstProjectNavHistory::clearResetsAndReportsChange()
{
  DataModel::ProjectNavHistory history;

  QVERIFY(!history.clear());

  QVERIFY(history.push(entity(1, 10)));
  QVERIFY(history.clear());
  QCOMPARE(history.size(), 0);
  QCOMPARE(history.cursor(), -1);
  QVERIFY(!history.clear());
}

//--------------------------------------------------------------------------------------------------
// sameTarget()
//--------------------------------------------------------------------------------------------------

void TstProjectNavHistory::sameTargetSeparatesKinds_data()
{
  QTest::addColumn<Entry>("lhs");
  QTest::addColumn<Entry>("rhs");
  QTest::addColumn<bool>("expected");

  QTest::newRow("identical entities") << entity(1, 5, 2) << entity(1, 5, 2) << true;
  QTest::newRow("different kind") << entity(1, 5, 2) << entity(3, 5, 2) << false;
  QTest::newRow("different id") << entity(1, 5, 2) << entity(1, 6, 2) << false;
  QTest::newRow("different parent") << entity(1, 5, 2) << entity(1, 5, 4) << false;
  QTest::newRow("different table key")
    << entity(1, 5, 2, QStringLiteral("a")) << entity(1, 5, 2, QStringLiteral("b")) << false;
  QTest::newRow("identical containers") << container(2) << container(2) << true;
  QTest::newRow("different view") << container(2) << container(3) << false;
  QTest::newRow("container versus entity") << container(0) << entity(0, -1) << false;
}

/**
 * @brief Container entries are identified by destination view, entity entries by the
 *        kind/id/parentId/key tuple; the two families never compare equal.
 */
void TstProjectNavHistory::sameTargetSeparatesKinds()
{
  QFETCH(Entry, lhs);
  QFETCH(Entry, rhs);
  QFETCH(bool, expected);

  QCOMPARE(DataModel::ProjectNavHistory::sameTarget(lhs, rhs), expected);
  QCOMPARE(DataModel::ProjectNavHistory::sameTarget(rhs, lhs), expected);
}

//--------------------------------------------------------------------------------------------------
// Resolution walks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stepping back walks past entries whose node no longer exists in the rebuilt tree.
 */
void TstProjectNavHistory::previousResolvableSkipsDeadEntries()
{
  DataModel::ProjectNavHistory history;
  for (int i = 0; i < 4; ++i)
    QVERIFY(history.push(entity(1, i)));

  const QSet<int> deleted = {1, 2};
  const int index         = history.previousResolvable(
    [&deleted](const Entry& entry) { return !deleted.contains(entry.id); });

  QCOMPARE(index, 0);
  QCOMPARE(history.cursor(), 3);
}

/**
 * @brief Stepping forward walks past entries whose node no longer exists in the rebuilt tree.
 */
void TstProjectNavHistory::nextResolvableSkipsDeadEntries()
{
  DataModel::ProjectNavHistory history;
  for (int i = 0; i < 4; ++i)
    QVERIFY(history.push(entity(1, i)));

  history.setCursor(0);

  const QSet<int> deleted = {1, 2};
  const int index =
    history.nextResolvable([&deleted](const Entry& entry) { return !deleted.contains(entry.id); });

  QCOMPARE(index, 3);
}

/**
 * @brief A walk that resolves nothing returns -1 and leaves the cursor where it was, so a fully
 *        stale history cannot strand the editor on a node it cannot select.
 */
void TstProjectNavHistory::resolvableWalksReportMissWithoutMovingCursor()
{
  DataModel::ProjectNavHistory history;
  for (int i = 0; i < 3; ++i)
    QVERIFY(history.push(entity(1, i)));

  history.setCursor(1);

  const auto never = [](const Entry&) {
    return false;
  };

  QCOMPARE(history.previousResolvable(never), -1);
  QCOMPARE(history.nextResolvable(never), -1);
  QCOMPARE(history.cursor(), 1);
}

//--------------------------------------------------------------------------------------------------
// Cursor and latches
//--------------------------------------------------------------------------------------------------

/**
 * @brief canGoBack/canGoForward track the cursor position within the recorded ring.
 */
void TstProjectNavHistory::cursorMoveFlipsCanGoBackAndForward()
{
  DataModel::ProjectNavHistory history;
  for (int i = 0; i < 3; ++i)
    QVERIFY(history.push(entity(1, i)));

  QVERIFY(history.canGoBack());
  QVERIFY(!history.canGoForward());

  history.setCursor(1);
  QVERIFY(history.canGoBack());
  QVERIFY(history.canGoForward());

  history.setCursor(0);
  QVERIFY(!history.canGoBack());
  QVERIFY(history.canGoForward());
}

/**
 * @brief The reveal direction and the replay guard are plain latches the editor raises around a
 *        programmatic selection change and lowers immediately after.
 */
void TstProjectNavHistory::directionAndNavigatingAreLatches()
{
  DataModel::ProjectNavHistory history;

  history.setDirection(-1);
  history.setNavigating(true);
  QCOMPARE(history.direction(), -1);
  QVERIFY(history.navigating());

  history.setDirection(0);
  history.setNavigating(false);
  QCOMPARE(history.direction(), 0);
  QVERIFY(!history.navigating());
}

QTEST_APPLESS_MAIN(TstProjectNavHistory)

#include "tst_project_nav_history.moc"
