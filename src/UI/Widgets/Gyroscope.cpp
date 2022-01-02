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

#include <QResizeEvent>

#include <UI/Dashboard.h>
#include <Misc/TimerEvents.h>
#include <Misc/ThemeManager.h>
#include <UI/Widgets/Gyroscope.h>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Gyroscope::Gyroscope(const int index)
    : m_index(index)
    , m_displayNum(0)
{
    // Get pointers to Serial Studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

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

    // Show different values each second
    // clang-format off
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz,
            this, &Widgets::Gyroscope::updateLabel);
    // clang-format on

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
    if (!isEnabled())
        return;

    // Invalid index, abort update
    auto dash = &UI::Dashboard::instance();
    if (m_index < 0 || m_index >= dash->gyroscopeCount())
        return;

    // Get group reference & validate dataset count
    auto group = dash->getGyroscope(m_index);
    if (group.datasetCount() != 3)
        return;

    // Initialize values for pitch, roll & yaw
    double p = 0;
    double r = 0;
    double y = 0;

    // Extract pitch, roll & yaw from group datasets
    for (int i = 0; i < 3; ++i)
    {
        auto dataset = group.getDataset(i);
        if (dataset.widget() == "pitch")
            p = dataset.value().toDouble();
        if (dataset.widget() == "roll")
            r = dataset.value().toDouble();
        if (dataset.widget() == "yaw")
            y = dataset.value().toDouble();
    }

    // Construct strings from pitch, roll & yaw
    m_yaw = QString::number(qAbs(y), 'f', dash->precision());
    m_roll = QString::number(qAbs(r), 'f', dash->precision());
    m_pitch = QString::number(qAbs(p), 'f', dash->precision());

    // Update gauge
    m_gauge.setValue(p);
    m_gauge.setGradient(r / 360.0);

    // Repaint the widget
    requestRepaint();
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
