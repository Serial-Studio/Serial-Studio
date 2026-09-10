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

#include "UI/Taskbar/TaskbarWorkspaces.h"

#include <algorithm>
#include <QTimer>
#include <QVariantMap>

#include "AppState.h"
#include "Core/IconRegistry.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "Misc/IconEngine.h"
#include "UI/Dashboard.h"
#include "UI/Taskbar.h"
#include "UI/Taskbar/TaskbarModel.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Grace period so a workspace switch finishes incubating its tiles before one is revealed.
constexpr int kRevealDelayMs = 100;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the workspace concern to its facade and to the models it projects.
 */
UI::TaskbarWorkspaces::TaskbarWorkspaces(Taskbar& taskbar,
                                         UI::Dashboard& dashboard,
                                         DataModel::ProjectModel& projectModel,
                                         AppState& appState,
                                         Misc::IconRegistry& iconRegistry,
                                         QObject* parent)
  : QObject(parent)
  , m_taskbar(taskbar)
  , m_dashboard(dashboard)
  , m_projectModel(projectModel)
  , m_appState(appState)
  , m_iconRegistry(iconRegistry)
{}

//--------------------------------------------------------------------------------------------------
// Switcher model projections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the flat workspace model the workspace selector binds to.
 */
QVariantList UI::TaskbarWorkspaces::model() const
{
  QVariantList model;
  const auto& workspaces = m_projectModel.activeWorkspaces();
  for (const auto& ws : workspaces) {
    QVariantMap entry;
    const bool fixedIcon = ws.icon.isEmpty();
    const auto icon =
      fixedIcon ? m_iconRegistry.icon(QStringLiteral("widgets"), QStringLiteral("workspace"), 16)
                : Misc::IconEngine::resolveActionIconSource(ws.icon);
    entry[QStringLiteral("id")]        = ws.workspaceId;
    entry[QStringLiteral("text")]      = ws.title;
    entry[QStringLiteral("separator")] = false;
    entry[QStringLiteral("icon")]      = icon;
    entry[QStringLiteral("iconId")] = fixedIcon ? QStringLiteral("widgets/workspace") : QString();
    model.append(entry);
  }

  return model;
}

/**
 * @brief Recursively builds one level of the workspace folder tree, skipping empty workspaces
 *        and folders whose subtree resolves to nothing.
 */
[[nodiscard]] static QVariantList buildWorkspaceTreeLevel(
  int parentFolderId,
  Misc::IconRegistry& registry,
  const std::vector<DataModel::Workspace>& workspaces,
  const std::vector<DataModel::WorkspaceFolder>& folders)
{
  QVariantList level;

  for (const auto& f : folders) {
    if (f.parentFolderId != parentFolderId)
      continue;

    const auto children = buildWorkspaceTreeLevel(f.folderId, registry, workspaces, folders);
    if (children.isEmpty())
      continue;

    QVariantMap node;
    node[QStringLiteral("isFolder")] = true;
    node[QStringLiteral("id")]       = f.folderId;
    node[QStringLiteral("text")]     = f.title;
    node[QStringLiteral("icon")] =
      registry.icon(QStringLiteral("widgets"), QStringLiteral("folder"), 16);
    node[QStringLiteral("iconId")]   = QStringLiteral("widgets/folder");
    node[QStringLiteral("children")] = children;
    level.append(node);
  }

  for (const auto& ws : workspaces) {
    if (ws.parentFolderId != parentFolderId || ws.widgetRefs.empty())
      continue;

    QVariantMap node;
    const bool fixedIcon             = ws.icon.isEmpty();
    const auto icon                  = fixedIcon
                                       ? registry.icon(QStringLiteral("widgets"), QStringLiteral("workspace"), 16)
                                       : Misc::IconEngine::resolveActionIconSource(ws.icon);
    node[QStringLiteral("isFolder")] = false;
    node[QStringLiteral("id")]       = ws.workspaceId;
    node[QStringLiteral("text")]     = ws.title;
    node[QStringLiteral("icon")]     = icon;
    node[QStringLiteral("iconId")]   = fixedIcon ? QStringLiteral("widgets/workspace") : QString();
    node[QStringLiteral("children")] = QVariantList();
    level.append(node);
  }

  return level;
}

/**
 * @brief Returns the workspace switcher model as a folder -> children tree.
 */
QVariantList UI::TaskbarWorkspaces::tree() const
{
  const auto& workspaces = m_projectModel.activeWorkspaces();

  const std::vector<DataModel::WorkspaceFolder> noFolders;
  const bool projectMode = m_appState.operationMode() == SerialStudio::ProjectFile;
  const auto& folders    = projectMode ? m_projectModel.editorWorkspaceFolders() : noFolders;
  return buildWorkspaceTreeLevel(-1, m_iconRegistry, workspaces, folders);
}

/**
 * @brief Returns whether @p workspaceId is present in the switcher model.
 */
bool UI::TaskbarWorkspaces::contains(int workspaceId) const
{
  const auto list  = model();
  const auto match = [workspaceId](const QVariant& v) {
    return v.toMap().value(QStringLiteral("id")).toInt() == workspaceId;
  };

  return workspaceId >= 0 && std::any_of(list.begin(), list.end(), match);
}

/**
 * @brief Returns the position of @p groupId in the workspace model, or -1 if absent.
 */
int UI::TaskbarWorkspaces::indexForGroupId(int groupId) const
{
  const auto list = model();
  int index       = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    const auto map = it->toMap();
    if (!map.contains(QStringLiteral("id")))
      continue;

    if (map.value(QStringLiteral("id")).toInt() == groupId)
      return index;

    ++index;
  }

  return -1;
}

/**
 * @brief Resolves both ends of a workspace switch in one model pass, so the pre-switch signal
 *        the facade emits never rebuilds the workspace model twice per switch.
 */
QPair<int, int> UI::TaskbarWorkspaces::changeIndices(int fromGroupId, int toGroupId) const
{
  const auto list = model();
  int fromIndex   = -1;
  int toIndex     = -1;
  int index       = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    const auto map = it->toMap();
    if (!map.contains(QStringLiteral("id")))
      continue;

    const int id = map.value(QStringLiteral("id")).toInt();
    if (id == fromGroupId)
      fromIndex = index;

    if (id == toGroupId)
      toIndex = index;

    ++index;
  }

  return {fromIndex, toIndex};
}

/**
 * @brief Picks the workspace to select after a model rebuild: the independent window's own
 *        choice first, then the project's saved group, then the first entry. Returns nullopt
 *        when there is no workspace to select at all.
 */
std::optional<int> UI::TaskbarWorkspaces::selectionAfterRebuild(bool independent,
                                                                int desiredGroupId,
                                                                int activeGroupId) const
{
  const auto list = model();
  if (list.isEmpty())
    return std::nullopt;

  const auto inList = [&list](int gid) {
    return gid >= 0 && std::any_of(list.begin(), list.end(), [gid](const QVariant& v) {
             return v.toMap().value(QStringLiteral("id")).toInt() == gid;
           });
  };

  if (independent && inList(desiredGroupId))
    return desiredGroupId;

  if (independent && inList(activeGroupId))
    return activeGroupId;

  const bool projectMode = m_appState.operationMode() == SerialStudio::ProjectFile;
  const int savedId      = m_projectModel.activeGroupId();
  if (projectMode && !independent && inList(savedId))
    return savedId;

  if (!list.first().canConvert<QVariantMap>())
    return -1;

  const QVariantMap first = list.first().toMap();
  return first.contains(QStringLiteral("id")) ? first[QStringLiteral("id")].toInt() : -1;
}

//--------------------------------------------------------------------------------------------------
// Widget reference resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves a stored workspace widget reference to its live windowId, or -1 if absent.
 */
int UI::TaskbarWorkspaces::resolveRefWindowId(const DataModel::WidgetRef& ref) const
{
  const auto& map    = m_taskbar.windowMap();
  const int windowId = map.findWindowIdByGroupAndIndex(ref.widgetType, ref.relativeIndex);
  if (windowId < 0)
    return -1;

  const int refGid = m_dashboard.groupIdForUniqueId(ref.groupUniqueId);
  auto* item       = m_taskbar.findItemByWindowId(windowId);
  if (!item || item->data(TaskbarModel::GroupIdRole).toInt() != refGid)
    return -1;

  return windowId;
}

/**
 * @brief Returns window IDs of all widgets in the given user workspace, empty for non-user IDs.
 */
QVariantList UI::TaskbarWorkspaces::widgetIds(int workspaceId) const
{
  QVariantList ids;
  if (workspaceId < WorkspaceIds::AutoStart)
    return ids;

  const auto& workspaces = m_projectModel.activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const int windowId = resolveRefWindowId(ref);
      if (windowId >= 0)
        ids.append(windowId);
    }

    break;
  }

  return ids;
}

/**
 * @brief Returns the user workspace containing windowId (active preferred), else -1.
 */
int UI::TaskbarWorkspaces::workspaceContainingWidget(int windowId) const
{
  const auto& workspaces = m_projectModel.activeWorkspaces();

  int firstMatch = -1;
  for (const auto& ws : workspaces) {
    if (ws.workspaceId < WorkspaceIds::AutoStart)
      continue;

    const auto matches = [this, windowId](const DataModel::WidgetRef& ref) {
      return resolveRefWindowId(ref) == windowId;
    };

    if (!std::any_of(ws.widgetRefs.begin(), ws.widgetRefs.end(), matches))
      continue;

    if (ws.workspaceId == m_taskbar.activeGroupId())
      return ws.workspaceId;

    if (firstMatch < 0)
      firstMatch = ws.workspaceId;
  }

  return firstMatch;
}

//--------------------------------------------------------------------------------------------------
// Taskbar row projection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates the facade's taskbar buttons for a user-defined workspace (id >= 5000).
 */
void UI::TaskbarWorkspaces::populateTaskbar(int groupId)
{
  auto* buttons = m_taskbar.taskbarButtons();
  SS_ASSERT(buttons != nullptr, return);

  const auto& workspaces = m_projectModel.activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != groupId)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const int windowId = resolveRefWindowId(ref);
      auto* item         = windowId < 0 ? nullptr : m_taskbar.findItemByWindowId(windowId);
      if (!item)
        continue;

      auto* clone = item->clone();
      m_taskbar.setWindowState(windowId, TaskbarModel::WindowNormal);
      buttons->appendRow(clone);
    }

    return;
  }
}

/**
 * @brief Removes the taskbar-button row matching windowId, if any.
 */
void UI::TaskbarWorkspaces::removeTaskbarRow(int windowId)
{
  auto* buttons = m_taskbar.taskbarButtons();
  SS_ASSERT(buttons != nullptr, return);

  for (int r = 0; r < buttons->rowCount(); ++r) {
    auto* row = buttons->item(r);
    if (!row || row->data(TaskbarModel::WindowIdRole).toInt() != windowId)
      continue;

    buttons->removeRow(r);
    return;
  }
}

/**
 * @brief Brings the tile for windowId on screen and asks the UI to highlight it.
 */
void UI::TaskbarWorkspaces::revealWindow(int windowId)
{
  if (auto* window = m_taskbar.windowData(windowId)) {
    m_taskbar.showWindow(window);
    m_taskbar.setActiveWindow(window);
  }

  Q_EMIT highlightRequested(windowId);
}

//--------------------------------------------------------------------------------------------------
// Membership bookkeeping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the position of the widget reference matching the given identity inside
 *        @p workspaceId, or -1 when the workspace or the reference does not exist.
 */
int UI::TaskbarWorkspaces::indexOfWidgetRef(int workspaceId,
                                            int widgetType,
                                            int groupUniqueId,
                                            int relativeIndex) const
{
  const auto& workspaces = m_projectModel.activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (size_t i = 0; i < ws.widgetRefs.size(); ++i) {
      const auto& ref = ws.widgetRefs[i];
      if (ref.widgetType == widgetType && ref.groupUniqueId == groupUniqueId
          && ref.relativeIndex == relativeIndex)
        return static_cast<int>(i);
    }

    return -1;
  }

  return -1;
}

/**
 * @brief Drops every widget reference of @p workspaceId, reporting whether the workspace exists.
 */
bool UI::TaskbarWorkspaces::clearWorkspaceWidgets(int workspaceId)
{
  const auto& workspaces = m_projectModel.activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (int i = static_cast<int>(ws.widgetRefs.size()) - 1; i >= 0; --i)
      m_projectModel.removeWidgetFromWorkspace(workspaceId, i);

    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Workspace mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Navigates to the widget: reveals it in place, adds it to the active user
 *        workspace when allowAddToWorkspace is set, else activates its own group.
 */
void UI::TaskbarWorkspaces::navigateToWidget(int windowId, int groupId, bool allowAddToWorkspace)
{
  auto* buttons = m_taskbar.taskbarButtons();
  SS_ASSERT(buttons != nullptr, return);

  for (int i = 0; i < buttons->rowCount(); ++i) {
    auto* row = buttons->item(i);
    if (!row || row->data(TaskbarModel::WindowIdRole).toInt() != windowId)
      continue;

    revealWindow(windowId);
    return;
  }

  const bool addToWorkspace = allowAddToWorkspace && !m_dashboard.frozen()
                           && m_taskbar.activeGroupId() >= WorkspaceIds::AutoStart;
  if (addToWorkspace)
    addWidgetToActiveWorkspace(windowId);

  else
    m_taskbar.setActiveGroupId(groupId);

  QTimer::singleShot(kRevealDelayMs, this, [this, windowId] { revealWindow(windowId); });
}

/**
 * @brief Creates a new user-defined workspace and switches to it.
 */
void UI::TaskbarWorkspaces::createWorkspace(const QString& name)
{
  m_projectModel.addWorkspace(name);

  const auto& workspaces = m_projectModel.activeWorkspaces();
  if (!workspaces.empty())
    m_taskbar.setActiveGroupId(workspaces.back().workspaceId);

  Q_EMIT workspacesChanged();
}

/**
 * @brief Deletes a user-defined workspace, or hides the group behind an auto-generated one;
 *        anything outside those two id ranges is not deletable and is ignored.
 */
void UI::TaskbarWorkspaces::deleteWorkspace(int workspaceId)
{
  const bool customized = m_projectModel.customizeWorkspaces();
  const int floorId     = customized ? WorkspaceIds::AutoStart : WorkspaceIds::PerGroupStart;
  if (workspaceId < floorId)
    return;

  if (customized)
    m_projectModel.deleteWorkspace(workspaceId);

  else
    m_projectModel.hideGroup(workspaceId - WorkspaceIds::PerGroupStart);

  if (m_taskbar.activeGroupId() == workspaceId) {
    const auto list = model();
    if (!list.isEmpty())
      m_taskbar.setActiveGroupId(list.first().toMap().value(QStringLiteral("id"), -1).toInt());
  }

  Q_EMIT workspacesChanged();
}

/**
 * @brief Renames a user-defined workspace.
 */
void UI::TaskbarWorkspaces::renameWorkspace(int workspaceId, const QString& name)
{
  if (workspaceId < WorkspaceIds::AutoStart)
    return;

  m_projectModel.renameWorkspace(workspaceId, name);
  Q_EMIT workspacesChanged();
}

/**
 * @brief Adds the widget identified by windowId to the active workspace.
 */
void UI::TaskbarWorkspaces::addWidgetToActiveWorkspace(int windowId)
{
  const int activeId = m_taskbar.activeGroupId();
  if (activeId < WorkspaceIds::AutoStart)
    return;

  auto* item = m_taskbar.findItemByWindowId(windowId);
  if (!item)
    return;

  const int relIdx = m_taskbar.windowMap().relativeIndexForWindow(windowId);
  if (relIdx < 0)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();
  const int groupUid    = m_dashboard.groupUniqueIdForGroupId(groupId);
  m_projectModel.addWidgetToWorkspace(activeId, widgetType, groupUid, relIdx);

  auto* buttons = m_taskbar.taskbarButtons();
  SS_ASSERT(buttons != nullptr, return);

  auto* clone = item->clone();
  m_taskbar.setWindowState(clone->data(TaskbarModel::WindowIdRole).toInt(),
                           TaskbarModel::WindowNormal);
  buttons->appendRow(clone);
  Q_EMIT taskbarRowsChanged();
}

/**
 * @brief Removes the widget identified by windowId from the active workspace.
 */
void UI::TaskbarWorkspaces::removeWidgetFromActiveWorkspace(int windowId)
{
  const int activeId = m_taskbar.activeGroupId();
  if (activeId < WorkspaceIds::AutoStart)
    return;

  auto* item = m_taskbar.findItemByWindowId(windowId);
  if (!item)
    return;

  const int relIdx = m_taskbar.windowMap().relativeIndexForWindow(windowId);
  if (relIdx < 0)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();
  const int targetUid   = m_dashboard.groupUniqueIdForGroupId(groupId);

  const int refIndex = indexOfWidgetRef(activeId, widgetType, targetUid, relIdx);
  if (refIndex < 0)
    return;

  m_projectModel.removeWidgetFromWorkspace(activeId, refIndex);
  if (auto* window = m_taskbar.windowData(windowId))
    m_taskbar.unregisterWindow(window);

  removeTaskbarRow(windowId);
  Q_EMIT taskbarRowsChanged();
}

/**
 * @brief Replaces the widget list of a user workspace with the given IDs.
 */
void UI::TaskbarWorkspaces::setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds)
{
  if (workspaceId < WorkspaceIds::AutoStart || !clearWorkspaceWidgets(workspaceId))
    return;

  for (const auto& idVar : windowIds) {
    const int windowId = idVar.toInt();
    auto* item         = m_taskbar.findItemByWindowId(windowId);
    const int relIdx   = m_taskbar.windowMap().relativeIndexForWindow(windowId);
    if (!item || relIdx < 0)
      continue;

    const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
    const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();
    const int groupUid    = m_dashboard.groupUniqueIdForGroupId(groupId);
    m_projectModel.addWidgetToWorkspace(workspaceId, widgetType, groupUid, relIdx);
  }

  if (m_taskbar.activeGroupId() == workspaceId)
    m_taskbar.setActiveGroupId(workspaceId);
}
