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

#include <QTimerEvent>
#include "Misc/TimerEvents.h"

/**
 * @brief Constructs the TimerEvents singleton instance.
 *
 * Initializes the UI timer frequency from application settings.
 * The value is read from the key "uiRefreshRate" with a default of 60 Hz.
 * The frequency is clamped between 1 and 240 Hz.
 */
Misc::TimerEvents::TimerEvents()
  : m_uiTimerHz(60)
{
  m_uiTimerHz = m_settings.value("uiRefreshRate", 60).toInt();
  m_uiTimerHz = qBound(1, m_uiTimerHz, 240);
}

/**
 * @brief Returns a reference to the singleton instance.
 * @return Reference to the only instance of TimerEvents.
 */
Misc::TimerEvents &Misc::TimerEvents::instance()
{
  static TimerEvents singleton;
  return singleton;
}

/**
 * @brief Gets the current UI timer frequency.
 * @return The UI timer frequency in Hz.
 */
int Misc::TimerEvents::fps() const
{
  return m_uiTimerHz;
}

/**
 * @brief Stops all timers managed by this class.
 *
 * This includes:
 * - UI timer
 * - 1 Hz timer
 * - 10 Hz timer
 * - 20 Hz timer
 */
void Misc::TimerEvents::stopTimers()
{
  m_uiTimer.stop();
  m_timer1Hz.stop();
  m_timer10Hz.stop();
  m_timer20Hz.stop();
}

/**
 * @brief Handles QBasicTimer expiration events.
 *
 * Identifies which timer triggered the event and emits the corresponding
 * timeout signal.
 *
 * @param event Pointer to the timer event.
 */
void Misc::TimerEvents::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_timer1Hz.timerId())
    Q_EMIT timeout1Hz();

  else if (event->timerId() == m_timer10Hz.timerId())
    Q_EMIT timeout10Hz();

  else if (event->timerId() == m_timer20Hz.timerId())
    Q_EMIT timeout20Hz();

  else if (event->timerId() == m_uiTimer.timerId())
    Q_EMIT uiTimeout();
}

/**
 * @brief Starts all timers managed by this class.
 *
 * The UI timer is started with the currently configured frequency.
 * Other timers are started at fixed rates:
 * - 1 Hz
 * - 10 Hz
 * - 20 Hz
 */
void Misc::TimerEvents::startTimers()
{
  m_uiTimer.start(1000 / m_uiTimerHz, Qt::PreciseTimer, this);

  m_timer1Hz.start(1000, Qt::PreciseTimer, this);
  m_timer20Hz.start(1000 / 20, Qt::PreciseTimer, this);
  m_timer10Hz.start(1000 / 10, Qt::PreciseTimer, this);
}

/**
 * @brief Sets the UI timer frequency in Hz.
 *
 * The value is clamped between 1 and 240 Hz and stored in the settings
 * under the key "uiTimerHz". If the timer is active, it will be restarted
 * immediately with the new frequency.
 *
 * @param hz New UI timer frequency in Hz.
 */
void Misc::TimerEvents::setFPS(int hz)
{
  hz = qBound(1, hz, 240);

  if (m_uiTimerHz != hz)
  {
    m_uiTimerHz = hz;
    m_settings.setValue("uiTimerHz", hz);

    if (m_uiTimer.isActive())
    {
      m_uiTimer.stop();
      m_uiTimer.start(1000 / m_uiTimerHz, Qt::PreciseTimer, this);
    }

    Q_EMIT fpsChanged();
  }
}
