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
#include "UI/Widgets/Compass.h"

/**
 * @brief Constructs a Compass widget.
 * @param index The index of the compass in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::Compass::Compass(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_value(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Compass::updateData);
}

/**
 * @brief Returns the current value of the compass.
 * @return The current value of the compass.
 */
double Widgets::Compass::value() const
{
  return m_value;
}

/**
 * @brief Returns the text representation of the compass value.
 * @return The text representation of the compass value.
 */
QString Widgets::Compass::text() const
{
  return m_text;
}

/**
 * @brief Updates the compass data from the Dashboard.
 *
 * This method retrieves the latest data for this compass from the Dashboard
 * and updates the compass's value and text display accordingly.
 */
void Widgets::Compass::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardCompass, m_index))
  {
    const auto &dataset = GET_DATASET(SerialStudio::DashboardCompass, m_index);
    const auto value = dataset.numericValue;
    if (!qFuzzyCompare(value, m_value))
    {
      // Update values
      m_value = qMin(360.0, qMax(0.0, value));
      m_text = FMT_VAL(m_value, dataset);

      // Ensure that angle always has 3 characters to avoid jiggling
      const int deg = qCeil(m_value);
      if (deg < 10)
        m_text.prepend(QStringLiteral("00"));
      else if (deg < 100)
        m_text.prepend(QStringLiteral("0"));

      // Determine the direction based on the angle value
      QString direction;
      if ((m_value >= 0 && m_value < 22.5)
          || (m_value >= 337.5 && m_value <= 360))
        direction = tr("N") + " ";
      else if (m_value >= 22.5 && m_value < 67.5)
        direction = tr("NE");
      else if (m_value >= 67.5 && m_value < 112.5)
        direction = tr("E") + " ";
      else if (m_value >= 112.5 && m_value < 157.5)
        direction = tr("SE");
      else if (m_value >= 157.5 && m_value < 202.5)
        direction = tr("S") + " ";
      else if (m_value >= 202.5 && m_value < 247.5)
        direction = tr("SW");
      else if (m_value >= 247.5 && m_value < 292.5)
        direction = tr("W") + " ";
      else if (m_value >= 292.5 && m_value < 337.5)
        direction = tr("NW");

      // Append the direction to the text
      m_text += " " + direction;

      // Request a redraw of the item
      Q_EMIT updated();
    }
  }
}
