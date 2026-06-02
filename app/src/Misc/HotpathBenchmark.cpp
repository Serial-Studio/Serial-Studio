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

#include "Misc/HotpathBenchmark.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/Server.h"
#include "AppState.h"
#include "CSV/Export.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/JsWatchdogThread.h"
#include "IO/ConnectionManager.h"
#include "IO/FrameReader.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"
#ifdef BUILD_COMMERCIAL
#  include "MDF4/Export.h"
#  include "MQTT/Publisher.h"
#  include "Sessions/DatabaseManager.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#endif
#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

namespace Misc {

//--------------------------------------------------------------------------------------------------
// Synthetic project + workload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an in-memory project JSON with one comma-decoded, N-channel source.
 */
QJsonObject HotpathBenchmark::buildProjectJson(int language, int channels)
{
  Q_ASSERT(channels > 0);
  Q_ASSERT(language == SerialStudio::JavaScript || language == SerialStudio::Lua);

  QJsonArray datasets;
  for (int i = 0; i < channels; ++i) {
    QJsonObject ds;
    ds.insert(Keys::Title, QStringLiteral("CH%1").arg(i + 1));
    ds.insert(Keys::Index, i + 1);
    ds.insert(Keys::Graph, true);
    datasets.append(ds);
  }

  QJsonObject group;
  group.insert(Keys::Title, QStringLiteral("Channels"));
  group.insert(Keys::Widget, QStringLiteral("multiplot"));
  group.insert(Keys::Datasets, datasets);

  QJsonObject source;
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::Title, QStringLiteral("Benchmark"));
  source.insert(Keys::FrameStart, QString());
  source.insert(Keys::FrameEnd, QStringLiteral("\n"));
  source.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::DecoderMethod, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::FrameParserLanguage, language);
  source.insert(Keys::FrameParserCode, DataModel::FrameParser::defaultTemplateCode(language));

  QJsonObject root;
  root.insert(Keys::Title, QStringLiteral("Hotpath Benchmark"));
  root.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  root.insert(Keys::Groups, QJsonArray{group});
  root.insert(Keys::Sources, QJsonArray{source});
  root.insert(Keys::Actions, QJsonArray{});
  return root;
}

/**
 * @brief Builds one socket-sized chunk of newline-delimited, N-channel comma frames.
 */
QByteArray HotpathBenchmark::buildChunk(int frames, int channels)
{
  Q_ASSERT(frames > 0);
  Q_ASSERT(channels > 0);

  QByteArray line;
  for (int ch = 0; ch < channels; ++ch) {
    if (ch > 0)
      line.append(',');

    line.append(QByteArrayLiteral("12.34"));
  }

  line.append('\n');

  QByteArray chunk;
  chunk.reserve(frames * line.size());
  for (int i = 0; i < frames; ++i)
    chunk.append(line);

  Q_ASSERT(!chunk.isEmpty());
  return chunk;
}

//--------------------------------------------------------------------------------------------------
// Pipeline + consumer setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the synthetic project and activates the parse pipeline for the given language.
 */
void HotpathBenchmark::setupProject(int language, int channels)
{
  Q_ASSERT(channels > 0);

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  const QJsonObject root = buildProjectJson(language, channels);
  const bool loaded      = project.loadFromJsonDocument(QJsonDocument(root));
  Q_ASSERT(loaded);
  Q_UNUSED(loaded);

  auto& parser = DataModel::FrameParser::instance();
  parser.setSuppressMessageBoxes(true);
  parser.readCode();

  DataModel::FrameBuilder::instance().syncFromProjectModel();
}

/**
 * @brief Enables every local export/output consumer (CSV/MDF4/Sessions/API/gRPC).
 */
void HotpathBenchmark::enableConsumers()
{
  // MQTT is omitted: it needs a live broker and would just spin reconnects.
  CSV::Export::instance().setExportEnabled(true);
  API::Server::instance().setEnabled(true);
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().setExportEnabled(true);
  Sessions::Export::instance().setExportEnabled(true);
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(true);
#endif
}

/**
 * @brief Disables every consumer enabled by enableConsumers() and drains queued teardown events.
 */
void HotpathBenchmark::disableConsumers()
{
  CSV::Export::instance().setExportEnabled(false);
  API::Server::instance().setEnabled(false);
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().setExportEnabled(false);
  Sessions::Export::instance().setExportEnabled(false);
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(false);
#endif

  // Stop consumer workers while QApplication lives (incl. MQTT, which hotpathTxFrame constructs).
  CSV::Export::instance().stopWorker();
  API::Server::instance().stopWorker();
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().stopWorker();
  Sessions::Export::instance().stopWorker();
  MQTT::Publisher::instance().stopWorker();

  // Join Sessions Player + DatabaseManager threads here (mirrors onQuit) so they stop under qApp.
  Sessions::Player::instance().closeFile();
  Sessions::Player::instance().shutdown();
  Sessions::DatabaseManager::instance().closeDatabase(false);
  Sessions::DatabaseManager::instance().shutdown();
#endif
}

//--------------------------------------------------------------------------------------------------
// Benchmark execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drives FrameReader -> FrameBuilder -> consumers end-to-end and measures parsed frames/sec.
 */
HotpathBenchmark::Result HotpathBenchmark::run(
  quint64 targetFrames, double minFps, double minSeconds, int language, bool withExporters)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);
  Q_ASSERT(minSeconds >= 0.0);

  constexpr int kChannels       = 8;
  constexpr int kFramesPerChunk = 1000;

  setupProject(language, kChannels);
  if (withExporters)
    enableConsumers();

  const QByteArray chunk = buildChunk(kFramesPerChunk, kChannels);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  // Measure raw pipeline capacity: the budget guard is an interactive throttle, not a hot path.
  auto& builder = DataModel::FrameBuilder::instance();
  builder.setParseBudgetEnabled(false);
  builder.resetFrameCounters();

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;

  using Clock = std::chrono::steady_clock;

  // Hard chunk bound so a stalled pipeline can never spin forever (1e9 fps is far above any CPU).
  const quint64 maxFrames = targetFrames + static_cast<quint64>(minSeconds * 1.0e9);
  const quint64 maxChunks = maxFrames / kFramesPerChunk + 1;

  quint64 fed      = 0;
  double seconds   = 0.0;
  const auto start = Clock::now();

  // Run until both the frame target and the wall-clock floor are met, whichever takes longer.
  for (quint64 c = 0; c < maxChunks && (fed < targetFrames || seconds < minSeconds); ++c) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained))
      builder.hotpathRxFrame(drained);
    // code-verify on

    fed     += kFramesPerChunk;
    seconds  = std::chrono::duration<double>(Clock::now() - start).count();
  }

  const quint64 parsed  = builder.parsedFrameCount();
  const quint64 skipped = builder.skippedFrameCount();
  const double fps      = seconds > 0.0 ? static_cast<double>(parsed) / seconds : 0.0;

  builder.setParseBudgetEnabled(true);

  if (withExporters)
    disableConsumers();

  Result result;
  result.passed          = fps >= minFps;
  result.language        = language;
  result.minFps          = minFps;
  result.framesPerSecond = fps;
  result.elapsedSeconds  = seconds;
  result.framesParsed    = parsed;
  result.framesSkipped   = skipped;
  return result;
}

/**
 * @brief Runs the gated Lua, training-only JS, and exporters-on diagnostic pipelines; returns code.
 */
int HotpathBenchmark::runAndReport(quint64 targetFrames, double minFps, double minSeconds)
{
  // Gated run: clean Lua pipeline (worst-case parser). Part of the release gate.
  const Result lua = run(targetFrames, minFps, minSeconds, SerialStudio::Lua, false);

  // Gated run: clean JS pipeline, threshold pinned at half the Lua gate (Lua must stay >= 2x JS).
  const Result js = run(targetFrames, minFps * 0.5, minSeconds, SerialStudio::JavaScript, false);

  // Diagnostic + training run: Lua with every exporter live. Ungated; shows the exporter slowdown.
  const Result luaX = run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, true);

  const auto report = [](const char* tag, const Result& r) {
    std::fprintf(stdout,
                 "hotpath[%s]: %llu parsed, %llu skipped in %.2fs\n",
                 tag,
                 static_cast<unsigned long long>(r.framesParsed),
                 static_cast<unsigned long long>(r.framesSkipped),
                 r.elapsedSeconds);
    std::fprintf(stdout,
                 "hotpath[%s]: %.0f frames/s  (target %.0f)  %s\n",
                 tag,
                 r.framesPerSecond,
                 r.minFps,
                 r.passed ? "PASS" : "FAIL");
  };

  report("lua", lua);
  report("js", js);
  report("lua+exporters", luaX);

  const double slowdown =
    luaX.framesPerSecond > 0.0 ? lua.framesPerSecond / luaX.framesPerSecond : 0.0;
  std::fprintf(stdout, "hotpath: exporters cost %.2fx throughput\n", slowdown);

  std::fprintf(stdout,
               "HOTPATH_FPS=%.0f HOTPATH_TARGET=%.0f HOTPATH_JS_FPS=%.0f HOTPATH_JS_TARGET=%.0f "
               "HOTPATH_PASS=%d HOTPATH_EXPORTER_FPS=%.0f\n",
               lua.framesPerSecond,
               lua.minFps,
               js.framesPerSecond,
               js.minFps,
               (lua.passed && js.passed) ? 1 : 0,
               luaX.framesPerSecond);
  std::fflush(stdout);

  // Stop the JS watchdog thread while QApplication is alive (its aboutToQuit hook never fires).
  DataModel::JsWatchdogThread::instance().shutdown();

  // Destroy IO drivers now so the USB libusb event thread joins before static destruction.
  IO::ConnectionManager::instance().shutdownDrivers();

  return (lua.passed && js.passed) ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace Misc
