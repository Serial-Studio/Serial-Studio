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
#include "Misc/ThemeManager.h"

/**
 * Generates the user interface elements & layout
 */
Widgets::GPS::GPS(const int index)
  : m_index(index)
  , m_altitude(0)
  , m_latitude(0)
  , m_longitude(0)
{
  // Get pointers to serial studio modules
  auto dash = &UI::Dashboard::instance();

  // Invalid index, abort initialization
  if (m_index < 0 || m_index >= dash->gpsCount())
    return;

  // Set visual style
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::GPS::onThemeChanged);

  // React to Qt signals
  connect(dash, SIGNAL(updated()), this, SLOT(updateData()),
          Qt::QueuedConnection);
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
  auto dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->gpsCount())
    return;

  // Get group reference
  auto group = dash->getGPS(m_index);

  // Get latitiude/longitude from datasets
  m_altitude = 0, m_latitude = 0;
  m_longitude = 0;
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    auto dataset = group.getDataset(i);
    if (dataset.widget() == QStringLiteral("lat"))
      m_latitude = dataset.value().toDouble();
    else if (dataset.widget() == QStringLiteral("lon"))
      m_longitude = dataset.value().toDouble();
    else if (dataset.widget() == QStringLiteral("alt"))
      m_altitude = dataset.value().toDouble();
  }

  // Update the QML user interface with the new data
  Q_EMIT updated();
}

/**
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::GPS::onThemeChanged()
{
  auto theme = &Misc::ThemeManager::instance();
  QPalette palette;
  palette.setColor(QPalette::Base,
                   theme->getColor(QStringLiteral("widget_base")));
  palette.setColor(QPalette::Window,
                   theme->getColor(QStringLiteral("widget_window")));
  setPalette(palette);
  update();
}
