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
#include <QMetaObject>
#include <QObject>
#include <QSet>

namespace UI {
class Dashboard;
class Taskbar;
class UISessionRegistry;

/**
 * @brief The datasets a widget on screen shows, folded from every dashboard view's taskbar (view
 *        shown, active workspace, window states, pop-outs) through the dashboard's widget map;
 *        AlarmMonitor gates the datasets that opted into Alarms only while visible on it
 *        (spec 0105). GUI thread, event rate: one deferred recompute per burst of changes.
 */
class WidgetVisibility : public QObject {
  Q_OBJECT

signals:
  void shownChanged();

public:
  explicit WidgetVisibility(Dashboard& dashboard,
                            UISessionRegistry& registry,
                            QObject* parent = nullptr);
  WidgetVisibility(WidgetVisibility&&)                 = delete;
  WidgetVisibility(const WidgetVisibility&)            = delete;
  WidgetVisibility& operator=(WidgetVisibility&&)      = delete;
  WidgetVisibility& operator=(const WidgetVisibility&) = delete;

  [[nodiscard]] bool isShown(int uniqueId) const;

private slots:
  void onTaskbarsChanged();
  void scheduleRecompute();
  void recompute();

private:
  void addWindow(int windowId, QSet<int>& shown) const;

  Dashboard& m_dashboard;
  UISessionRegistry& m_registry;
  bool m_pending;
  QSet<int> m_shown;
  QList<QMetaObject::Connection> m_taskbarConnections;
};
}  // namespace UI
