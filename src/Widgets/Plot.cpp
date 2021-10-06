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

#include "Plot.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

using namespace Widgets;

Plot::Plot(const int index)
    : m_index(index)
    , m_count(0)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->plotCount())
        return;

    // Set window palette
    QPalette palette;
    palette.setColor(QPalette::Base, theme->datasetWindowBackground());
    palette.setColor(QPalette::Window, theme->datasetWindowBackground());
    setPalette(palette);

    // Set plot palette
    palette.setColor(QPalette::Base, theme->base());
    palette.setColor(QPalette::Highlight, QColor("#f00"));
    palette.setColor(QPalette::Text, theme->widgetIndicator1());
    palette.setColor(QPalette::Dark, theme->widgetIndicator1());
    palette.setColor(QPalette::Light, theme->widgetIndicator1());
    palette.setColor(QPalette::ButtonText, theme->widgetIndicator1());
    palette.setColor(QPalette::WindowText, theme->widgetIndicator1());
    m_plot.setPalette(palette);
    m_plot.setCanvasBackground(theme->base());
    m_plot.setFrameStyle(QFrame::Plain);

    // Configure layout
    m_layout.addWidget(&m_plot);
    m_layout.setContentsMargins(24, 24, 24, 24);
    setLayout(&m_layout);

    // Create curve from data
    updateRange();
    m_curve.attach(&m_plot);
    m_plot.replot();
    m_plot.show();

    // Get curve color
    QString color;
    auto colors = theme->widgetColors();
    if (colors.count() > m_index)
        color = colors.at(m_index);
    else
        color = colors.at(colors.count() % m_index);

    // Set curve color & plot style
    m_curve.setPen(QColor(color), 2, Qt::SolidLine);

    // Lazy widgets, get initial properties from dataset
#ifdef LAZY_WIDGETS
    auto dataset = UI::Dashboard::getInstance()->getBar(m_index);
    if (dataset)
    {
        // Set max/min values
        auto min = dataset->min();
        auto max = dataset->max();
        if (max > min)
            m_plot.setAxisScale(m_plot.y(), min, max);
        else
            m_plot.setAxisAutoScale(m_plot.y(), true);

        // Set units
        if (!dataset->units().isEmpty())
            m_plot.setAxisTitle(m_plot.y(), dataset->units());
    }
#endif

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
    connect(dash, SIGNAL(pointsChanged()), this, SLOT(updateRange()));
}

void Plot::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Update plot widget with new data
    auto dataset = UI::Dashboard::getInstance()->getPlot(m_index);
    if (dataset)
    {
        // Update plot properties
#ifndef LAZY_WIDGETS
        // Set max/min values
        auto min = dataset->min();
        auto max = dataset->max();
        if (max > min)
            m_plot.setAxisScale(m_plot.y(), min, max);
        else
            m_plot.setAxisAutoScale(m_plot.y(), true);

        // Set units
        if (!dataset->units().isEmpty())
            m_plot.setAxisTitle(m_plot.y(), dataset->units());
#endif
        // Add new input to the plot
        memmove(m_yData.data(), m_yData.data() + 1, m_yData.count() * sizeof(double));
        m_yData[m_yData.count() - 1] = dataset->value().toDouble();
        m_curve.setSamples(m_xData, m_yData);
        m_plot.replot();
    }
}

void Plot::updateRange()
{
    // Get pointer to dashboard manager
    auto dash = UI::Dashboard::getInstance();

    // Set number of points
    m_xData.clear();
    m_yData.clear();
    m_xData.reserve(dash->points());
    m_yData.reserve(dash->points());
    for (int i = 0; i < dash->points(); ++i)
    {
        m_xData.append(i);
        m_yData.append(0);
    }

    // Create curve from data
    m_curve.setSamples(m_xData, m_yData);
}
