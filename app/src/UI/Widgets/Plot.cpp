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

#include "SIMD/SIMD.h"
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
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    const auto &yDataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);

    m_minY = qMin(yDataset.min(), yDataset.max());
    m_maxY = qMax(yDataset.min(), yDataset.max());

    const auto xAxisId = yDataset.xAxisId();
    if (UI::Dashboard::instance().datasets().contains(xAxisId))
    {
      const auto &xDataset = UI::Dashboard::instance().datasets()[xAxisId];
      m_xLabel = xDataset.title();
      if (!xDataset.units().isEmpty())
        m_xLabel += " (" + xDataset.units() + ")";
    }

    else
      m_xLabel = tr("Samples");

    m_yLabel = yDataset.title();
    if (!yDataset.units().isEmpty())
      m_yLabel += " (" + yDataset.units() + ")";

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Plot::updateData);
    connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this,
            &Plot::updateRange);

    calculateAutoScaleRange();
    updateRange();
  }
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
qreal Widgets::Plot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
qreal Widgets::Plot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
qreal Widgets::Plot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
qreal Widgets::Plot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
qreal Widgets::Plot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval with human-readable values.
 * @return The Y-axis tick interval.
 */
qreal Widgets::Plot::yTickInterval() const
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
void Widgets::Plot::draw(QLineSeries *series)
{
  if (series)
  {
    series->replace(m_data);
    calculateAutoScaleRange();
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the plot data from the Dashboard.
 */
void Widgets::Plot::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    // Get plotting data
    const auto &plotData = UI::Dashboard::instance().plotData(m_index);
    const auto X = plotData.x;
    const auto Y = plotData.y;

    // Resize series array if required
    if (m_data.count() != X->count())
      m_data.resize(X->count());

    // Convert data to a list of points
    int i = 0;
    for (auto x = X->begin(); x != X->end(); ++x)
    {
      if (Y->count() > i)
      {
        m_data[i] = QPointF(*x, Y->at(i));
        ++i;
      }

      else
        break;
    }
  }
}

/**
 * @brief Updates the range of the X-axis values.
 */
void Widgets::Plot::updateRange()
{
  // Clear memory
  m_data.clear();
  m_data.squeeze();
  m_data.resize(UI::Dashboard::instance().points() + 1);

  // Obtain dataset information
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    const auto &yD = GET_DATASET(SerialStudio::DashboardPlot, m_index);
    if (yD.xAxisId() > 0)
    {
      const auto &xD = UI::Dashboard::instance().datasets()[yD.xAxisId()];
      m_minX = xD.min();
      m_maxX = xD.max();
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
  // clang-format off
  const auto &yDataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);
  yChanged = computeMinMaxValues(m_minY, m_maxY, yDataset, true, [](const QPointF &p) { return p.y(); });
  // clang-format on

  // Obtain range scale for X-axis
  // clang-format off
  if (UI::Dashboard::instance().datasets().contains(yDataset.xAxisId()))
  {
    const auto &xDataset = UI::Dashboard::instance().datasets()[yDataset.xAxisId()];
    xChanged = computeMinMaxValues(m_minX, m_maxX, xDataset, false, [](const QPointF &p) { return p.x(); });
  }
  // clang-format on

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
 * @param min Reference to the variable storing the minimum value.
 * @param max Reference to the variable storing the maximum value.
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
bool Widgets::Plot::computeMinMaxValues(qreal &min, qreal &max,
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
    ok &= !qFuzzyCompare(dataset.min(), dataset.max());
    if (ok)
    {
      min = qMin(dataset.min(), dataset.max());
      max = qMax(dataset.min(), dataset.max());
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Initialize values to ensure that min/max are set
    min = std::numeric_limits<qreal>::max();
    max = std::numeric_limits<qreal>::lowest();

    // Loop through the plot data and update the min and max
    min = SIMD::findMin(m_data, extractor);
    max = SIMD::findMax(m_data, extractor);

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
        min = max + absValue * 0.1;
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
