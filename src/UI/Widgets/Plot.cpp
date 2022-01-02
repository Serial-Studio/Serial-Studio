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

#include <CSV/Player.h>
#include <UI/Dashboard.h>
#include <UI/Widgets/Plot.h>
#include <Misc/ThemeManager.h>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::Plot::Plot(const int index)
    : m_index(index)
    , m_min(INT_MAX)
    , m_max(INT_MIN)
    , m_autoscale(true)
{
    // Get pointers to serial studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->plotCount())
        return;

    // Set window palette
    QPalette palette;
    palette.setColor(QPalette::Base, theme->widgetWindowBackground());
    palette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(palette);

    // Set plot palette
    palette.setColor(QPalette::Base, theme->base());
    palette.setColor(QPalette::Highlight, QColor(255, 0, 0));
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
    auto xAxisEngine = m_plot.axisScaleEngine(QwtPlot::xBottom);
    xAxisEngine->setAttribute(QwtScaleEngine::Floating, true);

    // Create curve from data
    updateRange();
    m_curve.attach(&m_plot);
    m_plot.replot();
    m_plot.show();

    // Get curve color
    QString color;
    StringList colors = theme->widgetColors();
    if (colors.count() > m_index)
        color = colors.at(m_index);
    else
        color = colors.at(colors.count() % m_index);

    // Set curve color & plot style
    m_curve.setPen(QColor(color), 2, Qt::SolidLine);

    // Update graph scale
    auto dataset = UI::Dashboard::instance().getPlot(m_index);
    auto max = dataset.max();
    auto min = dataset.min();
    if (max > min)
    {
        m_max = max;
        m_min = min;
        m_autoscale = false;
        m_plot.setAxisScale(QwtPlot::yLeft, m_min, m_max);
    }

    // Enable logarithmic scale
    // clang-format off
    if (dataset.log())
        m_plot.setAxisScaleEngine(QwtPlot::yLeft,
                                  new QwtLogScaleEngine(10));
    // clang-format on

    // Set axis titles
    m_plot.setAxisTitle(QwtPlot::xBottom, tr("Samples"));
    m_plot.setAxisTitle(QwtPlot::yLeft, dataset.title());

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
 * vector, but the widget shall not be redrawn.
 */
void Widgets::Plot::updateData()
{
    // Widget not enabled, do not redraw
    if (!isEnabled())
        return;

    // Get new data
    auto plotData = UI::Dashboard::instance().linearPlotValues();
    if (plotData.count() > m_index)
    {
        // Check if we need to update graph scale
        if (m_autoscale)
        {
            // Scan new values to see if chart should be updated
            bool changed = false;
            for (int i = 0; i < plotData.at(m_index).count(); ++i)
            {
                auto v = plotData.at(m_index).at(i);
                if (v > m_max)
                {
                    m_max = v + 1;
                    changed = true;
                }

                if (v < m_min)
                {
                    m_min = v - 1;
                    changed = true;
                }
            }

            // Update graph scale
            if (changed)
            {
                // Get central value
                double median = qMax<double>(1, (m_max + m_min)) / 2;
                if (m_max == m_min)
                    median = m_max;

                // Center graph verticaly
                double mostDiff = qMax<double>(qAbs<double>(m_min), qAbs<double>(m_max));
                double min = median * (1 - 0.5) - qAbs<double>(median - mostDiff);
                double max = median * (1 + 0.5) + qAbs<double>(median - mostDiff);
                if (m_min < 0)
                    min = max * -1;

                // Fix issues when min & max are equal
                if (min == max)
                {
                    max = qAbs<double>(max);
                    min = max * -1;
                }

                // Fix issues on min = max = (0,0)
                if (min == 0 && max == 0)
                {
                    max = 1;
                    min = -1;
                }

                // Update axis scale
                m_max = max;
                m_min = min;
                m_plot.setAxisScale(m_plot.yLeft, m_min, m_max);
            }
        }

        // Replot graph
        m_curve.setSamples(plotData.at(m_index));
        m_plot.replot();

        // Repaint widget
        requestRepaint();
    }
}

/**
 * Updates the number of horizontal divisions of the plot
 */
void Widgets::Plot::updateRange()
{
    // Get pointer to dashboard manager
    auto dash = &UI::Dashboard::instance();

    // Clear Y-axis data
    PlotData tempYData;
    tempYData.reserve(dash->points());
    for (int i = 0; i < dash->points(); ++i)
        tempYData.append(0);

    // Redraw graph
    m_curve.setSamples(dash->xPlotValues(), tempYData);
    m_plot.replot();

    // Repaint widget
    requestRepaint();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Plot.cpp"
#endif
