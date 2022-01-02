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

#include <Misc/MacExtras.h>
#include <Misc/Translator.h>

#ifdef Q_OS_MAC
#    include <kdmactouchbar.h>
#endif

Misc::MacExtras::MacExtras()
{
#ifdef Q_OS_MAC
    // Configure action strings
    updateButtonText();

    // Configure action icons
    m_setupAction.setIcon(QIcon("://touchbar/setup.png"));
    m_consoleAction.setIcon(QIcon("://touchbar/console.png"));
    m_dashboardAction.setIcon(QIcon("://touchbar/dashboard.png"));

    // Setup checkable items
    m_setupAction.setCheckable(true);
    m_consoleAction.setCheckable(true);
    m_dashboardAction.setCheckable(true);

    // Set initial button status(es)
    m_setupAction.setChecked(true);
    m_consoleAction.setChecked(true);
    m_dashboardAction.setEnabled(false);

    // Configure signals
    connect(&m_setupAction, SIGNAL(triggered()), this, SIGNAL(setupClicked()));
    connect(&m_consoleAction, SIGNAL(triggered()), this, SIGNAL(consoleClicked()));
    connect(&m_dashboardAction, SIGNAL(triggered()), this, SIGNAL(dashboardClicked()));

    // Create touchbar
    KDMacTouchBar *bar = new KDMacTouchBar((QWidget *)Q_NULLPTR);
    bar->addAction(&m_setupAction);
    bar->addAction(&m_consoleAction);
    bar->addAction(&m_dashboardAction);

    // Re-translate buttons when language is changed
    connect(&Translator::instance(), SIGNAL(languageChanged()), this,
            SLOT(updateButtonText()));
#endif
}

Misc::MacExtras &Misc::MacExtras::instance()
{
    static MacExtras singleton;
    return singleton;
}

void Misc::MacExtras::setSetupChecked(const bool checked)
{
    m_setupAction.setChecked(checked);
}

void Misc::MacExtras::setConsoleChecked(const bool checked)
{
    m_consoleAction.setChecked(checked);
}

void Misc::MacExtras::setDashboardChecked(const bool checked)
{
    m_dashboardAction.setChecked(checked);
}

void Misc::MacExtras::setDashboardEnabled(const bool enabled)
{
    m_dashboardAction.setEnabled(enabled);
}

void Misc::MacExtras::updateButtonText()
{
    m_setupAction.setText(tr("Setup"));
    m_consoleAction.setText(tr("Console"));
    m_dashboardAction.setText(tr("Dashboard"));
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_MacExtras.cpp"
#endif
