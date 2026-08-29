/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "CSV/Player/RowCodec.h"

#include <QStringList>

#include "CSV/Player/RowSyntax.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction & configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a codec in the legacy default reading: comma-separated, numeric timestamp
 *        in the first cell.
 */
CSV::RowCodec::RowCodec()
  : m_separator(','), m_timestampColumn(0), m_mode(PlayerTimestampMode::Numeric)
{}

/**
 * @brief Returns the active cell separator.
 */
char CSV::RowCodec::separator() const
{
  return m_separator;
}

/**
 * @brief Returns the file cell index holding the timestamp in DateTimeColumn mode.
 */
int CSV::RowCodec::timestampColumn() const
{
  return m_timestampColumn;
}

/**
 * @brief Returns how the reader derives a row's time, which is what decides the exclusion the
 *        data-cell split applies.
 */
CSV::PlayerTimestampMode CSV::RowCodec::timestampMode() const
{
  return m_mode;
}

/**
 * @brief Returns every cell of the last splitCells() call, timestamp column included.
 */
const DataModel::ReplayCellViews& CSV::RowCodec::cells() const
{
  return m_cells;
}

/**
 * @brief Returns the data cells of the last splitDataCells() call.
 */
const QByteArrayView* CSV::RowCodec::dataSpans() const
{
  return m_dataSpans.constData();
}

/**
 * @brief Returns how many data cells the last splitDataCells() call produced.
 */
qsizetype CSV::RowCodec::dataSpanCount() const
{
  return m_dataSpans.size();
}

/**
 * @brief Restores the default reading and releases the scratch buffers; called when the player
 *        closes a file so a closed recording pins nothing.
 */
void CSV::RowCodec::reset()
{
  m_separator       = ',';
  m_timestampColumn = 0;
  m_mode            = PlayerTimestampMode::Numeric;
  m_cells           = {};
  m_splitScratch    = QByteArray();
  m_dataSpans       = {};
}

/**
 * @brief Sets the cell separator detected for the open recording.
 */
void CSV::RowCodec::setSeparator(char separator)
{
  SS_ASSERT(separator != '"', return);
  SS_ASSERT_LOG(separator != '\0');

  m_separator = separator;
}

/**
 * @brief Sets the timestamp reading; @p column is honoured in DateTimeColumn mode only.
 */
void CSV::RowCodec::setTimestampMode(PlayerTimestampMode mode, int column)
{
  SS_ASSERT(column >= 0, return);
  SS_ASSERT_LOG(mode != PlayerTimestampMode::DateTimeColumn || column >= 0);

  m_mode            = mode;
  m_timestampColumn = column;
}

//--------------------------------------------------------------------------------------------------
// Row splitting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Splits @p row into cells() verbatim, keeping the timestamp cell.
 */
void CSV::RowCodec::splitCells(QByteArrayView row)
{
  DataModel::splitReplayRowSpans(row, m_cells, m_splitScratch, m_separator);
}

/**
 * @brief Splits @p row and fills the data spans with its data cells (timestamp column excluded
 *        per the active mode); returns the data-cell count. Views stay valid until the next
 *        split or reset().
 */
qsizetype CSV::RowCodec::splitDataCells(QByteArrayView row)
{
  DataModel::splitReplayRowSpans(row, m_cells, m_splitScratch, m_separator);

  m_dataSpans.clear();
  switch (m_mode) {
    case PlayerTimestampMode::Interval:
      for (const auto& cell : m_cells)
        m_dataSpans.append(cell);

      break;

    case PlayerTimestampMode::DateTimeColumn:
      for (qsizetype i = 0; i < m_cells.size(); ++i)
        if (i != m_timestampColumn)
          m_dataSpans.append(m_cells.at(i));

      break;

    case PlayerTimestampMode::Numeric:
    case PlayerTimestampMode::DateTime:
      for (qsizetype i = 1; i < m_cells.size(); ++i)
        m_dataSpans.append(m_cells.at(i));

      break;
  }

  return m_dataSpans.size();
}

/**
 * @brief File cell index of data column @p i, undoing the timestamp-column exclusion that
 *        splitDataCells applies for the active timestamp mode.
 */
int CSV::RowCodec::dataColumnToFileColumn(int i) const
{
  SS_ASSERT(i >= 0, return -1);
  SS_ASSERT_LOG(m_timestampColumn >= 0);

  switch (m_mode) {
    case PlayerTimestampMode::Interval:
      return i;
    case PlayerTimestampMode::DateTimeColumn:
      return (i < m_timestampColumn) ? i : i + 1;
    case PlayerTimestampMode::Numeric:
    case PlayerTimestampMode::DateTime:
      return i + 1;
  }

  return -1;
}

//--------------------------------------------------------------------------------------------------
// QuickPlot payload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the QuickPlot byte payload for @p row: the raw row minus the timestamp cell,
 *        sliced verbatim from the caller's bytes where possible. Date-time-column mode and
 *        non-comma files (spec 0048) rebuild through the joiner, so injected payloads are
 *        always RFC-4180 comma rows for the downstream comma-only splitters.
 */
QByteArray CSV::RowCodec::quickPlotPayload(QByteArrayView row)
{
  SS_ASSERT_LOG(m_separator != '"');

  auto view = row;
  if (view.endsWith('\r'))
    view.chop(1);

  if (m_separator != ',' || m_mode == PlayerTimestampMode::DateTimeColumn) {
    const qsizetype count = splitDataCells(row);
    QStringList cells;
    cells.reserve(count);
    for (qsizetype i = 0; i < count; ++i)
      cells.append(QString::fromUtf8(m_dataSpans.at(i)));

    QByteArray frame = DataModel::joinReplayRow(cells);
    frame.append('\n');
    return frame;
  }

  if (m_mode == PlayerTimestampMode::Interval) {
    QByteArray frame(view.constData(), view.size());
    frame.append('\n');
    return frame;
  }

  const qsizetype comma = firstTopLevelSeparator(view, m_separator);
  if (comma < 0)
    return QByteArray();

  QByteArray frame(view.constData() + comma + 1, view.size() - comma - 1);
  frame.append('\n');
  return frame;
}
