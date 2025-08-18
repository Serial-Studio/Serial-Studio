/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Dashboard.h"
#include "UI/Widgets/Plot.h"

/**
 * @brief Constructs a Plot widget.
 * @param index The index of the plot in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Plot::Plot(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_dataW(0)
  , m_dataH(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_monotonicData(true)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    const auto &yDataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);

    m_minY = qMin(yDataset.pltMin, yDataset.pltMax);
    m_maxY = qMax(yDataset.pltMin, yDataset.pltMax);

    const auto xAxisId = SerialStudio::activated() ? yDataset.xAxisId : 0;
    if (UI::Dashboard::instance().datasets().contains(xAxisId))
    {
      m_monotonicData = false;
      const auto &xDataset = UI::Dashboard::instance().datasets()[xAxisId];
      m_xLabel = xDataset.title;
      if (!xDataset.units.isEmpty())
        m_xLabel += " (" + xDataset.units + ")";
    }

    else
    {
      m_monotonicData = true;
      m_xLabel = tr("Samples");
    }

    m_yLabel = yDataset.title;
    if (!yDataset.units.isEmpty())
      m_yLabel += " (" + yDataset.units + ")";

    connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this,
            &Plot::updateRange);

    calculateAutoScaleRange();
    updateRange();
  }
}

/**
 * @brief Returns the size of the down-sampled X axis data.
 * @return Size of down-sampled X axis data.
 */
int Widgets::Plot::dataW() const
{
  return m_dataW;
}

/**
 * @brief Returns the size of the down-sampled Y axis data.
 * @return Size of down-sampled Y axis data.
 */
int Widgets::Plot::dataH() const
{
  return m_dataH;
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
double Widgets::Plot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
double Widgets::Plot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
double Widgets::Plot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
double Widgets::Plot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Checks whether plot data updates are currently active.
 * @return @c true if updating is not paused, otherwise @c false.
 */
bool Widgets::Plot::running() const
{
  return UI::Dashboard::instance().plotRunning(m_index);
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
double Widgets::Plot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval with human-readable values.
 * @return The Y-axis tick interval.
 */
double Widgets::Plot::yTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minY, m_maxY);
}

/**
 * @brief Returns the Y-axis label.
 * @return The Y-axis label.
 */
const QString &Widgets::Plot::yLabel() const
{
  return m_yLabel;
}

/**
 * @brief Returns the X-axis label.
 * @return The X-axis label.
 */
const QString &Widgets::Plot::xLabel() const
{
  return m_xLabel;
}

/**
 * @brief Draws the data on the given QLineSeries.
 * @param series The QLineSeries to draw the data on.
 */
void Widgets::Plot::draw(QXYSeries *series)
{
  if (series)
  {
    updateData();
    series->replace(m_data);
    calculateAutoScaleRange();
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the size of the down-sampled X axis data.
 * @param width The new size of the down-sampled axis data.
 */
void Widgets::Plot::setDataW(const int width)
{
  if (m_dataW != width)
  {
    m_dataW = width;
    updateData();

    Q_EMIT dataSizeChanged();
  }
}

/**
 * @brief Updates the size of the down-sampled Y axis data.
 * @param height The new size of the down-sampled axis data.
 */
void Widgets::Plot::setDataH(const int height)
{
  if (m_dataH != height)
  {
    m_dataH = height;
    updateData();

    Q_EMIT dataSizeChanged();
  }
}

/**
 * @brief Enables or disables plot data updates.
 * @param enabled Set to @c true to allow updates, or @c false to pause them.
 */
void Widgets::Plot::setRunning(const bool enabled)
{
  UI::Dashboard::instance().setPlotRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Updates the plot data from the Dashboard.
 */
void Widgets::Plot::updateData()
{
  // Share workspace data
  static thread_local DSP::DownsampleWorkspace ws;

  // Stop if widget is disabled
  if (!isEnabled())
    return;

  // Only obtain data if widget data is still valid
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    // Obtain plot data
    const auto &plotData = UI::Dashboard::instance().plotData(m_index);

    // Downsample data that only has one Y point per X point
    if (m_monotonicData)
      DSP::downsampleMonotonic(plotData, m_dataW, m_dataH, m_data, &ws);

    // Draw directly on complex plots (such as Lorenz Attractor)
    else
    {
      // Get X/Y axis
      const auto &X = *plotData.x;
      const auto &Y = *plotData.y;

      // Resize series array if needed
      const qsizetype count = std::min(X.size(), Y.size());
      if (m_data.size() != count)
        m_data.resize(count);

      // Obtain raw pointers
      QPointF *out = m_data.data();
      const auto *xData = X.raw();
      const auto *yData = Y.raw();

      // Get queue states for faster iteration
      std::size_t xIdx = X.frontIndex();
      std::size_t yIdx = Y.frontIndex();

      // Update plot data points, avoid queue operations overhead
      for (qsizetype i = 0; i < count; ++i)
      {
        out[i].setX(xData[xIdx]);
        out[i].setY(yData[yIdx]);
        xIdx = (xIdx + 1) % X.capacity();
        yIdx = (yIdx + 1) % Y.capacity();
      }
    }
  }
}

/**
 * @brief Updates the range of the X-axis values.
 */
void Widgets::Plot::updateRange()
{
  // Obtain dataset information
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    const auto &yD = GET_DATASET(SerialStudio::DashboardPlot, m_index);
    if (yD.xAxisId > 0)
    {
      const auto &xD = UI::Dashboard::instance().datasets()[yD.xAxisId];
      m_minX = xD.pltMin;
      m_maxX = xD.pltMax;
    }

    else
    {
      m_minX = 0;
      m_maxX = UI::Dashboard::instance().points();
    }
  }

  // Update the plot
  Q_EMIT rangeChanged();
}

/**
 * @brief Calculates the auto-scale range for both X and Y axes of the plot.
 *
 * This function determines the minimum and maximum values for the X and Y axes
 * of the plot based on the associated dataset. If the X-axis data source is set
 * to a specific dataset, its range is computed; otherwise, the range defaults
 * to `[0, points]`. For the Y-axis, the range is always determined from the
 * dataset values.
 *
 * @note The function emits the `rangeChanged()` signal if either the X or Y
 *       range is updated.
 */
void Widgets::Plot::calculateAutoScaleRange()
{
  // Validate that the dataset exists
  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
    return;

  // Initialize parameters
  bool xChanged = false;
  bool yChanged = false;

  // Obtain scale range for Y-axis
  const auto &dy = GET_DATASET(SerialStudio::DashboardPlot, m_index);
  yChanged = computeMinMaxValues(m_minY, m_maxY, dy, true,
                                 [](const QPointF &p) { return p.y(); });

  // Obtain range scale for X-axis
  if (SerialStudio::activated())
  {
    if (UI::Dashboard::instance().datasets().contains(dy.xAxisId))
    {
      const auto &dx = UI::Dashboard::instance().datasets()[dy.xAxisId];
      xChanged = computeMinMaxValues(m_minX, m_maxX, dx, false,
                                     [](const QPointF &p) { return p.x(); });
    }
  }

  // X-axis data source set to samples, use [0, points] as range
  else
  {
    const auto points = UI::Dashboard::instance().points();

    if (m_minX != 0 || m_maxX != points)
    {
      m_minX = 0;
      m_maxX = points;
      xChanged = true;
    }
  }

  // Update user interface
  if (xChanged || yChanged)
    Q_EMIT rangeChanged();
}

/**
 * @brief Computes the minimum and maximum values for a given axis of the plot.
 *
 * This templated function calculates the minimum and maximum values for a plot
 * axis (either X or Y) using the provided dataset and an extractor function. If
 * the dataset has no valid range or is empty, a fallback range `[0, 1]` or an
 * adjusted range is applied.
 *
 * @tparam Extractor A callable object (e.g., lambda) used to extract values
 *                   from data points.
 *
 * @param pltMin Reference to the variable storing the minimum value.
 * @param pltMax Reference to the variable storing the maximum value.
 * @param dataset The dataset to compute the range from.
 * @param extractor A function used to extract axis-specific values (e.g.,
 *                  `p.y()` or `p.x()`).
 *
 * @return `true` if the computed range differs from the previous range, `false`
 * otherwise.
 *
 * @note If the dataset has the same minimum and maximum values, the range is
 * adjusted to provide a better display.
 */
template<typename Extractor>
bool Widgets::Plot::computeMinMaxValues(double &min, double &max,
                                        const JSON::Dataset &dataset,
                                        const bool addPadding,
                                        Extractor extractor)
{
  // Store previous values
  bool ok = true;
  const auto prevMinY = min;
  const auto prevMaxY = max;

  // If the data is empty, set the range to 0-1
  if (m_data.isEmpty())
  {
    min = 0;
    max = 1;
  }

  // Obtain min/max values from datasets
  else
  {
    ok &= !qFuzzyCompare(dataset.pltMin, dataset.pltMax);
    if (ok)
    {
      min = qMin(dataset.pltMin, dataset.pltMax);
      max = qMax(dataset.pltMin, dataset.pltMax);
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Get minimum and maximum values from data
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::lowest();

    // Loop through the plot data and update the min and max
    for (auto i = 0; i < m_data.size(); ++i)
    {
      min = qMin(min, extractor(m_data[i]));
      max = qMax(max, extractor(m_data[i]));
    }

    // If min and max are the same, adjust the range
    if (qFuzzyCompare(min, max))
    {
      if (qFuzzyIsNull(min))
      {
        min = -1;
        max = 1;
      }

      else
      {
        double absValue = qAbs(min);
        min = min - absValue * 0.1;
        max = max + absValue * 0.1;
      }
    }

    // If the min and max are not the same, set the range to 10% more
    else if (addPadding)
    {
      double range = max - min;
      min -= range * 0.1;
      max += range * 0.1;
    }

    // Round to integer numbers
    max = std::ceil(max);
    min = std::floor(min);
    if (qFuzzyCompare(max, min) && addPadding)
    {
      min -= 1;
      max += 1;
    }
  }

  // Update user interface if required
  if (qFuzzyCompare(prevMinY, min) || qFuzzyCompare(prevMaxY, max))
    return true;

  // Data not changed
  return false;
}
