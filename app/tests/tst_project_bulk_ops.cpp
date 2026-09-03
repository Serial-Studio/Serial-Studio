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

#include <algorithm>
#include <QList>
#include <QTest>

#include "DataModel/Project/ProjectBulkOps.h"

// ProjectBulkOps::deleteSelectedItems() cannot be linked here (ProjectModel drags the whole
// application, see the tst_proto_importer note in CMakeLists.txt), so the ordering rule it drains
// its queue with lives in the header and is driven directly. The kind constants are pinned to
// ProjectEditor::ItemKind by static_asserts inside ProjectBulkOps.cpp.

/**
 * @brief Builds a queue entry for @p kind; ids default to a fresh selection's positional values.
 */
static DataModel::BatchDeleteEntry entry(int kind, int id, int parentId = -1)
{
  DataModel::BatchDeleteEntry e;
  e.kind       = kind;
  e.id         = id;
  e.parentId   = parentId;
  e.groupUid   = -1;
  e.datasetUid = -1;
  return e;
}

/**
 * @brief Returns the kinds of @p entries after the production sort, in drain order.
 */
static QList<int> drainOrder(QList<DataModel::BatchDeleteEntry> entries)
{
  std::sort(entries.begin(), entries.end(), DataModel::batchDeleteOrderBefore);

  QList<int> kinds;
  kinds.reserve(entries.size());
  for (const auto& e : entries)
    kinds.append(e.kind);

  return kinds;
}

/**
 * @brief Deletion order of a tree multi-selection: contained items must drain before the
 *        container that would re-path or renumber them.
 */
class TstProjectBulkOps : public QObject {
  Q_OBJECT

private slots:
  void tableDrainsBeforeItsFolder();
  void groupDrainsBeforeItsFolder();
  void datasetDrainsBeforeItsGroup();
  void outputWidgetDrainsBeforeItsGroup();
  void workspaceDrainsBeforeItsFolder();
  void mixedSelectionKeepsEveryContainerLast();
  void idsDescendWithinAKind();
  void orderIsAStrictWeakOrdering();
  void unknownKindsSortLast();
};

//--------------------------------------------------------------------------------------------------
// Container-versus-content ordering
//--------------------------------------------------------------------------------------------------

/**
 * @brief The shipped defect (H3): KindTableFolder(11) outranked KindUserTable(10), the folder
 *        delete promoted the table to a new path, and the queued path no longer resolved.
 */
void TstProjectBulkOps::tableDrainsBeforeItsFolder()
{
  const auto order = drainOrder(
    {entry(DataModel::kBatchKindTableFolder, 3), entry(DataModel::kBatchKindUserTable, 0, 3)});

  QCOMPARE(order.first(), DataModel::kBatchKindUserTable);
  QCOMPARE(order.last(), DataModel::kBatchKindTableFolder);
}

/**
 * @brief Same shape for the group tree: the folder delete re-files the groups it held.
 */
void TstProjectBulkOps::groupDrainsBeforeItsFolder()
{
  const auto order = drainOrder(
    {entry(DataModel::kBatchKindGroupFolder, 2), entry(DataModel::kBatchKindGroup, 0, 2)});

  QCOMPARE(order.first(), DataModel::kBatchKindGroup);
  QCOMPARE(order.last(), DataModel::kBatchKindGroupFolder);
}

/**
 * @brief A dataset goes before its group: deleting the group first would leave the dataset entry
 *        unresolvable, and deleting the last dataset cascades into the group anyway.
 */
void TstProjectBulkOps::datasetDrainsBeforeItsGroup()
{
  const auto order =
    drainOrder({entry(DataModel::kBatchKindGroup, 1), entry(DataModel::kBatchKindDataset, 0, 1)});

  QCOMPARE(order.first(), DataModel::kBatchKindDataset);
  QCOMPARE(order.last(), DataModel::kBatchKindGroup);
}

/**
 * @brief Output widgets are group contents too.
 */
void TstProjectBulkOps::outputWidgetDrainsBeforeItsGroup()
{
  const auto order = drainOrder(
    {entry(DataModel::kBatchKindGroup, 1), entry(DataModel::kBatchKindOutputWidget, 0, 1)});

  QCOMPARE(order.first(), DataModel::kBatchKindOutputWidget);
  QCOMPARE(order.last(), DataModel::kBatchKindGroup);
}

/**
 * @brief Workspaces drain before the workspace folder that would re-file them.
 */
void TstProjectBulkOps::workspaceDrainsBeforeItsFolder()
{
  const auto order = drainOrder(
    {entry(DataModel::kBatchKindWorkspaceFolder, 4), entry(DataModel::kBatchKindWorkspace, 7, 4)});

  QCOMPARE(order.first(), DataModel::kBatchKindWorkspace);
  QCOMPARE(order.last(), DataModel::kBatchKindWorkspaceFolder);
}

/**
 * @brief A selection spanning every section drains contents first and the three folder kinds last.
 *        No folder kind contains another's items, so their order among themselves is whatever the
 *        comparator's descending-kind tie-break yields.
 */
void TstProjectBulkOps::mixedSelectionKeepsEveryContainerLast()
{
  const auto order = drainOrder({entry(DataModel::kBatchKindTableFolder, 3),
                                 entry(DataModel::kBatchKindGroupFolder, 2),
                                 entry(DataModel::kBatchKindGroup, 1),
                                 entry(DataModel::kBatchKindUserTable, 0, 3),
                                 entry(DataModel::kBatchKindAction, 0),
                                 entry(DataModel::kBatchKindDataset, 0, 1),
                                 entry(DataModel::kBatchKindOutputWidget, 0, 1)});

  const QList<int> expected = {DataModel::kBatchKindDataset,
                               DataModel::kBatchKindUserTable,
                               DataModel::kBatchKindAction,
                               DataModel::kBatchKindOutputWidget,
                               DataModel::kBatchKindGroup,
                               DataModel::kBatchKindTableFolder,
                               DataModel::kBatchKindGroupFolder};

  QCOMPARE(order, expected);
}

//--------------------------------------------------------------------------------------------------
// Positional stability and ordering algebra
//--------------------------------------------------------------------------------------------------

/**
 * @brief Within one container the highest positional id goes first, so every id still queued
 *        addresses the same item after the erase.
 */
void TstProjectBulkOps::idsDescendWithinAKind()
{
  QList<DataModel::BatchDeleteEntry> entries = {entry(DataModel::kBatchKindDataset, 0, 1),
                                                entry(DataModel::kBatchKindDataset, 2, 1),
                                                entry(DataModel::kBatchKindDataset, 1, 1)};

  std::sort(entries.begin(), entries.end(), DataModel::batchDeleteOrderBefore);

  QCOMPARE(entries[0].id, 2);
  QCOMPARE(entries[1].id, 1);
  QCOMPARE(entries[2].id, 0);
}

/**
 * @brief std::sort requires a strict weak ordering; an entry may never precede itself and two
 *        entries may never both precede each other.
 */
void TstProjectBulkOps::orderIsAStrictWeakOrdering()
{
  const QList<DataModel::BatchDeleteEntry> entries = {entry(DataModel::kBatchKindDataset, 0, 1),
                                                      entry(DataModel::kBatchKindGroup, 1),
                                                      entry(DataModel::kBatchKindTableFolder, 3),
                                                      entry(DataModel::kBatchKindUserTable, 0, 3),
                                                      entry(DataModel::kBatchKindWorkspace, 7, 4)};

  for (const auto& a : entries) {
    QVERIFY(!DataModel::batchDeleteOrderBefore(a, a));
    for (const auto& b : entries)
      QVERIFY(
        !(DataModel::batchDeleteOrderBefore(a, b) && DataModel::batchDeleteOrderBefore(b, a)));
  }
}

/**
 * @brief A kind the switch does not name (a tree row that is not deletable) ranks after every
 *        real one instead of silently displacing a container.
 */
void TstProjectBulkOps::unknownKindsSortLast()
{
  QVERIFY(DataModel::batchDeleteRank(DataModel::kBatchKindTableFolder)
          < DataModel::batchDeleteRank(0));
  QVERIFY(DataModel::batchDeleteRank(DataModel::kBatchKindDataset)
          < DataModel::batchDeleteRank(DataModel::kBatchKindGroup));
}

QTEST_APPLESS_MAIN(TstProjectBulkOps)

#include "tst_project_bulk_ops.moc"
