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

#include "Benchmark/BenchmarkRunner.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QSysInfo>
#include <QTimer>
#include <QVariantMap>

#include "API/Server.h"
#include "AppState.h"
#include "Benchmark/HotpathBenchmark.h"
#include "CSV/Export.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "Misc/Translator.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#ifdef BUILD_COMMERCIAL
#  include "MDF4/Export.h"
#  include "Sessions/Export.h"
#endif
#ifdef ENABLE_GRPC
#  include "API/GRPC/GRPCServer.h"
#endif

namespace Benchmark {

//--------------------------------------------------------------------------------------------------
// Phase table + selectable workloads
//--------------------------------------------------------------------------------------------------

struct PhaseSpec {
  int language;
  bool exporters;
  bool strings;
  bool dashboard;
  bool dataPipeline;
  double minFps;
};

// clang-format off
// code-verify off
static const PhaseSpec kPhases[] = {
  {                       -1, false, false, false,  true, 1024000.0}, // Data pipeline
  {     SerialStudio::Native, false, false, false, false, 1024000.0}, // Native parser (numeric)
  {     SerialStudio::Native, false,  true, false, false,  512000.0}, // Native parser (mixed)
  {        SerialStudio::Lua, false, false, false, false,  256000.0}, // Lua parser (numeric)
  { SerialStudio::JavaScript, false, false, false, false,  128000.0}, // Javascript parser (numeric)
  {        SerialStudio::Lua, false,  true, false, false,  128000.0}, // Lua parser (mixed)
  { SerialStudio::JavaScript, false,  true, false, false,   64000.0}, // Javascript parser (mixed)
  {        SerialStudio::Lua,  true,  true, false, false,       1.0}, // Lua + data export (mixed)
  {        SerialStudio::Lua, false, false,  true, false,       1.0}, // Lua + dashboard (numeric)
};
// code-verify on
// clang-format off

static constexpr int kPhaseCount = static_cast<int>(sizeof(kPhases) / sizeof(kPhases[0]));

static const quint64 kFrameValues[]    = {100'000ull, 250'000ull, 500'000ull, 1'000'000ull};
static const double kSecondValues[]    = {1.0, 2.0, 5.0, 10.0};
static constexpr int kFrameCount       = static_cast<int>(sizeof(kFrameValues) / sizeof(quint64));
static constexpr int kSecondCount      = static_cast<int>(sizeof(kSecondValues) / sizeof(double));
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
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &BenchmarkRunner::retranslate);
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
 * @brief Rebuilds every user-facing string for the active language (Translator-driven).
 */
void BenchmarkRunner::retranslate()
{
  m_phaseLabels    = {tr("Data pipeline"),
                      tr("Built-in parser (numeric)"),
                      tr("Built-in parser (mixed)"),
                      tr("Lua parser (numeric)"),
                      tr("JavaScript parser (numeric)"),
                      tr("Lua parser (mixed)"),
                      tr("JavaScript parser (mixed)"),
                      tr("Lua + data export (mixed)"),
                      tr("Lua + dashboard (numeric)")};
  m_frameOptions   = {tr("100 K frames"), tr("250 K frames"), tr("500 K frames"), tr("1 M frames")};
  m_secondsOptions = {tr("1 second"), tr("2 seconds"), tr("5 seconds"), tr("10 seconds")};

  // Re-translate the labels already shown in the results table (rows are in phase order).
  const int rows = qMin(m_results.size(), m_phaseLabels.size());
  for (int i = 0; i < rows; ++i) {
    QVariantMap row = m_results[i].toMap();
    row.insert(QStringLiteral("label"), m_phaseLabels[i]);
    m_results[i] = row;
  }

  Q_EMIT optionsChanged();
  if (rows > 0)
    Q_EMIT resultsChanged();
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
  text += tr("Serial Studio %1 - Hotpath Benchmark")
            .arg(QCoreApplication::applicationVersion());
  text += QStringLiteral("\n");
  text += tr("%1 (%2), workload: %3 frames minimum, %4 s minimum")
            .arg(QSysInfo::prettyProductName(),
                 QSysInfo::currentCpuArchitecture(),
                 QString::number(m_frames),
                 QString::number(m_seconds, 'f', 0));
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

    const QString target =
      gated ? tr("%1 frames/s").arg(QString::number(minFps, 'f', 0)) : tr("n/a");
    const QString result =
      gated ? (row.value(QStringLiteral("passed")).toBool() ? tr("Pass") : tr("Fail")) : tr("n/a");

    text += QStringLiteral("| %1 | %2 | %3 | %4 | %5 |\n")
              .arg(row.value(QStringLiteral("label")).toString(),
                   tr("%1 frames/s").arg(QString::number(fps, 'f', 0)),
                   target,
                   tr("%1 s").arg(QString::number(seconds, 'f', 2)),
                   result);
  }

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
 * @brief Starts a benchmark session at the selected workload; schedules the first phase.
 */
void BenchmarkRunner::start(int framesIndex, int secondsIndex)
{
  if (m_running)
    return;

  const int fi = qBound(0, framesIndex, kFrameCount - 1);
  const int si = qBound(0, secondsIndex, kSecondCount - 1);
  m_frames     = kFrameValues[fi];
  m_seconds    = kSecondValues[si];

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
 * @brief Snapshots project + consumer state and redirects exports into a throwaway workspace.
 */
void BenchmarkRunner::beginSession()
{
  // Snapshot the project so the synthetic benchmark project can be reverted afterwards.
  m_savedMode        = AppState::instance().operationMode();
  m_savedProjectPath = AppState::instance().projectFilePath();

  // Pin a fixed 10 s plot window so the dashboard phase has a defined axis range to render.
  m_savedPlotTimeRange = UI::Dashboard::instance().plotTimeRange();
  UI::Dashboard::instance().setPlotTimeRange(10.0);

  // Snapshot consumer toggles: HotpathBenchmark forces them on/off; restored verbatim afterward.
  m_savedCsvExport = CSV::Export::instance().exportEnabled();
  m_savedApiServer = API::Server::instance().enabled();
#ifdef BUILD_COMMERCIAL
  m_savedMdfExport     = MDF4::Export::instance().exportEnabled();
  m_savedSessionExport = Sessions::Export::instance().exportEnabled();
#endif
#ifdef ENABLE_GRPC
  m_savedGrpcServer = API::GRPC::GRPCServer::instance().enabled();
#endif

  // Redirect every exporter into a temp workspace, isolating output from the user's real data.
  m_tempWorkspace = std::make_unique<QTemporaryDir>();
  if (m_tempWorkspace->isValid())
    Misc::WorkspaceManager::instance().setTemporaryPath(m_tempWorkspace->path());
}

/**
 * @brief Restores the user's workspace/consumers/project and deletes the benchmark's temp files.
 */
void BenchmarkRunner::endSession()
{
  // Restore the workspace path; destroying the QTemporaryDir deletes every generated file.
  Misc::WorkspaceManager::instance().clearTemporaryPath();
  m_tempWorkspace.reset();

  // Restore the user's plot window.
  UI::Dashboard::instance().setPlotTimeRange(m_savedPlotTimeRange);

  // Restore consumer toggles to the user's pre-benchmark choices.
  CSV::Export::instance().setExportEnabled(m_savedCsvExport);
  API::Server::instance().setEnabled(m_savedApiServer);
#ifdef BUILD_COMMERCIAL
  MDF4::Export::instance().setExportEnabled(m_savedMdfExport);
  Sessions::Export::instance().setExportEnabled(m_savedSessionExport);
#endif
#ifdef ENABLE_GRPC
  API::GRPC::GRPCServer::instance().setEnabled(m_savedGrpcServer);
#endif

  // Drop setupProject()'s headless API mode before the reload so a failed reload is not silent.
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);
  DataModel::FrameParser::instance().setSuppressMessageBoxes(false);

  // Reload the user's project, or fall back to the prior mode with a clean dashboard.
  if (!m_savedProjectPath.isEmpty()) {
    AppState::instance().setOperationMode(SerialStudio::ProjectFile);
    (void)DataModel::ProjectModel::instance().openJsonFile(m_savedProjectPath);
  } else {
    AppState::instance().setOperationMode(static_cast<SerialStudio::OperationMode>(m_savedMode));
    DataModel::FrameBuilder::instance().syncFromProjectModel();
    UI::Dashboard::instance().resetData();
  }
}

//--------------------------------------------------------------------------------------------------
// Phase execution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes the phase label and defers the blocking run so the busy UI repaints first.
 */
void BenchmarkRunner::announcePhase(int index)
{
  Q_ASSERT(index >= 0 && index < kPhaseCount);

  m_currentPhase = m_phaseLabels.value(index);
  Q_EMIT currentPhaseChanged();

   Q_EMIT dashboardPreviewActive(false);
  if (kPhases[index].dashboard)
    Q_EMIT dashboardPreviewActive(true);

  QTimer::singleShot(0, this, [this, index] { executePhase(index); });
}

/**
 * @brief Runs one phase (this blocks the UI), records its result, and advances or finishes.
 */
void BenchmarkRunner::executePhase(int index)
{
  Q_ASSERT(index >= 0 && index < kPhaseCount);

  const PhaseSpec& spec = kPhases[index];
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
  row.insert(QStringLiteral("label"), m_phaseLabels.value(index));
  row.insert(QStringLiteral("fps"), r.framesPerSecond);
  row.insert(QStringLiteral("parsed"), static_cast<double>(r.framesParsed));
  row.insert(QStringLiteral("skipped"), static_cast<double>(r.framesSkipped));
  row.insert(QStringLiteral("seconds"), r.elapsedSeconds);
  row.insert(QStringLiteral("target"), r.minFps);
  row.insert(QStringLiteral("gated"), spec.minFps > 1.0);
  row.insert(QStringLiteral("passed"), r.passed);
  m_results.append(row);
  Q_EMIT resultsChanged();

  m_progress = static_cast<double>(index + 1) / kPhaseCount;
  Q_EMIT progressChanged();

  if (index + 1 < kPhaseCount) {
    m_phaseIndex = index + 1;
    announcePhase(index + 1);
    return;
  }

  finishSession();
}

/**
 * @brief Restores the session, clears the dashboard preview, and ends the run.
 */
void BenchmarkRunner::finishSession()
{
  endSession();
  Q_EMIT dashboardPreviewActive(false);

  m_currentPhase.clear();
  Q_EMIT currentPhaseChanged();

  m_running = false;
  Q_EMIT runningChanged();

  Q_EMIT finished();
}

}  // namespace Benchmark
