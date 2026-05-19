/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "UI/Widgets/Bar.h"

#include "DataModel/NotificationCenter.h"
#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Bar widget.
 */
Widgets::Bar::Bar(const int index, QQuickItem* parent, bool autoInitFromBarDataset)
  : QQuickItem(parent)
  , m_index(index)
  , m_displayTickCount(5)
  , m_value(0.0)
  , m_minValue(0.0)
  , m_maxValue(0.0)
  , m_alarmLow(std::nan(""))
  , m_alarmHigh(std::nan(""))
  , m_alarmsDefined(false)
  , m_alarmInitialized(false)
  , m_alarmActive(false)
{
  if (autoInitFromBarDataset && VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);

    m_title            = dataset.title;
    m_units            = dataset.units;
    m_displayFormat    = dataset.displayFormat;
    m_displayTickCount = dataset.displayTickCount;
    m_minValue         = qMin(dataset.wgtMin, dataset.wgtMax);
    m_maxValue         = qMax(dataset.wgtMin, dataset.wgtMax);
    m_alarmLow         = qMin(dataset.alarmLow, dataset.alarmHigh);
    m_alarmHigh        = qMax(dataset.alarmLow, dataset.alarmHigh);
    m_alarmLow         = qBound(m_minValue, m_alarmLow, m_maxValue);
    m_alarmHigh        = qBound(m_minValue, m_alarmHigh, m_maxValue);

    if (dataset.alarmEnabled) {
      if (m_alarmHigh == m_alarmLow)
        m_alarmLow = m_minValue;

      m_alarmsDefined = (m_alarmLow > m_minValue && m_alarmLow < m_maxValue)
                     || (m_alarmHigh < m_maxValue && m_alarmHigh > m_minValue);
    }

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Bar::updateData);
  }
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if the dataset options indicate that a valid alarm range is available.
 */
bool Widgets::Bar::alarmsDefined() const noexcept
{
  return m_alarmsDefined;
}

/**
 * @brief Checks if the current value is in an alarm state.
 */
bool Widgets::Bar::alarmTriggered() const noexcept
{
  // Check value against low and high alarm thresholds
  if (m_alarmsDefined) {
    if (!std::isnan(m_alarmLow) && m_alarmLow > m_minValue) {
      if (m_value <= m_alarmLow)
        return true;
    }

    if (!std::isnan(m_alarmHigh) && m_alarmHigh < m_maxValue) {
      if (m_value >= m_alarmHigh)
        return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Value getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the dataset title associated with the widget.
 */
const QString& Widgets::Bar::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the measurement units associated with the dataset.
 */
const QString& Widgets::Bar::units() const noexcept
{
  return m_units;
}

/**
 * @brief Returns the preferred major-tick count (0 = use widget's adaptive heuristic).
 */
int Widgets::Bar::displayTickCount() const noexcept
{
  return m_displayTickCount;
}

/**
 * @brief Returns the tick/value display format ("" = auto; "%.<n>f" or preset slugs accepted).
 */
const QString& Widgets::Bar::displayFormat() const noexcept
{
  return m_displayFormat;
}

/**
 * @brief Returns the current numeric value of the dataset.
 */
double Widgets::Bar::value() const noexcept
{
  return m_value;
}

/**
 * @brief Returns the minimum scale value for the bar.
 */
double Widgets::Bar::minValue() const noexcept
{
  return m_minValue;
}

/**
 * @brief Returns the maximum scale value for the bar.
 */
double Widgets::Bar::maxValue() const noexcept
{
  return m_maxValue;
}

/**
 * @brief Returns the configured low alarm threshold.
 */
double Widgets::Bar::alarmLow() const noexcept
{
  return m_alarmLow;
}

/**
 * @brief Returns the configured high alarm threshold.
 */
double Widgets::Bar::alarmHigh() const noexcept
{
  return m_alarmHigh;
}

//--------------------------------------------------------------------------------------------------
// Normalized position getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the normalized fractional position of the current value.
 */
double Widgets::Bar::normalizedValue() const noexcept
{
  return computeFractional(m_value);
}

/**
 * @brief Returns the normalized fractional position of the low alarm threshold.
 */
double Widgets::Bar::normalizedAlarmLow() const noexcept
{
  return computeFractional(m_alarmLow);
}

/**
 * @brief Returns the normalized fractional position of the high alarm threshold.
 */
double Widgets::Bar::normalizedAlarmHigh() const noexcept
{
  return computeFractional(m_alarmHigh);
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the current dataset value from the dashboard source.
 */
void Widgets::Bar::updateData()
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);
    if (!std::isfinite(dataset.numericValue))
      return;

    auto value = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    if (DSP::notEqual(value, m_value)) {
      m_value = value;
      notifyOnAlarmEdge();
      if (isEnabled())
        Q_EMIT updated();
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Alarm notification routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Posts a Critical notification on the rising edge of alarmTriggered().
 */
void Widgets::Bar::notifyOnAlarmEdge()
{
  const bool nowActive = alarmTriggered();

  // Suppress the first edge so an already-out-of-range value at connect doesn't fire
  if (!m_alarmInitialized) {
    m_alarmInitialized = true;
    m_alarmActive      = nowActive;
    return;
  }

  if (nowActive == m_alarmActive)
    return;

  m_alarmActive = nowActive;
  if (!nowActive)
    return;

  const QString unit = m_units.isEmpty() ? QString() : QStringLiteral(" ") + m_units;
  const QString name = m_title.isEmpty() ? tr("Alarm") : m_title;

  QString subtitle;
  if (!std::isnan(m_alarmHigh) && m_alarmHigh < m_maxValue && m_value >= m_alarmHigh)
    subtitle = tr("Value %1%2 reached the high alarm %3%2")
                 .arg(QString::number(m_value), unit, QString::number(m_alarmHigh));
  else if (!std::isnan(m_alarmLow) && m_alarmLow > m_minValue && m_value <= m_alarmLow)
    subtitle = tr("Value %1%2 reached the low alarm %3%2")
                 .arg(QString::number(m_value), unit, QString::number(m_alarmLow));

  DataModel::NotificationCenter::instance().post(
    DataModel::NotificationCenter::Critical, tr("Alarms"), name, subtitle);
}
