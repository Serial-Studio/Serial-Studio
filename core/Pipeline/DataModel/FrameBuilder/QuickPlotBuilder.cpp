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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/FrameBuilder/QuickPlotBuilder.h"

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/HotpathOptimization.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/ReplayRowCodec.h"
#include "IO/PipelineHost.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief The number a channel carries on a frame that brought it no sample (spec 0106 R11); every
 *        ring skips non-finite values, so the plot shows a gap and the data grid keeps its text.
 */
static constexpr double kNoSample = std::numeric_limits<double>::quiet_NaN();

//--------------------------------------------------------------------------------------------------
// Retained facts
//--------------------------------------------------------------------------------------------------

/**
 * @brief The bus type of the primary source as the connection manager last retained it, or UART
 *        before any link state was published.
 */
[[nodiscard]] static int activeBusType()
{
  const auto* bus = &Core::services().bus;
  const auto link = bus ? bus->latest<Core::Bus::ConnectionStateChanged>() : nullptr;
  return link ? link->busType : static_cast<int>(SerialStudio::BusType::UART);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the builder to the frame builder's cached operation mode, so the mode guards below
 *        read the same value the parse lanes do instead of a second copy that could drift.
 */
DataModel::QuickPlotBuilder::QuickPlotBuilder(const SerialStudio::OperationMode& operationMode)
  : m_operationMode(operationMode), m_hasHeader(false)
{
  m_fieldOfChannel.fill(-1);
}

//--------------------------------------------------------------------------------------------------
// Header registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers Quick Plot channel headers, or clears them when @p headers is empty.
 */
void DataModel::QuickPlotBuilder::setHeaders(const QStringList& headers)
{
  if (!headers.isEmpty()) {
    m_hasHeader    = true;
    m_channelNames = headers;
  } else {
    m_hasHeader = false;
    m_channelNames.clear();
  }
}

//--------------------------------------------------------------------------------------------------
// Line observation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forgets the detected format and the channel registry; the next line detects again.
 */
void DataModel::QuickPlotBuilder::reset()
{
  m_state.reset();
}

/**
 * @brief Pins the positional comma format with @p count channels (replay rows and the audio lane).
 */
void DataModel::QuickPlotBuilder::pinPositional(const int count)
{
  SS_ASSERT(count >= 0, return);
  SS_ASSERT(kMaxLineFields >= count, return);
  m_state.pinPositional(count);
}

/**
 * @brief Tokenizes one live line into the field scratch and classifies it.
 */
DataModel::QuickPlotLineOutcome DataModel::QuickPlotBuilder::observe(QByteArrayView line)
{
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return QuickPlotLineOutcome::Text);
  SS_ASSERT(!line.isEmpty(), return QuickPlotLineOutcome::Text);
  return m_state.observe(line, m_fields);
}

/**
 * @brief Replay twin of observe: the quote-aware comma cells of one synthesized row become
 *        positional fields under the byte-level toDouble grammar, so playback parses as before.
 */
DataModel::QuickPlotLineOutcome DataModel::QuickPlotBuilder::observeReplayRow(QByteArrayView row)
{
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return QuickPlotLineOutcome::Text);
  if (!m_state.pinned())
    m_state.pinPositional(0);

  qsizetype end = row.size();
  for (; end > 0 && (row.at(end - 1) == '\n' || row.at(end - 1) == '\r'); --end)
    continue;

  ReplayCellViews cells;
  splitReplayRowSpans(row.first(end), cells, m_replayScratch, ',');
  const qsizetype count = qMin<qsizetype>(cells.size(), kMaxLineFields);
  SS_ASSERT(count >= 0, return QuickPlotLineOutcome::Text);
  for (qsizetype i = 0; i < count; ++i) {
    auto& field  = m_fields.items[static_cast<size_t>(i)];
    field        = LineField();
    field.token  = cells[i];
    field.value  = cells[i];
    field.number = SerialStudio::toDouble(cells[i], &field.numeric);
  }

  m_fields.count     = count;
  m_fields.arenaUsed = 0;
  return m_state.observeTokenized(m_fields);
}

/**
 * @brief The raw token of every field of the last observed line, for the latest-frame capture.
 */
qsizetype DataModel::QuickPlotBuilder::tokenSpans(const QByteArrayView*& spans) noexcept
{
  SS_ASSERT(kMaxLineFields >= m_fields.count, m_fields.count = 0);
  for (qsizetype i = 0; i < m_fields.count; ++i)
    m_tokenSpans[static_cast<size_t>(i)] = m_fields.items[static_cast<size_t>(i)].token;

  spans = m_tokenSpans.data();
  SS_ASSERT(spans != nullptr, return 0);
  return m_fields.count;
}

//--------------------------------------------------------------------------------------------------
// Frame construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the synthetic source row that anchors a QuickPlot frame. Always returns a
 *        non-null title so downstream exporters bound to NOT NULL columns don't reject the row.
 */
DataModel::Source DataModel::QuickPlotBuilder::makeSource() const
{
  DataModel::Source src;
  src.sourceId = 0;
  src.title    = tr("Device A");
  src.busType  = activeBusType();
  return src;
}

/**
 * @brief Rebuilds the Quick Plot frame structure from the channel registry. The caller invalidates
 *        the frame pool first: a rebuilt structure must never share a generation with the slots
 *        staged from the old one.
 */
void DataModel::QuickPlotBuilder::build()
{
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return);
  const auto& channels = m_state.channels();
  SS_ASSERT(!channels.empty(), return);
  const int count = static_cast<int>(channels.size());

#ifdef BUILD_COMMERCIAL
  if (activeBusType() == static_cast<int>(SerialStudio::BusType::Audio)) {
    QStringList titles;
    titles.reserve(count);
    for (int i = 0; i < count; ++i)
      titles.append(channelTitle(i));

    buildAudio(titles);
    return;
  }
#endif

  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(static_cast<size_t>(count));
  for (int i = 0; i < count; ++i) {
    DataModel::Dataset dataset;
    dataset.groupId   = 0;
    dataset.datasetId = i;
    dataset.uniqueId  = dataset_unique_id(0, 0, i);
    dataset.index     = i + 1;
    dataset.plt       = false;
    dataset.title     = channelTitle(i);
    dataset.units     = QString::fromUtf8(channels[static_cast<size_t>(i)].unit);
    datasets.push_back(dataset);
  }

  clear_frame(m_frame);
  m_frame.title = tr("Quick Plot");
  m_frame.sources.push_back(makeSource());

  DataModel::Group datagrid;
  datagrid.groupId  = 0;
  datagrid.uniqueId = runtime_group_unique_id(0);
  datagrid.datasets = datasets;
  datagrid.title    = tr("Quick Plot Data");
  datagrid.widget   = QStringLiteral("datagrid");
  for (size_t i = 0; i < datagrid.datasets.size(); ++i)
    datagrid.datasets[i].plt = true;

  m_frame.groups.push_back(datagrid);

  if (datasets.size() > 1) {
    DataModel::Group multiplot;
    multiplot.groupId  = 1;
    multiplot.uniqueId = runtime_group_unique_id(1);
    multiplot.datasets = datasets;
    multiplot.title    = tr("Multi-Plot");
    multiplot.widget   = QStringLiteral("multiplot");
    for (size_t i = 0; i < multiplot.datasets.size(); ++i) {
      multiplot.datasets[i].groupId  = 1;
      multiplot.datasets[i].uniqueId = dataset_unique_id(0, 1, static_cast<int>(i));
    }

    m_frame.groups.push_back(multiplot);
  }

  finalize_frame(m_frame);
}

//--------------------------------------------------------------------------------------------------
// Value application
//--------------------------------------------------------------------------------------------------

/**
 * @brief Title precedence: the wire name or accepted header row held by the state, then a header a
 *        player registered, then the numbered default.
 */
QString DataModel::QuickPlotBuilder::channelTitle(const int index) const
{
  SS_ASSERT(index >= 0, return QString());
  SS_ASSERT(m_state.channels().size() > static_cast<size_t>(index), return QString());

  QString wire = m_state.channelTitle(index);
  if (!wire.isEmpty())
    return wire;

  if (m_hasHeader && index < m_channelNames.size())
    return m_channelNames.at(index);

  return tr("Channel %1").arg(index + 1);
}

/**
 * @brief Writes the last observed line into the frame: a present numeric field sets text and number
 *        in place, a present text field sets the text with the numeric flag off, an absent field
 *        only writes NaN (spec 0106 R11): its flag and text stay, so the data grid's cache keeps
 *        the last formatted value and the multi-plot never mistakes the gap for a text channel.
 */
void DataModel::QuickPlotBuilder::applyFields()
{
  SS_ASSERT(kMaxLineFields >= m_fields.count, return);
  SS_ASSERT(!m_frame.groups.empty(), return);

  mapFieldsToChannels();
  const int channelCount =
    std::min(static_cast<int>(m_state.channels().size()), static_cast<int>(kMaxLineFields));
  for (auto& group : m_frame.groups) {
    SS_NO_UNROLL
    for (auto& dataset : group.datasets) {
      const int channel    = dataset.index - 1;
      const bool inRange   = channel >= 0 && channel < channelCount;
      const int fieldIndex = inRange ? m_fieldOfChannel[static_cast<size_t>(channel)] : -1;
      applyField(dataset, fieldIndex);
    }
  }
}

/**
 * @brief Maps every channel to the field that carries it on this line, or -1.
 */
void DataModel::QuickPlotBuilder::mapFieldsToChannels() noexcept
{
  const size_t channelCount = m_state.channels().size();
  SS_ASSERT(m_fieldOfChannel.size() >= channelCount, return);
  SS_ASSERT(kMaxLineFields >= m_fields.count, return);
  std::fill_n(m_fieldOfChannel.begin(), channelCount, -1);

  for (qsizetype i = 0; i < m_fields.count; ++i) {
    const int channel = m_state.indexOf(m_fields.items[static_cast<size_t>(i)], i);
    if (channel >= 0 && static_cast<size_t>(channel) < channelCount)
      m_fieldOfChannel[static_cast<size_t>(channel)] = static_cast<int>(i);
  }
}

/**
 * @brief One dataset from one field; the number text excludes the unit the structure already shows.
 */
void DataModel::QuickPlotBuilder::applyField(DataModel::Dataset& dataset, const int fieldIndex)
{
  SS_ASSERT(static_cast<int>(m_fields.count) > fieldIndex, return);
  if (fieldIndex < 0) {
    dataset.numericValue = kNoSample;
    return;
  }

  const auto& field = m_fields.items[static_cast<size_t>(fieldIndex)];
  if (!field.numeric) {
    assign_utf8_in_place(dataset.value, field.value);
    dataset.numericValue = kNoSample;
    dataset.isNumeric    = false;
    return;
  }

  const qsizetype numberLength = field.value.size() - field.unit.size();
  SS_ASSERT(numberLength > 0, return);
  assign_utf8_in_place(dataset.value, trimLineAscii(field.value.first(numberLength)));
  dataset.numericValue = field.number;
  dataset.isNumeric    = true;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief The miniaudio ma_format ordinals the AudioCaptureFormat topic carries, mirrored here so
 *        the pipeline sizes the audio lane without the driver's header.
 */
enum AudioSampleFormat : int {
  kAudioFormatU8  = 1,
  kAudioFormatS16 = 2,
  kAudioFormatS24 = 3,
  kAudioFormatS32 = 4,
  kAudioFormatF32 = 5,
};

/**
 * @brief Returns the numeric display range of a miniaudio capture format, or the normalized
 * -1..1 range when the driver publishes normalized samples.
 */
static void audioFormatRange(const int fmt, bool normalized, double& minValue, double& maxValue)
{
  if (normalized) {
    minValue = -1.0;
    maxValue = 1.0;
    return;
  }

  switch (fmt) {
    case kAudioFormatU8:
      maxValue = 255;
      minValue = 0;
      break;
    case kAudioFormatS16:
      maxValue = 32767;
      minValue = -32768;
      break;
    case kAudioFormatS24:
      maxValue = 8388607;
      minValue = -8388608;
      break;
    case kAudioFormatS32:
      maxValue = 2147483647;
      minValue = -2147483648;
      break;
    case kAudioFormatF32:
      maxValue = 1.0;
      minValue = -1.0;
      break;
    default:
      maxValue = 1.0;
      minValue = 0.0;
      break;
  }
}
#endif

/**
 * @brief Builds an audio-specific Quick Plot frame with FFT configuration.
 */
void DataModel::QuickPlotBuilder::buildAudio(const QStringList& channels)
{
  SS_ASSERT(!channels.isEmpty(), return);
  SS_ASSERT(m_operationMode == SerialStudio::QuickPlot, return);

#ifdef BUILD_COMMERCIAL
  int format      = 0;
  int sampleRate  = 0;
  bool haveAudio  = false;
  bool normalized = false;
  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    const auto* bus  = &Core::services().bus;
    const auto audio = bus ? bus->latest<Core::Bus::AudioCaptureFormat>() : nullptr;
    if (!audio)
      return;

    format     = audio->format;
    sampleRate = audio->sampleRate;
    normalized = audio->normalized;
    haveAudio  = true;
  });

  if (!haveAudio)
    return;

  double maxValue = 1.0;
  double minValue = 0.0;
  audioFormatRange(format, normalized, minValue, maxValue);

  const int targetSamples = static_cast<int>(sampleRate * 0.05);
  int fftSamples          = 256;
  while (fftSamples < targetSamples && fftSamples < 8192)
    fftSamples *= 2;

  const bool multipleChannels = channels.count() > 1;
  int index                   = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto& channel : std::as_const(channels)) {
    DataModel::Dataset dataset;
    dataset.fft                  = true;
    dataset.plt                  = !multipleChannels;
    dataset.groupId              = 0;
    dataset.datasetId            = index - 1;
    dataset.uniqueId             = dataset_unique_id(0, 0, index - 1);
    dataset.index                = index;
    dataset.value                = channel;
    dataset.pltMax               = maxValue;
    dataset.pltMin               = minValue;
    dataset.fftMax               = maxValue;
    dataset.fftMin               = minValue;
    dataset.fftSamples           = fftSamples;
    dataset.fftSamplingRate      = sampleRate;
    dataset.fftLogX              = true;
    dataset.fftBallistics        = true;
    dataset.fftBallisticsRelease = 100;

    if (m_hasHeader && index > 0 && index - 1 < static_cast<int>(m_channelNames.size()))
      dataset.title = m_channelNames[index - 1];
    else
      dataset.title = tr("Channel %1").arg(index);

    dataset.numericValue = SerialStudio::toDouble(dataset.value, &dataset.isNumeric);
    datasets.push_back(dataset);
    ++index;
  }

  DataModel::Group group;
  group.groupId  = 0;
  group.uniqueId = runtime_group_unique_id(0);
  group.datasets = datasets;
  group.title    = tr("Audio Input");
  if (multipleChannels)
    group.widget = QStringLiteral("multiplot");

  clear_frame(m_frame);
  m_frame.title = tr("Quick Plot");
  m_frame.sources.push_back(makeSource());
  m_frame.groups.push_back(group);
  finalize_frame(m_frame);
#else
  Q_UNUSED(channels);
#endif
}
