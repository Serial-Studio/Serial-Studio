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
    const auto &dataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);

    m_minY = dataset.min();
    m_maxY = dataset.max();
    m_yLabel = dataset.title();

    if (!dataset.units().isEmpty())
      m_yLabel += " (" + dataset.units() + ")";

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
    const auto &plotData = UI::Dashboard::instance().linearPlotValues();

    if (m_index >= 0 && plotData.count() > m_index)
    {
      const auto &values = plotData[m_index];
      if (m_data.count() != values.count())
        m_data.resize(UI::Dashboard::instance().points());

      for (int i = 0; i < values.count(); ++i)
        m_data[i] = QPointF(i, values[i]);
    }
  }
}

/**
 * @brief Updates the range of the X-axis values.
 */
void Widgets::Plot::updateRange()
{
  // Reserve the number of points in the dashboard
  m_data.clear();
  m_data.squeeze();
  m_data.resize(UI::Dashboard::instance().points() + 1);

  // Update x-axis
  m_minX = 0;
  m_maxX = UI::Dashboard::instance().points();

  // Update the plot
  Q_EMIT rangeChanged();
}

/**
 * @brief Calculates the auto-scale range for the Y-axis.
 */
void Widgets::Plot::calculateAutoScaleRange()
{
  // Store previous values
  bool ok = true;
  const auto prevMinY = m_minY;
  const auto prevMaxY = m_maxY;

  // If the data is empty, set the range to 0-1
  if (m_data.isEmpty())
  {
    m_minY = 0;
    m_maxY = 1;
  }

  // Obtain min/max values from datasets
  else if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);
    ok &= !qFuzzyCompare(dataset.min(), dataset.max());
    if (ok)
    {
      m_minY = qMin(m_minY, dataset.min());
      m_maxY = qMax(m_maxY, dataset.max());
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Initialize values to ensure that min/max are set
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();

    // Loop through the plot data and update the min and max
    for (const auto &point : m_data)
    {
      m_minY = qMin(m_minY, point.y());
      m_maxY = qMax(m_maxY, point.y());
    }

    // If min and max are the same, adjust the range
    if (qFuzzyCompare(m_minY, m_maxY))
    {
      if (qFuzzyIsNull(m_minY))
      {
        m_minY = -1;
        m_maxY = 1;
      }

      else
      {
        double absValue = qAbs(m_minY);
        m_minY = m_minY - absValue * 0.1;
        m_maxY = m_maxY + absValue * 0.1;
      }
    }

    // If the min and max are not the same, set the range to 10% more
    else
    {
      double range = m_maxY - m_minY;
      m_minY -= range * 0.1;
      m_maxY += range * 0.1;
    }

    // Round to integer numbers
    m_maxY = std::ceil(m_maxY);
    m_minY = std::floor(m_minY);
    if (qFuzzyCompare(m_maxY, m_minY))
    {
      m_minY -= 1;
      m_maxY += 1;
    }
  }

  // Update user interface if required
  if (qFuzzyCompare(prevMinY, m_minY) || qFuzzyCompare(prevMaxY, m_maxY))
    Q_EMIT rangeChanged();
}
