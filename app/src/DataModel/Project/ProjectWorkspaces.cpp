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

#include "DataModel/Project/ProjectWorkspaces.h"

#include <algorithm>
#include <QInputDialog>
#include <QJsonObject>
#include <QMessageBox>
#include <QTimer>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectPresentation.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/IconRegistry.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

namespace DataModel {

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

//--------------------------------------------------------------------------------------------------
// Construction & accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty workspace store in auto mode, bound to @p model.
 */
DataModel::ProjectWorkspaces::ProjectWorkspaces(ProjectModel& model)
  : m_model(model), m_customizeWorkspaces(false)
{}

/**
 * @brief Returns the editor-owned workspace list.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectWorkspaces::list() const noexcept
{
  return m_workspaces;
}

/**
 * @brief Mutable workspace list, for the folder tree's filing and reordering operations.
 */
std::vector<DataModel::Workspace>& DataModel::ProjectWorkspaces::mutableList() noexcept
{
  return m_workspaces;
}

/**
 * @brief Returns the workspace list currently rendered by the dashboard.
 */
const std::vector<DataModel::Workspace>& DataModel::ProjectWorkspaces::activeList() const
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return m_sessionWorkspaces;

  return m_workspaces;
}

/**
 * @brief Returns the number of workspaces defined in the project.
 */
int DataModel::ProjectWorkspaces::count() const noexcept
{
  return static_cast<int>(m_workspaces.size());
}

/**
 * @brief Returns the set of hidden auto-generated group IDs.
 */
const QSet<int>& DataModel::ProjectWorkspaces::hiddenGroupIds() const noexcept
{
  return m_hiddenGroupIds;
}

/**
 * @brief Returns true when the auto-generated group workspace is hidden.
 */
bool DataModel::ProjectWorkspaces::isGroupHidden(int groupId) const
{
  return m_hiddenGroupIds.contains(groupId);
}

/**
 * @brief Returns whether the user has opted in to customising workspaces.
 */
bool DataModel::ProjectWorkspaces::customizeWorkspaces() const noexcept
{
  return m_customizeWorkspaces;
}

//--------------------------------------------------------------------------------------------------
// Document-level state transitions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resets every workspace container to factory defaults (new document).
 */
void DataModel::ProjectWorkspaces::resetDocument()
{
  m_workspaces.clear();
  m_autoSnapshot.clear();
  m_hiddenGroupIds.clear();
  m_customizeWorkspaces = false;
}

/**
 * @brief Drops the workspace list without touching the customize flag (load path).
 */
void DataModel::ProjectWorkspaces::clearList()
{
  m_workspaces.clear();
}

/**
 * @brief Drops the hidden-group set (load path).
 */
void DataModel::ProjectWorkspaces::clearHiddenGroupIds()
{
  m_hiddenGroupIds.clear();
}

/**
 * @brief Records a hidden group id read straight from the project file.
 */
void DataModel::ProjectWorkspaces::insertHiddenGroupId(int groupId)
{
  m_hiddenGroupIds.insert(groupId);
}

/**
 * @brief Sets the customize flag exactly as the file declares it, without the seed/regenerate
 *        side effects of the interactive toggle: the loader fills the list itself, and an
 *        older-schema load deliberately forces the flag off.
 */
void DataModel::ProjectWorkspaces::setCustomizeFlagFromFile(bool enabled)
{
  m_customizeWorkspaces = enabled;
}

/**
 * @brief Re-bases the merge baseline on the current auto layout.
 */
void DataModel::ProjectWorkspaces::refreshAutoSnapshot()
{
  m_autoSnapshot = buildAutoWorkspaces();
}

/**
 * @brief Rebuilds the session (non-ProjectFile) workspace list from the live widget set.
 */
void DataModel::ProjectWorkspaces::rebuildSessionWorkspaces()
{
  m_sessionWorkspaces = buildAutoWorkspaces();
}

/**
 * @brief Drops the session workspace list when the project file takes over.
 */
void DataModel::ProjectWorkspaces::clearSessionWorkspaces()
{
  m_sessionWorkspaces.clear();
}

/**
 * @brief Clears hidden groups and the workspace list for a session with no backing project file;
 *        returns whether the workspace list actually changed so the caller can emit once.
 */
bool DataModel::ProjectWorkspaces::clearTransientState()
{
  m_hiddenGroupIds.clear();

  if (m_workspaces.empty())
    return false;

  m_workspaces.clear();
  return true;
}

/**
 * @brief Emits the pair of signals every workspace-list mutation owes its consumers.
 */
void DataModel::ProjectWorkspaces::notifyWorkspaceListChanged()
{
  Q_EMIT m_model.editorWorkspacesChanged();
  Q_EMIT m_model.activeWorkspacesChanged();
}

/**
 * @brief Flips the customize switch (Off->On seeds auto layout, On->Off re-seeds it).
 */
void DataModel::ProjectWorkspaces::setCustomizeWorkspaces(const bool enabled)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_customizeWorkspaces == enabled)
    return;

  m_customizeWorkspaces = enabled;

  if (enabled) {
    m_workspaces                                = buildAutoWorkspaces();
    m_model.m_folders.mutableWorkspaceFolders() = buildAutoWorkspaceFoldersFor(m_workspaces);
    m_autoSnapshot                              = m_workspaces;
  } else {
    m_model.m_folders.clearWorkspaceFolders();
    regenerateAutoWorkspacesUnnotified();
  }

  m_model.setModified(true);
  Q_EMIT m_model.customizeWorkspacesChanged();
  notifyWorkspaceListChanged();
}

//--------------------------------------------------------------------------------------------------
// Workspace CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new user-defined workspace with the given title.
 */
int DataModel::ProjectWorkspaces::addWorkspace(const QString& title)
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
  ws.title = title.simplified().isEmpty() ? ProjectModel::tr("Workspace") : title.simplified();
  m_workspaces.push_back(ws);

  m_model.setModified(true);
  notifyWorkspaceListChanged();
  return ws.workspaceId;
}

/**
 * @brief Deletes the workspace with the given ID.
 */
void DataModel::ProjectWorkspaces::deleteWorkspace(int workspaceId)
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
  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Wipes every workspace, leaving an empty customised list.
 */
void DataModel::ProjectWorkspaces::clearAllWorkspaces()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  if (m_workspaces.empty())
    return;

  m_workspaces.clear();
  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Renames the workspace with the given ID.
 */
void DataModel::ProjectWorkspaces::renameWorkspace(int workspaceId, const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  for (auto& ws : m_workspaces) {
    if (ws.workspaceId == workspaceId) {
      ws.title = title.simplified();
      m_model.setModified(true);
      notifyWorkspaceListChanged();
      return;
    }
  }
}

/**
 * @brief Patches title, icon, and/or description on the workspace with the given ID.
 */
void DataModel::ProjectWorkspaces::updateWorkspace(int workspaceId,
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

      m_model.setModified(true);
      notifyWorkspaceListChanged();
      return;
    }
  }
}

/**
 * @brief Convenience entry point that sets only the icon of a workspace.
 */
void DataModel::ProjectWorkspaces::setWorkspaceIcon(int workspaceId, const QString& icon)
{
  updateWorkspace(workspaceId, QString(), icon, QString(), false, true, false);
}

/**
 * @brief Reorders user-defined workspaces (id >= UserStart) by the given id
 * sequence, bailing out when the id set does not match the existing user
 * workspaces because a partial reorder would silently corrupt the list.
 */
void DataModel::ProjectWorkspaces::reorderWorkspaces(const QList<int>& userWorkspaceIds)
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

  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Moves a workspace to the given index in the editor list.
 *        Auto workspaces (id < UserStart) are pinned to the top and ignored.
 */
void DataModel::ProjectWorkspaces::moveWorkspace(int workspaceId, int targetIndex)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (workspaceId < WorkspaceIds::UserStart)
    return;

  if (!m_customizeWorkspaces)
    setCustomizeWorkspaces(true);

  auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(), [workspaceId](const auto& ws) {
    return ws.workspaceId == workspaceId;
  });

  if (it == m_workspaces.end())
    return;

  const int from = static_cast<int>(std::distance(m_workspaces.begin(), it));

  int firstUserSlot = 0;
  for (const auto& ws : m_workspaces) {
    if (ws.workspaceId >= WorkspaceIds::UserStart)
      break;

    firstUserSlot += 1;
  }

  const int last   = static_cast<int>(m_workspaces.size()) - 1;
  const int target = std::clamp(targetIndex, firstUserSlot, last);
  if (target == from)
    return;

  auto ws = *it;
  m_workspaces.erase(it);
  m_workspaces.insert(m_workspaces.begin() + target, ws);

  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Appends a widget reference to the specified workspace.
 */
void DataModel::ProjectWorkspaces::addWidgetToWorkspace(int workspaceId,
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

    m_model.setModified(true);
    notifyWorkspaceListChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference from the specified workspace by index.
 */
void DataModel::ProjectWorkspaces::removeWidgetFromWorkspace(int workspaceId, int index)
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
    m_model.setModified(true);
    notifyWorkspaceListChanged();
    return;
  }
}

/**
 * @brief Removes a widget reference matching (widgetType, groupId, relativeIndex).
 */
void DataModel::ProjectWorkspaces::removeWidgetFromWorkspace(int workspaceId,
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
    m_model.setModified(true);
    notifyWorkspaceListChanged();
    return;
  }
}

/**
 * @brief Drops every workspace widget ref whose encoded key isn't in validKeys.
 */
int DataModel::ProjectWorkspaces::cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys)
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

  m_model.setModified(true);
  notifyWorkspaceListChanged();
  return removed;
}

/**
 * @brief Returns the title of a workspace, or empty if not found.
 */
QString DataModel::ProjectWorkspaces::workspaceTitle(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.title;

  return QString();
}

/**
 * @brief Returns the icon of a workspace, or empty if not found.
 */
QString DataModel::ProjectWorkspaces::workspaceIcon(int workspaceId) const
{
  for (const auto& ws : m_workspaces)
    if (ws.workspaceId == workspaceId)
      return ws.icon;

  return QString();
}

//--------------------------------------------------------------------------------------------------
// QInputDialog wrappers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a new workspace name and creates it. The new workspace
 *        is then selected in the project editor.
 */
void DataModel::ProjectWorkspaces::promptAddWorkspace()
{
  bool ok            = false;
  const QString name = QInputDialog::getText(nullptr,
                                             ProjectModel::tr("New Workspace"),
                                             ProjectModel::tr("Name:"),
                                             QLineEdit::Normal,
                                             ProjectModel::tr("Workspace"),
                                             &ok);

  if (!ok || name.trimmed().isEmpty())
    return;

  const int newId = addWorkspace(name.trimmed());
  QTimer::singleShot(0, &m_model, [newId] {
    static auto& projectEditor = DataModel::ProjectEditor::instance();
    projectEditor.selectWorkspace(newId);
  });
}

/**
 * @brief Prompts for a new title for the given workspace.
 */
void DataModel::ProjectWorkspaces::promptRenameWorkspace(int workspaceId)
{
  const QString current = workspaceTitle(workspaceId);
  if (current.isEmpty())
    return;

  bool ok            = false;
  const QString name = QInputDialog::getText(nullptr,
                                             ProjectModel::tr("Rename Workspace"),
                                             ProjectModel::tr("Name:"),
                                             QLineEdit::Normal,
                                             current,
                                             &ok);

  if (!ok || name.trimmed().isEmpty() || name.trimmed() == current)
    return;

  renameWorkspace(workspaceId, name.trimmed());
}

/**
 * @brief Asks the user to confirm before deleting a workspace.
 */
void DataModel::ProjectWorkspaces::confirmDeleteWorkspace(int workspaceId)
{
  const QString name = workspaceTitle(workspaceId);
  if (name.isEmpty())
    return;

  const int choice =
    Misc::Utilities::showMessageBox(ProjectModel::tr("Delete \"%1\"?").arg(name),
                                    ProjectModel::tr("This action cannot be undone."),
                                    QMessageBox::Warning,
                                    ProjectModel::tr("Delete Workspace"),
                                    QMessageBox::Yes | QMessageBox::Cancel,
                                    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteWorkspace(workspaceId);
}

//--------------------------------------------------------------------------------------------------
// Auto-layout synthesis
//--------------------------------------------------------------------------------------------------

/**
 * @brief Synthesises the default workspace layout for a project.
 */
std::vector<DataModel::Workspace> DataModel::ProjectWorkspaces::buildAutoWorkspaces() const
{
  std::vector<DataModel::Workspace> result;

  static auto& appState     = AppState::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  const auto mode = appState.operationMode();
  std::vector<DataModel::Group> quickPlotGroups;
  if (mode == SerialStudio::QuickPlot)
    frameBuilder.invokeOnBuilderThreadBlocking(
      [&] { quickPlotGroups = frameBuilder.quickPlotFrame().groups; });

  const auto& groups = (mode == SerialStudio::QuickPlot) ? quickPlotGroups : m_model.m_groups;

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
    ws.title       = ProjectModel::tr("Overview");
    ws.icon        = registry.iconById(QStringLiteral("panes/overview"), 32);
    ws.widgetRefs  = overviewRefs;
    result.push_back(std::move(ws));
  }

  if (eligibleGroups >= 2) {
    DataModel::Workspace ws;
    ws.workspaceId = WorkspaceIds::AllData;
    ws.title       = ProjectModel::tr("All Data");
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
void DataModel::ProjectWorkspaces::appendAutoGroupWorkspaces(
  std::vector<DataModel::Workspace>& result,
  const std::vector<DataModel::Group>& groups,
  const QMap<int, std::vector<DataModel::WidgetRef>>& perGroupRefs) const
{
  QHash<int, int> folderParent;
  QHash<int, QString> folderTitle;
  QSet<int> containerFolders;
  for (const auto& f : m_model.m_folders.groupFolders()) {
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
std::vector<DataModel::WorkspaceFolder> DataModel::ProjectWorkspaces::buildAutoWorkspaceFoldersFor(
  const std::vector<DataModel::Workspace>& workspaces) const
{
  std::vector<DataModel::WorkspaceFolder> result;
  const auto& groupFolders = m_model.m_folders.groupFolders();
  if (groupFolders.empty())
    return result;

  QHash<int, int> parentOf;
  for (const auto& f : std::as_const(groupFolders))
    parentOf.insert(f.folderId, f.parentFolderId);

  QSet<int> needed;
  const int kMax = static_cast<int>(groupFolders.size());
  for (const auto& ws : workspaces) {
    int id = ws.parentFolderId;
    for (int i = 0; i <= kMax && id != -1 && !needed.contains(id); ++i) {
      needed.insert(id);
      id = parentOf.value(id, -1);
    }
  }

  for (const auto& f : groupFolders) {
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
 * @brief Refreshes the workspace list from the project structure WITHOUT emitting signals.
 */
void DataModel::ProjectWorkspaces::regenerateAutoWorkspacesUnnotified()
{
  if (m_customizeWorkspaces)
    return;

  m_workspaces                                = buildAutoWorkspaces();
  m_model.m_folders.mutableWorkspaceFolders() = buildAutoWorkspaceFoldersFor(m_workspaces);
  m_autoSnapshot                              = m_workspaces;
}

/**
 * @brief Merges newly-eligible auto refs into the user-customised workspace list.
 */
bool DataModel::ProjectWorkspaces::mergeAutoWorkspaceUpdates()
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
    m_model.m_folders.sanitizeWorkspaceFolders();
    m_model.setModified(true);
  }

  return dirty;
}

/**
 * @brief Materialises the synthetic workspace list and flips the project into customize mode so
 *        the user can edit it from there.
 */
int DataModel::ProjectWorkspaces::autoGenerateWorkspaces()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return -1;

  if (m_customizeWorkspaces && !m_workspaces.empty())
    return m_workspaces.front().workspaceId;

  auto seed = buildAutoWorkspaces();
  if (seed.empty())
    return -1;

  m_workspaces                                = std::move(seed);
  m_model.m_folders.mutableWorkspaceFolders() = buildAutoWorkspaceFoldersFor(m_workspaces);
  m_autoSnapshot                              = m_workspaces;
  const bool flagChanged                      = !m_customizeWorkspaces;
  m_customizeWorkspaces                       = true;

  SS_ASSERT(!m_workspaces.empty(), return -1);
  SS_ASSERT(m_workspaces.front().workspaceId >= WorkspaceIds::AutoStart, return -1);

  m_model.setModified(true);
  if (flagChanged)
    Q_EMIT m_model.customizeWorkspacesChanged();

  notifyWorkspaceListChanged();
  return m_workspaces.front().workspaceId;
}

/**
 * @brief Drops user customisations and returns the project to the synthetic
 *        auto-layout. Idempotent: a no-op when already in auto mode.
 */
void DataModel::ProjectWorkspaces::resetWorkspacesToAuto()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  m_customizeWorkspaces = false;
  m_model.m_folders.clearWorkspaceFolders();
  regenerateAutoWorkspacesUnnotified();

  m_model.setModified(true);
  Q_EMIT m_model.customizeWorkspacesChanged();
  notifyWorkspaceListChanged();
}

/**
 * @brief Asks the user to confirm before discarding workspace customisations.
 */
void DataModel::ProjectWorkspaces::confirmResetWorkspacesToAuto()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_customizeWorkspaces)
    return;

  if (m_model.suppressMessageBoxes()) {
    resetWorkspacesToAuto();
    return;
  }

  const int choice = Misc::Utilities::showMessageBox(
    ProjectModel::tr("Discard workspace customisations?"),
    ProjectModel::tr("Switching off Customize discards your edits and rebuilds the "
                     "workspace list from the project's groups."),
    QMessageBox::Warning,
    ProjectModel::tr("Customize Workspaces"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    resetWorkspacesToAuto();
}

//--------------------------------------------------------------------------------------------------
// Hidden groups
//--------------------------------------------------------------------------------------------------

/**
 * @brief Hides an auto-generated group workspace from the workspace list.
 */
void DataModel::ProjectWorkspaces::hideGroup(int groupId)
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

  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Restores a previously hidden auto-generated group workspace.
 */
void DataModel::ProjectWorkspaces::showGroup(int groupId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (!m_hiddenGroupIds.remove(groupId))
    return;

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Restores every hidden auto-group in one shot. No-op when none hidden.
 */
void DataModel::ProjectWorkspaces::showAllHiddenGroups()
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_hiddenGroupIds.isEmpty())
    return;

  m_hiddenGroupIds.clear();

  if (!m_customizeWorkspaces)
    regenerateAutoWorkspacesUnnotified();

  m_model.setModified(true);
  notifyWorkspaceListChanged();
}

/**
 * @brief Returns {id, title} entries for every hidden auto-group, in group order.
 */
QVariantList DataModel::ProjectWorkspaces::hiddenGroupsSummary() const
{
  QVariantList result;
  for (const auto& g : m_model.m_groups) {
    if (!m_hiddenGroupIds.contains(g.groupId))
      continue;

    QVariantMap entry;
    entry[QStringLiteral("id")]    = g.groupId;
    entry[QStringLiteral("title")] = g.title;
    result.append(entry);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Ref repair after a structural change
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shifts or drops user-customised widget refs after a group delete.
 */
void DataModel::ProjectWorkspaces::shiftWorkspaceRefsAfterGroupDelete(
  int deletedGid, const QMap<int, int>& deletedTypeCounts)
{
  SS_ASSERT(m_customizeWorkspaces, return);
  WorkspaceRefs::shiftRefsAfterGroupDelete(
    m_workspaces, m_model.m_groups, deletedGid, deletedTypeCounts);
}

/**
 * @brief Shifts user-customised widget refs after a single dataset is deleted from a surviving
 *        group.
 */
void DataModel::ProjectWorkspaces::shiftWorkspaceRefsAfterDatasetDelete(
  int groupId, const QMap<int, int>& datasetTypeCounts)
{
  SS_ASSERT(m_customizeWorkspaces, return);
  const int groupUid = m_model.groupUniqueIdForGroupId(groupId);
  WorkspaceRefs::shiftRefsAfterDatasetDelete(
    m_workspaces, m_model.m_groups, groupId, groupUid, datasetTypeCounts);
}

/**
 * @brief Updates layout:N widgetSettings entries after a group renumber.
 */
void DataModel::ProjectWorkspaces::shiftLayoutKeysAfterGroupDelete(int deletedGid)
{
  auto& widgetSettings = m_model.m_presentation.mutableWidgetSettings();
  if (WorkspaceRefs::shiftLayoutKeysAfterGroupDelete(widgetSettings, deletedGid))
    Q_EMIT m_model.widgetSettingsChanged();
}

/**
 * @brief Rewrites every layout:N widgetSettings entry to use the new groupId.
 */
void DataModel::ProjectWorkspaces::remapLayoutKeysAfterReorder(const std::vector<int>& oldToNewGid)
{
  WorkspaceRefs::remapLayoutKeysAfterReorder(m_model.m_presentation.mutableWidgetSettings(),
                                             oldToNewGid);
}

/**
 * @brief Snapshots one anchor per workspace ref before a reorder, keyed by workspaceId so
 *        the buckets survive any reordering of the workspace list between snapshot and resolve.
 */
DataModel::ProjectWorkspaces::RefAnchors DataModel::ProjectWorkspaces::snapshotRefAnchors() const
{
  return WorkspaceRefs::snapshotRefAnchors(m_workspaces, m_model.m_groups);
}

/**
 * @brief Re-resolves every workspace against its own snapshot bucket, paired by workspaceId
 *        rather than by list position.
 */
void DataModel::ProjectWorkspaces::resolveRefAnchors(const RefAnchors& anchors)
{
  WorkspaceRefs::resolveRefAnchors(m_workspaces, anchors, m_model.m_groups);
}
