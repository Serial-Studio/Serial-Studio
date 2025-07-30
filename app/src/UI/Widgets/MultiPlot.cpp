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

    // Populate data from datasets
    for (size_t i = 0; i < group.datasets.size(); ++i)
    {
      const auto &dataset = group.datasets[i];

      m_drawOrders.append(i);
      m_visibleCurves.append(true);
      m_labels.append(dataset.title);
      m_minY = qMin(m_minY, qMin(dataset.min, dataset.max));
      m_maxY = qMax(m_maxY, qMax(dataset.min, dataset.max));
    }

    // Obtain group title
    m_yLabel = group.title;

    // Resize data container to fit curves
    m_data.resize(group.datasets.size());
    for (size_t i = 0; i < group.datasets.size(); ++i)
      m_data[i].resize(UI::Dashboard::instance().points());

    // Connect to the dashboard signals
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
double Widgets::MultiPlot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
double Widgets::MultiPlot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
double Widgets::MultiPlot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
double Widgets::MultiPlot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
double Widgets::MultiPlot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval with human-readable values.
 * @return The Y-axis tick interval.
 */
double Widgets::MultiPlot::yTickInterval() const
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
 * @brief Returns the labels of the datasets.
 * @return The labels of the datasets.
 */
const QStringList &Widgets::MultiPlot::labels() const
{
  return m_labels;
}

/**
 * @brief Returns the visibility state of all curves.
 *
 * Provides a reference to the internal list indicating which curves are
 * currently visible.
 *
 * @return Reference to a QList of booleans representing curve visibility.
 */
const QList<bool> &Widgets::MultiPlot::visibleCurves() const
{
  return m_visibleCurves;
}

/**
 * @brief Draws the data on the given QLineSeries.
 * @param series The QXYSeries to draw the data on.
 * @param index The index of the dataset to draw.
 */
void Widgets::MultiPlot::draw(QXYSeries *series, const int index)
{
  if (series && index >= 0 && index < count() && m_visibleCurves[index])
  {
    series->replace(m_data[index]);

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    const int drawOrder = m_drawOrders.indexOf(index);
    if (series->drawOrder() != drawOrder)
      series->setDrawOrder(drawOrder);
#endif

    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the data of the multiplot.
 */
void Widgets::MultiPlot::updateData()
{
  // Stop if widget is disabled
  if (!isEnabled())
    return;

  // Only obtain data if widget data is still valid
  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    // Fetch multiplot source data (shared X axis, multiple Y series)
    const auto &data = UI::Dashboard::instance().multiplotData(m_index);
    const auto &X = *data.x;

    // Ensure output container has one QVector<QPointF> per series
    const qsizetype plotCount = data.y.size();
    m_data.resize(plotCount);

    // Get raw X pointer and state
    const double *xData = X.raw();
    std::size_t xIdx = X.frontIndex();
    const std::size_t xCap = X.capacity();

    // Populate data for each plot
    for (qsizetype i = 0; i < plotCount; ++i)
    {
      // Skip if curve is not visible
      if (!m_visibleCurves[i])
        continue;

      // Get raw pointer to dashboard data
      const auto &Y = data.y[i];
      const double *yData = Y.raw();

      // Get queue states for faster iteration
      std::size_t yIdx = Y.frontIndex();
      const std::size_t yCap = Y.capacity();

      // Obtain the length of the shortest axis
      const qsizetype count = std::min(X.size(), Y.size());

      // Resize plot data if needed
      QVector<QPointF> &outSeries = m_data[i];
      if (outSeries.size() != count)
        outSeries.resize(count);

      // Keep local copies to avoid resetting each plot
      std::size_t xi = xIdx;
      std::size_t yi = yIdx;

      // Update plot data points, avoid queue operations overhead
      QPointF *dst = outSeries.data();
      for (qsizetype j = 0; j < count; ++j)
      {
        dst[j].setX(xData[xi]);
        dst[j].setY(yData[yi]);
        xi = (xi + 1) % xCap;
        yi = (yi + 1) % yCap;
      }
    }

    // Calculate auto scale range
    calculateAutoScaleRange();
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

  // Clear dataset curves
  for (auto &dataset : m_data)
  {
    dataset.clear();
    dataset.squeeze();
  }

  // Clear the data
  m_data.clear();
  m_data.squeeze();

  // Get data
  auto data = UI::Dashboard::instance().multiplotData(m_index);

  // Get the multiplot group and loop through each dataset
  const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);
  for (size_t i = 0; i < group.datasets.size(); ++i)
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

    int index = 0;
    for (const auto &dataset : group.datasets)
    {
      ok &= !qFuzzyCompare(dataset.min, dataset.max);
      if (ok && m_visibleCurves[index])
      {
        m_minY = qMin(m_minY, qMin(dataset.min, dataset.max));
        m_maxY = qMax(m_maxY, qMax(dataset.min, dataset.max));
      }

      else
      {
        ok = false;
        break;
      }

      ++index;
    }
  }

  // Set the min and max to the lowest and highest values
  if (!ok)
  {
    // Initialize values to ensure that min/max are set
    m_minY = std::numeric_limits<double>::max();
    m_maxY = std::numeric_limits<double>::lowest();

    // Loop through each dataset and find the min and max values
    int index = 0;
    for (const auto &curve : std::as_const(m_data))
    {
      if (m_visibleCurves[index])
      {
        for (auto i = 0; i < curve.count(); ++i)
        {
          m_minY = qMin(m_minY, curve[i].y());
          m_maxY = qMax(m_maxY, curve[i].y());
        }
      }

      ++index;
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

    // Expand range symmetrically around midY, with a 10% padding
    else
    {
      // Calculate center and half-range
      const double midY = (m_minY + m_maxY) / 2.0;
      const double halfRange = (m_maxY - m_minY) / 2.0;

      // Expand range symmetrically around midY, with a 10% padding
      double paddedRange = halfRange * 1.1;
      if (qFuzzyIsNull(paddedRange))
        paddedRange = 1;

      m_minY = std::floor(midY - paddedRange);
      m_maxY = std::ceil(midY + paddedRange);

      // Safety check to avoid zero-range
      if (qFuzzyCompare(m_minY, m_maxY))
      {
        m_minY -= 1;
        m_maxY += 1;
      }
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

/**
 * @brief Modifies the visibility state of a specific curve in the multi-plot.
 *
 * Updates the visibility flag for the curve at the given index. If the index is
 * valid, the internal visibility list is updated, the autoscale range is
 * recalculated, and the curvesChanged() signal is emitted.
 *
 * @param index   Index of the curve to modify.
 * @param visible True to show the curve, false to hide it.
 */
void Widgets::MultiPlot::modifyCurveVisibility(const int index,
                                               const bool visible)
{
  if (index >= 0 && index < m_visibleCurves.count())
  {
    m_visibleCurves[index] = visible;
    if (visible)
    {
      m_drawOrders.removeAll(index);
      m_drawOrders.append(index);
    }

    Q_EMIT curvesChanged();
  }
}

/**
 * @brief Updates the theme of the multiplot.
 */
void Widgets::MultiPlot::onThemeChanged()
{
  const auto &colors = Misc::ThemeManager::instance().widgetColors();
  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);

    m_colors.clear();
    m_colors.resize(group.datasets.size());
    for (size_t i = 0; i < group.datasets.size(); ++i)
    {
      const auto &dataset = group.datasets[i];
      const auto index = dataset.index - 1;
      const auto color = colors.count() > index
                             ? colors.at(index)
                             : colors.at(index % colors.count());

      m_colors[i] = color.name();
    }

    Q_EMIT themeChanged();
    Q_EMIT curvesChanged();
  }
}
