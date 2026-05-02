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
 *
 * Initializes a QStandardItemModel with custom roles defined by the
 * TaskbarModel for integration with QML. This model is used to represent
 * either:
 * - A full hierarchy of groups and their widgets (fullModel)
 * - A filtered subset based on the current UI state (taskbarButtons)
 *
 * @param parent Optional QObject parent.
 */
UI::TaskbarModel::TaskbarModel(QObject* parent) : QStandardItemModel(parent) {}

/**
 * @brief Returns the role names used for QML data binding.
 *
 * This function maps custom integer role identifiers to human-readable
 * property names that QML uses when accessing the model.
 *
 * Roles include:
 * - groupId: ID of the group the widget belongs to
 * - isGroup: Whether the item is a group or a widget
 * - windowId: The unique identifier of the window/widget
 * - groupName: Display name of the group
 * - widgetType: Enum type of the widget (e.g., FFT, Plot)
 * - widgetName: Display name of the widget
 * - widgetIcon: Path or ID of the widget's icon
 * - windowState: Current state of the window (normal, minimized, closed)
 *
 * @return A hash of role IDs to QML-visible role names.
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
 *
 * Initializes the internal models (`fullModel` and `taskbarButtons`) and
 * connects to `Dashboard::widgetCountChanged` to trigger a model rebuild when
 * the widget layout changes. Also registers `TaskbarModel` to QML for enum
 * access and role usage.
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
  // Register TaskbarModel as a QML-accessible type
  qmlRegisterUncreatableType<UI::TaskbarModel>(
    "SerialStudio.UI", 1, 0, "TaskbarModel", "TaskbarModel is exposed by Taskbar singleton");

  // Connect to widget registry lifecycle signals
  connectToRegistry();

  // Rebuild model when dashboard layout changes
  connect(&UI::Dashboard::instance(), &UI::Dashboard::dataReset, this, &UI::Taskbar::rebuildModel);
  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::widgetCountChanged,
          this,
          &UI::Taskbar::rebuildModel);

  // Handle terminal toggle incrementally (no full rebuild)
  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::terminalEnabledChanged,
          this,
          &UI::Taskbar::onTerminalToggled);

  // Handle notification-log toggle incrementally (Pro-only, no-op in GPL)
  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::notificationLogEnabledChanged,
          this,
          &UI::Taskbar::onNotificationLogToggled);

  // Sync active group selection with the project model
  auto* pm = &DataModel::ProjectModel::instance();
  connect(pm, &DataModel::ProjectModel::activeGroupIdChanged, this, [this, pm] {
    setActiveGroupId(pm->activeGroupId());
  });

  // Update workspace model when workspaces or full model changes
  connect(pm,
          &DataModel::ProjectModel::activeWorkspacesChanged,
          this,
          &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::workspaceModelChanged);
  connect(this, &UI::Taskbar::fullModelChanged, this, &UI::Taskbar::searchResultsChanged);

  // Perform initial model build and register with the session
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
 *
 * This allows incremental updates to the taskbar model when widgets are
 * created or destroyed, rather than always requiring a full rebuild.
 */
void UI::Taskbar::connectToRegistry()
{
  auto& registry = WidgetRegistry::instance();

  connect(&registry, &WidgetRegistry::widgetCreated, this, &Taskbar::onWidgetCreated);
  connect(&registry, &WidgetRegistry::widgetDestroyed, this, &Taskbar::onWidgetDestroyed);
  connect(&registry, &WidgetRegistry::registryCleared, this, &Taskbar::onRegistryCleared);
  connect(&registry, &WidgetRegistry::batchUpdateCompleted, this, &Taskbar::onBatchUpdateCompleted);
}

//--------------------------------------------------------------------------------------------------
// Taskbar class getter functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the currently active group ID.
 *
 * This is used by QML to determine which group's widgets should be displayed
 * in the taskbar button view.
 *
 * @return ID of the active group, or -1 for "All Data" mode.
 */
int UI::Taskbar::activeGroupId() const
{
  return m_activeGroupId;
}

/**
 * @brief Returns the index of the currently active group.
 *
 * Iterates through the list returned by workspaceModel() and finds the index
 * of the entry whose "id" matches the current active group ID. Returns -1 if
 * no entry matches, so QML bindings can handle the "not found" case explicitly
 * instead of receiving an out-of-bounds index.
 *
 * @return Index of the active group within the workspace model, or -1 if not
 *         found.
 */
int UI::Taskbar::activeGroupIndex() const
{
  // Walk the workspace model looking for the active group's entry
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

  // No match -- signal "not found" rather than an out-of-bounds index
  return -1;
}

/**
 * @brief Returns a flat list of group titles for use in QML tab bars or menus.
 *
 * The returned QVariantList contains entries for all known groups,
 * including an initial "All Data" entry with groupId -1.
 * Each item is a QVariantMap with:
 * - groupId: Group identifier (or -1 for All Data)
 * - groupName: Display name of the group
 * - widgetIcon: Icon path or identifier associated with the group type
 *
 * @return A list of groups as QVariantMap entries, including "All Data"
 */
QVariantList UI::Taskbar::groupModel() const
{
  // Retained for API stability; workspaces are now user-defined via workspaceModel()
  return QVariantList();
}

/**
 * @brief Returns the currently active QML window widget, if any.
 *
 * Used for visual focus, highlighting, or restoring window state.
 *
 * @return Pointer to the active QQuickItem window, or nullptr if none.
 */
QQuickItem* UI::Taskbar::activeWindow() const
{
  return m_activeWindow;
}

/**
 * @brief Returns the complete dashboard model containing all groups and
 *        widgets.
 *
 * This model is built during `rebuildModel()` and reflects the entire
 * dashboard layout as defined by the latest data frame structure.
 *
 * @return Pointer to the full TaskbarModel.
 */
UI::TaskbarModel* UI::Taskbar::fullModel() const
{
  return m_fullModel;
}

/**
 * @brief Returns the filtered model currently shown in the taskbar.
 *
 * This model may represent:
 * - Widgets grouped by type (All Data view)
 * - Widgets belonging to a specific group (Group view)
 *
 * @return Pointer to the filtered TaskbarModel.
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
 *
 * Iterates through all window IDs and returns true if at least one
 * window is in the "maximized" state.
 *
 * @return true if a maximized window exists, false otherwise.
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
 *
 * Looks up the internal window ID -> QQuickItem map to retrieve the actual
 * instance.
 *
 * @param id The windowId of the widget.
 * @return The corresponding QQuickItem pointer, or nullptr if not found.
 */
QQuickItem* UI::Taskbar::windowData(const int id) const
{
  for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it)
    if (it.value() == id)
      return it.key();

  return nullptr;
}

/**
 * @brief Returns the current state of a window widget.
 *
 * Given a QQuickItem window, this function:
 * - Looks up its associated window ID using `m_windowIDs`
 * - Searches for the corresponding item in the taskbar model
 * - Returns the stored window state (Normal, Minimized, Closed)
 *
 * If the window is not registered or found, `WindowClosed` is returned as a
 * fallback.
 *
 * @param window Pointer to the QML window (QQuickItem).
 * @return The window's state from the taskbar model.
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

  // Snapshot the visible, non-closed windows in taskbar order.
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
 * @brief Serializes the active group's layout and writes it to the project file.
 */
void UI::Taskbar::saveLayout()
{
  // Abort if no window manager, no windows, or invalid group
  if (!m_windowManager || m_windowIDs.isEmpty() || m_activeGroupId < -2)
    return;

  // Skip while a restore is mid-flight; restoreLayout's emissions would feed back into saveLayout
  if (m_restoringLayout)
    return;

  // Only persist layout in project file mode
  const auto opMode = AppState::instance().operationMode();
  if (opMode != SerialStudio::ProjectFile)
    return;

  // No project file loaded, nothing to save to
  auto* model = &DataModel::ProjectModel::instance();
  if (model->jsonFilePath().isEmpty())
    return;

  // Wait until all windows are registered before saving
  if (m_taskbarButtons && m_windowIDs.count() < m_taskbarButtons->rowCount())
    return;

  // Serialize and persist the current layout
  model->saveWidgetSetting(
    Keys::layoutKey(m_activeGroupId), QStringLiteral("data"), m_windowManager->serializeLayout());
}

//--------------------------------------------------------------------------------------------------
// Taskbar group selection code (e.g. when a tab is selected in the tab bar)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active group and updates taskbar buttons accordingly.
 *
 * This method clears the current `taskbarButtons` model and populates it
 * with widgets belonging to the specified group. It also:
 * - Clears the current active window reference
 * - Resets window states in the full model (minimizing inactive groups)
 * - Clones the selected group and its widgets into the visible taskbar
 *
 * If `groupId` is -1, all widgets are restored to visible state.
 *
 * @param groupId ID of the group to activate, or -1 for all groups.
 */
void UI::Taskbar::setActiveGroupId(int groupId)
{
  // Persist current layout before switching
  saveLayout();

  // Persist active group (skip during rebuilds to keep saved selection)
  if (!m_rebuildInProgress)
    DataModel::ProjectModel::instance().setActiveGroupId(groupId);

  // Disconnect all window state connections before clearing
  for (auto it = m_windowConnections.begin(); it != m_windowConnections.end(); ++it)
    disconnect(*it);

  m_windowConnections.clear();

  // Reset models and window state
  m_windowIDs.clear();
  m_taskbarButtons->clear();
  if (m_windowManager)
    m_windowManager->clear();

  m_activeWindow  = nullptr;
  m_activeGroupId = groupId;

  // Add global overview rows first (terminal, then notification log)
  cloneSpecialOverviewRow(SerialStudio::DashboardTerminal);
#ifdef BUILD_COMMERCIAL
  cloneSpecialOverviewRow(SerialStudio::DashboardNotificationLog);
#endif

  // Populate the rest from either a user workspace or an auto-generated group
  if (groupId >= 1000)
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

      auto* item = findItemByWindowId(windowId);
      if (!item || item->data(TaskbarModel::GroupIdRole).toInt() != ref.groupId)
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

    // Skip terminal and notification-log overview rows (already added)
    const auto type = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (type == SerialStudio::DashboardTerminal)
      continue;

#ifdef BUILD_COMMERCIAL
    if (type == SerialStudio::DashboardNotificationLog)
      continue;
#endif

    // Filter by group ID when viewing a specific group
    if (groupId > -1 && groupItem->data(TaskbarModel::GroupIdRole).toInt() != groupId)
      continue;

    // Append the group row itself when it has its own widget
    auto* group = groupItem->clone();
    if (type != SerialStudio::DashboardNoWidget) {
      setWindowState(group->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
      m_taskbarButtons->appendRow(group);
    }

    // Append child rows (datasets and sub-groups) for this group
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

      // Legacy "All Data" fallback for older session settings (groupId == -2)
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
 *
 * Retrieves the group at the specified index from the group model and updates
 * the active group ID accordingly. If the index is out of bounds or invalid,
 * the method does nothing.
 *
 * @param index The position of the group within the groupModel() list.
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
 *
 * Looks up the window's ID and updates its state to WindowNormal.
 *
 * @param window Pointer to the QML widget.
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
 *
 * The window will no longer appear in the visible taskbar.
 *
 * @param window Pointer to the QML widget.
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
 *
 * The window remains registered and retrievable via hidden widgets or
 * reactivation.
 *
 * @param window Pointer to the QML widget.
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
 *
 * Updates the `activeWindow` property and emits a change signal if it differs
 * from the previous active window.
 *
 * @param window Pointer to the QML window that should become active.
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
 *
 * Removes the association between the given QQuickItem and its internal window
 * ID, effectively detaching it from the taskbar state tracking.
 *
 * This is typically called when a window is destroyed or removed from the
 * scene, ensuring no stale references are kept.
 *
 * Emits `registeredWindowsChanged()` if the window was found and removed.
 *
 * @param window Pointer to the QML window to unregister.
 */
void UI::Taskbar::unregisterWindow(QQuickItem* window)
{
  if (m_windowIDs.contains(window)) {
    auto it = m_windowConnections.find(window);
    if (it != m_windowConnections.end()) {
      disconnect(*it);
      m_windowConnections.erase(it);
    }

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

  // Persist layout whenever a window geometry changes
  connect(m_windowManager, &UI::WindowManager::geometryChanged, this, [this](QQuickItem* item) {
    if (item != nullptr)
      saveLayout();
  });

  // Persist auto-layout toggle so it survives the next launch even without a window move
  connect(
    m_windowManager, &UI::WindowManager::autoLayoutEnabledChanged, this, &UI::Taskbar::saveLayout);

  Q_EMIT windowManagerChanged();
}

/**
 * @brief Registers a QML window with a corresponding internal window ID.
 *
 * This establishes a link between the UI object and the internal model,
 * allowing state management functions (minimize, close, etc.) to operate.
 *
 * @param id Internal window identifier.
 * @param window Pointer to the QML window.
 */
void UI::Taskbar::registerWindow(const int id, QQuickItem* window)
{
  if (!window)
    return;

  // Associate window with its internal ID
  m_windowIDs.insert(window, id);
  if (m_windowManager)
    m_windowManager->registerWindow(id, window);

  Q_EMIT registeredWindowsChanged();

  // Forward window state changes to the taskbar
  m_windowConnections[window] =
    connect(window, &QQuickItem::stateChanged, this, [=, this] { Q_EMIT statesChanged(); });

  // Restore saved layout once all windows are registered
  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager) {
    m_restoringLayout = true;
    const auto opMode = AppState::instance().operationMode();
    if (opMode == SerialStudio::ProjectFile) {
      const auto layout = DataModel::ProjectModel::instance().groupLayout(m_activeGroupId);
      if (!layout.isEmpty() && m_windowManager->restoreLayout(layout)) {
        m_restoringLayout = false;
        return;
      }
    }

    m_windowManager->loadLayout();
    m_restoringLayout = false;
  }
}

/**
 * @brief Updates the window state for a given internal ID.
 *
 * Finds the corresponding item in the full model and updates its state
 * to one of: WindowNormal, WindowMinimized, or WindowClosed.
 *
 * @param id Internal window ID.
 * @param state Desired window state.
 */
void UI::Taskbar::setWindowState(const int id, const UI::TaskbarModel::WindowState state)
{
  QStandardItem* item = findItemByWindowId(id);
  if (!item)
    return;

  // Apply the new state and notify the UI
  item->setData(state, UI::TaskbarModel::WindowStateRole);
  Q_EMIT windowStatesChanged();

  // Trigger a layout refresh when all windows are registered
  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager)
    m_windowManager->triggerLayoutUpdate();
}

//--------------------------------------------------------------------------------------------------
// General (full) model generation function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the internal full model to reflect the current dashboard
 *        layout.
 *
 * This function is triggered when the widget structure changes (e.g., due to a
 * new data frame). It regenerates the `fullModel`, which is a hierarchical
 * representation of all widgets grouped by their data groups, maintaining the
 * user's declared order and layout.
 *
 * What it does:
 * - Clears previous state and active window
 * - Iterates over all groups from the latest DataModel frame
 * - For each group, finds all associated widgets (both group-level and
 *   dataset-level)
 * - Builds a hierarchical QStandardItem tree for the group and its widgets
 * - Stores primary window IDs and widget types for later lookup
 * - Emits change notifications to update the UI bindings
 *
 * Grouping logic:
 * - Each top-level group is registered only once
 * - Any duplicate group references (composite widgets) are added as subgroups
 *
 * This is the authoritative source for all UI view models like tabs and
 * taskbars.
 */
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
  // Guard against re-entrant rebuilds
  if (m_rebuildInProgress)
    return;

  m_rebuildInProgress = true;
  {
    QSignalBlocker fullBlocker(m_fullModel);
    QSignalBlocker taskbarBlocker(m_taskbarButtons);

    // Disconnect all window state connections before clearing
    for (auto it = m_windowConnections.begin(); it != m_windowConnections.end(); ++it)
      disconnect(*it);

    m_windowConnections.clear();

    // Clear all state
    m_windowIDs.clear();
    m_fullModel->clear();
    m_activeWindow = nullptr;
    m_widgetIdToWindowId.clear();
    m_windowIdToWidgetId.clear();
    if (m_windowManager)
      m_windowManager->clear();
  }

  auto* db = &UI::Dashboard::instance();

  // Obtain and validate latest frame
  const auto& frame = db->processedFrame();
  if (frame.title.isEmpty() || frame.groups.size() <= 0) {
    setActiveGroupId(-1);
    Q_EMIT fullModelChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT registeredWindowsChanged();
    m_rebuildInProgress = false;
    return;
  }

  // Build group items from the dashboard frame
  QSet<int> groupIds;
  for (const DataModel::Group& group : frame.groups) {
    const auto groupId   = group.groupId;
    const auto groupName = group.title;
    const auto groupType = SerialStudio::getDashboardWidget(group);

    // Collect window IDs associated with this group
    QList<int> windowIds;
    QList<int> relativeIds;
    QList<SerialStudio::DashboardWidget> widgetTypes;
    collectGroupWidgetIds(groupId, windowIds, relativeIds, widgetTypes);

    // Extract the main group widget (-1 sentinel -- 0 is a valid windowId)
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

    // Build the group model item and map its main widget
    auto* groupItem              = new QStandardItem();
    const bool alreadyRegistered = groupIds.contains(groupId);
    buildOverviewGroupItem(
      groupItem, groupId, groupName, groupType, mainWindowId, alreadyRegistered);
    mapMainGroupWidgetId(groupType, groupId, mainWindowId);

    // Append child widgets for this group
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

  // Set title and map widget ID for sub-groups
  if (SerialStudio::isGroupWidget(widgetType)) {
    const auto& dbGroup = db->getGroupWidget(widgetType, relativeIndex);
    child->setData(dbGroup.title, TaskbarModel::WidgetNameRole);
    child->setData(false, TaskbarModel::OverviewRole);
    mapWidgetToWindow(registry.widgetIdByTypeAndIndex(widgetType, relativeIndex), windowId);
  }

  // Set title and map widget ID for datasets
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
 *
 * This function performs a depth-first traversal of the full taskbar model,
 * comparing each item's `WindowIdRole` to the given ID. This includes both
 * group widgets and dataset widgets, since either can be associated with a
 * window.
 *
 * @param windowId The internal window identifier to find.
 * @param parentItem Optional parent item to limit search scope.
 * @return Pointer to the matching QStandardItem, or nullptr if not found.
 */
QStandardItem* UI::Taskbar::findItemByWindowId(int windowId,
                                               QStandardItem* parentItem,
                                               int depth) const
{
  // Bounded recursion (NASA PoT Rule 1) -- tree is at most 2 levels deep.
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

    // Recurse into group children
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
 *
 * Similar to findItemByWindowId but uses the stable widget ID from the
 * WidgetRegistry. This is useful when processing registry events.
 *
 * @param widgetId The stable widget ID from the registry.
 * @param parentItem Optional parent item to limit search scope.
 * @return Pointer to the matching QStandardItem, or nullptr if not found.
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
 *
 * @param groupId The group ID to search for.
 * @return Pointer to the group item, or nullptr if not found.
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
 *
 * @param info The widget info from the registry.
 * @return A new QStandardItem populated with the widget's data.
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
 *
 * When a batch update is in progress (during reconfigureDashboard), this
 * method defers action since a full rebuild will happen anyway.
 *
 * For single widget additions (like toggling terminal), this method
 * adds the widget incrementally without rebuilding the entire model.
 *
 * @param id The stable widget ID.
 * @param info Complete information about the new widget.
 */
void UI::Taskbar::onWidgetCreated(UI::WidgetID id, const UI::WidgetInfo& info)
{
  Q_UNUSED(id)
  Q_UNUSED(info)
}

/**
 * @brief Handles widget destruction events from the registry.
 *
 * When a batch update is in progress, this method defers action.
 *
 * For single widget removals (like disabling terminal), this method
 * removes the widget incrementally.
 *
 * @param id The stable widget ID being destroyed.
 */
void UI::Taskbar::onWidgetDestroyed(UI::WidgetID id)
{
  Q_UNUSED(id)
}

/**
 * @brief Handles registry clear events.
 *
 * This is called when the entire registry is cleared, typically during
 * resetData(). The taskbar should clear its ID mappings but wait for
 * the subsequent dataReset signal to trigger a full rebuild.
 */
void UI::Taskbar::onRegistryCleared()
{
  m_widgetIdToWindowId.clear();
  m_windowIdToWidgetId.clear();
}

/**
 * @brief Handles batch update completion from the registry.
 *
 * This is called after all widgets have been created/destroyed during
 * a batch operation (like reconfigureDashboard). This is where we can
 * trigger expensive operations like layout recalculation.
 */
void UI::Taskbar::onBatchUpdateCompleted() {}

/**
 * @brief Incrementally adds or removes the terminal widget from the models.
 *
 * This avoids a full rebuildModel() which would destroy all windows and
 * lose layout state.
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

  // Skip if terminal already exists in fullModel (e.g. after rebuildModel)
  const int termType = static_cast<int>(SerialStudio::DashboardTerminal);
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == termType)
      return;
  }

  // Find the terminal entry in the widget map
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

  // Find terminal group info
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

  // Add to fullModel
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

  // Add to taskbarButtons (always first)
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
  // Remove from fullModel
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (!item || item->data(TaskbarModel::WidgetTypeRole).toInt() != widgetType)
      continue;

    m_fullModel->removeRow(i);
    break;
  }

  // Remove from taskbarButtons and unregister the window
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

  // Skip if already present
  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* item = m_fullModel->item(i);
    if (item && item->data(TaskbarModel::WidgetTypeRole).toInt() == logType)
      return;
  }

  // Locate the widget map entry created by Dashboard::setNotificationLogEnabled
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

  // Resolve the synthetic group that Dashboard injected
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

  // Build a row for the full model
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

  // Place after Terminal (if present) so Terminal stays first
  int insertRow             = 0;
  auto* first               = m_fullModel->rowCount() > 0 ? m_fullModel->item(0) : nullptr;
  const int firstWidgetType = first ? first->data(TaskbarModel::WidgetTypeRole).toInt() : -1;
  if (firstWidgetType == static_cast<int>(SerialStudio::DashboardTerminal))
    insertRow = 1;

  m_fullModel->insertRow(insertRow, groupItem);

  // Mirror into taskbarButtons
  auto* tbItem = groupItem->clone();
  setWindowState(tbItem->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->insertRow(insertRow, tbItem);

  Q_EMIT taskbarButtonsChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT searchResultsChanged();
#endif
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
 *
 * Each entry is a QVariantMap with: windowId, widgetName, widgetIcon,
 * widgetType, groupName, groupId. Limited to 30 results. Workspace entries
 * are not included -- search is for navigating to individual widgets only.
 */
QVariantList UI::Taskbar::searchResults() const
{
  QVariantList results;
  const auto filter   = m_searchFilter.trimmed();
  const bool noFilter = filter.isEmpty();

  // Search group and child widgets against the filter
  for (int i = 0; i < m_fullModel->rowCount() && results.size() < 30; ++i) {
    auto* groupItem = m_fullModel->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    // Check group-level widget
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

    // Check child widgets
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
 * @brief Returns a flat list of every widget in the full model.
 *
 * Unlike searchResults(), this function applies no filter and no result limit.
 * Each entry is a QVariantMap with: windowId, widgetName, widgetIcon,
 * widgetType, groupName, groupId, isWorkspace.
 *
 * Used by the workspace dialog, which needs to display every available widget.
 */
QVariantList UI::Taskbar::allWidgets() const
{
  QVariantList results;

  for (int i = 0; i < m_fullModel->rowCount(); ++i) {
    auto* groupItem = m_fullModel->item(i);
    if (!groupItem)
      continue;

    const auto groupName = groupItem->data(TaskbarModel::GroupNameRole).toString();

    // Add group-level widget
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

    // Add child widgets
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
 *
 * Includes auto-generated entries (same as groupModel) plus user-defined
 * workspaces from ProjectModel. User workspaces have IDs >= 1000.
 */
QVariantList UI::Taskbar::workspaceModel() const
{
  // Start with auto-generated entries, skipping hidden groups
  const auto& pm     = DataModel::ProjectModel::instance();
  const auto& hidden = pm.hiddenGroupIds();
  QVariantList model;
  for (const auto& entry : groupModel()) {
    const int id = entry.toMap().value(QStringLiteral("id")).toInt();
    if (id >= 0 && hidden.contains(id))
      continue;

    model.append(entry);
  }

  // Append user-defined workspaces
  const auto& workspaces = pm.activeWorkspaces();
  for (const auto& ws : workspaces) {
    QVariantMap entry;
    entry[QStringLiteral("id")]        = ws.workspaceId;
    entry[QStringLiteral("text")]      = ws.title;
    entry[QStringLiteral("separator")] = false;
    entry[QStringLiteral("icon")] =
      ws.icon.isEmpty() ? QStringLiteral("qrc:/icons/panes/dashboard.svg") : ws.icon;
    model.append(entry);
  }

  return model;
}

/**
 * @brief Navigates to the workspace containing the given widget and shows it.
 *
 * Logic:
 * 1. If the widget is already visible in the current workspace, just focus it.
 * 2. Otherwise, switch to the widget's owning group workspace.
 * 3. After switching, find and focus the window.
 *
 * For user-defined workspaces (id >= 1000), the widget is added to the
 * workspace if not already present.
 */
void UI::Taskbar::navigateToWidget(int windowId, int groupId)
{
  // Focus widget if already visible in current workspace
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

  // For user-defined workspaces, add the widget and show it
  if (m_activeGroupId >= 1000) {
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

  // Switch to the group workspace that owns this widget
  setActiveGroupId(groupId);

  // Delay focus to let QML create the delegates
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
 *
 * For user-defined workspaces (id >= 1000), permanently deletes the workspace.
 * For auto-generated group workspaces (id >= 0, id < 1000), hides the group
 * from the workspace list. Special workspaces (Overview, All Data, per-source)
 * cannot be deleted.
 *
 * If the affected workspace is active, switches to the first available.
 */
void UI::Taskbar::deleteWorkspace(int workspaceId)
{
  auto* pm = &DataModel::ProjectModel::instance();

  // User-defined workspace: delete permanently
  if (workspaceId >= 1000)
    pm->deleteWorkspace(workspaceId);

  // Auto-generated group workspace: hide it
  else if (workspaceId >= 0)
    pm->hideGroup(workspaceId);

  // Special workspaces (id < 0): not deletable
  else
    return;

  // Switch away if we deleted/hid the active workspace
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
  if (workspaceId < 1000)
    return;

  DataModel::ProjectModel::instance().renameWorkspace(workspaceId, name);
  Q_EMIT workspaceModelChanged();
}

/**
 * @brief Adds the widget identified by windowId to the active workspace.
 *
 * Only applies to user-defined workspaces (id >= 1000). For auto-generated
 * workspaces this is a no-op since their content is derived from groups.
 */
void UI::Taskbar::addWidgetToActiveWorkspace(int windowId)
{
  if (m_activeGroupId < 1000)
    return;

  // Find the widget info from fullModel
  auto* item = findItemByWindowId(windowId);
  if (!item)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();

  // Look up relative index from the dashboard widget map
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

  DataModel::ProjectModel::instance().addWidgetToWorkspace(
    m_activeGroupId, widgetType, groupId, relIdx);

  // Add to taskbar buttons and show
  auto clone = item->clone();
  setWindowState(clone->data(TaskbarModel::WindowIdRole).toInt(), TaskbarModel::WindowNormal);
  m_taskbarButtons->appendRow(clone);
  Q_EMIT taskbarButtonsChanged();
}

/**
 * @brief Removes the widget identified by windowId from the active workspace.
 *
 * Only applies to user-defined workspaces (id >= 1000).
 */
void UI::Taskbar::removeWidgetFromActiveWorkspace(int windowId)
{
  if (m_activeGroupId < 1000)
    return;

  // Resolve the widget info that identifies the ref to remove
  auto* item = findItemByWindowId(windowId);
  if (!item)
    return;

  const auto widgetType = item->data(TaskbarModel::WidgetTypeRole).toInt();
  const auto groupId    = item->data(TaskbarModel::GroupIdRole).toInt();
  const int relIdx      = relativeIndexForWindow(windowId);
  if (relIdx < 0)
    return;

  // Locate the active workspace and remove the matching ref
  auto* pm               = &DataModel::ProjectModel::instance();
  const auto& workspaces = pm->activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != m_activeGroupId)
      continue;

    for (size_t i = 0; i < ws.widgetRefs.size(); ++i) {
      const auto& ref = ws.widgetRefs[i];
      if (ref.widgetType != widgetType || ref.groupId != groupId || ref.relativeIndex != relIdx)
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
 * @brief Returns window IDs of all widgets in the given user workspace.
 *
 * Used by the workspace editor dialog to pre-check widgets that are already
 * in the workspace. Returns an empty list for non-user workspaces.
 */
QVariantList UI::Taskbar::workspaceWidgetIds(int workspaceId) const
{
  QVariantList ids;
  if (workspaceId < 1000)
    return ids;

  const auto& workspaces = DataModel::ProjectModel::instance().activeWorkspaces();
  for (const auto& ws : workspaces) {
    if (ws.workspaceId != workspaceId)
      continue;

    for (const auto& ref : ws.widgetRefs) {
      const int windowId = findWindowIdByGroupAndIndex(ref.widgetType, ref.relativeIndex);
      if (windowId < 0)
        continue;

      auto* item = findItemByWindowId(windowId);
      if (!item || item->data(TaskbarModel::GroupIdRole).toInt() != ref.groupId)
        continue;

      ids.append(windowId);
    }

    break;
  }

  return ids;
}

/**
 * @brief Replaces the widget list of a user workspace with the given IDs.
 *
 * Clears existing widget refs and rebuilds from the provided window IDs.
 * If the edited workspace is currently active, refreshes the taskbar.
 */
void UI::Taskbar::setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds)
{
  if (workspaceId < 1000)
    return;

  auto* pm = &DataModel::ProjectModel::instance();

  // Locate the workspace and clear its existing widget refs (iterate backwards)
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

  // Abort if the workspace no longer exists
  if (!found)
    return;

  // Add new refs from window IDs
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

    if (relIdx >= 0)
      pm->addWidgetToWorkspace(workspaceId, widgetType, groupId, relIdx);
  }

  // Refresh taskbar if this workspace is active
  if (m_activeGroupId == workspaceId)
    setActiveGroupId(workspaceId);
}
