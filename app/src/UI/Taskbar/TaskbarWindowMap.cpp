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

#include "UI/Taskbar/TaskbarWindowMap.h"

#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty map bound to the dashboard that owns the widget map and to the registry
 *        that mints widget ids.
 */
UI::TaskbarWindowMap::TaskbarWindowMap(UI::Dashboard& dashboard, UI::WidgetRegistry& registry)
  : m_dashboard(dashboard), m_registry(registry)
{}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the window id a widget renders into, or -1 when the widget has no row.
 */
int UI::TaskbarWindowMap::windowIdForWidget(UI::WidgetID widgetId) const
{
  return m_widgetIdToWindowId.value(widgetId, -1);
}

/**
 * @brief Returns the dashboard relative-index for windowId, or -1 if not found.
 */
int UI::TaskbarWindowMap::relativeIndexForWindow(int windowId) const
{
  const auto& widgetMap = m_dashboard.widgetMap();
  const auto it         = widgetMap.constFind(windowId);
  if (it == widgetMap.cend())
    return -1;

  return it.value().second;
}

/**
 * @brief Returns the windowId for the (widgetType, relativeIndex) pair, or -1 if not found.
 */
int UI::TaskbarWindowMap::findWindowIdByGroupAndIndex(int widgetType, int relativeIndex) const
{
  const auto& widgetMap = m_dashboard.widgetMap();
  for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
    if (static_cast<int>(it.value().first) != widgetType || it.value().second != relativeIndex)
      continue;

    return it.key();
  }

  return -1;
}

/**
 * @brief Collects the (windowId, widgetType, relativeIndex) triples that belong to groupId.
 */
void UI::TaskbarWindowMap::collectGroupWidgetIds(
  int groupId,
  QList<int>& windowIds,
  QList<int>& relativeIds,
  QList<SerialStudio::DashboardWidget>& widgetTypes) const
{
  const auto& widgetMap = m_dashboard.widgetMap();
  for (auto it = widgetMap.begin(); it != widgetMap.end(); ++it) {
    const auto windowId      = it.key();
    const auto widgetType    = it.value().first;
    const auto relativeIndex = it.value().second;

    const auto slot = m_dashboard.widgetSlot(widgetType, relativeIndex);
    if (!slot.valid)
      continue;

    const int candidateGroup = slot.group
                               ? m_dashboard.getGroupWidget(widgetType, slot.bucketIndex).groupId
                               : m_dashboard.getDatasetWidget(widgetType, slot.bucketIndex).groupId;

    if (candidateGroup != groupId)
      continue;

    windowIds.append(windowId);
    widgetTypes.append(widgetType);
    relativeIds.append(relativeIndex);
  }
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every mapping; the taskbar re-registers them on its next model rebuild.
 */
void UI::TaskbarWindowMap::clear()
{
  m_widgetIdToWindowId.clear();
  m_windowIdToWidgetId.clear();
}

/**
 * @brief Records the bidirectional mapping between a widget ID and a window ID.
 */
void UI::TaskbarWindowMap::map(UI::WidgetID widgetId, int windowId)
{
  if (widgetId == kInvalidWidgetId)
    return;

  m_widgetIdToWindowId.insert(widgetId, windowId);
  m_windowIdToWidgetId.insert(windowId, widgetId);
}

/**
 * @brief Maps the WidgetID of the main group widget (if any) to its windowId.
 */
void UI::TaskbarWindowMap::mapMainGroupWidget(SerialStudio::DashboardWidget groupType,
                                              int groupId,
                                              int mainWindowId)
{
  if (groupType == SerialStudio::DashboardNoWidget || mainWindowId < 0)
    return;

  const auto widgetIds = m_registry.widgetIdsByType(groupType);
  for (const auto& wid : std::as_const(widgetIds)) {
    const auto info = m_registry.widgetInfo(wid);
    if (info.groupId != groupId || !info.isGroupWidget)
      continue;

    m_widgetIdToWindowId.insert(wid, mainWindowId);
    m_windowIdToWidgetId.insert(mainWindowId, wid);
    return;
  }
}
