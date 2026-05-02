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

#include "UI/Widgets/Compass.h"

#include "DSP.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Compass widget.
 * @param index The index of the compass in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Compass::Compass(const int index, QQuickItem* parent)
  : QQuickItem(parent), m_index(index), m_value(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Compass::updateData);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current value of the compass.
 * @return The current value of the compass.
 */
double Widgets::Compass::value() const noexcept
{
  return m_value;
}

/**
 * @brief Returns the text representation of the compass value.
 * @return The text representation of the compass value.
 */
QString Widgets::Compass::text() const noexcept
{
  return m_text;
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the compass data from the Dashboard.
 *
 * This method retrieves the latest data for this compass from the Dashboard
 * and updates the compass's value and text display accordingly.
 */
void Widgets::Compass::updateData()
{
  // Validate widget state and fetch latest compass heading
  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index))
    return;

  const auto& dataset = GET_DATASET(SerialStudio::DashboardCompass, m_index);
  const auto value    = dataset.numericValue;
  if (!DSP::notEqual(value, m_value))
    return;

  // Clamp and format the compass value
  m_value = qMin(360.0, qMax(0.0, value));
  m_text  = FMT_VAL(m_value, dataset);

  // Pad angle to 3 characters to avoid UI jiggling
  const int deg = qCeil(m_value);
  if (deg < 10)
    m_text.prepend(QStringLiteral("  "));
  else if (deg < 100)
    m_text.prepend(QStringLiteral(" "));

  // Append direction and notify UI
  m_text += QStringLiteral(" ") + cardinalDirection(m_value);
  Q_EMIT updated();
}

/**
 * @brief Maps an angle in degrees to a cardinal/intercardinal label.
 */
QString Widgets::Compass::cardinalDirection(double angle) const
{
  if ((angle >= 0 && angle < 22.5) || (angle >= 337.5 && angle <= 360))
    return tr("N") + " ";

  if (angle < 67.5)
    return tr("NE");

  if (angle < 112.5)
    return tr("E") + " ";

  if (angle < 157.5)
    return tr("SE");

  if (angle < 202.5)
    return tr("S") + " ";

  if (angle < 247.5)
    return tr("SW");

  if (angle < 292.5)
    return tr("W") + " ";

  if (angle < 337.5)
    return tr("NW");

  return QString();
}
