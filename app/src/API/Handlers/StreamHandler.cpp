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

#include "API/Handlers/StreamHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the stream discovery commands.
 */
void API::Handlers::StreamHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  const auto empty = emptySchema();
  registry.registerCommand(
    QStringLiteral("stream.getInfo"),
    QStringLiteral(
      "Report the typed sample-stream surface: how many stream sources are live, the wire "
      "encoding of a streamBlock line (base64 float32le), the per-subscriber queue depth "
      "before the oldest block is dropped and counted, and the names of the connection-scoped "
      "commands (stream.subscribe / stream.unsubscribe) a subscriber sends on its own socket."),
    empty,
    &getInfo);

  registry.registerCommand(
    QStringLiteral("stream.getSources"),
    QStringLiteral("List every live stream source: sourceId, channel count, sample rate, and "
                   "the dataset uniqueIds its blocks carry."),
    empty,
    &getSources);
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the stream surface description (see the registration text).
 */
API::CommandResponse API::Handlers::StreamHandler::getInfo(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();

  QJsonObject result;
  result.insert(QStringLiteral("sourceCount"), static_cast<int>(manager.streamWorkers().size()));
  result.insert(QStringLiteral("encoding"), QStringLiteral("base64:float32le"));
  result.insert(QStringLiteral("queueDepth"), 8);
  result.insert(QStringLiteral("subscribeCommand"), QStringLiteral("stream.subscribe"));
  result.insert(QStringLiteral("unsubscribeCommand"), QStringLiteral("stream.unsubscribe"));
  result.insert(QStringLiteral("pushLine"), QStringLiteral("streamBlock"));
  result.insert(
    QStringLiteral("_summary"),
    QStringLiteral("Send {\"type\":\"command\",\"command\":\"stream.subscribe\"} on this socket "
                   "(optionally with params.sources = [sourceId,...]) to receive streamBlock "
                   "NDJSON lines; each line carries missed, the blocks dropped since the last "
                   "delivery."));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Lists the live stream sources and their dataset bindings.
 */
API::CommandResponse API::Handlers::StreamHandler::getSources(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();

  QJsonArray sources;
  for (const auto& worker : manager.streamWorkers()) {
    if (!worker)
      continue;

    const auto& config = worker->config();

    QJsonArray datasets;
    for (const auto& dataset : config.datasets) {
      QJsonObject entry;
      entry.insert(Keys::UniqueId, dataset.uniqueId);
      entry.insert(QStringLiteral("channel"), dataset.channel);
      entry.insert(QStringLiteral("fft"), dataset.fft);
      entry.insert(QStringLiteral("plot"), dataset.plot);
      datasets.append(entry);
    }

    QJsonObject source;
    source.insert(Keys::SourceId, config.sourceId);
    source.insert(QStringLiteral("channels"), config.channels);
    source.insert(QStringLiteral("sampleRate"), config.sampleRate);
    source.insert(QStringLiteral("datasets"), datasets);

    if (auto* processor = worker->processor()) {
      source.insert(QStringLiteral("samplesProcessed"),
                    static_cast<double>(processor->samplesProcessed()));
      source.insert(QStringLiteral("blocksProcessed"),
                    static_cast<double>(processor->blocksProcessed()));
      source.insert(QStringLiteral("transformErrors"),
                    static_cast<double>(processor->transformErrorCount()));
      source.insert(QStringLiteral("displayDrops"),
                    static_cast<double>(processor->displayDropCount()));
    }

    sources.append(source);
  }

  QJsonObject result;
  result.insert(QStringLiteral("count"), sources.size());
  result.insert(QStringLiteral("sources"), sources);
  return CommandResponse::makeSuccess(id, result);
}
