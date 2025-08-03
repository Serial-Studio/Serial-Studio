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
    m_states.resize(group.datasets.size());
    m_titles.resize(group.datasets.size());
    m_colors.resize(group.datasets.size());

    for (size_t i = 0; i < group.datasets.size(); ++i)
    {
      m_states[i] = false;
      m_titles[i] = group.datasets[i].title;
    }

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
            &LEDPanel::updateData);

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
    for (size_t i = 0; i < group.datasets.size(); ++i)
    {
      // Get the dataset and its values
      const auto &dataset = group.datasets[i];
      const auto value = dataset.numericValue;

      // Obtain the LED state
      const bool enabled = (value >= dataset.ledHigh);

      // Update the LED state
      if (m_states[i] != enabled)
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
 * @brief Updates the colors for each dataset in the widget based on the
 *        colorscheme defined by the application's currently loaded theme.
 */
void Widgets::LEDPanel::onThemeChanged()
{
  const auto &colors = Misc::ThemeManager::instance().widgetColors();
  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index))
  {
    const auto &group = GET_GROUP(SerialStudio::DashboardLED, m_index);

    m_colors.clear();
    m_colors.resize(group.datasets.size());
    for (size_t i = 0; i < group.datasets.size(); ++i)
    {
      const auto &dataset = group.datasets[i];
      const auto index = dataset.index - 1;
      const auto color = colors.count() > index
                             ? colors.at(index)
                             : colors.at(index % colors.count());

      m_colors[i] = color.name();
    }

    Q_EMIT themeChanged();
  }
}
