/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
    // clang-format off
    Q_OBJECT
    // clang-format on

Q_SIGNALS:
    void timeout1Hz();
    void timeout10Hz();
    void timeout20Hz();

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

public Q_SLOTS:
    void stopTimers();
    void startTimers();

private:
    QBasicTimer m_timer1Hz;
    QBasicTimer m_timer10Hz;
    QBasicTimer m_timer20Hz;
};
}
