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
#include "UI/Widgets/DataGrid.h"

/**
 * @brief Constructs a DataGrid widget.
 * @param index The index of the data grid in the Dashboard.
 * @param parent The parent QQuickItem (optional).
 */
Widgets::DataGrid::DataGrid(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(index)
{
  // Get the dashboard instance and populate the titles, values, and units
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->widgetCount(WC::DashboardDataGrid))
  {
    const auto &group = dash->getGroupWidget(WC::DashboardDataGrid, m_index);

    m_units.resize(group.datasetCount());
    m_titles.resize(group.datasetCount());
    m_values.resize(group.datasetCount());
    m_alarms.resize(group.datasetCount());

    for (int i = 0; i < group.datasetCount(); ++i)
    {
      auto dataset = group.getDataset(i);

      m_values[i] = "";
      m_alarms[i] = false;
      m_titles[i] = dataset.title();
      m_units[i] = dataset.units().isEmpty()
                       ? ""
                       : QString("[%1]").arg(dataset.units());
    }
  }

  // Connect to the dashboard update event
  connect(dash, &UI::Dashboard::updated, this, &DataGrid::updateData);

  // Set dataset colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::DataGrid::onThemeChanged);
}

/**
 * @brief Returns the number of datasets in the panel.
 * @return An integer number with the number/count of datasets in the panel.
 */
int Widgets::DataGrid::count() const
{
  return m_titles.count();
}

/**
 * @brief Returns the alarm states of the datasets in the panel.
 * @return A vector of booleans representing the alarm states of the datasets.
 */
const QList<bool> &Widgets::DataGrid::alarms() const
{
  return m_alarms;
}

/**
 * @brief Returns the units of the datasets in the data grid.
 * @return A vector of strings representing the units of the datasets.
 */
const QStringList &Widgets::DataGrid::units() const
{
  return m_units;
}

/**
 * @brief Returns the colors of the datasets in the data grid.
 * @return A vector of strings representing the colors of the datasets.
 */
const QStringList &Widgets::DataGrid::colors() const
{
  return m_colors;
}

/**
 * @brief Returns the titles of the datasets in the data grid.
 * @return A vector of strings representing the titles of the datasets.
 */
const QStringList &Widgets::DataGrid::titles() const
{
  return m_titles;
}

/**
 * @brief Returns the values of the datasets in the data grid.
 * @return A vector of strings representing the values of the datasets.
 */
const QStringList &Widgets::DataGrid::values() const
{
  return m_values;
}

/**
 * @brief Updates the data grid data from the Dashboard.
 *
 * This method retrieves the latest data for this data grid from the Dashboard
 * and updates the displayed values accordingly.
 */
void Widgets::DataGrid::updateData()
{
  // Get the dashboard instance and check if the index is valid
  static const auto *dash = &UI::Dashboard::instance();
  if (m_index < 0 || m_index >= dash->widgetCount(WC::DashboardDataGrid))
    return;

  // Regular expression to check if the value is a number
  static const QRegularExpression regex("^[+-]?(\\d*\\.)?\\d+$");

  // Get the datagrid group and update the LED states
  bool changed = false;
  const auto &group = dash->getGroupWidget(WC::DashboardDataGrid, m_index);
  for (int i = 0; i < group.datasetCount(); ++i)
  {
    // Get the dataset and its values
    const auto &dataset = group.getDataset(i);
    const auto alarmValue = dataset.alarm();
    auto value = dataset.value();

    // Process dataset numerical value
    bool alarm = false;
    if (regex.match(value).hasMatch())
    {
      const double v = value.toDouble();
      value = QString::number(v, 'f', dash->precision());
      alarm = (alarmValue != 0 && v >= alarmValue);
    }

    // Update the alarm state
    if (m_alarms[i] != alarm)
    {
      changed = true;
      m_alarms[i] = alarm;
    }

    // Update value text
    if (m_values[i] != value)
    {
      changed = true;
      m_values[i] = value;
    }
  }

  // Redraw the widget
  if (changed)
    Q_EMIT updated();
}

/**
 * @brief Updates the colors for each dataset in the widget based on the
 *        colorscheme defined by the application's currently loaded theme.
 */
void Widgets::DataGrid::onThemeChanged()
{
  // clang-format off
  const auto colors = Misc::ThemeManager::instance().colors()["widget_colors"].toArray();
  // clang-format on

  // Obtain colors for each dataset in the widget
  m_colors.clear();
  auto dash = &UI::Dashboard::instance();
  if (m_index >= 0 && m_index < dash->widgetCount(WC::DashboardDataGrid))
  {
    const auto &group = dash->getGroupWidget(WC::DashboardDataGrid, m_index);
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
