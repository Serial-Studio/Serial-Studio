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

#include "Compass.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QResizeEvent>
#include <QwtCompassScaleDraw>
#include <QwtCompassMagnetNeedle>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Compass::Compass(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    const auto dash = &UI::Dashboard::instance();
    const auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->compassCount())
        return;

    // Set compass style
    QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, true);
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Labels, true);
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    scaleDraw->setTickLength(QwtScaleDiv::MinorTick, 1);
    scaleDraw->setTickLength(QwtScaleDiv::MediumTick, 1);
    scaleDraw->setTickLength(QwtScaleDiv::MajorTick, 3);

    // Configure compass scale & needle
    m_compass.setScaleDraw(scaleDraw);
    m_compass.setScaleMaxMajor(36);
    m_compass.setScaleMaxMinor(5);
    m_compass.setNeedle(new QwtCompassMagnetNeedle(QwtCompassMagnetNeedle::ThinStyle));

    // Set compass palette
    QPalette palette;
    palette.setColor(QPalette::WindowText, theme->base());
    palette.setColor(QPalette::Text, theme->widgetIndicator());
    m_compass.setPalette(palette);

    // Set widget pointer
    setWidget(&m_compass);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()), Qt::QueuedConnection);
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::Compass::update()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update compass heading
    const auto dataset = UI::Dashboard::instance().getCompass(m_index);
    if (dataset)
    {
        auto value = dataset->value().toDouble();
        auto text = QString("%1°").arg(
            QString::number(value, 'f', UI::Dashboard::instance().precision()));
        m_compass.setValue(value);

        if (text.length() == 2)
            text.prepend("00");
        else if (text.length() == 3)
            text.prepend("0");

        setValue(text);

        // Repaint widget
        requestRepaint();
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Compass.cpp"
#endif
