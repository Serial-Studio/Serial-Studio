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

#include "Taskbar.h"

#include <QSignalBlocker>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "Misc/IconRegistry.h"
#include "UI/Dashboard.h"
#include "UI/UISessionRegistry.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"
#include "UI/WindowManager.h"

/**
 * @brief Returns the taskbar artwork for a widget: an extension widget takes the icon its package
 *        declared, everything else keeps the built-in table.
 */
[[nodiscard]] static QString taskbarIcon(SerialStudio::DashboardWidget type,
                                         const QString& extensionId,
                                         const bool large)
{
  if (type == SerialStudio::DashboardExtension) {
    static auto& catalog = UI::WidgetExtensions::instance();
    const auto art       = catalog.iconUrl(extensionId, large ? 32 : 16);
    if (!art.isEmpty())
      return art;
  }

  return SerialStudio::dashboardWidgetIcon(type, large);
}

/**
 * @brief Returns the icon-registry id a taskbar row re-resolves its own artwork tier from. A
 *        package that ships its icon as a file has none, so the row keeps the resolved path only.
 */
[[nodiscard]] static QString taskbarIconId(SerialStudio::DashboardWidget type,
                                           const QString& extensionId)
{
  if (type == SerialStudio::DashboardExtension) {
    static auto& catalog = UI::WidgetExtensions::instance();
    const auto& package  = catalog.descriptor(extensionId);
    if (!package.iconId.isEmpty() && !package.iconId.contains(QStringLiteral(".")))
      return package.iconId;
  }

  return SerialStudio::dashboardWidgetIconId(type);
}

//--------------------------------------------------------------------------------------------------
// Taskbar class implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Private constructor for the Taskbar singleton.
 */
UI::Taskbar::Taskbar(QQuickItem* parent)
  : QQuickItem(parent)
  , m_dashboard(UI::Dashboard::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_uiSessionRegistry(UISessionRegistry::instance())
  , m_widgetRegistry(WidgetRegistry::instance())
  , m_appState(AppState::instance())
  , m_activeGroupId(-1)
  , m_desiredGroupId(-1)
  , m_rebuildInProgress(false)
  , m_batchUpdateInProgress(false)
  , m_restoringLayout(false)
  , m_independentWorkspace(false)
  , m_activeWindow(nullptr)
  , m_windowManager(nullptr)
  , m_fullModel(new TaskbarModel(this))
  , m_taskbarButtons(new TaskbarModel(this))
  , m_windowMap(m_dashboard, m_widgetRegistry)
  , m_focusCycler()
  , m_search(m_fullModel)
  , m_workspaces(*this, m_dashboard, m_projectModel, m_appState, Misc::IconRegistry::instance())
{
  qmlRegisterUncreatableType<UI::TaskbarModel>(
    "SerialStudio.UI", 1, 0, "TaskbarModel", "TaskbarModel is exposed by Taskbar singleton");

  connect(&m_focusCycler, &UI::FocusCycler::focusRequested, this, &UI::Taskbar::setActiveWindow);
  connect(
    &m_focusCycler, &UI::FocusCycler::focusRefreshRequested, this, &UI::Taskbar::refocusWindow);
  connect(
    &m_focusCycler, &UI::FocusCycler::focusCleared, this, [this] { setActiveWindow(nullptr); });

  connect(&m_search, &UI::TaskbarSearch::dismissed, this, &UI::Taskbar::searchDismissed);
  connect(&m_search, &UI::TaskbarSearch::filterChanged, this, &UI::Taskbar::searchFilterChanged);
  connect(&m_search, &UI::TaskbarSearch::resultsChanged, this, &UI::Taskbar::searchResultsChanged);

  connect(&m_workspaces,
          &UI::TaskbarWorkspaces::workspacesChanged,
          this,
          &UI::Taskbar::workspaceModelChanged);
  connect(&m_workspaces,
          &UI::TaskbarWorkspaces::taskbarRowsChanged,
          this,
          &UI::Taskbar::taskbarButtonsChanged);
  connect(
    &m_workspaces, &UI::TaskbarWorkspaces::highlightRequested, this, &UI::Taskbar::highlightWidget);

  connectToRegistry();

  // code-verify off
  // Queued: Dashboard emits both mid-reconfigureDashboard on the display tick, so a Direct rebuild
  // reenters a half-built layout and incubates a QtGraphs widget whose polish loop never converges.
  connect(&m_dashboard,
          &UI::Dashboard::dataReset,
          this,
          &UI::Taskbar::rebuildModel,
          Qt::QueuedConnection);
  connect(&m_dashboard,
          &UI::Dashboard::widgetCountChanged,
          this,
          &UI::Taskbar::rebuildModel,
          Qt::QueuedConnection);
  // code-verify on

  auto* pm = &m_projectModel;
  connect(pm, &DataModel::ProjectModel::activeGroupIdChanged, this, [this, pm] {
    if (m_independentWorkspace)
      return;

    setActiveGroupId(pm->activeGroupId());
  });

  connect(pm,
          &DataModel::ProjectModel::activeWorkspacesChanged,
          this,
          &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::searchResultsChanged);

  connect(this, &UI::Taskbar::workspaceModelChanged, this, [this] {
    if (m_independentWorkspace && !m_rebuildInProgress)
      setDesiredGroupId(m_desiredGroupId);
  });

  rebuildModel();

  m_uiSessionRegistry.registerTaskbar(this);
}

/**
 * @brief Destroys the Taskbar and unregisters it from the UI session registry.
 */
UI::Taskbar::~Taskbar()
{
  m_uiSessionRegistry.unregisterTaskbar(this);
}

/**
 * @brief Connects the Taskbar to the WidgetRegistry's lifecycle signals.
 */
void UI::Taskbar::connectToRegistry()
{
  connect(&m_widgetRegistry, &WidgetRegistry::widgetCreated, this, &Taskbar::onWidgetCreated);
  connect(&m_widgetRegistry, &WidgetRegistry::widgetUpdated, this, &Taskbar::onWidgetUpdated);
  connect(&m_widgetRegistry, &WidgetRegistry::widgetDestroyed, this, &Taskbar::onWidgetDestroyed);
  connect(&m_widgetRegistry, &WidgetRegistry::registryCleared, this, &Taskbar::onRegistryCleared);
}

//--------------------------------------------------------------------------------------------------
// Taskbar class getter functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the currently active group ID.
 */
int UI::Taskbar::activeGroupId() const
{
  return m_activeGroupId;
}

/**
 * @brief Returns the workspace-model index of the active group, or -1 when none matches.
 */
int UI::Taskbar::activeGroupIndex() const
{
  return m_workspaces.indexForGroupId(m_activeGroupId);
}

/**
 * @brief Returns whether this taskbar tracks its own workspace independently of the project.
 */
bool UI::Taskbar::independentWorkspace() const
{
  return m_independentWorkspace;
}

/**
 * @brief Returns the layout-scope key used to namespace this taskbar's saved layouts.
 */
QString UI::Taskbar::layoutScope() const
{
  return m_layoutScope;
}

/**
 * @brief Returns the currently active QML window widget, if any.
 */
QQuickItem* UI::Taskbar::activeWindow() const
{
  return m_activeWindow;
}

/**
 * @brief Returns the complete dashboard model containing all groups and widgets.
 */
UI::TaskbarModel* UI::Taskbar::fullModel() const
{
  return m_fullModel;
}

/**
 * @brief Returns the filtered model currently shown in the taskbar.
 */
UI::TaskbarModel* UI::Taskbar::taskbarButtons() const
{
  return m_taskbarButtons;
}

/**
 * @brief Returns a pointer to the window manager object.
 */
UI::WindowManager* UI::Taskbar::windowManager() const
{
  return m_windowManager;
}

/**
 * @brief Returns the widget-id <-> window-id map, whose ownership stays with the taskbar.
 */
const UI::TaskbarWindowMap& UI::Taskbar::windowMap() const
{
  return m_windowMap;
}

/**
 * @brief Checks if any tracked window is maximized.
 */
bool UI::Taskbar::hasMaximizedWindow() const
{
  for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it)
    if (it.key()->state() == QStringLiteral("maximized"))
      return true;

  return false;
}

/**
 * @brief Returns the QML window (QQuickItem) for a given window ID.
 */
QQuickItem* UI::Taskbar::windowData(const int id) const
{
  for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it)
    if (it.value() == id)
      return it.key();

  return nullptr;
}

/**
 * @brief Returns the QQuickItem for the visually-first tile.
 */
QQuickItem* UI::Taskbar::firstWindow() const
{
  if (m_windowManager) {
    const int wid = m_windowManager->firstTileWindowId();
    if (wid >= 0) {
      if (auto* item = windowData(wid))
        return item;
    }
  }

  if (!m_taskbarButtons || m_taskbarButtons->rowCount() == 0)
    return nullptr;

  auto* row = m_taskbarButtons->item(0);
  if (!row)
    return nullptr;

  return windowData(row->data(TaskbarModel::WindowIdRole).toInt());
}

/**
 * @brief Returns the windowIds of the visible taskbar buttons in row order.
 */
QVector<int> UI::Taskbar::taskbarWindowIds() const
{
  QVector<int> ids;
  if (!m_taskbarButtons)
    return ids;

  ids.reserve(m_taskbarButtons->rowCount());
  for (int i = 0; i < m_taskbarButtons->rowCount(); ++i) {
    auto* row = m_taskbarButtons->item(i);
    if (!row)
      continue;

    const int wid = row->data(TaskbarModel::WindowIdRole).toInt();
    if (wid >= 0)
      ids.append(wid);
  }

  return ids;
}

/**
 * @brief Returns the current state of a window widget.
 */
UI::TaskbarModel::WindowState UI::Taskbar::windowState(QQuickItem* window) const
{
  if (!window)
    return TaskbarModel::WindowClosed;

  const int id = m_windowIDs.value(window, -1);
  if (id < 0)
    return TaskbarModel::WindowClosed;

  auto* item = findItemByWindowId(id);
  if (!item)
    return TaskbarModel::WindowClosed;

  return static_cast<TaskbarModel::WindowState>(item->data(TaskbarModel::WindowStateRole).toInt());
}

/**
 * @brief Returns the next/previous non-closed window in taskbar order.
 */
QQuickItem* UI::Taskbar::nextActiveWindow(int delta) const
{
  if (!m_taskbarButtons || delta == 0)
    return nullptr;

  QList<QQuickItem*> windows;
  windows.reserve(m_taskbarButtons->rowCount());
  for (int i = 0; i < m_taskbarButtons->rowCount(); ++i) {
    auto* row = m_taskbarButtons->item(i);
    if (!row)
      continue;

    const int wid = row->data(TaskbarModel::WindowIdRole).toInt();
    auto* win     = windowData(wid);
    if (!win)
      continue;

    if (windowState(win) == TaskbarModel::WindowClosed)
      continue;

    windows.append(win);
  }

  if (windows.isEmpty())
    return nullptr;

  const int currentIdx = windows.indexOf(m_activeWindow);
  const int n          = windows.size();
  const int base       = currentIdx >= 0 ? currentIdx : (delta > 0 ? -1 : 0);
  const int nextIdx    = ((base + delta) % n + n) % n;
  return windows.at(nextIdx);
}

/**
 * @brief Identifies the layout universe currently on screen: operation mode, project file,
 *        workspace scope and group. Any of them changing means the layout state held by the
 *        window manager belongs to a different dashboard and must not carry over.
 */
QString UI::Taskbar::layoutContextKey() const
{
  return QStringLiteral("%1|%2|%3|%4")
    .arg(QString::number(static_cast<int>(m_appState.operationMode())),
         m_projectModel.jsonFilePath(),
         m_layoutScope,
         QString::number(m_activeGroupId));
}

/**
 * @brief Serializes the active group's layout and writes it to the project file;
 *        no-ops during a layout restore so restore-driven signals can't loop back.
 */
void UI::Taskbar::saveLayout()
{
  if (!m_windowManager || m_windowIDs.isEmpty() || m_activeGroupId < -2)
    return;

  if (m_restoringLayout)
    return;

  const auto opMode = m_appState.operationMode();
  if (opMode != SerialStudio::ProjectFile)
    return;

  auto* model = &m_projectModel;
  if (model->jsonFilePath().isEmpty())
    return;

  if (m_taskbarButtons && m_windowIDs.count() < m_taskbarButtons->rowCount())
    return;

  model->saveWidgetSetting(Keys::layoutKey(m_layoutScope, m_activeGroupId),
                           QStringLiteral("data"),
                           m_windowManager->serializeLayout());
}

//--------------------------------------------------------------------------------------------------
// Taskbar group selection code (e.g. when a tab is selected in the tab bar)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active group and updates taskbar buttons accordingly.
 */
void UI::Taskbar::setActiveGroupId(int groupId)
{
  if (groupId != m_activeGroupId && !m_rebuildInProgress) {
    const auto indices = m_workspaces.changeIndices(m_activeGroupId, groupId);
    Q_EMIT aboutToChangeWorkspace(indices.first, indices.second);
  }

  m_focusCycler.stop();

  saveLayout();

  if (!m_rebuildInProgress && !m_independentWorkspace)
    m_projectModel.setActiveGroupId(groupId);

  for (auto it = m_windowConnections.begin(); it != m_windowConnections.end(); ++it)
    disconnect(*it);

  m_windowConnections.clear();

  m_windowIDs.clear();
  m_taskbarButtons->clear();
  if (m_windowManager)
    m_windowManager->clear();

  m_activeWindow  = nullptr;
  m_activeGroupId = groupId;

  if (m_windowManager && m_appState.operationMode() == SerialStudio::ProjectFile) {
    const auto layout = m_projectModel.groupLayout(m_layoutScope, groupId);
    m_windowManager->preloadPendingGeometries(layout);
  }

  if (groupId >= WorkspaceIds::AutoStart)
    m_workspaces.populateTaskbar(groupId);

  else
    populateTaskbarFromGroup(groupId);

  if (m_taskbarButtons->rowCount() > 0) {
    auto firstGroup = m_taskbarButtons->item(0);
    auto windowId   = firstGroup->data(TaskbarModel::WindowIdRole).toInt();
    for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it) {
      if (it.value() == windowId) {
        setActiveWindow(it.key());
        break;
      }
    }
  }

  Q_EMIT activeGroupIdChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT taskbarButtonsChanged();
}

/**
 * @brief Populates the taskbar buttons for an auto-generated group (id < 5000).
 */
void UI::Taskbar::populateTaskbarFromGroup(int groupId)
{
  for (int i = 0; i < fullModel()->rowCount(); ++i) {
    auto* groupItem = fullModel()->item(i);
    if (!groupItem)
      continue;

    const auto type = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (groupId > -1 && groupItem->data(TaskbarModel::GroupIdRole).toInt() != groupId)
      continue;

    auto* group = groupItem->clone();
    if (type != SerialStudio::DashboardNoWidget) {
      setWindowState(group->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
      m_taskbarButtons->appendRow(group);
    }

    const auto groupName = group->data(TaskbarModel::WidgetNameRole).toString();
    for (int j = 0; j < groupItem->rowCount(); ++j) {
      auto* rawChild = groupItem->child(j);
      if (!rawChild)
        continue;

      const auto childType = rawChild->data(TaskbarModel::WidgetTypeRole).toInt();
      if (childType == SerialStudio::DashboardNoWidget)
        continue;

      auto* child         = rawChild->clone();
      const auto windowId = child->data(TaskbarModel::WindowIdRole).toInt();
      if (groupId > -1) {
        setWindowState(windowId, TaskbarModel::WindowNormal);
        m_taskbarButtons->appendRow(child);
        continue;
      }

      if (groupId == -2) {
        const auto name = child->data(TaskbarModel::WidgetNameRole).toString();
        child->setData(QStringLiteral("%1 (%2)").arg(name, groupName),
                       TaskbarModel::WidgetNameRole);
        setWindowState(windowId, TaskbarModel::WindowNormal);
        m_taskbarButtons->appendRow(child);
      }
    }
  }
}

/**
 * @brief Sets the active group based on its index in the group model.
 */
void UI::Taskbar::setActiveGroupIndex(int index)
{
  auto model = workspaceModel();
  if (model.count() > index && index >= 0) {
    auto item = model[index];
    auto map  = item.toMap();
    auto id   = map.value("id", -1).toInt();
    if (m_independentWorkspace && id >= 0)
      m_desiredGroupId = id;

    setActiveGroupId(id);
  }
}

/**
 * @brief Activates a workspace by id (used by the folder switcher sub-menus).
 */
void UI::Taskbar::selectWorkspaceById(int workspaceId)
{
  if (workspaceId < 0)
    return;

  if (m_independentWorkspace)
    m_desiredGroupId = workspaceId;

  setActiveGroupId(workspaceId);
}

/**
 * @brief Records and applies the workspace an independent window should display.
 */
void UI::Taskbar::setDesiredGroupId(int groupId)
{
  m_desiredGroupId = groupId;
  if (!m_independentWorkspace || groupId < 0)
    return;

  if (groupId != m_activeGroupId && m_workspaces.contains(groupId))
    setActiveGroupId(groupId);
}

/**
 * @brief Marks this taskbar as owning its workspace selection independently of the project.
 */
void UI::Taskbar::setIndependentWorkspace(bool independent)
{
  if (m_independentWorkspace == independent)
    return;

  m_independentWorkspace = independent;
  Q_EMIT independentWorkspaceChanged();
}

/**
 * @brief Sets the scope key that namespaces this taskbar's saved per-workspace layouts.
 */
void UI::Taskbar::setLayoutScope(const QString& scope)
{
  if (m_layoutScope == scope)
    return;

  m_layoutScope = scope;
  Q_EMIT layoutScopeChanged();
}

//--------------------------------------------------------------------------------------------------
// Window state management functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores a window to its normal (visible) state.
 */
void UI::Taskbar::showWindow(QQuickItem* window)
{
  if (window) {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1) {
      setWindowState(id, UI::TaskbarModel::WindowNormal);
      setActiveWindow(window);
      saveLayout();
    }
  }
}

/**
 * @brief Closes a window and marks its state as WindowClosed.
 */
void UI::Taskbar::closeWindow(QQuickItem* window)
{
  if (window) {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1) {
      setWindowState(id, UI::TaskbarModel::WindowClosed);
      setActiveWindow(nullptr);
      saveLayout();
    }
  }
}

/**
 * @brief Minimizes a window, hiding it from the main taskbar area.
 */
void UI::Taskbar::minimizeWindow(QQuickItem* window)
{
  if (window) {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1) {
      setWindowState(id, UI::TaskbarModel::WindowMinimized);
      setActiveWindow(nullptr);
      saveLayout();
    }
  }
}

/**
 * @brief Sets the currently active window in the UI.
 */
void UI::Taskbar::setActiveWindow(QQuickItem* window)
{
  if (m_activeWindow == window)
    return;

  m_activeWindow = window;
  if (m_windowManager)
    m_windowManager->bringToFront(window);

  Q_EMIT activeWindowChanged();
}

/**
 * @brief Unregisters a previously registered QML window.
 */
void UI::Taskbar::unregisterWindow(QQuickItem* window)
{
  if (m_windowIDs.contains(window)) {
    auto it = m_windowConnections.find(window);
    if (it != m_windowConnections.end()) {
      disconnect(*it);
      m_windowConnections.erase(it);
    }

    m_focusCycler.remove(window);

    m_windowIDs.remove(window);
    if (m_windowManager)
      m_windowManager->unregisterWindow(window);

    Q_EMIT registeredWindowsChanged();
  }
}

/**
 * @brief Sets the window manager object, which is used to syncronize taskbar
 *        events with window manager events.
 */
void UI::Taskbar::setWindowManager(UI::WindowManager* manager)
{
  if (!manager)
    return;

  m_windowManager = manager;
  m_windowManager->setTaskbar(this);

  connect(m_windowManager, &UI::WindowManager::geometryChanged, this, [this](QQuickItem* item) {
    if (item != nullptr)
      saveLayout();
  });

  connect(
    m_windowManager, &UI::WindowManager::autoLayoutEnabledChanged, this, &UI::Taskbar::saveLayout);

  Q_EMIT windowManagerChanged();
}

/**
 * @brief Registers a QML window with a corresponding internal window ID.
 */
void UI::Taskbar::registerWindow(const int id, QQuickItem* window)
{
  if (!window)
    return;

  m_windowIDs.insert(window, id);

  if (m_windowManager)
    m_windowManager->registerWindow(id, window);

  Q_EMIT registeredWindowsChanged();

  m_windowConnections[window] =
    connect(window, &QQuickItem::stateChanged, this, [=, this] { Q_EMIT statesChanged(); });

  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager) {
    m_restoringLayout = true;
    m_windowManager->setLayoutContext(layoutContextKey());

    const auto opMode = m_appState.operationMode();
    QJsonObject layout;
    bool restored = false;
    if (opMode == SerialStudio::ProjectFile) {
      layout = m_projectModel.groupLayout(m_layoutScope, m_activeGroupId);
      if (!layout.isEmpty() && m_windowManager->restoreLayout(layout))
        restored = true;
    }

    m_windowManager->reconcileWindowOrder(taskbarWindowIds());

    if (restored)
      applySavedWindowStates(layout);
    else
      m_windowManager->loadLayout();

    m_restoringLayout = false;

    startFocusCycle();
  }
}

/**
 * @brief Starts a brief focus-ripple across all registered tiles in visual order; the cycler
 *        owns the timing, the taskbar owns which tiles are eligible.
 */
void UI::Taskbar::startFocusCycle()
{
  m_focusCycler.stop();
  if (!m_windowManager)
    return;

  QVector<QQuickItem*> queue;
  const auto& order = m_windowManager->windowOrder();
  for (int id : order)
    if (auto* win = windowData(id); win && windowState(win) == TaskbarModel::WindowNormal)
      queue.append(win);

  m_focusCycler.start(queue);
}

/**
 * @brief Re-activates a tile the focus ripple lands on. The active window is dropped first so a
 *        tile that is already active still re-emits, which is what makes the ripple visible.
 */
void UI::Taskbar::refocusWindow(QQuickItem* window)
{
  if (!window)
    return;

  if (m_activeWindow == window) {
    m_activeWindow = nullptr;
    Q_EMIT activeWindowChanged();
  }

  setActiveWindow(window);
}

/**
 * @brief Updates the window state for a given internal ID.
 */
void UI::Taskbar::setWindowState(const int id, const UI::TaskbarModel::WindowState state)
{
  QStandardItem* item = findItemByWindowId(id);
  if (!item)
    return;

  item->setData(state, UI::TaskbarModel::WindowStateRole);
  Q_EMIT windowStatesChanged();

  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager)
    m_windowManager->triggerLayoutUpdate();
}

/**
 * @brief Re-applies the minimized/closed states saved with a restored layout. Written straight
 *        onto the model rather than through setWindowState() so the canvas is retiled once
 *        instead of once per window.
 */
void UI::Taskbar::applySavedWindowStates(const QJsonObject& layout)
{
  if (!m_windowManager)
    return;

  const auto states = m_windowManager->savedWindowStates(layout);
  if (states.isEmpty())
    return;

  for (auto it = states.cbegin(); it != states.cend(); ++it) {
    QStandardItem* item = findItemByWindowId(it.key());
    if (!item)
      continue;

    auto state = TaskbarModel::WindowNormal;
    if (it.value() == QStringLiteral("minimized"))
      state = TaskbarModel::WindowMinimized;
    else if (it.value() == QStringLiteral("closed"))
      state = TaskbarModel::WindowClosed;

    item->setData(state, TaskbarModel::WindowStateRole);
  }

  Q_EMIT windowStatesChanged();
  m_windowManager->triggerLayoutUpdate();
}

//--------------------------------------------------------------------------------------------------
// General (full) model generation function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the taskbar's full and visible models from the latest dashboard frame.
 */
void UI::Taskbar::rebuildModel()
{
  if (m_rebuildInProgress)
    return;

  m_focusCycler.stop();

  m_rebuildInProgress = true;
  {
    QSignalBlocker fullBlocker(m_fullModel);
    QSignalBlocker taskbarBlocker(m_taskbarButtons);

    for (auto it = m_windowConnections.begin(); it != m_windowConnections.end(); ++it)
      disconnect(*it);

    m_windowConnections.clear();

    m_windowIDs.clear();
    m_fullModel->clear();
    m_activeWindow = nullptr;
    m_windowMap.clear();
    if (m_windowManager)
      m_windowManager->clear();
  }

  auto* db = &m_dashboard;

  const auto& frame = db->processedFrame();
  if (frame.title.isEmpty() || frame.groups.size() <= 0) {
    setActiveGroupId(-1);
    Q_EMIT fullModelChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT registeredWindowsChanged();
    m_rebuildInProgress = false;
    return;
  }

  QSet<int> groupIds;
  for (const DataModel::Group& group : frame.groups) {
    const auto groupId   = group.groupId;
    const auto groupName = group.title;
    const auto groupType = SerialStudio::getDashboardWidget(group);
    if (SerialStudio::isDashboardTool(groupType))
      continue;

    QList<int> windowIds;
    QList<int> relativeIds;
    QList<SerialStudio::DashboardWidget> widgetTypes;
    m_windowMap.collectGroupWidgetIds(groupId, windowIds, relativeIds, widgetTypes);

    int mainWindowId = -1;
    for (int i = 0; i < windowIds.count(); ++i) {
      if (widgetTypes[i] != groupType)
        continue;

      mainWindowId = windowIds[i];
      windowIds.removeAt(i);
      widgetTypes.removeAt(i);
      relativeIds.removeAt(i);
      break;
    }

    auto* groupItem              = new QStandardItem();
    const bool alreadyRegistered = groupIds.contains(groupId);
    buildOverviewGroupItem(
      groupItem, groupId, groupName, groupType, group.widget, mainWindowId, alreadyRegistered);
    m_windowMap.mapMainGroupWidget(groupType, groupId, mainWindowId);

    for (int i = 0; i < windowIds.count(); ++i)
      appendGroupChildItem(
        groupItem, groupId, groupName, windowIds[i], widgetTypes[i], relativeIds[i]);

    attachGroupItemToFullModel(groupItem, groupId, alreadyRegistered);
    if (!alreadyRegistered)
      groupIds.insert(groupId);
  }

  Q_EMIT fullModelChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT registeredWindowsChanged();

  selectGroupAfterRebuild();
  m_rebuildInProgress = false;
}

/**
 * @brief Populates the role data of an overview group item from group/main-widget info; a group
 *        rendered by an extension package takes its artwork from that package's descriptor.
 */
void UI::Taskbar::buildOverviewGroupItem(QStandardItem* groupItem,
                                         int groupId,
                                         const QString& groupName,
                                         SerialStudio::DashboardWidget groupType,
                                         const QString& extensionId,
                                         int mainWindowId,
                                         bool alreadyRegistered)
{
  const auto groupIcon = taskbarIcon(groupType, extensionId, true);
  groupItem->setData(groupId, TaskbarModel::GroupIdRole);
  groupItem->setData(groupName, TaskbarModel::GroupNameRole);
  groupItem->setData(groupName, TaskbarModel::WidgetNameRole);
  groupItem->setData(groupType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(groupIcon, TaskbarModel::WidgetIconRole);
  groupItem->setData(taskbarIconId(groupType, extensionId), TaskbarModel::IconIdRole);
  groupItem->setData(mainWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(!alreadyRegistered, TaskbarModel::IsGroupRole);
  groupItem->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);
}

/**
 * @brief Builds and appends a single child QStandardItem under the given group item; a widget
 *        rendered by an extension package takes its artwork from that package's descriptor.
 */
void UI::Taskbar::appendGroupChildItem(QStandardItem* groupItem,
                                       int groupId,
                                       const QString& groupName,
                                       int windowId,
                                       SerialStudio::DashboardWidget widgetType,
                                       int relativeIndex)
{
  auto* db = &m_dashboard;

  const auto slot = db->widgetSlot(widgetType, relativeIndex);
  const auto icon = taskbarIcon(widgetType, slot.extensionId, true);
  auto* child     = new QStandardItem();
  child->setData(false, TaskbarModel::IsGroupRole);
  child->setData(icon, TaskbarModel::WidgetIconRole);
  child->setData(taskbarIconId(widgetType, slot.extensionId), TaskbarModel::IconIdRole);
  child->setData(groupId, TaskbarModel::GroupIdRole);
  child->setData(groupName, TaskbarModel::GroupNameRole);
  child->setData(windowId, TaskbarModel::WindowIdRole);
  child->setData(widgetType, TaskbarModel::WidgetTypeRole);
  child->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);

  if (slot.valid) {
    const auto& title = slot.group ? db->getGroupWidget(widgetType, slot.bucketIndex).title
                                   : db->getDatasetWidget(widgetType, slot.bucketIndex).title;
    child->setData(title, TaskbarModel::WidgetNameRole);
    m_windowMap.map(m_widgetRegistry.widgetIdByTypeAndIndex(widgetType, relativeIndex), windowId);
  }

  groupItem->appendRow(child);
}

/**
 * @brief Attaches a built group item to the full model: top-level on first use, child on reuse.
 */
void UI::Taskbar::attachGroupItemToFullModel(QStandardItem* groupItem,
                                             int groupId,
                                             bool alreadyRegistered)
{
  if (!alreadyRegistered) {
    m_fullModel->appendRow(groupItem);
    return;
  }

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* g = m_fullModel->item(i);
    if (!g || g->data(TaskbarModel::GroupIdRole).toInt() != groupId)
      continue;

    g->appendRow(groupItem);
    return;
  }
}

/**
 * @brief Selects the saved or first group after a rebuild, or no-ops if none is available.
 */
void UI::Taskbar::selectGroupAfterRebuild()
{
  const auto target =
    m_workspaces.selectionAfterRebuild(m_independentWorkspace, m_desiredGroupId, m_activeGroupId);
  if (target.has_value())
    setActiveGroupId(target.value());
}

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recursively searches the full model for an item by its window ID.
 */
QStandardItem* UI::Taskbar::findItemByWindowId(int windowId,
                                               QStandardItem* parentItem,
                                               int depth) const
{
  static constexpr int kMaxDepth = 4;
  if (depth > kMaxDepth) [[unlikely]]
    return nullptr;

  int count = parentItem ? parentItem->rowCount() : fullModel()->rowCount();
  for (int i = 0; i < count; ++i) {
    QStandardItem* item = parentItem ? parentItem->child(i) : fullModel()->item(i);
    if (!item)
      continue;

    if (item->data(TaskbarModel::WindowIdRole).toInt() == windowId)
      return item;

    if (item->data(TaskbarModel::IsGroupRole).toBool()) {
      QStandardItem* found = findItemByWindowId(windowId, item, depth + 1);
      if (found)
        return found;
    }
  }

  return nullptr;
}

/**
 * @brief Searches the full model for an item by its widget ID.
 */
QStandardItem* UI::Taskbar::findItemByWidgetId(UI::WidgetID widgetId,
                                               QStandardItem* parentItem) const
{
  const int windowId = m_windowMap.windowIdForWidget(widgetId);
  if (windowId < 0)
    return nullptr;

  return findItemByWindowId(windowId, parentItem);
}

/**
 * @brief Finds a group item by its group ID.
 */
QStandardItem* UI::Taskbar::findGroupItemByGroupId(int groupId) const
{
  for (int i = 0; i < fullModel()->rowCount(); ++i) {
    QStandardItem* item = fullModel()->item(i);
    if (item && item->data(TaskbarModel::GroupIdRole).toInt() == groupId)
      return item;
  }

  return nullptr;
}

/**
 * @brief Creates a QStandardItem from widget info.
 */
QStandardItem* UI::Taskbar::createItemFromWidgetInfo(const UI::WidgetInfo& info)
{
  auto* item = new QStandardItem();
  auto icon  = SerialStudio::dashboardWidgetIcon(info.type, true);

  item->setData(info.groupId, TaskbarModel::GroupIdRole);
  item->setData(info.title, TaskbarModel::WidgetNameRole);
  item->setData(info.type, TaskbarModel::WidgetTypeRole);
  item->setData(icon, TaskbarModel::WidgetIconRole);
  item->setData(SerialStudio::dashboardWidgetIconId(info.type), TaskbarModel::IconIdRole);
  item->setData(info.isGroupWidget, TaskbarModel::IsGroupRole);
  item->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);

  return item;
}

//--------------------------------------------------------------------------------------------------
// Registry event handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles widget creation events from the registry.
 */
void UI::Taskbar::onWidgetCreated(UI::WidgetID id, const UI::WidgetInfo& info)
{
  Q_UNUSED(id)
  Q_UNUSED(info)
}

/**
 * @brief Retitles the matching taskbar item when a widget's registry metadata changes.
 */
void UI::Taskbar::onWidgetUpdated(UI::WidgetID id, const UI::WidgetInfo& info)
{
  auto* item = findItemByWidgetId(id);
  if (!item)
    return;

  if (item->data(TaskbarModel::WidgetNameRole).toString() != info.title)
    item->setData(info.title, TaskbarModel::WidgetNameRole);
}

/**
 * @brief Handles widget destruction events from the registry.
 */
void UI::Taskbar::onWidgetDestroyed(UI::WidgetID id)
{
  Q_UNUSED(id)
}

/**
 * @brief Handles registry clear events.
 */
void UI::Taskbar::onRegistryCleared()
{
  m_windowMap.clear();
}

//--------------------------------------------------------------------------------------------------
// Search functionality
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current search filter string.
 */
QString UI::Taskbar::searchFilter() const
{
  return m_search.filter();
}

/**
 * @brief Clears the search filter and emits searchDismissed to close the popup.
 */
void UI::Taskbar::dismissSearch()
{
  m_search.dismiss();
}

/**
 * @brief Sets the search filter and recomputes search results.
 */
void UI::Taskbar::setSearchFilter(const QString& filter)
{
  m_search.setFilter(filter);
}

/**
 * @brief Returns a flat list of widgets matching the current search filter.
 */
QVariantList UI::Taskbar::searchResults() const
{
  return m_search.results();
}

/**
 * @brief Returns an unfiltered, unlimited flat list of every widget in the full model.
 */
QVariantList UI::Taskbar::allWidgets() const
{
  return m_search.allWidgets();
}

//--------------------------------------------------------------------------------------------------
// Workspace model and management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the workspace model for the workspace selector.
 */
QVariantList UI::Taskbar::workspaceModel() const
{
  return m_workspaces.model();
}

/**
 * @brief Returns the workspace switcher model as a folder -> children tree.
 */
QVariantList UI::Taskbar::workspaceTree() const
{
  return m_workspaces.tree();
}

/**
 * @brief Navigates to the widget: reveals it in place, adds it to the active user
 *        workspace when allowAddToWorkspace is set, else activates its own group.
 */
void UI::Taskbar::navigateToWidget(int windowId, int groupId, bool allowAddToWorkspace)
{
  m_workspaces.navigateToWidget(windowId, groupId, allowAddToWorkspace);
}

/**
 * @brief Creates a new user-defined workspace and switches to it.
 */
void UI::Taskbar::createWorkspace(const QString& name)
{
  m_workspaces.createWorkspace(name);
}

/**
 * @brief Deletes or hides a workspace.
 */
void UI::Taskbar::deleteWorkspace(int workspaceId)
{
  m_workspaces.deleteWorkspace(workspaceId);
}

/**
 * @brief Renames a user-defined workspace.
 */
void UI::Taskbar::renameWorkspace(int workspaceId, const QString& name)
{
  m_workspaces.renameWorkspace(workspaceId, name);
}

/**
 * @brief Adds the widget identified by windowId to the active workspace.
 */
void UI::Taskbar::addWidgetToActiveWorkspace(int windowId)
{
  m_workspaces.addWidgetToActiveWorkspace(windowId);
}

/**
 * @brief Removes the widget identified by windowId from the active workspace.
 */
void UI::Taskbar::removeWidgetFromActiveWorkspace(int windowId)
{
  m_workspaces.removeWidgetFromActiveWorkspace(windowId);
}

/**
 * @brief Returns window IDs of all widgets in the given user workspace, empty for non-user IDs.
 */
QVariantList UI::Taskbar::workspaceWidgetIds(int workspaceId) const
{
  return m_workspaces.widgetIds(workspaceId);
}

/**
 * @brief Returns the user workspace containing windowId (active preferred), else -1.
 */
int UI::Taskbar::workspaceContainingWidget(int windowId) const
{
  return m_workspaces.workspaceContainingWidget(windowId);
}

/**
 * @brief Replaces the widget list of a user workspace with the given IDs.
 */
void UI::Taskbar::setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds)
{
  m_workspaces.setWorkspaceWidgets(workspaceId, windowIds);
}
