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

#include "Bar.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

using namespace Widgets;

Bar::Bar(const int index)
    : m_index(index)
{
    if (m_index >= 0)
    {
        // Configure thermo & add it to layout
        m_layout.addWidget(&m_thermo);
        m_layout.setContentsMargins(24, 24, 24, 24);
        setLayout(&m_layout);

        // Get thermo color
        QString color;
        auto theme = Misc::ThemeManager::getInstance();
        auto barcl = theme->barWidgetColors();
        if (barcl.count() > m_index)
            color = barcl.at(m_index);
        else
            color = barcl.at(barcl.count() % m_index);

        // Configure thermo style
        m_thermo.setPipeWidth(64);
        m_thermo.setFillBrush(QBrush(QColor(color)));

        // Set window background
        // clang-format off
        auto qss = QString("background-color: %1;").arg(theme->datasetWindowBackground().name());
        setStyleSheet(qss);

        // React to dashboard events
        connect(UI::Dashboard::getInstance(),
                &UI::Dashboard::updated,
                this, &Bar::update);
        // clang-format on
    }
}

void Bar::update()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update bar level
    auto dataset = UI::Dashboard::getInstance()->getBar(m_index);
    if (dataset)
    {
        m_thermo.setAlarmLevel(dataset->alarm());
        m_thermo.setAlarmEnabled(m_thermo.alarmLevel() > 0);
        m_thermo.setScale(dataset->min(), dataset->max());
        m_thermo.setValue(dataset->value().toDouble());
    }
}
