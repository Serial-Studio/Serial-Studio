/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "UI/Dashboard.h"

namespace DataModel {

/**
 * @brief Detaches cyclic or dangling folder parents and resets entities pointing at a missing
 *        folder. Generic over the folder + owning-entity types (workspace/group/table).
 */
template<typename Folder, typename Entity>
static void sanitizeFolderTree(std::vector<Folder>& folders, std::vector<Entity>& entities)
{
  QHash<int, int> parentOf;
  QSet<int> validFolders;
  for (const auto& f : std::as_const(folders)) {
    validFolders.insert(f.folderId);
    parentOf.insert(f.folderId, f.parentFolderId);
  }

  for (auto& f : folders) {
    if (f.parentFolderId != -1 && !validFolders.contains(f.parentFolderId)) {
      f.parentFolderId = -1;
      continue;
    }

    QSet<int> seen;
    int p          = f.parentFolderId;
    const int kMax = static_cast<int>(folders.size());
    for (int i = 0; i < kMax && p != -1; ++i) {
      if (p == f.folderId || seen.contains(p)) {
        f.parentFolderId = -1;
        break;
      }

      seen.insert(p);
      p = parentOf.value(p, -1);
    }
  }

  for (auto& e : entities)
    if (e.parentFolderId != -1 && !validFolders.contains(e.parentFolderId))
      e.parentFolderId = -1;
}

/**
 * @brief Returns @p rootId and every descendant folder id in top-down (breadth-first) order, so a
 *        parent is always listed before its children. Cycle-safe: bounded by the folder count.
 */
template<typename Folder>
static std::vector<int> collectFolderSubtreeOrdered(const std::vector<Folder>& folders, int rootId)
{
  std::vector<int> ordered;
  if (!folderExists(folders, rootId))
    return ordered;

  QSet<int> seen;
  ordered.push_back(rootId);
  seen.insert(rootId);

  const int kMax = static_cast<int>(folders.size());
  for (size_t head = 0; head < ordered.size() && static_cast<int>(ordered.size()) <= kMax; ++head) {
    const int parent = ordered[head];
    for (const auto& f : folders)
      if (f.parentFolderId == parent && !seen.contains(f.folderId)) {
        seen.insert(f.folderId);
        ordered.push_back(f.folderId);
      }
  }

  return ordered;
}

/**
 * @brief Reads the parent id and title of the folder with @p id into the out params; false if none.
 */
template<typename Folder>
static bool folderFields(const std::vector<Folder>& folders, int id, int& parent, QString& title)
{
  for (const auto& f : folders)
    if (f.folderId == id) {
      parent = f.parentFolderId;
      title  = f.title;
      return true;
    }

  return false;
}

/**
 * @brief Collects the titles of every folder filed directly under @p parentFolderId.
 */
template<typename Folder>
static QStringList siblingFolderTitles(const std::vector<Folder>& folders, int parentFolderId)
{
  QStringList titles;
  for (const auto& f : folders)
    if (f.parentFolderId == parentFolderId)
      titles.append(f.title);

  return titles;
}

/**
 * @brief Clones the folders in @p ordered (root first) via @p addFolder, mapping each OLD folder id
 *        to its new id. The root is re-filed beside itself with a nextDuplicateTitle name; every
 *        child keeps its title and follows its mapped parent.
 */
template<typename Folder, typename AddFn>
static QHash<int, int> cloneFolderSubtree(const std::vector<Folder>& folders,
                                          const std::vector<int>& ordered,
                                          int rootFolderId,
                                          AddFn addFolder)
{
  QHash<int, int> idMap;
  for (const int oldId : ordered) {
    int oldParent = -1;
    QString oldTitle;
    if (!folderFields(folders, oldId, oldParent, oldTitle))
      continue;

    const bool isRoot   = (oldId == rootFolderId);
    const int newParent = isRoot ? oldParent : idMap.value(oldParent, -1);
    const QString title =
      isRoot ? nextDuplicateTitle(oldTitle, siblingFolderTitles(folders, oldParent)) : oldTitle;

    idMap.insert(oldId, addFolder(newParent, title));
  }

  return idMap;
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Workspace folder CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the title of a workspace folder, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceFolderTitle(int folderId) const
{
  for (const auto& f : m_workspaceFolders)
    if (f.folderId == folderId)
      return f.title;

  return QString();
}

/**
 * @brief Adds a folder under @p parentFolderId (-1 = top level); returns its new id.
 */
int DataModel::ProjectModel::addWorkspaceFolder(int parentFolderId, const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (parentFolderId != -1 && !folderExists(m_workspaceFolders, parentFolderId))
    parentFolderId = -1;

  int newId = 1;
  for (const auto& f : std::as_const(m_workspaceFolders))
    if (f.folderId >= newId)
      newId = f.folderId + 1;

  DataModel::WorkspaceFolder folder;
  folder.folderId       = newId;
  folder.parentFolderId = parentFolderId;
  folder.title          = title.simplified().isEmpty() ? tr("Folder") : title.simplified();
  m_workspaceFolders.push_back(folder);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return newId;
}

/**
 * @brief Renames a workspace folder; ignores empty names.
 */
void DataModel::ProjectModel::renameWorkspaceFolder(int folderId, const QString& title)
{
  if (title.simplified().isEmpty())
    return;

  for (auto& f : m_workspaceFolders) {
    if (f.folderId != folderId)
      continue;

    f.title = title.simplified();
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Deletes a folder, promoting its child folders and workspaces to its parent.
 */
void DataModel::ProjectModel::deleteWorkspaceFolder(int folderId)
{
  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaceFolders.begin(),
                         m_workspaceFolders.end(),
                         [folderId](const auto& f) { return f.folderId == folderId; });
  if (it == m_workspaceFolders.end())
    return;

  const int promoteTo = it->parentFolderId;
  for (auto& f : m_workspaceFolders)
    if (f.parentFolderId == folderId)
      f.parentFolderId = promoteTo;

  for (auto& ws : m_workspaces)
    if (ws.parentFolderId == folderId)
      ws.parentFolderId = promoteTo;

  m_workspaceFolders.erase(it);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Files a workspace into folder @p parentFolderId (-1 = top level).
 */
void DataModel::ProjectModel::moveWorkspaceToFolder(int workspaceId, int parentFolderId)
{
  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (parentFolderId != -1 && !folderExists(m_workspaceFolders, parentFolderId))
    return;

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    if (ws.parentFolderId == parentFolderId)
      return;

    ws.parentFolderId = parentFolderId;
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Re-parents a folder, rejecting a cyclic move into its own subtree.
 */
void DataModel::ProjectModel::moveFolderToFolder(int folderId, int parentFolderId)
{
  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (parentFolderId != -1 && !folderExists(m_workspaceFolders, parentFolderId))
    return;

  if (folderIsSelfOrDescendant(m_workspaceFolders, folderId, parentFolderId))
    return;

  for (auto& f : m_workspaceFolders) {
    if (f.folderId != folderId)
      continue;

    if (f.parentFolderId == parentFolderId)
      return;

    f.parentFolderId = parentFolderId;
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Reorders a workspace among its siblings in the same folder.
 */
void DataModel::ProjectModel::moveWorkspaceInFolder(int workspaceId, int direction)
{
  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  const int n = static_cast<int>(m_workspaces.size());
  int from    = -1;
  for (int i = 0; i < n; ++i)
    if (m_workspaces[static_cast<size_t>(i)].workspaceId == workspaceId) {
      from = i;
      break;
    }

  if (from < 0)
    return;

  const int parent = m_workspaces[static_cast<size_t>(from)].parentFolderId;
  const int step   = (direction < 0) ? -1 : 1;
  for (int j = from + step; j >= 0 && j < n; j += step) {
    if (m_workspaces[static_cast<size_t>(j)].parentFolderId != parent)
      continue;

    std::swap(m_workspaces[static_cast<size_t>(from)], m_workspaces[static_cast<size_t>(j)]);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Reorders a folder among its sibling folders in the same parent.
 */
void DataModel::ProjectModel::moveWorkspaceFolderInParent(int folderId, int direction)
{
  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  const int n = static_cast<int>(m_workspaceFolders.size());
  int from    = -1;
  for (int i = 0; i < n; ++i)
    if (m_workspaceFolders[static_cast<size_t>(i)].folderId == folderId) {
      from = i;
      break;
    }

  if (from < 0)
    return;

  const int parent = m_workspaceFolders[static_cast<size_t>(from)].parentFolderId;
  const int step   = (direction < 0) ? -1 : 1;
  for (int j = from + step; j >= 0 && j < n; j += step) {
    if (m_workspaceFolders[static_cast<size_t>(j)].parentFolderId != parent)
      continue;

    std::swap(m_workspaceFolders[static_cast<size_t>(from)],
              m_workspaceFolders[static_cast<size_t>(j)]);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Prompts for a name and creates a folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddWorkspaceFolder(int parentFolderId)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Folder"), tr("Name:"), QLineEdit::Normal, tr("Folder"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspaceFolder(parentFolderId, name.trimmed());
  QTimer::singleShot(0, this, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectWorkspaceFolder(newId);
  });
}

/**
 * @brief Prompts for a name, creates a workspace, and files it into @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddWorkspaceInFolder(int parentFolderId)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Workspace"), tr("Name:"), QLineEdit::Normal, tr("Workspace"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspace(name.trimmed());
  moveWorkspaceToFolder(newId, parentFolderId);
  QTimer::singleShot(0, this, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectWorkspace(newId);
  });
}

/**
 * @brief Prompts for a new title for the given folder.
 */
void DataModel::ProjectModel::promptRenameWorkspaceFolder(int folderId)
{
  const QString current = workspaceFolderTitle(folderId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Folder"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameWorkspaceFolder(folderId, name.trimmed());
}

/**
 * @brief Confirms before deleting a folder; its contents move to the parent.
 */
void DataModel::ProjectModel::confirmDeleteWorkspaceFolder(int folderId)
{
  const QString name = workspaceFolderTitle(folderId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(
    tr("Delete folder \"%1\"?").arg(name),
    tr("The folder is removed; its workspaces and sub-folders move up to the parent."),
    QMessageBox::Warning,
    tr("Delete Folder"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteWorkspaceFolder(folderId);
}

//--------------------------------------------------------------------------------------------------
// Group folder CRUD (editor-only organization; never reorders m_groups)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the title of a group folder, or empty if not found.
 */
QString DataModel::ProjectModel::groupFolderTitle(int folderId) const
{
  for (const auto& f : m_groupFolders)
    if (f.folderId == folderId)
      return f.title;

  return QString();
}

/**
 * @brief Adds a group folder under @p parentFolderId (-1 = top level); returns its new id.
 */
int DataModel::ProjectModel::addGroupFolder(int parentFolderId, const QString& title)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Folder")};
  if (parentFolderId != -1 && !folderExists(m_groupFolders, parentFolderId))
    parentFolderId = -1;

  int newId = 1;
  for (const auto& f : std::as_const(m_groupFolders))
    if (f.folderId >= newId)
      newId = f.folderId + 1;

  DataModel::GroupFolder folder;
  folder.folderId       = newId;
  folder.parentFolderId = parentFolderId;
  folder.title          = title.simplified().isEmpty() ? tr("Folder") : title.simplified();
  m_groupFolders.push_back(folder);

  setModified(true);
  Q_EMIT groupsChanged();
  return newId;
}

/**
 * @brief Renames a group folder; ignores empty names.
 */
void DataModel::ProjectModel::renameGroupFolder(int folderId, const QString& title)
{
  const ProjectUndoScope undo_scope{*this, tr("Rename Folder")};
  if (title.simplified().isEmpty())
    return;

  for (auto& f : m_groupFolders) {
    if (f.folderId != folderId)
      continue;

    f.title = title.simplified();
    setModified(true);
    Q_EMIT groupsChanged();
    return;
  }
}

/**
 * @brief Deletes a group folder, promoting its child folders and groups to its parent.
 */
void DataModel::ProjectModel::deleteGroupFolder(int folderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Delete Folder")};
  auto it = std::find_if(m_groupFolders.begin(), m_groupFolders.end(), [folderId](const auto& f) {
    return f.folderId == folderId;
  });
  if (it == m_groupFolders.end())
    return;

  const int promoteTo = it->parentFolderId;
  for (auto& f : m_groupFolders)
    if (f.parentFolderId == folderId)
      f.parentFolderId = promoteTo;

  for (auto& g : m_groups)
    if (g.parentFolderId == folderId)
      g.parentFolderId = promoteTo;

  m_groupFolders.erase(it);

  setModified(true);
  Q_EMIT groupsChanged();
}

/**
 * @brief Files a group into folder @p parentFolderId (-1 = top level); never reorders m_groups.
 */
void DataModel::ProjectModel::moveGroupToFolder(int groupId, int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Group")};
  if (parentFolderId != -1 && !folderExists(m_groupFolders, parentFolderId))
    return;

  if (groupId < 0 || static_cast<size_t>(groupId) >= m_groups.size())
    return;

  auto& group = m_groups[static_cast<size_t>(groupId)];
  if (group.parentFolderId == parentFolderId)
    return;

  group.parentFolderId = parentFolderId;
  setModified(true);
  Q_EMIT groupsChanged();
}

/**
 * @brief Re-parents a group folder, rejecting a cyclic move into its own subtree.
 */
void DataModel::ProjectModel::moveGroupFolderToFolder(int folderId, int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Folder")};
  if (parentFolderId != -1 && !folderExists(m_groupFolders, parentFolderId))
    return;

  if (folderIsSelfOrDescendant(m_groupFolders, folderId, parentFolderId))
    return;

  for (auto& f : m_groupFolders) {
    if (f.folderId != folderId)
      continue;

    if (f.parentFolderId == parentFolderId)
      return;

    f.parentFolderId = parentFolderId;
    setModified(true);
    Q_EMIT groupsChanged();
    return;
  }
}

/**
 * @brief Reorders a group folder among its sibling folders in the same parent.
 */
void DataModel::ProjectModel::moveGroupFolderInParent(int folderId, int direction)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Folder")};
  const int n = static_cast<int>(m_groupFolders.size());
  int from    = -1;
  for (int i = 0; i < n; ++i)
    if (m_groupFolders[static_cast<size_t>(i)].folderId == folderId) {
      from = i;
      break;
    }

  if (from < 0)
    return;

  const int parent = m_groupFolders[static_cast<size_t>(from)].parentFolderId;
  const int step   = (direction < 0) ? -1 : 1;
  for (int j = from + step; j >= 0 && j < n; j += step) {
    if (m_groupFolders[static_cast<size_t>(j)].parentFolderId != parent)
      continue;

    std::swap(m_groupFolders[static_cast<size_t>(from)], m_groupFolders[static_cast<size_t>(j)]);
    setModified(true);
    Q_EMIT groupsChanged();
    return;
  }
}

/**
 * @brief Prompts for a name and creates a group folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddGroupFolder(int parentFolderId)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Folder"), tr("Name:"), QLineEdit::Normal, tr("Folder"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addGroupFolder(parentFolderId, name.trimmed());
  QTimer::singleShot(0, this, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectGroupFolder(newId);
  });
}

/**
 * @brief Prompts for a new title for the given group folder.
 */
void DataModel::ProjectModel::promptRenameGroupFolder(int folderId)
{
  const QString current = groupFolderTitle(folderId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Folder"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameGroupFolder(folderId, name.trimmed());
}

/**
 * @brief Confirms before deleting a group folder; its contents move to the parent.
 */
void DataModel::ProjectModel::confirmDeleteGroupFolder(int folderId)
{
  const QString name = groupFolderTitle(folderId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(
    tr("Delete folder \"%1\"?").arg(name),
    tr("The folder is removed; its groups and sub-folders move up to the parent."),
    QMessageBox::Warning,
    tr("Delete Folder"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteGroupFolder(folderId);
}

//--------------------------------------------------------------------------------------------------
// Table folder CRUD (folder path is part of each table's accessor; titles cannot contain '/')
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the title of a table folder, or empty if not found.
 */
QString DataModel::ProjectModel::tableFolderTitle(int folderId) const
{
  for (const auto& f : m_tableFolders)
    if (f.folderId == folderId)
      return f.title;

  return QString();
}

/**
 * @brief Adds a table folder under @p parentFolderId (-1 = top level); returns its new id.
 */
int DataModel::ProjectModel::addTableFolder(int parentFolderId, const QString& title)
{
  const ProjectUndoScope undo_scope{*this, tr("Add Folder")};
  if (parentFolderId != -1 && !folderExists(m_tableFolders, parentFolderId))
    parentFolderId = -1;

  int newId = 1;
  for (const auto& f : std::as_const(m_tableFolders))
    if (f.folderId >= newId)
      newId = f.folderId + 1;

  DataModel::TableFolder folder;
  folder.folderId       = newId;
  folder.parentFolderId = parentFolderId;
  folder.title          = title.simplified().remove(QLatin1Char('/'));
  if (folder.title.isEmpty())
    folder.title = tr("Folder");

  m_tableFolders.push_back(folder);
  setModified(true);
  Q_EMIT tablesChanged();
  return newId;
}

/**
 * @brief Renames a table folder (slashes stripped); ignores empty names.
 */
void DataModel::ProjectModel::renameTableFolder(int folderId, const QString& title)
{
  const ProjectUndoScope undo_scope{*this, tr("Rename Folder")};
  const QString clean = title.simplified().remove(QLatin1Char('/'));
  if (clean.isEmpty())
    return;

  for (auto& f : m_tableFolders) {
    if (f.folderId != folderId)
      continue;

    f.title = clean;
    setModified(true);
    Q_EMIT tablesChanged();
    return;
  }
}

/**
 * @brief Deletes a table folder, promoting its child folders and tables to its parent.
 */
void DataModel::ProjectModel::deleteTableFolder(int folderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Delete Folder")};
  auto it = std::find_if(m_tableFolders.begin(), m_tableFolders.end(), [folderId](const auto& f) {
    return f.folderId == folderId;
  });
  if (it == m_tableFolders.end())
    return;

  const int promoteTo = it->parentFolderId;
  for (auto& f : m_tableFolders)
    if (f.parentFolderId == folderId)
      f.parentFolderId = promoteTo;

  for (auto& t : m_tables)
    if (t.parentFolderId == folderId)
      t.parentFolderId = promoteTo;

  m_tableFolders.erase(it);

  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Files a table into folder @p parentFolderId; rejects a leaf-name collision in the target.
 */
void DataModel::ProjectModel::moveTableToFolder(const QString& tablePath, int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Table")};
  if (parentFolderId != -1 && !folderExists(m_tableFolders, parentFolderId))
    return;

  const int idx = findTableIndexByPath(tablePath);
  if (idx < 0)
    return;

  auto& table = m_tables[static_cast<size_t>(idx)];
  if (table.parentFolderId == parentFolderId)
    return;

  for (size_t i = 0; i < m_tables.size(); ++i)
    if (static_cast<int>(i) != idx && m_tables[i].parentFolderId == parentFolderId
        && m_tables[i].name == table.name)
      return;

  table.parentFolderId = parentFolderId;
  setModified(true);
  Q_EMIT tablesChanged();
}

/**
 * @brief Re-parents a table folder, rejecting a cyclic move into its own subtree.
 */
void DataModel::ProjectModel::moveTableFolderToFolder(int folderId, int parentFolderId)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Folder")};
  if (parentFolderId != -1 && !folderExists(m_tableFolders, parentFolderId))
    return;

  if (folderIsSelfOrDescendant(m_tableFolders, folderId, parentFolderId))
    return;

  for (auto& f : m_tableFolders) {
    if (f.folderId != folderId)
      continue;

    if (f.parentFolderId == parentFolderId)
      return;

    f.parentFolderId = parentFolderId;
    setModified(true);
    Q_EMIT tablesChanged();
    return;
  }
}

/**
 * @brief Reorders a table folder among its sibling folders in the same parent.
 */
void DataModel::ProjectModel::moveTableFolderInParent(int folderId, int direction)
{
  const ProjectUndoScope undo_scope{*this, tr("Move Folder")};
  const int n = static_cast<int>(m_tableFolders.size());
  int from    = -1;
  for (int i = 0; i < n; ++i)
    if (m_tableFolders[static_cast<size_t>(i)].folderId == folderId) {
      from = i;
      break;
    }

  if (from < 0)
    return;

  const int parent = m_tableFolders[static_cast<size_t>(from)].parentFolderId;
  const int step   = (direction < 0) ? -1 : 1;
  for (int j = from + step; j >= 0 && j < n; j += step) {
    if (m_tableFolders[static_cast<size_t>(j)].parentFolderId != parent)
      continue;

    std::swap(m_tableFolders[static_cast<size_t>(from)], m_tableFolders[static_cast<size_t>(j)]);
    setModified(true);
    Q_EMIT tablesChanged();
    return;
  }
}

/**
 * @brief Prompts for a name and creates a table folder under @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddTableFolder(int parentFolderId)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Folder"), tr("Name:"), QLineEdit::Normal, tr("Folder"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addTableFolder(parentFolderId, name.trimmed());
  QTimer::singleShot(0, this, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectTableFolder(newId);
  });
}

/**
 * @brief Prompts for a name, creates a table, and files it into @p parentFolderId.
 */
void DataModel::ProjectModel::promptAddTableInFolder(int parentFolderId)
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Shared Table"), tr("Name:"), QLineEdit::Normal, tr("Shared Table"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const QString added = addTable(name.trimmed(), parentFolderId);
  QTimer::singleShot(0, this, [added] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectUserTable(added);
  });
}

/**
 * @brief Prompts for a new title for the given table folder.
 */
void DataModel::ProjectModel::promptRenameTableFolder(int folderId)
{
  const QString current = tableFolderTitle(folderId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Folder"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameTableFolder(folderId, name.trimmed());
}

/**
 * @brief Confirms before deleting a table folder; its contents move to the parent.
 */
void DataModel::ProjectModel::confirmDeleteTableFolder(int folderId)
{
  const QString name = tableFolderTitle(folderId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(
    tr("Delete folder \"%1\"?").arg(name),
    tr("The folder is removed; its tables and sub-folders move up to the parent. The accessor "
       "path of those tables changes accordingly."),
    QMessageBox::Warning,
    tr("Delete Folder"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteTableFolder(folderId);
}

/**
 * @brief Repairs folder records after load: detaches cyclic or dangling folder parents and
 * resets workspaces pointing at a missing folder. Folder placement rides on each Workspace
 * object (m_workspaces is merged in place, never rebuilt), so this only guards hand-edited files.
 */
void DataModel::ProjectModel::sanitizeWorkspaceFolders()
{
  sanitizeFolderTree(m_workspaceFolders, m_workspaces);
}

/**
 * @brief Repairs the group folder tree after load (cyclic/dangling parents -> top level).
 */
void DataModel::ProjectModel::sanitizeGroupFolders()
{
  sanitizeFolderTree(m_groupFolders, m_groups);
}

/**
 * @brief Repairs the table folder tree after load (cyclic/dangling parents -> top level).
 */
void DataModel::ProjectModel::sanitizeTableFolders()
{
  sanitizeFolderTree(m_tableFolders, m_tables);
}

//--------------------------------------------------------------------------------------------------
// Group and table folder subtree duplication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deep-copies a group folder and its whole subtree: a new folder tree beside @p rootFolderId
 *        (root renamed via nextDuplicateTitle, children keep their titles) plus a fresh-uniqueId
 *        copy of every group filed in the subtree, re-filed into the mapped new folder. Returns the
 *        set of OLD subtree folder ids that were covered.
 */
QSet<int> DataModel::ProjectModel::duplicateGroupFolderSubtree(int rootFolderId)
{
  const auto ordered = collectFolderSubtreeOrdered(m_groupFolders, rootFolderId);
  if (ordered.empty())
    return QSet<int>();

  const auto idMap = cloneFolderSubtree(
    m_groupFolders, ordered, rootFolderId, [this](int parent, const QString& title) {
      return addGroupFolder(parent, title);
    });

  QSet<int> covered;
  for (const int oldId : ordered)
    covered.insert(oldId);

  std::vector<int> groupIds;
  for (const auto& g : m_groups)
    if (covered.contains(g.parentFolderId))
      groupIds.push_back(g.groupId);

  for (const int gid : groupIds) {
    const int oldParent = m_groups[static_cast<size_t>(gid)].parentFolderId;
    duplicateGroup(gid);
    m_groups.back().parentFolderId = idMap.value(oldParent, -1);
  }

  if (!groupIds.empty()) {
    Q_EMIT groupsChanged();
    setModified(true);
  }

  return covered;
}

/**
 * @brief Deep-copies a table folder and its whole subtree: a new folder tree beside @p rootFolderId
 *        (root renamed via nextDuplicateTitle, children keep their titles) plus a copy of every
 *        table filed in the subtree, re-filed into the mapped new folder. Returns the set of OLD
 *        subtree folder ids that were covered.
 */
QSet<int> DataModel::ProjectModel::duplicateTableFolderSubtree(int rootFolderId)
{
  const auto ordered = collectFolderSubtreeOrdered(m_tableFolders, rootFolderId);
  if (ordered.empty())
    return QSet<int>();

  const auto idMap = cloneFolderSubtree(
    m_tableFolders, ordered, rootFolderId, [this](int parent, const QString& title) {
      return addTableFolder(parent, title);
    });

  QSet<int> covered;
  for (const int oldId : ordered)
    covered.insert(oldId);

  std::vector<DataModel::TableDef> copies;
  std::vector<int> targetParents;
  for (const auto& t : m_tables)
    if (covered.contains(t.parentFolderId)) {
      copies.push_back(t);
      targetParents.push_back(idMap.value(t.parentFolderId, -1));
    }

  for (size_t i = 0; i < copies.size(); ++i)
    appendTableCopyToFolder(copies[i], targetParents[i]);

  if (!copies.empty()) {
    Q_EMIT tablesChanged();
    setModified(true);
  }

  return covered;
}
