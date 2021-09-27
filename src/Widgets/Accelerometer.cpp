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

#include "UI/Dashboard.h"
#include "Accelerometer.h"
#include "Misc/ThemeManager.h"

#include <QtMath>
#include <QResizeEvent>

using namespace Widgets;

Accelerometer::Accelerometer(const int index)
    : m_index(index)
{
    // Get pointers to Serial Studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

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
    auto needle = new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, true,
                                          QColor(needleColor), knobColor);
    m_gauge.setNeedle(needle);
    m_gauge.setFont(dash->monoFont());

    // Set gauge scale & display angles
    m_gauge.setScale(0, 12);
    m_gauge.setScaleArc(90, 360);
    m_gauge.setLabelEnabled(false);

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
    m_layout.addWidget(&m_gauge);
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

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()));
}

void Accelerometer::update()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update accelerometer values
    auto accelerometer = UI::Dashboard::getInstance()->getAccelerometer(m_index);
    if (accelerometer)
    {
        if (accelerometer->datasetCount() != 3)
            return;

        double x = 0;
        double y = 0;
        double z = 0;

        JSON::Dataset *dataset;
        for (int i = 0; i < 3; ++i)
        {
            dataset = accelerometer->getDataset(i);
            if (dataset->widget() == "x")
                x = dataset->value().toDouble();
            if (dataset->widget() == "y")
                y = dataset->value().toDouble();
            if (dataset->widget() == "z")
                z = dataset->value().toDouble();
        }

        x /= 9.18;
        y /= 9.18;
        z /= 9.18;

        const double G = qSqrt(qPow(x, 2) + qPow(y, 2) + qPow(z, 2));

        m_gauge.setValue(G);
        m_label.setText(QString("%1 G").arg(QString::number(G, 'f', 2)));
    }
}

void Accelerometer::resizeEvent(QResizeEvent *event)
{
    auto labelFont = UI::Dashboard::getInstance()->monoFont();
    auto gaugeFont = UI::Dashboard::getInstance()->monoFont();
    labelFont.setPixelSize(event->size().width() / 18);
    gaugeFont.setPixelSize(event->size().width() / 24);
    m_label.setFont(labelFont);
    m_gauge.setFont(gaugeFont);
    m_label.setMinimumWidth(event->size().width() * 0.4);
    m_label.setMaximumWidth(event->size().width() * 0.4);
    m_label.setMaximumHeight(event->size().height() * 0.4);
    event->accept();
}
