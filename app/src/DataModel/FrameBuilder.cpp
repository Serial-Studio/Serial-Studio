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
#include <QDateTime>

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "DataModel/FrameParser.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "Misc/JsonValidator.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/LemonSqueezy.h"
#endif

#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

/**
 * @brief Parse comma-separated values from data.
 *
 * IMPORTANT: This function ONLY supports comma (,) as the delimiter.
 *
 * For other delimiters (semicolon, tab, pipe), use a custom JavaScript
 * parser in your project file. Example for semicolon delimiter:
 *
 * @code{.js}
 * function parse(frame) {
 *   return frame.split(';');
 * }
 * @endcode
 *
 * This limitation is intentional to keep the hotpath simple and fast.
 * Custom parsers provide flexibility for non-standard formats without
 * complicating the default CSV parsing logic.
 *
 * @param data UTF-8 encoded CSV data with comma separators
 * @param out Output string list for parsed values (cleared before use)
 * @param reserveHint Expected number of values (optimization hint)
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

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs FrameBuilder and wires export-consumer state tracking.
 */
DataModel::FrameBuilder::FrameBuilder()
  : m_quickPlotChannels(-1), m_quickPlotHasHeader(false), m_timestampedFramesEnabled(false)
{
  // Configure the single-shot JS watchdog used by applyTransform() to
  // interrupt runaway user scripts. The timer fires on the main thread
  // while the QJSEngine call is still executing — setInterrupted() flips
  // an atomic flag that the JS VM checks on the next opcode, causing the
  // call to unwind with an error result.
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

  connect(&CSV::Export::instance(),
          &CSV::Export::enabledChanged,
          this,
          &DataModel::FrameBuilder::updateTimestampedFramesEnabled);
  connect(&MDF4::Export::instance(),
          &MDF4::Export::enabledChanged,
          this,
          &DataModel::FrameBuilder::updateTimestampedFramesEnabled);
  connect(&API::Server::instance(),
          &API::Server::enabledChanged,
          this,
          &DataModel::FrameBuilder::updateTimestampedFramesEnabled);

#ifdef ENABLE_GRPC
  connect(&API::GRPC::GRPCServer::instance(),
          &API::GRPC::GRPCServer::enabledChanged,
          this,
          &DataModel::FrameBuilder::updateTimestampedFramesEnabled);
#endif

  updateTimestampedFramesEnabled();
}

/**
 * @brief Returns the only instance of the class.
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
 * @brief Returns the currently loaded DataModel frame.
 * @return A constant reference to the current DataModel::Frame.
 */
const DataModel::Frame& DataModel::FrameBuilder::frame() const noexcept
{
  return m_frame;
}

//--------------------------------------------------------------------------------------------------
// External connection setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires connection-state signals. Project sync is driven by AppState.
 */
void DataModel::FrameBuilder::setupExternalConnections()
{
  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &DataModel::FrameBuilder::onConnectedChanged);
}

//--------------------------------------------------------------------------------------------------
// Project model sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates m_frame directly from ProjectModel's already-parsed data.
 *
 * Called by AppState::onProjectLoaded() after ProjectModel::openJsonFile()
 * completes. No file I/O and no JSON re-parsing — copies the in-memory
 * group and action vectors directly.
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

  Q_ASSERT(!m_frame.title.isEmpty());

  Q_EMIT jsonFileMapChanged();
}

//--------------------------------------------------------------------------------------------------
// Quick Plot header registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers explicit channel headers for Quick Plot mode.
 * @param headers List of channel names to use; pass empty list to clear.
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
 * @brief Dispatches raw data to the appropriate frame parser based on the
 * current operation mode (from AppState).
 *
 * @param data Raw binary input data to be processed.
 */
void DataModel::FrameBuilder::hotpathRxFrame(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() >= SerialStudio::ProjectFile
           && AppState::instance().operationMode() <= SerialStudio::QuickPlot);

  switch (AppState::instance().operationMode()) {
    case SerialStudio::QuickPlot:
      parseQuickPlotFrame(data);
      break;
    case SerialStudio::ProjectFile:
      parseProjectFrame(data);
      break;
    case SerialStudio::DeviceSendsJSON: {
      auto result = Misc::JsonValidator::parseAndValidate(data);
      if (result.valid && read(m_rawFrame, result.document.object())) [[likely]]
        hotpathTxFrame(m_rawFrame);
      break;
    }
    default:
      break;
  }
}

/**
 * @brief Dispatches raw data from the given source to the project frame parser.
 *
 * @param sourceId Identifier of the source that produced @p data.
 * @param data     Raw binary input data.
 */
void DataModel::FrameBuilder::hotpathRxSourceFrame(int sourceId, const QByteArray& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!data.isEmpty());

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
 * @brief Handles device connection events and triggers auto-execute actions.
 */
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
    return;
  }

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  // Reload parser scripts and recompile transforms for the new session
  Q_ASSERT(!m_frame.title.isEmpty());
  DataModel::FrameParser::instance().readCode();
  compileTransforms();

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
        hotpathTxFrame(srcFrame);
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
    hotpathTxFrame(m_frame);
}

//--------------------------------------------------------------------------------------------------
// Frame parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a project frame using the configured decoding method.
 *
 * @param data Raw binary input to be decoded and assigned to frame datasets.
 */
void DataModel::FrameBuilder::parseProjectFrame(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(!m_frame.groups.empty());

  // Decode via JS parser or CSV fallback
  QList<QStringList> multiChannels;

  if (!SerialStudio::isAnyPlayerOpen()) [[likely]] {
    auto& parser             = DataModel::FrameParser::instance();
    const auto decoderMethod = DataModel::ProjectModel::instance().decoderMethod();

    switch (decoderMethod) {
      case SerialStudio::Hexadecimal:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data.toHex()), 0);
        break;
      case SerialStudio::Base64:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data.toBase64()), 0);
        break;
      case SerialStudio::Binary:
        multiChannels = parser.parseMultiFrame(data, 0);
        break;
      case SerialStudio::PlainText:
      default:
        multiChannels = parser.parseMultiFrame(QString::fromUtf8(data), 0);
        break;
    }
  } else {
    auto& channels = m_channelScratch;
    parseCsvValues(data, channels, 64);
    multiChannels.append(channels);
  }

  auto applyChannelData = [this](const QStringList& chs, int srcId) {
    const auto* channelData = chs.data();
    const int channelCount  = chs.size();
    for (auto& group : m_frame.groups) {
      for (auto& dataset : group.datasets) {
        const int idx = dataset.index;
        if (idx <= 0 || idx > channelCount) [[unlikely]]
          continue;

        dataset.value        = channelData[idx - 1];
        dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);

        // Skip transforms during playback — exported data is already transformed
        if (!dataset.transformCode.isEmpty() && dataset.isNumeric
            && !SerialStudio::isAnyPlayerOpen()) {
          dataset.numericValue = applyTransform(srcId, dataset.uniqueId, dataset.numericValue);
          dataset.value        = QString::number(dataset.numericValue, 'g', 15);
        }
      }
    }
  };

  for (const auto& channels : std::as_const(multiChannels)) {
    if (channels.isEmpty()) [[unlikely]]
      continue;
    applyChannelData(channels, 0);
    hotpathTxFrame(m_frame);
  }
}

/**
 * @brief Source-aware variant of parseProjectFrame.
 *
 * @param sourceId Which source produced @p data.
 * @param data     Raw bytes received from the source's FrameReader.
 */
void DataModel::FrameBuilder::parseProjectFrame(int sourceId, const QByteArray& data)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!data.isEmpty());

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
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data.toHex()), sourceId);
        break;
      case SerialStudio::Base64:
        multiChannels = parser.parseMultiFrame(QString::fromLatin1(data.toBase64()), sourceId);
        break;
      case SerialStudio::Binary:
        multiChannels = parser.parseMultiFrame(data, sourceId);
        break;
      case SerialStudio::PlainText:
      default:
        multiChannels = parser.parseMultiFrame(QString::fromUtf8(data), sourceId);
        break;
    }
  } else {
    auto& channels = m_channelScratch;
    parseCsvValues(data, channels, 64);
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

    DataModel::Frame& srcFrame = it.value();
    for (auto& group : srcFrame.groups) {
      for (auto& dataset : group.datasets) {
        const int idx = dataset.index;
        if (idx <= 0 || idx > channelCount) [[unlikely]]
          continue;

        dataset.value        = channelData[idx - 1];
        dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);

        // Skip transforms during playback — exported data is already transformed
        if (!dataset.transformCode.isEmpty() && dataset.isNumeric
            && !SerialStudio::isAnyPlayerOpen()) {
          dataset.numericValue = applyTransform(sourceId, dataset.uniqueId, dataset.numericValue);
          dataset.value        = QString::number(dataset.numericValue, 'g', 15);
        }
      }
    }
  };

  for (const auto& channels : std::as_const(multiChannels)) {
    if (channels.isEmpty()) [[unlikely]]
      continue;

    applyChannelData(channels);

    auto txIt = m_sourceFrames.find(sourceId);
    if (txIt != m_sourceFrames.end()) [[likely]]
      hotpathTxFrame(txIt.value());
  }
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming CSV values.
 *
 * @param data UTF-8 encoded, comma-separated channel values for plotting.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(AppState::instance().operationMode() == SerialStudio::QuickPlot);

  auto& channels        = m_channelScratch;
  const int reserveHint = (m_quickPlotChannels > 0) ? m_quickPlotChannels : 64;
  parseCsvValues(data, channels, reserveHint);

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
        dataset.value        = channelData[idx - 1];
        dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
      }
    }
  }

  hotpathTxFrame(m_quickPlotFrame);
}

//--------------------------------------------------------------------------------------------------
// Quick-plot project generation functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the internal frame structure for Quick Plot mode.
 *
 * Only called when the number of input channels changes.
 *
 * @param channels List of channel values received in the most recent data frame.
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
 *
 * Reads the audio driver's sample format and rate to set min/max/FFT
 * parameters, then constructs datasets and a single multiplot group.
 *
 * @param channels List of channel values from the most recent audio frame.
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
 * @brief Updates the cached flag indicating if any timestamped frame consumer is enabled.
 */
void DataModel::FrameBuilder::updateTimestampedFramesEnabled()
{
  m_timestampedFramesEnabled = CSV::Export::instance().exportEnabled()
                            || MDF4::Export::instance().exportEnabled()
                            || API::Server::instance().enabled();

#ifdef ENABLE_GRPC
  m_timestampedFramesEnabled =
    m_timestampedFramesEnabled || API::GRPC::GRPCServer::instance().enabled();
#endif
}

/**
 * @brief Publishes a fully constructed DataModel frame to all registered output modules.
 *
 * @param frame The fully populated frame to distribute.
 */
void DataModel::FrameBuilder::hotpathTxFrame(const DataModel::Frame& frame)
{
  Q_ASSERT(!frame.groups.empty());
  Q_ASSERT(!frame.title.isEmpty());

  static auto& csvExport     = CSV::Export::instance();
  static auto& mdf4Export    = MDF4::Export::instance();
  static auto& dashboard     = UI::Dashboard::instance();
  static auto& pluginsServer = API::Server::instance();

  dashboard.hotpathRxFrame(frame);

  if (m_timestampedFramesEnabled) [[unlikely]] {
    auto timestampedFrame = std::make_shared<DataModel::TimestampedFrame>(frame);
    csvExport.hotpathTxFrame(timestampedFrame);
    mdf4Export.hotpathTxFrame(timestampedFrame);
    pluginsServer.hotpathTxFrame(timestampedFrame);

#ifdef ENABLE_GRPC
    static auto& grpcServer = API::GRPC::GRPCServer::instance();
    grpcServer.hotpathTxFrame(timestampedFrame);
#endif
  }
}

//--------------------------------------------------------------------------------------------------
// Per-dataset value transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a safe subset of Lua standard libraries for transform engines.
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
}

/**
 * @brief Lua instruction-count hook that aborts runaway transforms.
 *
 * Installed on every shared transform state via lua_sethook with
 * LUA_MASKCOUNT. Fires every kTransformHookInstrCount instructions: it
 * fetches the owning TransformEngine* from the Lua registry and checks
 * the per-engine deadline. When the deadline has expired, luaL_error()
 * unwinds the current lua_pcall() with an error, which applyTransform()
 * catches and translates to "return rawValue unchanged".
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
 * @brief Compiles per-dataset transform expressions into shared engines.
 *
 * Scans all datasets in m_frame.groups. For each source that has at least
 * one dataset with a non-empty transformCode, creates a shared Lua or JS
 * engine and compiles each expression into a named function. The function
 * reference is cached for O(1) lookup during the hotpath.
 */
void DataModel::FrameBuilder::compileTransforms()
{
  destroyTransformEngines();
  Q_ASSERT(m_transformEngines.empty());

  // Collect datasets with transforms, grouped by sourceId
  std::map<int, std::vector<TransformEntry>> bySource;
  for (const auto& group : m_frame.groups)
    for (const auto& ds : group.datasets)
      if (!ds.transformCode.isEmpty())
        bySource[ds.sourceId].push_back({ds.uniqueId, ds.transformCode});

  if (bySource.empty())
    return;

  // Determine language for each source and compile
  const auto& sources = DataModel::ProjectModel::instance().sources();

  for (auto& [sourceId, entries] : bySource) {
    // Resolve the scripting language for this source
    int lang = SerialStudio::Lua;
    for (const auto& src : sources)
      if (src.sourceId == sourceId) {
        lang = src.frameParserLanguage;
        break;
      }

    // Insert an empty engine into the map first so the TransformEngine*
    // we store in the Lua registry (below) remains valid — building the
    // engine locally and then std::move'ing it would invalidate the
    // address captured by the watchdog hook
    auto [it, inserted] = m_transformEngines.emplace(sourceId, TransformEngine{});
    Q_ASSERT(inserted);
    TransformEngine& engine = it->second;

    // Delegate to language-specific compiler
    if (lang == SerialStudio::Lua)
      compileTransformsLua(engine, entries);
    else
      compileTransformsJS(engine, entries);

    // Remove the engine entry if compilation produced nothing useful
    if (!engine.luaState && !engine.jsEngine)
      m_transformEngines.erase(it);
  }
}

/**
 * @brief Compiles per-dataset Lua transforms into a shared lua_State.
 *
 * Each dataset's chunk is executed to define a transform() global, which is
 * then renamed to a per-dataset alias and stored via luaL_ref for O(1)
 * hotpath lookup.
 */
void DataModel::FrameBuilder::compileTransformsLua(
  TransformEngine& engine, const std::vector<TransformEntry>& entries)
{
  // Create a shared Lua state for all transforms in this source
  lua_State* L = luaL_newstate();
  Q_ASSERT(L != nullptr);
  if (!L) [[unlikely]]
    return;

  openSafeLibsForTransform(L);

  // Store the engine pointer in the Lua registry so the watchdog
  // hook can look it up without needing a global singleton pointer
  lua_pushlightuserdata(L, &engine);
  lua_setfield(L, LUA_REGISTRYINDEX, "__ss_transform__");

  // Install instruction-count watchdog
  lua_sethook(L, &FrameBuilder::transformLuaWatchdogHook, LUA_MASKCOUNT,
              kTransformHookInstrCount);

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
    engine.luaRefs[entry.uniqueId] = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  // Disarm the deadline — subsequent arming happens per-call
  engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);
  engine.luaState    = L;
}

/**
 * @brief Compiles per-dataset JavaScript transforms into a shared QJSEngine.
 *
 * Each dataset's code is wrapped in an IIFE so that top-level var
 * declarations become per-dataset closure state.
 */
void DataModel::FrameBuilder::compileTransformsJS(
  TransformEngine& engine, const std::vector<TransformEntry>& entries)
{
  // Create a shared QJSEngine for all transforms in this source
  auto* js = new QJSEngine();

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
                 << "dataset" << entry.uniqueId << ":"
                 << evalResult.property("message").toString();
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
 * @brief Applies the pre-compiled transform for a dataset.
 *
 * Looks up the function reference by uniqueId and calls it with the raw
 * numeric value. Returns the transformed value, or @p rawValue if no
 * transform is configured or if the call fails.
 *
 * @param sourceId  Source that owns the dataset.
 * @param uniqueId  Dataset unique identifier.
 * @param rawValue  Raw numeric value from the frame parser.
 * @return Transformed value, or rawValue on error / missing transform.
 */
double DataModel::FrameBuilder::applyTransform(int sourceId, int uniqueId, double rawValue)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(uniqueId >= 0);

  auto engineIt = m_transformEngines.find(sourceId);
  if (engineIt == m_transformEngines.end())
    return rawValue;

  auto& engine = engineIt->second;

  // Lua transform path
  if (engine.luaState) {
    auto refIt = engine.luaRefs.find(uniqueId);
    if (refIt == engine.luaRefs.end())
      return rawValue;

    // Arm the per-call deadline — the lua_sethook watchdog installed in
    // compileTransforms() will abort the call if it runs past this point.
    // The deadline is disarmed on exit so out-of-band hook fires don't
    // interrupt anything unrelated.
    lua_State* L = engine.luaState;
    engine.luaDeadline.setRemainingTime(kTransformWatchdogMs);

    lua_rawgeti(L, LUA_REGISTRYINDEX, refIt->second);
    lua_pushnumber(L, rawValue);
    const int pcallStatus = lua_pcall(L, 1, 1, 0);

    engine.luaDeadline = QDeadlineTimer(QDeadlineTimer::Forever);

    if (pcallStatus != LUA_OK) [[unlikely]] {
      qWarning() << "[FrameBuilder] Lua transform call failed for dataset"
                 << uniqueId << ":" << lua_tostring(L, -1);
      lua_pop(L, 1);
      return rawValue;
    }

    // Validate return type — lua_tonumber silently returns 0.0 for nil
    if (!lua_isnumber(L, -1)) [[unlikely]] {
      lua_pop(L, 1);
      return rawValue;
    }

    const double result = lua_tonumber(L, -1);
    lua_pop(L, 1);

    // Guard against NaN/Inf propagating to the dashboard
    if (!std::isfinite(result)) [[unlikely]]
      return rawValue;

    return result;
  }

  // JavaScript transform path
  if (engine.jsEngine) {
    auto refIt = engine.jsRefs.find(uniqueId);
    if (refIt == engine.jsRefs.end())
      return rawValue;

    // Arm the QJSEngine watchdog: the QTimer fires on the main thread
    // while call() is still executing, and setInterrupted() flips an
    // atomic flag the JS VM checks on its next opcode — so the call
    // unwinds with an error result and we fall back to rawValue.
    engine.jsEngine->setInterrupted(false);
    m_jsTransformWatchdog.start();

    QJSValueList args;
    args << QJSValue(rawValue);
    auto result = refIt->second.call(args);

    m_jsTransformWatchdog.stop();

    if (engine.jsEngine->isInterrupted()) [[unlikely]] {
      engine.jsEngine->setInterrupted(false);
      qWarning() << "[FrameBuilder] JS transform for dataset" << uniqueId
                 << "timed out after" << kTransformWatchdogMs << "ms";
      return rawValue;
    }

    if (!result.isNumber())
      return rawValue;

    const double val = result.toNumber();
    if (!std::isfinite(val)) [[unlikely]]
      return rawValue;

    return val;
  }

  return rawValue;
}
