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

#include <optional>
#include <QObject>
#include <QPair>
#include <QString>
#include <QVariantList>

class AppState;

namespace Misc {
class IconRegistry;
}  // namespace Misc

namespace DataModel {
class ProjectModel;
struct WidgetRef;
}  // namespace DataModel

namespace UI {
class Taskbar;
class Dashboard;

/**
 * @brief Owns the taskbar's workspace concern: the switcher models projected out of the project's
 *        workspace list, widget-reference resolution, and the workspace mutations the dashboard UI
 *        drives. Holds a deliberate back-reference to its facade (the exception to the 0070
 *        injection rule) because the mutations are taskbar orchestration, not pure projections.
 */
class TaskbarWorkspaces : public QObject {
  Q_OBJECT

signals:
  void workspacesChanged();
  void taskbarRowsChanged();
  void highlightRequested(int windowId);

public:
  TaskbarWorkspaces(Taskbar& taskbar,
                    UI::Dashboard& dashboard,
                    DataModel::ProjectModel& projectModel,
                    AppState& appState,
                    Misc::IconRegistry& iconRegistry,
                    QObject* parent = nullptr);
  TaskbarWorkspaces(TaskbarWorkspaces&&)                 = delete;
  TaskbarWorkspaces(const TaskbarWorkspaces&)            = delete;
  TaskbarWorkspaces& operator=(TaskbarWorkspaces&&)      = delete;
  TaskbarWorkspaces& operator=(const TaskbarWorkspaces&) = delete;

  [[nodiscard]] QVariantList tree() const;
  [[nodiscard]] QVariantList model() const;
  [[nodiscard]] bool contains(int workspaceId) const;
  [[nodiscard]] int indexForGroupId(int groupId) const;
  [[nodiscard]] QVariantList widgetIds(int workspaceId) const;
  [[nodiscard]] int workspaceContainingWidget(int windowId) const;
  [[nodiscard]] QPair<int, int> changeIndices(int fromGroupId, int toGroupId) const;
  [[nodiscard]] int resolveRefWindowId(const DataModel::WidgetRef& ref) const;
  [[nodiscard]] std::optional<int> selectionAfterRebuild(bool independent,
                                                         int desiredGroupId,
                                                         int activeGroupId) const;

  void populateTaskbar(int groupId);

public slots:
  void deleteWorkspace(int workspaceId);
  void createWorkspace(const QString& name);
  void addWidgetToActiveWorkspace(int windowId);
  void removeWidgetFromActiveWorkspace(int windowId);
  void renameWorkspace(int workspaceId, const QString& name);
  void setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds);
  void navigateToWidget(int windowId, int groupId, bool allowAddToWorkspace);

private:
  void revealWindow(int windowId);
  void removeTaskbarRow(int windowId);
  [[nodiscard]] bool clearWorkspaceWidgets(int workspaceId);
  [[nodiscard]] int indexOfWidgetRef(int workspaceId,
                                     int widgetType,
                                     int groupUniqueId,
                                     int relativeIndex) const;

  Taskbar& m_taskbar;
  UI::Dashboard& m_dashboard;
  DataModel::ProjectModel& m_projectModel;
  AppState& m_appState;
  Misc::IconRegistry& m_iconRegistry;
};

}  // namespace UI
