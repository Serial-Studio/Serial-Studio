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

#include "UI/Dashboard/DashboardTools.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QVariantMap>

#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decrements a RepeatNTimes counter and stops the timer when it hits zero.
 */
static void tickRepeatTimer(int index, QMap<int, QTimer*>& timers, QMap<int, int>& counters)
{
  const auto it = counters.find(index);
  if (it == counters.end())
    return;

  if (--it.value() > 0)
    return;

  const auto timerIt = timers.find(index);
  if (timerIt != timers.end() && timerIt.value())
    timerIt.value()->stop();

  counters.erase(it);
}

/**
 * @brief Applies a non-RepeatNTimes timer mode to an action's QTimer.
 */
static void applyTimerMode(QTimer* timer,
                           DataModel::TimerMode mode,
                           bool guiTrigger,
                           const QString& actionTitle)
{
  if (!timer) {
    qWarning() << "Invalid timer pointer for action" << actionTitle;
    return;
  }

  if (mode == DataModel::TimerMode::StartOnTrigger && !timer->isActive())
    timer->start();

  else if (mode == DataModel::TimerMode::ToggleOnTrigger && guiTrigger) {
    if (timer->isActive())
      timer->stop();
    else
      timer->start();
  }
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the tools with every window off; @p settings is the facade's store, so both
 *        write the same keys, and @p ioManager is the transmit path every action uses.
 */
UI::DashboardTools::DashboardTools(QSettings& settings,
                                   IO::ConnectionManager& ioManager,
                                   QObject* parent)
  : QObject(parent)
  , m_settings(settings)
  , m_ioManager(ioManager)
  , m_persistSettings(true)
  , m_clockEnabled(false)
  , m_stopwatchEnabled(false)
  , m_terminalEnabled(false)
  , m_notificationLogEnabled(false)
{}

//--------------------------------------------------------------------------------------------------
// Dashboard tools (terminal, notification log, clock, stopwatch)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if a terminal widget should be displayed within the dashboard.
 */
bool UI::DashboardTools::terminalEnabled() const noexcept
{
  return m_terminalEnabled;
}

/**
 * @brief Returns true if the notification log widget should be displayed within the dashboard.
 */
bool UI::DashboardTools::notificationLogEnabled() const noexcept
{
  return m_notificationLogEnabled;
}

/**
 * @brief Returns true if the clock widget should be displayed within the dashboard.
 */
bool UI::DashboardTools::clockEnabled() const noexcept
{
  return m_clockEnabled;
}

/**
 * @brief Returns true if the stopwatch widget should be displayed within the dashboard.
 */
bool UI::DashboardTools::stopwatchEnabled() const noexcept
{
  return m_stopwatchEnabled;
}

/**
 * @brief Shows or hides the terminal tool window. The widget itself is always registered;
 *        the flag only drives external-window visibility, so no rebuild occurs.
 */
void UI::DashboardTools::setTerminalEnabled(const bool enabled)
{
  if (m_terminalEnabled == enabled)
    return;

  m_terminalEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/TerminalEnabled", m_terminalEnabled);

  Q_EMIT terminalEnabledChanged();
}

/**
 * @brief Shows or hides the notification log tool window (Pro-only widget).
 */
void UI::DashboardTools::setNotificationLogEnabled(const bool enabled)
{
  if (m_notificationLogEnabled == enabled)
    return;

  m_notificationLogEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/NotificationLogEnabled", m_notificationLogEnabled);

  Q_EMIT notificationLogEnabledChanged();
}

/**
 * @brief Shows or hides the clock tool window.
 */
void UI::DashboardTools::setClockEnabled(const bool enabled)
{
  if (m_clockEnabled == enabled)
    return;

  m_clockEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ClockEnabled", m_clockEnabled);

  Q_EMIT clockEnabledChanged();
}

/**
 * @brief Shows or hides the stopwatch tool window.
 */
void UI::DashboardTools::setStopwatchEnabled(const bool enabled)
{
  if (m_stopwatchEnabled == enabled)
    return;

  m_stopwatchEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/StopwatchEnabled", m_stopwatchEnabled);

  Q_EMIT stopwatchEnabledChanged();
}

/**
 * @brief Restores the persisted tool-window flags from QSettings.
 */
void UI::DashboardTools::restorePersistedSettings()
{
  m_terminalEnabled        = m_settings.value("Dashboard/TerminalEnabled", false).toBool();
  m_notificationLogEnabled = m_settings.value("Dashboard/NotificationLogEnabled", false).toBool();
  m_clockEnabled           = m_settings.value("Dashboard/ClockEnabled", false).toBool();
  m_stopwatchEnabled       = m_settings.value("Dashboard/StopwatchEnabled", false).toBool();
}

/**
 * @brief Toggles whether tool-window flags are written to QSettings.
 */
void UI::DashboardTools::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

//--------------------------------------------------------------------------------------------------
// Action access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the count of actions available within the dashboard.
 */
int UI::DashboardTools::actionCount() const
{
  return m_actions.count();
}

/**
 * @brief Returns a list of available dashboard actions with their metadata.
 */
QVariantList UI::DashboardTools::actions() const
{
  QVariantList actions;
  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];

    QVariantMap m;
    m["id"]      = i;
    m["checked"] = false;
    m["text"]    = action.title;
    m["icon"]    = Misc::IconEngine::resolveActionIconSource(action.icon);
    if (action.timerMode == DataModel::TimerMode::ToggleOnTrigger) {
      if (m_timers.contains(i) && m_timers[i] && m_timers[i]->isActive())
        m["checked"] = true;
    }

    actions.append(m);
  }

  return actions;
}

/**
 * @brief Returns the runtime index of the action with the given public @p actionId, or -1.
 */
int UI::DashboardTools::actionIndexForId(int actionId) const noexcept
{
  for (int i = 0; i < m_actions.count(); ++i)
    if (m_actions.at(i).actionId == actionId)
      return i;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Action handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and handling timer
 *        logic. actionStatusChanged makes QML rebuild the whole actions list, so it only fires
 *        when a timer's activity flips; per-tick transmissions emit nothing.
 */
void UI::DashboardTools::activateAction(const int index, const bool guiTrigger)
{
  if (index < 0 || index >= m_actions.count()) {
    qWarning() << "Invalid action index:" << index;
    return;
  }

  const auto& action = m_actions[index];

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && guiTrigger) {
    if (m_timers.contains(index) && m_timers[index]) {
      m_repeatCounters[index] = qMax(1, action.repeatCount);
      m_timers[index]->start();
    }

    if (!m_ioManager.paused())
      (void)m_ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);
    return;
  }

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && !guiTrigger) {
    if (!m_ioManager.paused())
      (void)m_ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);
    return;
  }

  bool timerFlipped  = false;
  const auto timerIt = m_timers.find(index);
  if (timerIt != m_timers.end()) {
    const bool wasActive = timerIt.value() && timerIt.value()->isActive();
    applyTimerMode(timerIt.value(), action.timerMode, guiTrigger, action.title);
    const bool isActive = timerIt.value() && timerIt.value()->isActive();
    timerFlipped        = (wasActive != isActive);
  }

  if (!m_ioManager.paused())
    (void)m_ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

  if (timerFlipped)
    Q_EMIT actionStatusChanged();
}

//--------------------------------------------------------------------------------------------------
// Action configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures dashboard actions and associated timers from the given DataModel frame.
 */
void UI::DashboardTools::configureActions(const DataModel::Frame& frame)
{
  if (frame.groups.size() <= 0)
    return;

  m_actions.clear();
  m_actions.squeeze();

  for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
    if (it.value()) {
      disconnect(it.value());
      it.value()->stop();
      delete it.value();
    }
  }

  m_timers.clear();
  m_repeatCounters.clear();

  for (const auto& action : frame.actions)
    m_actions.append(action);

  if (!m_ioManager.isConnected()) {
    Q_EMIT actionStatusChanged();
    return;
  }

  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];
    if (action.timerMode == DataModel::TimerMode::Off)
      continue;

    const auto interval = action.timerIntervalMs;
    if (interval <= 0) {
      qWarning() << "Interval for action" << action.title << "must be greater than 0!";
      continue;
    }

    auto* timer = new QTimer(this);
    timer->setInterval(interval);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, [this, i]() { activateAction(i, false); });

    const bool isRepeat = action.timerMode == DataModel::TimerMode::RepeatNTimes;
    if (isRepeat && action.autoExecuteOnConnect) {
      m_repeatCounters[i] = qMax(1, action.repeatCount);
      timer->start();
    }

    else if (!isRepeat
             && (action.timerMode == DataModel::TimerMode::AutoStart
                 || action.autoExecuteOnConnect))
      timer->start();

    m_timers.insert(i, timer);
  }

  Q_EMIT actionStatusChanged();
}
