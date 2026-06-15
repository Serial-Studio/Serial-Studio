/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "UI/Widgets/Compass.h"

#include <cmath>

#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Compass widget.
 */
Widgets::Compass::Compass(const int index, QQuickItem* parent)
  : QQuickItem(parent), m_index(index), m_decimalPoints(-1), m_value(0.0)
{
  m_cardinal = cardinalDirection(0.0);

  if (VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardCompass, m_index);
    m_title             = dataset.title;
    m_units             = dataset.units;
    m_displayFormat     = dataset.displayFormat;
    m_decimalPoints     = dataset.decimalPoints;

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Compass::updateData);
  }
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compass has no alarm semantics; reported for QML symmetry with Gauge.
 */
bool Widgets::Compass::alarmsDefined() const noexcept
{
  return false;
}

/**
 * @brief Compass has no alarm semantics; reported for QML symmetry with Gauge.
 */
bool Widgets::Compass::alarmTriggered() const noexcept
{
  return false;
}

//--------------------------------------------------------------------------------------------------
// Value getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current heading in degrees (0-360, wrap-clamped).
 */
double Widgets::Compass::value() const noexcept
{
  return m_value;
}

/**
 * @brief Returns the cardinal/intercardinal label for the current heading.
 */
const QString& Widgets::Compass::cardinal() const noexcept
{
  return m_cardinal;
}

/**
 * @brief Returns the dataset title associated with the widget.
 */
const QString& Widgets::Compass::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the measurement units associated with the dataset (empty = no suffix).
 */
const QString& Widgets::Compass::units() const noexcept
{
  return m_units;
}

/**
 * @brief Returns the value display format ("" = auto; "%.<n>f" or preset slugs accepted).
 */
const QString& Widgets::Compass::displayFormat() const noexcept
{
  return m_displayFormat;
}

/**
 * @brief Returns the fixed display decimal-place count, or -1 for range-driven auto.
 */
int Widgets::Compass::decimalPoints() const noexcept
{
  return m_decimalPoints;
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the compass heading from the dashboard source.
 */
void Widgets::Compass::updateData()
{
  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index))
    return;

  const auto& dataset = GET_DATASET(SerialStudio::DashboardCompass, m_index);
  if (!std::isfinite(dataset.numericValue))
    return;

  double v = std::fmod(dataset.numericValue, 360.0);
  if (v < 0.0)
    v += 360.0;

  const auto card = cardinalDirection(v);
  if (!DSP::notEqual(v, m_value) && card == m_cardinal)
    return;

  m_value    = v;
  m_cardinal = card;
  Q_EMIT updated();
}

//--------------------------------------------------------------------------------------------------
// Cardinal resolver
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps an angle in degrees to a 16-wind cardinal/intercardinal label.
 */
QString Widgets::Compass::cardinalDirection(double angle) const
{
  if ((angle >= 0 && angle < 22.5) || (angle >= 337.5 && angle <= 360))
    return tr("N");

  if (angle < 67.5)
    return tr("NE");

  if (angle < 112.5)
    return tr("E");

  if (angle < 157.5)
    return tr("SE");

  if (angle < 202.5)
    return tr("S");

  if (angle < 247.5)
    return tr("SW");

  if (angle < 292.5)
    return tr("W");

  if (angle < 337.5)
    return tr("NW");

  return tr("N");
}
