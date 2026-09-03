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

#pragma once

#include <QString>
#include <QVariantList>

namespace DataModel {

class ProjectModel;

/**
 * @brief Tree item kinds the delete ordering below distinguishes, mirroring the numeric values of
 *        ProjectEditor::ItemKind. ProjectBulkOps.cpp static_asserts every one against the enum, so
 *        the mirror cannot drift; it exists so the ordering rule stays header-inline (and unit
 *        testable) without pulling the editor's Qt Quick and QCodeEditor headers.
 */
inline constexpr int kBatchKindGroup           = 1;
inline constexpr int kBatchKindDataset         = 2;
inline constexpr int kBatchKindWorkspace       = 3;
inline constexpr int kBatchKindWorkspaceFolder = 4;
inline constexpr int kBatchKindAction          = 5;
inline constexpr int kBatchKindOutputWidget    = 6;
inline constexpr int kBatchKindGroupFolder     = 9;
inline constexpr int kBatchKindUserTable       = 10;
inline constexpr int kBatchKindTableFolder     = 11;

/**
 * @brief One pending batch-delete item, pinned to stable uniqueIds where they exist.
 */
struct BatchDeleteEntry {
  int kind;
  int id;
  int parentId;
  int groupUid;
  int datasetUid;
  QString path;
};

/**
 * @brief Deletion rank of a tree item kind: contents first, containers last, so a container
 *        delete cannot re-path an entry still queued behind it. Enum order is NOT that order --
 *        KindTableFolder(11) sorted ahead of KindUserTable(10), and deleting a table folder
 *        promotes its tables, so the queued path missed and the table survived the batch.
 */
[[nodiscard]] inline int batchDeleteRank(int kind)
{
  switch (kind) {
    case kBatchKindDataset:
      return 0;
    case kBatchKindUserTable:
      return 1;
    case kBatchKindAction:
      return 2;
    case kBatchKindOutputWidget:
      return 3;
    case kBatchKindGroup:
      return 4;
    case kBatchKindWorkspace:
      return 5;
    case kBatchKindGroupFolder:
    case kBatchKindTableFolder:
    case kBatchKindWorkspaceFolder:
      return 6;
    default:
      return 7;
  }
}

/**
 * @brief Strict-weak ordering of a delete batch: by rank, then descending id within a container,
 *        so positional ids stay valid while the batch drains.
 */
[[nodiscard]] inline bool batchDeleteOrderBefore(const BatchDeleteEntry& a,
                                                 const BatchDeleteEntry& b)
{
  const int rankA = batchDeleteRank(a.kind);
  const int rankB = batchDeleteRank(b.kind);
  if (rankA != rankB)
    return rankA < rankB;

  if (a.kind != b.kind)
    return a.kind > b.kind;

  if (a.parentId != b.parentId)
    return a.parentId > b.parentId;

  return a.id > b.id;
}

/**
 * @brief Tree multi-selection operations: duplicate, delete, re-file and enable/disable a whole
 *        selection in one undo step. Deletion is the delicate one: children go first, ids descend
 *        within a kind, and group-referencing entries are pinned to uniqueIds and re-resolved per
 *        delete, because a cascading group deletion renumbers every later group.
 */
class ProjectBulkOps {
public:
  explicit ProjectBulkOps(ProjectModel& model);
  ProjectBulkOps(ProjectBulkOps&&)                 = delete;
  ProjectBulkOps(const ProjectBulkOps&)            = delete;
  ProjectBulkOps& operator=(ProjectBulkOps&&)      = delete;
  ProjectBulkOps& operator=(const ProjectBulkOps&) = delete;

  void duplicateSelectedItems(const QVariantList& items);
  void deleteSelectedItems(const QVariantList& items);
  void confirmDeleteSelectedItems(const QVariantList& items);
  void moveSelectedItemsToFolder(const QVariantList& items, int folderId);
  void setItemsEnabled(const QVariantList& items, const bool enabled);

private:
  [[nodiscard]] bool setGroupsInFolderEnabled(const int folderId, const bool enabled);

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
