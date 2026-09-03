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

#include "DataModel/FrameBuilder/ReplayIngest.h"

#include <charconv>
#include <cstdio>

#include "SerialStudio.h"
#include "SSAssert.h"

/**
 * @brief Binds the three FrameBuilder members the cell writers read; the column map arrives when
 *        a player registers one.
 */
DataModel::ReplayIngest::ReplayIngest(const bool& captureDatasetValues,
                                      DataTableStore& tableStore,
                                      TransformEngine* const& exprEngine)
  : m_captureDatasetValues(captureDatasetValues), m_tableStore(tableStore), m_exprEngine(exprEngine)
{}

/**
 * @brief Installs the per-source uniqueId to column map a file player uses for final-value replay.
 */
void DataModel::ReplayIngest::setColumnMap(
  std::unordered_map<int, std::unordered_map<int, int>> map)
{
  m_replayColumnMap = std::move(map);
}

/**
 * @brief Returns the installed uniqueId to column map for @p sourceId, or nullptr when the player
 *        registered none (the index-based fallback applies).
 */
const std::unordered_map<int, int>* DataModel::ReplayIngest::columnsFor(int sourceId) const
{
  SS_ASSERT_HOTPATH(sourceId >= 0);

  const auto it = m_replayColumnMap.find(sourceId);
  return (it != m_replayColumnMap.end()) ? &it->second : nullptr;
}

/**
 * @brief Formats a double exactly like QString::number(v, 'g', 10) but locale-independent and
 *        in place (C-locale %g semantics): the typed replay lane's display string. Apple ships
 *        float std::to_chars only from macOS 13.3, so older targets use snprintf_l with the
 *        NULL (C) locale. The debug parity assert is temporary scaffolding for spec 0022.
 */
static void assignFormattedDouble(QString& dst, double value)
{
  char buf[32];
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const int len = snprintf_l(buf, sizeof(buf), nullptr, "%.10g", value);
  SS_ASSERT_HOTPATH(len > 0 && static_cast<size_t>(len) < sizeof(buf));
  DataModel::assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(len)));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general, 10);
  SS_ASSERT_HOTPATH(res.ec == std::errc());
  DataModel::assign_utf8_in_place(dst, QByteArrayView(buf, static_cast<qsizetype>(res.ptr - buf)));
#endif
  // code-verify off
  // Debug-only parity scaffolding (spec 0022): QString::number allocates, so this can never
  // become a per-cell runtime check.
  Q_ASSERT(dst == QString::number(value, 'g', 10));
  // code-verify on
}

/**
 * @brief The tail both replay writers share: mirror the raw value, clamp a non-numeric to the
 *        widget floor, feed the expression slots and mirror the final value. Identical to the
 *        live lane's tail minus the transform run, because replay keeps the engines torn down.
 */
void DataModel::ReplayIngest::publishDatasetValue(Dataset& dataset)
{
  dataset.rawNumericValue = dataset.numericValue;
  assign_string_in_place(dataset.rawValue, dataset.value);

  if (m_captureDatasetValues)
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.isNumeric)
    dataset.numericValue = (dataset.wgtMax > dataset.wgtMin) ? dataset.wgtMin : 0.0;

  if (m_exprEngine) [[unlikely]]
    m_exprEngine->exprSlots->publish(dataset.uniqueId, dataset.numericValue);

  if (m_captureDatasetValues)
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Replay twin of applyDatasetValue for UTF-8 view cells: identical final-value branch
 *        order (column map, virtual zeros, index fallback), in-place string writes, one parse
 *        per cell, and no transform run -- replay keeps engines torn down.
 */
void DataModel::ReplayIngest::applySpanValue(Dataset& dataset,
                                             const QByteArrayView* cells,
                                             qsizetype count,
                                             const std::unordered_map<int, int>* columns)
{
  SS_ASSERT_HOTPATH(cells != nullptr);
  SS_ASSERT_HOTPATH(count > 0);

  if (columns) [[likely]] {
    const auto it       = columns->find(dataset.uniqueId);
    const qsizetype col = (it != columns->end()) ? it->second : -1;
    if (col >= 0 && col < count) {
      assign_utf8_in_place(dataset.value, cells[col]);
      dataset.numericValue = SerialStudio::toDouble(cells[col], &dataset.isNumeric);
    } else {
      dataset.numericValue = 0.0;
      dataset.value.clear();
      dataset.isNumeric = true;
    }
  } else if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const qsizetype idx = dataset.index;
    if (idx <= 0 || idx > count) [[unlikely]]
      return;

    assign_utf8_in_place(dataset.value, cells[idx - 1]);
    dataset.numericValue = SerialStudio::toDouble(cells[idx - 1], &dataset.isNumeric);
  }

  publishDatasetValue(dataset);
}

/**
 * @brief Replay twin of applyDatasetValue for typed cells: numeric cells keep the native double
 *        (spec 0022's R7 -- no format/parse round trip) while the display string is written in
 *        place with the same 'g'/10 rendering as before; text cells parse once like today.
 */
void DataModel::ReplayIngest::applyTypedValue(Dataset& dataset,
                                              const Cell* cells,
                                              qsizetype count,
                                              const std::unordered_map<int, int>* columns)
{
  SS_ASSERT_HOTPATH(cells != nullptr);
  SS_ASSERT_HOTPATH(count > 0);

  const auto applyCell = [&](const Cell& cell) {
    if (cell.text) {
      assign_string_in_place(dataset.value, *cell.text);
      dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    } else {
      assignFormattedDouble(dataset.value, cell.number);
      dataset.numericValue = cell.number;
      dataset.isNumeric    = true;
    }
  };

  if (columns) [[likely]] {
    const auto it       = columns->find(dataset.uniqueId);
    const qsizetype col = (it != columns->end()) ? it->second : -1;
    if (col >= 0 && col < count) {
      applyCell(cells[col]);
    } else {
      dataset.numericValue = 0.0;
      dataset.value.clear();
      dataset.isNumeric = true;
    }
  } else if (dataset.virtual_) {
    dataset.numericValue = 0.0;
    dataset.value.clear();
    dataset.isNumeric = true;
  } else {
    const qsizetype idx = dataset.index;
    if (idx <= 0 || idx > count) [[unlikely]]
      return;

    applyCell(cells[idx - 1]);
  }

  publishDatasetValue(dataset);
}
