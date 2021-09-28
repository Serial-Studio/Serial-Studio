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

#include "Gauge.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>

using namespace Widgets;

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Gauge::Gauge(const int index)
    : m_index(index)
{
    // Get pointers to Serial Studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

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
#ifdef LAZY_WIDGETS
    auto dataset = dash->getGauge(m_index);
    if (dataset)
        m_gauge.setScale(dataset->min(), dataset->max());
#endif

    // Set gauge palette
    QPalette gaugePalette;
    gaugePalette.setColor(QPalette::WindowText, theme->base());
    gaugePalette.setColor(QPalette::Text, theme->widgetIndicator1());
    m_gauge.setPalette(gaugePalette);

    // Set window palette
    QPalette windowPalette;
    windowPalette.setColor(QPalette::Base, theme->datasetWindowBackground());
    windowPalette.setColor(QPalette::Window, theme->datasetWindowBackground());
    setPalette(windowPalette);

    // Configure label style
    QFont font = dash->monoFont();
    font.setPixelSize(24);
    m_label.setFont(font);
    m_label.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Configure layout
    m_layout.setSpacing(24);
    m_layout.addWidget(&m_gauge);
    m_layout.addWidget(&m_label);
    m_layout.setContentsMargins(24, 24, 24, 24);
    m_layout.setAlignment(&m_gauge, Qt::AlignHCenter);
    setLayout(&m_layout);

    // Set stylesheets
    // clang-format off
    auto valueQSS = QSS("background-color:%1; color:%2; border:1px solid %3;",
                        theme->base(),
                        theme->widgetForegroundPrimary(),
                        theme->widgetIndicator1());
    m_label.setStyleSheet(valueQSS);
    // clang-format on

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Gauge::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update compass heading
    auto dataset = UI::Dashboard::getInstance()->getGauge(m_index);
    if (dataset)
    {
#ifndef LAZY_WIDGETS
        m_gauge.setScale(dataset->min(), dataset->max());
#endif
        auto value = dataset->value().toDouble();
        m_gauge.setValue(dataset->value().toDouble());
        m_label.setText(
            QString("%1 %2").arg(QString::number(value, 'f', 2), dataset->units()));
    }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void Gauge::resizeEvent(QResizeEvent *event)
{
    // Get width & height (exluding layout margins & spacing)
    auto width = event->size().width() - 72;
    auto height = event->size().height() - 72;

    // Get fonts & calculate size
    auto labelFont = UI::Dashboard::getInstance()->monoFont();
    auto gaugeFont = UI::Dashboard::getInstance()->monoFont();
    labelFont.setPixelSize(width / 18);
    gaugeFont.setPixelSize(width / 24);

    // Set fonts
    m_label.setFont(labelFont);
    m_gauge.setFont(gaugeFont);

    // Set widget sizes
    m_label.setMinimumWidth(width * 0.4);
    m_label.setMaximumWidth(width * 0.4);
    m_gauge.setMinimumWidth(width * 0.6);
    m_gauge.setMaximumWidth(width * 0.6);
    m_label.setMaximumHeight(height * 0.4);

    // Accept event
    event->accept();
}

void Gauge::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Clicked";
    event->ignore();
}
