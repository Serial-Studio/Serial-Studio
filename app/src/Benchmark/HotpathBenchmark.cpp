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

#include "Benchmark/HotpathBenchmark.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
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
#include "UI/Dashboard.h"
#ifdef BUILD_COMMERCIAL
#  include "MDF4/Export.h"
#  include "Sessions/Export.h"
#endif
#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

// The benchmark hard-exits (skipping atexit), so PGO-instrumented builds flush the profile here.
#ifdef SS_PGO_INSTRUMENT
extern "C" {
#  if defined(__clang__)
int __llvm_profile_write_file(void);
#  elif defined(__GNUC__)
void __gcov_dump(void);
#  endif
}
#endif

// SS_NO_PGO excludes runAndReport from PGO: its body differs between the GENERATE and USE builds.
#if defined(__GNUC__) || defined(__clang__)
#  define SS_NO_PGO __attribute__((no_profile_instrument_function))
#else
#  define SS_NO_PGO
#endif

namespace Benchmark {

static bool s_benchmarkActive           = false;
static constexpr int kDashboardChannels = 13;
static constexpr int kStringChannels    = 3;

//--------------------------------------------------------------------------------------------------
// Benchmark-active flag (read by UI::Dashboard::streamAvailable)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while the headless dashboard benchmark is feeding frames.
 */
bool HotpathBenchmark::active() noexcept
{
  return s_benchmarkActive;
}

/**
 * @brief Marks the headless dashboard benchmark active so the Dashboard accepts frames.
 */
void HotpathBenchmark::setActive(bool active) noexcept
{
  s_benchmarkActive = active;
}

//--------------------------------------------------------------------------------------------------
// Synthetic project + workload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a comma-decoded N-channel source plus a trailing string datagrid group.
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
    ds.insert(Keys::Min, 0);
    ds.insert(Keys::Max, 0);
    datasets.append(ds);
  }

  QJsonObject group;
  group.insert(Keys::Title, QStringLiteral("Channels"));
  group.insert(Keys::Widget, QStringLiteral("multiplot"));
  group.insert(Keys::Datasets, datasets);

  QJsonArray stringDatasets;
  for (int i = 0; i < kStringChannels; ++i) {
    QJsonObject ds;
    ds.insert(Keys::Title, QStringLiteral("STR%1").arg(i + 1));
    ds.insert(Keys::Index, channels + i + 1);
    stringDatasets.append(ds);
  }

  QJsonObject stringGroup;
  stringGroup.insert(Keys::Title, QStringLiteral("Status"));
  stringGroup.insert(Keys::Widget, QStringLiteral("datagrid"));
  stringGroup.insert(Keys::Datasets, stringDatasets);

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
  root.insert(Keys::Groups, QJsonArray{group, stringGroup});
  root.insert(Keys::Sources, QJsonArray{source});
  root.insert(Keys::Actions, QJsonArray{});
  return root;
}

/**
 * @brief Builds a project that exercises every per-frame dashboard sub-hotpath.
 */
QJsonObject HotpathBenchmark::buildDashboardProjectJson(int language)
{
  Q_ASSERT(language == SerialStudio::JavaScript || language == SerialStudio::Lua);

  const auto makeDataset = [](int index, const QString& title) {
    QJsonObject ds;
    ds.insert(Keys::Title, title);
    ds.insert(Keys::Index, index);
    ds.insert(Keys::Min, 0);
    ds.insert(Keys::Max, 0);
    return ds;
  };
  const auto makeAxis = [&](int index, const QString& widget) {
    QJsonObject ds = makeDataset(index, widget);
    ds.insert(Keys::Widget, widget);
    return ds;
  };
  const auto makeGroup = [](const QString& title, const QString& widget, const QJsonArray& ds) {
    QJsonObject g;
    g.insert(Keys::Title, title);
    g.insert(Keys::Widget, widget);
    g.insert(Keys::Datasets, ds);
    return g;
  };

  QJsonArray multiDatasets;
  for (int i = 1; i <= 3; ++i) {
    QJsonObject ds = makeDataset(i, QStringLiteral("MP%1").arg(i));
    ds.insert(Keys::Graph, true);
    multiDatasets.append(ds);
  }

  QJsonObject plotBar = makeDataset(4, QStringLiteral("Plot"));
  plotBar.insert(Keys::Graph, true);
  plotBar.insert(Keys::Widget, QStringLiteral("bar"));
  plotBar.insert(Keys::Max, 100);

  QJsonObject fft = makeDataset(5, QStringLiteral("FFT"));
  fft.insert(Keys::FFT, true);
  fft.insert(Keys::FFTSamples, 256);

  QJsonObject led = makeDataset(6, QStringLiteral("LED"));
  led.insert(Keys::Graph, true);
  led.insert(Keys::LED, true);

  QJsonObject waterfall = makeDataset(7, QStringLiteral("Waterfall"));
  waterfall.insert(Keys::FFT, true);
  waterfall.insert(Keys::Waterfall, true);
  waterfall.insert(Keys::FFTSamples, 256);

  QJsonArray gpsDatasets{makeAxis(8, QStringLiteral("lat")),
                         makeAxis(9, QStringLiteral("lon")),
                         makeAxis(10, QStringLiteral("alt"))};
  QJsonArray plot3dDatasets{makeAxis(11, QStringLiteral("x")),
                            makeAxis(12, QStringLiteral("y")),
                            makeAxis(13, QStringLiteral("z"))};

  QJsonArray groups{
    makeGroup(QStringLiteral("Channels"), QStringLiteral("multiplot"), multiDatasets),
    makeGroup(QStringLiteral("Sensors"),
              QStringLiteral("datagrid"),
              QJsonArray{plotBar, fft, led, waterfall}),
    makeGroup(QStringLiteral("Location"), QStringLiteral("gps"), gpsDatasets),
    makeGroup(QStringLiteral("Trajectory"), QStringLiteral("plot3d"), plot3dDatasets)};

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
  root.insert(Keys::Title, QStringLiteral("Hotpath Dashboard Benchmark"));
  root.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  root.insert(Keys::Groups, groups);
  root.insert(Keys::Sources, QJsonArray{source});
  root.insert(Keys::Actions, QJsonArray{});

  return root;
}

/**
 * @brief Builds newline-delimited frames: sine numeric channels plus trailing string tokens.
 */
QByteArray HotpathBenchmark::buildChunk(int frames, int channels, int stringColumns)
{
  Q_ASSERT(frames > 0);
  Q_ASSERT(channels > 0);
  Q_ASSERT(stringColumns >= 0);

  // Rotating status words give the parser non-numeric tokens (toDouble fails) of varying length.
  static constexpr const char* kStatusWords[] = {"OK", "WARN", "FAIL", "IDLE"};
  constexpr int kStatusWordCount              = 4;

  // Varying values (not a constant) exercise auto-range and give PGO a realistic distribution.
  constexpr double kTwoPi = 6.283185307179586;
  constexpr int kPeriod   = 360;

  QByteArray chunk;
  chunk.reserve(static_cast<qsizetype>(frames) * (channels * 8 + stringColumns * 6));
  for (int i = 0; i < frames; ++i) {
    for (int ch = 0; ch < channels; ++ch) {
      if (ch > 0)
        chunk.append(',');

      const double phase = static_cast<double>(ch) / channels;
      const double t     = static_cast<double>(i % kPeriod) / kPeriod + phase;
      const double value = 50.0 + 40.0 * std::sin(kTwoPi * t);
      chunk.append(QByteArray::number(value, 'f', 3));
    }

    for (int s = 0; s < stringColumns; ++s) {
      chunk.append(',');
      chunk.append(kStatusWords[(i + s) % kStatusWordCount]);
    }

    chunk.append('\n');
  }

  Q_ASSERT(!chunk.isEmpty());
  return chunk;
}

//--------------------------------------------------------------------------------------------------
// Pipeline + consumer setup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the synthetic project and activates the parse pipeline for the given language.
 */
void HotpathBenchmark::setupProject(int language, int channels, bool dashboard)
{
  Q_ASSERT(channels > 0);

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  const QJsonObject root =
    dashboard ? buildDashboardProjectJson(language) : buildProjectJson(language, channels);
  const bool loaded = project.loadFromJsonDocument(QJsonDocument(root));
  Q_ASSERT(loaded);
  Q_UNUSED(loaded);

  auto& parser = DataModel::FrameParser::instance();
  parser.setSuppressMessageBoxes(true);
  parser.readCode();

  DataModel::FrameBuilder::instance().syncFromProjectModel();

  if (dashboard) {
    project.setCustomizeWorkspaces(true);
    project.setCustomizeWorkspaces(false);
  }
}

/**
 * @brief Enables every local export/output consumer (CSV/MDF4/Sessions/API/gRPC).
 */
void HotpathBenchmark::enableConsumers()
{
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
 * @brief Disables every consumer enabled by enableConsumers(), then drains their backlogs.
 */
void HotpathBenchmark::disableConsumers()
{
  // Disable flags only; stopWorker() here would orphan the worker and crash a later setEnabled().
  CSV::Export::instance().setExportEnabled(false);
  API::Server::instance().setEnabled(false);
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().setExportEnabled(false);
  Sessions::Export::instance().setExportEnabled(false);
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(false);
#endif

  // Flush the backlog so the queued heap frames are released instead of pinned until app exit.
  CSV::Export::instance().flushWorker();
  API::Server::instance().flushWorker();
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().flushWorker();
  Sessions::Export::instance().flushWorker();
#endif
}

/**
 * @brief Marks every plot/FFT/multiplot/waterfall widget running so its sub-hotpath fires.
 */
void HotpathBenchmark::activateDashboardWidgets()
{
  auto& dashboard = UI::Dashboard::instance();

  const int plots = dashboard.widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < plots; ++i)
    dashboard.setPlotRunning(i, true);

  const int ffts = dashboard.widgetCount(SerialStudio::DashboardFFT);
  for (int i = 0; i < ffts; ++i)
    dashboard.setFFTPlotRunning(i, true);

  const int multiplots = dashboard.widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < multiplots; ++i)
    dashboard.setMultiplotRunning(i, true);

#ifdef BUILD_COMMERCIAL
  const int waterfalls = dashboard.widgetCount(SerialStudio::DashboardWaterfall);
  for (int i = 0; i < waterfalls; ++i)
    dashboard.setWaterfallRunning(i, true);
#endif
}

//--------------------------------------------------------------------------------------------------
// Benchmark execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Measures raw FrameReader extraction throughput (the driver pipeline, no parse).
 */
HotpathBenchmark::Result HotpathBenchmark::runDataPipeline(quint64 targetFrames, double minSeconds)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minSeconds >= 0.0);

  constexpr int kChannels       = 8;
  constexpr int kFramesPerChunk = 1000;

  const QByteArray chunk = buildChunk(kFramesPerChunk, kChannels);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;

  using Clock             = std::chrono::steady_clock;
  const quint64 maxFrames = targetFrames + static_cast<quint64>(minSeconds * 1.0e9);
  const quint64 maxChunks = maxFrames / kFramesPerChunk + 1;

  quint64 extracted = 0;
  quint64 fed       = 0;
  double seconds    = 0.0;
  const auto start  = Clock::now();
  for (quint64 c = 0; c < maxChunks && (fed < targetFrames || seconds < minSeconds); ++c) {
    reader.processData(IO::makeCapturedData(chunk));

    // Drain the extracted frames without parsing; this is the driver-pipeline-only path.
    // code-verify off
    while (queue.try_dequeue(drained))
      ++extracted;
    // code-verify on

    fed     += kFramesPerChunk;
    seconds  = std::chrono::duration<double>(Clock::now() - start).count();
  }

  const double fps = seconds > 0.0 ? static_cast<double>(extracted) / seconds : 0.0;

  Result result;
  result.passed          = true;
  result.language        = -1;
  result.minFps          = 1.0;
  result.framesPerSecond = fps;
  result.elapsedSeconds  = seconds;
  result.framesParsed    = extracted;
  result.framesSkipped   = 0;
  return result;
}

/**
 * @brief Drives FrameReader -> FrameBuilder -> consumers end-to-end and measures parsed frames/sec.
 */
HotpathBenchmark::Result HotpathBenchmark::run(quint64 targetFrames,
                                               double minFps,
                                               double minSeconds,
                                               int language,
                                               bool withExporters,
                                               bool withDashboard)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);
  Q_ASSERT(minSeconds >= 0.0);

  constexpr int kChannels       = 8;
  constexpr int kFramesPerChunk = 1000;

  const int channels = withDashboard ? kDashboardChannels : kChannels;
  setupProject(language, channels, withDashboard);
  if (withExporters)
    enableConsumers();

  if (withDashboard)
    setActive(true);

  const int stringColumns = withDashboard ? 0 : kStringChannels;
  const QByteArray chunk  = buildChunk(kFramesPerChunk, channels, stringColumns);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  auto& builder = DataModel::FrameBuilder::instance();
  builder.setParseBudgetEnabled(false);

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;

  if (withDashboard) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained))
      builder.hotpathRxFrame(drained);
    // code-verify on

    activateDashboardWidgets();
  }

  builder.resetFrameCounters();

  using Clock             = std::chrono::steady_clock;
  const quint64 maxFrames = targetFrames + static_cast<quint64>(minSeconds * 1.0e9);
  const quint64 maxChunks = maxFrames / kFramesPerChunk + 1;

  quint64 fed        = 0;
  double seconds     = 0.0;
  double lastSpinSec = 0.0;
  const auto start   = Clock::now();
  for (quint64 c = 0; c < maxChunks && (fed < targetFrames || seconds < minSeconds); ++c) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained))
      builder.hotpathRxFrame(drained);
    // code-verify on

    fed     += kFramesPerChunk;
    seconds  = std::chrono::duration<double>(Clock::now() - start).count();

    if (withDashboard && seconds - lastSpinSec >= 0.016) {
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
      lastSpinSec = seconds;
    }
  }

  const quint64 parsed  = builder.parsedFrameCount();
  const quint64 skipped = builder.skippedFrameCount();
  const double fps      = seconds > 0.0 ? static_cast<double>(parsed) / seconds : 0.0;

  builder.setParseBudgetEnabled(true);

  if (withExporters)
    disableConsumers();

  if (withDashboard)
    setActive(false);

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
 * @brief Runs the gated Lua, JS, exporters-on, and dashboard-on pipelines; returns the exit code.
 */
SS_NO_PGO
int HotpathBenchmark::runAndReport(quint64 targetFrames,
                                   double minFps,
                                   double minSeconds,
                                   const QString& outputFile)
{
  const Result data = runDataPipeline(targetFrames, minSeconds);
  const Result lua  = run(targetFrames, minFps, minSeconds, SerialStudio::Lua, false);
  const Result js   = run(targetFrames, minFps * 0.5, minSeconds, SerialStudio::JavaScript, false);
  const Result luaX = run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, true);
  const Result luaD = run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, false, true);

  QFile file(outputFile);
  const bool fileOpen  = !outputFile.isEmpty() && file.open(QIODevice::WriteOnly | QIODevice::Text);
  const auto printData = [&](const char* fmt, auto... args) {
    const QByteArray line = QString::asprintf(fmt, args...).toUtf8();
    std::fputs(line.constData(), stdout);
    if (fileOpen)
      file.write(line);
  };

  const auto report = [&](const char* tag, const Result& r) {
    printData("hotpath[%s]: %llu parsed, %llu skipped in %.2fs\n",
              tag,
              static_cast<unsigned long long>(r.framesParsed),
              static_cast<unsigned long long>(r.framesSkipped),
              r.elapsedSeconds);
    printData("hotpath[%s]: %.0f frames/s  (target %.0f)  %s\n",
              tag,
              r.framesPerSecond,
              r.minFps,
              r.passed ? "PASS" : "FAIL");
  };

  report("data-pipeline", data);
  report("lua", lua);
  report("js", js);
  report("lua+exporters", luaX);
  report("lua+dashboard", luaD);

  const double slowdown =
    luaX.framesPerSecond > 0.0 ? lua.framesPerSecond / luaX.framesPerSecond : 0.0;
  printData("hotpath: exporters cost %.2fx throughput\n", slowdown);

  const double dashSlowdown =
    luaD.framesPerSecond > 0.0 ? lua.framesPerSecond / luaD.framesPerSecond : 0.0;
  printData("hotpath: dashboard costs %.2fx throughput\n", dashSlowdown);

  printData("HOTPATH_FPS=%.0f HOTPATH_TARGET=%.0f HOTPATH_JS_FPS=%.0f HOTPATH_JS_TARGET=%.0f "
            "HOTPATH_PASS=%d HOTPATH_EXPORTER_FPS=%.0f HOTPATH_DASHBOARD_FPS=%.0f "
            "HOTPATH_DATA_FPS=%.0f\n",
            lua.framesPerSecond,
            lua.minFps,
            js.framesPerSecond,
            js.minFps,
            (lua.passed && js.passed) ? 1 : 0,
            luaX.framesPerSecond,
            luaD.framesPerSecond,
            data.framesPerSecond);

  std::fflush(stdout);
  if (fileOpen) {
    file.flush();
    file.close();
  }

  const int code = (lua.passed && js.passed) ? EXIT_SUCCESS : EXIT_FAILURE;

#ifdef SS_PGO_INSTRUMENT
#  if defined(__clang__)
  (void)__llvm_profile_write_file();
#  elif defined(__GNUC__)
  __gcov_dump();
#  endif
#endif
  std::_Exit(code);
}

}  // namespace Benchmark
