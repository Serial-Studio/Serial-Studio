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

#include <QTimerEvent>
#include <Misc/TimerEvents.h>

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
}

/**
 * Starts all the timer of the module
 */
void Misc::TimerEvents::startTimers()
{
    m_timer20Hz.start(50, this);
    m_timer10Hz.start(100, this);
    m_timer1Hz.start(1000, this);
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_TimerEvents.cpp"
#endif
