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

#include <array>
#include <cstddef>

#include "Core/DataModel/DataBlock.h"
#include "Core/DataModel/IBlockSink.h"
#include "Core/HotpathOptimization.h"
#include "DataModel/CachedFlag.h"

namespace IO {
class PipelineHost;
}  // namespace IO

namespace DataModel {

/**
 * @brief The publish fan-out of the frame builder (spec 0075, A6/R12.8): the dashboard hop, the
 *        cached any-async-sink flag and the ONE trimmed copy every recording sink shares. Sinks are
 *        bound by interface (spec 0077), so this translation unit names none of them; it runs on
 *        the pipeline thread only, which is what makes it the single producer for every sink.
 */
class BlockPublisher {
public:
  /**
   * @brief Every sink a finished block reaches. The composition root resolves them once and binds
   *        them here; the two read-only observers are also named by slot because the masked lane
   *        (replay, synthetic refresh) reaches them and nothing else.
   */
  struct Sinks {
    static constexpr std::size_t kMaxSinks = 8;

    IO::PipelineHost* pipeline = nullptr;
    std::size_t sinkCount      = 0;
    std::array<IBlockSink*, kMaxSinks> sinks{};
    IBlockSink* server = nullptr;
    IBlockSink* grpc   = nullptr;
  };

  BlockPublisher(const bool& maskSinks, CachedFlagChecker& checker);

  BlockPublisher(BlockPublisher&&)                 = delete;
  BlockPublisher(const BlockPublisher&)            = delete;
  BlockPublisher& operator=(BlockPublisher&&)      = delete;
  BlockPublisher& operator=(const BlockPublisher&) = delete;

  void bind(const Sinks& sinks);
  void refreshSinkFlag();
  void publish(const DataBlockPtr& block);

  [[nodiscard]] bool bound() const noexcept;
  [[nodiscard]] bool anyAsyncSink() const noexcept;
  [[nodiscard]] bool deriveSinkFlag() const;
  [[nodiscard]] const Sinks& sinks() const noexcept;

private:
  [[nodiscard]] bool observedByReadOnly() const;
  void fanOutToObservers(const DataBlockPtr& block);

private:
  // Binds FrameBuilder::m_maskSinks, whose address never moves; BlockStager binds the same bool
  const bool& m_maskSinks;
  Cached<bool> m_anyAsyncSink;
  Sinks m_sinks;
};

}  // namespace DataModel
