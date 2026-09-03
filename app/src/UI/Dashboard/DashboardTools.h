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

#include <QMap>
#include <QObject>
#include <QVariantList>
#include <QVector>

#include "DataModel/Frame.h"

class QTimer;
class QSettings;

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace UI {

/**
 * @brief Owns the dashboard's user-rate tools: the four external tool windows (terminal,
 *        notification log, clock, stopwatch) and the project's transmit actions with their timers.
 *        The tool flags are pure view state; toggling a tool must never emit widgetCountChanged or
 *        touch the widget map, because the widget itself is registered unconditionally.
 */
class DashboardTools : public QObject {
  Q_OBJECT

signals:
  void actionStatusChanged();
  void clockEnabledChanged();
  void stopwatchEnabledChanged();
  void terminalEnabledChanged();
  void notificationLogEnabledChanged();

public:
  DashboardTools(QSettings& settings,
                 IO::ConnectionManager& ioManager,
                 DataModel::ProjectModel& projectModel,
                 QObject* parent = nullptr);
  DashboardTools(DashboardTools&&)                 = delete;
  DashboardTools(const DashboardTools&)            = delete;
  DashboardTools& operator=(DashboardTools&&)      = delete;
  DashboardTools& operator=(const DashboardTools&) = delete;
  ~DashboardTools() override                       = default;

  [[nodiscard]] bool clockEnabled() const noexcept;
  [[nodiscard]] bool stopwatchEnabled() const noexcept;
  [[nodiscard]] bool terminalEnabled() const noexcept;
  [[nodiscard]] bool notificationLogEnabled() const noexcept;

  [[nodiscard]] int actionCount() const;
  [[nodiscard]] QVariantList actions() const;
  [[nodiscard]] int actionIndexForId(int actionId) const noexcept;

  void restorePersistedSettings();
  void setSettingsPersistent(const bool persistent);
  void configureActions(const DataModel::Frame& frame);
  void refreshActionsFromProject();
  void activateAction(const int index, const bool guiTrigger);

  void setClockEnabled(const bool enabled);
  void setStopwatchEnabled(const bool enabled);
  void setTerminalEnabled(const bool enabled);
  void setNotificationLogEnabled(const bool enabled);

private:
  void rebuildActions(const QVector<DataModel::Action>& actions);

private:
  QSettings& m_settings;
  IO::ConnectionManager& m_ioManager;
  DataModel::ProjectModel& m_projectModel;

  bool m_persistSettings;
  bool m_clockEnabled;
  bool m_stopwatchEnabled;
  bool m_terminalEnabled;
  bool m_notificationLogEnabled;

  QMap<int, QTimer*> m_timers;
  QMap<int, int> m_repeatCounters;
  QVector<DataModel::Action> m_actions;
};

}  // namespace UI
