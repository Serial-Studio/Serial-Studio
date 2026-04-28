/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "DataModel/FrameParser.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

/**
 * @brief Hotpath CSV split — comma delimiter only; users needing other delimiters write a custom
 * parser.
 */
void parseCsvValues(const QByteArray& data, QStringList& out, const int reserveHint)
{
  out.clear();
  if (reserveHint > 0)
    out.reserve(reserveHint);
  else
    out.reserve(64);

  const char* raw      = data.constData();
  const int dataLength = data.size();
  int start            = 0;
  for (int i = 0; i <= dataLength; ++i) {
    if (i == dataLength || raw[i] == ',') {
      int s = start;
      int e = i;
      while (s < e && (raw[s] == ' ' || raw[s] == '\t' || raw[s] == '\r' || raw[s] == '\n'))
        ++s;
      while (
        e > s
        && (raw[e - 1] == ' ' || raw[e - 1] == '\t' || raw[e - 1] == '\r' || raw[e - 1] == '\n'))
        --e;

      if (e > s)
        out.append(QString::fromUtf8(raw + s, e - s));
      else
        out.append(QString());

      start = i + 1;
    }
  }
}

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
DataModel::FrameBuilder::FrameBuilder() : m_quickPlotChannels(-1), m_quickPlotHasHeader(false)
{
  // JS transform watchdog — flips interrupt flag to unwind runaway scripts.
  m_jsTransformWatchdog.setSingleShot(true);
  m_jsTransformWatchdog.setInterval(kTransformWatchdogMs);
  connect(&m_jsTransformWatchdog, &QTimer::timeout, this, [this]() {
    for (auto& [id, engine] : m_transformEngines)
      if (engine.jsEngine)
        engine.jsEngine->setInterrupted(true);
  });

#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          [this] { syncFromProjectModel(); });
#endif

  // Tear down engines before static destruction — QJSEngine depends on live Qt.
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
 * @brief Returns the current Quick Plot frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::quickPlotFrame() const noexcept
{
  return m_quickPlotFrame;
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

  // Source IDs get renumbered on delete — wipe transform engines so stale
  // keys can't fire the wrong Lua/JS ref on the next frame. Engines
  // recompile lazily on the next connect or project sync.
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceDeleted,
          this,
          &DataModel::FrameBuilder::onSourceRemoved);

#ifdef BUILD_COMMERCIAL
  // Session player opens don't go through ConnectionManager — rebuild transform
  // engines on its openChanged so replay runs transforms with notify API intact
  connect(&Sessions::Player::instance(), &Sessions::Player::openChanged, this, [this] {
    if (Sessions::Player::instance().isOpen()
        && AppState::instance().operationMode() == SerialStudio::ProjectFile
        && !m_frame.title.isEmpty()) {
      compileTransforms();
      initializeTableStore();
    }
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

  m_frame.title   = pm.title();
  m_frame.groups  = pm.groups();
  m_frame.actions = pm.actions();

  finalize_frame(m_frame);
  compileTransforms();
  initializeTableStore();

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
  Q_ASSERT(data->data);
  Q_ASSERT(!data->data->isEmpty());
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
 * @brief Per-source variant of hotpathRxFrame — routes data through the matching source parser.
 */
void DataModel::FrameBuilder::hotpathRxSourceFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(data->data);
  Q_ASSERT(!data->data->isEmpty());

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
 * @brief Wipes transform engines on source deletion — ProjectModel renumbers IDs and engines
 * recompile lazily.
 */
void DataModel::FrameBuilder::onSourceRemoved()
{
  destroyTransformEngines();
}

/** @brief Handles connect/disconnect transitions: recompiles transforms, reloads parser, fires
 * auto-actions. */
void DataModel::FrameBuilder::onConnectedChanged()
{
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  // Reset quick-plot channel count
  m_quickPlotChannels = -1;

  // Clear per-source frames and transform engines on disconnect
  if (!IO::ConnectionManager::instance().isConnected()) {
    m_sourceFrames.clear();
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
    for (const auto& src : sources) {
      DataModel::Frame srcFrame;
      srcFrame.sourceId                   = src.sourceId;
      srcFrame.title                      = m_frame.title;
      srcFrame.actions                    = m_frame.actions;
      srcFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
      for (const auto& g : m_frame.groups)
        if (g.sourceId == src.sourceId)
          srcFrame.groups.push_back(g);

      if (!srcFrame.groups.empty()) {
        m_sourceFrames.insert(src.sourceId, srcFrame);
        hotpathTxFrame(std::make_shared<DataModel::TimestampedFrame>(srcFrame));
      }
    }

    return;
  }

  // Single-source image-only projects also need upfront configuration
  const bool allImageGroups =
    !m_frame.groups.empty()
    && std::all_of(m_frame.groups.begin(), m_frame.groups.end(), [](const DataModel::Group& g) {
         return g.widget == QLatin1String("image");
       });

  if (allImageGroups)
    hotpathTxFrame(std::make_shared<DataModel::TimestampedFrame>(m_frame));
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
  Q_ASSERT(data->data);
  Q_ASSERT(!data->data->isEmpty());
  Q_ASSERT(!m_frame.groups.empty());

  // Decode via JS parser or CSV fallback
  QList<QStringList> multiChannels;

  if (!SerialStudio::isAnyPlayerOpen()) [[likely]] {
    auto& parser             = DataModel::FrameParser::instance();
    const auto decoderMethod = DataModel::ProjectModel::instance().decoderMethod();

    switch (decoderMethod) {
      case SerialStudio::Hexadecimal:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data->data->toHex()), 0);
        break;
      case SerialStudio::Base64:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data->data->toBase64()), 0);
        break;
      case SerialStudio::Binary:
        multiChannels = parser.parseMultiFrame(*data->data, 0);
        break;
      case SerialStudio::PlainText:
      default:
        multiChannels = parser.parseMultiFrame(QString::fromUtf8(*data->data), 0);
        break;
    }
  } else {
    auto& channels = m_channelScratch;
    parseCsvValues(*data->data, channels, 64);
    multiChannels.append(channels);
  }

  auto applyChannelData = [this](const QStringList& chs, int srcId) {
    const auto* channelData = chs.data();
    const int channelCount  = chs.size();

    // Reset computed registers at the start of each frame
    if (m_tableStore.isInitialized())
      m_tableStore.resetComputedRegisters();

    for (auto& group : m_frame.groups) {
      for (auto& dataset : group.datasets) {
        if (dataset.virtual_) {
          // Virtual datasets have no source data — reset to defaults so a
          // missing transform doesn't leak last frame's values. The transform
          // runs below and supplies the real final value.
          dataset.numericValue = 0.0;
          dataset.value.clear();
          dataset.isNumeric = true;
        } else {
          const int idx = dataset.index;
          if (idx <= 0 || idx > channelCount) [[unlikely]]
            continue;

          dataset.value        = channelData[idx - 1];
          dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
        }

        // Snapshot raw values before any transform
        dataset.rawNumericValue = dataset.numericValue;
        dataset.rawValue        = dataset.value;

        // Write raw values to the system dataset table
        if (m_tableStore.isInitialized())
          m_tableStore.setDatasetRaw(
            dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

        // Apply transform (skip for CSV/MDF4 playback — those stores hold transformed values)
        if (!dataset.transformCode.isEmpty() && !SerialStudio::isFinalValuePlayerOpen())
          [[unlikely]] {
          const auto input =
            dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
          const auto result =
            applyTransform(srcId, dataset.transformLanguage, dataset.uniqueId, input);

          if (result.typeId() == QMetaType::Double) {
            dataset.numericValue = result.toDouble();
            dataset.value        = QString::number(dataset.numericValue, 'g', 15);
            dataset.isNumeric    = true;
          } else {
            dataset.value     = result.toString();
            dataset.isNumeric = false;
          }
        }

        // Write final values to the system dataset table
        if (m_tableStore.isInitialized())
          m_tableStore.setDatasetFinal(
            dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
      }
    }
  };

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    applyChannelData(channels, 0);
    hotpathTxFrame(
      std::make_shared<DataModel::TimestampedFrame>(m_frame, data->timestamp + step * i));
  }
}

/**
 * @brief Source-aware variant of parseProjectFrame.
 */
void DataModel::FrameBuilder::parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(data);
  Q_ASSERT(data->data);
  Q_ASSERT(!data->data->isEmpty());

  // Decode via source-specific parser
  QList<QStringList> multiChannels;

  if (!SerialStudio::isAnyPlayerOpen()) [[likely]] {
    auto& parser = DataModel::FrameParser::instance();

    SerialStudio::DecoderMethod decoderMethod = DataModel::ProjectModel::instance().decoderMethod();
    const auto& sources                       = DataModel::ProjectModel::instance().sources();
    for (const auto& src : sources) {
      if (src.sourceId == sourceId) {
        decoderMethod = static_cast<SerialStudio::DecoderMethod>(src.decoderMethod);
        break;
      }
    }

    switch (decoderMethod) {
      case SerialStudio::Hexadecimal:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data->data->toHex()), sourceId);
        break;
      case SerialStudio::Base64:
        multiChannels =
          parser.parseMultiFrame(QString::fromLatin1(data->data->toBase64()), sourceId);
        break;
      case SerialStudio::Binary:
        multiChannels = parser.parseMultiFrame(*data->data, sourceId);
        break;
      case SerialStudio::PlainText:
      default:
        multiChannels = parser.parseMultiFrame(QString::fromUtf8(*data->data), sourceId);
        break;
    }
  } else {
    auto& channels = m_channelScratch;
    parseCsvValues(*data->data, channels, 64);
    multiChannels.append(channels);
  }

  auto applyChannelData = [this, sourceId](const QStringList& chs) {
    const auto* channelData = chs.data();
    const int channelCount  = chs.size();

    // Create source frame on first encounter (single lookup)
    auto it = m_sourceFrames.find(sourceId);
    if (it == m_sourceFrames.end()) [[unlikely]] {
      DataModel::Frame newFrame;
      newFrame.sourceId                   = sourceId;
      newFrame.title                      = m_frame.title;
      newFrame.actions                    = m_frame.actions;
      newFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
      for (const auto& g : m_frame.groups)
        if (g.sourceId == sourceId)
          newFrame.groups.push_back(g);

      it = m_sourceFrames.insert(sourceId, std::move(newFrame));
    }

    // Reset computed registers per frame — shared DataTableStore state across sources.
    if (m_tableStore.isInitialized())
      m_tableStore.resetComputedRegisters();

    DataModel::Frame& srcFrame = it.value();
    for (auto& group : srcFrame.groups) {
      for (auto& dataset : group.datasets) {
        if (dataset.virtual_) {
          // Reset virtual datasets so a missing transform can't leak last
          // frame's value; the transform below supplies the real output.
          dataset.numericValue = 0.0;
          dataset.value.clear();
          dataset.isNumeric = true;
        } else {
          const int idx = dataset.index;
          if (idx <= 0 || idx > channelCount) [[unlikely]]
            continue;

          dataset.value        = channelData[idx - 1];
          dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
        }

        // Snapshot raw values before any transform
        dataset.rawNumericValue = dataset.numericValue;
        dataset.rawValue        = dataset.value;

        // Write raw values to the system dataset table
        if (m_tableStore.isInitialized())
          m_tableStore.setDatasetRaw(
            dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);

        // Apply transform (skip for CSV/MDF4 playback — those stores hold transformed values)
        if (!dataset.transformCode.isEmpty() && !SerialStudio::isFinalValuePlayerOpen())
          [[unlikely]] {
          const auto input =
            dataset.isNumeric ? QVariant(dataset.numericValue) : QVariant(dataset.value);
          const auto result =
            applyTransform(sourceId, dataset.transformLanguage, dataset.uniqueId, input);

          if (result.typeId() == QMetaType::Double) {
            dataset.numericValue = result.toDouble();
            dataset.value        = QString::number(dataset.numericValue, 'g', 15);
            dataset.isNumeric    = true;
          } else {
            dataset.value     = result.toString();
            dataset.isNumeric = false;
          }
        }

        // Write final values to the system dataset table
        if (m_tableStore.isInitialized())
          m_tableStore.setDatasetFinal(
            dataset.uniqueId, dataset.numericValue, dataset.value, dataset.isNumeric);
      }
    }
  };

  const auto step = capturedFrameStep(data);
  for (int i = 0; i < multiChannels.size(); ++i) {
    const auto& channels = multiChannels.at(i);
    if (channels.isEmpty()) [[unlikely]]
      continue;

    applyChannelData(channels);

    auto txIt = m_sourceFrames.find(sourceId);
    if (txIt != m_sourceFrames.end()) [[likely]]
      hotpathTxFrame(
        std::make_shared<DataModel::TimestampedFrame>(txIt.value(), data->timestamp + step * i));
  }
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming CSV values.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const IO::CapturedDataPtr& data)
{
  Q_ASSERT(data);
  Q_ASSERT(data->data);
  Q_ASSERT(!data->data->isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

  auto& channels        = m_channelScratch;
  const int reserveHint = (m_quickPlotChannels > 0) ? m_quickPlotChannels : 64;
  parseCsvValues(*data->data, channels, reserveHint);

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

  hotpathTxFrame(std::make_shared<DataModel::TimestampedFrame>(m_quickPlotFrame, data->timestamp));
}

//--------------------------------------------------------------------------------------------------
// Quick-plot project generation functions
//--------------------------------------------------------------------------------------------------

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

  DataModel::Group datagrid;
  datagrid.groupId  = 0;
  datagrid.datasets = datasets;
  datagrid.title    = tr("Quick Plot Data");
  datagrid.widget   = QStringLiteral("datagrid");
  for (size_t i = 0; i < datagrid.datasets.size(); ++i)
    datagrid.datasets[i].plt = true;

  m_quickPlotFrame.groups.push_back(datagrid);

  if (datasets.size() > 1) {
    DataModel::Group multiplot;
    multiplot.groupId  = 1;
    multiplot.datasets = datasets;
    multiplot.title    = tr("Multiple Plots");
    multiplot.widget   = QStringLiteral("multiplot");
    for (size_t i = 0; i < multiplot.datasets.size(); ++i)
      multiplot.datasets[i].groupId = 1;

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

  // Build datasets with FFT and plot enabled
  int index = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.fft             = true;
    dataset.plt             = true;
    dataset.groupId         = 0;
    dataset.datasetId       = index - 1;
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
  group.datasets = datasets;
  group.title    = tr("Audio Input");
  if (index > 2)
    group.widget = QStringLiteral("multiplot");

  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");
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
    {    "_G",   luaopen_base},
    { "table",  luaopen_table},
    {"string", luaopen_string},
    {  "math",   luaopen_math},
    { nullptr,        nullptr}
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

  // Strip string.dump — lets user code serialise bytecode, which combined
  // with the runtime's loader can inject unverified code.
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

  // Retrieve the engine pointer from the registry
  lua_getfield(L, LUA_REGISTRYINDEX, "__ss_transform__");
  auto* engine = static_cast<TransformEngine*>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  // Defensive — registry entry should always be present for our states
  if (!engine) [[unlikely]]
    return;

  // Abort the current Lua call if the per-call deadline has expired
  if (engine->luaDeadline.hasExpired()) [[unlikely]]
    luaL_error(L, "transform timed out after %d ms", kTransformWatchdogMs);
}

/**
 * @brief Compiles per-dataset transforms into one shared Lua/JS engine per source, caching function
 * refs.
 */
void DataModel::FrameBuilder::compileTransforms()
{
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
    // Insert before compile — the Lua watchdog captures &engine by pointer
    auto [it, inserted] = m_transformEngines.emplace(key, TransformEngine{});
    Q_ASSERT(inserted);
    TransformEngine& engine = it->second;

    if (key.language == SerialStudio::Lua)
      compileTransformsLua(engine, entries);
    else
      compileTransformsJS(engine, entries);

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
                                                   const std::vector<TransformEntry>& entries)
{
  // Create a shared Lua state for all transforms in this source
  lua_State* L = luaL_newstate();
  Q_ASSERT(L != nullptr);
  if (!L) [[unlikely]]
    return;

  openSafeLibsForTransform(L);

  // Inject table/dataset/variant API before any user code runs
  injectTableApiLua(L);

  // Notification API — gated internally on the active license tier
  DataModel::NotificationCenter::installScriptApi(L);

  // Store the engine pointer in the Lua registry so the watchdog
  // hook can look it up without needing a global singleton pointer
  lua_pushlightuserdata(L, &engine);
  lua_setfield(L, LUA_REGISTRYINDEX, "__ss_transform__");

  // Install instruction-count watchdog
  lua_sethook(L, &FrameBuilder::transformLuaWatchdogHook, LUA_MASKCOUNT, kTransformHookInstrCount);

  // Arm the deadline while the user's compile-time chunk runs
  engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

  for (const auto& entry : entries) {
    // Execute the user's code, which defines transform(value)
    const QByteArray utf8 = entry.code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      qWarning() << "[FrameBuilder] Transform compile error for"
                 << "dataset" << entry.uniqueId << ":" << lua_tostring(L, -1);
      lua_pop(L, 1);
      continue;
    }

    // Clear any values the chunk may have returned
    lua_settop(L, 0);

    // Verify that the user defined a transform() function
    lua_getglobal(L, "transform");
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 1);
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      continue;
    }

    // Move transform → __tf_<id> so the next dataset can reuse the name
    const auto alias = QStringLiteral("__tf_%1").arg(entry.uniqueId);
    lua_setglobal(L, alias.toUtf8().constData());
    lua_pushnil(L);
    lua_setglobal(L, "transform");

    // Store a registry reference for O(1) hotpath lookup
    lua_getglobal(L, alias.toUtf8().constData());
    Q_ASSERT(lua_isfunction(L, -1));

    // Release any previous ref for this dataset before overwriting
    auto existingIt = engine.luaRefs.find(entry.uniqueId);
    if (existingIt != engine.luaRefs.end()) [[unlikely]]
      luaL_unref(L, LUA_REGISTRYINDEX, existingIt->second);

    engine.luaRefs[entry.uniqueId] = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  // Disarm the deadline — subsequent arming happens per-call
  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  engine.luaState    = L;
}

/**
 * @brief Compiles per-dataset JavaScript transforms into a shared QJSEngine; code is IIFE-wrapped
 * for isolation.
 */
void DataModel::FrameBuilder::compileTransformsJS(TransformEngine& engine,
                                                  const std::vector<TransformEntry>& entries)
{
  // Create a shared QJSEngine for all transforms in this source
  auto* js = new QJSEngine();

  // Inject table/dataset/variant API before any user code runs
  injectTableApiJS(js);

  // Notification API — gated internally on the active license tier
  DataModel::NotificationCenter::installScriptApi(js);

  for (const auto& entry : entries) {
    // Wrap the user's code in an IIFE for per-dataset closure isolation
    const QString wrapped =
      QStringLiteral("(function() {\n"
                     "%1\n"
                     "return (typeof transform === 'function') ? transform : null;\n"
                     "})();")
        .arg(entry.code);

    // Evaluate the IIFE — its return value is the transform function
    auto evalResult = js->evaluate(wrapped);
    if (evalResult.isError()) {
      qWarning() << "[FrameBuilder] Transform compile error for"
                 << "dataset" << entry.uniqueId << ":" << evalResult.property("message").toString();
      continue;
    }

    // Reject if the IIFE didn't yield a callable transform function
    if (!evalResult.isCallable()) {
      qWarning() << "[FrameBuilder] Dataset" << entry.uniqueId
                 << "transform code does not define transform()";
      continue;
    }

    engine.jsRefs[entry.uniqueId] = evalResult;
  }

  engine.jsEngine = js;
}

/**
 * @brief Destroys all per-source transform engines and releases resources.
 */
void DataModel::FrameBuilder::destroyTransformEngines()
{
  for (auto& [id, engine] : m_transformEngines) {
    // Clear JS function refs BEFORE deleting the engine — QJSValue
    // destructors access the engine's internal state
    engine.jsRefs.clear();

    // Release each Lua registry ref before closing the state
    if (engine.luaState)
      for (const auto& [uid, ref] : engine.luaRefs)
        luaL_unref(engine.luaState, LUA_REGISTRYINDEX, ref);
    engine.luaRefs.clear();

    // Release Lua state
    if (engine.luaState) {
      lua_close(engine.luaState);
      engine.luaState = nullptr;
    }

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
QVariant DataModel::FrameBuilder::applyTransform(int sourceId,
                                                 int language,
                                                 int uniqueId,
                                                 const QVariant& rawValue)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(uniqueId >= 0);

  auto engineIt = m_transformEngines.find({sourceId, language});

  if (engineIt == m_transformEngines.end())
    return rawValue;

  auto& engine = engineIt->second;

  // Lua transform path
  if (engine.luaState) {
    auto refIt = engine.luaRefs.find(uniqueId);
    if (refIt == engine.luaRefs.end())
      return rawValue;

    // Arm the per-call deadline
    lua_State* L = engine.luaState;
    engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

    // Push the function and argument
    lua_rawgeti(L, LUA_REGISTRYINDEX, refIt->second);
    if (rawValue.typeId() == QMetaType::Double) {
      lua_pushnumber(L, rawValue.toDouble());
    } else {
      // QByteArray holds the UTF-8 buffer alive while Lua copies it into its
      // own arena; lua_pushlstring avoids Lua's internal strlen() over the
      // same bytes we already have a length for.
      const auto utf8 = rawValue.toString().toUtf8();
      lua_pushlstring(L, utf8.constData(), static_cast<size_t>(utf8.size()));
    }

    const int pcallStatus = lua_pcall(L, 1, 1, 0);
    engine.luaDeadline    = QDeadlineTimer(QDeadlineTimer::Forever);

    if (pcallStatus != LUA_OK) [[unlikely]] {
      qWarning() << "[FrameBuilder] Lua transform call failed for dataset" << uniqueId << ":"
                 << lua_tostring(L, -1);
      lua_pop(L, 1);
      return rawValue;
    }

    // Return the appropriate type
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
  }

  // JavaScript transform path
  if (engine.jsEngine) {
    auto refIt = engine.jsRefs.find(uniqueId);
    if (refIt == engine.jsRefs.end())
      return rawValue;

    // Arm the watchdog
    engine.jsEngine->setInterrupted(false);
    m_jsTransformWatchdog.start();

    // Push argument as appropriate type
    QJSValueList args;
    if (rawValue.typeId() == QMetaType::Double)
      args << QJSValue(rawValue.toDouble());
    else
      args << QJSValue(rawValue.toString());

    auto result = refIt->second.call(args);
    m_jsTransformWatchdog.stop();

    if (engine.jsEngine->isInterrupted()) [[unlikely]] {
      engine.jsEngine->setInterrupted(false);
      qWarning() << "[FrameBuilder] JS transform for dataset" << uniqueId << "timed out after"
                 << kTransformWatchdogMs << "ms";
      return rawValue;
    }

    // Return the appropriate type
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
  // Let Project Editor test dialogs see in-flight edits to shared tables
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

  const auto* val = store->get(QString::fromUtf8(table), QString::fromUtf8(reg));
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
 * @brief Lua C closure for tableSet(table, reg, value).
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

  store->set(QString::fromUtf8(table), QString::fromUtf8(reg), rv);
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

/**
 * @brief Injects tableGet / tableSet / datasetGetRaw / datasetGetFinal into the Lua state as C
 * closures.
 */
void DataModel::FrameBuilder::injectTableApiLua(lua_State* L)
{
  Q_ASSERT(L);

  // Lua and JavaScript share identical camelCase global names so transforms
  // can be rewritten from one language into the other without renaming calls.
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
  js->evaluate(
    QStringLiteral("function tableGet(t, r)       { return __ss.tableGet(t, r); }\n"
                   "function tableSet(t, r, v)    { __ss.tableSet(t, r, v); }\n"
                   "function datasetGetRaw(uid)   { return __ss.datasetGetRaw(uid); }\n"
                   "function datasetGetFinal(uid) { return __ss.datasetGetFinal(uid); }\n"));
}
