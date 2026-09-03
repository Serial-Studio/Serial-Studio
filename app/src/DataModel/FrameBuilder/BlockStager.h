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

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/Frame.h"
#include "DataModel/HotpathOptimization.h"

namespace DataModel {

/**
 * @brief The four facade hooks block staging needs: the structure announcement that must precede a
 *        source's first block, the sink fan-out a flush publishes into, the pool-exhaustion note,
 *        and the display tick's flush epoch. FrameBuilder implements it; the unit tier substitutes
 *        a stub, which is why the stager reaches for no singleton of its own.
 */
class BlockStagerHost {
public:
  BlockStagerHost()                                  = default;
  BlockStagerHost(BlockStagerHost&&)                 = delete;
  BlockStagerHost(const BlockStagerHost&)            = delete;
  BlockStagerHost& operator=(BlockStagerHost&&)      = delete;
  BlockStagerHost& operator=(const BlockStagerHost&) = delete;
  virtual ~BlockStagerHost();

  virtual void noteStagingPoolExhausted()                                   = 0;
  virtual void publishStagedBlock(const DataModel::DataBlockPtr& block)     = 0;
  virtual void announceStructure(int sourceId, const DataModel::Frame& src) = 0;
  [[nodiscard]] virtual quint64 stagingFlushEpoch() const                   = 0;
};

/**
 * @brief Frame-lane block staging (spec 0055): the pooled block slots, the per-source open-block
 *        map and the cap/epoch flush rules, owned by FrameBuilder as a concern sub-object. Every
 *        method is pipeline-thread only -- that is what makes the pool's use_count()==1 free probe
 *        exact, and why none of this state carries a mutex or an atomic.
 */
class BlockStager {
public:
  // Frame-lane flush cap (spec 0055 D1/D6); low because these columns carry a string per sample
  static constexpr qsizetype kFrameBlockSampleCap = 64;

  static constexpr int kBlockPoolSlots = 64;

  BlockStager(BlockStagerHost& host, const quint64& generation, const bool& maskSinks);
  BlockStager(BlockStager&&)                 = delete;
  BlockStager(const BlockStager&)            = delete;
  BlockStager& operator=(BlockStager&&)      = delete;
  BlockStager& operator=(const BlockStager&) = delete;

  [[nodiscard]] quint64 blockNumber(int sourceId) const noexcept;

  void refreshBudget(const DataModel::Frame& src) noexcept;
  SS_HOT void stage(int sourceId,
                    const DataModel::Frame& src,
                    const DataModel::TimestampedFrame::SteadyTimePoint& ts);
  void flush(int sourceId);
  void flushAll();
  void releaseIdleStorage();

private:
  /**
   * @brief Recyclable pool slot holding one staged DataBlock plus the generation and source it
   *        is bound to. A slot is free exactly when the pool's shared_ptr is its only reference,
   *        so the builder keeps its own reference while a block is open and hands out an aliasing
   *        shared_ptr on publish -- no per-block control block, and no other source can steal a
   *        slot that is still filling.
   */
  struct PooledBlockSlot {
    PooledBlockSlot();
    DataModel::DataBlock block;
    quint64 generation;
    quint64 flushEpoch;
    int sourceId;
  };

  [[nodiscard]] std::shared_ptr<PooledBlockSlot> claimSlot(int sourceId) noexcept;
  void bindToFrame(PooledBlockSlot& slot, const DataModel::Frame& src, bool uniform);
  [[nodiscard]] PooledBlockSlot* openBlockFor(int sourceId, const DataModel::Frame& src);

  // Memory ceiling for materialised block slots; a slot's storage scales with the dataset count
  static constexpr std::size_t kBlockPoolBudgetBytes = 192ULL * 1024ULL * 1024ULL;

  BlockStagerHost& m_host;
  const quint64& m_generation;
  const bool& m_maskSinks;

  std::vector<std::shared_ptr<PooledBlockSlot>> m_pool;
  std::map<int, std::shared_ptr<PooledBlockSlot>> m_open;
  std::map<int, quint64> m_blockNumbers;
  std::size_t m_poolHint;
  int m_slotsUsable;
};

}  // namespace DataModel
