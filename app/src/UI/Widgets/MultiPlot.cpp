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
#include "Misc/ThemeManager.h"
#include "UI/Widgets/MultiPlot.h"

/**
 * @brief Constructs a MultiPlot widget.
 * @param index The index of the multiplot in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::MultiPlot::MultiPlot(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
{
  // Obtain group information
  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    // Obtain min/max values from datasets
    const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();
    for (const auto &dataset : group.datasets())
    {
      m_minY = qMin(m_minY, dataset.min());
      m_maxY = qMax(m_maxY, dataset.max());
    }

    // Obtain group title
    m_yLabel = group.title();

    // Resize data container to fit curves
    m_data.resize(group.datasetCount());
    for (auto i = 0; i < group.datasetCount(); ++i)
      m_data[i].resize(UI::Dashboard::instance().points());

    // Connect to the dashboard signals to update the plot data and range
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &MultiPlot::updateData);
    connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this,
            &MultiPlot::updateRange);

    // Connect to the theme manager to update the curve colors
    onThemeChanged();
    connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
            this, &MultiPlot::onThemeChanged);

    // Update the range
    calculateAutoScaleRange();
    updateRange();
  }
}

/**
 * @brief Returns the number of datasets in the multiplot.
 * @return The number of datasets.
 */
int Widgets::MultiPlot::count() const
{
  return m_data.count();
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
qreal Widgets::MultiPlot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
qreal Widgets::MultiPlot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
qreal Widgets::MultiPlot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
qreal Widgets::MultiPlot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
qreal Widgets::MultiPlot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval with human-readable values.
 * @return The Y-axis tick interval.
 */
qreal Widgets::MultiPlot::yTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minY, m_maxY);
}

/**
 * @brief Returns the Y-axis label.
 * @return The Y-axis label.
 */
const QString &Widgets::MultiPlot::yLabel() const
{
  return m_yLabel;
}

/**
 * @brief Returns the colors of the datasets.
 * @return The colors of the datasets.
 */
const QStringList &Widgets::MultiPlot::colors() const
{
  return m_colors;
}

/**
 * @brief Draws the data on the given QLineSeries.
 * @param series The QLineSeries to draw the data on.
 * @param index The index of the dataset to draw.
 */
void Widgets::MultiPlot::draw(QLineSeries *series, const int index)
{
  if (series && index >= 0 && index < count())
  {
    if (index == 0)
      calculateAutoScaleRange();

    series->replace(m_data[index]);
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the data of the multiplot.
 */
void Widgets::MultiPlot::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    const auto &plotData = UI::Dashboard::instance().multiplotValues();
    if (m_index >= 0 && plotData.count() > m_index)
    {
      const auto &curves = plotData[m_index];
      for (int i = 0; i < curves.count(); ++i)
      {
        const auto &values = curves[i];
        if (m_data[i].count() != values.count())
          m_data[i].resize(values.count());

        for (int j = 0; j < values.count(); ++j)
          m_data[i][j] = QPointF(j, values[j]);
      }
    }
  }
}

/**
 * @brief Updates the range of the multiplot.
 */
void Widgets::MultiPlot::updateRange()
{
  // Get the dashboard instance and check if the index is valid
  if (!VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
    return;

  // Clear the data
  m_data.clear();
  m_data.squeeze();

  // Get the multiplot group and loop through each dataset
  const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    m_data.append(QVector<QPointF>());
    m_data.last().resize(UI::Dashboard::instance().points() + 1);
  }

  // Update X-axis range
  m_minX = 0;
  m_maxX = UI::Dashboard::instance().points();

  // Update the plot
  Q_EMIT rangeChanged();
}

/**
 * @brief Updates the theme of the multiplot.
 */
void Widgets::MultiPlot::onThemeChanged()
{
  // clang-format off
  const auto colors = Misc::ThemeManager::instance().colors()["widget_colors"].toArray();
  // clang-format on

  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);

    m_colors.clear();
    m_colors.resize(group.datasetCount());
    for (int i = 0; i < group.datasetCount(); ++i)
    {
      const auto &dataset = group.getDataset(i);
      const auto index = dataset.index() - 1;
      const auto color = colors.count() > index
                             ? colors.at(index).toString()
                             : colors.at(index % colors.count()).toString();

      m_colors[i] = color;
    }

    Q_EMIT themeChanged();
  }
}

/**
 * @brief Calculates the auto scale range of the multiplot.
 */
void Widgets::MultiPlot::calculateAutoScaleRange()
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
  else if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);

    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();

    for (const auto &dataset : group.datasets())
    {
      ok &= !qFuzzyCompare(dataset.min(), dataset.max());
      if (ok)
      {
        m_minY = qMin(m_minY, dataset.min());
        m_maxY = qMax(m_maxY, dataset.max());
      }

      else
        break;
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Initialize values to ensure that min/max are set
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();

    // Loop through each dataset and find the min and max values
    for (const auto &dataset : m_data)
    {
      for (const auto &point : dataset)
      {
        m_minY = qMin(m_minY, point.y());
        m_maxY = qMax(m_maxY, point.y());
      }
    }

    // If the min and max are the same, set the range to 0-1
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
