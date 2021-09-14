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

#ifndef MISC_MAC_EXTRAS_H
#define MISC_MAC_EXTRAS_H

#include <QAction>
#include <QObject>

namespace Misc
{
class MacExtras : public QObject
{
    Q_OBJECT

signals:
    void setupClicked();
    void widgetsClicked();
    void connectClicked();
    void consoleClicked();
    void dashboardClicked();

public:
    static MacExtras *getInstance();

public slots:
    void setSetupChecked(const bool checked);
    void setConsoleChecked(const bool checked);
    void setWidgetsChecked(const bool checked);
    void setWidgetsEnabled(const bool enabled);
    void setDashboardChecked(const bool checked);
    void setDashboardEnabled(const bool enabled);

private slots:
    void updateButtonText();

private:
    MacExtras();

private:
    QAction m_setupAction;
    QAction m_consoleAction;
    QAction m_widgetsAction;
    QAction m_dashboardAction;
};
}

#endif
