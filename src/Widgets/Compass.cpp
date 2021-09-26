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

#include <QwtSimpleCompassRose>
#include <QwtCompassWindArrow>

using namespace Widgets;

Compass::Compass(const int index)
    : m_index(index)
{
    // Invalid index, abort
    if (m_index < 0)
        return;

    // Configure compass
    m_compass.setScale(0, 360);
    m_compass.setNeedle(
        new QwtCompassMagnetNeedle(QwtCompassMagnetNeedle::TriangleStyle));

    // Add compass to layout
    m_layout.addWidget(&m_compass);
    m_layout.setContentsMargins(24, 24, 24, 24);
    setLayout(&m_layout);

    // Set stylesheet
    auto theme = Misc::ThemeManager::getInstance();
    auto qss
        = QString("background-color: %1;").arg(theme->datasetWindowBackground().name());
    setStyleSheet(qss);

    // React to dashboard events
    connect(UI::Dashboard::getInstance(), &UI::Dashboard::updated, this,
            &Compass::update);
}

void Compass::update()
{
    auto dash = UI::Dashboard::getInstance();
    auto data = dash->getCompass(m_index);

    if (data)
        m_compass.setValue(data->value().toDouble());
}
