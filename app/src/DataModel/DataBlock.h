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

#include <chrono>
#include <cstring>
#include <memory>
#include <QHash>
#include <QString>
#include <vector>

#include "DataModel/Frame.h"
#include "HotpathOptimization.h"
#include "SSAssert.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Unified publication payload (spec 0055)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One dataset's samples inside a block. Numeric values are always present; @c text is
 *        populated only for datasets that produce strings, which per spec-0055 D2 never includes
 *        a dense (stream-lane) source -- so a numeric block leaves it empty and pays nothing.
 */
struct BlockColumn {
  int uniqueId = -1;              ///< Dataset identity this column carries
  bool hasText = false;           ///< True when @c text mirrors @c values
  bool hasRaw  = false;           ///< True when the pre-transform values are carried too
  std::vector<double> values;     ///< Sized to block capacity; @c DataBlock::samples is the fill
  std::vector<QString> text;      ///< Empty unless @c hasText
  std::vector<quint8> numeric;    ///< Per-sample parsed-as-number flag; empty means always true
  std::vector<double> rawValues;  ///< Pre-transform values; empty unless @c hasRaw
  std::vector<QString> rawText;   ///< Pre-transform strings; empty unless @c hasRaw && @c hasText
  std::vector<double> fftWindow;  ///< Producer-computed FFT window; empty when it computed none
};

/**
 * @brief The one payload every consumer ingests (spec 0055): N samples of M datasets on a shared
 *        timebase, uniform when @c dt is non-zero (sample i at <tt>t0 + i * dt</tt>) and
 *        irregular otherwise, where @c times carries one offset per sample. Blocks come from a
 *        pool, so @c samples is a fill cursor into storage sized once and reused.
 */
struct DataBlock {
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  int sourceId                = 0;   ///< Source that produced these samples
  quint64 blockNumber         = 0;   ///< Monotonic per-source block counter
  quint64 structureGeneration = 0;   ///< Pool generation the block was staged under
  qsizetype samples           = 0;   ///< Samples currently filled
  SteadyTimePoint t0;                ///< Capture time of sample 0
  std::chrono::nanoseconds dt{0};    ///< Uniform step; 0 means read @c times instead
  std::vector<qint64> times;         ///< Per-sample ns offsets from @c t0; empty when @c dt != 0
  std::vector<BlockColumn> columns;  ///< One entry per dataset, in export-schema order
  bool masked = false;               ///< Replay: read-only observers only, never a recording sink
};

/**
 * @typedef DataBlockPtr
 * @brief Shared immutable pointer to a published block.
 */
typedef std::shared_ptr<const DataBlock> DataBlockPtr;

/**
 * @brief Structure travels separately from values and is republished only when the frame pool's
 *        generation bumps, so consumers reconfigure once per layout change instead of comparing
 *        structures per frame. A block whose generation does not match the last snapshot is stale.
 */
struct StructureSnapshot {
  quint64 generation = 0;  ///< Matches DataBlock::structureGeneration
  DataModel::Frame data;   ///< Groups, datasets, actions and sources; values are not meaningful
};

/**
 * @typedef StructureSnapshotPtr
 * @brief Shared immutable pointer to a structure snapshot.
 */
typedef std::shared_ptr<const StructureSnapshot> StructureSnapshotPtr;

//--------------------------------------------------------------------------------------------------
// Timebase helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether @p block carries a uniform sample grid rather than explicit times.
 */
[[nodiscard]] SS_FORCE_INLINE bool uniform_grid(const DataBlock& block) noexcept
{
  return block.dt.count() != 0;
}

/**
 * @brief Nanosecond offset of sample @p index from the block's @c t0.
 */
[[nodiscard]] SS_FORCE_INLINE qint64 sample_offset_ns(const DataBlock& block,
                                                      qsizetype index) noexcept
{
  SS_ASSERT_HOTPATH(index >= 0 && index < block.samples);

  if (uniform_grid(block))
    return static_cast<qint64>(index) * block.dt.count();

  return block.times[static_cast<std::size_t>(index)];
}

/**
 * @brief Absolute capture time of sample @p index, on the driver's own clock.
 */
[[nodiscard]] SS_FORCE_INLINE DataBlock::SteadyTimePoint sample_time(const DataBlock& block,
                                                                     qsizetype index) noexcept
{
  return block.t0 + std::chrono::nanoseconds(sample_offset_ns(block, index));
}

//--------------------------------------------------------------------------------------------------
// Storage management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sizes every column (and the irregular-time array) to @p capacity, leaving the block empty.
 *        Called once per structural binding; the storage is then reused for the pool slot's life,
 *        which is what keeps staging allocation-free.
 */
inline void size_block_storage(DataBlock& block, qsizetype capacity, bool irregular)
{
  SS_ASSERT(capacity > 0, return);

  const auto room = static_cast<std::size_t>(capacity);
  for (auto& column : block.columns) {
    column.values.resize(room);
    if (column.hasText) {
      column.text.resize(room);
      column.numeric.resize(room);
    } else {
      column.text.clear();
      column.numeric.clear();
    }

    if (!column.hasRaw) {
      column.rawValues.clear();
      column.rawText.clear();
      continue;
    }

    column.rawValues.resize(room);
    if (column.hasText)
      column.rawText.resize(room);
  }

  if (irregular)
    block.times.resize(room);
  else
    block.times.clear();

  block.samples = 0;
}

/**
 * @brief Empties @p block without releasing its storage, so the next binding refills in place.
 */
inline void reset_block(DataBlock& block) noexcept
{
  block.samples             = 0;
  block.blockNumber         = 0;
  block.structureGeneration = 0;
  block.masked              = false;
}

/**
 * @brief Detaches a copy of @p block holding exactly its filled samples: a queued sink holding a
 *        pool slot would pin the pool and starve staging. fftWindow is left behind as display-only
 *        state the dashboard reads off the pooled slot.
 */
[[nodiscard]] inline std::shared_ptr<DataBlock> clone_block_trimmed(const DataBlock& block)
{
  auto copy                 = std::make_shared<DataBlock>();
  copy->sourceId            = block.sourceId;
  copy->blockNumber         = block.blockNumber;
  copy->structureGeneration = block.structureGeneration;
  copy->samples             = block.samples;
  copy->t0                  = block.t0;
  copy->dt                  = block.dt;
  copy->masked              = block.masked;

  const auto used = static_cast<std::size_t>(block.samples);
  if (!block.times.empty())
    copy->times.assign(block.times.begin(), block.times.begin() + used);

  copy->columns.resize(block.columns.size());
  for (std::size_t c = 0; c < block.columns.size(); ++c) {
    const auto& src = block.columns[c];
    auto& dst       = copy->columns[c];
    dst.uniqueId    = src.uniqueId;
    dst.hasText     = src.hasText;
    dst.hasRaw      = src.hasRaw;
    dst.values.assign(src.values.begin(), src.values.begin() + used);
    if (src.hasText) {
      dst.text.assign(src.text.begin(), src.text.begin() + used);
      dst.numeric.assign(src.numeric.begin(), src.numeric.begin() + used);
    }

    if (!src.hasRaw)
      continue;

    dst.rawValues.assign(src.rawValues.begin(), src.rawValues.begin() + used);
    if (src.hasText)
      dst.rawText.assign(src.rawText.begin(), src.rawText.begin() + used);
  }

  return copy;
}

/**
 * @brief Bytes a materialised block currently holds, for the pool's budget accounting.
 */
[[nodiscard]] inline std::size_t block_footprint_bytes(const DataBlock& block) noexcept
{
  std::size_t bytes = block.times.capacity() * sizeof(qint64);
  for (const auto& column : block.columns) {
    bytes += column.values.capacity() * sizeof(double);
    bytes += column.text.capacity() * sizeof(QString);
    bytes += column.numeric.capacity();
    bytes += column.rawValues.capacity() * sizeof(double);
    bytes += column.rawText.capacity() * sizeof(QString);
  }

  return bytes;
}

//--------------------------------------------------------------------------------------------------
// Per-sample writes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies @p src into @p dst while keeping @p dst's buffer its own. Frame.h's
 *        assign_string_in_place share-assigns when the destination is not already detached, which
 *        is exactly a freshly sized block column -- and sharing the producer's dataset string
 *        makes the pipeline's next assign_utf8_in_place into it detach and allocate.
 */
SS_FORCE_INLINE void assign_string_owned(QString& dst, const QString& src)
{
  const qsizetype n = src.size();
  if (!dst.isDetached() || dst.capacity() < n) {
    dst = QString();
    dst.reserve(qMax<qsizetype>(n, 8));
  }

  dst.resize(n);
  if (n > 0)
    memcpy(dst.data(), src.constData(), static_cast<std::size_t>(n) * sizeof(QChar));
}

/**
 * @brief Writes one sample into @p column at @p index, keeping the text buffer owned so the
 *        producer's string stays uniquely referenced and writable in place.
 */
SS_FORCE_INLINE void write_block_sample(
  BlockColumn& column, qsizetype index, double value, const QString& text, bool numeric)
{
  SS_ASSERT_HOTPATH(index >= 0);
  SS_ASSERT_HOTPATH(static_cast<std::size_t>(index) < column.values.size());

  const auto slot     = static_cast<std::size_t>(index);
  column.values[slot] = value;
  if (!column.hasText)
    return;

  column.numeric[slot] = numeric ? 1 : 0;
  assign_string_owned(column.text[slot], text);
}

/**
 * @brief Records sample @p index's pre-transform value. MDF4 writes it as a "(raw)" channel, the
 *        session database stores it beside the final, and spec-0044 verification compares the two
 *        to tell a parse-stage divergence from a transform-stage one -- so a column that carries
 *        raw must carry it for every sample, not just the ones a transform changed.
 */
SS_FORCE_INLINE void write_block_raw(BlockColumn& column,
                                     qsizetype index,
                                     double rawValue,
                                     const QString& rawText)
{
  if (!column.hasRaw)
    return;

  const auto slot        = static_cast<std::size_t>(index);
  column.rawValues[slot] = rawValue;
  if (column.hasText)
    assign_string_owned(column.rawText[slot], rawText);
}

/**
 * @brief Whether sample @p index parsed as a number. A column that carries no per-sample flag is a
 *        dense source's, which is numeric by construction (spec 0055 D2).
 */
[[nodiscard]] SS_FORCE_INLINE bool sample_is_numeric(const BlockColumn& column,
                                                     qsizetype index) noexcept
{
  if (column.numeric.empty())
    return true;

  return column.numeric[static_cast<std::size_t>(index)] != 0;
}

/**
 * @brief Records the sample's own capture offset on an irregular-timebase block; a no-op on a
 *        uniform grid, where the offset is derived from @c dt instead of stored.
 */
SS_FORCE_INLINE void write_block_time(DataBlock& block, qsizetype index, qint64 offsetNs) noexcept
{
  SS_ASSERT_HOTPATH(index >= 0);

  if (!uniform_grid(block))
    block.times[static_cast<std::size_t>(index)] = offsetNs;
}

/**
 * @brief One source's structure plus a uniqueId -> dataset lookup. Consumers whose published shape
 *        is still a frame (the API wire, MQTT payloads, gRPC messages) keep one of these per source
 *        and stamp a block's values onto it, so the storage change underneath them is invisible on
 *        the wire (spec 0055 D5).
 */
struct FrameTemplate {
  FrameTemplate()                                = default;
  FrameTemplate(FrameTemplate&&)                 = delete;
  FrameTemplate(const FrameTemplate&)            = delete;
  FrameTemplate& operator=(FrameTemplate&&)      = delete;
  FrameTemplate& operator=(const FrameTemplate&) = delete;

  DataModel::Frame frame;
  QHash<int, DataModel::Dataset*> byUniqueId;
};

/**
 * @brief Adopts @p src as @p tpl's structure and rebuilds its uniqueId lookup.
 */
inline void bind_frame_template(FrameTemplate& tpl, const DataModel::Frame& src)
{
  tpl.frame = src;
  tpl.byUniqueId.clear();
  for (auto& group : tpl.frame.groups)
    for (auto& dataset : group.datasets)
      tpl.byUniqueId.insert(dataset.uniqueId, &dataset);
}

/**
 * @brief Writes sample @p index of @p block onto @p tpl's datasets. Columns the template does not
 *        know are skipped rather than appended: the template is the published structure, and a
 *        block from a since-changed layout must not invent fields in it.
 */
inline void apply_block_sample(FrameTemplate& tpl, const DataBlock& block, qsizetype index)
{
  const auto slot = static_cast<std::size_t>(index);
  for (const auto& column : block.columns) {
    const auto it = tpl.byUniqueId.constFind(column.uniqueId);
    if (it == tpl.byUniqueId.constEnd())
      continue;

    auto* dataset         = it.value();
    dataset->numericValue = column.values[slot];
    dataset->isNumeric    = sample_is_numeric(column, index);
    dataset->value =
      column.hasText ? column.text[slot] : QString::number(column.values[slot], 'g', 10);

    dataset->rawNumericValue = column.hasRaw ? column.rawValues[slot] : dataset->numericValue;
    dataset->rawValue = column.hasRaw && column.hasText ? column.rawText[slot] : dataset->value;
  }
}

}  // namespace DataModel
