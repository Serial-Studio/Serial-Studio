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

#include <CSV/Player.h>
#include <UI/Dashboard.h>
#include <Misc/ThemeManager.h>
#include <UI/Widgets/MultiPlot.h>

/**
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::MultiPlot::MultiPlot(const int index)
  : m_index(index)
{
  // Get pointers to serial studio modules
  auto dash = &UI::Dashboard::instance();

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->multiPlotCount())
    return;

  // Configure layout
  m_layout.addWidget(&m_plot);
  m_layout.setContentsMargins(8, 8, 8, 8);
  setLayout(&m_layout);

  // Fit data horizontally
  m_plot.axisScaleEngine(QwtPlot::xBottom)
      ->setAttribute(QwtScaleEngine::Floating, true);

  // Create curves from datasets
  bool normalize = true;
  auto group = dash->getMultiplot(m_index);
  m_curves.reserve(group.datasetCount());
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    // Get dataset title & min/max values
    QString title = tr("Unknown");
    auto dataset = group.getDataset(i);
    title = dataset.title();
    normalize &= dataset.max() > dataset.min();

    // Create curve & register curve
    auto curve = new QwtPlotCurve(title);
    curve->attach(&m_plot);
    m_curves.append(curve);
  }

  // Add plot legend to display curve names
  m_plot.setFrameStyle(QFrame::Plain);
  m_legend.setFrameStyle(QFrame::Plain);
  m_plot.setAxisTitle(QwtPlot::yLeft, group.title());
  m_plot.setAxisTitle(QwtPlot::xBottom, tr("Samples"));
  // m_plot.insertLegend(&m_legend, QwtPlot::BottomLegend);

  // Normalize data curves
  if (normalize)
    m_plot.setAxisScale(QwtPlot::yLeft, 0, 1);

  // Show plot
  updateRange();
  m_plot.replot();
  m_plot.show();

  // Configure visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::MultiPlot::onThemeChanged);

  // React to dashboard events
  connect(dash, SIGNAL(updated()), this, SLOT(updateData()),
          Qt::QueuedConnection);
  connect(dash, SIGNAL(pointsChanged()), this, SLOT(updateRange()),
          Qt::QueuedConnection);
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the new data shall be saved to the plot
 * vectors, but the widget shall not be redrawn.
 */
void Widgets::MultiPlot::updateData()
{
  // Invalid index, abort update
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->multiPlotCount())
    return;

  // Get group
  auto group = dash->getMultiplot(m_index);

  // Plot each dataset
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    // Check vector size
    if (m_yData.count() < i)
      break;

    // Get dataset
    auto dataset = group.getDataset(i);

    // Add point to plot data
    auto data = m_yData[i].data();
    auto count = m_yData[i].count();
    memmove(data, data + 1, count * sizeof(double));

    // Normalize dataset value
    if (dataset.max() > dataset.min())
    {
      auto vmin = dataset.min();
      auto vmax = dataset.max();
      auto v = dataset.value().toDouble();
      m_yData[i][count - 1] = (v - vmin) / (vmax - vmin);
    }

    // Plot dataset value directly
    else
      m_yData[i][count - 1] = dataset.value().toDouble();

    // Widget not enabled, do not redraw
    if (!isEnabled())
      continue;

    // Plot new data
    m_curves.at(i)->setSamples(m_yData[i]);
  }

  // Plot widget again
  if (isEnabled())
  {
    m_plot.replot();
    requestRepaint();
  }
}

/**
 * Updates the number of horizontal divisions of the plot
 */
void Widgets::MultiPlot::updateRange()
{
  // Invalid index, abort update
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->multiPlotCount())
    return;

  // Set number of points
  m_yData.clear();
  auto group = UI::Dashboard::instance().getMultiplot(m_index);
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    m_yData.append(QVector<qreal>());
    m_yData.last().resize(dash->points());
    std::fill(m_yData.last().begin(), m_yData.last().end(), 0.0001);
  }

  // Create curve from data
  for (int i = 0; i < group.datasetCount(); ++i)
    if (m_curves.count() > i)
      m_curves.at(i)->setSamples(dash->xPlotValues(), m_yData[i]);

  // Repaint widget
  requestRepaint();
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::MultiPlot::onThemeChanged()
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

  // Update curve color
  const auto colors = theme->colors()["widget_colors"].toArray();
  for (int i = 0; i < m_curves.count(); ++i)
  {
    auto curve = m_curves.at(i);
    const auto color = colors.count() > m_index
                           ? colors.at(i).toString()
                           : colors.at(colors.count() % i).toString();

    curve->setPen(QColor(color), 2, Qt::SolidLine);
  }

  // Re-draw widget
  update();
}
