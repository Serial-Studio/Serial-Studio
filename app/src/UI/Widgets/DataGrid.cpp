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
#include "Misc/CommonFonts.h"
#include "UI/Widgets/DataGrid.h"

/**
 * @brief Constructs a DataGrid widget and initializes it with dashboard data.
 *
 * Sets up the static table with dataset titles and initial values,
 * applies default fonts, and connects to the dashboard update signal.
 *
 * @param index Index of the data grid group in the dashboard model.
 * @param parent Optional parent QQuickItem.
 */
Widgets::DataGrid::DataGrid(const int index, QQuickItem *parent)
  : StaticTable(parent)
  , m_index(index)
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index))
    return;

  const auto &group = GET_GROUP(SerialStudio::DashboardDataGrid, m_index);

  QList<QStringList> rows;
  rows.append({tr("Title"), tr("Value")});

  for (const auto &dataset : group.datasets())
    rows.append(getRow(dataset));

  setData(rows);
  setFont(Misc::CommonFonts::instance().monoFont());
  setHeaderFont(Misc::CommonFonts::instance().boldUiFont());
  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
          &Widgets::DataGrid::updateData);
}

/**
 * @brief Returns whether the DataGrid is paused.
 *
 * When paused, incoming updates are ignored.
 *
 * @return True if updates are paused, false otherwise.
 */
bool Widgets::DataGrid::paused() const
{
  return m_paused;
}

/**
 * @brief Sets the paused state of the DataGrid.
 *
 * If unpaused, triggers an immediate update.
 * Emits pausedChanged() if the state changes.
 *
 * @param paused New paused state.
 */
void Widgets::DataGrid::setPaused(const bool paused)
{
  if (m_paused != paused)
  {
    m_paused = paused;

    if (!m_paused)
      updateData();

    Q_EMIT pausedChanged();
  }
}

/**
 * @brief Updates the displayed values in the DataGrid.
 *
 * Reads the latest dataset values from the dashboard and updates the table
 * if there are any changes. Units are appended to values when applicable.
 * Skips execution if paused or if the widget index is invalid.
 */
void Widgets::DataGrid::updateData()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index))
    return;

  if (paused())
    return;

  const auto &group = GET_GROUP(SerialStudio::DashboardDataGrid, m_index);

  bool changed = false;
  auto currentData = data();

  for (int i = 0; i < group.datasetCount(); ++i)
  {
    const auto &dataset = group.getDataset(i);

    QString value = dataset.value();
    bool isNumber;
    const double n = value.toDouble(&isNumber);

    if (isNumber)
      value = QString::number(n, 'f', UI::Dashboard::instance().precision());

    if (!dataset.units().isEmpty())
      value += " " + dataset.units();

    if (currentData[i + 1][1] != value)
    {
      currentData[i + 1][1] = value;
      changed = true;
    }
  }

  if (changed)
    setData(currentData);
}

/**
 * @brief Generates a table row from a dataset.
 *
 * Constructs a QStringList containing the dataset's title and value.
 * Units are appended to the value if present.
 *
 * @param dataset The dataset from which to generate the row.
 * @return A QStringList with two entries: title and value.
 */
QStringList Widgets::DataGrid::getRow(const JSON::Dataset &dataset)
{
  const QString title = dataset.title();
  const QString units = dataset.units();
  QString value = dataset.value();

  if (!units.isEmpty())
    value += " " + units;

  return {title, value};
}
