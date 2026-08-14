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
#include "Misc/IconRegistry.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

namespace DataModel {

/**
 * @brief Increments the per-type counter for every eligible dataset widget.
 */
static void tallyDatasetWidgetTypes(const DataModel::Dataset& ds, QMap<int, int>& counts)
{
  const auto keys = SerialStudio::getDashboardWidgets(ds);
  for (const auto& k : keys)
    if (SerialStudio::datasetWidgetEligibleForWorkspace(k))
      counts[static_cast<int>(k)] += 1;
}

/**
 * @brief Appends a dataset widget ref unless the widget type is the LED aggregator.
 */
static bool appendDatasetRef(SerialStudio::DashboardWidget k,
                             int groupUniqueId,
                             QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
                             std::vector<DataModel::WidgetRef>& groupRefs,
                             std::vector<DataModel::WidgetRef>& allRefs)
{
  if (k == SerialStudio::DashboardLED)
    return true;

  if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
    return false;

  DataModel::WidgetRef r;
  r.widgetType    = static_cast<int>(k);
  r.groupUniqueId = groupUniqueId;
  r.relativeIndex = datasetIdx.value(k, 0);
  datasetIdx[k]   = r.relativeIndex + 1;

  groupRefs.push_back(r);
  allRefs.push_back(r);
  return false;
}

/**
 * @brief Collects per-dataset widget refs for a group, returning whether any LED is present.
 */
static bool collectGroupDatasetRefs(const DataModel::Group& group,
                                    QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
                                    std::vector<DataModel::WidgetRef>& groupRefs,
                                    std::vector<DataModel::WidgetRef>& allRefs)
{
  bool groupHasLed = false;
  for (const auto& ds : group.datasets) {
    if (ds.hideOnDashboard)
      continue;

    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys)
      if (appendDatasetRef(k, group.uniqueId, datasetIdx, groupRefs, allRefs))
        groupHasLed = true;
  }
  return groupHasLed;
}

/**
 * @brief Pushes a tracked widget ref into the supplied output vectors.
 */
static void pushTrackedRef(SerialStudio::DashboardWidget key,
                           int groupUniqueId,
                           QMap<SerialStudio::DashboardWidget, int>& runningIdx,
                           std::vector<DataModel::WidgetRef>& groupRefs,
                           std::vector<DataModel::WidgetRef>& allRefs,
                           std::vector<DataModel::WidgetRef>& overviewRefs)
{
  DataModel::WidgetRef r;
  r.widgetType    = static_cast<int>(key);
  r.groupUniqueId = groupUniqueId;
  r.relativeIndex = runningIdx.value(key, 0);
  runningIdx[key] = r.relativeIndex + 1;

  groupRefs.push_back(r);
  allRefs.push_back(r);
  overviewRefs.push_back(r);
}

/**
 * @brief Builds widget refs for one group during auto-workspace synthesis.
 */
static std::vector<DataModel::WidgetRef> buildAutoRefsForGroup(
  const DataModel::Group& group,
  bool pro,
  QMap<SerialStudio::DashboardWidget, int>& groupIdx,
  QMap<SerialStudio::DashboardWidget, int>& datasetIdx,
  std::vector<DataModel::WidgetRef>& allRefs,
  std::vector<DataModel::WidgetRef>& overviewRefs)
{
  std::vector<DataModel::WidgetRef> groupRefs;

  auto groupKey = SerialStudio::getDashboardWidget(group);
  if (groupKey == SerialStudio::DashboardPlot3D && !pro)
    groupKey = SerialStudio::DashboardMultiPlot;

  const bool isEmptyOutputPanel =
    group.groupType == DataModel::GroupType::Output && group.outputWidgets.empty();

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
    pushTrackedRef(groupKey, group.uniqueId, groupIdx, groupRefs, allRefs, overviewRefs);

  const bool groupHasLed = collectGroupDatasetRefs(group, datasetIdx, groupRefs, allRefs);

  if (groupHasLed)
    pushTrackedRef(
      SerialStudio::DashboardLED, group.uniqueId, groupIdx, groupRefs, allRefs, overviewRefs);

  return groupRefs;
}

}  // namespace DataModel

// code-verify off
//--------------------------------------------------------------------------------------------------
// Workspace CRUD
//
// State machine:
//   - Auto state (m_customizeWorkspaces == false):
//       m_workspaces is a derived view, rebuilt from m_groups on every
//       groupsChanged via regenerateAutoWorkspacesUnnotified(). Treat it as
//       read-only. m_autoSnapshot mirrors m_workspaces.
//       Persistence: neither customizeWorkspaces nor workspaces emitted in JSON.
//   - Customized state (m_customizeWorkspaces == true):
//       m_workspaces is user-owned. Structural changes invoke
//       mergeAutoWorkspaceUpdates() which adds first-appearance auto refs
//       without resurrecting user-removed entries (m_autoSnapshot is the
//       diff baseline).
//       Persistence: both keys emitted verbatim.
//
// Transitions (all via setCustomizeWorkspaces):
//   - Auto -> Customized: seed m_workspaces from buildAutoWorkspaces() so the
//     editor never opens empty. Refresh m_autoSnapshot.
//   - Customized -> Auto: discard user list, regenerate from groups.
//
// Workspace ID layout (see WorkspaceIds:: in Frame.h):
//   [1000, 5000) = auto IDs   (Overview=1000, AllData=1001, per-group=1002+gid)
//   [5000, ...)  = user IDs   (addWorkspace picks max(>=5000)+1)
// Disjoint ranges: a future group can never claim an ID a user picked.
//
// m_hiddenGroupIds removes per-group entries from buildAutoWorkspaces output;
// it is independent of customize state and survives Customize toggles.
//
// code-verify on
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new user-defined workspace with the given title.
 */
int DataModel::ProjectModel::addWorkspace(const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  int maxId = WorkspaceIds::UserStart - 1;
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId >= WorkspaceIds::UserStart && ws.workspaceId > maxId)
      maxId = ws.workspaceId;

  DataModel::Workspace ws;
  ws.workspaceId = maxId + 1;
  ws.title       = title.simplified().isEmpty() ? tr("Workspace") : title.simplified();
  m_workspaces.push_back(ws);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return ws.workspaceId;
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectModel::deleteWorkspace(int workspaceId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  m_workspaces.erase(it);
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Wipes every workspace, leaving an empty customised list.
 */
void DataModel::ProjectModel::clearAllWorkspaces()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (m_workspaces.empty())
    return;

  m_workspaces.clear();
  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectModel::renameWorkspace(int workspaceId, const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      ws.title = title.simplified();
      setModified(true);
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
      return;
    }
  }
}

/**
 * @brief Patches title, icon, and/or description on the workspace with the given ID.
 */
void DataModel::ProjectModel::updateWorkspace(int workspaceId,
                                              const QString& title,
                                              const QString& icon,
                                              const QString& description,
                                              bool setTitle,
                                              bool setIcon,
                                              bool setDescription)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      if (setTitle)
        ws.title = title.simplified();

      if (setIcon)
        ws.icon = SerialStudio::normalizeIconPath(icon);

      if (setDescription)
        ws.description = description;

      setModified(true);
      Q_EMIT editorWorkspacesChanged();
      Q_EMIT activeWorkspacesChanged();
      return;
    }
  }
}

/**
 * @brief Reorders user-defined workspaces (id >= UserStart) by the given id
 * sequence, bailing out when the id set does not match the existing user
 * workspaces because a partial reorder would silently corrupt the list.
 */
void DataModel::ProjectModel::reorderWorkspaces(const QList<int>& userWorkspaceIds)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  QHash<int, DataModel::Workspace> userById;
  std::vector<DataModel::Workspace> systemSlots;
  for (auto& ws : m_workspaces)
    if (ws.workspaceId >= WorkspaceIds::UserStart)
      userById.insert(ws.workspaceId, std::move(ws));
    else
      systemSlots.push_back(std::move(ws));

  if (userWorkspaceIds.size() != userById.size())
    return;

  for (int id : userWorkspaceIds)
    if (!userById.contains(id))
      return;

  std::vector<DataModel::Workspace> rebuilt;
  rebuilt.reserve(systemSlots.size() + userById.size());
  for (auto& ws : systemSlots)
    rebuilt.push_back(std::move(ws));

  for (int id : userWorkspaceIds)
    rebuilt.push_back(std::move(userById[id]));

  m_workspaces = std::move(rebuilt);

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Appends a widget reference to the specified workspace.
 */
void DataModel::ProjectModel::addWidgetToWorkspace(int workspaceId,
                                                   int widgetType,
                                                   int groupUniqueId,
                                                   int relativeIndex)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (const auto& ref : ws.widgetRefs)
      if (ref.widgetType == widgetType && ref.groupUniqueId == groupUniqueId
          && ref.relativeIndex == relativeIndex)
        return;

    DataModel::WidgetRef ref;
    ref.widgetType    = widgetType;
    ref.groupUniqueId = groupUniqueId;
    ref.relativeIndex = relativeIndex;
    ws.widgetRefs.push_back(ref);

    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId, int index)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    if (index < 0 || static_cast<size_t>(index) >= ws.widgetRefs.size())
      return;

    ws.widgetRefs.erase(ws.widgetRefs.begin() + index);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference matching (widgetType, groupId, relativeIndex).
 */
void DataModel::ProjectModel::removeWidgetFromWorkspace(int workspaceId,
                                                        int widgetType,
                                                        int groupUniqueId,
                                                        int relativeIndex)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    auto it = std::find_if(ws.widgetRefs.begin(), ws.widgetRefs.end(), [=](const auto& r) {
      return r.widgetType == widgetType && r.groupUniqueId == groupUniqueId
          && r.relativeIndex == relativeIndex;
    });

    if (it == ws.widgetRefs.end())
      return;

    ws.widgetRefs.erase(it);
    setModified(true);
    Q_EMIT editorWorkspacesChanged();
    Q_EMIT activeWorkspacesChanged();
    return;
  }
}

/**
 * @brief Drops every workspace widget ref whose encoded key isn't in validKeys.
 */
int DataModel::ProjectModel::cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return 0;

  const auto encode = [](int widgetType, int groupId, int relIdx) {
    return (static_cast<qint64>(widgetType) << 40) | (static_cast<qint64>(groupId) << 20)
         | static_cast<qint64>(relIdx);
  };

  int removed = 0;
  for (auto& ws : m_workspaces) {
    auto& refs    = ws.widgetRefs;
    const auto it = std::remove_if(refs.begin(), refs.end(), [&](const auto& r) {
      return !validKeys.contains(encode(r.widgetType, r.groupUniqueId, r.relativeIndex));
    });

    const auto count = std::distance(it, refs.end());
    if (count > 0) {
      refs.erase(it, refs.end());
      removed += static_cast<int>(count);
    }
  }

  if (removed == 0)
    return 0;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return removed;
}

/**
 * @brief Returns the title of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceTitle(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.title;

  return QString();
}

/**
 * @brief Returns the icon of a workspace, or empty if not found.
 */
QString DataModel::ProjectModel::workspaceIcon(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.icon;

  return QString();
}

/**
 * @brief Convenience slot that sets only the icon of a workspace.
 */
void DataModel::ProjectModel::setWorkspaceIcon(int workspaceId, const QString& icon)
{
  updateWorkspace(workspaceId, QString(), icon, QString(), false, true, false);
}

/**
 * @brief Prompts for a new workspace name and creates it. The new workspace
 *        is then selected in the project editor.
 */
void DataModel::ProjectModel::promptAddWorkspace()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("New Workspace"), tr("Name:"), QLineEdit::Normal, tr("Workspace"), &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspace(name.trimmed());
  QTimer::singleShot(0, this, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectWorkspace(newId);
  });
}

/**
 * @brief Prompts for a new title for the given workspace.
 */
void DataModel::ProjectModel::promptRenameWorkspace(int workspaceId)
{
  const QString current = workspaceTitle(workspaceId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(
    nullptr, tr("Rename Workspace"), tr("Name:"), QLineEdit::Normal, current, &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameWorkspace(workspaceId, name.trimmed());
}

/**
 * @brief Returns whether the user has opted in to customising workspaces.
 */
bool DataModel::ProjectModel::customizeWorkspaces() const noexcept
{
  return m_customizeWorkspaces;
}

/**
 * @brief Flips the customize switch (Off->On seeds auto layout, On->Off re-seeds it).
 */
void DataModel::ProjectModel::setCustomizeWorkspaces(const bool enabled)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_customizeWorkspaces == enabled)
    return;

  m_customizeWorkspaces = enabled;

  if (enabled) {
    m_workspaces       = buildAutoWorkspaces();
    m_workspaceFolders = buildAutoWorkspaceFoldersFor(m_workspaces);
    m_autoSnapshot     = m_workspaces;
  } else {
    m_workspaceFolders.clear();
    regenerateAutoWorkspacesUnnotified();
  }

  setModified(true);
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Synthesises the default workspace layout for a project.
 */
std::vector<DataModel::Workspace> DataModel::ProjectModel::buildAutoWorkspaces() const
{
  std::vector<DataModel::Workspace> result;

  static auto& appState     = AppState::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  const auto mode = appState.operationMode();
  std::vector<DataModel::Group> quickPlotGroups;
  if (mode == SerialStudio::QuickPlot)
    frameBuilder.invokeOnBuilderThreadBlocking(
      [&] { quickPlotGroups = frameBuilder.quickPlotFrame().groups; });

  const auto& groups = (mode == SerialStudio::QuickPlot) ? quickPlotGroups : m_groups;

  QMap<SerialStudio::DashboardWidget, int> groupIdx;
  QMap<SerialStudio::DashboardWidget, int> datasetIdx;
  datasetIdx.insert(SerialStudio::DashboardExtension,
                    SerialStudio::extensionGroupWidgetCount(groups));

  const bool pro = SerialStudio::activated();

  std::vector<DataModel::WidgetRef> allRefs;
  std::vector<DataModel::WidgetRef> overviewRefs;
  QMap<int, std::vector<DataModel::WidgetRef>> perGroupRefs;

  int eligibleGroups = 0;

  for (const auto& group : groups) {
    if (!SerialStudio::groupEligibleForWorkspace(group))
      continue;

    auto groupRefs = buildAutoRefsForGroup(group, pro, groupIdx, datasetIdx, allRefs, overviewRefs);
    if (groupRefs.empty())
      continue;

    perGroupRefs.insert(group.groupId, std::move(groupRefs));
    ++eligibleGroups;
  }

  if (eligibleGroups == 0)
    return result;

  static auto& registry = Misc::IconRegistry::instance();
  if (overviewRefs.size() >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::Overview;
    ws.title       = tr("Overview");
    ws.icon        = registry.iconById(QStringLiteral("panes/overview"), 32);
    ws.widgetRefs  = overviewRefs;
    result.push_back(std::move(ws));
  }

  if (eligibleGroups >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::AllData;
    ws.title       = tr("All Data");
    ws.icon        = registry.iconById(QStringLiteral("panes/dashboard"), 32);
    ws.widgetRefs  = allRefs;
    result.push_back(std::move(ws));
  }

  appendAutoGroupWorkspaces(result, groups, perGroupRefs);
  return result;
}

/**
 * @brief Emits group workspaces for the auto layout: a leaf group folder (no sub-folders) collapses
 *        into one workspace aggregating every widget of the groups in it; groups in a container
 *        folder or at the top level each get their own workspace.
 */
void DataModel::ProjectModel::appendAutoGroupWorkspaces(
  std::vector<DataModel::Workspace>& result,
  const std::vector<DataModel::Group>& groups,
  const QMap<int, std::vector<DataModel::WidgetRef>>& perGroupRefs) const
{
  QHash<int, int> folderParent;
  QHash<int, QString> folderTitle;
  QSet<int> containerFolders;
  for (const auto& f : m_groupFolders) {
    folderParent.insert(f.folderId, f.parentFolderId);
    folderTitle.insert(f.folderId, f.title);
    if (f.parentFolderId != -1)
      containerFolders.insert(f.parentFolderId);
  }

  QHash<int, int> leafWorkspaceIndex;

  for (const auto& group : groups) {
    if (m_hiddenGroupIds.contains(group.groupId))
      continue;

    const auto it = perGroupRefs.constFind(group.groupId);
    if (it == perGroupRefs.constEnd())
      continue;

    const int fk = group.parentFolderId;
    if (fk != -1 && !containerFolders.contains(fk)) {
      const auto existing = leafWorkspaceIndex.constFind(fk);
      if (existing != leafWorkspaceIndex.constEnd()) {
        auto& ws = result[static_cast<size_t>(existing.value())];
        ws.widgetRefs.insert(ws.widgetRefs.end(), it.value().begin(), it.value().end());
        continue;
      }

      DataModel::Workspace ws;
      ws.workspaceId    = WorkspaceIds::PerFolderStart + fk;
      ws.parentFolderId = folderParent.value(fk, -1);
      ws.title          = folderTitle.value(fk);
      ws.widgetRefs     = it.value();
      leafWorkspaceIndex.insert(fk, static_cast<int>(result.size()));
      result.push_back(std::move(ws));
      continue;
    }

    DataModel::Workspace ws;
    ws.workspaceId    = WorkspaceIds::PerGroupStart + group.groupId;
    ws.parentFolderId = fk;
    ws.title          = group.title;
    ws.widgetRefs     = it.value();
    result.push_back(std::move(ws));
  }
}

/**
 * @brief Derives the auto-layout workspace folder tree from the group folders, mirroring (1:1 ids)
 *        every group folder that transitively contains one of @p workspaces. Empty branches are
 *        pruned. The result is regenerated, never persisted, so it always tracks the group tree.
 */
std::vector<DataModel::WorkspaceFolder> DataModel::ProjectModel::buildAutoWorkspaceFoldersFor(
  const std::vector<DataModel::Workspace>& workspaces) const
{
  std::vector<DataModel::WorkspaceFolder> result;
  if (m_groupFolders.empty())
    return result;

  QHash<int, int> parentOf;
  for (const auto& f : std::as_const(m_groupFolders))
    parentOf.insert(f.folderId, f.parentFolderId);

  QSet<int> needed;
  const int kMax = static_cast<int>(m_groupFolders.size());
  for (const auto& ws : workspaces) {
    int id = ws.parentFolderId;
    for (int i = 0; i <= kMax && id != -1 && !needed.contains(id); ++i) {
      needed.insert(id);
      id = parentOf.value(id, -1);
    }
  }

  for (const auto& f : m_groupFolders) {
    if (!needed.contains(f.folderId))
      continue;

    DataModel::WorkspaceFolder wf;
    wf.folderId       = f.folderId;
    wf.parentFolderId = f.parentFolderId;
    wf.title          = f.title;
    result.push_back(wf);
  }

  return result;
}

/**
 * @brief Refreshes m_workspaces from the project structure WITHOUT emitting signals.
 */
void DataModel::ProjectModel::regenerateAutoWorkspacesUnnotified()
{
  if (m_customizeWorkspaces)
    return;

  m_workspaces       = buildAutoWorkspaces();
  m_workspaceFolders = buildAutoWorkspaceFoldersFor(m_workspaces);
  m_autoSnapshot     = m_workspaces;
}

/**
 * @brief Merges newly-eligible auto refs into the user-customised workspace list.
 */
bool DataModel::ProjectModel::mergeAutoWorkspaceUpdates()
{
  if (!m_customizeWorkspaces)
    return false;

  const auto current = buildAutoWorkspaces();
  bool dirty         = false;

  const auto refsEqual = [](const WidgetRef& a, const WidgetRef& b) {
    return a.widgetType == b.widgetType && a.groupUniqueId == b.groupUniqueId
        && a.relativeIndex == b.relativeIndex;
  };

  const auto findById = [](std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  const auto findByIdConst = [](const std::vector<DataModel::Workspace>& list, int id) {
    return std::find_if(
      list.begin(), list.end(), [id](const auto& w) { return w.workspaceId == id; });
  };

  for (const auto& cur : current) {
    auto userIt = findById(m_workspaces, cur.workspaceId);
    auto snapIt = findByIdConst(m_autoSnapshot, cur.workspaceId);

    if (userIt == m_workspaces.end()) {
      if (snapIt != m_autoSnapshot.end())
        continue;

      m_workspaces.push_back(cur);
      dirty = true;
      continue;
    }

    if (cur.workspaceId >= WorkspaceIds::PerGroupStart && userIt->title != cur.title) {
      userIt->title = cur.title;
      dirty         = true;
    }

    for (const auto& r : cur.widgetRefs) {
      const bool inSnap = snapIt != m_autoSnapshot.end()
                       && std::any_of(snapIt->widgetRefs.begin(),
                                      snapIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inSnap)
        continue;

      const bool inUser = std::any_of(userIt->widgetRefs.begin(),
                                      userIt->widgetRefs.end(),
                                      [&](const auto& s) { return refsEqual(s, r); });
      if (inUser)
        continue;

      userIt->widgetRefs.push_back(r);
      dirty = true;
    }
  }

  m_autoSnapshot = current;

  if (dirty) {
    sanitizeWorkspaceFolders();
    setModified(true);
  }

  return dirty;
}

/**
 * @brief Materialises the synthetic workspace list into m_workspaces and flips the project into
 * customize mode so the user can edit it from there.
 */
int DataModel::ProjectModel::autoGenerateWorkspaces()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (m_customizeWorkspaces && !m_workspaces.empty())
    return m_workspaces.front().workspaceId;

  auto seed = buildAutoWorkspaces();
  if (seed.empty())
    return -1;

  m_workspaces           = std::move(seed);
  m_workspaceFolders     = buildAutoWorkspaceFoldersFor(m_workspaces);
  m_autoSnapshot         = m_workspaces;
  const bool flagChanged = !m_customizeWorkspaces;
  m_customizeWorkspaces  = true;

  SS_ASSERT(!m_workspaces.empty(), return -1);
  SS_ASSERT(m_workspaces.front().workspaceId >= WorkspaceIds::AutoStart, return -1);

  setModified(true);
  if (flagChanged)
    Q_EMIT customizeWorkspacesChanged();

  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
  return m_workspaces.front().workspaceId;
}

/**
 * @brief Drops user customisations and returns the project to the synthetic
 *        auto-layout. Idempotent: a no-op when already in auto mode.
 */
void DataModel::ProjectModel::resetWorkspacesToAuto()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  m_customizeWorkspaces = false;
  m_workspaceFolders.clear();
  regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT customizeWorkspacesChanged();
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Asks the user to confirm before discarding workspace customisations.
 */
void DataModel::ProjectModel::confirmResetWorkspacesToAuto()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  if (m_suppressMessageBoxes) {
    resetWorkspacesToAuto();
    return;
  }

  const int choice = Misc::Utilities::showMessageBox(
    tr("Discard workspace customisations?"),
    tr("Switching off Customize discards your edits and rebuilds the "
       "workspace list from the project's groups."),
    QMessageBox::Warning,
    tr("Customize Workspaces"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    resetWorkspacesToAuto();
}

/**
 * @brief Counts, per dashboard-widget type, how many widgets the given group
 *        contributes to Dashboard::buildWidgetGroups's running type counter.
 */
QMap<int, int> DataModel::ProjectModel::widgetTypeCountsForGroup(const Group& g) const
{
  QMap<int, int> counts;

  if (!SerialStudio::groupEligibleForWorkspace(g))
    return counts;

  auto groupKey = SerialStudio::getDashboardWidget(g);
  if (groupKey == SerialStudio::DashboardPlot3D && !SerialStudio::activated())
    groupKey = SerialStudio::DashboardMultiPlot;

  const bool isEmptyOutputPanel =
    g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

  if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
    counts[static_cast<int>(groupKey)] += 1;

  bool groupHasLed = false;
  for (const auto& ds : g.datasets) {
    if (ds.hideOnDashboard)
      continue;

    const auto keys = SerialStudio::getDashboardWidgets(ds);
    for (const auto& k : keys) {
      if (k == SerialStudio::DashboardLED) {
        groupHasLed = true;
        continue;
      }
      if (!SerialStudio::datasetWidgetEligibleForWorkspace(k))
        continue;

      counts[static_cast<int>(k)] += 1;
    }
  }

  if (groupHasLed)
    counts[static_cast<int>(SerialStudio::DashboardLED)] += 1;

  return counts;
}

/**
 * @brief Shifts or drops user-customised widget refs after a group delete.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterGroupDelete(
  int deletedGid, const QMap<int, int>& deletedTypeCounts)
{
  SS_ASSERT(deletedGid >= 0, return);
  SS_ASSERT(m_customizeWorkspaces, return);

  const int deletedAutoId = WorkspaceIds::PerGroupStart + deletedGid;

  m_workspaces.erase(
    std::remove_if(m_workspaces.begin(),
                   m_workspaces.end(),
                   [deletedAutoId](const Workspace& w) { return w.workspaceId == deletedAutoId; }),
    m_workspaces.end());

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId > deletedAutoId && ws.workspaceId < WorkspaceIds::PerFolderStart)
      ws.workspaceId -= 1;

    for (auto it = ws.widgetRefs.begin(); it != ws.widgetRefs.end();) {
      const int newPos = groupIdForUniqueId(it->groupUniqueId);
      if (newPos < 0) {
        it = ws.widgetRefs.erase(it);
        continue;
      }

      if (newPos >= deletedGid) {
        const int lost    = deletedTypeCounts.value(it->widgetType, 0);
        it->relativeIndex = std::max(0, it->relativeIndex - lost);
      }

      ++it;
    }
  }
}

/**
 * @brief Updates m_hiddenGroupIds after a group is removed and surviving groups are renumbered down
 * by 1.
 */
void DataModel::ProjectModel::shiftHiddenGroupIdsAfterGroupDelete(int deletedGid)
{
  if (m_hiddenGroupIds.isEmpty())
    return;

  QSet<int> updated;
  for (const int id : std::as_const(m_hiddenGroupIds)) {
    if (id == deletedGid)
      continue;

    updated.insert(id > deletedGid ? id - 1 : id);
  }

  m_hiddenGroupIds = std::move(updated);
}

/**
 * @brief Updates layout:N widgetSettings entries after a group renumber.
 */
void DataModel::ProjectModel::shiftLayoutKeysAfterGroupDelete(int deletedGid)
{
  if (m_widgetSettings.isEmpty())
    return;

  const auto keys = m_widgetSettings.keys();
  bool changed    = false;

  if (m_widgetSettings.contains(Keys::layoutKey(deletedGid))) {
    m_widgetSettings.remove(Keys::layoutKey(deletedGid));
    changed = true;
  }

  const QString prefix = QStringLiteral("layout:");
  QList<QPair<int, QJsonObject>> moves;
  for (const auto& key : keys) {
    if (!key.startsWith(prefix))
      continue;

    bool ok      = false;
    const int id = key.mid(prefix.length()).toInt(&ok);
    if (!ok || id <= deletedGid)
      continue;

    moves.append({id, m_widgetSettings.value(key).toObject()});
  }

  std::sort(
    moves.begin(), moves.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  for (const auto& move : moves) {
    m_widgetSettings.remove(Keys::layoutKey(move.first));
    m_widgetSettings.insert(Keys::layoutKey(move.first - 1), move.second);
    changed = true;
  }

  if (changed)
    Q_EMIT widgetSettingsChanged();
}

/**
 * @brief Shifts user-customised widget refs after a single dataset is deleted from a surviving
 * group.
 */
void DataModel::ProjectModel::shiftWorkspaceRefsAfterDatasetDelete(
  int groupId, const QMap<int, int>& datasetTypeCounts)
{
  SS_ASSERT(groupId >= 0, return);
  SS_ASSERT(m_customizeWorkspaces, return);

  if (datasetTypeCounts.isEmpty())
    return;

  QMap<int, int> runningAtGroup;
  for (const auto& g : m_groups) {
    if (!SerialStudio::groupEligibleForWorkspace(g))
      continue;

    if (g.groupId == groupId)
      break;

    const auto groupKey = SerialStudio::getDashboardWidget(g);

    const bool isEmptyOutputPanel =
      g.groupType == DataModel::GroupType::Output && g.outputWidgets.empty();

    if (SerialStudio::groupWidgetEligibleForWorkspace(groupKey) && !isEmptyOutputPanel)
      runningAtGroup[static_cast<int>(groupKey)] += 1;

    for (const auto& ds : g.datasets)
      tallyDatasetWidgetTypes(ds, runningAtGroup);
  }

  const int groupUid = groupUniqueIdForGroupId(groupId);

  for (auto& ws : m_workspaces) {
    ws.widgetRefs.erase(std::remove_if(ws.widgetRefs.begin(),
                                       ws.widgetRefs.end(),
                                       [&](const WidgetRef& r) {
                                         const int lost = datasetTypeCounts.value(r.widgetType, 0);
                                         if (lost == 0 || r.groupUniqueId != groupUid)
                                           return false;

                                         const int base = runningAtGroup.value(r.widgetType, 0);
                                         return r.relativeIndex >= base
                                             && r.relativeIndex < base + lost;
                                       }),
                        ws.widgetRefs.end());

    for (auto& r : ws.widgetRefs) {
      const int lost = datasetTypeCounts.value(r.widgetType, 0);
      if (lost == 0)
        continue;

      const int base = runningAtGroup.value(r.widgetType, 0);
      if (r.relativeIndex < base + lost)
        continue;

      r.relativeIndex -= lost;
      SS_ASSERT(r.relativeIndex >= 0, r.relativeIndex = 0);
    }
  }
}

/**
 * @brief Asks the user to confirm before deleting a workspace.
 */
void DataModel::ProjectModel::confirmDeleteWorkspace(int workspaceId)
{
  const QString name = workspaceTitle(workspaceId);
  if (name.isEmpty())
    return;

  const int choice = Misc::Utilities::showMessageBox(tr("Delete \"%1\"?").arg(name),
                                                     tr("This action cannot be undone."),
                                                     QMessageBox::Warning,
                                                     tr("Delete Workspace"),
                                                     QMessageBox::Yes | QMessageBox::Cancel,
                                                     QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteWorkspace(workspaceId);
}

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 */
void DataModel::ProjectModel::hideGroup(int groupId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (groupId < 0)
    return;

  if (m_hiddenGroupIds.contains(groupId))
    return;

  m_hiddenGroupIds.insert(groupId);

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectModel::showGroup(int groupId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_hiddenGroupIds.remove(groupId))
    return;

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}

/**
 * @brief Returns {id, title} entries for every hidden auto-group, in group order.
 */
QVariantList DataModel::ProjectModel::hiddenGroupsSummary() const
{
  QVariantList result;
  for (const auto& g : m_groups) {
    if (!m_hiddenGroupIds.contains(g.groupId))
      continue;

    QVariantMap entry;
    entry[QStringLiteral("id")]    = g.groupId;
    entry[QStringLiteral("title")] = g.title;
    result.append(entry);
  }

  return result;
}

/**
 * @brief Restores every hidden auto-group in one shot. No-op when none hidden.
 */
void DataModel::ProjectModel::showAllHiddenGroups()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_hiddenGroupIds.isEmpty())
    return;

  m_hiddenGroupIds.clear();

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  setModified(true);
  Q_EMIT editorWorkspacesChanged();
  Q_EMIT activeWorkspacesChanged();
}
