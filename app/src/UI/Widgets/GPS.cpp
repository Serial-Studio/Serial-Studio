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
  // Widget not enabled, do nothing
  if (!isEnabled())
    return;

  // Invalid index, abort update
  static const auto *dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->widgetCount(WC::DashboardGPS))
    return;

  // Get group reference
  const auto &group = dash->getGroupWidget(WC::DashboardGPS, m_index);

  // Get latitiude/longitude from datasets
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

  // Redraw widget if required
  if (!qFuzzyCompare(lat, m_latitude) || !qFuzzyCompare(lon, m_longitude)
      || !qFuzzyCompare(alt, m_altitude))
  {
    m_latitude = lat;
    m_altitude = alt;
    m_longitude = lon;
    Q_EMIT updated();
  }
}
