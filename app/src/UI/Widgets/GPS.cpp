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
double Widgets::GPS::altitude() const
{
  return m_altitude;
}

/**
 * Returns the latest GPS latitude value extracted from the JSON data.
 */
double Widgets::GPS::latitude() const
{
  return m_latitude;
}

/**
 * Returns the latest GPS longitude value extracted from the JSON data.
 */
double Widgets::GPS::longitude() const
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

    double lat = 0, lon = 0, alt = 0;
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
