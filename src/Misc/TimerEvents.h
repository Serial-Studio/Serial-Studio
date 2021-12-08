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

#include <QTimer>
#include <QObject>

namespace Misc
{
/**
 * @brief The TimerEvents class
 *
 * The @c TimerEvents class implements periodic timers that are used to update
 * the user interface elements at a specific frequency.
 *
 * It is necessary to use this class in order to avoid overloading the computer when
 * incoming data is received at high frequencies. We do not want to re-generate the UI
 * with every received frame, because that would probably freeze the GUI thread. Instead,
 * we "register" all incoming frames and process these frames in regular intervals.
 */
class TimerEvents : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int highFreqTimeoutHz
               READ highFreqTimeoutHz
               WRITE setHighFreqTimeout
               NOTIFY highFreqTimeoutChanged)
    // clang-format on

Q_SIGNALS:
    void lowFreqTimeout();
    void highFreqTimeout();
    void highFreqTimeoutChanged();

public:
    static TimerEvents *getInstance();
    int highFreqTimeoutHz() const;

public Q_SLOTS:
    void stopTimers();
    void startTimers();
    void setHighFreqTimeout(const int hz);

private:
    TimerEvents();

private:
    QTimer m_timerLowFreq;
    QTimer m_timerHighFreq;
};
}
