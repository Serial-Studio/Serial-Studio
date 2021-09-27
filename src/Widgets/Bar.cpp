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
 * Constructor function, configures widget style & signal/slot connections.
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

    // Set stylesheets
    // clang-format off
    auto valueQSS = QSS("background-color:%1; color:%2; border:1px solid %3;",
                        theme->base(),
                        theme->widgetForegroundPrimary(),
                        theme->widgetIndicator1());
    m_label.setStyleSheet(valueQSS);
    // clang-format on

    // Set window palette
    QPalette windowPalette;
    windowPalette.setColor(QPalette::Base, theme->datasetWindowBackground());
    windowPalette.setColor(QPalette::Window, theme->datasetWindowBackground());
    setPalette(windowPalette);

    // Set thermo palette
    QPalette thermoPalette;
    thermoPalette.setColor(QPalette::Base, theme->base());
    thermoPalette.setColor(QPalette::Highlight, QColor("#f00"));
    thermoPalette.setColor(QPalette::Text, theme->widgetIndicator1());
    thermoPalette.setColor(QPalette::Dark, theme->widgetIndicator1());
    thermoPalette.setColor(QPalette::Light, theme->widgetIndicator1());
    thermoPalette.setColor(QPalette::ButtonText, theme->widgetIndicator1());
    thermoPalette.setColor(QPalette::WindowText, theme->widgetIndicator1());
    m_thermo.setPalette(thermoPalette);

    // Get thermo color
    QString color;
    auto colors = theme->widgetColors();
    if (colors.count() > m_index)
        color = colors.at(m_index);
    else
        color = colors.at(colors.count() % m_index);

    // Configure thermo style
    m_thermo.setPipeWidth(64);
    m_thermo.setBorderWidth(1);
    m_thermo.setFillBrush(QBrush(QColor(color)));

    // Lazy widgets, get initial properties from dataset
#ifdef LAZY_WIDGETS
    auto dataset = UI::Dashboard::getInstance()->getBar(m_index);
    if (dataset)
    {
        m_thermo.setAlarmLevel(dataset->alarm());
        m_thermo.setAlarmEnabled(m_thermo.alarmLevel() > 0);
        m_thermo.setScale(dataset->min(), dataset->max());
    }
#endif

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()));
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
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
#ifndef LAZY_WIDGETS
        m_thermo.setAlarmLevel(dataset->alarm());
        m_thermo.setAlarmEnabled(m_thermo.alarmLevel() > 0);
        m_thermo.setScale(dataset->min(), dataset->max());
#endif
        auto value = dataset->value().toDouble();
        m_thermo.setValue(value);
        m_label.setText(
            QString("%1 %2").arg(QString::number(value, 'f', 2), dataset->units()));
    }
}

/**
 * Resizes the thermo and the value display label to fit the size
 * of the parent window.
 */
void Bar::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    m_thermo.setPipeWidth(width / 4);
    QFont font = m_label.font();
    font.setPixelSize(width / 16);
    m_label.setFont(font);
    m_label.setMaximumHeight(event->size().height() * 0.4);
    event->accept();
}
