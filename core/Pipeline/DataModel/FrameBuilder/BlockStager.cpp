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

#include "DataModel/FrameBuilder/BlockStager.h"

#include <algorithm>
#include <chrono>

#include "Core/SSAssert.h"
#include "IO/PipelineHost.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Out-of-line key function: pins the host interface's vtable to this translation unit.
 */
DataModel::BlockStagerHost::~BlockStagerHost() = default;

/**
 * @brief Default-constructs a block slot with no generation or source binding.
 */
DataModel::BlockStager::PooledBlockSlot::PooledBlockSlot()
  : generation(0), flushEpoch(0), sourceId(-1)
{}

/**
 * @brief Binds the stager to its host and materialises the block pool. @p generation and
 *        @p maskSinks are the facade's own members, read (never written) here, so a pool
 *        invalidation or a masked republish bracket is visible without a second copy of the state.
 */
DataModel::BlockStager::BlockStager(DataModel::BlockStagerHost& host,
                                    const quint64& generation,
                                    const bool& maskSinks)
  : m_host(host)
  , m_generation(generation)
  , m_maskSinks(maskSinks)
  , m_poolHint(0)
  , m_slotsUsable(kBlockPoolSlots)
{
  m_pool.reserve(kBlockPoolSlots);
  for (int i = 0; i < kBlockPoolSlots; ++i)
    m_pool.emplace_back(std::make_shared<PooledBlockSlot>());
}

//--------------------------------------------------------------------------------------------------
// Pool management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns how many blocks @p sourceId has published, or 0 when it never published one.
 */
quint64 DataModel::BlockStager::blockNumber(int sourceId) const noexcept
{
  const auto it = m_blockNumbers.find(sourceId);
  return it != m_blockNumbers.end() ? it->second : quint64(0);
}

/**
 * @brief Caps how many block slots may be materialised so a wide project cannot blow past
 *        kBlockPoolBudgetBytes: a slot's storage scales with the dataset count, and 64 of them
 *        is a lot of memory once a project carries hundreds of datasets. Never drops below the
 *        dashboard ring plus headroom, since starving staging is worse than exceeding the budget.
 */
void DataModel::BlockStager::refreshBudget(const DataModel::Frame& src) noexcept
{
  std::size_t datasets = 0;
  for (const auto& group : src.groups)
    datasets += group.datasets.size();

  constexpr std::size_t kFloor = IO::PipelineHost::kBlockRingSize + 8;
  if (datasets == 0) {
    m_slotsUsable = kBlockPoolSlots;
    return;
  }

  const std::size_t perSample = 2 * sizeof(double) + 2 * sizeof(QString) + 1 + sizeof(qint64);
  const std::size_t perSlot = datasets * static_cast<std::size_t>(kFrameBlockSampleCap) * perSample;
  const std::size_t affordable = perSlot > 0 ? kBlockPoolBudgetBytes / perSlot : kBlockPoolSlots;

  m_slotsUsable = static_cast<int>(std::clamp<std::size_t>(affordable, kFloor, kBlockPoolSlots));
}

/**
 * @brief Probes for a free block slot, preferring one already laid out for @p sourceId at the
 *        current generation so openBlockFor() can skip the rebind and keep the slot's column
 *        storage. use_count()==1 is exact here for the same reason it is on the frame pool: every
 *        alias lives on this thread, and the builder holds its own reference while a block is open.
 */
std::shared_ptr<DataModel::BlockStager::PooledBlockSlot> DataModel::BlockStager::claimSlot(
  int sourceId) noexcept
{
  static_assert(IO::PipelineHost::kBlockRingSize < kBlockPoolSlots - 8,
                "block pool must outsize the dashboard ring or staging starves");

  SS_ASSERT(!m_pool.empty(), return nullptr);

  const std::size_t n = std::min(m_pool.size(), static_cast<std::size_t>(m_slotsUsable));
  std::size_t freeIdx = n;

  for (std::size_t k = 0; k < n; ++k) {
    const std::size_t idx = (m_poolHint + k) % n;
    const auto& slot      = m_pool[idx];
    if (slot.use_count() != 1)
      continue;

    if (slot->generation == m_generation && slot->sourceId == sourceId) {
      m_poolHint = (idx + 1) % n;
      return m_pool[idx];
    }

    if (freeIdx == n)
      freeIdx = idx;
  }

  if (freeIdx == n)
    return nullptr;

  m_poolHint = (freeIdx + 1) % n;
  return m_pool[freeIdx];
}

/**
 * @brief Returns every idle slot's storage to the allocator (called after the last player closes):
 *        a replay binds slots to the project's full column layout, and pool storage otherwise
 *        persists for the whole session. Busy slots are skipped; the next claim rebinds them.
 */
void DataModel::BlockStager::releaseIdleStorage()
{
  for (auto& slot : m_pool) {
    if (slot.use_count() != 1)
      continue;

    slot->block      = DataModel::DataBlock();
    slot->generation = 0;
    slot->sourceId   = -1;
  }
}

//--------------------------------------------------------------------------------------------------
// Block staging (spec 0055)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds a claimed slot's block to @p src's dataset layout and sizes its storage once. Runs
 *        on structural change only; every later frame of that generation is a plain store into the
 *        storage laid out here.
 */
void DataModel::BlockStager::bindToFrame(PooledBlockSlot& slot,
                                         const DataModel::Frame& src,
                                         bool uniform)
{
  auto& block               = slot.block;
  block.sourceId            = src.sourceId;
  block.structureGeneration = m_generation;
  block.dt                  = std::chrono::nanoseconds(0);

  block.columns.clear();
  for (const auto& group : src.groups)
    for (const auto& dataset : group.datasets) {
      DataModel::BlockColumn column;
      column.uniqueId = dataset.uniqueId;
      column.hasText  = true;
      column.hasRaw   = true;
      block.columns.push_back(std::move(column));
    }

  DataModel::size_block_storage(block, kFrameBlockSampleCap, !uniform);
  slot.generation = m_generation;
  slot.sourceId   = src.sourceId;
}

/**
 * @brief Returns @p sourceId's open block, opening one when none is held and flushing first when
 *        the held block was staged under an older layout -- a block must never straddle a
 *        structural change. Null when every pool slot is still referenced by a consumer.
 */
DataModel::BlockStager::PooledBlockSlot* DataModel::BlockStager::openBlockFor(
  int sourceId, const DataModel::Frame& src)
{
  const auto it = m_open.find(sourceId);
  if (it != m_open.end()) [[likely]] {
    const bool reusable =
      it->second->generation == m_generation && it->second->block.masked == m_maskSinks;
    if (reusable) [[likely]]
      return it->second.get();

    flush(sourceId);
  }

  auto slot = claimSlot(sourceId);
  if (!slot) [[unlikely]] {
    m_host.noteStagingPoolExhausted();
    return nullptr;
  }

  if (slot->generation != m_generation || slot->sourceId != src.sourceId)
    bindToFrame(*slot, src, false);

  slot->flushEpoch = m_host.stagingFlushEpoch();
  DataModel::reset_block(slot->block);
  slot->block.structureGeneration = m_generation;
  slot->block.masked              = m_maskSinks;

  const auto inserted = m_open.emplace(sourceId, std::move(slot));
  return inserted.first->second.get();
}

/**
 * @brief Appends one parsed frame's dataset values to @p sourceId's open block, flushing when the
 *        block is full or the display tick moved the flush epoch on (spec 0055 D1). Every write is
 *        a store into storage sized at bind time, so the steady state allocates nothing.
 */
SS_HOT void DataModel::BlockStager::stage(int sourceId,
                                          const DataModel::Frame& src,
                                          const DataModel::TimestampedFrame::SteadyTimePoint& ts)
{
  SS_ASSERT_HOTPATH(sourceId >= 0);

  m_host.announceStructure(sourceId, src);

  auto* slot = openBlockFor(sourceId, src);
  if (!slot) [[unlikely]]
    return;

  auto& block           = slot->block;
  const qsizetype index = block.samples;
  if (index == 0)
    block.t0 = ts;

  std::size_t column = 0;
  for (const auto& group : src.groups) {
    SS_NO_UNROLL
    for (const auto& dataset : group.datasets) {
      if (column >= block.columns.size()) [[unlikely]]
        break;

      SS_ASSERT_HOTPATH(block.columns[column].uniqueId == dataset.uniqueId);
      DataModel::write_block_sample(
        block.columns[column], index, dataset.numericValue, dataset.value, dataset.isNumeric);
      DataModel::write_block_raw(
        block.columns[column], index, dataset.rawNumericValue, dataset.rawValue);
      ++column;
    }
  }

  DataModel::write_block_time(
    block, index, std::chrono::duration_cast<std::chrono::nanoseconds>(ts - block.t0).count());

  block.samples = index + 1;

  const quint64 epoch = m_host.stagingFlushEpoch();
  if (block.samples >= kFrameBlockSampleCap || epoch != slot->flushEpoch) [[unlikely]]
    flush(sourceId);
}

/**
 * @brief Publishes @p sourceId's open block and releases the stager's reference to its slot. The
 *        hand-out is an aliasing shared_ptr over the pool slot, so there is no per-block control
 *        block and the slot frees itself once every consumer has drained it.
 */
void DataModel::BlockStager::flush(int sourceId)
{
  const auto it = m_open.find(sourceId);
  if (it == m_open.end())
    return;

  auto slot = it->second;
  m_open.erase(it);

  if (slot->block.samples == 0)
    return;

  slot->block.blockNumber = ++m_blockNumbers[sourceId];
  m_host.publishStagedBlock(DataModel::DataBlockPtr(slot, &slot->block));
}

/**
 * @brief Flushes every open block regardless of fill (queued from the GUI display tick, spec 0055
 *        D1). A source that has gone quiet would otherwise hold its partial block indefinitely,
 *        because the cap and the epoch check are only reached while frames keep arriving.
 */
void DataModel::BlockStager::flushAll()
{
  if (m_open.empty()) [[likely]]
    return;

  std::vector<int> sources;
  sources.reserve(m_open.size());
  for (const auto& [sourceId, slot] : m_open)
    sources.push_back(sourceId);

  for (const int sourceId : sources)
    flush(sourceId);
}
