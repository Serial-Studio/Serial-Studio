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
#include "UI/Widgets/Bar.h"

/**
 * @brief Constructs a Bar widget.
 * @param index The index of the bar in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Bar::Bar(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(0)
  , m_minValue(0)
  , m_maxValue(100)
  , m_alarmValue(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);

    m_units = dataset.units();
    m_alarmValue = dataset.alarm();
    m_minValue = qMin(dataset.min(), dataset.max());
    m_maxValue = qMax(dataset.min(), dataset.max());

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Bar::updateData);
  }
}

/**
 * @brief Returns the measurement units of the dataset.
 */
const QString &Widgets::Bar::units() const
{
  return m_units;
}

/**
 * @brief Returns the current value of the bar.
 * @return The current value of the bar.
 */
qreal Widgets::Bar::value() const
{
  return m_value;
}

/**
 * @brief Returns the minimum value of the bar scale.
 * @return The minimum value of the bar scale.
 */
qreal Widgets::Bar::minValue() const
{
  return m_minValue;
}

/**
 * @brief Returns the maximum value of the bar scale.
 * @return The maximum value of the bar scale.
 */
qreal Widgets::Bar::maxValue() const
{
  return m_maxValue;
}

/**
 * @brief Returns the alarm level of the bar.
 * @return The alarm level of the bar.
 */
qreal Widgets::Bar::alarmValue() const
{
  return m_alarmValue;
}

/**
 * @brief Calculates the fractional value of the bar's current level.
 *
 * This function computes the relative position of the current value within
 * the range defined by minValue() and maxValue(). It is used to determine
 * the visual height of the bar in the UI.
 */
qreal Widgets::Bar::fractionalValue() const
{
  qreal min = qMin(minValue(), maxValue());
  qreal max = qMax(minValue(), maxValue());
  const qreal range = max - min;
  if (qFuzzyIsNull(range))
    return 0.0;

  const qreal level = value() - min;
  return qBound(0.0, level / range, 1.0);
}

/**
 * @brief Calculates the fractional value of the bar's alarm level.
 *
 * This function computes the relative position of the alarm level within
 * the range defined by minValue() and maxValue(). It is used to determine
 * the position where the bar's color should change to indicate an alarm state.
 */
qreal Widgets::Bar::alarmFractionalValue() const
{
  const qreal range = maxValue() - minValue();
  if (qFuzzyIsNull(range))
    return 0.0;

  const qreal clampedAlarmLevel = qBound(minValue(), m_alarmValue, maxValue());
  const qreal alarmPosition = clampedAlarmLevel - minValue();
  return qBound(0.0, alarmPosition / range, 1.0);
}

/**
 * @brief Updates the bar data from the Dashboard.
 *
 * This method retrieves the latest data for this bar from the Dashboard
 * and updates the bar's value and text display accordingly.
 */
void Widgets::Bar::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);
    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.value().toDouble()));
    if (!qFuzzyCompare(value, m_value))
    {
      m_value = value;
      Q_EMIT updated();
    }
  }
}
