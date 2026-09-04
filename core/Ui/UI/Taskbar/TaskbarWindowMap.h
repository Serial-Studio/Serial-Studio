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

#pragma once

#include <QList>
#include <QMap>

#include "SerialStudio.h"
#include "UI/WidgetRegistry.h"

namespace UI {
class Dashboard;

/**
 * @brief Owns the bidirectional widget-id <-> window-id mapping the taskbar rows are keyed by,
 *        plus the dashboard lookups that resolve one identity into the other. A window id is the
 *        taskbar's own row identity and a WidgetID belongs to the widget registry; the two are
 *        minted independently, so every crossing between them goes through this class.
 */
class TaskbarWindowMap {
public:
  TaskbarWindowMap(UI::Dashboard& dashboard, UI::WidgetRegistry& registry);
  TaskbarWindowMap(TaskbarWindowMap&&)                 = delete;
  TaskbarWindowMap(const TaskbarWindowMap&)            = delete;
  TaskbarWindowMap& operator=(TaskbarWindowMap&&)      = delete;
  TaskbarWindowMap& operator=(const TaskbarWindowMap&) = delete;

  [[nodiscard]] int windowIdForWidget(UI::WidgetID widgetId) const;
  [[nodiscard]] int relativeIndexForWindow(int windowId) const;
  [[nodiscard]] int findWindowIdByGroupAndIndex(int widgetType, int relativeIndex) const;

  void clear();
  void map(UI::WidgetID widgetId, int windowId);
  void mapMainGroupWidget(SerialStudio::DashboardWidget groupType, int groupId, int mainWindowId);
  void collectGroupWidgetIds(int groupId,
                             QList<int>& windowIds,
                             QList<int>& relativeIds,
                             QList<SerialStudio::DashboardWidget>& widgetTypes) const;

private:
  UI::Dashboard& m_dashboard;
  UI::WidgetRegistry& m_registry;

  QMap<UI::WidgetID, int> m_widgetIdToWindowId;
  QMap<int, UI::WidgetID> m_windowIdToWidgetId;
};

}  // namespace UI
