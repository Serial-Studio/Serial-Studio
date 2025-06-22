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
