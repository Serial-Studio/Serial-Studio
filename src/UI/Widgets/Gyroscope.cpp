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

#include "Gyroscope.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QResizeEvent>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Gyroscope::Gyroscope(const int index)
    : m_index(index)
    , m_displayNum(0)
{
    // Get pointers to Serial Studio modules
    const auto dash = &UI::Dashboard::instance();
    const auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->gyroscopeCount())
        return;

    // Set gauge palette
    QPalette palette;
    palette.setColor(QPalette::WindowText, theme->base());
    palette.setColor(QPalette::Text, theme->widgetIndicator());
    m_gauge.setPalette(palette);

    // Set widget pointer
    setWidget(&m_gauge);

    // Configure timer
    m_timer.setInterval(500);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateLabel()));
    m_timer.start();

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
void Widgets::Gyroscope::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update gyroscope values
    const auto dash = &UI::Dashboard::instance();
    const auto gyro = dash->getGyroscope(m_index);
    if (gyro)
    {
        if (gyro->datasetCount() != 3)
            return;

        double pitch = 0;
        double roll = 0;
        double yaw = 0;

        JSON::Dataset *dataset;
        for (int i = 0; i < 3; ++i)
        {
            dataset = gyro->getDataset(i);
            if (dataset->widget() == "pitch")
                pitch = dataset->value().toDouble();
            if (dataset->widget() == "roll")
                roll = dataset->value().toDouble();
            if (dataset->widget() == "yaw")
                yaw = dataset->value().toDouble();
        }

        m_yaw = QString::number(qAbs(yaw), 'f', dash->precision());
        m_roll = QString::number(qAbs(roll), 'f', dash->precision());
        m_pitch = QString::number(qAbs(pitch), 'f', dash->precision());

        m_gauge.setValue(pitch);
        m_gauge.setGradient(roll / 360.0);

        requestRepaint();
    }
}

void Widgets::Gyroscope::updateLabel()
{
    switch (m_displayNum)
    {
        case 0:
            setValue(QString("%1° PITCH").arg(m_pitch));
            break;
        case 1:
            setValue(QString("%1° ROLL").arg(m_roll));
            break;
        case 2:
            setValue(QString("%1° YAW").arg(m_yaw));
            break;
    }

    ++m_displayNum;
    if (m_displayNum > 2)
        m_displayNum = 0;
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Gyroscope.cpp"
#endif
