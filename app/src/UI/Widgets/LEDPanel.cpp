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
  // Populate the LED states, titles, and alarm states
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->widgetCount(WC::DashboardLED))
  {
    const auto &group = dash->getGroupWidget(WC::DashboardLED, m_index);
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
  }

  // Connect to the Dashboard to update the LED states
  connect(dash, &UI::Dashboard::updated, this, &LEDPanel::updateData);

  // Configure alarm blinker timer
  m_alarmTimer.setInterval(250);
  m_alarmTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_alarmTimer, &QTimer::timeout, this,
          &Widgets::LEDPanel::onAlarmTimeout);
  m_alarmTimer.start();

  // Set dataset colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::LEDPanel::onThemeChanged);
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
  // Get the dashboard instance and check if the index is valid
  static const auto *dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->widgetCount(WC::DashboardLED))
    return;

  // Get the LED group and update the LED states
  bool changed = false;
  const auto &group = dash->getGroupWidget(WC::DashboardLED, m_index);
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

  // Obtain colors for each dataset in the widget
  m_colors.clear();
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->widgetCount(WC::DashboardLED))
  {
    const auto &group = dash->getGroupWidget(WC::DashboardLED, m_index);
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
  }

  // Update user interface
  Q_EMIT themeChanged();
}
