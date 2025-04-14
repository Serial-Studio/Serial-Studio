/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
