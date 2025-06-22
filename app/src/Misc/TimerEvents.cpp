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
 * Returns a pointer to the only instance of the class
 */
Misc::TimerEvents &Misc::TimerEvents::instance()
{
  static TimerEvents singleton;
  return singleton;
}

/**
 * Stops all the timers of this module
 */
void Misc::TimerEvents::stopTimers()
{
  m_timer1Hz.stop();
  m_timer10Hz.stop();
  m_timer20Hz.stop();
  m_timer24Hz.stop();
}

/**
 * Emits the @c timeout signal when the basic timer expires
 */
void Misc::TimerEvents::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_timer1Hz.timerId())
    Q_EMIT timeout1Hz();

  else if (event->timerId() == m_timer10Hz.timerId())
    Q_EMIT timeout10Hz();

  else if (event->timerId() == m_timer20Hz.timerId())
    Q_EMIT timeout20Hz();

  else if (event->timerId() == m_timer24Hz.timerId())
    Q_EMIT timeout24Hz();
}

/**
 * Starts all the timer of the module
 */
void Misc::TimerEvents::startTimers()
{
  m_timer1Hz.start(1000, Qt::PreciseTimer, this);
  m_timer20Hz.start(1000 / 20, Qt::PreciseTimer, this);
  m_timer24Hz.start(1000 / 24, Qt::PreciseTimer, this);
  m_timer10Hz.start(1000 / 10, Qt::PreciseTimer, this);
}
