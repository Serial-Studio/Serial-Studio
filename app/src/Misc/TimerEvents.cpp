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

#include "Misc/TimerEvents.h"

#include <QTimerEvent>

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the TimerEvents singleton instance.
 */
Misc::TimerEvents::TimerEvents() : m_uiTimerHz(60)
{
  m_uiTimerHz = m_settings.value("uiRefreshRate", 60).toInt();
  m_uiTimerHz = qBound(1, m_uiTimerHz, 240);
}

/**
 * @brief Returns a reference to the singleton instance.
 */
Misc::TimerEvents& Misc::TimerEvents::instance()
{
  static TimerEvents singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Frequency control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current UI timer frequency.
 */
int Misc::TimerEvents::fps() const noexcept
{
  return m_uiTimerHz;
}

//--------------------------------------------------------------------------------------------------
// Timer management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stops all timers managed by this class.
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
 */
void Misc::TimerEvents::timerEvent(QTimerEvent* event)
{
  const int id = event->timerId();

  if (id == m_timer1Hz.timerId()) {
    Q_EMIT timeout1Hz();
    return;
  }

  if (id == m_timer10Hz.timerId()) {
    Q_EMIT timeout10Hz();
    return;
  }

  if (id == m_timer20Hz.timerId()) {
    Q_EMIT timeout20Hz();
    return;
  }

  if (id == m_uiTimer.timerId())
    Q_EMIT uiTimeout();
}

/**
 * @brief Starts all timers managed by this class.
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
 */
void Misc::TimerEvents::setFPS(int hz)
{
  hz = qBound(1, hz, 240);

  if (m_uiTimerHz != hz) {
    m_uiTimerHz = hz;
    m_settings.setValue("uiRefreshRate", hz);

    if (m_uiTimer.isActive()) {
      m_uiTimer.stop();
      m_uiTimer.start(1000 / m_uiTimerHz, Qt::PreciseTimer, this);
    }

    Q_EMIT fpsChanged();
  }
}
