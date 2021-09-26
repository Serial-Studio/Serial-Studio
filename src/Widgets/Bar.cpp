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

#include <QResizeEvent>

using namespace Widgets;

/**
 * Configures the bard widget style & the signals/slots with the dashboard module
 */
Bar::Bar(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->barCount())
        return;

    // Get thermo color
    QString color;
    auto barcl = theme->barWidgetColors();
    if (barcl.count() > m_index)
        color = barcl.at(m_index);
    else
        color = barcl.at(barcl.count() % m_index);

    // Configure thermo style
    m_thermo.setPipeWidth(64);
    m_thermo.setFillBrush(QBrush(QColor(color)));

    // Configure label style
    QFont font = dash->monoFont();
    font.setPixelSize(24);
    m_label.setFont(font);
    m_label.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Configure layout
    m_layout.addWidget(&m_thermo);
    m_layout.addWidget(&m_label);
    m_layout.setSpacing(24);
    m_layout.setStretch(0, 0);
    m_layout.setStretch(1, 1);
    m_layout.setContentsMargins(24, 24, 24, 24);
    setLayout(&m_layout);

    // Generate stylesheets
    // clang-format off
    auto labelQss = QSS("color:%1; border:1px solid %2;",
                        theme->datasetValue(),
                        theme->datasetWindowBorder());
    auto windowQss = QSS("background-color:%1",
                         theme->datasetWindowBackground());
    // clang-format on

    // Set stylesheets
    setStyleSheet(windowQss);
    m_label.setStyleSheet(labelQss);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()));
}

/**
 * Updates the widget's data
 */
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
        m_label.setText(QString("%1 %2").arg(dataset->value(), dataset->units()));
    }
}

/**
 * Changes the size of the thermo and value label when the widget is resized
 */
void Bar::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    m_thermo.setPipeWidth(width / 4);
    QFont font = m_label.font();
    font.setPixelSize(width / 16);
    m_label.setFont(font);
    event->accept();
}
