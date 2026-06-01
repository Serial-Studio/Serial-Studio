/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/FrameBuilder.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <algorithm>
#include <cmath>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <stdexcept>

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

/**
 * @brief Returns the per-frame cadence carried by a captured chunk, clamped to >=1 ns.
 */
[[nodiscard]] std::chrono::nanoseconds capturedFrameStep(const IO::CapturedDataPtr& data)
{
  if (!data)
    return std::chrono::nanoseconds(1);

  return std::max(std::chrono::nanoseconds(1), data->frameStep);
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the FrameBuilder singleton and wires its watchdog/license hooks.
 */
DataModel::FrameBuilder::FrameBuilder()
  : m_quickPlotChannels(-1)
  , m_quickPlotHasHeader(false)
  , m_parseBudgetSkipping(false)
  , m_parseBudgetWarned(false)
  , m_parseBudgetEnabled(true)
  , m_lastConnectedState(false)
  , m_parseBudgetUsedNs(0)
  , m_parseBudgetWindowStart(BudgetClock::time_point{})
  , m_parsedFrameCount(0)
  , m_skippedFrameCount(0)
  , m_jsTransformTimedOut(false)
  , m_engineCacheSourceId(-1)
  , m_luaEngineForSource(nullptr)
  , m_jsEngineForSource(nullptr)
  , m_compileGuard(0)
  , m_compilePending(false)
  , m_framePoolHint(0)
{
  // Pre-allocate frame pool: reused across sub-frames so steady-state publishing won't malloc.
  m_framePool.reserve(kFramePoolSize);
  for (int i = 0; i < kFramePoolSize; ++i)
    m_framePool.emplace_back(std::make_shared<PooledFrameSlot>());

#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [this] { syncFromProjectModel(); });
#endif

  // Tear down engines before static destruction: QJSEngine depends on live Qt.
  if (auto* app = QCoreApplication::instance())
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() { destroyTransformEngines(); });
}

/**
 * @brief Returns the singleton FrameBuilder instance.
 */
DataModel::FrameBuilder& DataModel::FrameBuilder::instance()
{
  static FrameBuilder singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Frame pool
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-constructs a pool slot with inUse = false.
 */
DataModel::FrameBuilder::PooledFrameSlot::PooledFrameSlot() : inUse(false) {}

/**
 * @brief Claims a pool slot, copies @p src + @p ts into it, and returns a shared_ptr whose
 *        deleter releases the slot back to the pool when the last consumer drops it.
 */
DataModel::TimestampedFramePtr DataModel::FrameBuilder::acquireFrame(
  const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts)
{
  const size_t n    = m_framePool.size();
  const size_t hint = m_framePoolHint.load(std::memory_order_relaxed);

  // Raw-pointer scan: copy the shared_ptr only on success so a miss skips its two atomic ops.
  for (size_t k = 0; k < n; ++k) {
    const size_t idx = (hint + k) % n;
    auto* slotRaw    = m_framePool[idx].get();
    bool expected    = false;
    if (slotRaw->inUse.compare_exchange_strong(
          expected, true, std::memory_order_acquire, std::memory_order_relaxed)) {
      m_framePoolHint.store(idx + 1, std::memory_order_relaxed);

      // In-place assignment reuses slot vector capacity; QStrings COW-bump only.
      slotRaw->frame.data      = src;
      slotRaw->frame.timestamp = ts;

      // shared_ptr capture keeps the slot alive if a consumer outlives FrameBuilder.
      auto slotPtr = m_framePool[idx];
      return TimestampedFramePtr(&slotRaw->frame, [slotPtr](TimestampedFrame*) noexcept {
        slotPtr->inUse.store(false, std::memory_order_release);
      });
    }
  }

  // Pool exhausted: log once, fall back to heap allocation so the producer never blocks
  static bool warned = false;
  if (!warned) [[unlikely]] {
    warned = true;
    qWarning() << "[FrameBuilder] Frame pool exhausted (" << kFramePoolSize
               << " slots), consumers are not draining; falling back to heap allocation.";
    QMetaObject::invokeMethod(
      &NotificationCenter::instance(),
      "postWarning",
      Qt::QueuedConnection,
      Q_ARG(QString, QStringLiteral("FrameBuilder")),
      Q_ARG(QString, tr("Frame pool exhausted")),
      Q_ARG(QString,
            tr("A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API "
               "subscriber) is not draining frames fast enough. Serial Studio is falling "
               "back to per-frame allocations until the backlog clears. Disable a heavy "
               "consumer or reduce the data rate.")));
  }
  return std::make_shared<TimestampedFrame>(src, ts);
}

/**
 * @brief Convenience overload that timestamps the slot with SteadyClock::now().
 */
DataModel::TimestampedFramePtr DataModel::FrameBuilder::acquireFrame(const DataModel::Frame& src)
{
  return acquireFrame(src, DataModel::TimestampedFrame::SteadyClock::now());
}

//--------------------------------------------------------------------------------------------------
// Public accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current project-mode frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::frame() const noexcept
{
  return m_frame;
}

/**
 * @brief Returns the number of project frames published since the last counter reset.
 */
quint64 DataModel::FrameBuilder::parsedFrameCount() const noexcept
{
  return m_parsedFrameCount;
}

/**
 * @brief Returns the number of frames dropped by the parse-load budget since the last reset.
 */
quint64 DataModel::FrameBuilder::skippedFrameCount() const noexcept
{
  return m_skippedFrameCount;
}

/**
 * @brief Zeroes the parsed/skipped frame counters (used by the throughput benchmark).
 */
void DataModel::FrameBuilder::resetFrameCounters() noexcept
{
  m_parsedFrameCount  = 0;
  m_skippedFrameCount = 0;
}

/**
 * @brief Enables/disables the parse-load budget guard (disabled by the throughput benchmark).
 */
void DataModel::FrameBuilder::setParseBudgetEnabled(bool enabled) noexcept
{
  m_parseBudgetEnabled = enabled;
}

/**
 * @brief Returns the current Quick Plot frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::quickPlotFrame() const noexcept
{
  return m_quickPlotFrame;
}

/**
 * @brief Returns the shared DataTableStore for read-only callers (e.g. clearLookupCache).
 */
const DataModel::DataTableStore& DataModel::FrameBuilder::tableStore() const noexcept
{
  return m_tableStore;
}

//--------------------------------------------------------------------------------------------------
// External connection setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires ConnectionManager and ProjectModel signals to the FrameBuilder.
 */
void DataModel::FrameBuilder::setupExternalConnections()
{
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &DataModel::FrameBuilder::onConnectedChanged);

  // Wipe transform engines on source-delete: IDs renumber, stale refs would misfire.
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceDeleted,
          this,
          &DataModel::FrameBuilder::onSourceRemoved);

  // Periodic GC for the transform Lua/JS engines
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameBuilder::collectTransformEngineGarbage);

  // File players force a disconnect, tearing down transforms + table store; rebuild on open.
  connect(&CSV::Player::instance(), &CSV::Player::openChanged, this, [this] {
    if (CSV::Player::instance().isOpen())
      rebuildTransformsForPlayback();
  });
  connect(&MDF4::Player::instance(), &MDF4::Player::openChanged, this, [this] {
    if (MDF4::Player::instance().isOpen())
      rebuildTransformsForPlayback();
  });

#ifdef BUILD_COMMERCIAL
  // Session player bypasses ConnectionManager; rebuild engines on its openChanged.
  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, [this] {
    if (Sessions::Player::instance().isOpen())
      rebuildTransformsForPlayback();
  });
#endif
}

//--------------------------------------------------------------------------------------------------
// Project model sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds m_frame from ProjectModel's already-parsed in-memory state (no file I/O).
 */
void DataModel::FrameBuilder::syncFromProjectModel()
{
  const auto& pm = DataModel::ProjectModel::instance();
  Q_ASSERT(!pm.title().isEmpty());

  clear_frame(m_frame);
  m_sourceFrames.clear();
  m_sourceFrameCounters.clear();

  m_frame.title   = pm.title();
  m_frame.groups  = pm.groups();
  m_frame.actions = pm.actions();
  m_frame.sources = pm.sources();

  finalize_frame(m_frame);
  compileTransforms();
  initializeTableStore();
  parseBudgetReset();

  Q_ASSERT(!m_frame.title.isEmpty());

  Q_EMIT jsonFileMapChanged();
}

//--------------------------------------------------------------------------------------------------
// Quick Plot header registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers Quick Plot channel headers, or clears them when @p headers is empty.
 */
void DataModel::FrameBuilder::registerQuickPlotHeaders(const QStringList& headers)
{
  if (!headers.isEmpty()) {
    m_quickPlotHasHeader    = true;
    m_quickPlotChannelNames = headers;
  } else {
    m_quickPlotHasHeader = false;
    m_quickPlotChannelNames.clear();
  }
}

//--------------------------------------------------------------------------------------------------
// Hotpath data processing functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches a captured chunk to the parser for the current operation mode.
 */
void DataModel::FrameBuilder::hotpathRxFrame(const IO::CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  switch (AppState::instance().operationMode()) {
    case SerialStudio::QuickPlot:
      parseQuickPlotFrame(data);
      break;
    case SerialStudio::ProjectFile:
      parseProjectFrame(data);
      break;
    case SerialStudio::ConsoleOnly:
      break;
    default:
      break;
  }
}

/**
 * @brief Per-source variant of hotpathRxFrame -- routes data through the matching source parser.
 */
void DataModel::FrameBuilder::hotpathRxSourceFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile) {
    hotpathRxFrame(data);
    return;
  }

  parseProjectFrame(sourceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wipes transform engines on source deletion -- ProjectModel renumbers IDs and engines
 * recompile lazily.
 */
void DataModel::FrameBuilder::onSourceRemoved()
{
  destroyTransformEngines();
}

/**
 * @brief Builds and publishes a per-source template frame for dashboard configuration.
 */
void DataModel::FrameBuilder::publishSourceTemplateFrame(const DataModel::Source& src)
{
  DataModel::Frame srcFrame;
  srcFrame.sourceId                   = src.sourceId;
  srcFrame.title                      = m_frame.title;
  srcFrame.actions                    = m_frame.actions;
  srcFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
  for (const auto& g : m_frame.groups)
    if (g.sourceId == src.sourceId)
      srcFrame.groups.push_back(g);

  if (srcFrame.groups.empty())
    return;

  m_sourceFrames.insert(src.sourceId, srcFrame);
  hotpathTxFrame(acquireFrame(srcFrame));
}

/** @brief Handles connect/disconnect transitions: recompiles transforms, reloads parser, fires
 * auto-actions. */
void DataModel::FrameBuilder::onConnectedChanged()
{
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  // Short-circuit non-transitions: ConnectionManager re-emits connectedChanged on UI tweaks.
  const bool nowConnected = IO::ConnectionManager::instance().isConnected();
  if (nowConnected == m_lastConnectedState)
    return;

  m_lastConnectedState = nowConnected;

  // Reset quick-plot channel count
  m_quickPlotChannels = -1;

  // Re-arm the parser-load circuit breaker
  parseBudgetReset();

  // Clear per-source frames and transform engines on disconnect
  if (!nowConnected) {
    m_sourceFrames.clear();
    m_sourceFrameCounters.clear();
    destroyTransformEngines();
    m_tableStore.clear();
    return;
  }

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  // Reload parser scripts and recompile transforms for the new session
  Q_ASSERT(!m_frame.title.isEmpty());
  DataModel::FrameParser::instance().readCode();
  compileTransforms();
  initializeTableStore();

  const auto& actions = m_frame.actions;
  for (const auto& action : actions)
    if (action.autoExecuteOnConnect) {
      const qint64 written = IO::ConnectionManager::instance().writeData(get_tx_bytes(action));
      if (written < 0) [[unlikely]]
        qWarning() << "[FrameBuilder] Auto-execute writeData() failed for action:" << action.title;
    }

  // Pre-build per-source frames so the dashboard configures immediately
  const auto& sources = DataModel::ProjectModel::instance().sources();
  if (sources.size() > 1) {
    for (const auto& src : sources)
      publishSourceTemplateFrame(src);

    return;
  }

  // Single-source image-only projects also need upfront configuration
  const bool allImageGroups =
    !m_frame.groups.empty()
    && std::all_of(m_frame.groups.begin(), m_frame.groups.end(), [](const DataModel::Group& g) {
         return g.widget == QLatin1String("image");
       });

  if (allImageGroups)
    hotpathTxFrame(acquireFrame(m_frame));
}

//--------------------------------------------------------------------------------------------------
// Frame parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a project frame using the configured decoding method.
 */
void DataModel::FrameBuilder::parseProjectFrame(const IO::CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(!m_frame.groups.empty());

  if (parseBudgetSkipFrame()) [[unlikely]]
    return;

  const auto t0 = BudgetClock::now();

  QList<QStringList> multiChannels;
  decodeProjectChannels(0, /*applyPerSourceOverride=*/false, data, multiChannels);

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    const auto frameTs = data->timestamp + step * i;
    TransformFrameInfo info;
    info.sourceId    = 0;
    info.frameNumber = ++m_sourceFrameCounters[0];
    info.timestampMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    applyDatasetValues(m_frame, channels, info);
    // Pooled fan-out: one TimestampedFramePtr per parsed frame, slot recycled by deleter.
    hotpathTxFrame(acquireFrame(m_frame, frameTs));
    ++m_parsedFrameCount;
  }

  parseBudgetAccount(t0);
}

/**
 * @brief Source-aware variant of parseProjectFrame.
 */
void DataModel::FrameBuilder::parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());

  if (parseBudgetSkipFrame()) [[unlikely]]
    return;

  const auto t0 = BudgetClock::now();

  QList<QStringList> multiChannels;
  decodeProjectChannels(sourceId, /*applyPerSourceOverride=*/true, data, multiChannels);

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    DataModel::Frame& srcFrame = ensureSourceFrame(sourceId);
    const auto frameTs         = data->timestamp + step * i;
    TransformFrameInfo info;
    info.sourceId    = sourceId;
    info.frameNumber = ++m_sourceFrameCounters[sourceId];
    info.timestampMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(frameTs.time_since_epoch()).count();
    applyDatasetValues(srcFrame, channels, info);
    // Pooled fan-out: one TimestampedFramePtr per parsed frame, slot recycled by deleter.
    hotpathTxFrame(acquireFrame(srcFrame, frameTs));
  }

  parseBudgetAccount(t0);
}

//--------------------------------------------------------------------------------------------------
// Parser-load budget guard
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the parser should skip this frame to keep the GUI responsive.
 */
bool DataModel::FrameBuilder::parseBudgetSkipFrame()
{
  // Disabled by the throughput benchmark: a 100%-duty run would trip this interactive throttle.
  if (!m_parseBudgetEnabled) [[unlikely]]
    return false;

  const auto now = BudgetClock::now();

  // Lazily start the window on the first call.
  if (m_parseBudgetWindowStart == BudgetClock::time_point{}) [[unlikely]] {
    m_parseBudgetWindowStart = now;
    m_parseBudgetUsedNs      = 0;
    m_parseBudgetSkipping    = false;
    return false;
  }

  // Roll the window forward once kParseBudgetWindowMs elapses.
  const auto windowNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_parseBudgetWindowStart).count();
  if (windowNs >= static_cast<qint64>(kParseBudgetWindowMs) * 1'000'000LL) {
    m_parseBudgetWindowStart = now;
    m_parseBudgetUsedNs      = 0;
    m_parseBudgetSkipping    = false;
  }

  if (m_parseBudgetSkipping)
    ++m_skippedFrameCount;

  return m_parseBudgetSkipping;
}

/**
 * @brief Updates the parse-time accumulator and trips the breaker past the warn limit.
 */
void DataModel::FrameBuilder::parseBudgetAccount(BudgetClock::time_point startedAt)
{
  if (!m_parseBudgetEnabled) [[unlikely]]
    return;

  const auto elapsed =
    std::chrono::duration_cast<std::chrono::nanoseconds>(BudgetClock::now() - startedAt).count();
  m_parseBudgetUsedNs += elapsed;

  if (m_parseBudgetSkipping)
    return;

  const auto limitNs = static_cast<qint64>(kParseBudgetWarnLimitMs) * 1'000'000LL;
  if (m_parseBudgetUsedNs <= limitNs)
    return;

  // Over 80% of wall-clock window spent in parsing
  m_parseBudgetSkipping = true;
  qWarning() << "[FrameBuilder] Parser load exceeded budget (" << m_parseBudgetUsedNs / 1'000'000LL
             << "ms /" << kParseBudgetWindowMs << "ms)"
             << "...dropping frames until the next window rolls.";

  // Warn the user
  if (!m_parseBudgetWarned) {
    m_parseBudgetWarned = true;
    Misc::Utilities::showMessageBox(
      QObject::tr("The frame parser is using more than %1% of CPU time.")
        .arg(100 * kParseBudgetWarnLimitMs / kParseBudgetWindowMs),
      QObject::tr("Serial Studio is dropping frames to keep the application responsive. "
                  "Please simplify or optimize the frame parser script to reduce its workload."),
      QMessageBox::Warning);
  }
}

/**
 * @brief Clears the parser-budget state -- called when the active project changes.
 */
void DataModel::FrameBuilder::parseBudgetReset() noexcept
{
  m_parseBudgetWindowStart = BudgetClock::time_point{};
  m_parseBudgetUsedNs      = 0;
  m_parseBudgetSkipping    = false;
  m_parseBudgetWarned      = false;
}

/**
 * @brief Resolves the decoder method, optionally honoring a per-source override.
 */
SerialStudio::DecoderMethod DataModel::FrameBuilder::resolveDecoderMethod(
  int sourceId, bool applyPerSourceOverride) const
{
  auto& project = DataModel::ProjectModel::instance();
  if (!applyPerSourceOverride)
    return project.decoderMethod();

  for (const auto& src : project.sources())
    if (src.sourceId == sourceId)
      return static_cast<SerialStudio::DecoderMethod>(src.decoderMethod);

  return project.decoderMethod();
}

/**
 * @brief Decodes raw captured bytes into one or more channel-string frames.
 */
void DataModel::FrameBuilder::decodeProjectChannels(int sourceId,
                                                    bool applyPerSourceOverride,
                                                    const IO::CapturedDataPtr& data,
                                                    QList<QStringList>& outChannels)
{
  // Player playback: replayed rows arrive pre-CSV; reuse the QuickPlot comma split.
  if (SerialStudio::isAnyPlayerOpen()) [[unlikely]] {
    DataModel::splitQuickPlotChannels(data->data, outChannels);
    return;
  }

  // Shared decoder seam (DataModel::decodeAndParseFrame).
  decodeAndParseFrame(data->data,
                      resolveDecoderMethod(sourceId, applyPerSourceOverride),
                      DataModel::FrameParser::instance(),
                      sourceId,
                      outChannels);
}

/**
 * @brief Returns (and lazily creates) the per-source frame seeded from the project template.
 */
DataModel::Frame& DataModel::FrameBuilder::ensureSourceFrame(int sourceId)
{
  auto it = m_sourceFrames.find(sourceId);
  if (it != m_sourceFrames.end()) [[likely]]
    return it.value();

  DataModel::Frame newFrame;
  newFrame.sourceId                   = sourceId;
  newFrame.title                      = m_frame.title;
  newFrame.actions                    = m_frame.actions;
  newFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
  for (const auto& g : m_frame.groups)
    if (g.sourceId == sourceId)
      newFrame.groups.push_back(g);

  it = m_sourceFrames.insert(sourceId, std::move(newFrame));
  return it.value();
}

/**
 * @brief Updates a single dataset from its channel and any registered transform.
 */
void DataModel::FrameBuilder::applyDatasetValue(Dataset& dataset,
                                                const QString* channelData,
                                                int channelCount,
                                                const TransformFrameInfo& info,
                                                const std::unordered_map<int, int>* replayColumns)
{
  // Final-value replay: read recorded export columns by uniqueId, not parser index (virtual too).
  if (replayColumns) [[unlikely]] {
    const auto it = replayColumns->find(dataset.uniqueId);
    const int col = (it != replayColumns->end()) ? it->second : -1;
    if (col >= 0 && col < channelCount) {
      dataset.value        = channelData[col];
      dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
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
    const int idx = dataset.index;
    if (idx <= 0 || idx > channelCount) [[unlikely]]
      return;

    dataset.value        = channelData[idx - 1];
    dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
  }

  dataset.rawNumericValue = dataset.numericValue;
  dataset.rawValue        = dataset.value;

  if (m_tableStore.isInitialized())
    m_tableStore.setDatasetRaw(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

  if (!dataset.transformCode.isEmpty() && !SerialStudio::isFinalValuePlayerOpen()) [[unlikely]] {
    const auto input = dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
    const auto result = applyTransform(dataset.transformLanguage, dataset.uniqueId, input, info);

    if (result.typeId() == QMetaType::Double) {
      dataset.numericValue = result.toDouble();
      dataset.value        = QString::number(dataset.numericValue, 'g', 15);
      dataset.isNumeric    = true;
    } else {
      dataset.value     = result.toString();
      dataset.isNumeric = false;
    }
  }

  if (m_tableStore.isInitialized())
    m_tableStore.setDatasetFinal(
      dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
}

/**
 * @brief Writes channel values + transforms into every dataset of @p frame.
 */
void DataModel::FrameBuilder::applyDatasetValues(DataModel::Frame& frame,
                                                 const QStringList& channels,
                                                 const TransformFrameInfo& info)
{
  const auto* channelData = channels.data();
  const int channelCount  = channels.size();

  // Final-value replay (CSV/MDF4) reads recorded values by export-schema column for this source.
  const std::unordered_map<int, int>* replayColumns = nullptr;
  if (SerialStudio::isFinalValuePlayerOpen()) [[unlikely]] {
    const auto it = m_replayColumnMap.find(info.sourceId);
    if (it != m_replayColumnMap.end())
      replayColumns = &it->second;
  }

  // Refresh per-source engine cache on sourceId change: one map lookup per rotation.
  if (info.sourceId != m_engineCacheSourceId) [[unlikely]] {
    m_engineCacheSourceId = info.sourceId;
    auto luaIt            = m_transformEngines.find({info.sourceId, SerialStudio::Lua});
    auto jsIt             = m_transformEngines.find({info.sourceId, SerialStudio::JavaScript});
    m_luaEngineForSource  = (luaIt != m_transformEngines.end()) ? &luaIt->second : nullptr;
    m_jsEngineForSource   = (jsIt != m_transformEngines.end()) ? &jsIt->second : nullptr;
  }

  // Frame-level JS watchdog: one arm/disarm per frame; the budget covers the whole frame.
  const bool armJsWatchdog = (m_jsEngineForSource != nullptr);
  if (armJsWatchdog) [[unlikely]] {
    Q_ASSERT(m_jsEngineForSource->jsWatchdog);
    m_jsTransformTimedOut = false;
    m_jsEngineForSource->jsWatchdog->arm();
  }

  // Re-entrancy guard: pin engine storage while cached pointers into it are live
  ++m_compileGuard;

  for (auto& group : frame.groups)
    for (auto& dataset : group.datasets)
      applyDatasetValue(dataset, channelData, channelCount, info, replayColumns);

  --m_compileGuard;

  if (armJsWatchdog) [[unlikely]] {
    m_jsEngineForSource->jsWatchdog->disarm();
    if (m_jsTransformTimedOut)
      NotificationCenter::instance().postWarning(
        QStringLiteral("FrameBuilder"),
        tr("JavaScript transform exceeded budget"),
        tr("A dataset transform took longer than %1 ms; remaining datasets in the frame fell "
           "back to raw values until the next frame. Profile or simplify the transform code.")
          .arg(kTransformWatchdogMs));
  }

  // Drain any project mutation a transform queued, now that hot pointers have been released
  if (m_compileGuard == 0 && m_compilePending) [[unlikely]] {
    m_compilePending = false;
    QMetaObject::invokeMethod(this, [this] { compileTransforms(); }, Qt::QueuedConnection);
  }
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming CSV values.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const IO::CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(!data->data.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

  // Shared QuickPlot split (DataModel::splitQuickPlotChannels).
  QList<QStringList> splitRows;
  DataModel::splitQuickPlotChannels(data->data, splitRows);

  auto& channels = m_channelScratch;
  channels.clear();
  if (!splitRows.isEmpty())
    channels = splitRows.first();

  const int channelCount = channels.size();
  if (channelCount <= 0)
    return;

  if (m_quickPlotChannels == -1) {
    bool allNonNumeric = true;
    for (const auto& channel : std::as_const(channels)) {
      bool isNumeric = false;
      channel.toDouble(&isNumeric);
      if (!isNumeric)
        continue;

      allNonNumeric = false;
      break;
    }

    if (allNonNumeric) {
      m_quickPlotHasHeader    = true;
      m_quickPlotChannelNames = channels;
      return;
    }
  }

  if (channelCount != m_quickPlotChannels) [[unlikely]] {
    buildQuickPlotFrame(channels);
    m_quickPlotChannels = channelCount;
  }

  const auto* channelData = channels.constData();
  const size_t groupCount = m_quickPlotFrame.groups.size();
  for (size_t g = 0; g < groupCount; ++g) {
    auto& group               = m_quickPlotFrame.groups[g];
    const size_t datasetCount = group.datasets.size();
    for (size_t d = 0; d < datasetCount; ++d) {
      auto& dataset = group.datasets[d];
      const int idx = dataset.index;
      if (idx > 0 && idx <= channelCount) [[likely]] {
        dataset.value           = channelData[idx - 1];
        dataset.numericValue    = dataset.value.toDouble(&dataset.isNumeric);
        dataset.rawValue        = dataset.value;
        dataset.rawNumericValue = dataset.numericValue;
      }
    }
  }

  hotpathTxFrame(acquireFrame(m_quickPlotFrame, data->timestamp));
}

//--------------------------------------------------------------------------------------------------
// Quick-plot project generation functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the synthetic source row that anchors a QuickPlot frame. Always returns a
 *        non-null title so downstream exporters bound to NOT NULL columns don't reject the row.
 */
DataModel::Source DataModel::FrameBuilder::makeQuickPlotSource() const
{
  DataModel::Source src;
  src.sourceId = 0;
  src.title    = tr("Device A");
  src.busType  = static_cast<int>(IO::ConnectionManager::instance().busType());
  return src;
}

/**
 * @brief Rebuilds the Quick Plot frame structure when the channel count changes.
 */
void DataModel::FrameBuilder::buildQuickPlotFrame(const QStringList& channels)
{
  Q_ASSERT(!channels.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

#ifdef BUILD_COMMERCIAL
  const auto busType = IO::ConnectionManager::instance().busType();
  if (busType == SerialStudio::BusType::Audio) {
    buildQuickPlotAudioFrame(channels);
    return;
  }
#endif

  int idx = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.groupId   = 0;
    dataset.datasetId = idx - 1;
    dataset.uniqueId  = dataset_unique_id(0, 0, idx - 1);
    dataset.index     = idx;
    dataset.plt       = false;
    dataset.value     = channel;

    if (m_quickPlotHasHeader && idx > 0
        && idx - 1 < static_cast<int>(m_quickPlotChannelNames.size()))
      dataset.title = m_quickPlotChannelNames[idx - 1];
    else
      dataset.title = tr("Channel %1").arg(idx);

    dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
    datasets.push_back(dataset);

    ++idx;
  }

  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");
  m_quickPlotFrame.sources.push_back(makeQuickPlotSource());

  DataModel::Group datagrid;
  datagrid.groupId  = 0;
  datagrid.uniqueId = runtime_group_unique_id(0);
  datagrid.datasets = datasets;
  datagrid.title    = tr("Quick Plot Data");
  datagrid.widget   = QStringLiteral("datagrid");
  for (size_t i = 0; i < datagrid.datasets.size(); ++i)
    datagrid.datasets[i].plt = true;

  m_quickPlotFrame.groups.push_back(datagrid);

  if (datasets.size() > 1) {
    DataModel::Group multiplot;
    multiplot.groupId  = 1;
    multiplot.uniqueId = runtime_group_unique_id(1);
    multiplot.datasets = datasets;
    multiplot.title    = tr("Multiple Plots");
    multiplot.widget   = QStringLiteral("multiplot");
    for (size_t i = 0; i < multiplot.datasets.size(); ++i) {
      multiplot.datasets[i].groupId  = 1;
      multiplot.datasets[i].uniqueId = dataset_unique_id(0, 1, static_cast<int>(i));
    }

    m_quickPlotFrame.groups.push_back(multiplot);
  }

  finalize_frame(m_quickPlotFrame);
}

/**
 * @brief Builds an audio-specific Quick Plot frame with FFT configuration.
 */
void DataModel::FrameBuilder::buildQuickPlotAudioFrame(const QStringList& channels)
{
  Q_ASSERT(!channels.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

#ifdef BUILD_COMMERCIAL
  const auto* audioPtr = IO::ConnectionManager::instance().audio();
  if (!audioPtr)
    return;

  // Derive value range from the audio sample format
  const auto& audio     = *audioPtr;
  const auto format     = audio.config().capture.format;
  const auto sampleRate = audio.config().sampleRate;

  double maxValue = 1.0;
  double minValue = 0.0;
  switch (format) {
    case ma_format_u8:
      maxValue = 255;
      minValue = 0;
      break;
    case ma_format_s16:
      maxValue = 32767;
      minValue = -32768;
      break;
    case ma_format_s24:
      maxValue = 8388607;
      minValue = -8388608;
      break;
    case ma_format_s32:
      maxValue = 2147483647;
      minValue = -2147483648;
      break;
    case ma_format_f32:
      maxValue = 1.0;
      minValue = -1.0;
      break;
    default:
      break;
  }

  // FFT size: next power-of-two covering at least 50 ms of samples
  const int targetSamples = static_cast<int>(sampleRate * 0.05);
  int fftSamples          = 256;
  while (fftSamples < targetSamples && fftSamples < 8192)
    fftSamples *= 2;

  // Build datasets with FFT enabled; per-dataset plot only when mono
  const bool multipleChannels = channels.count() > 1;
  int index                   = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.fft             = true;
    dataset.plt             = !multipleChannels;
    dataset.groupId         = 0;
    dataset.datasetId       = index - 1;
    dataset.uniqueId        = dataset_unique_id(0, 0, index - 1);
    dataset.index           = index;
    dataset.value           = channel;
    dataset.pltMax          = maxValue;
    dataset.pltMin          = minValue;
    dataset.fftMax          = maxValue;
    dataset.fftMin          = minValue;
    dataset.fftSamples      = fftSamples;
    dataset.fftSamplingRate = sampleRate;

    if (m_quickPlotHasHeader && index > 0
        && index - 1 < static_cast<int>(m_quickPlotChannelNames.size()))
      dataset.title = m_quickPlotChannelNames[index - 1];
    else
      dataset.title = tr("Channel %1").arg(index);

    dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
    datasets.push_back(dataset);
    ++index;
  }

  // Assemble audio group and frame
  DataModel::Group group;
  group.groupId  = 0;
  group.uniqueId = runtime_group_unique_id(0);
  group.datasets = datasets;
  group.title    = tr("Audio Input");
  if (multipleChannels)
    group.widget = QStringLiteral("multiplot");

  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");
  m_quickPlotFrame.sources.push_back(makeQuickPlotSource());
  m_quickPlotFrame.groups.push_back(group);
  finalize_frame(m_quickPlotFrame);
#else
  Q_UNUSED(channels);
#endif
}

//--------------------------------------------------------------------------------------------------
// Hotpath data publishing functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a fully constructed DataModel frame to all registered output modules.
 */
void DataModel::FrameBuilder::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  Q_ASSERT(frame);
  Q_ASSERT(!frame->data.groups.empty());
  Q_ASSERT(!frame->data.title.isEmpty());

  static auto& csvExport     = CSV::Export::instance();
  static auto& mdf4Export    = MDF4::Export::instance();
  static auto& dashboard     = UI::Dashboard::instance();
  static auto& pluginsServer = API::Server::instance();

  dashboard.hotpathRxFrame(frame);
  csvExport.hotpathTxFrame(frame);
  mdf4Export.hotpathTxFrame(frame);
  pluginsServer.hotpathTxFrame(frame);

#ifdef BUILD_COMMERCIAL
  static auto& sqliteExport = Sessions::Export::instance();
  sqliteExport.hotpathTxFrame(frame);

  static auto& mqttPublisher = MQTT::Publisher::instance();
  mqttPublisher.hotpathTxFrame(frame);
#endif

#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  grpcServer.hotpathTxFrame(frame);
#endif
}

//--------------------------------------------------------------------------------------------------
// Per-dataset value transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the safe Lua libraries needed by transforms and strips dangerous globals.
 */
static void openSafeLibsForTransform(lua_State* L)
{
  static const luaL_Reg kSafeLibs[] = {
    {       "_G",      luaopen_base},
    {    "table",     luaopen_table},
    {   "string",    luaopen_string},
    {     "math",      luaopen_math},
    {     "utf8",      luaopen_utf8},
    {"coroutine", luaopen_coroutine},
    {    nullptr,           nullptr}
  };

  for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);
  }

  // Remove dangerous functions from base library
  for (const char* name : {"dofile", "loadfile", "load"}) {
    lua_pushnil(L);
    lua_setglobal(L, name);
  }

  // Strip string.dump: bytecode serialization paired with the loader is unsafe.
  lua_getglobal(L, "string");
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    lua_setfield(L, -2, "dump");
  }
  lua_pop(L, 1);
}

/**
 * @brief Lua LUA_MASKCOUNT hook that aborts runaway transforms via luaL_error() when the per-engine
 * deadline expires.
 */
void DataModel::FrameBuilder::transformLuaWatchdogHook(lua_State* L, lua_Debug* ar)
{
  Q_UNUSED(ar)

  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_transform__");
  auto* engine = static_cast<TransformEngine*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  if (!engine) [[unlikely]]
    return;

  if (engine->luaDeadline.hasExpired()) [[unlikely]]
    luaL_error(L, "transform timed out after %d ms", kTransformWatchdogMs);
}

/**
 * @brief Recompiles transforms + table store for a file player that bypasses ConnectionManager.
 */
void DataModel::FrameBuilder::rebuildTransformsForPlayback()
{
  if (AppState::instance().operationMode() != SerialStudio::ProjectFile || m_frame.title.isEmpty())
    return;

  compileTransforms();
  initializeTableStore();
}

/**
 * @brief Installs the per-source uniqueId->column map a file player uses for final-value replay.
 */
void DataModel::FrameBuilder::setReplayColumnMap(
  std::unordered_map<int, std::unordered_map<int, int>> map)
{
  m_replayColumnMap = std::move(map);
}

/**
 * @brief Compiles per-dataset transforms into one shared Lua/JS engine per source, caching function
 * refs.
 */
void DataModel::FrameBuilder::compileTransforms()
{
  // Defer if a frame is in flight: mutating m_transformEngines under hot pointers dangles them
  if (m_compileGuard > 0) [[unlikely]] {
    m_compilePending = true;
    return;
  }

  destroyTransformEngines();
  Q_ASSERT(m_transformEngines.empty());

  // Group transforms by (sourceId, transformLanguage). ProjectModel resolves unset (-1)
  std::map<EngineKey, std::vector<TransformEntry>> byKey;
  for (const auto& group : m_frame.groups) {
    for (const auto& ds : group.datasets) {
      if (ds.transformCode.isEmpty())
        continue;

      byKey[{ds.sourceId, ds.transformLanguage}].push_back({ds.uniqueId, ds.transformCode});
    }
  }

  if (byKey.empty())
    return;

  // Compile one engine per (source, language) key
  for (auto& [key, entries] : byKey) {
    // Insert before compile: the Lua watchdog captures &engine by pointer
    auto [it, inserted] = m_transformEngines.emplace(key, TransformEngine{});
    Q_ASSERT(inserted);
    TransformEngine& engine = it->second;

    if (key.language == SerialStudio::Lua)
      compileTransformsLua(engine, key.sourceId, entries);
    else
      compileTransformsJS(engine, key.sourceId, entries);

    // Drop empty entries when compilation produced nothing usable
    if (!engine.luaState && !engine.jsEngine)
      m_transformEngines.erase(it);
  }
}

/**
 * @brief Compiles per-dataset Lua transforms into a shared lua_State, caching refs for O(1) hotpath
 * lookup.
 */
void DataModel::FrameBuilder::compileTransformsLua(TransformEngine& engine,
                                                   int sourceId,
                                                   const std::vector<TransformEntry>& entries)
{
  lua_State* L = luaL_newstate();
  Q_ASSERT(L != nullptr);
  if (!L) [[unlikely]]
    return;

  // Replace default panic (which aborts) with one that throws -> caller's catch
  lua_atpanic(L, [](lua_State* state) -> int {
    const char* msg = lua_tostring(state, -1);
    qWarning() << "[FrameBuilder] Lua transform panic:" << (msg ? msg : "<unknown>");
    throw std::runtime_error(msg ? msg : "lua transform panic");
  });

  openSafeLibsForTransform(L);

  // Re-add Lua 5.1 / 5.2 names that 5.4 dropped (math.log10, math.pow, bit32, ...)
  DataModel::installLuaCompat(L);

  // Inject table/dataset/variant API before any user code runs
  injectTableApiLua(L);

  // Expose deviceWrite(data, sourceId) for closed-loop control from transforms
  DataModel::DeviceWriteApi::installLua(L, sourceId);

  // Expose actionFire(actionId) for firing dashboard actions from transforms
  DataModel::ActionFireApi::installLua(L);

  // Expose dashboard.* helpers (clearPlots, setPlotPoints, UI toggles, setActiveWorkspace)
  DataModel::DashboardApi::installLua(L);

  // Expose apiCall(method, params?): gated to a read-only allow-list by default
  DataModel::ScriptApiCall::installLua(L, sourceId);

  // Notification API: gated internally on the active license tier
  DataModel::NotificationCenter::installScriptApi(L);

  // Stash engine pointer in registry so the watchdog hook can find it.
  lua_pushlightuserdata(L, &engine);
  lua_setfield(L, LUA_REGISTRYINDEX, "__ss_transform__");

  // Install instruction-count watchdog
  lua_sethook(L, &FrameBuilder::transformLuaWatchdogHook, LUA_MASKCOUNT, kTransformHookInstrCount);

  // Arm the deadline while the user's compile-time chunk runs
  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  for (const auto& entry : entries)
    compileTransformsLuaEntry(L, engine, entry);

  // Disarm the deadline: subsequent arming happens per-call
  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  engine.luaState    = L;
}

/**
 * @brief Compiles a single Lua dataset transform; logs and skips on any error.
 */
void DataModel::FrameBuilder::compileTransformsLuaEntry(lua_State* L,
                                                        TransformEngine& engine,
                                                        const TransformEntry& entry)
{
  // Each dataset runs in its own _ENV table; reads fall through to _G via metatable
  const int baseTop = lua_gettop(L);

  try {
    // 1. Build per-dataset env { __index = _G }
    lua_newtable(L);                 // env
    lua_createtable(L, 0, 1);        // mt
    lua_pushglobaltable(L);          // _G
    lua_setfield(L, -2, "__index");  // mt.__index = _G
    lua_setmetatable(L, -2);         // setmetatable(env, mt)

    // 2. Load the user's chunk
    const QByteArray utf8 = entry.code.toUtf8();
    const QByteArray chunkName =
      QByteArray("=transform[") + QByteArray::number(entry.uniqueId) + "]";
    if (luaL_loadbufferx(L, utf8.constData(), utf8.size(), chunkName.constData(), "t") != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform compile error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    // 3. Re-target chunk._ENV (upvalue 1 of a main chunk in Lua 5.4) at the per-dataset env
    lua_pushvalue(L, -2);
    lua_setupvalue(L, -2, 1);

    // 4. Execute the chunk
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform runtime error for dataset" << entry.uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_settop(L, baseTop);
      return;
    }

    // 5. Look the transform up in the per-dataset env (not _G)
    lua_getfield(L, -1, "transform");
    if (!lua_isfunction(L, -1)) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      lua_settop(L, baseTop);
      return;
    }

    // 6. Detect whether transform declares an `info` parameter (lua_getinfo with ">" pops)
    bool acceptsInfo = false;
    lua_pushvalue(L, -1);
    lua_Debug ar;
    if (lua_getinfo(L, ">u", &ar) != 0) [[likely]]
      acceptsInfo = (ar.nparams >= 2);

    // 7. Release any previous ref for this dataset before overwriting
    auto existingIt = engine.luaRefs.find(entry.uniqueId);
    if (existingIt != engine.luaRefs.end()) [[unlikely]]
      luaL_unref(L, LUA_REGISTRYINDEX, existingIt->second.ref);

    // 8. Store the function ref; the per-dataset env stays alive via its _ENV upvalue
    engine.luaRefs[entry.uniqueId] = LuaTransformRef{luaL_ref(L, LUA_REGISTRYINDEX), acceptsInfo};

    // Pop the env table
    lua_pop(L, 1);
    Q_ASSERT(lua_gettop(L) == baseTop);
  } catch (const std::exception& e) {
    qWarning() << "[FrameBuilder] Transform compile uncaught exception for dataset"
               << entry.uniqueId << ":" << e.what();
    lua_settop(L, baseTop);
  } catch (...) {
    qWarning() << "[FrameBuilder] Transform compile uncaught non-std exception for dataset"
               << entry.uniqueId;
    lua_settop(L, baseTop);
  }
}

/**
 * @brief Compiles per-dataset JavaScript transforms into a shared QJSEngine; code is IIFE-wrapped
 * for isolation.
 */
void DataModel::FrameBuilder::compileTransformsJS(TransformEngine& engine,
                                                  int sourceId,
                                                  const std::vector<TransformEntry>& entries)
{
  auto* js = new QJSEngine();

  // Inject table/dataset/variant API before any user code runs
  injectTableApiJS(js);

  // Expose deviceWrite(data, sourceId?) for closed-loop control from transforms
  DataModel::DeviceWriteApi::installJS(js, sourceId);

  // Expose actionFire(actionId) for firing dashboard actions from transforms
  DataModel::ActionFireApi::installJS(js);

  // Expose dashboard.* helpers (clearPlots, setPlotPoints, UI toggles, setActiveWorkspace)
  DataModel::DashboardApi::installJS(js);

  // Expose apiCall(method, params?): gated to a read-only allow-list by default
  DataModel::ScriptApiCall::installJS(js, sourceId);

  // Notification API: gated internally on the active license tier
  DataModel::NotificationCenter::installScriptApi(js);

  for (const auto& entry : entries) {
    // Single-line IIFE prefix keeps user code on line 1 for accurate engine lineNumber
    const QString wrapped =
      QStringLiteral("(function() {%1\n"
                     ";return (typeof transform === 'function') ? transform : null;\n"
                     "})();")
        .arg(entry.code);

    // Evaluate the IIFE: its return value is the transform function
    auto evalResult = js->evaluate(wrapped);
    if (evalResult.isError()) {
      qWarning() << "[FrameBuilder] Transform compile error for"
                 << "dataset" << entry.uniqueId << "at line"
                 << evalResult.property("lineNumber").toInt() << ":"
                 << evalResult.property("message").toString();
      continue;
    }

    // Reject if the IIFE didn't yield a callable transform function
    if (!evalResult.isCallable()) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      continue;
    }

    // function.length = formal param count; 1-arg transforms skip the info-object build.
    const bool acceptsInfo        = (evalResult.property(QStringLiteral("length")).toInt() >= 2);
    engine.jsRefs[entry.uniqueId] = JsTransformRef{evalResult, acceptsInfo};
  }

  engine.jsEngine = js;
  engine.jsWatchdog =
    std::make_unique<JsWatchdog>(js, kTransformWatchdogMs, QStringLiteral("transform"));
}

/**
 * @brief Runs one GC pass over every per-source transform engine.
 */
void DataModel::FrameBuilder::collectTransformEngineGarbage()
{
  if (m_transformEngines.empty())
    return;

  for (auto& [id, engine] : m_transformEngines) {
    if (engine.luaState)
      lua_gc(engine.luaState, LUA_GCCOLLECT);

    if (engine.jsEngine)
      engine.jsEngine->collectGarbage();
  }
}

/**
 * @brief Destroys all per-source transform engines and releases resources.
 */
void DataModel::FrameBuilder::destroyTransformEngines()
{
  // Invalidate the per-source engine cache before any pointer-bearing storage moves.
  m_engineCacheSourceId = -1;
  m_luaEngineForSource  = nullptr;
  m_jsEngineForSource   = nullptr;

  // Drop the (interned-string) lookup cache before any owning Lua state closes
  m_tableStore.clearLookupCache();

  for (auto& [id, engine] : m_transformEngines) {
    // Clear JS function refs before deleting the engine.
    engine.jsRefs.clear();

    // Release each Lua registry ref before closing the state
    if (engine.luaState)
      for (const auto& [uid, ref] : engine.luaRefs)
        luaL_unref(engine.luaState, LUA_REGISTRYINDEX, ref.ref);

    engine.luaRefs.clear();

    // Release Lua state
    if (engine.luaState) {
      lua_close(engine.luaState);
      engine.luaState = nullptr;
    }

    // Unregister the watchdog before freeing the engine: it can never touch a freed engine.
    engine.jsWatchdog.reset();

    // Release JS engine (refs already cleared above)
    delete engine.jsEngine;
    engine.jsEngine = nullptr;
  }

  m_transformEngines.clear();
  Q_ASSERT(m_transformEngines.empty());
}

/**
 * @brief Applies the pre-compiled transform for a dataset; returns @p rawValue on error or missing
 * transform.
 */
QVariant DataModel::FrameBuilder::applyTransform(int language,
                                                 int uniqueId,
                                                 const QVariant& rawValue,
                                                 const TransformFrameInfo& info)
{
  Q_ASSERT(info.sourceId >= 0);
  Q_ASSERT(uniqueId >= 0);
  Q_ASSERT(info.sourceId == m_engineCacheSourceId);

  TransformEngine* engine =
    (language == SerialStudio::Lua) ? m_luaEngineForSource : m_jsEngineForSource;
  if (!engine)
    return rawValue;

  if (engine->luaState)
    return applyTransformLua(*engine, uniqueId, rawValue, info);

  if (engine->jsEngine)
    return applyTransformJs(*engine, uniqueId, rawValue, info);

  return rawValue;
}

/**
 * @brief Calls the cached Lua transform function for @p uniqueId under the per-call deadline.
 */
QVariant DataModel::FrameBuilder::applyTransformLua(TransformEngine& engine,
                                                    int uniqueId,
                                                    const QVariant& rawValue,
                                                    const TransformFrameInfo& info)
{
  auto refIt = engine.luaRefs.find(uniqueId);
  if (refIt == engine.luaRefs.end())
    return rawValue;

  lua_State* L           = engine.luaState;
  const auto& transform  = refIt->second;
  const bool acceptsInfo = transform.acceptsInfo;
  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  // C++-boundary exception guard
  try {
    lua_rawgeti(L, LUA_REGISTRYINDEX, transform.ref);
    if (rawValue.typeId() == QMetaType::Double) {
      lua_pushnumber(L, rawValue.toDouble());
    } else {
      const auto utf8 = rawValue.toString().toUtf8();
      lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    }

    // 2nd arg { frameNumber, sourceId, timestampMs }
    int argCount = 1;
    if (acceptsInfo) {
      lua_createtable(L, 0, 3);
      lua_pushinteger(L, static_cast<lua_Integer>(info.frameNumber));
      lua_setfield(L, -2, "frameNumber");
      lua_pushinteger(L, info.sourceId);
      lua_setfield(L, -2, "sourceId");
      lua_pushinteger(L, static_cast<lua_Integer>(info.timestampMs));
      lua_setfield(L, -2, "timestampMs");
      argCount = 2;
    }

    int pcallStatus = LUA_ERRRUN;
    try {
      pcallStatus = lua_pcall(L, argCount, 1, 0);
    } catch (...) {
      qWarning() << "[FrameBuilder] Uncaught exception escaped lua_pcall in transform for"
                 << uniqueId;
      try {
        lua_settop(L, 0);
        lua_pushstring(L, "uncaught Lua exception (escaped lua_pcall)");
      } catch (...) {
      }
      pcallStatus = LUA_ERRRUN;
    }
    engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);

    if (pcallStatus != LUA_OK) [[unlikely]] {
      qWarning() << "[FrameBuilder] Lua transform call failed for dataset" << uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_pop(L, 1);
      return rawValue;
    }

    if (lua_isnumber(L, -1)) {
      const double result = lua_tonumber(L, -1);
      lua_pop(L, 1);
      if (!std::isfinite(result)) [[unlikely]]
        return rawValue;

      return QVariant(result);
    }

    if (lua_isstring(L, -1)) {
      const QString result = QString::fromUtf8(lua_tostring(L, -1));
      lua_pop(L, 1);
      return QVariant(result);
    }

    lua_pop(L, 1);
    return rawValue;
  } catch (const std::exception& e) {
    qWarning() << "[FrameBuilder] applyTransformLua uncaught exception for" << uniqueId << ":"
               << e.what();
  } catch (...) {
    qWarning() << "[FrameBuilder] applyTransformLua uncaught non-std exception for" << uniqueId;
  }

  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  lua_settop(L, 0);
  return rawValue;
}

/**
 * @brief Calls the cached JS transform function for @p uniqueId under the watchdog timer.
 */
QVariant DataModel::FrameBuilder::applyTransformJs(TransformEngine& engine,
                                                   int uniqueId,
                                                   const QVariant& rawValue,
                                                   const TransformFrameInfo& info)
{
  auto refIt = engine.jsRefs.find(uniqueId);
  if (refIt == engine.jsRefs.end())
    return rawValue;

  // Watchdog is armed once per frame in applyDatasetValues, not per call.
  QJSValueList args;
  if (rawValue.typeId() == QMetaType::Double)
    args << QJSValue(rawValue.toDouble());
  else
    args << QJSValue(rawValue.toString());

  // 2nd arg { frameNumber, sourceId, timestampMs }: only built when the transform takes it.
  if (refIt->second.acceptsInfo) {
    QJSValue jsInfo = engine.jsEngine->newObject();
    jsInfo.setProperty(QStringLiteral("frameNumber"),
                       QJSValue(static_cast<double>(info.frameNumber)));
    jsInfo.setProperty(QStringLiteral("sourceId"), QJSValue(info.sourceId));
    jsInfo.setProperty(QStringLiteral("timestampMs"),
                       QJSValue(static_cast<double>(info.timestampMs)));
    args << jsInfo;
  }

  auto result = refIt->second.fn.call(args);

  if (engine.jsEngine->isInterrupted()) [[unlikely]] {
    engine.jsEngine->setInterrupted(false);
    m_jsTransformTimedOut = true;
    qWarning() << "[FrameBuilder] JS transform for dataset" << uniqueId << "timed out after"
               << kTransformWatchdogMs << "ms";
    return rawValue;
  }

  if (result.isNumber()) {
    const double val = result.toNumber();
    if (!std::isfinite(val)) [[unlikely]]
      return rawValue;

    return QVariant(val);
  }

  if (result.isString())
    return QVariant(result.toString());

  return rawValue;
}

//--------------------------------------------------------------------------------------------------
// Data table store initialization and transform API injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the DataTableStore from the project model and current frame.
 */
void DataModel::FrameBuilder::initializeTableStore()
{
  const auto& pm = DataModel::ProjectModel::instance();
  m_tableStore.initialize(pm.tables(), m_frame);
}

/**
 * @brief Re-initializes the DataTableStore from the project model's in-flight edits.
 */
void DataModel::FrameBuilder::refreshTableStoreFromProjectModel()
{
  const auto& pm = DataModel::ProjectModel::instance();
  DataModel::Frame scratch;
  scratch.title  = pm.title();
  scratch.groups = pm.groups();
  m_tableStore.initialize(pm.tables(), scratch);
}

/**
 * @brief Lua C closure for tableGet(table, reg).
 */
static int luaTableGet(lua_State* L)
{
  // Retrieve the DataTableStore pointer from upvalue
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  const auto* val = store->getByInternedKey(table, reg);
  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

/**
 * @brief Lua C closure for tableSet(table, reg, value). Cache-aware like tableGet.
 */
static int luaTableSet(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const char* table = luaL_checkstring(L, 1);
  const char* reg   = luaL_checkstring(L, 2);

  DataModel::RegisterValue rv;
  if (lua_isnumber(L, 3)) {
    rv.numericValue = lua_tonumber(L, 3);
    rv.isNumeric    = true;
  } else {
    rv.stringValue = QString::fromUtf8(luaL_checkstring(L, 3));
    rv.isNumeric   = false;
  }

  store->setByInternedKey(table, reg, rv);
  return 0;
}

/**
 * @brief Lua C closure for datasetGetRaw(uniqueId).
 */
static int luaDatasetGetRaw(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const int uid   = static_cast<int>(luaL_checkinteger(L, 1));
  const auto* val = store->getDatasetRaw(uid);

  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

/**
 * @brief Lua C closure for datasetGetFinal(uniqueId).
 */
static int luaDatasetGetFinal(lua_State* L)
{
  auto* store = static_cast<DataModel::DataTableStore*>(lua_touserdata(L, lua_upvalueindex(1)));
  Q_ASSERT(store);

  const int uid   = static_cast<int>(luaL_checkinteger(L, 1));
  const auto* val = store->getDatasetFinal(uid);

  if (!val) {
    lua_pushnil(L);
    return 1;
  }

  if (val->isNumeric) {
    lua_pushnumber(L, val->numericValue);
  } else {
    const auto utf8 = val->stringValue.toUtf8();
    lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
  }

  return 1;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Lua C function for mqttPublish(topic, payload, qos?, retain?).
 */
static int luaMqttPublish(lua_State* L)
{
  const char* topic = luaL_checkstring(L, 1);

  size_t len            = 0;
  const char* payload_d = luaL_checklstring(L, 2, &len);

  int qos = 0;
  if (lua_gettop(L) >= 3 && !lua_isnil(L, 3))
    qos = static_cast<int>(luaL_checkinteger(L, 3));

  bool retain = false;
  if (lua_gettop(L) >= 4 && !lua_isnil(L, 4))
    retain = lua_toboolean(L, 4) != 0;

  const auto id = MQTT::Publisher::instance().mqttPublish(
    QString::fromUtf8(topic), QByteArray(payload_d, static_cast<qsizetype>(len)), qos, retain);

  lua_pushinteger(L, static_cast<lua_Integer>(id));
  return 1;
}
#endif

/**
 * @brief Injects tableGet / tableSet / datasetGetRaw / datasetGetFinal into the Lua state as C
 * closures.
 */
void DataModel::FrameBuilder::injectTableApiLua(lua_State* L)
{
  Q_ASSERT(L);

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableGet, 1);
  lua_setglobal(L, "tableGet");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaTableSet, 1);
  lua_setglobal(L, "tableSet");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaDatasetGetRaw, 1);
  lua_setglobal(L, "datasetGetRaw");

  lua_pushlightuserdata(L, &m_tableStore);
  lua_pushcclosure(L, luaDatasetGetFinal, 1);
  lua_setglobal(L, "datasetGetFinal");

#ifdef BUILD_COMMERCIAL
  lua_pushcfunction(L, luaMqttPublish);
  lua_setglobal(L, "mqttPublish");
#endif
}

/**
 * @brief Injects the same four API globals into a QJSEngine via a `TableApiBridge` QObject.
 */
void DataModel::FrameBuilder::injectTableApiJS(QJSEngine* js)
{
  Q_ASSERT(js);

  // Create bridge parented to the engine (destroyed when engine is deleted)
  auto* bridge  = new DataModel::TableApiBridge(js);
  bridge->store = &m_tableStore;

  // Install as __ss global property
  auto global    = js->globalObject();
  auto bridgeVal = js->newQObject(bridge);
  global.setProperty(QStringLiteral("__ss"), bridgeVal);

  // Thin JS wrappers so user code calls clean function names
  QString jsApi =
    QStringLiteral("function tableGet(t, r)       { return __ss.tableGet(t, r); }\n"
                   "function tableSet(t, r, v)    { __ss.tableSet(t, r, v); }\n"
                   "function datasetGetRaw(uid)   { return __ss.datasetGetRaw(uid); }\n"
                   "function datasetGetFinal(uid) { return __ss.datasetGetFinal(uid); }\n");
#ifdef BUILD_COMMERCIAL
  jsApi +=
    QStringLiteral("function mqttPublish(t, p, q, r) { return __ss.mqttPublish(t, p, q, r); }\n");
#endif

  js->evaluate(jsApi);
}
