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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/LEDPanel.h"

/**
 * @brief Constructs an LEDPanel widget.
 * @param index The index of the LED panel in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::LEDPanel::LEDPanel(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardLED, m_index);
    m_states.resize(group.datasetCount());
    m_titles.resize(group.datasetCount());
    m_colors.resize(group.datasetCount());
    m_alarms.resize(group.datasetCount());

    for (int i = 0; i < group.datasetCount(); ++i)
    {
      m_states[i] = false;
      m_alarms[i] = false;
      m_titles[i] = group.getDataset(i).title();
    }

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &LEDPanel::updateData);

    m_alarmTimer.setInterval(250);
    m_alarmTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_alarmTimer, &QTimer::timeout, this,
            &Widgets::LEDPanel::onAlarmTimeout);
    m_alarmTimer.start();

    onThemeChanged();
    connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
            this, &Widgets::LEDPanel::onThemeChanged);
  }
}

/**
 * @brief Returns the number of LEDs in the panel.
 * @return An integer number with the number/count of LEDs in the panel.
 */
int Widgets::LEDPanel::count() const
{
  return m_titles.count();
}

/**
 * @brief Returns the alarm states of the LEDs in the panel.
 * @return A vector of boolean values representing the alarm states of the LEDs.
 */
const QList<bool> &Widgets::LEDPanel::alarms() const
{
  return m_alarms;
}

/**
 * @brief Returns the states of the LEDs in the panel.
 * @return A vector of boolean values representing the states of the LEDs.
 */
const QList<bool> &Widgets::LEDPanel::states() const
{
  return m_states;
}

/**
 * @brief Returns the colors of the LEDs in the panel.
 * @return A vector of strings representing the activated colors of the LEDs.
 */
const QStringList &Widgets::LEDPanel::colors() const
{
  return m_colors;
}

/**
 * @brief Returns the titles of the LEDs in the panel.
 * @return A vector of strings representing the titles of the LEDs.
 */
const QStringList &Widgets::LEDPanel::titles() const
{
  return m_titles;
}

/**
 * @brief Updates the LED panel data from the Dashboard.
 *
 * This method retrieves the latest data for this LED panel from the Dashboard
 * and updates the LEDs' states and titles accordingly.
 */
void Widgets::LEDPanel::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index))
  {
    // Get the LED group and update the LED states
    bool changed = false;
    const auto &group = GET_GROUP(SerialStudio::DashboardLED, m_index);
    for (int i = 0; i < group.datasetCount(); ++i)
    {
      // Get the dataset and its values
      const auto &dataset = group.getDataset(i);
      const auto value = dataset.value().toDouble();
      const auto alarmValue = dataset.alarm();

      // Obtain the LED state
      const bool enabled = (value >= dataset.ledHigh());
      const bool alarm = (alarmValue != 0 && value >= alarmValue);

      // Update the alarm state
      if (m_alarms[i] != alarm)
      {
        changed = true;
        m_alarms[i] = alarm;
      }

      // Update the LED state
      if (!alarm && m_states[i] != enabled)
      {
        changed = true;
        m_states[i] = enabled;
      }
    }

    // Redraw the widget
    if (changed)
      Q_EMIT updated();
  }
}

/**
 * @brief Toggles the LED enabled state when the alarm blinker timer expires
 */
void Widgets::LEDPanel::onAlarmTimeout()
{
  bool changed = false;
  for (int i = 0; i < m_alarms.count(); ++i)
  {
    if (m_alarms[i])
    {
      changed = true;
      m_states[i] = !m_states[i];
    }
  }

  if (changed)
    Q_EMIT updated();
}

/**
 * @brief Updates the colors for each dataset in the widget based on the
 *        colorscheme defined by the application's currently loaded theme.
 */
void Widgets::LEDPanel::onThemeChanged()
{
  // clang-format off
  const auto colors = Misc::ThemeManager::instance().colors()["widget_colors"].toArray();
  // clang-format on

  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardLED, m_index);

    m_colors.clear();
    m_colors.resize(group.datasetCount());
    for (int i = 0; i < group.datasetCount(); ++i)
    {
      const auto &dataset = group.getDataset(i);
      const auto index = dataset.index() - 1;
      const auto color = colors.count() > index
                             ? colors.at(index).toString()
                             : colors.at(index % colors.count()).toString();
      m_colors[i] = color;
    }

    Q_EMIT themeChanged();
  }
}
