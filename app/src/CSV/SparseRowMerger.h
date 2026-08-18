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

#pragma once

#include <algorithm>
#include <functional>
#include <limits>
#include <QByteArray>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/ExportSchema.h"
#include "SSAssert.h"

namespace CSV {

/**
 * @brief Formats @p value into @p dst exactly like QString::number would, without the per-cell
 *        QString. Declared by the merger's owner so both the header and the rows share one
 *        formatter; see Export.cpp.
 */
void appendCsvDouble(QByteArray& dst, double value, bool fixed, int precision);

/**
 * @brief Escapes one CSV field per RFC 4180 and neutralises formula injection; see Export.cpp.
 */
[[nodiscard]] QByteArray escapeCsvBytes(const QString& field);

/**
 * @brief Merges several sources' blocks into one time-ordered sparse CSV body (spec 0055 R6): one
 *        row per distinct sample instant, cells filled only where a source sampled, nothing
 *        forward-filled, equal instants coalesced. Buffers blocks, not rows -- 48 kHz over a
 *        250 ms window is ~12k rows, hundreds of thousands of live QStrings as cells.
 */
class SparseRowMerger {
public:
  using RowSink = std::function<void(const QByteArray&)>;

  /**
   * @brief Binds the column layout every emitted row spans. Clears any buffered blocks.
   */
  void setSchema(const DataModel::ExportSchema& schema)
  {
    m_schema = schema;
    m_pending.clear();
  }

  /**
   * @brief Drops every buffered block without emitting it.
   */
  void clear() noexcept { m_pending.clear(); }

  /**
   * @brief Whether anything is still waiting inside the reorder window.
   */
  [[nodiscard]] bool empty() const noexcept { return m_pending.empty(); }

  /**
   * @brief Buffers @p block with its already-resolved per-sample times. The caller owns the time
   *        policy: a uniform grid keeps its derived offsets, an irregular block takes the
   *        monotonic bump so two frames on one coarse-clock nanosecond stay distinct rows.
   */
  void addBlock(const DataModel::DataBlockPtr& block, std::vector<qint64> times)
  {
    SS_ASSERT(block != nullptr, return);
    SS_ASSERT(times.size() == static_cast<std::size_t>(block->samples), return);

    PendingBlock pending;
    pending.block = block;
    pending.times = std::move(times);
    pending.columnToSchema.reserve(block->columns.size());
    for (const auto& column : block->columns)
      pending.columnToSchema.push_back(m_schema.uniqueIdToColumnIndex.value(column.uniqueId, -1));

    m_pending.push_back(std::move(pending));
  }

  /**
   * @brief Newest buffered sample time; the reorder window is measured back from this.
   */
  [[nodiscard]] qint64 newestNs() const noexcept
  {
    qint64 newest = std::numeric_limits<qint64>::min();
    for (const auto& pending : m_pending)
      if (!pending.times.empty())
        newest = std::max(newest, pending.times.back());

    return newest;
  }

  /**
   * @brief Emits every buffered instant at or before @p cutoffNs, in ascending time order, then
   *        drops the blocks it fully consumed. Pass the max qint64 to drain everything at close.
   */
  void flush(qint64 cutoffNs, const RowSink& sink)
  {
    if (m_pending.empty())
      return;

    // code-verify off
    // Bounded: each pass consumes at least one buffered sample and the buffer is finite.
    while (true) {
      qint64 next  = std::numeric_limits<qint64>::max();
      bool anyLeft = false;
      for (const auto& pending : m_pending) {
        if (pending.cursor >= static_cast<qsizetype>(pending.times.size()))
          continue;

        anyLeft = true;
        next    = std::min(next, pending.times[static_cast<std::size_t>(pending.cursor)]);
      }

      if (!anyLeft || next > cutoffNs)
        break;

      emitRow(next, sink);
    }
    // code-verify on

    const auto spent = [](const PendingBlock& pending) {
      return pending.cursor >= static_cast<qsizetype>(pending.times.size());
    };
    m_pending.erase(std::remove_if(m_pending.begin(), m_pending.end(), spent), m_pending.end());
  }

private:
  /**
   * @brief One buffered block, its resolved schema slots, and how far the merge has consumed it.
   */
  struct PendingBlock {
    DataModel::DataBlockPtr block;
    qsizetype cursor = 0;
    std::vector<int> columnToSchema;
    std::vector<qint64> times;
  };

  /**
   * @brief Builds and emits the row for instant @p ns, advancing every block that had a sample
   *        there. Columns no block filled are left empty: the file states what was sampled, and
   *        inventing a value for the rest is what the sparse layout exists to avoid.
   */
  void emitRow(qint64 ns, const RowSink& sink)
  {
    const auto columns = static_cast<qsizetype>(m_schema.columns.size());

    m_row.resize(0);
    appendCsvDouble(m_row, static_cast<double>(ns) / 1'000'000'000.0, true, 9);

    m_cells.assign(static_cast<std::size_t>(columns), nullptr);
    m_offsets.assign(static_cast<std::size_t>(columns), 0);
    auto& cells   = m_cells;
    auto& offsets = m_offsets;

    for (auto& pending : m_pending) {
      const auto cursor = pending.cursor;
      if (cursor >= static_cast<qsizetype>(pending.times.size()))
        continue;

      if (pending.times[static_cast<std::size_t>(cursor)] != ns)
        continue;

      for (std::size_t c = 0; c < pending.block->columns.size(); ++c) {
        const int slot = pending.columnToSchema[c];
        if (slot < 0 || slot >= columns)
          continue;

        cells[slot]   = &pending.block->columns[c];
        offsets[slot] = cursor;
      }

      ++pending.cursor;
    }

    for (qsizetype j = 0; j < columns; ++j) {
      m_row              += ',';
      const auto* column  = cells[j];
      if (!column)
        continue;

      const auto slot = static_cast<std::size_t>(offsets[j]);
      if (column->hasText)
        m_row += escapeCsvBytes(column->text[slot]);
      else
        appendCsvDouble(m_row, column->values[slot], false, 10);
    }

    m_row += '\n';
    sink(m_row);
  }

private:
  QByteArray m_row;

  // Per-row scratch kept as members so a wide schema does not heap-allocate twice per row
  std::vector<const DataModel::BlockColumn*> m_cells;
  std::vector<qsizetype> m_offsets;
  DataModel::ExportSchema m_schema;
  std::vector<PendingBlock> m_pending;
};

}  // namespace CSV
