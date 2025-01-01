/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include "UI/Dashboard.h"
#include "UI/Widgets/GPS.h"

/**
 * @brief Constructs a GPS widget.
 * @param index The index of the GPS widget in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::GPS::GPS(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_altitude(0)
  , m_latitude(0)
  , m_longitude(0)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &Widgets::GPS::updateData);
}

/**
 * Returns the latest GPS altitude value extracted from the JSON data.
 */
qreal Widgets::GPS::altitude() const
{
  return m_altitude;
}

/**
 * Returns the latest GPS latitude value extracted from the JSON data.
 */
qreal Widgets::GPS::latitude() const
{
  return m_latitude;
}

/**
 * Returns the latest GPS longitude value extracted from the JSON data.
 */
qreal Widgets::GPS::longitude() const
{
  return m_longitude;
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to process the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::GPS::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardGPS, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardGPS, m_index);

    qreal lat = 0, lon = 0, alt = 0;
    for (int i = 0; i < group.datasetCount(); ++i)
    {
      const auto &dataset = group.getDataset(i);
      if (dataset.widget() == QStringLiteral("lat"))
        lat = dataset.value().toDouble();
      else if (dataset.widget() == QStringLiteral("lon"))
        lon = dataset.value().toDouble();
      else if (dataset.widget() == QStringLiteral("alt"))
        alt = dataset.value().toDouble();
    }

    if (!qFuzzyCompare(lat, m_latitude) || !qFuzzyCompare(lon, m_longitude)
        || !qFuzzyCompare(alt, m_altitude))
    {
      m_latitude = lat;
      m_altitude = alt;
      m_longitude = lon;
      Q_EMIT updated();
    }
  }
}
