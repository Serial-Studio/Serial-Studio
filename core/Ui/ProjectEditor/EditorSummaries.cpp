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

#include "ProjectEditor/EditorSummaries.h"

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "Core/Checksum.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/IconRegistry.h"
#include "Core/License.h"
#include "Core/Prompt/UserPrompt.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/WorkspaceKeys.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/WidgetResolution.h"
#include "Misc/IconEngine.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::CurrentView;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model.
 */
EditorSummaries::EditorSummaries(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model)
{}

/**
 * @brief Builds the nested folder hierarchy (folders only) under @p parentId for a cascading
 *        "Move to Folder" menu: each node is {id, title, children:[...]} (directory-explorer
 * style).
 */
template<typename Folder>
static QVariantList buildFolderTree(const std::vector<Folder>& folders, int parentId)
{
  QVariantList out;
  for (const auto& f : folders) {
    if (f.parentFolderId != parentId)
      continue;

    QVariantMap node;
    node[QStringLiteral("id")]       = f.folderId;
    node[QStringLiteral("title")]    = f.title;
    node[QStringLiteral("children")] = buildFolderTree(folders, f.folderId);
    out.append(node);
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Multi-selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Packages each tree selection into a {kind, id, parentId} QVariantMap.
 */
QVariantList EditorSummaries::selectedTreeItems() const
{
  QVariantList result;
  if (!m_editor.m_selectionModel || !m_editor.m_treeModel)
    return result;

  QSet<qint64> seen;
  const auto indexes = m_editor.m_selectionModel->selectedIndexes();
  for (const auto& idx : indexes) {
    if (!idx.isValid() || idx.column() != 0)
      continue;

    const auto key =
      (static_cast<qint64>(idx.row()) << 32) ^ reinterpret_cast<qint64>(idx.internalPointer());
    if (seen.contains(key))
      continue;

    seen.insert(key);

    const int kind = m_editor.m_treeModel->data(idx, TreeItemKind).toInt();
    if (kind == KindNone)
      continue;

    QVariantMap entry;
    entry.insert(QStringLiteral("kind"), kind);
    entry.insert(QStringLiteral("id"), m_editor.m_treeModel->data(idx, TreeItemId).toInt());
    entry.insert(QStringLiteral("parentId"),
                 m_editor.m_treeModel->data(idx, TreeItemParentId).toInt());
    entry.insert(QStringLiteral("path"), m_editor.m_treeModel->data(idx, TreeItemPath).toString());
    result.append(entry);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Data tables: read-only summaries for QML views
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a summary of user tables plus the __datasets__ system table.
 */
QVariantList EditorSummaries::tablesSummary() const
{
  QVariantList result;
  const auto& groups = m_model.groups();

  int datasetCount = 0;
  for (const auto& g : groups)
    datasetCount += static_cast<int>(g.datasets.size());

  QVariantMap sysRow;
  sysRow["name"]        = tr("Dataset Values");
  sysRow["description"] = tr("Raw and transformed values for every dataset (read-only)");
  sysRow["isSystem"]    = true;
  sysRow["entryCount"]  = datasetCount;
  result.append(sysRow);

  const auto& tables = m_model.tables();
  for (const auto& table : tables) {
    QVariantMap row;
    row["name"]        = table.name;
    row["description"] = tr("Shared table defined in &m_editor project");
    row["isSystem"]    = false;
    row["entryCount"]  = static_cast<int>(table.registers.size());
    result.append(row);
  }

  return result;
}

/**
 * @brief Selects the given user table and switches to the UserTableView.
 */
void EditorSummaries::selectUserTable(const QString& tableName)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_userTableItems.begin(); it != m_editor.m_userTableItems.end(); ++it) {
    if (it.value() == tableName) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the workspace with the given id and switches to its editor.
 */
void EditorSummaries::selectWorkspace(int workspaceId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_workspaceItems.begin(); it != m_editor.m_workspaceItems.end(); ++it) {
    if (it.value() == workspaceId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects a workspace folder tree item by id.
 */
void EditorSummaries::selectWorkspaceFolder(int folderId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_workspaceFolderItems.begin();
       it != m_editor.m_workspaceFolderItems.end();
       ++it) {
    if (it.value() == folderId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Returns the workspace folders as {id, title} for move-target menus.
 */
QVariantList EditorSummaries::workspaceFolderTree() const
{
  return buildFolderTree(m_model.editorWorkspaceFolders(), -1);
}

/**
 * @brief Returns the folders and workspaces directly under @p parentFolderId (folders first).
 */
QVariantList EditorSummaries::workspaceFolderContents(int parentFolderId) const
{
  auto& registry         = Core::services().iconRegistry;
  const auto& pm         = m_model;
  const auto& folders    = pm.editorWorkspaceFolders();
  const auto& workspaces = pm.editorWorkspaces();

  QVariantList out;
  for (const auto& f : folders) {
    if (f.parentFolderId != parentFolderId)
      continue;

    int children = 0;
    for (const auto& sub : folders)
      if (sub.parentFolderId == f.folderId)
        ++children;

    for (const auto& ws : workspaces)
      if (ws.parentFolderId == f.folderId)
        ++children;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = true;
    row[QStringLiteral("id")]       = f.folderId;
    row[QStringLiteral("title")]    = f.title;
    row[QStringLiteral("icon")] =
      registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16);
    row[QStringLiteral("count")] = children;
    out.append(row);
  }

  for (const auto& ws : workspaces) {
    if (ws.parentFolderId != parentFolderId)
      continue;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = false;
    row[QStringLiteral("id")]       = ws.workspaceId;
    row[QStringLiteral("title")]    = ws.title;
    row[QStringLiteral("icon")] =
      ws.icon.isEmpty() ? registry.icon(QStringLiteral("widgets"), QStringLiteral("workspace"), 16)
                        : Misc::IconEngine::resolveActionIconSource(ws.icon);
    row[QStringLiteral("count")] = static_cast<int>(ws.widgetRefs.size());
    out.append(row);
  }

  return out;
}

/**
 * @brief Returns the nested group folder hierarchy for the cascading "Move to Folder" menu.
 */
QVariantList EditorSummaries::groupFolderTree() const
{
  return buildFolderTree(m_model.editorGroupFolders(), -1);
}

/**
 * @brief Returns the folders and groups directly under @p parentFolderId (folders first).
 */
QVariantList EditorSummaries::groupFolderContents(int parentFolderId) const
{
  auto& registry      = Core::services().iconRegistry;
  const auto& pm      = m_model;
  const auto& folders = pm.editorGroupFolders();
  const auto& groups  = pm.groups();

  QVariantList out;
  for (const auto& f : folders) {
    if (f.parentFolderId != parentFolderId)
      continue;

    int children = 0;
    for (const auto& sub : folders)
      if (sub.parentFolderId == f.folderId)
        ++children;

    for (const auto& g : groups)
      if (g.parentFolderId == f.folderId)
        ++children;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = true;
    row[QStringLiteral("id")]       = f.folderId;
    row[QStringLiteral("title")]    = f.title;
    row[QStringLiteral("icon")] =
      registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16);
    row[QStringLiteral("count")] = children;
    out.append(row);
  }

  for (const auto& g : groups) {
    if (g.parentFolderId != parentFolderId)
      continue;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = false;
    row[QStringLiteral("id")]       = g.groupId;
    row[QStringLiteral("title")]    = g.title;
    row[QStringLiteral("icon")] =
      SerialStudio::dashboardWidgetIcon(SerialStudio::getDashboardWidget(g), false);
    row[QStringLiteral("count")] = static_cast<int>(g.datasets.size());
    out.append(row);
  }

  return out;
}

/**
 * @brief Maps each group title to its folder path (empty at top level) for report selection.
 */
QVariantMap EditorSummaries::groupFolderPaths() const
{
  const auto& pm      = m_model;
  const auto& folders = pm.editorGroupFolders();
  const auto& groups  = pm.groups();

  QVariantMap out;
  for (const auto& g : groups) {
    const QString path =
      (g.parentFolderId != -1) ? folderDisplayPath(folders, g.parentFolderId) : QString();
    out.insert(g.title, path);
  }

  return out;
}

/**
 * @brief Returns the nested table folder hierarchy for the cascading "Move to Folder" menu.
 */
QVariantList EditorSummaries::tableFolderTree() const
{
  return buildFolderTree(m_model.editorTableFolders(), -1);
}

/**
 * @brief Returns the folders and tables directly under @p parentFolderId (folders first); each
 *        table row carries its full folder-qualified path as the editor handle.
 */
QVariantList EditorSummaries::tableFolderContents(int parentFolderId) const
{
  auto& registry      = Core::services().iconRegistry;
  const auto& pm      = m_model;
  const auto& folders = pm.editorTableFolders();
  const auto& tables  = pm.tables();

  QVariantList out;
  for (const auto& f : folders) {
    if (f.parentFolderId != parentFolderId)
      continue;

    int children = 0;
    for (const auto& sub : folders)
      if (sub.parentFolderId == f.folderId)
        ++children;

    for (const auto& t : tables)
      if (t.parentFolderId == f.folderId)
        ++children;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = true;
    row[QStringLiteral("id")]       = f.folderId;
    row[QStringLiteral("title")]    = f.title;
    row[QStringLiteral("icon")] =
      registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16);
    row[QStringLiteral("count")] = children;
    out.append(row);
  }

  for (const auto& t : tables) {
    if (t.parentFolderId != parentFolderId)
      continue;

    QVariantMap row;
    row[QStringLiteral("isFolder")] = false;
    row[QStringLiteral("path")]     = DataModel::tableFullPath(folders, t.parentFolderId, t.name);
    row[QStringLiteral("title")]    = t.name;
    row[QStringLiteral("icon")] =
      registry.icon(QStringLiteral("editor"), QStringLiteral("shared-table-alt"), 16);
    row[QStringLiteral("count")] = static_cast<int>(t.registers.size());
    out.append(row);
  }

  return out;
}

/**
 * @brief Moves tree selection to the given group folder.
 */
void EditorSummaries::selectGroupFolder(int folderId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_groupFolderItems.begin(); it != m_editor.m_groupFolderItems.end();
       ++it) {
    if (it.value() == folderId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Moves tree selection to the given table folder.
 */
void EditorSummaries::selectTableFolder(int folderId)
{
  if (!m_editor.m_selectionModel)
    return;

  for (auto it = m_editor.m_tableFolderItems.begin(); it != m_editor.m_tableFolderItems.end();
       ++it) {
    if (it.value() == folderId) {
      m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                 QItemSelectionModel::ClearAndSelect);
      return;
    }
  }
}

/**
 * @brief Selects the MQTT Publisher tree item when available.
 */
void EditorSummaries::selectMqttPublisher()
{
  if (!m_editor.m_selectionModel || !m_editor.m_mqttPublisherItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_mqttPublisherItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the InfluxDB sink tree item when available.
 */
void EditorSummaries::selectInfluxSink()
{
  if (!m_editor.m_selectionModel || !m_editor.m_influxSinkItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_influxSinkItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the project-global control-script node in the tree.
 */
void EditorSummaries::selectControlScript()
{
  if (!m_editor.m_selectionModel || !m_editor.m_controlScriptItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_controlScriptItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the project-global Lua library node in the tree.
 */
void EditorSummaries::selectTransformLibrary()
{
  if (!m_editor.m_selectionModel || !m_editor.m_transformLibraryItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_transformLibraryItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the project-global JavaScript library node in the tree.
 */
void EditorSummaries::selectJsLibrary()
{
  if (!m_editor.m_selectionModel || !m_editor.m_jsLibraryItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_jsLibraryItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the Project Scripts node in the tree.
 */
void EditorSummaries::selectProjectScripts()
{
  if (!m_editor.m_selectionModel || !m_editor.m_scriptsRootItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_scriptsRootItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Selects the Data Export node in the tree.
 */
void EditorSummaries::selectDataExport()
{
  if (!m_editor.m_selectionModel || !m_editor.m_exportRootItem)
    return;

  m_editor.m_selectionModel->setCurrentIndex(m_editor.m_exportRootItem->index(),
                                             QItemSelectionModel::ClearAndSelect);
}

/**
 * @brief Updates the tree search query and rebuilds the tree to apply it.
 */
void EditorSummaries::setTreeSearchQuery(const QString& query)
{
  if (m_editor.m_treeSearchQuery == query)
    return;

  m_editor.m_treeSearchQuery = query;
  Q_EMIT m_editor.treeSearchQueryChanged();

  const auto current = m_editor.m_treeSearchQuery;
  QTimer::singleShot(0, &m_editor, [this, current] {
    if (m_editor.m_treeSearchQuery == current)
      m_editor.m_tree.buildTreeModel();
  });
}

/**
 * @brief Returns a summary row per dataset for the __datasets__ table.
 */
QVariantList EditorSummaries::systemDatasetsSummary() const
{
  QVariantList result;
  const auto& groups = m_model.groups();

  for (const auto& group : groups) {
    for (const auto& ds : group.datasets) {
      const int uid = ds.uniqueId;

      QVariantMap row;
      row[Keys::UniqueId] = uid;
      row[Keys::Alias]    = ds.alias;
      row["groupTitle"]   = group.title;
      row["title"]        = ds.title;
      row["units"]        = ds.units;
      row["rawReg"]       = QStringLiteral("raw:") + QString::number(uid);
      row["finalReg"]     = QStringLiteral("final:") + QString::number(uid);
      row["isVirtual"]    = ds.virtual_;
      result.append(row);
    }
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Workspaces: read-only summaries for QML views
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a summary of every workspace defined in the project.
 */
QVariantList EditorSummaries::workspacesSummary() const
{
  QVariantList result;
  const auto& workspaces = m_model.editorWorkspaces();

  for (const auto& ws : workspaces) {
    QVariantMap row;
    row["id"]          = ws.workspaceId;
    row["title"]       = ws.title;
    row["icon"]        = SerialStudio::normalizeIconPath(ws.icon);
    row["widgetCount"] = static_cast<int>(ws.widgetRefs.size());
    result.append(row);
  }

  return result;
}

/**
 * @brief Returns the widget references attached to the given workspace.
 */
QVariantList EditorSummaries::widgetsForWorkspace(int workspaceId) const
{
  QVariantList result;
  const auto& pm     = m_model;
  const auto& wsList = pm.editorWorkspaces();

  auto wsIt = std::find_if(wsList.begin(), wsList.end(), [workspaceId](const auto& w) {
    return w.workspaceId == workspaceId;
  });

  if (wsIt == wsList.end())
    return result;

  const auto lookup = WorkspaceKeys::buildResolvedWidgetLookup(pm);

  for (const auto& ref : wsIt->widgetRefs) {
    QVariantMap row;
    row["widgetType"]     = ref.widgetType;
    row["widgetTypeName"] = SerialStudio::dashboardWidgetTitle(
      static_cast<SerialStudio::DashboardWidget>(ref.widgetType));
    row["groupId"]             = ref.groupUniqueId;
    row["relativeIndex"]       = ref.relativeIndex;
    row[Keys::DatasetUniqueId] = ref.datasetUniqueId;
    row["groupTitle"]          = QString();
    row["datasetTitle"]        = QString();
    row[Keys::UniqueId]        = -1;
    row["isGroupWidget"]       = false;
    row["isLedPanel"]          = false;
    row["displayTitle"]        = QString();
    row["fallbackTitle"]       = QString();
    row["freezeTitleMode"]     = SerialStudio::dashboardWidgetPaintsTitle(
                                   static_cast<SerialStudio::DashboardWidget>(ref.widgetType))
                                 ? QStringLiteral("painted")
                                 : QStringLiteral("bar");

    const auto it = lookup.constFind(
      WorkspaceKeys::workspaceWidgetKey(ref.widgetType, ref.groupUniqueId, ref.relativeIndex));
    if (it != lookup.constEnd()) {
      row["groupTitle"]    = it->groupTitle;
      row["datasetTitle"]  = it->datasetTitle;
      row[Keys::UniqueId]  = it->uniqueId;
      row["isGroupWidget"] = it->isGroupWidget;
      row["isLedPanel"]    = it->isLedPanel;
      if (it->uniqueId >= 0) {
        const auto entity      = pm.displayTitle(it->uniqueId);
        row["displayTitle"]    = pm.widgetDisplayTitle(ref.widgetType, it->uniqueId);
        row["freezeTitleMode"] = pm.freezeTitleMode(ref.widgetType, it->uniqueId);
        row["fallbackTitle"] =
          !entity.isEmpty() ? entity : (it->isGroupWidget ? it->groupTitle : it->datasetTitle);
      }
    }

    result.append(row);
  }

  return result;
}

/**
 * @brief Returns true if the workspace contains at least one ref the project can no longer resolve.
 */
bool EditorSummaries::workspaceHasUnresolvedRefs(int workspaceId) const
{
  const auto& pm     = m_model;
  const auto& wsList = pm.editorWorkspaces();

  const auto wsIt = std::find_if(wsList.begin(), wsList.end(), [workspaceId](const auto& w) {
    return w.workspaceId == workspaceId;
  });

  if (wsIt == wsList.end())
    return false;

  if (wsIt->widgetRefs.empty())
    return false;

  const auto lookup = WorkspaceKeys::buildResolvedWidgetLookup(pm);
  for (const auto& ref : wsIt->widgetRefs) {
    const auto key =
      WorkspaceKeys::workspaceWidgetKey(ref.widgetType, ref.groupUniqueId, ref.relativeIndex);
    if (!lookup.contains(key))
      return true;
  }

  return false;
}

/**
 * @brief Counts every widget reference in every workspace whose target no longer exists.
 */
int EditorSummaries::unresolvedWorkspaceWidgetCount() const
{
  const auto& pm     = m_model;
  const auto lookup  = WorkspaceKeys::buildResolvedWidgetLookup(pm);
  const auto& wsList = pm.editorWorkspaces();

  int count = 0;
  for (const auto& ws : wsList) {
    for (const auto& ref : ws.widgetRefs) {
      const auto key =
        WorkspaceKeys::workspaceWidgetKey(ref.widgetType, ref.groupUniqueId, ref.relativeIndex);
      if (!lookup.contains(key))
        ++count;
    }
  }

  return count;
}

/**
 * @brief Drops every workspace widget reference whose target group/dataset no longer exists.
 */
int EditorSummaries::cleanupUnresolvedWorkspaceWidgets()
{
  auto& pm          = m_model;
  const auto lookup = WorkspaceKeys::buildResolvedWidgetLookup(pm);

  QSet<qint64> validKeys;
  validKeys.reserve(lookup.size());
  for (auto it = lookup.constBegin(); it != lookup.constEnd(); ++it)
    validKeys.insert(it.key());

  return pm.cleanupWorkspaceWidgetRefs(validKeys);
}

/**
 * @brief Asks the user to confirm before removing every stale workspace widget reference.
 */
void EditorSummaries::confirmCleanupUnresolvedWorkspaceWidgets()
{
  const int count = unresolvedWorkspaceWidgetCount();
  if (count <= 0)
    return;

  const QString text =
    (count == 1)
      ? tr("Remove 1 widget reference whose target group or dataset no longer exists?")
      : tr("Remove %1 widget references whose target groups or datasets no longer exist?")
          .arg(count);

  const int choice =
    Core::Prompt::showMessageBox(text,
                                 tr("This will only affect workspace tile placement; "
                                    "no groups, datasets, or data are deleted."),
                                 Core::Prompt::Question,
                                 tr("Clean Up Workspaces"),
                                 Core::Prompt::Yes | Core::Prompt::Cancel,
                                 Core::Prompt::Cancel);

  if (choice == Core::Prompt::Yes)
    cleanupUnresolvedWorkspaceWidgets();
}

/**
 * @brief Returns every widget the project can show with its routing triple.
 */
QVariantList EditorSummaries::allWidgetsSummary() const
{
  QVariantList result;
  QMap<SerialStudio::DashboardWidget, int> groupIdx;
  QMap<SerialStudio::DashboardWidget, int> datasetIdx;

  const auto& groups = m_model.groups();
  const bool pro     = Core::License::activated();
  datasetIdx.insert(SerialStudio::DashboardExtension,
                    SerialStudio::extensionGroupWidgetCount(groups));

  for (const auto& group : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(group))
      continue;

    auto groupKey = SerialStudio::getDashboardWidget(group);
    if (groupKey == SerialStudio::DashboardPlot3D && !pro)
      groupKey = SerialStudio::DashboardMultiPlot;

    const bool isEmptyOutputPanel =
      group.groupType == DataModel::GroupType::Output && group.outputWidgets.empty();

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel) {
      QVariantMap row;
      row["widgetType"]          = static_cast<int>(groupKey);
      row["groupId"]             = group.uniqueId;
      row["relativeIndex"]       = groupIdx.value(groupKey, 0);
      row[Keys::DatasetUniqueId] = -1;
      row["groupTitle"]          = group.title;
      row["datasetTitle"]        = QString();
      row["isGroupWidget"]       = true;
      row["widgetLabel"]         = SerialStudio::dashboardWidgetTitle(groupKey);
      groupIdx[groupKey]         = row["relativeIndex"].toInt() + 1;
      result.append(row);
    }

    const auto recordDatasetWidget = [&](const DataModel::Dataset& ds,
                                         SerialStudio::DashboardWidget k) {
      QVariantMap row;
      row["widgetType"]          = static_cast<int>(k);
      row["groupId"]             = group.uniqueId;
      row["relativeIndex"]       = datasetIdx.value(k, 0);
      row[Keys::DatasetUniqueId] = ds.uniqueId;
      row["groupTitle"]          = group.title;
      row["datasetTitle"]        = ds.title;
      row["isGroupWidget"]       = false;
      row["widgetLabel"]         = SerialStudio::dashboardWidgetTitle(k);
      datasetIdx[k]              = row["relativeIndex"].toInt() + 1;
      result.append(row);
    };

    const auto walkDatasetWidgets = [&](const DataModel::Dataset& ds) {
      const auto keys = SerialStudio::getDashboardWidgets(ds);
      for (const auto& k : keys)
        if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
          recordDatasetWidget(ds, k);
    };

    for (const auto& ds : group.datasets)
      walkDatasetWidgets(ds);

    const bool groupHasLed =
      std::any_of(group.datasets.begin(), group.datasets.end(), [](const DataModel::Dataset& ds) {
        return !ds.hideOnDashboard && ds.led;
      });
    if (groupHasLed) {
      const auto ledKey = SerialStudio::DashboardLED;
      QVariantMap row;
      row["widgetType"]          = static_cast<int>(ledKey);
      row["groupId"]             = group.uniqueId;
      row["relativeIndex"]       = groupIdx.value(ledKey, 0);
      row[Keys::DatasetUniqueId] = -1;
      row["groupTitle"]          = group.title;
      row["datasetTitle"]        = QString();
      row["isGroupWidget"]       = true;
      row["widgetLabel"]         = SerialStudio::dashboardWidgetTitle(ledKey);
      groupIdx[ledKey]           = row["relativeIndex"].toInt() + 1;
      result.append(row);
    }
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Reorder API: button/keyboard wrappers + drag-drop request handler
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether the current selection can move up; drives the faded state of the Move Up button.
 */
bool EditorSummaries::canMoveCurrentUp() const
{
  return canMoveCurrent(-1);
}

/**
 * @brief Whether the current selection can move down; drives the faded state of Move Down.
 */
bool EditorSummaries::canMoveCurrentDown() const
{
  return canMoveCurrent(1);
}

/**
 * @brief Bounds check mirroring the move guards without mutating: true only when a sibling exists
 *        in @p direction. Folders allow it (bounds enforced at commit); non-orderable views false.
 */
bool EditorSummaries::canMoveCurrent(int direction) const
{
  const int step = direction < 0 ? -1 : 1;

  if (m_editor.m_currentView == GroupView) {
    const int target = m_editor.m_selectedGroup.groupId + step;
    return m_editor.m_selectedGroup.groupId >= 0 && target >= 0 && target < m_model.groupCount();
  }

  if (m_editor.m_currentView == ActionView) {
    const int target = m_editor.m_selectedAction.actionId + step;
    return m_editor.m_selectedAction.actionId >= 0 && target >= 0
        && target < static_cast<int>(m_model.actions().size());
  }

  const auto& groups = m_model.groups();

  if (m_editor.m_currentView == DatasetView) {
    const int gid = m_editor.m_selectedDataset.groupId;
    if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
      return false;

    const int target = m_editor.m_selectedDataset.datasetId + step;
    return m_editor.m_selectedDataset.datasetId >= 0 && target >= 0
        && target < static_cast<int>(groups[static_cast<size_t>(gid)].datasets.size());
  }

  if (m_editor.m_currentView == OutputWidgetView) {
    const int gid = m_editor.m_selectedOutputWidget.groupId;
    if (gid < 0 || static_cast<size_t>(gid) >= groups.size())
      return false;

    const int target = m_editor.m_selectedOutputWidget.widgetId + step;
    return m_editor.m_selectedOutputWidget.widgetId >= 0 && target >= 0
        && target < static_cast<int>(groups[static_cast<size_t>(gid)].outputWidgets.size());
  }

  if (m_editor.m_currentView == WorkspaceView) {
    const auto& ws = m_model.editorWorkspaces();
    const int n    = static_cast<int>(ws.size());
    int from       = -1;
    for (int i = 0; i < n; ++i)
      if (ws[static_cast<size_t>(i)].workspaceId == m_editor.m_selectedWorkspaceId) {
        from = i;
        break;
      }

    if (from < 0)
      return false;

    const int parent = ws[static_cast<size_t>(from)].parentFolderId;
    for (int j = from + step; j >= 0 && j < n; j += step)
      if (ws[static_cast<size_t>(j)].parentFolderId == parent)
        return true;

    return false;
  }

  return m_editor.m_currentView == GroupFolderView || m_editor.m_currentView == TableFolderView
      || m_editor.m_currentView == WorkspaceFolderView;
}

/**
 * @brief Moves the currently selected group by one step (direction = -1 up, +1 down).
 */
bool EditorSummaries::moveCurrentGroup(int direction)
{
  if (m_editor.m_currentView != GroupView)
    return false;

  const int gid = m_editor.m_selectedGroup.groupId;
  if (gid < 0)
    return false;

  const int n      = m_model.groupCount();
  const int target = gid + (direction < 0 ? -1 : 1);
  if (target < 0 || target >= n)
    return false;

  m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::Group;
  m_editor.m_pendingSelectionGroupId = target;
  m_editor.m_pendingSelectionItemId  = -1;
  m_model.moveGroup(gid, target);
  return true;
}

/**
 * @brief Moves the currently selected dataset within its group.
 */
bool EditorSummaries::moveCurrentDataset(int direction)
{
  if (m_editor.m_currentView != DatasetView)
    return false;

  const int gid = m_editor.m_selectedDataset.groupId;
  const int did = m_editor.m_selectedDataset.datasetId;
  if (gid < 0 || did < 0)
    return false;

  const auto& groups = m_model.groups();
  if (static_cast<size_t>(gid) >= groups.size())
    return false;

  const int n      = static_cast<int>(groups[gid].datasets.size());
  const int target = did + (direction < 0 ? -1 : 1);
  if (target < 0 || target >= n)
    return false;

  m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::Dataset;
  m_editor.m_pendingSelectionGroupId = gid;
  m_editor.m_pendingSelectionItemId  = target;
  m_model.moveDataset(gid, did, target);
  return true;
}

/**
 * @brief Moves the currently selected action up or down in the actions list.
 */
bool EditorSummaries::moveCurrentAction(int direction)
{
  if (m_editor.m_currentView != ActionView)
    return false;

  const int aid = m_editor.m_selectedAction.actionId;
  if (aid < 0)
    return false;

  const int n      = static_cast<int>(m_model.actions().size());
  const int target = aid + (direction < 0 ? -1 : 1);
  if (target < 0 || target >= n)
    return false;

  m_model.moveAction(aid, target);
  return true;
}

/**
 * @brief Moves the currently selected output widget within its group.
 */
bool EditorSummaries::moveCurrentOutputWidget(int direction)
{
  if (m_editor.m_currentView != OutputWidgetView)
    return false;

  const int gid = m_editor.m_selectedOutputWidget.groupId;
  const int wid = m_editor.m_selectedOutputWidget.widgetId;
  if (gid < 0 || wid < 0)
    return false;

  const auto& groups = m_model.groups();
  if (static_cast<size_t>(gid) >= groups.size())
    return false;

  const int n      = static_cast<int>(groups[gid].outputWidgets.size());
  const int target = wid + (direction < 0 ? -1 : 1);
  if (target < 0 || target >= n)
    return false;

  m_model.moveOutputWidget(gid, wid, target);
  return true;
}

/**
 * @brief Moves a workspace by one step in the editor list.
 */
bool EditorSummaries::moveWorkspace(int workspaceId, int direction)
{
  const auto& workspaces = m_model.editorWorkspaces();
  const int n            = static_cast<int>(workspaces.size());

  int from = -1;
  for (int i = 0; i < n; ++i)
    if (workspaces[static_cast<size_t>(i)].workspaceId == workspaceId) {
      from = i;
      break;
    }

  if (from < 0)
    return false;

  const int parent = workspaces[static_cast<size_t>(from)].parentFolderId;
  const int step   = (direction < 0) ? -1 : 1;

  int target = -1;
  for (int j = from + step; j >= 0 && j < n; j += step)
    if (workspaces[static_cast<size_t>(j)].parentFolderId == parent) {
      target = j;
      break;
    }

  if (target < 0)
    return false;

  m_model.moveWorkspaceInFolder(workspaceId, direction);
  return true;
}

}  // namespace DataModel
