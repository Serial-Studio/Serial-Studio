/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "UI/Widgets/DataGrid.h"

#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// DataGridRowsModel -- backing list model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty list model.
 */
Widgets::DataGridRowsModel::DataGridRowsModel(QObject* parent) : QAbstractListModel(parent) {}

/**
 * @brief Returns the number of rows currently held by the model.
 */
int Widgets::DataGridRowsModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return m_rows.size();
}

/**
 * @brief Returns the value of @p role for the row at @p index.
 */
QVariant Widgets::DataGridRowsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
    return {};

  switch (role) {
    case TitleRole:
      return m_rows.at(index.row()).first;
    case ValueRole:
      return m_rows.at(index.row()).second;
    default:
      return {};
  }
}

/**
 * @brief Maps the role enum to the names QML uses to bind ("title", "value").
 */
QHash<int, QByteArray> Widgets::DataGridRowsModel::roleNames() const
{
  return {
    {TitleRole, "title"},
    {ValueRole, "value"},
  };
}

/**
 * @brief Replaces every row with @p rows. Use only when the row set itself changes
 *        (project edit, dataset added or removed) -- tears down all delegates.
 */
void Widgets::DataGridRowsModel::reset(const QVector<QPair<QString, QString>>& rows)
{
  beginResetModel();
  m_rows = rows;
  endResetModel();
}

/**
 * @brief Updates the title and/or value of an existing row, emitting dataChanged() only
 *        for the roles that actually moved. Out-of-range rows are ignored.
 */
bool Widgets::DataGridRowsModel::updateRow(int row, const QString& title, const QString& value)
{
  if (row < 0 || row >= m_rows.size())
    return false;

  auto& entry = m_rows[row];
  QVector<int> roles;
  roles.reserve(2);

  if (entry.first != title) {
    entry.first = title;
    roles.append(TitleRole);
  }

  if (entry.second != value) {
    entry.second = value;
    roles.append(ValueRole);
  }

  if (roles.isEmpty())
    return false;

  const auto idx = index(row);
  Q_EMIT dataChanged(idx, idx, roles);
  return true;
}

//--------------------------------------------------------------------------------------------------
// DataGrid -- QML model wrapper
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the row model and wires it to the dashboard update tick.
 */
Widgets::DataGrid::DataGrid(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_paused(false)
  , m_titleHeader(tr("Title"))
  , m_valueHeader(tr("Value"))
  , m_rowsModel(new DataGridRowsModel(this))
  , m_lastRowCount(-1)
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index))
    return;

  rebuildRows();

  connect(
    &UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Widgets::DataGrid::updateData);
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the DataGrid is paused.
 */
bool Widgets::DataGrid::paused() const noexcept
{
  return m_paused;
}

/**
 * @brief Returns the row model exposed to QML.
 */
Widgets::DataGridRowsModel* Widgets::DataGrid::rowsModel() const noexcept
{
  return m_rowsModel;
}

/**
 * @brief Returns the localized header label for the left column.
 */
const QString& Widgets::DataGrid::titleHeader() const noexcept
{
  return m_titleHeader;
}

/**
 * @brief Returns the localized header label for the right column.
 */
const QString& Widgets::DataGrid::valueHeader() const noexcept
{
  return m_valueHeader;
}

//--------------------------------------------------------------------------------------------------
// Configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pauses or resumes the per-tick refresh. Resuming pulls a fresh snapshot immediately.
 */
void Widgets::DataGrid::setPaused(const bool paused)
{
  if (m_paused == paused)
    return;

  m_paused = paused;

  if (!m_paused)
    updateData();

  Q_EMIT pausedChanged();
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the dashboard's current dataset values and pushes them into the model.
 */
void Widgets::DataGrid::updateData()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index) || m_paused)
    return;

  const auto& group  = GET_GROUP(SerialStudio::DashboardDataGrid, m_index);
  const int rowCount = static_cast<int>(group.datasets.size());

  if (rowCount != m_lastRowCount) [[unlikely]] {
    rebuildRows();
    return;
  }

  for (int i = 0; i < rowCount; ++i) {
    const auto& dataset = group.datasets[static_cast<size_t>(i)];
    m_rowsModel->updateRow(i, dataset.title, formatValue(dataset));
  }
}

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reseeds the row model from scratch when the dataset count or set changes.
 */
void Widgets::DataGrid::rebuildRows()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardDataGrid, m_index)) {
    m_rowsModel->reset({});
    m_lastRowCount = 0;
    return;
  }

  const auto& group = GET_GROUP(SerialStudio::DashboardDataGrid, m_index);
  QVector<QPair<QString, QString>> rows;
  rows.reserve(static_cast<int>(group.datasets.size()));

  for (const auto& dataset : group.datasets)
    rows.append({dataset.title, formatValue(dataset)});

  m_rowsModel->reset(rows);
  m_lastRowCount = rows.size();
}

/**
 * @brief Formats a dataset value for display; returns empty when no sample has arrived.
 */
QString Widgets::DataGrid::formatValue(const DataModel::Dataset& dataset) const
{
  // Leave the cell empty so the QML side can paint its placeholder
  if (dataset.value.isEmpty())
    return QString();

  // Numeric path goes through the dashboard's locale-aware formatter
  QString value  = dataset.value;
  bool isNumber  = false;
  const double n = value.toDouble(&isNumber);
  if (isNumber)
    value = FMT_VAL(n, dataset);

  if (!dataset.units.isEmpty())
    value += QStringLiteral(" ") + dataset.units;

  return value;
}
