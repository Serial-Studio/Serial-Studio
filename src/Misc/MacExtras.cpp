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

#include "MacExtras.h"
#include "Translator.h"

#ifdef Q_OS_MAC
#    include <kdmactouchbar.h>
#endif

using namespace Misc;

MacExtras *INSTANCE = nullptr;

MacExtras::MacExtras()
{
#ifdef Q_OS_MAC
    // Configure action strings
    updateButtonText();

    // Configure action icons
    m_setupAction.setIcon(QIcon("://mac-icons/setup.png"));
    m_consoleAction.setIcon(QIcon("://mac-icons/console.png"));
    m_widgetsAction.setIcon(QIcon("://mac-icons/widgets.png"));
    m_dashboardAction.setIcon(QIcon("://mac-icons/dashboard.png"));

    // Setup checkable items
    m_setupAction.setCheckable(true);
    m_consoleAction.setCheckable(true);
    m_widgetsAction.setCheckable(true);
    m_dashboardAction.setCheckable(true);

    // Set initial button status(es)
    m_setupAction.setChecked(true);
    m_consoleAction.setChecked(true);
    m_widgetsAction.setEnabled(false);
    m_dashboardAction.setEnabled(false);

    // Configure signals
    connect(&m_setupAction, SIGNAL(triggered()), this, SIGNAL(setupClicked()));
    connect(&m_consoleAction, SIGNAL(triggered()), this, SIGNAL(consoleClicked()));
    connect(&m_widgetsAction, SIGNAL(triggered()), this, SIGNAL(widgetsClicked()));
    connect(&m_dashboardAction, SIGNAL(triggered()), this, SIGNAL(dashboardClicked()));

    // Create touchbar
    KDMacTouchBar *bar = new KDMacTouchBar((QWidget *)nullptr);
    bar->addAction(&m_setupAction);
    bar->addAction(&m_consoleAction);
    bar->addAction(&m_dashboardAction);
    bar->addAction(&m_widgetsAction);

    // Re-translate buttons when language is changed
    connect(Translator::getInstance(), SIGNAL(languageChanged()), this,
            SLOT(updateButtonText()));
#endif
}

MacExtras *MacExtras::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new MacExtras;

    return INSTANCE;
}

void MacExtras::setSetupChecked(const bool checked)
{
    m_setupAction.setChecked(checked);
}

void MacExtras::setConsoleChecked(const bool checked)
{
    m_consoleAction.setChecked(checked);
}

void MacExtras::setWidgetsChecked(const bool checked)
{
    m_widgetsAction.setChecked(checked);
}

void MacExtras::setWidgetsEnabled(const bool enabled)
{
    m_widgetsAction.setEnabled(enabled);
}

void MacExtras::setDashboardChecked(const bool checked)
{
    m_dashboardAction.setChecked(checked);
}

void MacExtras::setDashboardEnabled(const bool enabled)
{
    m_dashboardAction.setEnabled(enabled);
}

void MacExtras::updateButtonText()
{
    m_setupAction.setText(tr("Setup"));
    m_consoleAction.setText(tr("Console"));
    m_widgetsAction.setText(tr("Widgets"));
    m_dashboardAction.setText(tr("Dashboard"));
}
