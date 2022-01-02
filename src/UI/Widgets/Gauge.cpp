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

#include <UI/Dashboard.h>
#include <UI/Widgets/Gauge.h>
#include <Misc/ThemeManager.h>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Gauge::Gauge(const int index)
    : m_index(index)
{
    // Get pointers to Serial Studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->gaugeCount())
        return;

    // Get needle & knob color
    QString needleColor;
    auto colors = theme->widgetColors();
    auto knobColor = theme->widgetControlBackground();
    if (colors.count() > m_index)
        needleColor = colors.at(m_index);
    else
        needleColor = colors.at(colors.count() % m_index);

    // Configure gauge needle
    auto needle = new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, true,
                                          QColor(needleColor), knobColor);
    m_gauge.setNeedle(needle);
    m_gauge.setFont(dash->monoFont());

    // Set gauge scale
    auto dataset = dash->getGauge(m_index);
    m_gauge.setScale(dataset.min(), dataset.max());

    // Set gauge palette
    QPalette palette;
    palette.setColor(QPalette::WindowText, theme->base());
    palette.setColor(QPalette::Text, theme->widgetIndicator());
    m_gauge.setPalette(palette);

    // Set widget pointer
    setWidget(&m_gauge);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()), Qt::QueuedConnection);
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::Gauge::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Invalid index, abort update
    auto dash = &UI::Dashboard::instance();
    if (m_index < 0 || m_index >= dash->gaugeCount())
        return;

    // Update gauge value
    auto dataset = dash->getGauge(m_index);
    m_gauge.setValue(dataset.value().toDouble());
    setValue(QString("%1 %2").arg(
        QString::number(dataset.value().toDouble(), 'f', dash->precision()),
        dataset.units()));

    // Repaint widget
    requestRepaint();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Gauge.cpp"
#endif
