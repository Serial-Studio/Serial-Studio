/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QTimer>
#include <QGuiApplication>
#include <QSignalBlocker>

#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "UI/WindowManager.h"
#include "UI/WidgetRegistry.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/FrameBuilder.h"

//------------------------------------------------------------------------------
// Taskbar model implementation
//------------------------------------------------------------------------------

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
UI::TaskbarModel::TaskbarModel(QObject *parent)
  : QStandardItemModel(parent)
{
}

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

//------------------------------------------------------------------------------
// Taskbar class implementation
//------------------------------------------------------------------------------

/**
 * @brief Private constructor for the Taskbar singleton.
 *
 * Initializes the internal models (`fullModel` and `taskbarButtons`) and
 * connects to `Dashboard::widgetCountChanged` to trigger a model rebuild when
 * the widget layout changes. Also registers `TaskbarModel` to QML for enum
 * access and role usage.
 */
UI::Taskbar::Taskbar(QQuickItem *parent)
  : QQuickItem(parent)
  , m_activeWindow(nullptr)
  , m_windowManager(nullptr)
  , m_fullModel(new TaskbarModel(this))
  , m_taskbarButtons(new TaskbarModel(this))
{
  qmlRegisterUncreatableType<UI::TaskbarModel>(
      "SerialStudio.UI", 1, 0, "TaskbarModel",
      "TaskbarModel is exposed by Taskbar singleton");

  connectToRegistry();

  connect(&UI::Dashboard::instance(), &UI::Dashboard::dataReset, this,
          &UI::Taskbar::rebuildModel);
  connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetCountChanged, this,
          &UI::Taskbar::rebuildModel);

  auto *pm = &DataModel::ProjectModel::instance();
  connect(pm, &DataModel::ProjectModel::dashboardLayoutChanged, this,
          &UI::Taskbar::onDashboardLayoutChanged);
  connect(pm, &DataModel::ProjectModel::activeGroupIdChanged, this,
          [this, pm] { setActiveGroupId(pm->activeGroupId()); });

  connect(qApp, &QGuiApplication::aboutToQuit, this, &UI::Taskbar::saveLayout);

  auto *io = &IO::Manager::instance();
  connect(io, &IO::Manager::connectedChanged, this, [this, io] {
    if (!io->isConnected())
      saveLayout();
    else
      onDashboardLayoutChanged();
  });

  rebuildModel();
}

/**
 * @brief Connects the Taskbar to the WidgetRegistry's lifecycle signals.
 *
 * This allows incremental updates to the taskbar model when widgets are
 * created or destroyed, rather than always requiring a full rebuild.
 */
void UI::Taskbar::connectToRegistry()
{
  auto &registry = WidgetRegistry::instance();

  connect(&registry, &WidgetRegistry::widgetCreated, this,
          &Taskbar::onWidgetCreated);
  connect(&registry, &WidgetRegistry::widgetDestroyed, this,
          &Taskbar::onWidgetDestroyed);
  connect(&registry, &WidgetRegistry::registryCleared, this,
          &Taskbar::onRegistryCleared);
  connect(&registry, &WidgetRegistry::batchUpdateCompleted, this,
          &Taskbar::onBatchUpdateCompleted);
}

//------------------------------------------------------------------------------
// Taskbar class getter functions
//------------------------------------------------------------------------------

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
 * Iterates through the list returned by groupModel() and finds the index
 * of the group whose "id" matches the current active group ID
 * (m_activeGroupId). If no match is found, the method returns the total number
 * of groups, which may indicate an invalid or unassigned state.
 *
 * @return Index of the active group within the group model.
 */
int UI::Taskbar::activeGroupIndex() const
{
  int index = 0;
  const auto model = groupModel();
  for (auto it = model.begin(); it != model.end(); ++it)
  {
    auto map = it->toMap();
    if (map.contains("id"))
    {
      auto id = map.value("id").toInt();
      if (id == m_activeGroupId)
        break;

      ++index;
    }
  }

  return index;
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
  // Initialize the model
  QVariantList model;

  // Count number of widgets for overview section
  int groupCount = 0;
  int widgetGroups = 0;
  for (const auto &group : UI::Dashboard::instance().rawFrame().groups)
  {
    ++groupCount;
    auto widget = SerialStudio::getDashboardWidget(group);
    if (widget != SerialStudio::DashboardNoWidget)
      ++widgetGroups;

    for (const auto &dataset : group.datasets)
    {
      if (dataset.overviewDisplay)
        ++widgetGroups;
    }
  }

  // Create overview group
  if (widgetGroups > 1)
  {
    QVariantMap main;
    main["id"] = -1;
    main["text"] = tr("Overview");
    main["icon"] = QStringLiteral("qrc:/rcc/icons/panes/overview.svg");
    model.append(main);
  }

  // Create all data group
  if (groupCount > 1)
  {
    QVariantMap main;
    main["id"] = -2;
    main["text"] = tr("All Data");
    main["icon"] = QStringLiteral("qrc:/rcc/icons/panes/dashboard.svg");
    model.append(main);
  }

  // Append frame groups
  for (int i = 0; i < m_fullModel->rowCount(); ++i)
  {
    // Validate that the group is valid
    const QStandardItem *groupItem = m_fullModel->item(i);
    if (!groupItem)
      continue;

    // Register the group in the model
    QVariantMap group;
    group["id"] = groupItem->data(TaskbarModel::GroupIdRole).toInt();
    group["text"] = groupItem->data(TaskbarModel::GroupNameRole).toString();
    group["icon"] = groupItem->data(TaskbarModel::WidgetIconRole).toString();
    model.append(group);
  }

  // Return obtained model
  return model;
}

/**
 * @brief Returns the currently active QML window widget, if any.
 *
 * Used for visual focus, highlighting, or restoring window state.
 *
 * @return Pointer to the active QQuickItem window, or nullptr if none.
 */
QQuickItem *UI::Taskbar::activeWindow() const
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
UI::TaskbarModel *UI::Taskbar::fullModel() const
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
UI::TaskbarModel *UI::Taskbar::taskbarButtons() const
{
  return m_taskbarButtons;
}

/**
 * @brief Returns a pointer to the window manager object.
 */
UI::WindowManager *UI::Taskbar::windowManager() const
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
  {
    if (it.key()->state() == QStringLiteral("maximized"))
      return true;
  }

  return false;
}

/**
 * @brief Returns the QML window (QQuickItem) for a given window ID.
 *
 * Looks up the internal window ID → QQuickItem map to retrieve the actual
 * instance.
 *
 * @param id The windowId of the widget.
 * @return The corresponding QQuickItem pointer, or nullptr if not found.
 */
QQuickItem *UI::Taskbar::windowData(const int id) const
{
  for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it)
  {
    if (it.value() == id)
      return it.key();
  }

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
UI::TaskbarModel::WindowState UI::Taskbar::windowState(QQuickItem *window) const
{
  if (window)
  {
    const int id = m_windowIDs.value(window, -1);
    if (id > -1)
    {
      auto item = findItemByWindowId(id);
      if (item)
      {
        return static_cast<TaskbarModel::WindowState>(
            item->data(TaskbarModel::WindowStateRole).toInt());
      }
    }
  }

  return TaskbarModel::WindowClosed;
}

//------------------------------------------------------------------------------
// Taskbar group selection code (e.g. when a tab is selected in the tab bar)
//------------------------------------------------------------------------------

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
  // Reset the models
  m_windowIDs.clear();
  m_taskbarButtons->clear();

  // Clear windows in window manager
  if (m_windowManager)
    m_windowManager->clear();

  // Reset active window
  m_activeWindow = nullptr;
  m_activeGroupId = groupId;

  // Add console first
  for (int i = 0; i < fullModel()->rowCount(); ++i)
  {
    auto groupItem = fullModel()->item(i);
    if (groupItem)
    {
      const auto type = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
      if (type == SerialStudio::DashboardTerminal)
      {
        auto g = groupItem->clone();
        setWindowState(g->data(TaskbarModel::WindowIdRole).toInt(),
                       TaskbarModel::WindowNormal);
        m_taskbarButtons->appendRow(g);
        break;
      }
    }
  }

  // Add group children as taskbar buttons
  for (int i = 0; i < fullModel()->rowCount(); ++i)
  {
    // Get & validate group item
    auto groupItem = fullModel()->item(i);
    if (!groupItem)
      continue;

    // Get widget type, skip terminal as we already added it
    auto type = groupItem->data(TaskbarModel::WidgetTypeRole).toInt();
    if (type == SerialStudio::DashboardTerminal)
      continue;

    // Verify if group item matches group ID
    if (groupId > -1)
    {
      if (groupItem->data(TaskbarModel::GroupIdRole).toInt() != groupId)
        continue;
    }

    // Clone the group
    auto group = groupItem->clone();
    if (type != SerialStudio::DashboardNoWidget)
    {
      setWindowState(group->data(TaskbarModel::WindowIdRole).toInt(),
                     TaskbarModel::WindowNormal);
      m_taskbarButtons->appendRow(group);
    }

    // Append group children
    const auto groupName = group->data(TaskbarModel::WidgetNameRole).toString();
    for (int j = 0; j < groupItem->rowCount(); ++j)
    {
      if (groupItem->child(j))
      {
        auto child = groupItem->child(j)->clone();
        auto name = child->data(TaskbarModel::WidgetNameRole).toString();
        auto overview = child->data(TaskbarModel::OverviewRole).toBool();

        if (groupId > -1)
        {
          setWindowState(child->data(TaskbarModel::WindowIdRole).toInt(),
                         TaskbarModel::WindowNormal);
          m_taskbarButtons->appendRow(child);
        }

        else if (overview || groupId == -2)
        {
          child->setData(QStringLiteral("%1 (%2)").arg(name, groupName),
                         TaskbarModel::WidgetNameRole);
          setWindowState(child->data(TaskbarModel::WindowIdRole).toInt(),
                         TaskbarModel::WindowNormal);
          m_taskbarButtons->appendRow(child);
        }
      }
    }
  }

  // Focus the first window
  if (m_taskbarButtons->rowCount() > 0)
  {
    auto firstGroup = m_taskbarButtons->item(0);
    auto windowId = firstGroup->data(TaskbarModel::WindowIdRole).toInt();
    for (auto it = m_windowIDs.begin(); it != m_windowIDs.end(); ++it)
    {
      if (it.value() == windowId)
      {
        setActiveWindow(it.key());
        break;
      }
    }
  }

  // Update user interface
  Q_EMIT activeGroupIdChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT taskbarButtonsChanged();
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
  auto model = groupModel();
  if (model.count() > index && index >= 0)
  {
    auto item = model[index];
    auto id = item.toMap().value("id", -1).toInt();
    setActiveGroupId(id);
  }
}

//------------------------------------------------------------------------------
// Window state management functions
//------------------------------------------------------------------------------

/**
 * @brief Restores a window to its normal (visible) state.
 *
 * Looks up the window's ID and updates its state to WindowNormal.
 *
 * @param window Pointer to the QML widget.
 */
void UI::Taskbar::showWindow(QQuickItem *window)
{
  if (window)
  {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1)
    {
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
void UI::Taskbar::closeWindow(QQuickItem *window)
{
  if (window)
  {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1)
    {
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
void UI::Taskbar::minimizeWindow(QQuickItem *window)
{
  if (window)
  {
    const auto id = m_windowIDs.value(window, -1);
    if (id > -1)
    {
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
void UI::Taskbar::setActiveWindow(QQuickItem *window)
{
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
void UI::Taskbar::unregisterWindow(QQuickItem *window)
{
  if (m_windowIDs.contains(window))
  {
    disconnect(window);
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
void UI::Taskbar::setWindowManager(UI::WindowManager *manager)
{
  if (manager)
  {
    m_windowManager = static_cast<UI::WindowManager *>(manager);
    m_windowManager->setTaskbar(this);
    Q_EMIT windowManagerChanged();
  }
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
void UI::Taskbar::registerWindow(const int id, QQuickItem *window)
{
  // Register the window
  m_windowIDs.insert(window, id);
  m_windowManager->registerWindow(id, window);
  Q_EMIT registeredWindowsChanged();

  // Keep track of window state
  connect(window, &QQuickItem::stateChanged, this,
          [=, this] { Q_EMIT statesChanged(); });

  // Trigger a layout update when the QML code created all the windows
  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager)
    m_windowManager->loadLayout();
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
void UI::Taskbar::setWindowState(const int id,
                                 const UI::TaskbarModel::WindowState state)
{
  // Validate that the item exists
  QStandardItem *item = findItemByWindowId(id);
  if (!item)
    return;

  // Update the window states
  item->setData(state, UI::TaskbarModel::WindowStateRole);
  Q_EMIT windowStatesChanged();

  // Trigger a layout update
  if (m_windowIDs.count() >= m_taskbarButtons->rowCount() && m_windowManager)
    m_windowManager->triggerLayoutUpdate();
}

//------------------------------------------------------------------------------
// General (full) model generation function
//------------------------------------------------------------------------------

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
void UI::Taskbar::rebuildModel()
{
  if (m_rebuildInProgress)
    return;

  m_rebuildInProgress = true;

  {
    QSignalBlocker fullBlocker(m_fullModel);
    QSignalBlocker taskbarBlocker(m_taskbarButtons);

    // Clear the model
    m_windowIDs.clear();
    m_fullModel->clear();
    m_activeWindow = nullptr;

    // Clear widget ID mappings
    m_widgetIdToWindowId.clear();
    m_windowIdToWidgetId.clear();

    // Clear windows in window manager
    if (m_windowManager)
      m_windowManager->clear();
  }

  // Reduce calls to UI::Dashboard::instance()
  auto *db = &UI::Dashboard::instance();
  auto &registry = WidgetRegistry::instance();

  // Obtain and validate latest frame
  const auto &frame = db->processedFrame();
  if (frame.title.isEmpty() || frame.groups.size() <= 0)
  {
    setActiveGroupId(-1);
    Q_EMIT fullModelChanged();
    Q_EMIT windowStatesChanged();
    Q_EMIT registeredWindowsChanged();
    m_rebuildInProgress = false;
    return;
  }

  // Loop through the groups in the dashboard
  QSet<int> groupIds;
  const auto &widgetMap = db->widgetMap();
  for (const DataModel::Group &group : frame.groups)
  {
    // Obtain group parameters
    const auto groupId = group.groupId;
    const auto groupName = group.title;
    const auto groupType = SerialStudio::getDashboardWidget(group);
    const auto groupIcon = SerialStudio::dashboardWidgetIcon(groupType, true);

    // Obtain the window IDs associated to the group
    QList<int> windowIds;
    QList<int> relativeIds;
    QList<SerialStudio::DashboardWidget> widgetTypes;
    for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it)
    {
      const auto windowId = it.key();
      const auto widgetType = it.value().first;
      const auto relativeIndex = it.value().second;

      if (SerialStudio::isGroupWidget(widgetType))
      {
        const auto dbGroup = db->getGroupWidget(widgetType, relativeIndex);
        if (dbGroup.groupId == groupId)
        {
          windowIds.append(windowId);
          widgetTypes.append(widgetType);
          relativeIds.append(relativeIndex);
        }
      }

      else if (SerialStudio::isDatasetWidget(widgetType))
      {
        const auto dbDataset = db->getDatasetWidget(widgetType, relativeIndex);
        if (dbDataset.groupId == groupId)
        {
          windowIds.append(windowId);
          widgetTypes.append(widgetType);
          relativeIds.append(relativeIndex);
        }
      }
    }

    // Obtain main window ID and remove it from the lists
    int mainWindowId = 0;
    for (int i = 0; i < windowIds.count(); ++i)
    {
      if (widgetTypes[i] == groupType)
      {
        mainWindowId = windowIds[i];

        windowIds.removeAt(i);
        widgetTypes.removeAt(i);
        relativeIds.removeAt(i);
        break;
      }
    }

    // Register item to the model
    auto *groupItem = new QStandardItem();
    bool alreadyRegistered = groupIds.contains(groupId);
    groupItem->setData(groupId, TaskbarModel::GroupIdRole);
    groupItem->setData(groupName, TaskbarModel::GroupNameRole);
    groupItem->setData(groupName, TaskbarModel::WidgetNameRole);
    groupItem->setData(groupType, TaskbarModel::WidgetTypeRole);
    groupItem->setData(groupIcon, TaskbarModel::WidgetIconRole);
    groupItem->setData(mainWindowId, TaskbarModel::WindowIdRole);
    groupItem->setData(!alreadyRegistered, TaskbarModel::IsGroupRole);
    groupItem->setData(TaskbarModel::WindowNormal,
                       TaskbarModel::WindowStateRole);
    if (!alreadyRegistered)
      groupItem->setData(true, TaskbarModel::OverviewRole);

    // Map widget ID to window ID for the main group widget
    if (groupType != SerialStudio::DashboardNoWidget)
    {
      auto widgetIds = registry.widgetIdsByType(groupType);
      for (const auto &wid : std::as_const(widgetIds))
      {
        auto info = registry.widgetInfo(wid);
        if (info.groupId == groupId && info.isGroupWidget)
        {
          m_widgetIdToWindowId.insert(wid, mainWindowId);
          m_windowIdToWidgetId.insert(mainWindowId, wid);
          break;
        }
      }
    }

    // Append group children (including any automatically generated groups)
    for (int i = 0; i < windowIds.count(); ++i)
    {
      // Get group icon
      auto icon = SerialStudio::dashboardWidgetIcon(widgetTypes[i], true);

      // Create the child item
      auto *child = new QStandardItem();
      child->setData(false, TaskbarModel::IsGroupRole);
      child->setData(icon, TaskbarModel::WidgetIconRole);
      child->setData(groupId, TaskbarModel::GroupIdRole);
      child->setData(groupName, TaskbarModel::GroupNameRole);
      child->setData(windowIds[i], TaskbarModel::WindowIdRole);
      child->setData(widgetTypes[i], TaskbarModel::WidgetTypeRole);
      child->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);

      // Register window title for sub-groups
      if (SerialStudio::isGroupWidget(widgetTypes[i]))
      {
        auto &dbGroup = db->getGroupWidget(widgetTypes[i], relativeIds[i]);
        child->setData(dbGroup.title, TaskbarModel::WidgetNameRole);
        child->setData(false, TaskbarModel::OverviewRole);

        // Map widget ID to window ID
        auto wid
            = registry.widgetIdByTypeAndIndex(widgetTypes[i], relativeIds[i]);
        if (wid != kInvalidWidgetId)
        {
          m_widgetIdToWindowId.insert(wid, windowIds[i]);
          m_windowIdToWidgetId.insert(windowIds[i], wid);
        }
      }

      // Register window title for datasets
      else if (SerialStudio::isDatasetWidget(widgetTypes[i]))
      {
        auto &dbDataset = db->getDatasetWidget(widgetTypes[i], relativeIds[i]);
        child->setData(dbDataset.title, TaskbarModel::WidgetNameRole);
        child->setData(dbDataset.overviewDisplay, TaskbarModel::OverviewRole);

        // Map widget ID to window ID
        auto wid
            = registry.widgetIdByTypeAndIndex(widgetTypes[i], relativeIds[i]);
        if (wid != kInvalidWidgetId)
        {
          m_widgetIdToWindowId.insert(wid, windowIds[i]);
          m_windowIdToWidgetId.insert(windowIds[i], wid);
        }
      }

      // Register the child
      groupItem->appendRow(child);
    }

    // Append the group to the model
    if (!alreadyRegistered)
    {
      groupIds.insert(groupId);
      m_fullModel->appendRow(groupItem);
    }

    // Append group as subgroup
    else
    {
      for (int i = 0; i < m_fullModel->rowCount(); ++i)
      {
        auto g = m_fullModel->item(i);
        if (g)
        {
          if (g->data(TaskbarModel::GroupIdRole).toInt() == group.groupId)
          {
            g->appendRow(groupItem);
            break;
          }
        }
      }
    }
  }

  // Reset taskbar
  Q_EMIT fullModelChanged();
  Q_EMIT windowStatesChanged();
  Q_EMIT registeredWindowsChanged();

  // Select the first group
  auto model = groupModel();
  if (!model.isEmpty() && model.first().canConvert<QVariantMap>())
  {
    QVariantMap firstItem = model.first().toMap();
    if (firstItem.contains("id"))
    {
      int firstId = firstItem["id"].toInt();
      setActiveGroupId(firstId);
    }
  }

  m_rebuildInProgress = false;
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

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
QStandardItem *UI::Taskbar::findItemByWindowId(int windowId,
                                               QStandardItem *parentItem) const
{
  // Get total number of items & loop over them
  int count = parentItem ? parentItem->rowCount() : fullModel()->rowCount();
  for (int i = 0; i < count; ++i)
  {
    // Check if item is valid
    QStandardItem *item
        = parentItem ? parentItem->child(i) : fullModel()->item(i);
    if (!item)
      continue;

    // Check current item first (regardless of type)
    if (item->data(TaskbarModel::WindowIdRole).toInt() == windowId)
      return item;

    // Recurse if it's a group
    if (item->data(TaskbarModel::IsGroupRole).toBool())
    {
      QStandardItem *found = findItemByWindowId(windowId, item);
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
QStandardItem *UI::Taskbar::findItemByWidgetId(UI::WidgetID widgetId,
                                               QStandardItem *parentItem) const
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
QStandardItem *UI::Taskbar::findGroupItemByGroupId(int groupId) const
{
  for (int i = 0; i < fullModel()->rowCount(); ++i)
  {
    QStandardItem *item = fullModel()->item(i);
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
QStandardItem *UI::Taskbar::createItemFromWidgetInfo(const UI::WidgetInfo &info)
{
  auto *item = new QStandardItem();
  auto icon = SerialStudio::dashboardWidgetIcon(info.type, true);

  item->setData(info.groupId, TaskbarModel::GroupIdRole);
  item->setData(info.title, TaskbarModel::WidgetNameRole);
  item->setData(info.type, TaskbarModel::WidgetTypeRole);
  item->setData(icon, TaskbarModel::WidgetIconRole);
  item->setData(info.isGroupWidget, TaskbarModel::IsGroupRole);
  item->setData(TaskbarModel::WindowNormal, TaskbarModel::WindowStateRole);

  return item;
}

//------------------------------------------------------------------------------
// Registry event handlers
//------------------------------------------------------------------------------

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
void UI::Taskbar::onWidgetCreated(UI::WidgetID id, const UI::WidgetInfo &info)
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
 * @brief Handles dashboard layout changes from the ProjectModel.
 *
 * This is called when a project file is loaded that contains a saved
 * dashboard layout. The layout is restored via the WindowManager.
 */
void UI::Taskbar::onDashboardLayoutChanged()
{
  if (!m_windowManager)
    return;

  const auto opMode = DataModel::FrameBuilder::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile)
  {
    auto *model = &DataModel::ProjectModel::instance();
    if (!model->jsonFilePath().isEmpty())
    {
      const auto &layout = model->dashboardLayout();
      if (!layout.isEmpty())
      {
        QTimer::singleShot(100, this, [this, layout] {
          if (m_windowManager)
            m_windowManager->restoreLayout(layout);
        });
      }
    }
  }
}

/**
 * @brief Saves the current dashboard layout to the ProjectModel.
 *
 * This serializes the current window positions, sizes, and order
 * to the project file. Call this when the user modifies the layout
 * and you want to persist the changes.
 */
void UI::Taskbar::saveLayout()
{
  if (!m_windowManager)
    return;

  const auto opMode = DataModel::FrameBuilder::instance().operationMode();
  if (opMode == SerialStudio::ProjectFile)
  {
    auto *model = &DataModel::ProjectModel::instance();
    if (!model->jsonFilePath().isEmpty())
    {
      auto layout = m_windowManager->serializeLayout();
      model->setDashboardLayout(layout);
      model->setActiveGroupId(m_activeGroupId);
      model->saveJsonFile();
    }
  }
}
