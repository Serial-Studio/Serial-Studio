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

#include <QPainter>
#include <QApplication>
#include <QResizeEvent>
#include <QwtDialNeedle>
#include <QwtRoundScaleDraw>

using namespace Widgets;

//--------------------------------------------------------------------------------------------------
// Gauge/Speedometer implementation (based on Qwt examples)
//--------------------------------------------------------------------------------------------------

GaugeObject::GaugeObject(QWidget *parent)
    : QwtDial(parent)
{
    // Disable controling the gauge with the mouse or keyboard
    setReadOnly(true);

    // Do not reset gauge if we reach maximum value
    setWrapping(false);

    // Set gauge origin & min/max angles
    setOrigin(135);
    setScaleArc(0, 270);
}

QString GaugeObject::label() const
{
    return m_label;
}

void GaugeObject::setLabel(const QString &label)
{
    m_label = label;
    update();
}

void GaugeObject::drawScaleContents(QPainter *painter, const QPointF &center,
                                    double radius) const
{
    // Get label font
    auto labelFont = font();
    labelFont.setPixelSize(1.4 * font().pixelSize());

    // Create draw rectangle
    QRectF rect(0.0, 0.0, 2.0 * radius, 2.0 * radius - labelFont.pixelSize() * 2);
    rect.moveCenter(center);

    // Set text alignment flags
    const int flags = Qt::AlignBottom | Qt::AlignHCenter;

    // Paint label
    painter->setFont(labelFont);
    painter->setPen(Misc::ThemeManager::getInstance()->widgetIndicator1());
    painter->drawText(rect, flags,
                      QString("%1 %2").arg(QString::number(value(), 'f', 2), m_label));
}

//--------------------------------------------------------------------------------------------------
// Gauge widget implementation
//--------------------------------------------------------------------------------------------------

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

    // Set gauge scale & label
#ifdef LAZY_WIDGETS
    auto dataset = dash->getGauge(m_index);
    if (dataset)
    {
        m_gauge.setScale(dataset->min(), dataset->max());
        if (!dataset->units().isEmpty())
            m_gauge.setLabel(dataset->units());
    }
#endif

    // Set window palette
    QPalette windowPalette;
    windowPalette.setColor(QPalette::Base, theme->datasetWindowBackground());
    windowPalette.setColor(QPalette::Window, theme->datasetWindowBackground());
    setPalette(windowPalette);

    // Add gauge to layout
    m_layout.addWidget(&m_gauge);
    m_layout.setContentsMargins(8, 8, 8, 8);
    setLayout(&m_layout);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()));
}

void Gauge::update()
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
        if (!dataset->units().isEmpty())
            m_gauge.setLabel(dataset->units());
#endif
        m_gauge.setValue(dataset->value().toDouble());
    }
}

void Gauge::resizeEvent(QResizeEvent *event)
{
    auto font = UI::Dashboard::getInstance()->monoFont();
    font.setPixelSize(event->size().width() / 32);
    m_gauge.setFont(font);
    event->accept();
}
