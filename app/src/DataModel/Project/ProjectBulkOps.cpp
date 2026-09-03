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

#include "DataModel/Project/ProjectBulkOps.h"

#include <algorithm>
#include <QMessageBox>
#include <QSet>

#include "DataModel/Project/ProjectEntities.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectOutputWidgets.h"
#include "DataModel/Project/ProjectPersistence.h"
#include "DataModel/Project/ProjectTables.h"
#include "DataModel/Project/ProjectWorkspaces.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"

namespace DataModel {

static_assert(kBatchKindGroup == ProjectEditor::KindGroup);
static_assert(kBatchKindDataset == ProjectEditor::KindDataset);
static_assert(kBatchKindWorkspace == ProjectEditor::KindWorkspace);
static_assert(kBatchKindWorkspaceFolder == ProjectEditor::KindWorkspaceFolder);
static_assert(kBatchKindAction == ProjectEditor::KindAction);
static_assert(kBatchKindOutputWidget == ProjectEditor::KindOutputWidget);
static_assert(kBatchKindGroupFolder == ProjectEditor::KindGroupFolder);
static_assert(kBatchKindUserTable == ProjectEditor::KindUserTable);
static_assert(kBatchKindTableFolder == ProjectEditor::KindTableFolder);

/**
 * @brief Returns the uniqueId of the group at positional @p gid, or -1 when out of range.
 */
static int batchGroupUidAt(const std::vector<Group>& groups, int gid)
{
  return (gid >= 0 && static_cast<size_t>(gid) < groups.size()) ? groups[gid].uniqueId : -1;
}

/**
 * @brief Returns the uniqueId of dataset @p did inside group @p gid, or -1 when out of range.
 */
static int batchDatasetUidAt(const std::vector<Group>& groups, int gid, int did)
{
  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return -1;

  const auto& datasets = groups[gid].datasets;
  return (did >= 0 && static_cast<size_t>(did) < datasets.size()) ? datasets[did].uniqueId : -1;
}

/**
 * @brief Re-resolves a pinned group uniqueId to its current positional id; falls back to the
 *        original positional id for legacy unassigned uniqueIds, and -1 when the group is gone.
 */
static int batchResolveGroupId(const std::vector<Group>& groups, int groupUid, int staleGid)
{
  if (groupUid < 0)
    return staleGid;

  for (const auto& group : groups)
    if (group.uniqueId == groupUid)
      return group.groupId;

  return -1;
}

/**
 * @brief Re-resolves a pinned dataset uniqueId to its current index inside group @p gid; falls
 *        back to the original positional id when unpinned, and -1 when the dataset is gone.
 */
static int batchResolveDatasetId(const std::vector<Group>& groups,
                                 int gid,
                                 int datasetUid,
                                 int staleDid)
{
  if (datasetUid < 0)
    return staleDid;

  if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
    return -1;

  const auto& datasets = groups[gid].datasets;
  for (size_t d = 0; d < datasets.size(); ++d)
    if (datasets[d].uniqueId == datasetUid)
      return static_cast<int>(d);

  return -1;
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the bulk operations to @p model.
 */
DataModel::ProjectBulkOps::ProjectBulkOps(ProjectModel& model) : m_model(model) {}

//--------------------------------------------------------------------------------------------------
// Bulk duplication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Duplicates every item in @p items, folder subtrees first: each clones its whole folder
 *        tree plus every group or table filed under it, skipping a nested folder whose ancestor is
 *        also selected. Remaining leaf items follow in declared order, skipping any group or table
 * a duplicated subtree already re-created.
 */
void DataModel::ProjectBulkOps::duplicateSelectedItems(const QVariantList& items)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Selection")};
  auto& folders = m_model.m_folders;

  QSet<int> selectedGroupFolders;
  QSet<int> selectedTableFolders;
  for (const auto& v : items) {
    const auto m   = v.toMap();
    const int kind = m.value(QStringLiteral("kind"), -1).toInt();
    const int id   = m.value(QStringLiteral("id"), -1).toInt();
    if (kind == ProjectEditor::KindGroupFolder)
      selectedGroupFolders.insert(id);
    else if (kind == ProjectEditor::KindTableFolder)
      selectedTableFolders.insert(id);
  }

  QSet<int> coveredGroupFolders;
  QSet<int> coveredTableFolders;
  for (const auto& v : items) {
    const auto m   = v.toMap();
    const int kind = m.value(QStringLiteral("kind"), -1).toInt();
    const int id   = m.value(QStringLiteral("id"), -1).toInt();

    if (kind == ProjectEditor::KindGroupFolder
        && !folderHasSelectedAncestor(folders.groupFolders(), id, selectedGroupFolders))
      coveredGroupFolders.unite(folders.duplicateGroupFolderSubtree(id));

    else if (kind == ProjectEditor::KindTableFolder
             && !folderHasSelectedAncestor(folders.tableFolders(), id, selectedTableFolders))
      coveredTableFolders.unite(folders.duplicateTableFolderSubtree(id));
  }

  for (const auto& v : items) {
    const auto entry   = v.toMap();
    const int kind     = entry.value(QStringLiteral("kind"), -1).toInt();
    const int id       = entry.value(QStringLiteral("id"), -1).toInt();
    const int parent   = entry.value(QStringLiteral("parentId"), -1).toInt();
    const QString path = entry.value(QStringLiteral("path")).toString();

    if (kind == ProjectEditor::KindGroup && coveredGroupFolders.contains(parent))
      continue;

    if (kind == ProjectEditor::KindUserTable && coveredTableFolders.contains(parent))
      continue;

    switch (kind) {
      case ProjectEditor::KindGroup:
        m_model.m_entities.duplicateGroup(id);
        break;
      case ProjectEditor::KindDataset:
        m_model.m_entities.duplicateDataset(parent, id);
        break;
      case ProjectEditor::KindAction:
        m_model.m_entities.duplicateAction(id);
        break;
      case ProjectEditor::KindOutputWidget:
        m_model.m_outputWidgets.duplicateOutputWidget(parent, id);
        break;
      case ProjectEditor::KindUserTable:
        m_model.m_tables.duplicateTableByPath(path);
        break;
      default:
        break;
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Bulk deletion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes every item in @p items, contained items first (batchDeleteRank), descending ids
 * within a rank. Group-referencing entries are pinned to uniqueIds and re-resolved per delete:
 * deleting a group's last dataset/output widget cascades into deleting the group and renumbers
 * every later group, so a stale positional id would delete an unrelated, unselected item.
 */
void DataModel::ProjectBulkOps::deleteSelectedItems(const QVariantList& items)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Selection")};
  const auto& groups = m_model.m_groups;

  QList<BatchDeleteEntry> entries;
  entries.reserve(items.size());
  for (const auto& v : items) {
    const auto m = v.toMap();
    BatchDeleteEntry e;
    e.kind       = m.value(QStringLiteral("kind"), -1).toInt();
    e.id         = m.value(QStringLiteral("id"), -1).toInt();
    e.parentId   = m.value(QStringLiteral("parentId"), -1).toInt();
    e.path       = m.value(QStringLiteral("path")).toString();
    e.groupUid   = -1;
    e.datasetUid = -1;

    if (e.kind == ProjectEditor::KindGroup)
      e.groupUid = batchGroupUidAt(groups, e.id);

    if (e.kind == ProjectEditor::KindDataset || e.kind == ProjectEditor::KindOutputWidget)
      e.groupUid = batchGroupUidAt(groups, e.parentId);

    if (e.kind == ProjectEditor::KindDataset)
      e.datasetUid = batchDatasetUidAt(groups, e.parentId, e.id);

    entries.append(e);
  }

  std::sort(entries.begin(), entries.end(), batchDeleteOrderBefore);

  for (const auto& e : entries) {
    switch (e.kind) {
      case ProjectEditor::KindGroup: {
        const int gid = batchResolveGroupId(groups, e.groupUid, e.id);
        if (gid >= 0)
          m_model.m_entities.deleteGroup(gid, false);

        break;
      }
      case ProjectEditor::KindDataset: {
        const int gid = batchResolveGroupId(groups, e.groupUid, e.parentId);
        const int did = batchResolveDatasetId(groups, gid, e.datasetUid, e.id);
        if (gid >= 0 && did >= 0)
          m_model.m_entities.deleteDataset(gid, did, false);

        break;
      }
      case ProjectEditor::KindAction:
        m_model.m_entities.deleteAction(e.id, false);
        break;
      case ProjectEditor::KindOutputWidget: {
        const int gid = batchResolveGroupId(groups, e.groupUid, e.parentId);
        if (gid >= 0)
          m_model.m_outputWidgets.deleteOutputWidget(gid, e.id, false);

        break;
      }
      case ProjectEditor::KindWorkspace:
        m_model.m_workspaces.deleteWorkspace(e.id);
        break;
      case ProjectEditor::KindWorkspaceFolder:
        m_model.m_folders.deleteWorkspaceFolder(e.id);
        break;
      case ProjectEditor::KindGroupFolder:
        m_model.m_folders.deleteGroupFolder(e.id);
        break;
      case ProjectEditor::KindUserTable:
        m_model.m_tables.deleteTable(e.path);
        break;
      case ProjectEditor::KindTableFolder:
        m_model.m_folders.deleteTableFolder(e.id);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Prompts before deleting a multi-selection, then deletes it. A single
 * item is removed without a prompt to preserve the existing delete behavior.
 */
void DataModel::ProjectBulkOps::confirmDeleteSelectedItems(const QVariantList& items)
{
  if (items.isEmpty())
    return;

  const int count = static_cast<int>(items.size());
  if (count > 1) {
    const int choice =
      Misc::Utilities::showMessageBox(ProjectModel::tr("Delete %1 selected items?").arg(count),
                                      ProjectModel::tr("This action cannot be undone."),
                                      QMessageBox::Warning,
                                      ProjectModel::tr("Delete Items"),
                                      QMessageBox::Yes | QMessageBox::Cancel,
                                      QMessageBox::Cancel);

    if (choice != QMessageBox::Yes)
      return;
  }

  deleteSelectedItems(items);
}

//--------------------------------------------------------------------------------------------------
// Bulk re-filing and enablement
//--------------------------------------------------------------------------------------------------

/**
 * @brief Files every item in @p items into folder @p folderId via its per-kind move. The caller
 * filters the selection to a single section so @p folderId is interpreted correctly.
 */
void DataModel::ProjectBulkOps::moveSelectedItemsToFolder(const QVariantList& items, int folderId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Move Selection")};
  auto& folders = m_model.m_folders;

  for (const auto& v : items) {
    const auto m       = v.toMap();
    const int kind     = m.value(QStringLiteral("kind"), -1).toInt();
    const int id       = m.value(QStringLiteral("id"), -1).toInt();
    const QString path = m.value(QStringLiteral("path")).toString();

    switch (kind) {
      case ProjectEditor::KindWorkspace:
        folders.moveWorkspaceToFolder(id, folderId);
        break;
      case ProjectEditor::KindWorkspaceFolder:
        folders.moveFolderToFolder(id, folderId);
        break;
      case ProjectEditor::KindGroup:
        folders.moveGroupToFolder(id, folderId);
        break;
      case ProjectEditor::KindGroupFolder:
        folders.moveGroupFolderToFolder(id, folderId);
        break;
      case ProjectEditor::KindUserTable:
        folders.moveTableToFolder(path, folderId);
        break;
      case ProjectEditor::KindTableFolder:
        folders.moveTableFolderToFolder(id, folderId);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Applies @p enabled to every group whose folder is @p folderId or nested beneath it, so a
 *        folder toggle cascades to its whole subtree. Returns true when a group actually changed.
 */
bool DataModel::ProjectBulkOps::setGroupsInFolderEnabled(const int folderId, const bool enabled)
{
  bool changed = false;
  for (auto& g : m_model.m_groups) {
    if (g.parentFolderId == -1)
      continue;

    if (!folderIsSelfOrDescendant(m_model.m_folders.groupFolders(), folderId, g.parentFolderId))
      continue;

    if (g.enabled == enabled)
      continue;

    g.enabled = enabled;
    changed   = true;
  }

  return changed;
}

/**
 * @brief Enables or disables every applicable item in a tree multi-selection in one pass; a group
 *        folder cascades to its whole subtree. Emits one change, refreshes the runtime frame once,
 *        and folds the batch into a single autosave.
 */
void DataModel::ProjectBulkOps::setItemsEnabled(const QVariantList& items, const bool enabled)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Toggle Selection")};
  if (items.isEmpty())
    return;

  auto& groups         = m_model.m_groups;
  auto& persistence    = m_model.m_persistence;
  const int groupCount = static_cast<int>(groups.size());

  persistence.setAutoSaveSuspended(true);
  bool changed = false;
  for (const auto& v : items) {
    const auto m     = v.toMap();
    const int kind   = m.value(QStringLiteral("kind"), -1).toInt();
    const int id     = m.value(QStringLiteral("id"), -1).toInt();
    const int parent = m.value(QStringLiteral("parentId"), -1).toInt();

    if (kind == ProjectEditor::KindGroupFolder) {
      changed |= setGroupsInFolderEnabled(id, enabled);
      continue;
    }

    if (kind == ProjectEditor::KindGroup && id >= 0 && id < groupCount
        && groups[id].enabled != enabled) {
      groups[id].enabled = enabled;
      changed            = true;
      continue;
    }

    if (kind == ProjectEditor::KindDataset && parent >= 0 && parent < groupCount) {
      auto& datasets = groups[parent].datasets;
      if (id < 0 || static_cast<size_t>(id) >= datasets.size() || datasets[id].enabled == enabled)
        continue;

      datasets[id].enabled = enabled;
      changed              = true;
    }
  }

  persistence.setAutoSaveSuspended(false);
  if (!changed)
    return;

  Q_EMIT m_model.groupsChanged();
  m_model.setModified(true);
  persistence.syncRuntime();
  persistence.flushAutoSave();
}
