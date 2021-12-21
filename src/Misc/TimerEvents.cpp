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

#include "TimerEvents.h"

#include <QtMath>

/**
 * Converts the given @a hz to milliseconds
 */
static int HZ_TO_MS(const int hz)
{
    const double rHz = hz;
    const double uHz = 1000;
    return qRound(uHz / rHz);
}

/**
 * Constructor function
 */
Misc::TimerEvents::TimerEvents()
{
    // Configure timeout intevals
    m_timerLowFreq.setInterval(HZ_TO_MS(1));
    m_timerHighFreq.setInterval(HZ_TO_MS(20));

    // Configure signals/slots
    connect(&m_timerLowFreq, &QTimer::timeout, this, &Misc::TimerEvents::lowFreqTimeout);
    connect(&m_timerHighFreq, &QTimer::timeout, this,
            &Misc::TimerEvents::highFreqTimeout);
}

/**
 * Returns a pointer to the only instance of the class
 */
Misc::TimerEvents &Misc::TimerEvents::instance()
{
    static auto singleton = new TimerEvents();
    return *singleton;
}

/**
 * Returns the target UI refresh frequency
 */
int Misc::TimerEvents::highFreqTimeoutHz() const
{
    return HZ_TO_MS(m_timerHighFreq.interval());
}

/**
 * Stops all the timers of this module
 */
void Misc::TimerEvents::stopTimers()
{
    m_timerLowFreq.stop();
    m_timerHighFreq.stop();
}

/**
 * Starts all the timer of the module
 */
void Misc::TimerEvents::startTimers()
{
    m_timerLowFreq.start();
    m_timerHighFreq.start();
}

/**
 * Updates the target UI refresh frequency
 */
void Misc::TimerEvents::setHighFreqTimeout(const int hz)
{
    if (hz > 0)
    {
        m_timerHighFreq.setInterval(HZ_TO_MS(hz));
        Q_EMIT highFreqTimeoutChanged();
    }

    else
    {
        m_timerHighFreq.setInterval(HZ_TO_MS(1));
        Q_EMIT highFreqTimeoutChanged();
    }
}
