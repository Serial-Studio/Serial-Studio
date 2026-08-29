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

#include "IO/ConnectionManager/StreamWorkerPool.h"

#include <QSet>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "IO/ConnectionManager/StreamConfigBuilder.h"
#include "IO/HAL_Driver.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the pool to the single block consumer, the operation-mode source and the
 *        configuration derivation it rebuilds from.
 */
IO::StreamWorkerPool::StreamWorkerPool(DataModel::FrameBuilder& frameBuilder,
                                       AppState& appState,
                                       const StreamConfigBuilder& configs)
  : m_frameBuilder(frameBuilder), m_appState(appState), m_configs(configs)
{}

/**
 * @brief Joins every worker still running. ModuleManager stops the pool explicitly before the
 *        session context releases anything, so this is the last-resort path only.
 */
IO::StreamWorkerPool::~StreamWorkerPool()
{
  stop();
}

//--------------------------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the per-source workers: one per source whose lane is on (driver default +
 *        per-source override, R6); old workers stop first (bounded join). The driver's lane flag
 *        is set only once a worker will exist -- a lane with no channel-bound datasets would
 *        otherwise leave that source dark.
 */
void IO::StreamWorkerPool::rebuild(const std::vector<Source>& sources, bool paused, bool connected)
{
  stop();

  for (const auto& source : sources) {
    HAL_Driver* halDriver = source.driver;
    SS_ASSERT_LOG(halDriver != nullptr);
    if (!halDriver)
      continue;

    const bool laneOn = IO::streamLaneOn(halDriver, m_configs.streamLane(source.deviceId));
    auto config = laneOn ? m_configs.streamConfig(source.deviceId, halDriver) : StreamConfig{};
    const bool active = laneOn && !config.datasets.empty();

#ifdef BUILD_COMMERCIAL
    if (auto* audioDriver = qobject_cast<IO::Drivers::Audio*>(halDriver))
      audioDriver->setStreamLaneActive(active);
#endif

    if (!active)
      continue;

    auto worker = std::make_unique<StreamWorker>(halDriver, config, &m_frameBuilder, nullptr);
    worker->setPaused(paused);
    wireSinks(*worker);
    m_workers.push_back(std::move(worker));
  }

  refreshExportFlags();

  QSet<int> streamSourceIds;
  for (const auto& worker : m_workers)
    if (worker)
      streamSourceIds.insert(worker->sourceId());

  m_frameBuilder.setStreamSourceIds(streamSourceIds);

  if (!m_workers.empty() && connected)
    publishTemplates();
}

/**
 * @brief Stops and destroys every worker (idempotent; called on rebuilds and at quit from
 *        ModuleManager::stopFrameConsumerWorkers before SessionContext::shutdown).
 */
void IO::StreamWorkerPool::stop()
{
  for (auto& worker : m_workers)
    if (worker)
      worker->stop();

  m_workers.clear();
}

/**
 * @brief Mirrors the session pause onto every worker's pause atomic.
 */
void IO::StreamWorkerPool::setPaused(bool paused)
{
  for (auto& worker : m_workers)
    if (worker)
      worker->setPaused(paused);
}

/**
 * @brief Returns the live workers (GUI thread only; Dashboard drains their display rings on the
 *        display tick).
 */
const std::vector<std::unique_ptr<IO::StreamWorker>>& IO::StreamWorkerPool::workers() const noexcept
{
  return m_workers;
}

//--------------------------------------------------------------------------------------------------
// Sink wiring & publication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Connects one worker's block-rate outputs, both to the FrameBuilder on the pipeline
 *        thread (spec 0055 D8): blocks join the frame lane's publish tail so the pipeline stays
 *        the SINGLE producer for every sink, and latest values keep feeding the data-table store
 *        whose single writer is that same thread.
 */
void IO::StreamWorkerPool::wireSinks(StreamWorker& worker) const
{
  auto* processor = worker.processor();
  SS_ASSERT(processor != nullptr, return);

  QObject::connect(processor,
                   &IO::StreamProcessor::blockReady,
                   &m_frameBuilder,
                   &DataModel::FrameBuilder::ingestStreamBlock,
                   Qt::QueuedConnection);

  QObject::connect(processor,
                   &IO::StreamProcessor::latestValuesReady,
                   &m_frameBuilder,
                   &DataModel::FrameBuilder::ingestStreamValues,
                   Qt::QueuedConnection);
}

/**
 * @brief Re-derives the FrameBuilder's cached any-async-sink flag when a typed sink's enable
 *        state moves. Since spec 0055 D8 both lanes publish through that one flag, so this no
 *        longer pushes a per-worker export gate -- but it must still fire for every sink, or a
 *        recording produces a valid-looking file containing nothing.
 */
void IO::StreamWorkerPool::refreshExportFlags() const
{
  m_frameBuilder.refreshAsyncSinks();
}

/**
 * @brief Publishes the dashboard structure for every stream source so widget models build
 *        before display updates arrive: per-source template frames in ProjectFile mode, the
 *        synthesized audio frame in QuickPlot (spec 0051 T25).
 */
void IO::StreamWorkerPool::publishTemplates() const
{
  SS_ASSERT_LOG(!m_workers.empty());

  auto* frameBuilder = &m_frameBuilder;
  if (m_appState.operationMode() == SerialStudio::ProjectFile) {
    for (const auto& worker : m_workers) {
      const int sourceId = worker->sourceId();
      frameBuilder->invokeOnBuilderThread(
        [frameBuilder, sourceId] { frameBuilder->publishSourceTemplate(sourceId); });
    }

    return;
  }

  for (const auto& worker : m_workers) {
    const int channels = worker->config().channels;
    frameBuilder->invokeOnBuilderThread(
      [frameBuilder, channels] { frameBuilder->publishQuickPlotAudioTemplate(channels); });
  }
}
