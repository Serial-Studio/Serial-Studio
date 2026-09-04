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

#include "Benchmark/BenchmarkRunner.h"

#if defined(SS_MIMALLOC_ACTIVE)
#  include <mimalloc.h>
#endif

#include <cmath>
#include <QClipboard>
#include <QGuiApplication>
#include <QLocale>
#include <QSysInfo>
#include <QTimer>
#include <QVariantMap>

#include "API/Server.h"
#include "AppState.h"
#include "Benchmark/HotpathBenchmark.h"
#include "Core/SSAssert.h"
#include "CSV/Export.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "IO/ConnectionManager.h"
#include "IO/PipelineHost.h"
#include "MDF4/Player.h"
#include "Misc/Translator.h"
#include "Misc/WorkspaceManager.h"
#include "Platform/AppPlatform.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#ifdef BUILD_COMMERCIAL
#  include "MDF4/Export.h"
#  include "Sessions/Export.h"
#  include "Sessions/Player.h"
#endif
#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

namespace Benchmark {

//--------------------------------------------------------------------------------------------------
// Gated targets + selectable workloads
//--------------------------------------------------------------------------------------------------

// Per-engine parser gates (frames/s). Export and dashboard phases are informational (ungated).
static constexpr double kDataPipelineFps  = 1024000.0;
static constexpr double kNativeNumericFps = 1024000.0;
static constexpr double kNativeMixedFps   = 512000.0;
static constexpr double kLuaNumericFps    = 256000.0;
static constexpr double kLuaMixedFps      = 128000.0;
static constexpr double kJsNumericFps     = 128000.0;
static constexpr double kJsMixedFps       = 64000.0;
static constexpr double kUngatedFps       = 1.0;

static const quint64 kFrameValues[]  = {100'000ull, 250'000ull, 500'000ull, 1'000'000ull};
static const double kSecondValues[]  = {1.0, 2.0, 5.0, 10.0};
static constexpr int kFrameCount     = static_cast<int>(sizeof(kFrameValues) / sizeof(quint64));
static constexpr int kSecondCount    = static_cast<int>(sizeof(kSecondValues) / sizeof(double));
static constexpr int kDefaultFrames  = 1;
static constexpr int kDefaultSeconds = 1;

//--------------------------------------------------------------------------------------------------
// Construction & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the runner in an idle, empty-result state.
 */
BenchmarkRunner::BenchmarkRunner()
  : m_running(false)
  , m_progress(0.0)
  , m_phaseIndex(0)
  , m_frames(kFrameValues[kDefaultFrames])
  , m_seconds(kSecondValues[kDefaultSeconds])
  , m_savedMode(SerialStudio::ProjectFile)
  , m_savedEphemeral(false)
  , m_savedPlotTimeRange(0.0)
  , m_savedCsvExport(false)
  , m_savedApiServer(false)
#ifdef BUILD_COMMERCIAL
  , m_savedMdfExport(false)
  , m_savedSessionExport(false)
#endif
#ifdef ENABLE_GRPC
  , m_savedGrpcServer(false)
#endif
{
  retranslate();
  static auto& translator = Misc::Translator::instance();
  connect(&translator, &Misc::Translator::languageChanged, this, &BenchmarkRunner::retranslate);

  static auto& ioManager = IO::ConnectionManager::instance();
  connect(&ioManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &BenchmarkRunner::deviceConnectedChanged);

  const auto playerOpenNotify = [this] {
    Q_EMIT playerOpenChanged();
  };
  static auto& csvPlayer = CSV::Player::instance();
  static auto& mdfPlayer = MDF4::Player::instance();
  connect(&csvPlayer, &CSV::Player::openChanged, this, playerOpenNotify);
  connect(&mdfPlayer, &MDF4::Player::openChanged, this, playerOpenNotify);
#ifdef BUILD_COMMERCIAL
  static auto& sessionPlayer = Sessions::Player::instance();
  connect(&sessionPlayer, &Sessions::Player::openChanged, this, playerOpenNotify);
#endif

  if (auto* app = qApp)
    connect(app, &QCoreApplication::aboutToQuit, this, &BenchmarkRunner::abortSession);
}

/**
 * @brief Returns the singleton instance of BenchmarkRunner.
 */
BenchmarkRunner& BenchmarkRunner::instance()
{
  static BenchmarkRunner instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a benchmark session is in progress.
 */
bool BenchmarkRunner::running() const noexcept
{
  return m_running;
}

/**
 * @brief Returns true while a device is streaming: the benchmark owns the processing objects for
 *        the run (it drives them from the GUI thread), so a live pipeline-thread producer would
 *        both wreck the measurement and reach a FrameBuilder that no longer lives there.
 */
bool BenchmarkRunner::deviceConnected() const
{
  static auto& ioManager = IO::ConnectionManager::instance();
  return ioManager.isConnected();
}

/**
 * @brief Returns true while a recording is loaded for playback: replayed frames enter the same
 *        FrameBuilder the phases are driving, so their rows would be mixed into the measurement.
 */
bool BenchmarkRunner::playerOpen() const
{
  return SerialStudio::isAnyPlayerOpen();
}

/**
 * @brief Returns the completed fraction of the current session (0..1).
 */
double BenchmarkRunner::progress() const noexcept
{
  return m_progress;
}

/**
 * @brief Returns the human-readable label of the phase currently running.
 */
QString BenchmarkRunner::currentPhase() const
{
  return m_currentPhase;
}

/**
 * @brief Returns one map per completed phase for the QML results table.
 */
QVariantList BenchmarkRunner::results() const
{
  return m_results;
}

/**
 * @brief Returns the process peak resident set size, formatted (empty until a run completes).
 */
QString BenchmarkRunner::peakMemory() const
{
  return m_peakMemory;
}

/**
 * @brief Returns the selectable frame-count workloads as display strings.
 */
QStringList BenchmarkRunner::frameOptions() const
{
  return m_frameOptions;
}

/**
 * @brief Returns the selectable wall-clock floors as display strings.
 */
QStringList BenchmarkRunner::secondsOptions() const
{
  return m_secondsOptions;
}

/**
 * @brief Formats a count with thousands separators, forcing grouping even in the C locale.
 */
QString BenchmarkRunner::formatCount(double value) const
{
  QLocale locale;
  locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
  return locale.toString(static_cast<qulonglong>(value > 0.0 ? std::llround(value) : 0));
}

/**
 * @brief Rebuilds every user-facing string for the active language (Translator-driven).
 */
void BenchmarkRunner::retranslate()
{
  m_frameOptions   = {tr("100 K frames"), tr("250 K frames"), tr("500 K frames"), tr("1 M frames")};
  m_secondsOptions = {tr("1 second"), tr("2 seconds"), tr("5 seconds"), tr("10 seconds")};
  Q_EMIT optionsChanged();
}

//--------------------------------------------------------------------------------------------------
// Public control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the results table to the clipboard as a Markdown report.
 */
void BenchmarkRunner::copyResults()
{
  if (m_results.isEmpty())
    return;

  QString text;
  text += tr("Serial Studio %1 - Hotpath Benchmark").arg(QCoreApplication::applicationVersion());
  text += QStringLiteral("\n");
  text += tr("%1 (%2), workload: %3 frames minimum, %4 s minimum")
            .arg(QSysInfo::prettyProductName(),
                 QSysInfo::currentCpuArchitecture(),
                 formatCount(static_cast<double>(m_frames)),
                 QString::number(m_seconds, 'f', 0));
  text += QStringLiteral("\n");
  text += tr("Build: %1").arg(HotpathBenchmark::buildProvenance());
  text += QStringLiteral("\n\n");
  text += QStringLiteral("| %1 | %2 | %3 | %4 | %5 |\n")
            .arg(tr("Pipeline"), tr("Throughput"), tr("Target"), tr("Time"), tr("Result"));
  text += QStringLiteral("|---|---:|---:|---:|---|\n");

  for (const auto& entry : std::as_const(m_results)) {
    const auto row       = entry.toMap();
    const bool gated     = row.value(QStringLiteral("gated")).toBool();
    const double fps     = SerialStudio::toDouble(row.value(QStringLiteral("fps")));
    const double seconds = SerialStudio::toDouble(row.value(QStringLiteral("seconds")));
    const double minFps  = SerialStudio::toDouble(row.value(QStringLiteral("target")));

    const QString target = gated ? tr("%1 frames/s").arg(formatCount(minFps)) : tr("n/a");
    const QString result =
      gated ? (row.value(QStringLiteral("passed")).toBool() ? tr("Pass") : tr("Fail")) : tr("n/a");

    text += QStringLiteral("| %1 | %2 | %3 | %4 | %5 |\n")
              .arg(row.value(QStringLiteral("label")).toString(),
                   tr("%1 frames/s").arg(formatCount(fps)),
                   target,
                   tr("%1 s").arg(QString::number(seconds, 'f', 2)),
                   result);
  }

  if (!m_peakMemory.isEmpty())
    text += tr("Peak memory: %1").arg(m_peakMemory) + QStringLiteral("\n");

  if (auto* clipboard = QGuiApplication::clipboard())
    clipboard->setText(text);
}

/**
 * @brief Clears the results table (no-op while a session is running).
 */
void BenchmarkRunner::clearResults()
{
  if (m_running || m_results.isEmpty())
    return;

  m_results.clear();
  Q_EMIT resultsChanged();
}

/**
 * @brief Assembles the ordered phase list from the user's section + variant selection.
 */
void BenchmarkRunner::buildPhases(
  bool parsers, bool dataExport, bool dashboard, bool numeric, bool mixed)
{
  m_phases.clear();

  m_phases.push_back({-1, false, false, false, true, kDataPipelineFps, tr("Data pipeline")});

  if (parsers)
    appendParserPhases(numeric, mixed);

  if (dataExport)
    appendDataExportPhases(numeric, mixed);

  if (dashboard)
    appendDashboardPhases(numeric, mixed);
}

/**
 * @brief Appends the parser-section phases: gated per engine + variant, the numbers CI enforces.
 */
void BenchmarkRunner::appendParserPhases(bool numeric, bool mixed)
{
  if (numeric) {
    m_phases.push_back({SerialStudio::Native,
                        false,
                        false,
                        false,
                        false,
                        kNativeNumericFps,
                        tr("Built-In parser (numeric)")});
    m_phases.push_back(
      {SerialStudio::Lua, false, false, false, false, kLuaNumericFps, tr("Lua parser (numeric)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        false,
                        false,
                        false,
                        false,
                        kJsNumericFps,
                        tr("JavaScript parser (numeric)")});
  }
  if (mixed) {
    m_phases.push_back({SerialStudio::Native,
                        false,
                        true,
                        false,
                        false,
                        kNativeMixedFps,
                        tr("Built-In parser (mixed)")});
    m_phases.push_back(
      {SerialStudio::Lua, false, true, false, false, kLuaMixedFps, tr("Lua parser (mixed)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        false,
                        true,
                        false,
                        false,
                        kJsMixedFps,
                        tr("JavaScript parser (mixed)")});
  }
}

/**
 * @brief Appends the data-export-section phases: informational, exporters on, every engine.
 */
void BenchmarkRunner::appendDataExportPhases(bool numeric, bool mixed)
{
  if (numeric) {
    m_phases.push_back({SerialStudio::Native,
                        true,
                        false,
                        false,
                        false,
                        kUngatedFps,
                        tr("Built-In + data export (numeric)")});
    m_phases.push_back({SerialStudio::Lua,
                        true,
                        false,
                        false,
                        false,
                        kUngatedFps,
                        tr("Lua + data export (numeric)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        true,
                        false,
                        false,
                        false,
                        kUngatedFps,
                        tr("JavaScript + data export (numeric)")});
  }
  if (mixed) {
    m_phases.push_back({SerialStudio::Native,
                        true,
                        true,
                        false,
                        false,
                        kUngatedFps,
                        tr("Built-In + data export (mixed)")});
    m_phases.push_back(
      {SerialStudio::Lua, true, true, false, false, kUngatedFps, tr("Lua + data export (mixed)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        true,
                        true,
                        false,
                        false,
                        kUngatedFps,
                        tr("JavaScript + data export (mixed)")});
  }
}

/**
 * @brief Appends the dashboard-section phases: informational, dashboard on, every engine.
 */
void BenchmarkRunner::appendDashboardPhases(bool numeric, bool mixed)
{
  if (numeric) {
    m_phases.push_back({SerialStudio::Native,
                        false,
                        false,
                        true,
                        false,
                        kUngatedFps,
                        tr("Built-In + dashboard (numeric)")});
    m_phases.push_back(
      {SerialStudio::Lua, false, false, true, false, kUngatedFps, tr("Lua + dashboard (numeric)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        false,
                        false,
                        true,
                        false,
                        kUngatedFps,
                        tr("JavaScript + dashboard (numeric)")});
  }
  if (mixed) {
    m_phases.push_back({SerialStudio::Native,
                        false,
                        true,
                        true,
                        false,
                        kUngatedFps,
                        tr("Built-In + dashboard (mixed)")});
    m_phases.push_back(
      {SerialStudio::Lua, false, true, true, false, kUngatedFps, tr("Lua + dashboard (mixed)")});
    m_phases.push_back({SerialStudio::JavaScript,
                        false,
                        true,
                        true,
                        false,
                        kUngatedFps,
                        tr("JavaScript + dashboard (mixed)")});
  }
}

/**
 * @brief Starts a benchmark session at the selected workload; schedules the first phase.
 */
void BenchmarkRunner::start(int framesIndex,
                            int secondsIndex,
                            bool parsers,
                            bool dataExport,
                            bool dashboard,
                            bool numeric,
                            bool mixed)
{
  if (m_running || deviceConnected() || playerOpen())
    return;

  if (!(parsers || dataExport || dashboard) || !(numeric || mixed))
    return;

  const int fi = qBound(0, framesIndex, kFrameCount - 1);
  const int si = qBound(0, secondsIndex, kSecondCount - 1);
  m_frames     = kFrameValues[fi];
  m_seconds    = kSecondValues[si];

  buildPhases(parsers, dataExport, dashboard, numeric, mixed);

  m_results.clear();
  Q_EMIT resultsChanged();

  m_progress = 0.0;
  Q_EMIT progressChanged();

  m_running = true;
  Q_EMIT runningChanged();

  beginSession();

  m_phaseIndex = 0;
  announcePhase(0);
}

//--------------------------------------------------------------------------------------------------
// Session save / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots project + consumer state, redirects exports to a throwaway workspace, and takes
 *        the FrameBuilder/FrameParser onto this thread: the phases drive the pipeline with plain
 *        synchronous calls, legal only while the script engines live here. The session stays
 *        ephemeral and non-persistent, so a quit mid-run writes none of it to QSettings.
 */
void BenchmarkRunner::beginSession()
{
  static auto& appState = AppState::instance();
  m_savedMode           = appState.operationMode();
  m_savedProjectPath    = appState.projectFilePath();
  m_savedEphemeral      = appState.ephemeralSession();
  appState.setEphemeralSession(true);

  static auto& dashboard = UI::Dashboard::instance();
  m_savedPlotTimeRange   = dashboard.plotTimeRange();
  dashboard.setSettingsPersistent(false);
  dashboard.setPlotTimeRange(10.0);

  static auto& csvExport = CSV::Export::instance();
  m_savedCsvExport       = csvExport.exportEnabled();
  csvExport.setSettingsPersistent(false);
  static auto& apiServer = API::Server::instance();
  m_savedApiServer       = apiServer.enabled();
#ifdef BUILD_COMMERCIAL
  static auto& mdfExport     = MDF4::Export::instance();
  m_savedMdfExport           = mdfExport.exportEnabled();
  static auto& sessionExport = Sessions::Export::instance();
  m_savedSessionExport       = sessionExport.exportEnabled();
  mdfExport.setSettingsPersistent(false);
  sessionExport.setSettingsPersistent(false);
#endif
#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  m_savedGrpcServer       = grpcServer.enabled();
#endif

  m_tempWorkspace = std::make_unique<QTemporaryDir>();
  if (m_tempWorkspace->isValid()) {
    static auto& workspaceManager = Misc::WorkspaceManager::instance();
    workspaceManager.setTemporaryPath(m_tempWorkspace->path());
  }

  static auto& pipeline = IO::PipelineHost::instance();
  pipeline.moveProcessingObjectsTo(thread());
}

/**
 * @brief Gives the processing objects back to the pipeline thread and restores the user's
 *        workspace/consumers/dashboard, re-arming settings persistence only once the saved
 *        values are back in memory (so the benchmark's values never reach the disk).
 */
void BenchmarkRunner::restoreEnvironment()
{
  static auto& pipeline = IO::PipelineHost::instance();
  pipeline.moveProcessingObjectsTo(pipeline.pipelineThread());

  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  workspaceManager.clearTemporaryPath();
  m_tempWorkspace.reset();

  static auto& dashboard = UI::Dashboard::instance();
  dashboard.setPlotTimeRange(m_savedPlotTimeRange);

  static auto& csvExport = CSV::Export::instance();
  csvExport.setExportEnabled(m_savedCsvExport);
  static auto& apiServer = API::Server::instance();
  apiServer.setEnabled(m_savedApiServer);
#ifdef BUILD_COMMERCIAL
  static auto& mdfExport = MDF4::Export::instance();
  mdfExport.setExportEnabled(m_savedMdfExport);
  static auto& sessionExport = Sessions::Export::instance();
  sessionExport.setExportEnabled(m_savedSessionExport);
#endif
#ifdef ENABLE_GRPC
  static auto& grpcServer = API::GRPC::GRPCServer::instance();
  grpcServer.setEnabled(m_savedGrpcServer);
#endif

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setSuppressMessageBoxes(false);
  static auto& frameParser = DataModel::FrameParser::instance();
  frameParser.setSuppressMessageBoxes(false);

  dashboard.setSettingsPersistent(true);
  csvExport.setSettingsPersistent(true);
#ifdef BUILD_COMMERCIAL
  mdfExport.setSettingsPersistent(true);
  sessionExport.setSettingsPersistent(true);
#endif
}

/**
 * @brief Restores the environment and then the user's project, which also re-arms the persisted
 *        project path the synthetic benchmark project displaced in memory.
 */
void BenchmarkRunner::endSession()
{
  restoreEnvironment();

  static auto& appState = AppState::instance();
  appState.setEphemeralSession(m_savedEphemeral);

  static auto& projectModel = DataModel::ProjectModel::instance();
  if (!m_savedProjectPath.isEmpty()) {
    appState.setOperationMode(SerialStudio::ProjectFile);
    (void)projectModel.openJsonFile(m_savedProjectPath);
  } else {
    appState.setOperationMode(static_cast<SerialStudio::OperationMode>(m_savedMode));
    static auto& frameBuilder = DataModel::FrameBuilder::instance();
    static auto& dashboard    = UI::Dashboard::instance();
    frameBuilder.syncFromProjectModel();
    dashboard.resetData();
  }
}

/**
 * @brief Tears a session down on application quit: the run never reaches its last phase, so the
 *        environment is restored without reloading the user's project (the session stayed
 *        ephemeral, so nothing of the benchmark's was ever persisted to restore over).
 */
void BenchmarkRunner::abortSession()
{
  if (!m_running)
    return;

  m_running = false;
  m_phases.clear();
  HotpathBenchmark::setActive(false);
  restoreEnvironment();
}

//--------------------------------------------------------------------------------------------------
// Phase execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes the phase label and defers the blocking run so the busy UI repaints first.
 */
void BenchmarkRunner::announcePhase(int index)
{
  SS_ASSERT(index >= 0 && index < static_cast<int>(m_phases.size()), {
    finishSession();
    return;
  });

  m_currentPhase = m_phases[index].label;
  Q_EMIT currentPhaseChanged();

  Q_EMIT dashboardPreviewActive(m_phases[index].dashboard);

  QTimer::singleShot(0, this, [this, index] { executePhase(index); });
}

/**
 * @brief Runs one phase (this blocks the UI), records its result, and advances or finishes.
 */
void BenchmarkRunner::executePhase(int index)
{
  if (!m_running)
    return;

  const int phaseCount = static_cast<int>(m_phases.size());
  SS_ASSERT(index >= 0 && index < phaseCount, {
    finishSession();
    return;
  });

  const PhaseSpec& spec = m_phases[index];
  const HotpathBenchmark::Result r =
    spec.dataPipeline ? HotpathBenchmark::runDataPipeline(m_frames, spec.minFps, m_seconds)
                      : HotpathBenchmark::run(m_frames,
                                              spec.minFps,
                                              m_seconds,
                                              spec.language,
                                              spec.exporters,
                                              spec.strings,
                                              spec.dashboard);

  QVariantMap row;
  row.insert(QStringLiteral("label"), spec.label);
  row.insert(QStringLiteral("fps"), r.framesPerSecond);
  row.insert(QStringLiteral("parsed"), static_cast<double>(r.framesParsed));
  row.insert(QStringLiteral("skipped"), static_cast<double>(r.framesSkipped));
  row.insert(QStringLiteral("seconds"), r.elapsedSeconds);
  row.insert(QStringLiteral("target"), r.minFps);
  row.insert(QStringLiteral("gated"), spec.minFps > 1.0);
  row.insert(QStringLiteral("passed"), r.passed);
  m_results.append(row);
  Q_EMIT resultsChanged();

  m_progress = static_cast<double>(index + 1) / phaseCount;
  Q_EMIT progressChanged();

  if (index + 1 < phaseCount) {
    m_phaseIndex = index + 1;
    announcePhase(index + 1);
    return;
  }

  finishSession();
}

/**
 * @brief Restores the session, clears the dashboard preview, and ends the run. Allocator stats
 *        go to stderr first so mimalloc tuning (mi_option_purge_delay in main.cpp) is validated
 *        against the run's real peak commit and purge counts instead of guessed.
 */
void BenchmarkRunner::finishSession()
{
#if defined(SS_MIMALLOC_ACTIVE)
  mi_stats_print(nullptr);
#endif

  const quint64 peakBytes = Platform::AppPlatform::peakResidentBytes();
  const double peakMiB    = static_cast<double>(peakBytes) / (1024.0 * 1024.0);
  m_peakMemory = peakBytes > 0 ? tr("%1 MiB").arg(QString::number(peakMiB, 'f', 1)) : QString();
  Q_EMIT peakMemoryChanged();

  endSession();
  Q_EMIT dashboardPreviewActive(false);

  m_currentPhase.clear();
  Q_EMIT currentPhaseChanged();

  m_running = false;
  Q_EMIT runningChanged();

  Q_EMIT finished();
}

}  // namespace Benchmark
