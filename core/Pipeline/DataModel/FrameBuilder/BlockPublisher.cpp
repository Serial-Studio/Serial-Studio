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

#include "DataModel/FrameBuilder/BlockPublisher.h"

#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder/BuilderFlagAudit.h"
#include "IO/PipelineHost.h"

/**
 * @brief Binds the sink mask the replay and synthetic lanes raise and registers the cached sink
 *        flag with the builder's cross-check; the sinks arrive later, from the composition root.
 */
DataModel::BlockPublisher::BlockPublisher(const bool& maskSinks, CachedFlagChecker& checker)
  : m_maskSinks(maskSinks), m_anyAsyncSink(checker, BuilderFlagAudit::kAnyAsyncSink, false)
{}

/**
 * @brief Adopts the resolved sink table and derives the cached flag once. Every bound slot is
 *        asserted non-null here so the per-block loop never tests a pointer.
 */
void DataModel::BlockPublisher::bind(const Sinks& sinks)
{
  SS_ASSERT(sinks.pipeline != nullptr, return);
  SS_ASSERT(sinks.sinkCount <= Sinks::kMaxSinks, return);

  for (std::size_t i = 0; i < sinks.sinkCount; ++i)
    SS_ASSERT(sinks.sinks[i] != nullptr, return);

  m_sinks = sinks;
  refreshSinkFlag();
}

/**
 * @brief True once a composition root bound the pipeline the dashboard hop goes through.
 */
bool DataModel::BlockPublisher::bound() const noexcept
{
  return m_sinks.pipeline != nullptr;
}

/**
 * @brief The bound sink table, for the builder's activity wiring.
 */
const DataModel::BlockPublisher::Sinks& DataModel::BlockPublisher::sinks() const noexcept
{
  return m_sinks;
}

/**
 * @brief Recomputes the cached any-async-consumer flag from every bound sink's own verdict: a
 *        streaming server counts only while a client is connected, a recorder only while enabled,
 *        so with nothing consuming the per-frame detached copy is never made. A pre-bind() call
 *        leaves the flag false, which is startup ordering: nothing publishes before bind().
 */
void DataModel::BlockPublisher::refreshSinkFlag()
{
  m_anyAsyncSink = deriveSinkFlag();
}

/**
 * @brief The any-async-consumer verdict derived from scratch: the one expression the refresh and
 *        the 1 Hz cross-check share. Every sinkActive() answers from an atomic (IBlockSink).
 */
bool DataModel::BlockPublisher::deriveSinkFlag() const
{
  SS_ASSERT(m_sinks.sinkCount <= Sinks::kMaxSinks, return true);

  bool any = false;
  for (std::size_t i = 0; i < m_sinks.sinkCount; ++i)
    any = any || m_sinks.sinks[i]->sinkActive();

  return any;
}

/**
 * @brief True while at least one recording or output sink would consume a detached copy.
 */
bool DataModel::BlockPublisher::anyAsyncSink() const noexcept
{
  return m_anyAsyncSink;
}

/**
 * @brief True when a read-only observer (the API server or gRPC) has a client attached.
 */
bool DataModel::BlockPublisher::observedByReadOnly() const
{
  const bool api  = m_sinks.server && m_sinks.server->sinkActive();
  const bool grpc = m_sinks.grpc && m_sinks.grpc->sinkActive();
  return api || grpc;
}

/**
 * @brief Hands ONE trimmed copy to the read-only observers, which is all a masked block may
 *        reach: a replay or a synthetic refresh must never re-record itself.
 */
void DataModel::BlockPublisher::fanOutToObservers(const DataBlockPtr& block)
{
  SS_ASSERT_HOTPATH(block != nullptr);

  const DataBlockPtr replayed = clone_block_trimmed(*block);
  if (m_sinks.server && m_sinks.server->sinkActive())
    m_sinks.server->ingestBlock(replayed);

  if (m_sinks.grpc && m_sinks.grpc->sinkActive())
    m_sinks.grpc->ingestBlock(replayed);
}

/**
 * @brief Publishes one finished block: the dashboard gets the pooled slot, async sinks get ONE
 *        trimmed values-only copy between them -- a queued sink must never hold a pool slot or a
 *        backlog would starve staging. While the sink mask is set only the read-only observers
 *        see it, so a replay or a synthetic refresh can never re-record itself.
 */
void DataModel::BlockPublisher::publish(const DataBlockPtr& block)
{
  SS_ASSERT_HOTPATH(block != nullptr);
  SS_ASSERT_HOTPATH(block->samples > 0);
  SS_ASSERT(m_sinks.pipeline != nullptr, return);

  m_sinks.pipeline->publishBlockToDashboard(block);

  if (block->masked || m_maskSinks) [[unlikely]] {
    if (observedByReadOnly())
      fanOutToObservers(block);

    return;
  }

  if (!m_anyAsyncSink)
    return;

  const DataBlockPtr detached = clone_block_trimmed(*block);
  for (std::size_t i = 0; i < m_sinks.sinkCount; ++i)
    m_sinks.sinks[i]->ingestBlock(detached);
}
