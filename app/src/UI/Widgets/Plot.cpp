/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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
#include "UI/Widgets/Plot.h"
#include "Misc/ThemeManager.h"

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

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->plotCount())
    return;

  // Configure layout
  m_layout.addWidget(&m_plot);
  m_layout.setContentsMargins(8, 8, 8, 8);
  setLayout(&m_layout);

  // Fit data horizontally
  auto xAxisEngine = m_plot.axisScaleEngine(QwtPlot::xBottom);
  xAxisEngine->setAttribute(QwtScaleEngine::Floating, true);

  // Create curve from data
  updateRange();
  m_plot.setFrameStyle(QFrame::Plain);
  m_curve.attach(&m_plot);
  m_plot.replot();
  m_plot.show();

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
  if (dataset.log())
    m_plot.setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));

  // Set axis titles
  m_plot.setAxisTitle(QwtPlot::xBottom, tr("Samples"));
  m_plot.setAxisTitle(QwtPlot::yLeft, dataset.title());

  // Configure visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Plot::onThemeChanged);

  // React to dashboard events
  onAxisOptionsChanged();
  connect(dash, &UI::Dashboard::updated, this, &Plot::updateData,
          Qt::DirectConnection);
  connect(dash, &UI::Dashboard::pointsChanged, this, &Plot::updateRange,
          Qt::DirectConnection);
  connect(dash, &UI::Dashboard::axisVisibilityChanged, this,
          &Plot::onAxisOptionsChanged, Qt::DirectConnection);
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
        double mostDiff
            = qMax<double>(qAbs<double>(m_min), qAbs<double>(m_max));
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
  QVector<qreal> tempYData;
  tempYData.reserve(dash->points());
  for (int i = 0; i < dash->points(); ++i)
    tempYData.append(0);

  // Redraw graph
  m_curve.setSamples(dash->xPlotValues(), tempYData);
  m_plot.replot();
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::Plot::onThemeChanged()
{
  // Set window palette
  auto theme = &Misc::ThemeManager::instance();
  QPalette palette;
  palette.setColor(QPalette::Base,
                   theme->getColor(QStringLiteral("widget_base")));
  palette.setColor(QPalette::Window,
                   theme->getColor(QStringLiteral("widget_window")));
  setPalette(palette);

  // Set plot palette
  palette.setColor(QPalette::Base,
                   theme->getColor(QStringLiteral("widget_base")));
  palette.setColor(QPalette::Highlight,
                   theme->getColor(QStringLiteral("widget_highlight")));
  palette.setColor(QPalette::Text,
                   theme->getColor(QStringLiteral("widget_text")));
  palette.setColor(QPalette::ButtonText,
                   theme->getColor(QStringLiteral("widget_text")));
  palette.setColor(QPalette::WindowText,
                   theme->getColor(QStringLiteral("widget_text")));
  palette.setColor(QPalette::Dark,
                   theme->getColor(QStringLiteral("groupbox_hard_border")));
  palette.setColor(QPalette::Light,
                   theme->getColor(QStringLiteral("groupbox_hard_border")));
  m_plot.setPalette(palette);
  m_plot.setCanvasBackground(
      theme->getColor(QStringLiteral("groupbox_background")));

  // Get curve color
  const auto colors = theme->colors()["widget_colors"].toArray();
  const auto color = colors.count() > m_index
                         ? colors.at(m_index).toString()
                         : colors.at(colors.count() % m_index).toString();

  // Set curve color & plot style
  m_curve.setPen(QColor(color), 2, Qt::SolidLine);
  update();
}

/**
 * @brief Updates the visibility of the plot axes based on user-selected axis
 *        options.
 *
 * This function responds to changes in axis visibility settings from the
 * dashboard. Depending on the user’s selection, it will set the visibility of
 * the X and/or Y axes on the plot.
 *
 * After adjusting the visibility settings, the plot is updated by calling the
 * `update()` method.
 *
 * @see UI::Dashboard::axisVisibility()
 * @see QwtPlot::setAxisVisible()
 */
void Widgets::Plot::onAxisOptionsChanged()
{
  switch (UI::Dashboard::instance().axisVisibility())
  {
    case UI::Dashboard::AxisXY:
      m_plot.setAxisVisible(QwtPlot::yLeft, true);
      m_plot.setAxisVisible(QwtPlot::xBottom, true);
      break;
    case UI::Dashboard::AxisXOnly:
      m_plot.setAxisVisible(QwtPlot::yLeft, false);
      m_plot.setAxisVisible(QwtPlot::xBottom, true);
      break;
    case UI::Dashboard::AxisYOnly:
      m_plot.setAxisVisible(QwtPlot::yLeft, true);
      m_plot.setAxisVisible(QwtPlot::xBottom, false);
      break;
    case UI::Dashboard::NoAxesVisible:
      m_plot.setAxisVisible(QwtPlot::yLeft, false);
      m_plot.setAxisVisible(QwtPlot::xBottom, false);
      break;
  }

  update();
}
