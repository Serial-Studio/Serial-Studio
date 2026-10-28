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

#include "UI/WidgetVisibility.h"

#include <utility>

#include "Core/SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the registry, the dashboard rebuild signals and every live taskbar; the first
 *        recompute is deferred like every later one, so construction never walks a half-built
 *        dashboard.
 */
UI::WidgetVisibility::WidgetVisibility(Dashboard& dashboard,
                                       UISessionRegistry& registry,
                                       QObject* parent)
  : QObject(parent), m_dashboard(dashboard), m_registry(registry), m_pending(false)
{
  connect(
    &m_registry, &UISessionRegistry::taskbarsChanged, this, &WidgetVisibility::onTaskbarsChanged);
  connect(&m_dashboard, &Dashboard::widgetCountChanged, this, &WidgetVisibility::scheduleRecompute);
  connect(&m_dashboard, &Dashboard::dataReset, this, &WidgetVisibility::scheduleRecompute);

  onTaskbarsChanged();
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a widget on screen shows the dataset with @p uniqueId.
 */
bool UI::WidgetVisibility::isShown(const int uniqueId) const
{
  return m_shown.contains(uniqueId);
}

//--------------------------------------------------------------------------------------------------
// Recompute
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-wires the per-taskbar connections after a view opened or closed; the connections are
 *        captured so a retired taskbar's are dropped by handle, never by a null-slot disconnect.
 */
void UI::WidgetVisibility::onTaskbarsChanged()
{
  for (const auto& connection : std::as_const(m_taskbarConnections))
    disconnect(connection);

  m_taskbarConnections.clear();
  const auto& taskbars = m_registry.taskbars();
  for (auto* taskbar : taskbars) {
    SS_ASSERT_LOG(taskbar != nullptr);
    if (!taskbar)
      continue;

    m_taskbarConnections.append(
      connect(taskbar, &Taskbar::viewShownChanged, this, &WidgetVisibility::scheduleRecompute));
    m_taskbarConnections.append(
      connect(taskbar, &Taskbar::activeGroupIdChanged, this, &WidgetVisibility::scheduleRecompute));
    m_taskbarConnections.append(
      connect(taskbar, &Taskbar::windowStatesChanged, this, &WidgetVisibility::scheduleRecompute));
    m_taskbarConnections.append(
      connect(taskbar, &Taskbar::popOutsChanged, this, &WidgetVisibility::scheduleRecompute));
  }

  scheduleRecompute();
}

/**
 * @brief Coalesces a burst of input changes (a workspace switch fires three taskbar signals) into
 *        one recompute on the next event-loop turn.
 */
void UI::WidgetVisibility::scheduleRecompute()
{
  if (m_pending)
    return;

  m_pending = true;
  QMetaObject::invokeMethod(this, &WidgetVisibility::recompute, Qt::QueuedConnection);
}

/**
 * @brief Folds the shown set: every pop-out of every taskbar, plus the normal-state windows of
 *        the active workspace of each taskbar whose view is on screen; emits only on a change.
 */
void UI::WidgetVisibility::recompute()
{
  m_pending = false;

  QSet<int> shown;
  const auto& taskbars = m_registry.taskbars();
  for (const auto* taskbar : taskbars) {
    SS_ASSERT_LOG(taskbar != nullptr);
    if (!taskbar)
      continue;

    for (const int windowId : taskbar->popOuts())
      addWindow(windowId, shown);

    if (!taskbar->viewShown())
      continue;

    const auto windowIds = taskbar->shownWindowIds();
    for (const int windowId : windowIds)
      addWindow(windowId, shown);
  }

  if (shown == m_shown)
    return;

  m_shown = std::move(shown);
  Q_EMIT shownChanged();
}

/**
 * @brief Adds the datasets one window shows: the dataset of a dataset-scope widget, every member
 *        of a group-scope widget. A window id the widget map no longer knows (a taskbar row that
 *        outlived a rebuild by one tick) adds nothing.
 */
void UI::WidgetVisibility::addWindow(const int windowId, QSet<int>& shown) const
{
  SS_ASSERT(windowId >= 0, return);

  const auto type = m_dashboard.widgetType(windowId);
  if (type == SerialStudio::DashboardNoWidget)
    return;

  const auto slot = m_dashboard.widgetSlot(type, m_dashboard.relativeIndex(windowId));
  if (!slot.valid)
    return;

  SS_ASSERT(slot.bucketIndex >= 0, return);
  if (!slot.group) {
    shown.insert(m_dashboard.getDatasetWidget(type, slot.bucketIndex).uniqueId);
    return;
  }

  const auto& group = m_dashboard.getGroupWidget(type, slot.bucketIndex);
  for (const auto& dataset : group.datasets)
    shown.insert(dataset.uniqueId);
}
