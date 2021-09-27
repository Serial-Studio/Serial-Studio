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

using namespace Widgets;

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Compass::Compass(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

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
    palette.setColor(QPalette::Text, theme->widgetIndicator1());
    m_compass.setPalette(palette);

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
    m_layout.addWidget(&m_compass);
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

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Compass::update()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update compass heading
    auto dataset = UI::Dashboard::getInstance()->getCompass(m_index);
    if (dataset)
    {
        auto value = dataset->value().toDouble() * 12;
        auto text = QString("%1°").arg(QString::number(value, 'f', 0));
        m_compass.setValue(value);

        if (text.length() == 2)
            text.prepend("00");
        else if (text.length() == 3)
            text.prepend("0");

        m_label.setText(text);
    }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void Compass::resizeEvent(QResizeEvent *event)
{
    auto labelFont = UI::Dashboard::getInstance()->monoFont();
    auto compassFont = UI::Dashboard::getInstance()->monoFont();
    labelFont.setPixelSize(event->size().width() / 18);
    compassFont.setPixelSize(event->size().width() / 24);
    m_label.setFont(labelFont);
    m_compass.setFont(compassFont);
    m_label.setMinimumWidth(event->size().width() * 0.4);
    m_label.setMaximumWidth(event->size().width() * 0.4);
    m_label.setMaximumHeight(event->size().height() * 0.4);
    event->accept();
}
