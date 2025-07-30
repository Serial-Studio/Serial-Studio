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

  for (const auto &dataset : group.datasets)
    rows.append(getRow(dataset));

  setData(rows);
  setFont(Misc::CommonFonts::instance().monoFont());
  setHeaderFont(Misc::CommonFonts::instance().boldUiFont());
  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this,
          &Widgets::DataGrid::updateData);

  updateData();
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
  // Validate that the widget exists and that the dashboard is not paused
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index) || paused())
    return;

  // Obtain a reference to the datagrid group & copy current table data
  const auto &group = GET_GROUP(SerialStudio::DashboardDataGrid, m_index);
  auto rows = data();

  // Initialize parameters
  bool changed = false;
  const int valueIndex = 1;

  // Update values for every dataset in the group
  for (size_t i = 0; i < group.datasets.size(); ++i)
  {
    // Obtain a reference to the dataset object & read its value
    const auto &dataset = group.datasets[i];
    QString value = dataset.value;

    // Convert the dataset to a number if needed
    bool isNumber;
    const double n = value.toDouble(&isNumber);
    if (isNumber)
      value = FMT_VAL(n, dataset);

    // Append dataset units (if available)
    if (!dataset.units.isEmpty())
      value += " " + dataset.units;

    // Obtain the row index for the current dataset
    const int rowIndex = i + 1;

    // Validate to table data to ensure we can update it (issue #307)
    if (rows.count() <= rowIndex || rows[rowIndex].count() <= valueIndex)
    {
      QList<QStringList> r;
      r.append({tr("Title"), tr("Value")});
      for (const auto &ds : group.datasets)
        r.append(getRow(ds));

      setData(r);
      return;
    }

    // Update dataset value in current row
    auto &row = rows[rowIndex];
    if (row[valueIndex] != value)
    {
      row[valueIndex] = value;
      changed = true;
    }
  }

  // Update table data & render
  if (changed)
    setData(rows);
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
  const QString title = dataset.title;
  const QString units = dataset.units;

  QString value = dataset.value;
  if (!units.isEmpty())
    value += " " + units;

  return {title, value};
}
