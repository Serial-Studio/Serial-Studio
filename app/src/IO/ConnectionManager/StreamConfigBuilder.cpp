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

#include "IO/ConnectionManager/StreamConfigBuilder.h"

#include <algorithm>

#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "IO/HAL_Driver.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the builder to the two modules every derivation reads. Both are constructed
 *        ahead of ConnectionManager in the pinned order, so the facade captures them in its own
 *        constructor and this object is usable before any wiring pass runs.
 */
IO::StreamConfigBuilder::StreamConfigBuilder(AppState& appState,
                                             DataModel::ProjectModel& projectModel)
  : m_appState(appState), m_projectModel(projectModel)
{}

//--------------------------------------------------------------------------------------------------
// Frame configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a FrameConfig for the given @p deviceId from current settings.
 */
IO::FrameConfig IO::StreamConfigBuilder::frameConfig(int deviceId) const
{
  SS_ASSERT_LOG(deviceId >= 0);

  const auto opMode = m_appState.operationMode();
  if (opMode == SerialStudio::QuickPlot || opMode == SerialStudio::ConsoleOnly)
    return m_appState.frameConfig();

  FrameConfig cfg;
  cfg.operationMode = opMode;

  const auto& sources = m_projectModel.sources();
  for (const auto& src : sources) {
    if (src.sourceId != deviceId)
      continue;

    QByteArray start, end;
    QString checksum;
    DataModel::read_io_settings(start, end, checksum, DataModel::serialize(src));

    cfg.startSequences    = start.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{start};
    cfg.finishSequences   = end.isEmpty() ? QList<QByteArray>{} : QList<QByteArray>{end};
    cfg.checksumAlgorithm = checksum;
    cfg.frameDetection    = static_cast<SerialStudio::FrameDetection>(src.frameDetection);

    if ((cfg.frameDetection == SerialStudio::StartDelimiterOnly
         || cfg.frameDetection == SerialStudio::StartAndEndDelimiter)
        && cfg.startSequences.isEmpty()) [[unlikely]]
      cfg.frameDetection =
        cfg.finishSequences.isEmpty() ? SerialStudio::NoDelimiters : SerialStudio::EndDelimiterOnly;

    if (cfg.frameDetection == SerialStudio::EndDelimiterOnly && cfg.finishSequences.isEmpty())
      [[unlikely]]
      cfg.frameDetection = SerialStudio::NoDelimiters;

    return cfg;
  }

  SS_ASSERT_LOG(cfg.operationMode == opMode);

  cfg.startSequences    = {QByteArray("/*")};
  cfg.finishSequences   = {QByteArray("*/")};
  cfg.checksumAlgorithm = QString();
  cfg.frameDetection    = m_projectModel.frameDetection();
  return cfg;
}

//--------------------------------------------------------------------------------------------------
// Stream lane & stream configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the per-source stream-lane override for @p deviceId ("", "on" or "off").
 */
QString IO::StreamConfigBuilder::streamLane(int deviceId) const
{
  SS_ASSERT_LOG(deviceId >= 0);

  if (m_appState.operationMode() != SerialStudio::ProjectFile)
    return QString();

  for (const auto& src : m_projectModel.sources())
    if (src.sourceId == deviceId)
      return src.streamLane;

  return QString();
}

/**
 * @brief Builds one stream source's worker configuration: bindings come from the project's
 *        datasets (ProjectFile) or are synthesized per audio channel (QuickPlot), carrying
 *        the PERSISTED uniqueId the dashboard registers its stream targets under (positional
 *        ids are only the loader's legacy backfill); rate/channels come from the driver.
 */
IO::StreamConfig IO::StreamConfigBuilder::streamConfig(int deviceId, HAL_Driver* driver) const
{
  SS_ASSERT(driver != nullptr, return {});
  SS_ASSERT_LOG(deviceId >= 0);

  StreamConfig config;
  config.sourceId = deviceId;

#ifdef BUILD_COMMERCIAL
  if (auto* audioDriver = qobject_cast<IO::Drivers::Audio*>(driver)) {
    config.channels   = std::max(1u, audioDriver->config().capture.channels);
    config.sampleRate = static_cast<double>(audioDriver->config().sampleRate);
  }
#endif

  config.luaFastMode = m_projectModel.luaFastMode();

  if (m_appState.operationMode() == SerialStudio::ProjectFile)
    appendProjectChannels(deviceId, config);
  else
    appendQuickPlotChannels(config);

  return config;
}

/**
 * @brief Appends one stream channel per channel-bound dataset of every group owned by
 *        @p deviceId, preserving the persisted uniqueId the dashboard binds its targets to.
 */
void IO::StreamConfigBuilder::appendProjectChannels(int deviceId, StreamConfig& config) const
{
  SS_ASSERT_LOG(deviceId >= 0);
  SS_ASSERT_LOG(config.sourceId == deviceId);

  for (const auto& group : m_projectModel.groups()) {
    if (group.sourceId != deviceId)
      continue;

    for (const auto& dataset : group.datasets) {
      if (!dataset.enabled || dataset.index <= 0)
        continue;

      StreamChannelConfig channel;
      channel.uniqueId =
        dataset.uniqueId >= 0 ? dataset.uniqueId : DataModel::dataset_unique_id(group, dataset);
      channel.channel = dataset.index - 1;
      channel.plot    = dataset.plt;
#ifdef BUILD_COMMERCIAL
      channel.fft = dataset.fft || dataset.waterfall;
#else
      channel.fft = dataset.fft;
#endif
      channel.fftSamples        = dataset.fftSamples;
      channel.transformLanguage = dataset.transformLanguage;
      channel.transformCode     = dataset.transformCode;
      channel.title             = dataset.title;
      channel.alias             = dataset.alias;
      config.datasets.push_back(std::move(channel));
    }
  }
}

/**
 * @brief Synthesizes one plotted+FFT channel per driver channel for QuickPlot, sizing the FFT
 *        window to roughly 50 ms of the source's own sample rate.
 */
void IO::StreamConfigBuilder::appendQuickPlotChannels(StreamConfig& config)
{
  SS_ASSERT_LOG(config.channels >= 0);
  SS_ASSERT_LOG(config.datasets.empty());

  const int targetSamples = static_cast<int>(config.sampleRate * 0.05);
  int fftSamples          = 256;
  while (fftSamples < targetSamples && fftSamples < 8192)
    fftSamples *= 2;

  for (int i = 0; i < config.channels; ++i) {
    StreamChannelConfig channel;
    channel.uniqueId   = DataModel::dataset_unique_id(0, 0, i);
    channel.channel    = i;
    channel.plot       = true;
    channel.fft        = true;
    channel.fftSamples = fftSamples;
    config.datasets.push_back(std::move(channel));
  }
}
