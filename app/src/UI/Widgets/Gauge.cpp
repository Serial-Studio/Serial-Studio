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
#include "UI/Widgets/Gauge.h"

/**
 * @brief Constructs a Gauge widget.
 * @param index The index of the gauge in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Gauge::Gauge(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(0)
  , m_minValue(0)
  , m_maxValue(100)
  , m_alarmValue(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);

    m_units = dataset.units();
    m_alarmValue = dataset.alarm();
    m_minValue = qMin(dataset.min(), dataset.max());
    m_maxValue = qMax(dataset.min(), dataset.max());

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Gauge::updateData);
  }
}

/**
 * @brief Returns the measurement units of the dataset.
 */
const QString &Widgets::Gauge::units() const
{
  return m_units;
}

/**
 * @brief Returns the current value of the gauge.
 * @return The current value of the gauge.
 */
qreal Widgets::Gauge::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum value of the gauge scale.
 * @return The minimum value of the gauge scale.
 */
qreal Widgets::Gauge::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value of the gauge scale.
 * @return The maximum value of the gauge scale.
 */
qreal Widgets::Gauge::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the alarm level of the gauge.
 * @return The alarm level of the gauge.
 */
qreal Widgets::Gauge::alarmValue() const
{
  return m_alarmValue;
}

/**
 * @brief Updates the gauge data from the Dashboard.
 *
 * This method retrieves the latest data for this gauge from the Dashboard
 * and updates the gauge's value and text display accordingly.
 */
void Widgets::Gauge::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGauge, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardGauge, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.value().toDouble()));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
