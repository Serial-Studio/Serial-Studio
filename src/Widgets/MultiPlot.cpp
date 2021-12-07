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

#include "MultiPlot.h"
#include "CSV/Player.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

namespace Widgets
{

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
MultiPlot::MultiPlot(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    const auto dash = UI::Dashboard::getInstance();
    const auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->multiPlotCount())
        return;

    // Set window palette
    QPalette palette;
    palette.setColor(QPalette::Base, theme->widgetWindowBackground());
    palette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(palette);

    // Set plot palette
    palette.setColor(QPalette::Base, theme->base());
    palette.setColor(QPalette::Highlight, QColor("#f00"));
    palette.setColor(QPalette::Text, theme->widgetIndicator());
    palette.setColor(QPalette::Dark, theme->widgetIndicator());
    palette.setColor(QPalette::Light, theme->widgetIndicator());
    palette.setColor(QPalette::ButtonText, theme->widgetIndicator());
    palette.setColor(QPalette::WindowText, theme->widgetIndicator());
    m_plot.setPalette(palette);
    m_plot.setCanvasBackground(theme->base());
    m_plot.setFrameStyle(QFrame::Plain);

    // Configure layout
    m_layout.addWidget(&m_plot);
    m_layout.setContentsMargins(24, 24, 24, 24);
    setLayout(&m_layout);

    // Fit data horizontally
    m_plot.axisScaleEngine(QwtPlot::xBottom)
        ->setAttribute(QwtScaleEngine::Floating, true);

    // Create curves from datasets
    bool normalize = true;
    const auto group = dash->getMultiplot(m_index);
    StringList colors = theme->widgetColors();
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        // Get dataset title & min/max values
        QString title = tr("Unknown");
        const auto dataset = group->getDataset(i);
        if (dataset)
        {
            title = group->getDataset(i)->title();
            normalize &= dataset->max() > dataset->min();
        }

        // Create curve
        auto curve = new QwtPlotCurve(title);

        // Get curve color
        QString color;
        if (colors.count() > i)
            color = colors.at(i);
        else if (i > 0)
            color = colors.at(colors.count() % i);

        // Configure curve
        curve->setPen(QColor(color), 2, Qt::SolidLine);
        curve->attach(&m_plot);

        // Register curve
        m_curves.append(curve);
    }

    // Add plot legend to display curve names
    m_legend.setFrameStyle(QFrame::Plain);
    m_plot.setAxisTitle(QwtPlot::yLeft, group->title());
    m_plot.setAxisTitle(QwtPlot::xBottom, tr("Samples"));
    m_plot.insertLegend(&m_legend, QwtPlot::BottomLegend);

    // Normalize data curves
    if (normalize)
        m_plot.setAxisScale(QwtPlot::yLeft, 0, 1);

    // Show plot
    updateRange();
    m_plot.replot();
    m_plot.show();

    // React to dashboard events
    // clang-format off
    connect(dash, SIGNAL(updated()),
            this, SLOT(updateData()),
            Qt::QueuedConnection);
    connect(dash, SIGNAL(pointsChanged()),
            this, SLOT(updateRange()),
            Qt::QueuedConnection);
    // clang-format on
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the new data shall be saved to the plot
 * vectors, but the widget shall not be redrawn.
 */
void MultiPlot::updateData()
{
    // Get group
    const auto group = UI::Dashboard::getInstance()->getMultiplot(m_index);
    if (!group)
        return;

    // Plot each dataset
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        // Get dataset
        const auto dataset = group->getDataset(i);
        if (!dataset)
            continue;

        // Add point to plot data
        const auto count = m_yData[i].count();
        memmove(m_yData[i].data(), m_yData[i].data() + 1, count * sizeof(double));

        // Normalize dataset value
        if (dataset->max() > dataset->min())
        {
            const auto vmin = dataset->min();
            const auto vmax = dataset->max();
            const auto v = dataset->value().toDouble();
            m_yData[i][count - 1] = (v - vmin) / (vmax - vmin);
        }

        // Plot dataset value directly
        else
            m_yData[i][count - 1] = dataset->value().toDouble();

        // Widget not enabled, do not redraw
        if (!isEnabled())
            continue;

        // Plot new data
        m_curves.at(i)->setSamples(m_yData[i]);
    }

    // Plot widget again
    if (isEnabled())
        m_plot.replot();
}

/**
 * Updates the number of horizontal divisions of the plot
 */
void MultiPlot::updateRange()
{
    // Get pointer to dashboard manager
    const auto dash = UI::Dashboard::getInstance();

    // Set number of points
    m_yData.clear();
    const auto group = UI::Dashboard::getInstance()->getMultiplot(m_index);
    for (int i = 0; i < dash->points(); ++i)
    {
        m_yData.append(PlotData());
        m_yData[i].reserve(dash->points());
        for (int j = 0; j < dash->points(); ++j)
            m_yData[i].append(0);
    }

    // Create curve from data
    for (int i = 0; i < group->datasetCount(); ++i)
        if (m_curves.count() > i)
            m_curves.at(i)->setSamples(*dash->xPlotValues(), m_yData[i]);
}
}
