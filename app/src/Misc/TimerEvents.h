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

#pragma once

#include <QObject>
#include <QBasicTimer>

namespace Misc
{
/**
 * @brief The TimerEvents class
 *
 * The @c TimerEvents class implements periodic timers that are used to update
 * the user interface elements at a specific frequency.
 */
class TimerEvents : public QObject
{
  Q_OBJECT

signals:
  void timeout1Hz();
  void timeout10Hz();
  void timeout20Hz();
  void timeout24Hz();

private:
  TimerEvents() {};
  TimerEvents(TimerEvents &&) = delete;
  TimerEvents(const TimerEvents &) = delete;
  TimerEvents &operator=(TimerEvents &&) = delete;
  TimerEvents &operator=(const TimerEvents &) = delete;

public:
  static TimerEvents &instance();

protected:
  void timerEvent(QTimerEvent *event) override;

public slots:
  void stopTimers();
  void startTimers();

private:
  QBasicTimer m_timer1Hz;
  QBasicTimer m_timer10Hz;
  QBasicTimer m_timer20Hz;
  QBasicTimer m_timer24Hz;
};
} // namespace Misc
