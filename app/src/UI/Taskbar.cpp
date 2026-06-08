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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Taskbar.h"

#include <algorithm>
#include <QSignalBlocker>
#include <QTimer>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "UI/Dashboard.h"
#include "UI/UISessionRegistry.h"
#include "UI/WidgetRegistry.h"
#include "UI/WindowManager.h"

//--------------------------------------------------------------------------------------------------
// Taskbar model implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the taskbar model for representing dashboard widgets.
 */
UI::TaskbarModel::TaskbarModel(QObject* parent) : QStandardItemModel(parent) {}

/**
 * @brief Returns the role names used for QML data binding.
 */
QHash<int, QByteArray> UI::TaskbarModel::roleNames() const
{
  QHash<int, QByteArray> names;

#define BAL(x) QByteArrayLiteral(x)
  names.insert(GroupIdRole, BAL("groupId"));
  names.insert(IsGroupRole, BAL("isGroup"));
  names.insert(WindowIdRole, BAL("windowId"));
  names.insert(GroupNameRole, BAL("groupName"));
  names.insert(WidgetTypeRole, BAL("widgetType"));
  names.insert(WidgetNameRole, BAL("widgetName"));
  names.insert(WidgetIconRole, BAL("widgetIcon"));
  names.insert(WindowStateRole, BAL("windowState"));
#undef BAL

  return names;
}

//--------------------------------------------------------------------------------------------------
// Taskbar class implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Private constructor for the Taskbar singleton.
 */
UI::Taskbar::Taskbar(QQuickItem* parent)
  : QQuickItem(parent)
  , m_activeGroupId(-1)
  , m_rebuildInProgress(false)
  , m_batchUpdateInProgress(false)
  , m_restoringLayout(false)
  , m_activeWindow(nullptr)
  , m_windowManager(nullptr)
  , m_fullModel(new TaskbarModel(this))
  , m_taskbarButtons(new TaskbarModel(this))
{
  qmlRegisterUncreatableType<UI::TaskbarModel>(
    "SerialStudio.UI", 1, 0, "TaskbarModel", "TaskbarModel is exposed by Taskbar singleton");

  m_focusCycleTimer.setSingleShot(false);
  connect(&m_focusCycleTimer, &QTimer::timeout, this, &UI::Taskbar::onFocusCycleTick);

  connectToRegistry();

  connect(&UI::Dashboard::instance(), &UI::Dashboard::dataReset, this, &UI::Taskbar::rebuildModel);
  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::widgetCountChanged,
          this,
          &UI::Taskbar::rebuildModel);

  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::terminalEnabledChanged,
          this,
          &UI::Taskbar::onTerminalToggled);

  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::notificationLogEnabledChanged,
          this,
          &UI::Taskbar::onNotificationLogToggled);

  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::clockEnabledChanged,
          this,
          &UI::Taskbar::onClockToggled);

  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::stopwatchEnabledChanged,
          this,
          &UI::Taskbar::onStopwatchToggled);

  auto* pm = &DataModel::ProjectModel::instance();
  connect(pm, &DataModel::ProjectModel::activeGroupIdChanged, this, [this, pm] {
    setActiveGroupId(pm->activeGroupId());
  });

  connect(pm,
          &DataModel::ProjectModel::activeWorkspacesChanged,
          this,
          &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::searchResultsChanged);

  rebuildModel();

  UISessionRegistry::instance().registerTaskbar(this);
}

/**
 * @brief Destroys the Taskbar and unregisters it from the UI session registry.
 */
UI::Taskbar::~Taskbar()
{
  UISessionRegistry::instance().unregisterTaskbar(this);
}

/**
 * @brief Connects the Taskbar to the WidgetRegistry's lifecycle signals.
 */
void UI::Taskbar::connectToRegistry()
{
  auto& registry = WidgetRegistry::instance();

  connect(&registry, &WidgetRegistry::widgetCreated, this, &Taskbar::onWidgetCreated);
  connect(&registry, &WidgetRegistry::widgetDestroyed, this, &Taskbar::onWidgetDestroyed);
  connect(&registry, &WidgetRegistry::registryCleared, this, &Taskbar::onRegistryCleared);
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
  const auto model = workspaceModel();
  int index        = 0;
  for (auto it = model.begin(); it != model.end(); ++it) {
    const auto map = it->toMap();
    if (!map.contains(QStringLiteral("id")))
      continue;

    if (map.value(QStringLiteral("id")).toInt() == m_activeGroupId)
      return index;

    ++index;
  }

  return -1;
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

    const auto state =
      static_cast<TaskbarModel::WindowState>(row->data(TaskbarModel::WindowStateRole).toInt());
    if (state == TaskbarModel::WindowClosed)
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
 * @brief Serializes the active group's layout and writes it to the project file;
 *        no-ops during a layout restore so restore-driven signals can't loop back.
 */
void UI::Taskbar::saveLayout()
{
  if (!m_windowManager || m_windowIDs.isEmpty() || m_activeGroupId < -2)
    return;

  if (m_restoringLayout)
    return;

  const auto opMode = AppState::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile)
    return;

  auto* model = &DataModel::ProjectModel::instance();
  if (model->jsonFilePath().isEmpty())
    return;

  if (m_taskbarButtons && m_windowIDs.count() < m_taskbarButtons->rowCount())
    return;

  model->saveWidgetSetting(
    Keys::layoutKey(m_activeGroupId), QStringLiteral("data"), m_windowManager->serializeLayout());
}

//--------------------------------------------------------------------------------------------------
// Taskbar group selection code (e.g. when a tab is selected in the tab bar)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active group and updates taskbar buttons accordingly.
 */
void UI::Taskbar::setActiveGroupId(int groupId)
{
  m_focusCycleTimer.stop();
  m_focusCycleQueue.clear();

  saveLayout();

  if (!m_rebuildInProgress)
    DataModel::ProjectModel::instance().setActiveGroupId(groupId);

  for (auto it = m_windowConnections.begin(); it != m_windowConnections.end(); ++it)
    disconnect(*it);

  m_windowConnections.clear();

  m_windowIDs.clear();
  m_taskbarButtons->clear();
  if (m_windowManager)
    m_windowManager->clear();

  m_activeWindow  = nullptr;
  m_activeGroupId = groupId;

  if (m_windowManager && AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    const auto layout = DataModel::ProjectModel::instance().groupLayout(groupId);
    m_windowManager->preloadPendingGeometries(layout);
  }

  cloneSpecialOverviewRow(SerialStudio::DashboardTerminal);
#ifdef BUILD_COMMERCIAL
  cloneSpecialOverviewRow(SerialStudio::DashboardNotificationLog);
#endif
  cloneSpecialOverviewRow(SerialStudio::DashboardClock);
  cloneSpecialOverviewRow(SerialStudio::DashboardStopwatch);

  if (groupId >= WorkspaceIds::AutoStart)
    populateTaskbarFromWorkspace(groupId);

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
 * @brief Clones a global overview row (terminal, notification log) into the taskbar buttons.
 */
void UI::Taskbar::cloneSpecialOverviewRow(int widgetType)
{
  for (int i = 0; i < fullModel()->rowCount(); ++i) {
    auto* groupItem = fullModel()->item(i);
    if (!groupItem)
      continue;

    if (groupItem->data(TaskbarModel::WidgetTypeRole).toInt() != widgetType)
      continue;

    auto* clone = groupItem->clone();
    setWindowState(clone->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
    m_taskbarButtons->appendRow(clone);
    return;
  }
}

/**
 * @brief Populates the taskbar buttons for a user-defined workspace (id >= 1000).
 */
void UI::Taskbar::populateTaskbarFromWorkspace(int groupId)
{
  const auto& workspaces = DataModel::ProjectModel::instance().activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != groupId)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const int windowId = findWindowIdByGroupAndIndex(ref.widgetType, ref.relativeIndex);
      if (windowId < 0)
        continue;

      const int refGid = UI::Dashboard::instance().groupIdForUniqueId(ref.groupUniqueId);
      auto* item       = findItemByWindowId(windowId);
      if (!item || item->data(TaskbarModel::GroupIdRole).toInt() != refGid)
        continue;

      auto* clone = item->clone();
      setWindowState(windowId, TaskbarModel::WindowNormal);
      m_taskbarButtons->appendRow(clone);
    }

    return;
  }
}

/**
 * @brief Populates the taskbar buttons for an auto-generated group (id < 1000).
 */
void UI::Taskbar::populateTaskbarFromGroup(int groupId)
{
  for (int i = 0; i < fullModel()->rowCount(); ++i) {
    auto* groupItem = fullModel()->item(i);
    if (!groupItem)
      continue;

    const auto type = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (type == SerialStudio::DashboardTerminal)
      continue;

#ifdef BUILD_COMMERCIAL
    if (type == SerialStudio::DashboardNotificationLog)
      continue;
#endif

    if (type == SerialStudio::DashboardClock)
      continue;

    if (type == SerialStudio::DashboardStopwatch)
      continue;

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
 * @brief Returns the windowId for the (widgetType, relativeIndex) pair, or -1 if not found.
 */
int UI::Taskbar::findWindowIdByGroupAndIndex(int widgetType, int relativeIndex) const
{
  const auto& widgetMap = UI::Dashboard::instance().widgetMap();
  for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
    if (static_cast<int>(it.value().first) != widgetType || it.value().second != relativeIndex)
      continue;

    return it.key();
  }

  return -1;
}

/**
 * @brief Returns the dashboard relative-index for windowId, or -1 if not found.
 */
int UI::Taskbar::relativeIndexForWindow(int windowId) const
{
  const auto& widgetMap = UI::Dashboard::instance().widgetMap();
  for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it)
    if (it.key() == windowId)
      return it.value().second;

  return -1;
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
    setActiveGroupId(id);
  }
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

    m_focusCycleQueue.removeAll(window);

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
    const auto opMode = AppState::instance().operationMode();
    bool restored     = false;
    if (opMode == SerialStudio::ProjectFile) {
      const auto layout = DataModel::ProjectModel::instance().groupLayout(m_activeGroupId);
      if (!layout.isEmpty() && m_windowManager->restoreLayout(layout))
        restored = true;
    }

    m_windowManager->reconcileWindowOrder(taskbarWindowIds());

    if (!restored)
      m_windowManager->loadLayout();

    m_restoringLayout = false;

    startFocusCycle();
  }
}

/**
 * @brief Starts a brief focus-ripple across all registered tiles in visual order.
 */
void UI::Taskbar::startFocusCycle()
{
  m_focusCycleTimer.stop();
  m_focusCycleQueue.clear();

  if (!m_windowManager)
    return;

  const auto& order = m_windowManager->windowOrder();
  for (int id : order)
    if (auto* win = windowData(id))
      m_focusCycleQueue.append(win);

  if (m_focusCycleQueue.isEmpty())
    return;

  if (m_focusCycleQueue.size() == 1) {
    QQuickItem* only = m_focusCycleQueue.first();
    m_focusCycleQueue.clear();
    setActiveWindow(only);
    return;
  }

  QQuickItem* firstTile = m_focusCycleQueue.first();
  m_focusCycleQueue.append(firstTile);

  constexpr int kBudgetMs = 200;
  constexpr int kMinMs    = 10;
  constexpr int kMaxMs    = 40;
  const int interval      = qBound(kMinMs, kBudgetMs / m_focusCycleQueue.size(), kMaxMs);
  m_focusCycleTimer.setInterval(interval);
  m_focusCycleTimer.start();
}

/**
 * @brief Advances the focus-cycle by one tile, stopping when the queue drains.
 */
void UI::Taskbar::onFocusCycleTick()
{
  if (m_focusCycleQueue.isEmpty()) {
    m_focusCycleTimer.stop();
    return;
  }

  QQuickItem* win = m_focusCycleQueue.takeFirst();
  if (win) {
    if (m_activeWindow == win) {
      m_activeWindow = nullptr;
      Q_EMIT activeWindowChanged();
    }

    setActiveWindow(win);
  }

  if (m_focusCycleQueue.isEmpty())
    m_focusCycleTimer.stop();
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

//--------------------------------------------------------------------------------------------------
// General (full) model generation function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the bidirectional mapping between a widget ID and a window ID.
 */
void UI::Taskbar::mapWidgetToWindow(UI::WidgetID wid, int windowId)
{
  if (wid == kInvalidWidgetId)
    return;

  m_widgetIdToWindowId.insert(wid, windowId);
  m_windowIdToWidgetId.insert(windowId, wid);
}

/**
 * @brief Rebuilds the taskbar's full and visible models from the latest dashboard frame.
 */
void UI::Taskbar::rebuildModel()
{
  if (m_rebuildInProgress)
    return;

  m_focusCycleTimer.stop();
  m_focusCycleQueue.clear();

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
    m_widgetIdToWindowId.clear();
    m_windowIdToWidgetId.clear();
    if (m_windowManager)
      m_windowManager->clear();
  }

  auto* db = &UI::Dashboard::instance();

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

    QList<int> windowIds;
    QList<int> relativeIds;
    QList<SerialStudio::DashboardWidget> widgetTypes;
    collectGroupWidgetIds(groupId, windowIds, relativeIds, widgetTypes);

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
      groupItem, groupId, groupName, groupType, mainWindowId, alreadyRegistered);
    mapMainGroupWidgetId(groupType, groupId, mainWindowId);

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
 * @brief Populates the role data of an overview group item from group/main-widget info.
 */
void UI::Taskbar::buildOverviewGroupItem(QStandardItem* groupItem,
                                         int groupId,
                                         const QString& groupName,
                                         SerialStudio::DashboardWidget groupType,
                                         int mainWindowId,
                                         bool alreadyRegistered)
{
  const auto groupIcon = SerialStudio::dashboardWidgetIcon(groupType, true);
  groupItem->setData(groupId, TaskbarModel::GroupIdRole);
  groupItem->setData(groupName, TaskbarModel::GroupNameRole);
  groupItem->setData(groupName, TaskbarModel::WidgetNameRole);
  groupItem->setData(groupType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(groupIcon, TaskbarModel::WidgetIconRole);
  groupItem->setData(mainWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(!alreadyRegistered, TaskbarModel::IsGroupRole);
  groupItem->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);
  if (!alreadyRegistered)
    groupItem->setData(true, TaskbarModel::OverviewRole);
}

/**
 * @brief Maps the WidgetID of the main group widget (if any) to its windowId.
 */
void UI::Taskbar::mapMainGroupWidgetId(SerialStudio::DashboardWidget groupType,
                                       int groupId,
                                       int mainWindowId)
{
  if (groupType == SerialStudio::DashboardNoWidget || mainWindowId < 0)
    return;

  auto& registry       = WidgetRegistry::instance();
  const auto widgetIds = registry.widgetIdsByType(groupType);
  for (const auto& wid : std::as_const(widgetIds)) {
    const auto info = registry.widgetInfo(wid);
    if (info.groupId != groupId || !info.isGroupWidget)
      continue;

    m_widgetIdToWindowId.insert(wid, mainWindowId);
    m_windowIdToWidgetId.insert(mainWindowId, wid);
    return;
  }
}

/**
 * @brief Collects (windowId, widgetType, relativeIndex) triples that belong to groupId.
 */
void UI::Taskbar::collectGroupWidgetIds(int groupId,
                                        QList<int>& windowIds,
                                        QList<int>& relativeIds,
                                        QList<SerialStudio::DashboardWidget>& widgetTypes) const
{
  auto* db              = &UI::Dashboard::instance();
  const auto& widgetMap = db->widgetMap();
  for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
    const auto windowId      = it.key();
    const auto widgetType    = it.value().first;
    const auto relativeIndex = it.value().second;

    int candidateGroup = -1;
    if (SerialStudio::isGroupWidget(widgetType))
      candidateGroup = db->getGroupWidget(widgetType, relativeIndex).groupId;

    else if (SerialStudio::isDatasetWidget(widgetType))
      candidateGroup = db->getDatasetWidget(widgetType, relativeIndex).groupId;

    else
      continue;

    if (candidateGroup != groupId)
      continue;

    windowIds.append(windowId);
    widgetTypes.append(widgetType);
    relativeIds.append(relativeIndex);
  }
}

/**
 * @brief Builds and appends a single child QStandardItem under the given group item.
 */
void UI::Taskbar::appendGroupChildItem(QStandardItem* groupItem,
                                       int groupId,
                                       const QString& groupName,
                                       int windowId,
                                       SerialStudio::DashboardWidget widgetType,
                                       int relativeIndex)
{
  auto* db       = &UI::Dashboard::instance();
  auto& registry = WidgetRegistry::instance();

  const auto icon = SerialStudio::dashboardWidgetIcon(widgetType, true);
  auto* child     = new QStandardItem();
  child->setData(false, TaskbarModel::IsGroupRole);
  child->setData(icon, TaskbarModel::WidgetIconRole);
  child->setData(groupId, TaskbarModel::GroupIdRole);
  child->setData(groupName, TaskbarModel::GroupNameRole);
  child->setData(windowId, TaskbarModel::WindowIdRole);
  child->setData(widgetType, TaskbarModel::WidgetTypeRole);
  child->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);

  if (SerialStudio::isGroupWidget(widgetType)) {
    const auto& dbGroup = db->getGroupWidget(widgetType, relativeIndex);
    child->setData(dbGroup.title, TaskbarModel::WidgetNameRole);
    child->setData(false, TaskbarModel::OverviewRole);
    mapWidgetToWindow(registry.widgetIdByTypeAndIndex(widgetType, relativeIndex), windowId);
  }

  else if (SerialStudio::isDatasetWidget(widgetType)) {
    const auto& dbDataset = db->getDatasetWidget(widgetType, relativeIndex);
    child->setData(dbDataset.title, TaskbarModel::WidgetNameRole);
    child->setData(false, TaskbarModel::OverviewRole);
    mapWidgetToWindow(registry.widgetIdByTypeAndIndex(widgetType, relativeIndex), windowId);
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
  const auto model = workspaceModel();
  if (model.isEmpty())
    return;

  int targetGroupId = -1;
  bool restored     = false;

  const auto opMode = AppState::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile) {
    auto* pm          = &DataModel::ProjectModel::instance();
    const int savedId = pm->activeGroupId();
    const bool found =
      savedId >= 0 && std::any_of(model.begin(), model.end(), [savedId](const QVariant& v) {
        return v.toMap().value("id").toInt() == savedId;
      });
    if (found) {
      targetGroupId = savedId;
      restored      = true;
    }
  }

  if (!restored && model.first().canConvert<QVariantMap>()) {
    const QVariantMap firstItem = model.first().toMap();
    if (firstItem.contains("id"))
      targetGroupId = firstItem["id"].toInt();
  }

  setActiveGroupId(targetGroupId);
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
  Q_ASSERT(depth <= kMaxDepth);
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
  if (!m_widgetIdToWindowId.contains(widgetId))
    return nullptr;

  int windowId = m_widgetIdToWindowId.value(widgetId);
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
  m_widgetIdToWindowId.clear();
  m_windowIdToWidgetId.clear();
}

/**
 * @brief Incrementally adds or removes the terminal widget from the models.
 */
void UI::Taskbar::onTerminalToggled()
{
  auto& db      = UI::Dashboard::instance();
  const bool on = db.terminalEnabled();

  if (!on) {
    removeOverviewByType(static_cast<int>(SerialStudio::DashboardTerminal));
    Q_EMIT taskbarButtonsChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT searchResultsChanged();
    return;
  }

  const int termType = static_cast<int>(SerialStudio::DashboardTerminal);
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == termType)
      return;
  }

  int termWindowId = -1;
  const auto& wm   = db.widgetMap();
  for (auto it = wm.begin(); it != wm.end(); ++it) {
    if (it.value().first != SerialStudio::DashboardTerminal)
      continue;

    termWindowId = it.key();
    break;
  }

  if (termWindowId < 0)
    return;

  int termGroupId = -1;
  QString termTitle;
  const auto& frame = db.processedFrame();
  for (const auto& g : frame.groups) {
    if (g.widget != QStringLiteral("terminal"))
      continue;

    termGroupId = g.groupId;
    termTitle   = g.title;
    break;
  }

  const auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::DashboardTerminal, true);
  auto* groupItem = new QStandardItem();
  groupItem->setData(termGroupId, TaskbarModel::GroupIdRole);
  groupItem->setData(termTitle, TaskbarModel::GroupNameRole);
  groupItem->setData(termTitle, TaskbarModel::WidgetNameRole);
  groupItem->setData(termType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(icon, TaskbarModel::WidgetIconRole);
  groupItem->setData(termWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(true, TaskbarModel::IsGroupRole);
  groupItem->setData(true, TaskbarModel::OverviewRole);
  groupItem->setData(static_cast<int>(TaskbarModel::WindowNormal), TaskbarModel::WindowStateRole);
  m_fullModel->insertRow(0, groupItem);

  auto* tbItem = groupItem->clone();
  setWindowState(tbItem->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->insertRow(0, tbItem);

  Q_EMIT taskbarButtonsChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT searchResultsChanged();
}

/**
 * @brief Removes the row(s) matching widgetType from both fullModel and taskbarButtons.
 */
void UI::Taskbar::removeOverviewByType(int widgetType)
{
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (!item || item->data(TaskbarModel::WidgetTypeRole).toInt() != widgetType)
      continue;

    m_fullModel->removeRow(i);
    break;
  }

  for (int i = 0; i < m_taskbarButtons->rowCount(); ++i) {
    auto* item = m_taskbarButtons->item(i);
    if (!item || item->data(TaskbarModel::WidgetTypeRole).toInt() != widgetType)
      continue;

    const int wid = item->data(TaskbarModel::WindowIdRole).toInt();
    if (auto* window = windowData(wid))
      unregisterWindow(window);

    m_taskbarButtons->removeRow(i);
    break;
  }
}

/**
 * @brief Incrementally adds or removes the notification-log widget from the models.
 */
void UI::Taskbar::onNotificationLogToggled()
{
#ifdef BUILD_COMMERCIAL
  auto& db          = UI::Dashboard::instance();
  const bool on     = db.notificationLogEnabled();
  const int logType = static_cast<int>(SerialStudio::DashboardNotificationLog);

  if (!on) {
    removeOverviewByType(logType);
    Q_EMIT taskbarButtonsChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT searchResultsChanged();
    return;
  }

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == logType)
      return;
  }

  int logWindowId = -1;
  const auto& wm  = db.widgetMap();
  for (auto it = wm.begin(); it != wm.end(); ++it) {
    if (it.value().first != SerialStudio::DashboardNotificationLog)
      continue;

    logWindowId = it.key();
    break;
  }

  if (logWindowId < 0)
    return;

  int logGroupId = -1;
  QString logTitle;
  const auto& frame = db.processedFrame();
  for (const auto& g : frame.groups) {
    if (g.widget != QStringLiteral("notification-log"))
      continue;

    logGroupId = g.groupId;
    logTitle   = g.title;
    break;
  }

  const auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::DashboardNotificationLog, true);
  auto* groupItem = new QStandardItem();
  groupItem->setData(logGroupId, TaskbarModel::GroupIdRole);
  groupItem->setData(logTitle, TaskbarModel::GroupNameRole);
  groupItem->setData(logTitle, TaskbarModel::WidgetNameRole);
  groupItem->setData(logType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(icon, TaskbarModel::WidgetIconRole);
  groupItem->setData(logWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(true, TaskbarModel::IsGroupRole);
  groupItem->setData(true, TaskbarModel::OverviewRole);
  groupItem->setData(static_cast<int>(TaskbarModel::WindowNormal), TaskbarModel::WindowStateRole);

  int insertRow             = 0;
  auto* first               = m_fullModel->rowCount() > 0 ? m_fullModel->item(0) : nullptr;
  const int firstWidgetType = first ? first->data(TaskbarModel::WidgetTypeRole).toInt() : -1;
  if (firstWidgetType == static_cast<int>(SerialStudio::DashboardTerminal))
    insertRow = 1;

  m_fullModel->insertRow(insertRow, groupItem);

  auto* tbItem = groupItem->clone();
  setWindowState(tbItem->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->insertRow(insertRow, tbItem);

  Q_EMIT taskbarButtonsChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT searchResultsChanged();
#endif
}

/**
 * @brief Incrementally adds or removes the clock widget from the models.
 */
void UI::Taskbar::onClockToggled()
{
  auto& db            = UI::Dashboard::instance();
  const bool on       = db.clockEnabled();
  const int clockType = static_cast<int>(SerialStudio::DashboardClock);

  if (!on) {
    removeOverviewByType(clockType);
    Q_EMIT taskbarButtonsChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT searchResultsChanged();
    return;
  }

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == clockType)
      return;
  }

  int clockWindowId = -1;
  const auto& wm    = db.widgetMap();
  for (auto it = wm.begin(); it != wm.end(); ++it) {
    if (it.value().first != SerialStudio::DashboardClock)
      continue;

    clockWindowId = it.key();
    break;
  }

  if (clockWindowId < 0)
    return;

  int clockGroupId = -1;
  QString clockTitle;
  const auto& frame = db.processedFrame();
  for (const auto& g : frame.groups) {
    if (g.widget != QStringLiteral("clock"))
      continue;

    clockGroupId = g.groupId;
    clockTitle   = g.title;
    break;
  }

  const auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::DashboardClock, true);
  auto* groupItem = new QStandardItem();
  groupItem->setData(clockGroupId, TaskbarModel::GroupIdRole);
  groupItem->setData(clockTitle, TaskbarModel::GroupNameRole);
  groupItem->setData(clockTitle, TaskbarModel::WidgetNameRole);
  groupItem->setData(clockType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(icon, TaskbarModel::WidgetIconRole);
  groupItem->setData(clockWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(true, TaskbarModel::IsGroupRole);
  groupItem->setData(true, TaskbarModel::OverviewRole);
  groupItem->setData(static_cast<int>(TaskbarModel::WindowNormal), TaskbarModel::WindowStateRole);

  int insertRow = 0;
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item       = m_fullModel->item(i);
    const auto wtype = item ? item->data(TaskbarModel::WidgetTypeRole).toInt() : -1;
    if (wtype == static_cast<int>(SerialStudio::DashboardTerminal)
#ifdef BUILD_COMMERCIAL
        || wtype == static_cast<int>(SerialStudio::DashboardNotificationLog)
#endif
    )
      insertRow = i + 1;
    else
      break;
  }

  m_fullModel->insertRow(insertRow, groupItem);

  auto* tbItem = groupItem->clone();
  setWindowState(tbItem->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->insertRow(insertRow, tbItem);

  Q_EMIT taskbarButtonsChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT searchResultsChanged();
}

/**
 * @brief Incrementally adds or removes the stopwatch widget from the models.
 */
void UI::Taskbar::onStopwatchToggled()
{
  auto& db         = UI::Dashboard::instance();
  const bool on    = db.stopwatchEnabled();
  const int swType = static_cast<int>(SerialStudio::DashboardStopwatch);

  if (!on) {
    removeOverviewByType(swType);
    Q_EMIT taskbarButtonsChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT searchResultsChanged();
    return;
  }

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == swType)
      return;
  }

  int swWindowId = -1;
  const auto& wm = db.widgetMap();
  for (auto it = wm.begin(); it != wm.end(); ++it) {
    if (it.value().first != SerialStudio::DashboardStopwatch)
      continue;

    swWindowId = it.key();
    break;
  }

  if (swWindowId < 0)
    return;

  int swGroupId = -1;
  QString swTitle;
  const auto& frame = db.processedFrame();
  for (const auto& g : frame.groups) {
    if (g.widget != QStringLiteral("stopwatch"))
      continue;

    swGroupId = g.groupId;
    swTitle   = g.title;
    break;
  }

  const auto icon = SerialStudio::dashboardWidgetIcon(SerialStudio::DashboardStopwatch, true);
  auto* groupItem = new QStandardItem();
  groupItem->setData(swGroupId, TaskbarModel::GroupIdRole);
  groupItem->setData(swTitle, TaskbarModel::GroupNameRole);
  groupItem->setData(swTitle, TaskbarModel::WidgetNameRole);
  groupItem->setData(swType, TaskbarModel::WidgetTypeRole);
  groupItem->setData(icon, TaskbarModel::WidgetIconRole);
  groupItem->setData(swWindowId, TaskbarModel::WindowIdRole);
  groupItem->setData(true, TaskbarModel::IsGroupRole);
  groupItem->setData(true, TaskbarModel::OverviewRole);
  groupItem->setData(static_cast<int>(TaskbarModel::WindowNormal), TaskbarModel::WindowStateRole);

  int insertRow = 0;
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item       = m_fullModel->item(i);
    const auto wtype = item ? item->data(TaskbarModel::WidgetTypeRole).toInt() : -1;
    if (wtype == static_cast<int>(SerialStudio::DashboardTerminal)
#ifdef BUILD_COMMERCIAL
        || wtype == static_cast<int>(SerialStudio::DashboardNotificationLog)
#endif
        || wtype == static_cast<int>(SerialStudio::DashboardClock))
      insertRow = i + 1;
    else
      break;
  }

  m_fullModel->insertRow(insertRow, groupItem);

  auto* tbItem = groupItem->clone();
  setWindowState(tbItem->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->insertRow(insertRow, tbItem);

  Q_EMIT taskbarButtonsChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT searchResultsChanged();
}

//--------------------------------------------------------------------------------------------------
// Search functionality
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current search filter string.
 */
QString UI::Taskbar::searchFilter() const
{
  return m_searchFilter;
}

/**
 * @brief Clears the search filter and emits searchDismissed to close the popup.
 */
void UI::Taskbar::dismissSearch()
{
  m_searchFilter.clear();
  Q_EMIT searchFilterChanged();
  Q_EMIT searchResultsChanged();
  Q_EMIT searchDismissed();
}

/**
 * @brief Sets the search filter and recomputes search results.
 */
void UI::Taskbar::setSearchFilter(const QString& filter)
{
  if (m_searchFilter == filter)
    return;

  m_searchFilter = filter;
  Q_EMIT searchFilterChanged();
  Q_EMIT searchResultsChanged();
}

/**
 * @brief Returns a flat list of widgets matching the current search filter.
 */
QVariantList UI::Taskbar::searchResults() const
{
  QVariantList results;
  const auto filter   = m_searchFilter.trimmed();
  const bool noFilter = filter.isEmpty();

  for (int i = 0; i < m_fullModel->rowCount() && results.size() < 30; ++i) {
    auto* groupItem = m_fullModel->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    const auto groupWidgetName = groupItem->data(TaskbarModel::WidgetNameRole).toString();
    const auto groupType       = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (groupType != SerialStudio::DashboardNoWidget
        && (noFilter || groupWidgetName.contains(filter, Qt::CaseInsensitive)
            || groupName.contains(filter, Qt::CaseInsensitive))) {
      QVariantMap entry;
      entry[QStringLiteral("windowId")]    = groupItem->data(TaskbarModel::WindowIdRole);
      entry[QStringLiteral("widgetName")]  = groupWidgetName;
      entry[QStringLiteral("widgetIcon")]  = groupItem->data(TaskbarModel::WidgetIconRole);
      entry[QStringLiteral("widgetType")]  = groupType;
      entry[QStringLiteral("groupName")]   = groupName;
      entry[QStringLiteral("groupId")]     = groupItem->data(TaskbarModel::GroupIdRole);
      entry[QStringLiteral("isWorkspace")] = false;
      results.append(entry);
    }

    for (int j = 0; j < groupItem->rowCount() && results.size() < 30; ++j) {
      auto* child = groupItem->child(j);
      if (!child)
        continue;

      const auto childType = child->data(TaskbarModel::WidgetTypeRole).toInt();
      if (childType == SerialStudio::DashboardNoWidget)
        continue;

      const auto name = child->data(TaskbarModel::WidgetNameRole).toString();
      if (noFilter || name.contains(filter, Qt::CaseInsensitive)
          || groupName.contains(filter, Qt::CaseInsensitive)) {
        QVariantMap entry;
        entry[QStringLiteral("windowId")]    = child->data(TaskbarModel::WindowIdRole);
        entry[QStringLiteral("widgetName")]  = name;
        entry[QStringLiteral("widgetIcon")]  = child->data(TaskbarModel::WidgetIconRole);
        entry[QStringLiteral("widgetType")]  = child->data(TaskbarModel::WidgetTypeRole);
        entry[QStringLiteral("groupName")]   = groupName;
        entry[QStringLiteral("groupId")]     = child->data(TaskbarModel::GroupIdRole);
        entry[QStringLiteral("isWorkspace")] = false;
        results.append(entry);
      }
    }
  }

  return results;
}

/**
 * @brief Returns an unfiltered, unlimited flat list of every widget in the full model.
 */
QVariantList UI::Taskbar::allWidgets() const
{
  QVariantList results;

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* groupItem = m_fullModel->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    const auto groupType = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (groupType != SerialStudio::DashboardNoWidget) {
      QVariantMap entry;
      entry[QStringLiteral("windowId")]    = groupItem->data(TaskbarModel::WindowIdRole);
      entry[QStringLiteral("widgetName")]  = groupItem->data(TaskbarModel::WidgetNameRole);
      entry[QStringLiteral("widgetIcon")]  = groupItem->data(TaskbarModel::WidgetIconRole);
      entry[QStringLiteral("widgetType")]  = groupType;
      entry[QStringLiteral("groupName")]   = groupName;
      entry[QStringLiteral("groupId")]     = groupItem->data(TaskbarModel::GroupIdRole);
      entry[QStringLiteral("isWorkspace")] = false;
      results.append(entry);
    }

    for (int j = 0; j < groupItem->rowCount(); ++j) {
      auto* child = groupItem->child(j);
      if (!child)
        continue;

      const auto childType = child->data(TaskbarModel::WidgetTypeRole).toInt();
      if (childType == SerialStudio::DashboardNoWidget)
        continue;

      QVariantMap entry;
      entry[QStringLiteral("windowId")]    = child->data(TaskbarModel::WindowIdRole);
      entry[QStringLiteral("widgetName")]  = child->data(TaskbarModel::WidgetNameRole);
      entry[QStringLiteral("widgetIcon")]  = child->data(TaskbarModel::WidgetIconRole);
      entry[QStringLiteral("widgetType")]  = child->data(TaskbarModel::WidgetTypeRole);
      entry[QStringLiteral("groupName")]   = groupName;
      entry[QStringLiteral("groupId")]     = child->data(TaskbarModel::GroupIdRole);
      entry[QStringLiteral("isWorkspace")] = false;
      results.append(entry);
    }
  }

  return results;
}

//--------------------------------------------------------------------------------------------------
// Workspace model and management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the workspace model for the workspace selector.
 */
QVariantList UI::Taskbar::workspaceModel() const
{
  const auto& pm = DataModel::ProjectModel::instance();
  QVariantList model;
  const auto& workspaces = pm.activeWorkspaces();
  for (const auto& ws : workspaces) {
    QVariantMap entry;
    entry[QStringLiteral("id")]        = ws.workspaceId;
    entry[QStringLiteral("text")]      = ws.title;
    entry[QStringLiteral("separator")] = false;
    entry[QStringLiteral("icon")]      = ws.icon.isEmpty()
                                         ? QStringLiteral("qrc:/icons/panes/dashboard.svg")
                                         : SerialStudio::normalizeIconPath(ws.icon);
    model.append(entry);
  }

  return model;
}

/**
 * @brief Navigates to the workspace containing the given widget and shows it.
 */
void UI::Taskbar::navigateToWidget(int windowId, int groupId)
{
  for (int i = 0; i < m_taskbarButtons->rowCount(); ++i) {
    auto* item = m_taskbarButtons->item(i);
    if (item && item->data(TaskbarModel::WindowIdRole).toInt() == windowId) {
      auto* window = windowData(windowId);
      if (window) {
        showWindow(window);
        setActiveWindow(window);
      }

      Q_EMIT highlightWidget(windowId);
      return;
    }
  }

  if (m_activeGroupId >= WorkspaceIds::AutoStart) {
    addWidgetToActiveWorkspace(windowId);
    QTimer::singleShot(100, this, [this, windowId]() {
      auto* window = windowData(windowId);
      if (window) {
        showWindow(window);
        setActiveWindow(window);
      }

      Q_EMIT highlightWidget(windowId);
    });

    return;
  }

  setActiveGroupId(groupId);

  QTimer::singleShot(100, this, [this, windowId]() {
    auto* window = windowData(windowId);
    if (window) {
      showWindow(window);
      setActiveWindow(window);
    }

    Q_EMIT highlightWidget(windowId);
  });
}

/**
 * @brief Creates a new user-defined workspace and switches to it.
 */
void UI::Taskbar::createWorkspace(const QString& name)
{
  auto* pm = &DataModel::ProjectModel::instance();
  pm->addWorkspace(name);

  const auto& workspaces = pm->activeWorkspaces();
  if (!workspaces.empty())
    setActiveGroupId(workspaces.back().workspaceId);

  Q_EMIT workspaceModelChanged();
}

/**
 * @brief Deletes or hides a workspace.
 */
void UI::Taskbar::deleteWorkspace(int workspaceId)
{
  auto* pm = &DataModel::ProjectModel::instance();

  if (pm->customizeWorkspaces())
    if (workspaceId >= WorkspaceIds::AutoStart)
      pm->deleteWorkspace(workspaceId);
    else
      return;
  else if (workspaceId >= WorkspaceIds::PerGroupStart)
    pm->hideGroup(workspaceId - WorkspaceIds::PerGroupStart);
  else
    return;

  if (m_activeGroupId == workspaceId) {
    auto model = workspaceModel();
    if (!model.isEmpty())
      setActiveGroupId(model.first().toMap().value(QStringLiteral("id"), -1).toInt());
  }

  Q_EMIT workspaceModelChanged();
}

/**
 * @brief Renames a user-defined workspace.
 */
void UI::Taskbar::renameWorkspace(int workspaceId, const QString& name)
{
  if (workspaceId < WorkspaceIds::AutoStart)
    return;

  DataModel::ProjectModel::instance().renameWorkspace(workspaceId, name);
  Q_EMIT workspaceModelChanged();
}

/**
 * @brief Adds the widget identified by windowId to the active workspace.
 */
void UI::Taskbar::addWidgetToActiveWorkspace(int windowId)
{
  if (m_activeGroupId < WorkspaceIds::AutoStart)
    return;

  auto* item = findItemByWindowId(windowId);
  if (!item)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();

  auto& db        = UI::Dashboard::instance();
  const auto& map = db.widgetMap();
  int relIdx      = -1;
  for (auto it = map.begin(); it != map.end(); ++it) {
    if (it.key() == windowId) {
      relIdx = it.value().second;
      break;
    }
  }

  if (relIdx < 0)
    return;

  const int groupUid = UI::Dashboard::instance().groupUniqueIdForGroupId(groupId);
  DataModel::ProjectModel::instance().addWidgetToWorkspace(
    m_activeGroupId, widgetType, groupUid, relIdx);

  auto clone = item->clone();
  setWindowState(clone->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->appendRow(clone);
  Q_EMIT taskbarButtonsChanged();
}

/**
 * @brief Removes the widget identified by windowId from the active workspace.
 */
void UI::Taskbar::removeWidgetFromActiveWorkspace(int windowId)
{
  if (m_activeGroupId < WorkspaceIds::AutoStart)
    return;

  auto* item = findItemByWindowId(windowId);
  if (!item)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();
  const int relIdx      = relativeIndexForWindow(windowId);
  if (relIdx < 0)
    return;

  auto* pm               = &DataModel::ProjectModel::instance();
  const auto& workspaces = pm->activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != m_activeGroupId)
      continue;

    const int targetUid = UI::Dashboard::instance().groupUniqueIdForGroupId(groupId);
    for (size_t i = 0; i < ws.widgetRefs.size(); ++i) {
      const auto& ref = ws.widgetRefs[i];
      if (ref.widgetType != widgetType || ref.groupUniqueId != targetUid
          || ref.relativeIndex != relIdx)
        continue;

      pm->removeWidgetFromWorkspace(m_activeGroupId, static_cast<int>(i));
      if (auto* window = windowData(windowId))
        unregisterWindow(window);

      removeWorkspaceTaskbarRow(windowId);
      Q_EMIT taskbarButtonsChanged();
      return;
    }

    return;
  }
}

/**
 * @brief Removes the taskbar-button row matching windowId, if any.
 */
void UI::Taskbar::removeWorkspaceTaskbarRow(int windowId)
{
  for (int r = 0; r < m_taskbarButtons->rowCount(); ++r) {
    auto* tbItem = m_taskbarButtons->item(r);
    if (!tbItem || tbItem->data(TaskbarModel::WindowIdRole).toInt() != windowId)
      continue;

    m_taskbarButtons->removeRow(r);
    return;
  }
}

/**
 * @brief Returns window IDs of all widgets in the given user workspace, empty for non-user IDs.
 */
QVariantList UI::Taskbar::workspaceWidgetIds(int workspaceId) const
{
  QVariantList ids;
  if (workspaceId < WorkspaceIds::AutoStart)
    return ids;

  const auto& workspaces = DataModel::ProjectModel::instance().activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const int windowId = findWindowIdByGroupAndIndex(ref.widgetType, ref.relativeIndex);
      if (windowId < 0)
        continue;

      const int refGid = UI::Dashboard::instance().groupIdForUniqueId(ref.groupUniqueId);
      auto* item       = findItemByWindowId(windowId);
      if (!item || item->data(TaskbarModel::GroupIdRole).toInt() != refGid)
        continue;

      ids.append(windowId);
    }

    break;
  }

  return ids;
}

/**
 * @brief Replaces the widget list of a user workspace with the given IDs.
 */
void UI::Taskbar::setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds)
{
  if (workspaceId < WorkspaceIds::AutoStart)
    return;

  auto* pm = &DataModel::ProjectModel::instance();

  const auto& workspaces = pm->activeWorkspaces();
  bool found             = false;
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    found = true;
    for (int i = static_cast<int>(ws.widgetRefs.size()) - 1; i >= 0; --i)
      pm->removeWidgetFromWorkspace(workspaceId, i);

    break;
  }

  if (!found)
    return;

  const auto& widgetMap = UI::Dashboard::instance().widgetMap();
  for (const auto& idVar : windowIds) {
    const int windowId = idVar.toInt();
    auto* item         = findItemByWindowId(windowId);
    if (!item)
      continue;

    const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
    const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();

    int relIdx = -1;
    for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
      if (it.key() == windowId) {
        relIdx = it.value().second;
        break;
      }
    }

    if (relIdx >= 0) {
      const int groupUid = UI::Dashboard::instance().groupUniqueIdForGroupId(groupId);
      pm->addWidgetToWorkspace(workspaceId, widgetType, groupUid, relIdx);
    }
  }

  if (m_activeGroupId == workspaceId)
    setActiveGroupId(workspaceId);
}
