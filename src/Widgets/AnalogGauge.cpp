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

#include <QPainter>
#include <QwtDialNeedle>
#include <QwtRoundScaleDraw>

#include "AnalogGauge.h"
#include "Misc/ThemeManager.h"

using namespace Widgets;

/**
 * Configures the @c QwtDial so that it resembles a speedometer gauge.
 */
AnalogGauge::AnalogGauge(QWidget *parent)
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

/**
 * Returns the current value & units displayed by the gauge.
 */
QString AnalogGauge::label() const
{
    return m_label;
}

/**
 * Changes the label text displayed under the gauge.
 */
void AnalogGauge::setLabel(const QString &label)
{
    m_label = label;
    update();
}

/**
 * Re-draws the label that displays the current value & units.
 */
void AnalogGauge::drawScaleContents(QPainter *painter, const QPointF &center,
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
    painter->setPen(Misc::ThemeManager::getInstance()->widgetForegroundPrimary());
    painter->drawText(rect, flags,
                      QString("%1 %2").arg(QString::number(value(), 'f', 2), m_label));
}
