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

#include <algorithm>
#include <QSet>
#include <QString>
#include <vector>

#include "DataModel/Frame.h"

namespace DataModel {

class ProjectModel;

//--------------------------------------------------------------------------------------------------
// Folder-tree arithmetic, shared by every folder flavour (workspace / group / table)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when a folder with @p id exists in any folder vector.
 */
template<typename Folder>
[[nodiscard]] bool folderExists(const std::vector<Folder>& folders, int id)
{
  return std::any_of(
    folders.begin(), folders.end(), [id](const auto& f) { return f.folderId == id; });
}

/**
 * @brief Returns the parent folder id of @p folderId, or -1 when the folder is absent or top level.
 */
template<typename Folder>
[[nodiscard]] int folderParentId(const std::vector<Folder>& folders, int folderId)
{
  for (const auto& f : folders)
    if (f.folderId == folderId)
      return f.parentFolderId;

  return -1;
}

/**
 * @brief Returns true when @p candidate is @p folderId or sits inside its subtree.
 */
template<typename Folder>
[[nodiscard]] bool folderIsSelfOrDescendant(const std::vector<Folder>& folders,
                                            int folderId,
                                            int candidate)
{
  const int kMax = static_cast<int>(folders.size());
  int p          = candidate;
  for (int i = 0; i <= kMax && p != -1; ++i) {
    if (p == folderId)
      return true;

    p = folderParentId(folders, p);
  }

  return false;
}

/**
 * @brief True when a folder strictly above @p folderId (an ancestor, never itself) is in @p chosen.
 */
template<typename Folder>
[[nodiscard]] bool folderHasSelectedAncestor(const std::vector<Folder>& folders,
                                             int folderId,
                                             const QSet<int>& chosen)
{
  int parent     = folderParentId(folders, folderId);
  const int kMax = static_cast<int>(folders.size());
  for (int i = 0; i < kMax && parent != -1; ++i) {
    if (chosen.contains(parent))
      return true;

    parent = folderParentId(folders, parent);
  }

  return false;
}

/**
 * @brief Builds a folder's "/"-joined path from any folder vector (root -> leaf).
 */
template<typename Folder>
[[nodiscard]] QString folderDisplayPath(const std::vector<Folder>& folders, int folderId)
{
  QString path;
  int cur        = folderId;
  const int kMax = static_cast<int>(folders.size());
  for (int i = 0; i <= kMax && cur != -1; ++i) {
    const Folder* match = nullptr;
    for (const auto& f : folders)
      if (f.folderId == cur) {
        match = &f;
        break;
      }

    if (!match)
      break;

    path = path.isEmpty() ? match->title : (match->title + QLatin1Char('/') + path);
    cur  = match->parentFolderId;
  }

  return path;
}

/**
 * @brief The project's three editor-only folder trees (workspace, group, table) and every
 *        operation over them: CRUD, re-parenting, sibling reordering, the post-load sanitizers,
 *        and subtree duplication. Owns the three folder vectors; the entities filed into them
 *        stay with their own owners and are reached through the injected facade.
 */
class ProjectFolders {
public:
  explicit ProjectFolders(ProjectModel& model);
  ProjectFolders(ProjectFolders&&)                 = delete;
  ProjectFolders(const ProjectFolders&)            = delete;
  ProjectFolders& operator=(ProjectFolders&&)      = delete;
  ProjectFolders& operator=(const ProjectFolders&) = delete;

  [[nodiscard]] const std::vector<WorkspaceFolder>& workspaceFolders() const noexcept;
  [[nodiscard]] const std::vector<GroupFolder>& groupFolders() const noexcept;
  [[nodiscard]] const std::vector<TableFolder>& tableFolders() const noexcept;

  [[nodiscard]] std::vector<WorkspaceFolder>& mutableWorkspaceFolders() noexcept;
  [[nodiscard]] std::vector<GroupFolder>& mutableGroupFolders() noexcept;
  [[nodiscard]] std::vector<TableFolder>& mutableTableFolders() noexcept;

  void clearAll();
  void clearWorkspaceFolders();

  [[nodiscard]] QString workspaceFolderTitle(int folderId) const;
  [[nodiscard]] QString groupFolderTitle(int folderId) const;
  [[nodiscard]] QString tableFolderTitle(int folderId) const;

  [[nodiscard]] int addWorkspaceFolder(int parentFolderId, const QString& title);
  void renameWorkspaceFolder(int folderId, const QString& title);
  void deleteWorkspaceFolder(int folderId);
  void moveWorkspaceToFolder(int workspaceId, int parentFolderId);
  void moveFolderToFolder(int folderId, int parentFolderId);
  void moveWorkspaceInFolder(int workspaceId, int direction);
  void moveWorkspaceFolderInParent(int folderId, int direction);
  void promptAddWorkspaceFolder(int parentFolderId);
  void promptAddWorkspaceInFolder(int parentFolderId);
  void promptRenameWorkspaceFolder(int folderId);
  void confirmDeleteWorkspaceFolder(int folderId);

  [[nodiscard]] int addGroupFolder(int parentFolderId, const QString& title);
  void renameGroupFolder(int folderId, const QString& title);
  void deleteGroupFolder(int folderId);
  void moveGroupToFolder(int groupId, int parentFolderId);
  void moveGroupFolderToFolder(int folderId, int parentFolderId);
  void moveGroupFolderInParent(int folderId, int direction);
  void promptAddGroupFolder(int parentFolderId);
  void promptRenameGroupFolder(int folderId);
  void confirmDeleteGroupFolder(int folderId);

  [[nodiscard]] int addTableFolder(int parentFolderId, const QString& title);
  void renameTableFolder(int folderId, const QString& title);
  void deleteTableFolder(int folderId);
  void moveTableToFolder(const QString& tablePath, int parentFolderId);
  void moveTableFolderToFolder(int folderId, int parentFolderId);
  void moveTableFolderInParent(int folderId, int direction);
  void promptAddTableFolder(int parentFolderId);
  void promptAddTableInFolder(int parentFolderId);
  void promptRenameTableFolder(int folderId);
  void confirmDeleteTableFolder(int folderId);

  void sanitizeWorkspaceFolders();
  void sanitizeGroupFolders();
  void sanitizeTableFolders();

  [[nodiscard]] QSet<int> duplicateGroupFolderSubtree(int rootFolderId);
  [[nodiscard]] QSet<int> duplicateTableFolderSubtree(int rootFolderId);

private:
  ProjectModel& m_model;

  std::vector<WorkspaceFolder> m_workspaceFolders;
  std::vector<GroupFolder> m_groupFolders;
  std::vector<TableFolder> m_tableFolders;
};

}  // namespace DataModel
