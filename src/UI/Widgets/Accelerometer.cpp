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

#include <QtMath>
#include <QResizeEvent>

#include <UI/Dashboard.h>
#include <Misc/ThemeManager.h>
#include <UI/Widgets/Accelerometer.h>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Accelerometer::Accelerometer(const int index)
    : m_index(index)
{
    // Get pointers to Serial Studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->accelerometerCount())
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
    m_gauge.setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, true,
                                              QColor(needleColor), knobColor));

    // Set gauge scale & display angles
    m_gauge.setScale(0, 12);
    m_gauge.setScaleArc(90, 360);

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
void Widgets::Accelerometer::updateData()
{
    // Widget disabled
    if (!isEnabled())
        return;

    // Invalid index, abort update
    auto dash = &UI::Dashboard::instance();
    if (m_index < 0 || m_index >= dash->accelerometerCount())
        return;

    // Get accelerometer group & validate it
    auto accelerometer = dash->getAccelerometer(m_index);
    if (accelerometer.datasetCount() != 3)
        return;

    // Initialize x, y, z
    double x = 0;
    double y = 0;
    double z = 0;

    // Extract x, y, z from accelerometer group
    for (int i = 0; i < 3; ++i)
    {
        auto dataset = accelerometer.getDataset(i);
        if (dataset.widget() == "x")
            x = dataset.value().toDouble();
        if (dataset.widget() == "y")
            y = dataset.value().toDouble();
        if (dataset.widget() == "z")
            z = dataset.value().toDouble();
    }

    // Divide accelerations by gravitational constant
    x /= 9.18;
    y /= 9.18;
    z /= 9.18;

    // Normalize acceleration vector
    const double G = qSqrt(qPow(x, 2) + qPow(y, 2) + qPow(z, 2));

    // Update gauge
    m_gauge.setValue(G);
    setValue(QString("%1 G").arg(
        QString::number(G, 'f', UI::Dashboard::instance().precision())));

    // Repaint the widget
    requestRepaint();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Accelerometer.cpp"
#endif
