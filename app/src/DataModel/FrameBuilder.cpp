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

#include <algorithm>
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

  clear_frame(m_frame);
  m_sourceFrames.clear();

  m_frame.title   = pm.title();
  m_frame.groups  = pm.groups();
  m_frame.actions = pm.actions();

  finalize_frame(m_frame);

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
  m_quickPlotChannels = -1;

  if (!IO::ConnectionManager::instance().isConnected()) {
    m_sourceFrames.clear();
    return;
  }

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return;

  DataModel::FrameParser::instance().readCode();

  const auto& actions = m_frame.actions;
  for (const auto& action : actions)
    if (action.autoExecuteOnConnect)
      IO::ConnectionManager::instance().writeData(get_tx_bytes(action));

  // Pre-build per-source frames so the dashboard configures all widgets
  // immediately instead of waiting for each source to send its first data
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

  auto applyChannelData = [this](const QStringList& chs) {
    const auto* channelData = chs.data();
    const int channelCount  = chs.size();
    for (auto& group : m_frame.groups) {
      for (auto& dataset : group.datasets) {
        const int idx = dataset.index;
        if (idx <= 0 || idx > channelCount) [[unlikely]]
          continue;
        dataset.value        = channelData[idx - 1];
        dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
      }
    }
  };

  for (const auto& channels : std::as_const(multiChannels)) {
    if (channels.isEmpty()) [[unlikely]]
      continue;
    applyChannelData(channels);
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

    // Lazily initialize source frame on first encounter
    if (!m_sourceFrames.contains(sourceId)) {
      DataModel::Frame newFrame;
      newFrame.sourceId                   = sourceId;
      newFrame.title                      = m_frame.title;
      newFrame.actions                    = m_frame.actions;
      newFrame.containsCommercialFeatures = m_frame.containsCommercialFeatures;
      for (const auto& g : m_frame.groups)
        if (g.sourceId == sourceId)
          newFrame.groups.push_back(g);

      m_sourceFrames.insert(sourceId, std::move(newFrame));
    }

    DataModel::Frame& srcFrame = m_sourceFrames[sourceId];
    for (auto& group : srcFrame.groups) {
      for (auto& dataset : group.datasets) {
        const int idx = dataset.index;
        if (idx <= 0 || idx > channelCount) [[unlikely]]
          continue;

        dataset.value        = channelData[idx - 1];
        dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
      }
    }
  };

  for (const auto& channels : std::as_const(multiChannels)) {
    if (channels.isEmpty()) [[unlikely]]
      continue;

    applyChannelData(channels);
    hotpathTxFrame(m_sourceFrames[sourceId]);
  }
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming CSV values.
 *
 * @param data UTF-8 encoded, comma-separated channel values for plotting.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const QByteArray& data)
{
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
#ifdef BUILD_COMMERCIAL
  const auto busType = IO::ConnectionManager::instance().busType();
  if (busType == SerialStudio::BusType::Audio) {
    const auto* audioPtr = IO::ConnectionManager::instance().audio();
    if (!audioPtr)
      return;

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

    const int targetSamples = static_cast<int>(sampleRate * 0.05);
    int fftSamples          = 256;
    while (fftSamples < targetSamples && fftSamples < 8192)
      fftSamples *= 2;

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

      if (m_quickPlotHasHeader && index > 0 && index - 1 < m_quickPlotChannelNames.size())
        dataset.title = m_quickPlotChannelNames[index - 1];
      else
        dataset.title = tr("Channel %1").arg(index);

      dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
      datasets.push_back(dataset);

      ++index;
    }

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

    if (m_quickPlotHasHeader && idx > 0 && idx - 1 < m_quickPlotChannelNames.size())
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
