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

#include "API/Server.h"
#include "CSV/Export.h"
#include "IO/PipelineHost.h"
#include "MDF4/Export.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "InfluxDB/Export.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#  include "UI/Widgets/AudioExport.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

/**
 * @brief Binds the sink mask the replay and synthetic lanes raise; the sinks themselves arrive
 *        later, from the composition root.
 */
DataModel::BlockPublisher::BlockPublisher(const bool& maskSinks)
  : m_maskSinks(maskSinks), m_anyAsyncSink(false)
{}

/**
 * @brief Adopts the resolved sink pointers and derives the cached flag once.
 */
void DataModel::BlockPublisher::bind(const Sinks& sinks)
{
  SS_ASSERT(sinks.pipeline != nullptr, return);
  SS_ASSERT(sinks.server && sinks.csv && sinks.mdf4, return);

  m_sinks = sinks;
  refreshSinkFlag();
}

/**
 * @brief Recomputes the cached any-async-consumer flag from every export/output module. The TCP
 *        and gRPC servers only count while a client is connected: with zero clients their workers
 *        drop every frame, so the per-frame detached copy would be pure waste. A pre-bind() call
 *        leaves the flag false, which is startup ordering: nothing publishes before bind().
 */
void DataModel::BlockPublisher::refreshSinkFlag()
{
  if (!m_sinks.csv || !m_sinks.mdf4 || !m_sinks.server) {
    m_anyAsyncSink = false;
    return;
  }

  bool any = m_sinks.csv->exportEnabled() || m_sinks.mdf4->exportEnabled()
          || (m_sinks.server->enabled() && m_sinks.server->clientCount() > 0);
#ifdef BUILD_COMMERCIAL
  any = any || m_sinks.sessions->exportEnabled() || m_sinks.mqtt->enabled()
     || m_sinks.audio->hasActiveSessions() || m_sinks.influx->exportEnabled();
#endif
#ifdef ENABLE_GRPC
  any = any || (m_sinks.grpc->enabled() && m_sinks.grpc->clientCount() > 0);
#endif

  m_anyAsyncSink = any;
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
  if (!m_sinks.server) [[unlikely]]
    return false;

  const bool api = m_sinks.server->enabled() && m_sinks.server->clientCount() > 0;
#ifdef ENABLE_GRPC
  return api || (m_sinks.grpc->enabled() && m_sinks.grpc->clientCount() > 0);
#else
  return api;
#endif
}

/**
 * @brief Hands ONE trimmed copy to the read-only observers, which is all a masked block may
 *        reach: a replay or a synthetic refresh must never re-record itself.
 */
void DataModel::BlockPublisher::fanOutToObservers(const DataBlockPtr& block)
{
  SS_ASSERT_HOTPATH(block != nullptr);

  const DataBlockPtr replayed = clone_block_trimmed(*block);
  if (m_sinks.server->enabled() && m_sinks.server->clientCount() > 0)
    m_sinks.server->ingestBlock(replayed);

#ifdef ENABLE_GRPC
  if (m_sinks.grpc->enabled() && m_sinks.grpc->clientCount() > 0)
    m_sinks.grpc->ingestBlock(replayed);
#endif
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

  m_sinks.pipeline->publishBlockToDashboard(block);

  if (block->masked || m_maskSinks) [[unlikely]] {
    if (observedByReadOnly())
      fanOutToObservers(block);

    return;
  }

  if (!m_anyAsyncSink)
    return;

  const DataBlockPtr detached = clone_block_trimmed(*block);
  m_sinks.csv->ingestBlock(detached);
  m_sinks.mdf4->ingestBlock(detached);
  m_sinks.server->ingestBlock(detached);
#ifdef BUILD_COMMERCIAL
  m_sinks.sessions->ingestBlock(detached);
  m_sinks.mqtt->ingestBlock(detached);
  m_sinks.audio->ingestBlock(detached);
  m_sinks.influx->ingestBlock(detached);
#endif
#ifdef ENABLE_GRPC
  m_sinks.grpc->ingestBlock(detached);
#endif
}
