/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player/ReplaySynthesis.h"

#  include <algorithm>
#  include <memory>
#  include <QScopedValueRollback>
#  include <QStringList>

#  include "AppState.h"
#  include "DataModel/DataBlock.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/Scripting/FrameParserPipeline.h"
#  include "IO/ConnectionManager.h"
#  include "SerialStudio.h"
#  include "SSAssert.h"

/**
 * @brief Binds the synthesis to the reader it draws cells from, the layout that describes them,
 *        and the pipeline objects it publishes into.
 */
Sessions::ReplaySynthesis::ReplaySynthesis(SessionDbReader& reader,
                                           const ReplayLayout& layout,
                                           AppState& appState,
                                           DataModel::FrameBuilder& frameBuilder,
                                           IO::ConnectionManager& connectionManager)
  : m_injecting(false)
  , m_reader(reader)
  , m_layout(layout)
  , m_appState(appState)
  , m_frameBuilder(frameBuilder)
  , m_connectionManager(connectionManager)
{}

/**
 * @brief Returns @c true while an injection is on the stack. The player polls this to re-queue a
 *        teardown instead of freeing state the builder is still reading through borrowed views.
 */
bool Sessions::ReplaySynthesis::injecting() const noexcept
{
  return m_injecting;
}

/**
 * @brief Drops the per-session caches; the injected collaborators outlive the session.
 */
void Sessions::ReplaySynthesis::clear()
{
  m_streamBlocks     = {};
  m_streamChannelBuf = {};
}

/**
 * @brief Adopts the loaded session's dense-block index (metadata only: the sample blobs stay on
 *        disk and are fetched per block, so replay memory is flat in session length).
 */
void Sessions::ReplaySynthesis::setStreamBlocks(std::vector<PlayerStreamBlockIndex> blocks)
{
  m_streamBlocks = std::move(blocks);
}

/**
 * @brief Anchors the replay clock at the recorded instant @p rowSeconds.
 */
void Sessions::ReplaySynthesis::anchorSteadyBase(double rowSeconds)
{
  m_clock.anchor(rowSeconds);
}

/**
 * @brief Reads the cells stored at @p timestampNs and injects them, the one order the two halves
 *        are ever used in: the source set the injection fans out over is built by the read.
 */
void Sessions::ReplaySynthesis::replayFrameAt(qint64 timestampNs)
{
  const auto frame = buildFrameAt(timestampNs);
  injectFrame(frame, timestampNs);
}

/**
 * @brief Reads the replayed cells for @p timestampNs, plus the sources they belong to.
 */
Sessions::ReplayRowValues Sessions::ReplaySynthesis::buildFrameAt(qint64 timestampNs)
{
  return m_reader.readFrameValues(timestampNs, m_layout);
}

/**
 * @brief Feeds per-source cell lists in stored column order through the FrameBuilder replay fast
 *        lane (spec 0020) with the recorded timestamp; the single-source map is rekeyed to source
 *        0, matching buildMultiSourceMapping. QuickPlot mode keeps the byte path (its parser
 *        consumes raw payloads).
 */
void Sessions::ReplaySynthesis::injectFrame(const ReplayRowValues& frame, qint64 timestampNs)
{
  injectStreamBlocksAt(timestampNs);

  if (frame.values.isEmpty())
    return;

  if (m_appState.operationMode() != SerialStudio::ProjectFile) {
    QStringList cells;
    cells.reserve(static_cast<int>(m_layout.columnUniqueIds.size()));
    for (int uid : m_layout.columnUniqueIds)
      cells.append(frame.values.value(uid));

    QByteArray payload = DataModel::joinReplayRow(cells);
    payload.append('\n');
    m_connectionManager.processPayload(payload);
    return;
  }

  if (frame.sources.isEmpty())
    return;

  // code-verify off
  // The replay marshal is a plain BlockingQueuedConnection: it does NOT run this thread's event
  // loop (dataflow.md). The latch is what makes the player re-queue a close, and the loop walks
  // copies so a re-queued close cannot clear these members between iterations.
  // code-verify on
  if (m_injecting)
    return;

  const QScopedValueRollback<bool> reentry_guard(m_injecting, true);

  const auto timestamp     = m_clock.timestampFor(timestampNs);
  const auto sources       = frame.sources;
  const auto sourceColumns = m_layout.sourceColumns;

  for (int srcId : std::as_const(sources)) {
    const auto colsIt = sourceColumns.constFind(srcId);
    if (colsIt == sourceColumns.constEnd() || colsIt.value().empty())
      continue;

    QStringList cells;
    cells.reserve(static_cast<int>(colsIt.value().size()));
    for (int uid : colsIt.value())
      cells.append(frame.values.value(uid));

    m_frameBuilder.replayChannels(m_layout.multiSource ? srcId : 0, cells, timestamp);
  }
}

//--------------------------------------------------------------------------------------------------
// Stream-block replay (spec 0054)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replays every stream block whose start time is @p timestampNs. Blocks arrive as a burst
 *        with per-sample timestamps, exactly as they do live -- the recording, not the wall clock,
 *        owns the sample times.
 */
void Sessions::ReplaySynthesis::injectStreamBlocksAt(qint64 timestampNs)
{
  if (m_streamBlocks.empty())
    return;

  const auto begin = std::lower_bound(
    m_streamBlocks.begin(),
    m_streamBlocks.end(),
    timestampNs,
    [](const PlayerStreamBlockIndex& entry, qint64 ts) { return entry.t0Ns < ts; });

  std::size_t i          = static_cast<std::size_t>(begin - m_streamBlocks.begin());
  const std::size_t size = m_streamBlocks.size();
  while (i < size && m_streamBlocks[i].t0Ns == timestampNs) {
    const int sourceId   = m_streamBlocks[i].sourceId;
    std::size_t groupEnd = i;
    while (groupEnd < size && m_streamBlocks[groupEnd].t0Ns == timestampNs
           && m_streamBlocks[groupEnd].sourceId == sourceId)
      ++groupEnd;

    replayStreamGroup(sourceId, i, groupEnd);
    i = groupEnd;
  }
}

/**
 * @brief Replays one source's slice of a block: decodes each channel straight into a DataBlock and
 *        publishes it through the same tail a live source uses (spec 0055). Only this block's
 *        channels are resident. QuickPlot has no project groups, so the session's own column order
 *        stands in for the empty source-column map.
 */
void Sessions::ReplaySynthesis::replayStreamGroup(int sourceId, std::size_t first, std::size_t last)
{
  SS_ASSERT(first < m_streamBlocks.size(), return);
  SS_ASSERT(last <= m_streamBlocks.size(), return);

  const auto colsIt = m_layout.sourceColumns.constFind(sourceId);
  const bool mapped = colsIt != m_layout.sourceColumns.constEnd() && !colsIt.value().empty();
  const std::vector<int>& columns = mapped ? colsIt.value() : m_layout.columnUniqueIds;
  if (columns.empty())
    return;

  m_streamChannelBuf.resize(columns.size());
  for (auto& channel : m_streamChannelBuf)
    channel.clear();

  QHash<int, std::size_t> uidToSlot;
  for (std::size_t c = 0; c < columns.size(); ++c)
    uidToSlot.insert(columns[c], c);

  qint64 frames = 0;
  qint64 t0Ns   = m_streamBlocks[first].t0Ns;
  qint64 dtNs   = m_streamBlocks[first].dtNs;
  for (std::size_t b = first; b < last; ++b) {
    const auto& entry = m_streamBlocks[b];
    const auto slotIt = uidToSlot.constFind(entry.uniqueId);
    if (slotIt == uidToSlot.constEnd())
      continue;

    if (m_reader.fetchStreamSamples(entry, m_streamChannelBuf[slotIt.value()]))
      frames = std::max(frames, entry.frames);
  }

  if (frames <= 0)
    return;

  auto block                 = std::make_shared<DataModel::DataBlock>();
  block->sourceId            = sourceId;
  block->structureGeneration = 0;
  block->samples             = frames;
  block->t0                  = m_clock.timestampFor(t0Ns);
  block->dt                  = std::chrono::nanoseconds(dtNs > 0 ? dtNs : 1);

  block->columns.resize(columns.size());
  for (std::size_t c = 0; c < columns.size(); ++c) {
    auto& column    = block->columns[c];
    column.uniqueId = columns[c];
    column.hasText  = false;
    column.hasRaw   = false;
    column.values.assign(static_cast<std::size_t>(frames), 0.0);

    const auto& samples = m_streamChannelBuf[c];
    const auto used     = std::min(samples.size(), static_cast<std::size_t>(frames));
    std::copy_n(samples.begin(), used, column.values.begin());
  }

  m_frameBuilder.replayBlock(block);
}

#endif
