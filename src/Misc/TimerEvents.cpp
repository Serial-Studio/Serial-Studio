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
#include <Logger.h>
#include <ConsoleAppender.h>

using namespace Misc;
#define HZ_TO_MS(x) qCeil(1000 / x)

/**
 * Pointer to the only instance of the class
 */
static TimerEvents *INSTANCE = nullptr;

/**
 * Constructor function
 */
TimerEvents::TimerEvents()
{
    // Configure timeout intevals
    m_timer1Hz.setInterval(HZ_TO_MS(1));
    m_timer42Hz.setInterval(HZ_TO_MS(42));

    // Configure timer precision
    m_timer1Hz.setTimerType(Qt::PreciseTimer);
    m_timer42Hz.setTimerType(Qt::PreciseTimer);

    // Configure signals/slots
    connect(&m_timer1Hz, &QTimer::timeout, this, &TimerEvents::timeout1Hz);
    connect(&m_timer42Hz, &QTimer::timeout, this, &TimerEvents::timeout42Hz);
    LOG_TRACE() << "Class initialized";
}

/**
 * Returns a pointer to the only instance of the class
 */
TimerEvents *TimerEvents::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new TimerEvents;

    return INSTANCE;
}

/**
 * Stops all the timers of this module
 */
void TimerEvents::stopTimers()
{
    m_timer1Hz.stop();
    m_timer42Hz.stop();

    LOG_INFO() << "Timers stopped";
}

/**
 * Starts all the timer of the module
 */
void TimerEvents::startTimers()
{
    m_timer1Hz.start();
    m_timer42Hz.start();

    LOG_TRACE() << "Timers started";
}
