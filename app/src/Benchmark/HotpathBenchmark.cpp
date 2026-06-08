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

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <QByteArray>
#include <QByteArrayView>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <vector>

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
#include "Platform/AppPlatform.h"
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

// Event-loop pump interval so the dialog repaints mid-phase (macOS coalesces paints otherwise).
static constexpr double kSpinIntervalSec = 0.016;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Named rows: gated parser tiers plus the three lua reference rows the HOTPATH_* tokens read.
enum ReportIndex {
  kReportData,
  kReportNative,
  kReportNativeMix,
  kReportLua,
  kReportJs,
  kReportLuaMix,
  kReportJsMix,
  kReportLuaX,
  kReportLuaD,
  kReportLuaDoff,
  kReportCount
};

static constexpr const char* kReportTags[kReportCount] = {"data-pipeline",
                                                          "native(numeric)",
                                                          "native(mixed)",
                                                          "lua(numeric)",
                                                          "js(numeric)",
                                                          "lua(mixed)",
                                                          "js(mixed)",
                                                          "lua+exporters",
                                                          "lua+dashboard",
                                                          "lua+dashboard(off)"};

// One ungated coverage row: an engine x {numeric,mixed} x {exporters,dashboard} combination.
struct CoverageRow {
  int language;
  bool strings;
  bool exporters;
  bool dashboard;
  const char* tag;
};

// The rest of the matrix the named rows do not cover; ungated, run for code-path coverage only.
static constexpr CoverageRow kCoverageMatrix[] = {
  {    SerialStudio::Native, false,  true, false, "native+exporters(numeric)"},
  {       SerialStudio::Lua, false,  true, false,    "lua+exporters(numeric)"},
  {SerialStudio::JavaScript, false,  true, false,     "js+exporters(numeric)"},
  {    SerialStudio::Native,  true,  true, false,   "native+exporters(mixed)"},
  {SerialStudio::JavaScript,  true,  true, false,       "js+exporters(mixed)"},
  {    SerialStudio::Native, false, false,  true, "native+dashboard(numeric)"},
  {SerialStudio::JavaScript, false, false,  true,     "js+dashboard(numeric)"},
  {    SerialStudio::Native,  true, false,  true,   "native+dashboard(mixed)"},
  {       SerialStudio::Lua,  true, false,  true,      "lua+dashboard(mixed)"},
  {SerialStudio::JavaScript,  true, false,  true,       "js+dashboard(mixed)"},
};

static constexpr int kCoverageCount =
  static_cast<int>(sizeof(kCoverageMatrix) / sizeof(CoverageRow));

static constexpr int kReportColumns = 7;

/**
 * @brief Pumps the GUI event loop once and returns the wall-clock it consumed (discounted from
 * the measurement so a repaint never deflates throughput).
 */
[[nodiscard]] static double spinEventLoop()
{
  using Clock      = std::chrono::steady_clock;
  const auto start = Clock::now();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  return std::chrono::duration<double>(Clock::now() - start).count();
}

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

  UI::Dashboard::instance().updateStreamAvailable();
}

//--------------------------------------------------------------------------------------------------
// Synthetic project + workload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a comma-decoded N-channel source, optionally plus a trailing string datagrid group.
 */
QJsonObject HotpathBenchmark::buildProjectJson(int language, int channels, bool withStrings)
{
  Q_ASSERT(channels > 0);
  Q_ASSERT(language == SerialStudio::JavaScript || language == SerialStudio::Lua
           || language == SerialStudio::Native);

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

  QJsonArray groups{group};
  if (withStrings) {
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
    groups.append(stringGroup);
  }

  QJsonObject source;
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::Title, QStringLiteral("Benchmark"));
  source.insert(Keys::FrameStart, QString());
  source.insert(Keys::FrameEnd, QStringLiteral("\n"));
  source.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::DecoderMethod, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::FrameParserLanguage, language);

  if (language == SerialStudio::Native)
    source.insert(Keys::FrameParserTemplate, QStringLiteral("delimited"));
  else
    source.insert(Keys::FrameParserCode, DataModel::FrameParser::defaultTemplateCode(language));

  QJsonObject root;
  root.insert(Keys::Title, QStringLiteral("Hotpath Benchmark"));
  root.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  root.insert(Keys::Groups, groups);
  root.insert(Keys::Sources, QJsonArray{source});
  root.insert(Keys::Actions, QJsonArray{});
  return root;
}

/**
 * @brief Builds a project that exercises every per-frame dashboard sub-hotpath.
 */
QJsonObject HotpathBenchmark::buildDashboardProjectJson(int language, bool withStrings)
{
  Q_ASSERT(language == SerialStudio::JavaScript || language == SerialStudio::Lua
           || language == SerialStudio::Native);

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

  if (withStrings) {
    QJsonArray stringDatasets;
    for (int i = 0; i < kStringChannels; ++i)
      stringDatasets.append(
        makeDataset(kDashboardChannels + i + 1, QStringLiteral("STR%1").arg(i + 1)));

    groups.append(makeGroup(QStringLiteral("Status"), QStringLiteral("datagrid"), stringDatasets));
  }

  QJsonObject source;
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::Title, QStringLiteral("Benchmark"));
  source.insert(Keys::FrameStart, QString());
  source.insert(Keys::FrameEnd, QStringLiteral("\n"));
  source.insert(Keys::FrameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  source.insert(Keys::Decoder, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::DecoderMethod, static_cast<int>(SerialStudio::PlainText));
  source.insert(Keys::FrameParserLanguage, language);

  if (language == SerialStudio::Native)
    source.insert(Keys::FrameParserTemplate, QStringLiteral("delimited"));
  else
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

  static constexpr const char* kStatusWords[] = {"OK", "WARN", "FAIL", "IDLE"};
  constexpr int kStatusWordCount              = 4;
  constexpr double kTwoPi                     = 6.283185307179586;
  constexpr int kPeriod                       = 360;

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
void HotpathBenchmark::setupProject(int language, int channels, bool withStrings, bool dashboard)
{
  Q_ASSERT(channels > 0);

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  const QJsonObject root = dashboard ? buildDashboardProjectJson(language, withStrings)
                                     : buildProjectJson(language, channels, withStrings);
  const bool loaded      = project.loadFromJsonDocument(QJsonDocument(root));
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
  CSV::Export::instance().setExportEnabled(false);
  API::Server::instance().setEnabled(false);
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().setExportEnabled(false);
  Sessions::Export::instance().setExportEnabled(false);
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(false);
#endif

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
HotpathBenchmark::Result HotpathBenchmark::runDataPipeline(quint64 targetFrames,
                                                           double minFps,
                                                           double minSeconds)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);
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
  double spentSpin  = 0.0;
  double lastSpin   = 0.0;
  const auto start  = Clock::now();
  for (quint64 c = 0; c < maxChunks && (fed < targetFrames || seconds < minSeconds); ++c) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained))
      ++extracted;
    // code-verify on

    fed     += kFramesPerChunk;
    seconds  = std::chrono::duration<double>(Clock::now() - start).count() - spentSpin;

    if (seconds - lastSpin >= kSpinIntervalSec) {
      spentSpin += spinEventLoop();
      lastSpin   = seconds;
    }
  }

  const double fps = seconds > 0.0 ? static_cast<double>(extracted) / seconds : 0.0;

  Result result;
  result.passed          = fps >= minFps;
  result.language        = -1;
  result.minFps          = minFps;
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
                                               bool withStrings,
                                               bool withDashboard,
                                               bool dashboardIngest)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);
  Q_ASSERT(minSeconds >= 0.0);

  constexpr int kChannels       = 8;
  constexpr int kFramesPerChunk = 1000;
  const bool activateDashboard  = withDashboard && dashboardIngest;

  const int channels = withDashboard ? kDashboardChannels : kChannels;
  setupProject(language, channels, withStrings, withDashboard);
  if (withExporters)
    enableConsumers();

  if (activateDashboard)
    setActive(true);

  const int stringColumns = withStrings ? kStringChannels : 0;
  const QByteArray chunk  = buildChunk(kFramesPerChunk, channels, stringColumns);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  auto& builder = DataModel::FrameBuilder::instance();
  builder.setParseBudgetEnabled(false);

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;

  if (activateDashboard) {
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
  double spentSpin   = 0.0;
  double lastSpinSec = 0.0;
  const auto start   = Clock::now();
  for (quint64 c = 0; c < maxChunks && (fed < targetFrames || seconds < minSeconds); ++c) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained))
      builder.hotpathRxFrame(drained);
    // code-verify on

    fed     += kFramesPerChunk;
    seconds  = std::chrono::duration<double>(Clock::now() - start).count() - spentSpin;

    if (seconds - lastSpinSec >= kSpinIntervalSec) {
      spentSpin   += spinEventLoop();
      lastSpinSec  = seconds;
    }
  }

  const quint64 parsed  = builder.parsedFrameCount();
  const quint64 skipped = builder.skippedFrameCount();
  const double fps      = seconds > 0.0 ? static_cast<double>(parsed) / seconds : 0.0;

  builder.setParseBudgetEnabled(true);

  if (withExporters)
    disableConsumers();

  if (activateDashboard)
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
 * @brief Attributes the native numeric cost by composition (extraction run + tokenize-only loop,
 *        publish derived), so the hotpath itself carries no instrumentation.
 */
HotpathBenchmark::StageBreakdown HotpathBenchmark::measureNativeStages(const Result& data,
                                                                       const Result& native)
{
  Q_ASSERT(data.framesParsed > 0);

  StageBreakdown stages = {};
  if (data.framesPerSecond <= 0.0 || native.framesPerSecond <= 0.0) [[unlikely]]
    return stages;

  constexpr int kChannels        = 8;
  constexpr int kFramesPerChunk  = 1000;
  constexpr size_t kSampleFrames = 100000;
  constexpr int kTimingPasses    = 5;

  const QByteArray chunk = buildChunk(kFramesPerChunk, kChannels);

  IO::FrameReader reader;
  reader.setOperationMode(SerialStudio::QuickPlot);
  reader.setFinishSequences({QByteArrayLiteral("\n")});

  std::vector<IO::CapturedDataPtr> frames;
  frames.reserve(kSampleFrames);

  auto& queue = reader.queue();
  IO::CapturedDataPtr drained;
  while (frames.size() < kSampleFrames) {
    reader.processData(IO::makeCapturedData(chunk));

    // code-verify off
    while (queue.try_dequeue(drained) && frames.size() < kSampleFrames)
      frames.push_back(drained);
    // code-verify on
  }

  auto& parser = DataModel::FrameParser::instance();
  std::array<QByteArrayView, 64> spans;

  using Clock    = std::chrono::steady_clock;
  quint64 tokens = 0;
  const auto t0  = Clock::now();
  for (int pass = 0; pass < kTimingPasses; ++pass) {
    for (const auto& f : frames) {
      const qsizetype n = parser.parseSpansUtf8(f->data, 0, spans.data(), 64);
      if (n < 0) [[unlikely]]
        return stages;

      tokens += static_cast<quint64>(n);
    }
  }

  const double seconds = std::chrono::duration<double>(Clock::now() - t0).count();
  const double calls   = static_cast<double>(frames.size()) * kTimingPasses;
  if (calls <= 0.0 || tokens == 0) [[unlikely]]
    return stages;

  stages.tokenizeNs        = seconds * 1.0e9 / calls;
  stages.extractNs         = 1.0e9 / data.framesPerSecond;
  stages.totalNs           = 1.0e9 / native.framesPerSecond;
  stages.datasetsPublishNs = std::max(0.0, stages.totalNs - stages.extractNs - stages.tokenizeNs);
  stages.valid             = true;
  return stages;
}

//--------------------------------------------------------------------------------------------------
// Report generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a count with thousands separators (fixed English grouping for stable CI logs).
 */
[[nodiscard]] static QString groupedCount(double value)
{
  static const QLocale s_locale(QLocale::English);
  return s_locale.toString(static_cast<qulonglong>(value > 0.0 ? std::llround(value) : 0));
}

/**
 * @brief Fills one report row: ungated runs (informational tiers) show n/a target and result.
 */
static void fillReportRow(const HotpathBenchmark::Result& r, const char* tag, QString* cells)
{
  Q_ASSERT(tag != nullptr);
  Q_ASSERT(cells != nullptr);

  const bool gated = r.minFps > 1.0;
  cells[0]         = QString::fromLatin1(tag);
  cells[1]         = groupedCount(static_cast<double>(r.framesParsed));
  cells[2]         = groupedCount(static_cast<double>(r.framesSkipped));
  cells[3]         = QString::number(r.elapsedSeconds, 'f', 2);
  cells[4]         = groupedCount(r.framesPerSecond);
  cells[5]         = gated ? groupedCount(r.minFps) : QStringLiteral("n/a");
  if (gated)
    cells[6] = r.passed ? QStringLiteral("PASS") : QStringLiteral("FAIL");
  else
    cells[6] = QStringLiteral("n/a");
}

/**
 * @brief Prints the per-run throughput table through the shared stdout + file sink. The named
 *        rows (gated tiers + lua reference rows) print first, then the ungated coverage matrix.
 */
template<typename PrintFn>
static void printRunTable(const HotpathBenchmark::Result* results,
                          const HotpathBenchmark::Result* coverage,
                          const PrintFn& printData)
{
  Q_ASSERT(results != nullptr);
  Q_ASSERT(coverage != nullptr);

  static const QString s_headers[kReportColumns] = {QStringLiteral("Benchmark"),
                                                    QStringLiteral("Parsed"),
                                                    QStringLiteral("Skipped"),
                                                    QStringLiteral("Time (s)"),
                                                    QStringLiteral("Frames/s"),
                                                    QStringLiteral("Target"),
                                                    QStringLiteral("Result")};

  constexpr int kRowCount = kReportCount + kCoverageCount;
  QString cells[kRowCount][kReportColumns];
  for (int i = 0; i < kReportCount; ++i)
    fillReportRow(results[i], kReportTags[i], cells[i]);

  for (int i = 0; i < kCoverageCount; ++i)
    fillReportRow(coverage[i], kCoverageMatrix[i].tag, cells[kReportCount + i]);

  qsizetype widths[kReportColumns];
  for (int col = 0; col < kReportColumns; ++col) {
    widths[col] = s_headers[col].size();
    for (int row = 0; row < kRowCount; ++row)
      widths[col] = qMax(widths[col], cells[row][col].size());
  }

  const auto formatRow = [&](const QString* row) {
    QString line = QStringLiteral("|");
    for (int col = 0; col < kReportColumns; ++col) {
      const bool left  = (col == 0 || col == kReportColumns - 1);
      line            += QStringLiteral(" ");
      line += left ? row[col].leftJustified(widths[col]) : row[col].rightJustified(widths[col]);
      line += QStringLiteral(" |");
    }
    return line;
  };

  QString separator = QStringLiteral("+");
  for (int col = 0; col < kReportColumns; ++col)
    separator += QString(widths[col] + 2, QLatin1Char('-')) + QLatin1Char('+');

  printData("%s\n", separator.toUtf8().constData());
  printData("%s\n", formatRow(s_headers).toUtf8().constData());
  printData("%s\n", separator.toUtf8().constData());
  for (int row = 0; row < kRowCount; ++row)
    printData("%s\n", formatRow(cells[row]).toUtf8().constData());

  printData("%s\n", separator.toUtf8().constData());
}

/**
 * @brief Prints the per-run, stage, ratio, and machine-readable readouts; true when every gate
 *        passed.
 */
bool HotpathBenchmark::printReport(const Result* results,
                                   const Result* coverage,
                                   const StageBreakdown& stages,
                                   const QString& outputFile)
{
  Q_ASSERT(results != nullptr);
  Q_ASSERT(coverage != nullptr);

  QFile file(outputFile);
  const bool fileOpen  = !outputFile.isEmpty() && file.open(QIODevice::WriteOnly | QIODevice::Text);
  const auto printData = [&](const char* fmt, auto... args) {
    const QByteArray line = QString::asprintf(fmt, args...).toUtf8();
    std::fputs(line.constData(), stdout);
    if (fileOpen)
      file.write(line);
  };

  printRunTable(results, coverage, printData);

  const Result& data      = results[kReportData];
  const Result& native    = results[kReportNative];
  const Result& nativeMix = results[kReportNativeMix];
  const Result& lua       = results[kReportLua];
  const Result& js        = results[kReportJs];
  const Result& luaMix    = results[kReportLuaMix];
  const Result& jsMix     = results[kReportJsMix];
  const Result& luaX      = results[kReportLuaX];
  const Result& luaD      = results[kReportLuaD];
  const Result& luaDoff   = results[kReportLuaDoff];

  if (stages.valid) {
    printData("hotpath-stage[native]: extract %.0f ns, tokenize %.0f ns, datasets+publish %.0f ns "
              "(total %.0f ns/frame)\n",
              stages.extractNs,
              stages.tokenizeNs,
              stages.datasetsPublishNs,
              stages.totalNs);
  }

  const double slowdown =
    luaX.framesPerSecond > 0.0 ? luaMix.framesPerSecond / luaX.framesPerSecond : 0.0;
  printData("hotpath: exporters cost %.2fx throughput\n", slowdown);

  const double dashSlowdown =
    luaD.framesPerSecond > 0.0 ? lua.framesPerSecond / luaD.framesPerSecond : 0.0;
  printData("hotpath: dashboard costs %.2fx throughput\n", dashSlowdown);

  const double dashIngest =
    luaD.framesPerSecond > 0.0 ? luaDoff.framesPerSecond / luaD.framesPerSecond : 0.0;
  printData("hotpath: dashboard ingest costs %.2fx throughput (same project, ingest on vs off)\n",
            dashIngest);

  const bool allPassed = data.passed && native.passed && nativeMix.passed && lua.passed && js.passed
                      && luaMix.passed && jsMix.passed;
  printData("HOTPATH_FPS=%.0f HOTPATH_TARGET=%.0f HOTPATH_JS_FPS=%.0f HOTPATH_JS_TARGET=%.0f "
            "HOTPATH_PASS=%d HOTPATH_EXPORTER_FPS=%.0f HOTPATH_DASHBOARD_FPS=%.0f "
            "HOTPATH_DATA_FPS=%.0f HOTPATH_DATA_TARGET=%.0f\n",
            lua.framesPerSecond,
            lua.minFps,
            js.framesPerSecond,
            js.minFps,
            allPassed ? 1 : 0,
            luaX.framesPerSecond,
            luaD.framesPerSecond,
            data.framesPerSecond,
            data.minFps);
  printData("HOTPATH_LUA_MIXED_FPS=%.0f HOTPATH_LUA_MIXED_TARGET=%.0f "
            "HOTPATH_JS_MIXED_FPS=%.0f HOTPATH_JS_MIXED_TARGET=%.0f\n",
            luaMix.framesPerSecond,
            luaMix.minFps,
            jsMix.framesPerSecond,
            jsMix.minFps);
  printData("HOTPATH_DASHBOARD_OFF_FPS=%.0f HOTPATH_DASHBOARD_INGEST_COST=%.2f\n",
            luaDoff.framesPerSecond,
            dashIngest);
  printData("HOTPATH_NATIVE_FPS=%.0f HOTPATH_NATIVE_TARGET=%.0f "
            "HOTPATH_NATIVE_MIXED_FPS=%.0f HOTPATH_NATIVE_MIXED_TARGET=%.0f\n",
            native.framesPerSecond,
            native.minFps,
            nativeMix.framesPerSecond,
            nativeMix.minFps);
  if (stages.valid)
    printData("HOTPATH_STAGE_EXTRACT_NS=%.0f HOTPATH_STAGE_TOKENIZE_NS=%.0f "
              "HOTPATH_STAGE_PUBLISH_NS=%.0f\n",
              stages.extractNs,
              stages.tokenizeNs,
              stages.datasetsPublishNs);

  std::fflush(stdout);
  if (fileOpen) {
    file.flush();
    file.close();
  }

  return allPassed;
}

/**
 * @brief Runs the gated data + parser tiers, then the full ungated export x dashboard x engine
 *        coverage matrix so PGO training and CI exercise every consumer/engine combination.
 */
SS_NO_PGO
int HotpathBenchmark::runAndReport(quint64 targetFrames,
                                   double minFps,
                                   double minSeconds,
                                   const QString& outputFile)
{
  Q_ASSERT(targetFrames > 0);
  Q_ASSERT(minFps > 0.0);

  Platform::AppPlatform::registerIngestThreadWithMmcss();

  Result results[kReportCount] = {};
  results[kReportData]         = runDataPipeline(targetFrames, minFps * 4.0, minSeconds);
  results[kReportNative] =
    run(targetFrames, minFps * 4.0, minSeconds, SerialStudio::Native, false, false);

  const StageBreakdown stages = measureNativeStages(results[kReportData], results[kReportNative]);

  results[kReportNativeMix] =
    run(targetFrames, minFps * 2.0, minSeconds, SerialStudio::Native, false);
  results[kReportLua] = run(targetFrames, minFps, minSeconds, SerialStudio::Lua, false, false);
  results[kReportJs] =
    run(targetFrames, minFps * 0.5, minSeconds, SerialStudio::JavaScript, false, false);
  results[kReportLuaMix] = run(targetFrames, minFps * 0.5, minSeconds, SerialStudio::Lua, false);
  results[kReportJsMix] =
    run(targetFrames, minFps * 0.25, minSeconds, SerialStudio::JavaScript, false);
  results[kReportLuaX] = run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, true);
  results[kReportLuaD] = run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, false, false, true);

  results[kReportLuaDoff] =
    run(targetFrames, 1.0, minSeconds, SerialStudio::Lua, false, false, true, false);

  Result coverage[kCoverageCount] = {};
  for (int i = 0; i < kCoverageCount; ++i) {
    const CoverageRow& row = kCoverageMatrix[i];
    coverage[i] =
      run(targetFrames, 1.0, minSeconds, row.language, row.exporters, row.strings, row.dashboard);
  }

  const int code = printReport(results, coverage, stages, outputFile) ? EXIT_SUCCESS : EXIT_FAILURE;

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
