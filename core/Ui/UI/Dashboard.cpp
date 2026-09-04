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

#include "UI/Dashboard.h"

#include "API/Mirror/MirrorSession.h"
#include "AppState.h"
#include "Benchmark/HotpathBenchmark.h"
#include "Core/SSAssert.h"
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/PipelineHost.h"
#include "MDF4/Player.h"
#include "Misc/TimerEvents.h"
#include "SessionContext.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"
#include "UI/Widgets/FFTWindow.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#  include "Sessions/Player.h"
#  include "UI/Widgets/AudioExport.h"
#endif

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kDefaultPlotPoints   = 1000;
constexpr int kDefaultPlotBuckets  = 1024;
constexpr int kMaxTimeRingSamples  = 262144;
constexpr double kAssumedMaxRateHz = 1024000.0;
constexpr double kTimeRingHeadroom = 1.25;

// Ceiling for a rate-sized ring: one cell per sample to 256 kHz on a 10 s axis, folding at 1 MHz
constexpr int kMaxRateSizedRingSamples = 1 << 22;
constexpr double kRingGrowthFactor     = 1.5;

// Ring-drain budget per display tick: kDrainBudgetNs / fps == 40% of the tick period
constexpr qint64 kDrainBudgetNs = 400000000LL;
constexpr int kMaxDisplayFps    = 240;
constexpr int kBudgetCheckMask  = 7;

/**
 * @brief Restores the saved run/pause flag of every widget the rebuild kept. Only keys the fresh
 *        map already holds are written: the push tables cache the address of each flag, so a new
 *        key would resolve to a slot nothing points at.
 */
static void restoreRunFlags(const QMap<int, bool>& saved, QMap<int, bool>& live)
{
  for (auto it = saved.cbegin(); it != saved.cend(); ++it) {
    auto slot = live.find(it.key());
    if (slot != live.end())
      slot.value() = it.value();
  }
}

/**
 * @brief Time-ring capacity for a window: enough for the assumed max rate, capped.
 */
static int timeRingCapacity(const double plotTimeRangeSec)
{
  const double want = plotTimeRangeSec * kAssumedMaxRateHz;
  if (want >= static_cast<double>(kMaxTimeRingSamples))
    return kMaxTimeRingSamples;

  return std::max(kDefaultPlotBuckets, static_cast<int>(want));
}

/**
 * @brief Time-ring capacity for a stream source: enough slots to hold the window at the source's
 *        real sample rate, bounded by a per-ring byte budget. Sizing off the actual rate is what
 *        keeps the trace spanning the whole axis -- a ring bounded in samples runs out of history
 *        in seconds as soon as the rate is high (44.1 kHz filled a 10 s axis to 5.9 s).
 */
static int streamRingCapacity(const double windowSec, const double sampleRateHz)
{
  const double want = windowSec * sampleRateHz;
  const double cap  = static_cast<double>(kMaxRateSizedRingSamples);
  return static_cast<int>(std::clamp(want, static_cast<double>(kDefaultPlotBuckets), cap));
}

/**
 * @brief Builds a scrolling-history ring for the visible window plus headroom, so a
 *        saturated min/max source (two slots per decimation cell) still spans the full
 *        axis instead of erasing at the left edge. A positive @p sampleRateHz sizes the ring
 *        for that rate, so a stream keeps one cell per sample until the byte budget binds.
 */
static DSP::EnvelopeRing makeHistoryRing(const double plotTimeRangeSec,
                                         const double sampleRateHz = 0)
{
  const double window = plotTimeRangeSec * kTimeRingHeadroom;
  if (sampleRateHz > 0)
    return DSP::EnvelopeRing(streamRingCapacity(window, sampleRateHz), window);

  return DSP::EnvelopeRing(timeRingCapacity(window), window);
}

/**
 * @brief Sample rate of the stream worker feeding a source, or 0 when the source is frame-fed.
 *        Read at layout-build time only; a source that connects later rebuilds the layout.
 */
static double streamSampleRate(const int sourceId)
{
  static auto& ioManager = IO::ConnectionManager::instance();

  for (const auto& worker : ioManager.streamWorkers())
    if (worker && worker->sourceId() == sourceId)
      return worker->config().sampleRate;

  return 0.0;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Dashboard, wires reset signals and loads persisted settings. Every module
 *        bound here is already built: the composition root pins this object last, so the binding
 *        resolves an existing instance instead of constructing one inside a constructor.
 */
UI::Dashboard::Dashboard()
  : m_appState(&AppState::instance())
  , m_widgetRegistry(&UI::WidgetRegistry::instance())
  , m_frameBuilder(&DataModel::FrameBuilder::instance())
  , m_projectModel(&DataModel::ProjectModel::instance())
  , m_drainBudgetNs(kDrainBudgetNs / kMaxDisplayFps)
  , m_points(kDefaultPlotPoints)
  , m_widgetCount(0)
  , m_updateRequired(false)
  , m_thinningActive(false)
  , m_updateRetryInProgress(false)
  , m_layoutValid(false)
  , m_streamAvailable(false)
  , m_plotTimeRange(10.0)
  , m_plotDisplayTimeSec(0)
  , m_pltXAxis(kDefaultPlotPoints)
  , m_multipltXAxis(kDefaultPlotPoints)
  , m_tools(m_settings, IO::ConnectionManager::instance(), DataModel::ProjectModel::instance())
  , m_viewState(m_settings)
  , m_plotControls(UI::PlotControlBindings{.activePlots      = m_activePlots,
                                           .activeFFTPlots   = m_activeFFTPlots,
                                           .activeMultiplots = m_activeMultiplots,
#ifdef BUILD_COMMERCIAL
                                           .activeWaterfalls = m_activeWaterfalls,
#endif
                                           .plotSweep      = m_plotSweep,
                                           .multiplotSweep = m_multiplotSweep})
  , m_replaySeek(UI::ReplaySeekBindings{.xAxisData          = m_xAxisData,
                                        .yAxisData          = m_yAxisData,
                                        .plotTimeRings      = m_plotTimeRings,
                                        .multiplotTimeRings = m_multiplotTimeRings,
                                        .multiplotValues    = m_multipltValues,
                                        .datasets           = m_datasets,
                                        .widgetGroups       = m_widgetGroups,
                                        .widgetDatasets     = m_widgetDatasets})
  , m_widgetMapBuilder(UI::WidgetModelBindings{.widgetCount         = m_widgetCount,
                                               .lastFrame           = m_lastFrame,
                                               .widgetMap           = m_widgetMap,
                                               .extensionGroupIds   = m_extensionGroupIds,
                                               .extensionDatasetIds = m_extensionDatasetIds,
                                               .datasets            = m_datasets,
                                               .sourceRawFrames     = m_sourceRawFrames,
                                               .datasetExtremes     = m_datasetExtremes,
                                               .valuePushes         = m_valuePushes,
                                               .extremePushes       = m_extremePushes,
                                               .datasetReferences   = m_datasetReferences,
                                               .widgetGroups        = m_widgetGroups,
                                               .widgetDatasets      = m_widgetDatasets},
                       *m_appState,
                       *m_projectModel,
                       *m_widgetRegistry,
                       UI::WidgetExtensions::instance())
  , m_ingest(UI::IngestBindings{.layoutValid           = m_layoutValid,
                                .streamAvailable       = m_streamAvailable,
                                .updateRequired        = m_updateRequired,
                                .updateRetryInProgress = m_updateRetryInProgress,
                                .widgetCount           = m_widgetCount,
                                .points                = m_points,
                                .plotDisplayTimeSec    = m_plotDisplayTimeSec,
                                .plotClocks            = m_plotClocks,
                                .widgetMap             = m_widgetMap,
                                .xAxisData             = m_xAxisData,
                                .yAxisData             = m_yAxisData,
                                .plotTimeRings         = m_plotTimeRings,
                                .multiplotTimeRings    = m_multiplotTimeRings,
                                .plotSweep             = m_plotSweep,
                                .multiplotSweep        = m_multiplotSweep,
                                .activePlots           = m_activePlots,
                                .activeFFTPlots        = m_activeFFTPlots,
                                .activeMultiplots      = m_activeMultiplots,
                                .gpsValues             = m_gpsValues,
                                .fftValues             = m_fftValues,
                                .pltValues             = m_pltValues,
                                .multipltValues        = m_multipltValues,
#ifdef BUILD_COMMERCIAL
                                .activeWaterfalls = m_activeWaterfalls,
                                .waterfallValues  = m_waterfallValues,
                                .plot3DRings      = m_plot3DRings,
                                .plotData3D       = m_plotData3D,
#endif
                                .datasets           = m_datasets,
                                .datasetExtremes    = m_datasetExtremes,
                                .valuePushes        = m_valuePushes,
                                .extremePushes      = m_extremePushes,
                                .widgetGroups       = m_widgetGroups,
                                .widgetDatasets     = m_widgetDatasets,
                                .sourceRawFrames    = m_sourceRawFrames,
                                .sourceStructureGen = m_sourceStructureGen},
             *this)
{
  connectSessionResets();
  connectStreamAvailableInputs();
  connectViewStateResets(*m_appState);
  connectDisplayTimers();
  connectToolSignals();

  updateStreamAvailable();
  restorePersistedSettings();
}

/**
 * @brief Wires every input that invalidates the dashboard's data: a player opening or closing, a
 *        device disconnect that leaves no stream behind, a project-file swap, an operation-mode
 *        change, and the builder's source-map edits.
 */
void UI::Dashboard::connectSessionResets()
{
  static auto* csvPlayer  = &CSV::Player::instance();
  static auto* mdf4Player = &MDF4::Player::instance();
  static auto* ioManager  = &IO::ConnectionManager::instance();

  // clang-format off
  connect(csvPlayer, &CSV::Player::openChanged, this, [this] { resetData(true); }, Qt::QueuedConnection);
  connect(mdf4Player, &MDF4::Player::openChanged, this, [this] { resetData(true); }, Qt::QueuedConnection);
  connect(ioManager, &IO::ConnectionManager::connectedChanged, this, [this] {
    if (!ioManager->isConnected() && !streamAvailable())
      resetData(true);
  }, Qt::QueuedConnection);
  connect(m_appState, &AppState::projectFileChanged, this, [this] { resetData(); }, Qt::QueuedConnection);
  connect(m_frameBuilder, &DataModel::FrameBuilder::jsonFileMapChanged, this, [this] {
    m_sourceRawFrames.clear();
    m_datasetReferences.clear();
    m_valuePushes.clear();
  }, Qt::QueuedConnection);
  connect(m_appState, &AppState::operationModeChanged, this, &UI::Dashboard::applyOperationModeDefaults, Qt::QueuedConnection);
  // clang-format on

#ifdef BUILD_COMMERCIAL
  static auto* sessPlayer = &Sessions::Player::instance();
  connect(
    sessPlayer,
    &Sessions::Player::openChanged,
    this,
    [this] { resetData(true); },
    Qt::QueuedConnection);
#endif
}

/**
 * @brief Re-derives the point count and the plot window after an operation-mode change: a project
 *        file dictates both, every other mode falls back to the persisted preference and drops the
 *        data, because a mode change replaces the widget identity space.
 */
void UI::Dashboard::applyOperationModeDefaults()
{
  const auto mode = m_appState->operationMode();
  if (mode == SerialStudio::ProjectFile) {
    const int project_pts = m_projectModel->pointCount();
    if (project_pts > 0 && m_points != project_pts) {
      m_points = project_pts;
      Q_EMIT pointsChanged();
    }

    const double project_range = m_projectModel->plotTimeRange();
    if (project_range > 0 && !qFuzzyCompare(m_plotTimeRange, project_range)) {
      m_plotTimeRange = project_range;
      Q_EMIT plotTimeRangeChanged();
    }
  } else {
    if (m_points != kDefaultPlotPoints) {
      m_points = kDefaultPlotPoints;
      Q_EMIT pointsChanged();
    }

    const double saved =
      qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));
    if (!qFuzzyCompare(m_plotTimeRange, saved)) {
      m_plotTimeRange = saved;
      Q_EMIT plotTimeRangeChanged();
    }

    resetData(true);
  }

  Q_EMIT frozenChanged();
}

/**
 * @brief Wires the display tick, the 1 Hz thinning poll and the per-tick ring-drain budget, which
 *        is re-derived whenever the user changes the display frame rate.
 */
void UI::Dashboard::connectDisplayTimers()
{
  static auto* timerEvents = &Misc::TimerEvents::instance();
  connect(timerEvents, &Misc::TimerEvents::uiTimeout, this, &UI::Dashboard::onDisplayTick);
  connect(timerEvents, &Misc::TimerEvents::timeout1Hz, this, &UI::Dashboard::pollThinningState);

  const auto refreshDrainBudget = [this] {
    m_drainBudgetNs = kDrainBudgetNs / qBound(1, timerEvents->fps(), kMaxDisplayFps);
  };

  connect(timerEvents, &Misc::TimerEvents::fpsChanged, this, refreshDrainBudget);
  refreshDrainBudget();
}

/**
 * @brief Republishes the notifications QML binds to through the facade: the tool-window flags and
 *        action list owned by DashboardTools, and the project's freeze and display-title edits.
 */
void UI::Dashboard::connectToolSignals()
{
  connect(this, &UI::Dashboard::widgetCountChanged, this, &UI::Dashboard::actionStatusChanged);

  // clang-format off
  connect(&m_tools, &UI::DashboardTools::actionStatusChanged, this, &UI::Dashboard::actionStatusChanged);
  connect(&m_tools, &UI::DashboardTools::clockEnabledChanged, this, &UI::Dashboard::clockEnabledChanged);
  connect(&m_tools, &UI::DashboardTools::stopwatchEnabledChanged, this, &UI::Dashboard::stopwatchEnabledChanged);
  connect(&m_tools, &UI::DashboardTools::terminalEnabledChanged, this, &UI::Dashboard::terminalEnabledChanged);
  connect(&m_tools, &UI::DashboardTools::notificationLogEnabledChanged, this, &UI::Dashboard::notificationLogEnabledChanged);
  // clang-format on

  connect(
    m_projectModel, &DataModel::ProjectModel::frozenChanged, this, &UI::Dashboard::frozenChanged);
  connect(m_projectModel,
          &DataModel::ProjectModel::widgetDisplayChanged,
          this,
          &UI::Dashboard::refreshDisplayTitles);
#ifdef BUILD_COMMERCIAL
  static auto* lemonSqueezy = &Licensing::LemonSqueezy::instance();
  connect(
    lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, &UI::Dashboard::frozenChanged);
#endif
}

/**
 * @brief Display-tick drain: ingests the frames and stream updates queued since the last tick
 *        under a shared wall-clock budget, refreshes the GUI-side data-table mirror the widget
 *        scripts read, then coalesces the widget repaint into one updated() emission (spec 0051
 *        M3/M5). The mirror refresh precedes updated() so scripts see this tick.
 */
void UI::Dashboard::onDisplayTick()
{
  QElapsedTimer clock;
  clock.start();

  SS_ASSERT(m_drainBudgetNs > 0, return);

  growTimeRings();
  drainStructureSnapshots();
  drainBlockRing(clock, m_drainBudgetNs);

  m_frameBuilder->drainTableSnapshot();
  m_frameBuilder->drainLatestFrameSnapshot();

  if (m_updateRequired) {
    m_updateRequired = false;
    Q_EMIT updated();
  }
}

/**
 * @brief Adopts every structure snapshot queued since the last tick. Drained BEFORE the block ring
 *        so a block never reaches a layout it was not staged under; snapshots arrive at
 * project-edit rate, so the loop is bounded by the ring and costs nothing in the steady state.
 */
void UI::Dashboard::drainStructureSnapshots()
{
  static auto& pipeline = IO::PipelineHost::instance();

  DataModel::StructureSnapshotPtr snapshot;
  // code-verify off
  // Ring drain: bounded by the structure ring capacity, provably finite per tick.
  while (pipeline.dequeueStructureSnapshot(snapshot))
    if (snapshot)
      applyStructureSnapshot(snapshot);
  // code-verify on

  snapshot.reset();
}

/**
 * @brief Drains the block ring under @p budgetNs of @p clock, hard-bounded by the ring capacity:
 *        the producer is a live thread, so draining "until empty" livelocks the GUI. Blocks past
 *        the budget are discarded except the newest, so the display stays current; exports are
 *        untouched, they fan out on the pipeline thread.
 */
void UI::Dashboard::drainBlockRing(const QElapsedTimer& clock, const qint64 budgetNs)
{
  static auto& pipeline = IO::PipelineHost::instance();

  const int max_drain = pipeline.dashboardRingCapacity();
  SS_ASSERT(max_drain > 0, return);
  SS_ASSERT(budgetNs > 0, return);

  quint64 discarded = 0;
  bool over_budget  = false;
  DataModel::DataBlockPtr block;
  DataModel::DataBlockPtr newest;

  for (int drained = 0; drained < max_drain && pipeline.dequeueDashboardBlock(block); ++drained) {
    if (over_budget) [[unlikely]] {
      newest = block;
      ++discarded;
      continue;
    }

    applyBlock(block);
    if ((drained & kBudgetCheckMask) == kBudgetCheckMask)
      over_budget = clock.nsecsElapsed() >= budgetNs;
  }

  if (newest) [[unlikely]] {
    applyBlock(newest);
    --discarded;
    newest.reset();
  }

  block.reset();
  pipeline.noteDisplayDrops(discarded);
}

/**
 * @brief Grows one saturated time ring whose source's measured cadence outruns the sizing it was
 *        built with. The frame lane cannot know its rate at layout time (that is the device's to
 *        decide), so a full ring is re-sized from the plot clock's smoothed sample period, upward
 *        only: shrinking would throw away history over a momentary lull.
 */
void UI::Dashboard::growTimeRing(DSP::EnvelopeRing& ring,
                                 const int sourceId,
                                 const double windowSec)
{
  if (ring.level0.time.size() < ring.level0.time.capacity())
    return;

  const auto clockIt = m_plotClocks.constFind(sourceId);
  if (clockIt == m_plotClocks.cend() || !(clockIt->samplePeriodSec > 0))
    return;

  const double want    = windowSec / clockIt->samplePeriodSec;
  const double ceiling = static_cast<double>(kMaxRateSizedRingSamples);
  const double desired = std::clamp(want, static_cast<double>(kDefaultPlotBuckets), ceiling);
  if (desired < static_cast<double>(ring.level0.time.capacity()) * kRingGrowthFactor)
    return;

  ring.resizeCapacity(static_cast<int>(desired), windowSec);
}

/**
 * @brief Re-checks every time ring against its source's measured cadence. Runs on the display
 *        tick, costs one hash probe per plot, and resizes only on the rare tick where a source
 *        proved faster than its ring; a stream source never triggers it, since its ring is
 *        already sized from the real rate and the plot clock only sees its block cadence.
 */
void UI::Dashboard::growTimeRings()
{
  const double window = m_plotTimeRange * kTimeRingHeadroom;

  for (auto it = m_plotTimeRings.begin(); it != m_plotTimeRings.end(); ++it)
    growTimeRing(
      it.value(), getDatasetWidget(SerialStudio::DashboardPlot, it.key()).sourceId, window);

  for (auto it = m_multiplotTimeRings.begin(); it != m_multiplotTimeRings.end(); ++it) {
    const int sourceId = getGroupWidget(SerialStudio::DashboardMultiPlot, it.key()).sourceId;
    for (auto& ring : it.value())
      growTimeRing(ring, sourceId, window);
  }
}

/**
 * @brief Ingests one published block through the ingest sub-object, which owns the push tables
 *        and the per-source plot clocks. The facade keeps the entry point because the mirror
 *        session and the benchmark drive it as a public slot.
 */
void UI::Dashboard::applyBlock(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT_HOTPATH(block);
  m_ingest.applyBlock(block);
}

/**
 * @brief Adopts one structure snapshot: caches the source's layout, rebuilds the widget model from
 *        the union of every cached source, and republishes the commercial-feature flag. Runs at
 *        project-edit rate, which is what lets the per-block path skip structural revalidation
 *        entirely -- the compare_frames() walk every frame used to pay for is gone.
 */
void UI::Dashboard::applyStructureSnapshot(const DataModel::StructureSnapshotPtr& snapshot)
{
  SS_ASSERT(snapshot != nullptr, return);

  if (snapshot->data.groups.empty() || !m_streamAvailable)
    return;

  const int sid             = snapshot->data.sourceId;
  const bool hadProFeatures = containsCommercialFeatures();

  m_sourceStructureGen[sid] = snapshot->generation;
  m_sourceRawFrames[sid]    = snapshot->data;

  reconfigureDashboard(combineSourceFrames(snapshot->data));

  if (hadProFeatures != containsCommercialFeatures())
    Q_EMIT containsCommercialFeaturesChanged();
}

/**
 * @brief Restores the persisted view-state flags and layout preferences from QSettings.
 */
void UI::Dashboard::restorePersistedSettings()
{
  m_viewState.restoreViewPreferences();
  m_tools.restorePersistedSettings();
  m_plotTimeRange =
    qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));

  m_viewState.restoreLayoutPreferences();
}

/**
 * @brief Returns this session's dashboard. The object is owned by the SessionContext and built
 *        last by the composition root, so a reach before adoption is a named fatal instead of an
 *        out-of-order lazy construction. Every widget and helper binds it once into a static or a
 *        member reference, so the draw path never re-enters this (spec 0039 M2, wave D3).
 */
UI::Dashboard& UI::Dashboard::instance()
{
  return SessionContext::current().dashboard();
}

//--------------------------------------------------------------------------------------------------
// Session view state (spec 0062)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records one per-widget view value; emits viewStateChanged only on a real change so the
 *        recording bundle's debounce sees edits, not repaints.
 */
void UI::Dashboard::saveWidgetViewState(const QString& widgetId,
                                        const QString& key,
                                        const QVariant& value)
{
  if (m_viewState.saveWidgetViewState(widgetId, key, value))
    Q_EMIT viewStateChanged();
}

/**
 * @brief Records one global view value (see saveWidgetViewState).
 */
void UI::Dashboard::saveGlobalViewState(const QString& key, const QVariant& value)
{
  if (m_viewState.saveGlobalViewState(key, value))
    Q_EMIT viewStateChanged();
}

/**
 * @brief Replaces the view state from a bundled document (session playback); widgets created
 *        afterwards read it in their Component.onCompleted. Malformed input clears it.
 */
void UI::Dashboard::setViewStateJson(const QString& json)
{
  if (m_viewState.setViewStateJson(json))
    Q_EMIT viewStateChanged();
}

/**
 * @brief Drops every recorded view value.
 */
void UI::Dashboard::clearViewState()
{
  if (m_viewState.clearViewState())
    Q_EMIT viewStateChanged();
}

//--------------------------------------------------------------------------------------------------
// Availability & state queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if the dashboard is currently available.
 */
bool UI::Dashboard::available() const
{
  return streamAvailable() && totalWidgetCount() > 0;
}

/**
 * @brief Returns the effective dashboard freeze state: the project's stored flag gated on an
 *        active Pro/Trial license and ProjectFile mode; QML-binding-time only, never read on
 *        the frame path. QuickPlot/ConsoleOnly is always live so a frozen project cannot lock
 *        the clean-slate dashboard.
 */
bool UI::Dashboard::frozen() const
{
  return m_projectModel->frozen() && SerialStudio::activated()
      && m_appState->operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief 1 Hz poll of the parse governor's thinning latch; emits only on transitions so QML
 *        bindings stay quiet while the state is stable.
 */
void UI::Dashboard::pollThinningState()
{
  const bool now_thinning = m_frameBuilder->parseBudgetThinning();
  if (now_thinning != m_thinningActive) {
    m_thinningActive = now_thinning;
    Q_EMIT thinningActiveChanged();
  }
}

/**
 * @brief Returns true when a plot dataset should render against time.
 */
bool UI::Dashboard::useTimeXAxis(const DataModel::Dataset& dataset) const
{
  return dataset.xAxisId == DataModel::kXAxisTime;
}

/**
 * @brief Returns true when a multiplot group should render against time.
 */
bool UI::Dashboard::useTimeXAxisGroup(const DataModel::Group& group) const
{
  return !group.datasets.empty()
      && SerialStudio::groupXAxisMode(group) == SerialStudio::XAxisMode::Time;
}

/**
 * @brief Checks if at least one data source/stream is active. An attached remote dashboard counts
 *        as a stream: its snapshots enter through applyBlock like any local block, and the flag
 *        read there is what decides whether they are drawn. The query is a plain flag read
 *        because this function is reached from the constructor, inside the pinned module order.
 */
bool UI::Dashboard::streamAvailable() const
{
  if (Benchmark::HotpathBenchmark::active()) [[unlikely]]
    return true;

  if (API::MirrorSession::mirroring()) [[unlikely]]
    return true;

  static auto& manager   = IO::ConnectionManager::instance();
  static auto& csvPlayer = CSV::Player::instance();
  static auto& mf4Player = MDF4::Player::instance();

  const bool csvOpen = csvPlayer.isOpen();
  const bool mf4Open = mf4Player.isOpen();
  const bool devOpen = manager.isConnected();

#ifdef BUILD_COMMERCIAL
  static auto& sessPlayer = Sessions::Player::instance();
  const bool sessOpen     = sessPlayer.isOpen();
  return devOpen || csvOpen || mf4Open || sessOpen;
#else
  return devOpen || csvOpen || mf4Open;
#endif
}

/**
 * @brief Refreshes the cached stream flag read by applyBlock; wired to every input that
 *        streamAvailable() derives from (connection, players, benchmark activation).
 */
void UI::Dashboard::updateStreamAvailable()
{
  m_streamAvailable = streamAvailable();

  static auto& pipeline = IO::PipelineHost::instance();
  pipeline.setDashboardAccepting(m_streamAvailable);
}

/**
 * @brief Wires every streamAvailable() input to the cache refresh. Direct connections keep the
 *        cached flag valid for frames arriving in the same event-loop turn. The mirror-attached
 *        input is wired from API::MirrorSession's own constructor, also direct: that module is
 *        built after the pinned order, so reaching it from here would add a constructor edge.
 */
void UI::Dashboard::connectStreamAvailableInputs()
{
  static auto& ioManager  = IO::ConnectionManager::instance();
  static auto& csvPlayer  = CSV::Player::instance();
  static auto& mdf4Player = MDF4::Player::instance();

  connect(&ioManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
  connect(&csvPlayer,
          &CSV::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
  connect(&mdf4Player,
          &MDF4::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
#ifdef BUILD_COMMERCIAL
  static auto& sessPlayer = Sessions::Player::instance();
  connect(&sessPlayer,
          &Sessions::Player::openChanged,
          this,
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);
#endif
}

/**
 * @brief Drops the session view state when the widget identity space changes: a widget id is only
 *        type:groupId:datasetIndex, so a new project or mode inherits foreign zoom, pan and
 *        cursors. Direct rather than queued, because Sessions::Player applies a recording's bundle
 *        right after the project load and a queued clear would land on top of it.
 */
void UI::Dashboard::connectViewStateResets(AppState& appState)
{
  connect(&appState,
          &AppState::projectFileChanged,
          this,
          &UI::Dashboard::clearViewState,
          Qt::DirectConnection);
  connect(&appState,
          &AppState::operationModeChanged,
          this,
          &UI::Dashboard::clearViewState,
          Qt::DirectConnection);
}

/**
 * @brief Determines if the point-selector widget should be visible.
 */
bool UI::Dashboard::pointsWidgetVisible() const
{
#ifdef BUILD_COMMERCIAL
  return m_widgetGroups.contains(SerialStudio::DashboardMultiPlot)
      || m_widgetDatasets.contains(SerialStudio::DashboardPlot)
      || m_widgetGroups.contains(SerialStudio::DashboardPlot3D);
#else
  return m_widgetGroups.contains(SerialStudio::DashboardMultiPlot)
      || m_widgetDatasets.contains(SerialStudio::DashboardPlot);
#endif
}

/**
 * @brief Returns true if the frame contains Pro-only features.
 */
bool UI::Dashboard::containsCommercialFeatures() const noexcept
{
  for (const auto& f : m_sourceRawFrames)
    if (f.containsCommercialFeatures)
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Data & widget queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a numerical value according to its context range.
 */
QString UI::Dashboard::formatValue(double val, double min, double max) const
{
  return FMT_VAL(val, min, max);
}

/**
 * @brief Counts the number of instances of a specified widget type.
 */
int UI::Dashboard::widgetCount(const SerialStudio::DashboardWidget widget) const
{
  if (widget == SerialStudio::DashboardExtension)
    return m_extensionGroupIds.count() + m_extensionDatasetIds.count();

  if (SerialStudio::isGroupWidget(widget)) {
    auto it = m_widgetGroups.constFind(widget);
    return it != m_widgetGroups.cend() ? it->count() : 0;
  }

  if (SerialStudio::isDatasetWidget(widget)) {
    auto it = m_widgetDatasets.constFind(widget);
    return it != m_widgetDatasets.cend() ? it->count() : 0;
  }

  return 0;
}

/**
 * @brief Resolves an extension widget's scope, owning package and bucket position from the
 *        relative index the widget map carries. Group-scope slots come first, so a caller that
 *        needs a group or dataset copy reads the scope from here instead of the enum.
 */
UI::Dashboard::ExtensionSlot UI::Dashboard::extensionSlot(const int relativeIndex) const
{
  ExtensionSlot slot;
  if (relativeIndex < 0)
    return slot;

  const int groups = m_extensionGroupIds.count();
  slot.group       = relativeIndex < groups;
  slot.bucketIndex = slot.group ? relativeIndex : relativeIndex - groups;
  slot.extensionId = extensionIdAt(slot.group, slot.bucketIndex);
  slot.valid       = !slot.extensionId.isEmpty();

  return slot;
}

/**
 * @brief Resolves the scope and bucket position of any widget slot: extension widgets answer from
 *        the package descriptor recorded at layout time, built-ins from the enum. This is the one
 *        place that discriminates group from dataset, because one enum value serves both extension
 *        scopes and the enum predicates therefore cannot answer for them.
 */
UI::Dashboard::ExtensionSlot UI::Dashboard::widgetSlot(const SerialStudio::DashboardWidget type,
                                                       const int relativeIndex) const
{
  if (type == SerialStudio::DashboardExtension)
    return extensionSlot(relativeIndex);

  ExtensionSlot slot;
  slot.group       = SerialStudio::isGroupWidget(type);
  slot.valid       = slot.group || SerialStudio::isDatasetWidget(type);
  slot.bucketIndex = relativeIndex;
  return slot;
}

//--------------------------------------------------------------------------------------------------
// Specialized data access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves a Group.uniqueId to its positional groupId in the live frame.
 */
int UI::Dashboard::groupIdForUniqueId(int uniqueId) const
{
  if (uniqueId < 0)
    return -1;

  for (const auto& group : m_lastFrame.groups)
    if (group.uniqueId == uniqueId)
      return group.groupId;

  return -1;
}

/**
 * @brief Resolves a positional groupId to its Group.uniqueId; returns -1 if absent.
 */
int UI::Dashboard::groupUniqueIdForGroupId(int groupId) const
{
  if (groupId < 0)
    return -1;

  for (const auto& group : m_lastFrame.groups)
    if (group.groupId == groupId)
      return group.uniqueId;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Dataset & group access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves a group widget by type and index.
 */
const DataModel::Group& UI::Dashboard::getGroupWidget(const SerialStudio::DashboardWidget widget,
                                                      const int index) const
{
  static const DataModel::Group emptyGroup;
  const auto it = m_widgetGroups.constFind(widget);

  if (it == m_widgetGroups.cend()) [[unlikely]] {
    qWarning() << "getGroupWidget: widget type not found:" << widget;
    return emptyGroup;
  }

  if (index < 0 || index >= it->size()) [[unlikely]] {
    qWarning() << "getGroupWidget: index out of bounds:" << index << "for widget" << widget;
    return emptyGroup;
  }

  return it->at(index);
}

/**
 * @brief Retrieves a dataset widget by type and index.
 */
const DataModel::Dataset& UI::Dashboard::getDatasetWidget(
  const SerialStudio::DashboardWidget widget, const int index) const
{
  static const DataModel::Dataset emptyDataset;
  const auto it = m_widgetDatasets.constFind(widget);

  if (it == m_widgetDatasets.cend()) [[unlikely]] {
    qWarning() << "getDatasetWidget: widget type not found:" << widget;
    return emptyDataset;
  }

  if (index < 0 || index >= it->size()) [[unlikely]] {
    qWarning() << "getDatasetWidget: index out of bounds:" << index << "for widget" << widget;
    return emptyDataset;
  }

  return it->at(index);
}

//--------------------------------------------------------------------------------------------------
// Time-series data access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the FFT plot data currently displayed on the dashboard.
 */
const DSP::AxisData& UI::Dashboard::fftData(const int index) const
{
  if (index < 0 || index >= m_fftValues.size()) [[unlikely]] {
    static const DSP::AxisData kEmpty;
    return kEmpty;
  }

  return m_fftValues[index];
}

/**
 * @brief Returns the GPS trajectory data currently tracked by the dashboard.
 */
const DSP::GpsSeries& UI::Dashboard::gpsSeries(const int index) const
{
  if (index < 0 || index >= m_gpsValues.size()) [[unlikely]] {
    static const DSP::GpsSeries kEmpty;
    return kEmpty;
  }

  return m_gpsValues[index];
}

/**
 * @brief Returns the Y-axis values for a linear plot widget.
 */
const DSP::LineSeries& UI::Dashboard::plotData(const int index) const
{
  if (index < 0 || index >= m_pltValues.size()) [[unlikely]] {
    static const DSP::LineSeries kEmpty{};
    return kEmpty;
  }

  return m_pltValues[index];
}

/**
 * @brief Returns the series data used by a multiplot widget.
 */
const DSP::MultiLineSeries& UI::Dashboard::multiplotData(const int index) const
{
  if (index < 0 || index >= m_multipltValues.size()) [[unlikely]] {
    static const DSP::MultiLineSeries kEmpty{};
    return kEmpty;
  }

  return m_multipltValues[index];
}

/**
 * @brief Returns the decimating time ring for a time-axis plot widget.
 */
const DSP::EnvelopeRing& UI::Dashboard::plotTimeRing(const int index) const
{
  const auto it = m_plotTimeRings.find(index);
  if (it == m_plotTimeRings.end()) [[unlikely]] {
    static const DSP::EnvelopeRing kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the per-curve decimating time rings for a time-axis multiplot widget.
 */
const std::vector<DSP::EnvelopeRing>& UI::Dashboard::multiplotTimeRings(const int index) const
{
  const auto it = m_multiplotTimeRings.find(index);
  if (it == m_multiplotTimeRings.end()) [[unlikely]] {
    static const std::vector<DSP::EnvelopeRing> kEmpty{};
    return kEmpty;
  }

  return it.value();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns the 3D trajectory data for a 3D plot widget.
 */
const DSP::LineSeries3D& UI::Dashboard::plotData3D(const int index) const
{
  if (index < 0 || index >= m_plotData3D.size()) [[unlikely]] {
    static const DSP::LineSeries3D kEmpty;
    return kEmpty;
  }

  SS_ASSERT(index < m_plot3DRings.size(), return m_plotData3D[index]);

  const auto& ring = m_plot3DRings[index];
  auto& snapshot   = m_plotData3D[index];

  const std::size_t count = ring.size();
  snapshot.resize(count);
  for (std::size_t k = 0; k < count; ++k)
    snapshot[k] = ring[k];

  return snapshot;
}

/**
 * @brief Returns the time-domain ring buffer feeding a waterfall widget.
 */
const DSP::AxisData& UI::Dashboard::waterfallData(const int index) const
{
  if (index < 0 || index >= m_waterfallValues.size()) [[unlikely]] {
    static const DSP::AxisData kEmpty;
    return kEmpty;
  }

  return m_waterfallValues[index];
}
#endif

//--------------------------------------------------------------------------------------------------
// UI configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the number of data points for the dashboard plots.
 */
void UI::Dashboard::setPoints(const int points)
{
  if (m_points != points) {
    m_points = points;

    if (m_appState->operationMode() != SerialStudio::ProjectFile)
      m_settings.setValue("Dashboard/Points", m_points);

    rebuildLineSeriesPreservingState();

    Q_EMIT pointsChanged();
  }
}

/**
 * @brief Resets all data in the dashboard, including plot values, widget structures, and actions.
 */
void UI::Dashboard::resetData(const bool notify)
{
  if (m_appState->operationMode() != SerialStudio::ProjectFile && m_points != kDefaultPlotPoints) {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  }

  m_widgetRegistry->clear();

  m_fftValues.clear();
  m_pltValues.clear();
  m_multipltValues.clear();

  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_multipltValues.squeeze();

  m_layoutValid = false;

  m_valuePushes.clear();
  m_extremePushes.clear();
  m_ingest.clearPushTables();

#ifdef BUILD_COMMERCIAL
  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plot3DRings.clear();
  m_plot3DRings.squeeze();
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
#endif

  m_gpsValues.clear();
  m_gpsValues.squeeze();

  m_xAxisData.clear();
  m_yAxisData.clear();
  m_plotTimeRings.clear();
  m_multiplotTimeRings.clear();
  m_plotSweep.clear();
  m_multiplotSweep.clear();
  resetPlotClocks();

  m_widgetCount = 0;
  m_widgetMap.clear();
  m_widgetGroups.clear();
  m_widgetDatasets.clear();
  m_datasetReferences.clear();
  m_extensionGroupIds.clear();
  m_extensionDatasetIds.clear();

  m_datasets.clear();

  m_activePlots.clear();
  m_activeFFTPlots.clear();
  m_activeMultiplots.clear();
#ifdef BUILD_COMMERCIAL
  m_activeWaterfalls.clear();
#endif

  m_lastFrame = DataModel::Frame();
  m_sourceRawFrames.clear();
  m_quarantinedSources.clear();
  m_updateRetryInProgress = false;

  if (m_appState->operationMode() == SerialStudio::ProjectFile) {
    DataModel::Frame templateFrame;
    auto* builder = m_frameBuilder;
    builder->invokeOnBuilderThreadBlocking(
      [&templateFrame, builder] { templateFrame = builder->frame(); });
    m_tools.configureActions(templateFrame);
  }

  if (notify) {
    m_datasetExtremes.clear();

    // code-verify off
    // Only a HARD reset (player open, disconnect) forgets the builder's published-structure
    // marks; the reconfigure path re-enters resetData(false), and forgetting there loops
    // reconfigure -> republish -> reconfigure at block rate (2026-08-19 incident).
    // code-verify on
    m_frameBuilder->forgetPublishedStructures();

    m_updateRequired = true;

    Q_EMIT updated();
    Q_EMIT dataReset();
    Q_EMIT widgetCountChanged();
    Q_EMIT containsCommercialFeaturesChanged();
  }
}

/**
 * @brief Clears only the time-series plot data without rebuilding the dashboard.
 */
void UI::Dashboard::clearPlotData()
{
  for (auto& fft : m_fftValues)
    fft.clear();

#ifdef BUILD_COMMERCIAL
  for (auto& wf : m_waterfallValues)
    wf.clear();
#endif

  for (auto it = m_yAxisData.begin(); it != m_yAxisData.end(); ++it)
    it.value().clear();

  for (auto it = m_xAxisData.begin(); it != m_xAxisData.end(); ++it)
    it.value().clear();

  for (auto it = m_plotTimeRings.begin(); it != m_plotTimeRings.end(); ++it)
    it.value().clear();

  for (auto it = m_multiplotTimeRings.begin(); it != m_multiplotTimeRings.end(); ++it)
    for (auto& ring : it.value())
      ring.clear();

  m_plotControls.resetSweepStates();
  resetPlotClocks();

  for (auto& multiSeries : m_multipltValues)
    for (auto& yAxis : multiSeries.y)
      yAxis.clear();

  for (auto& gps : m_gpsValues) {
    gps.latitudes.clear();
    gps.longitudes.clear();
    gps.altitudes.clear();
  }

#ifdef BUILD_COMMERCIAL
  for (auto& ring : m_plot3DRings)
    ring.clear();

  for (auto& plot3d : m_plotData3D)
    plot3d.clear();
#endif
}

//--------------------------------------------------------------------------------------------------
// Replay seek bulk fill (spec 0020)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds every plot ring from a replay seek window (spec 0020), then re-anchors what the
 *        rewritten timeline invalidated: the sweep captures and the plot clocks. Resetting the
 *        clocks stays the facade's job -- m_plotClocks and m_plotDisplayTimeSec are one state and
 *        only resetPlotClocks() clears both.
 */
void UI::Dashboard::bulkLoadPlotWindow(const QVector<double>& timesSec,
                                       const QHash<qint64, QVector<double>>& series)
{
  if (!m_layoutValid) [[unlikely]]
    return;

  if (!m_replaySeek.bulkLoadPlotWindow(timesSec, series))
    return;

  m_plotControls.resetSweepStates();
  resetPlotClocks();
  m_updateRequired = true;
}

/**
 * @brief Enables/disables the action panel.
 */
void UI::Dashboard::setShowActionPanel(const bool enabled)
{
  if (m_viewState.setShowActionPanel(enabled))
    Q_EMIT showActionPanelChanged();
}

/**
 * @brief Enables or disables auto-hiding the toolbar when the dashboard is shown.
 */
void UI::Dashboard::setAutoHideToolbar(const bool enabled)
{
  if (m_viewState.setAutoHideToolbar(enabled))
    Q_EMIT autoHideToolbarChanged();
}

/**
 * @brief Shows or hides the smart alignment guides drawn during manual-mode gestures.
 */
void UI::Dashboard::setShowAlignmentGuides(const bool enabled)
{
  if (m_viewState.setShowAlignmentGuides(enabled))
    Q_EMIT showAlignmentGuidesChanged();
}

/**
 * @brief Sets the canvas edge margin (px) shared by both layout modes; clamped to >= 0 and
 *        persisted.
 */
void UI::Dashboard::setLayoutMargin(const int margin)
{
  if (m_viewState.setLayoutMargin(margin))
    Q_EMIT layoutMarginChanged();
}

/**
 * @brief Sets the inter-window spacing (px) shared by both layout modes; clamped to >= -1
 *        (the default, which overlaps two borders into one shared line) and persisted.
 */
void UI::Dashboard::setLayoutSpacing(const int spacing)
{
  if (m_viewState.setLayoutSpacing(spacing))
    Q_EMIT layoutSpacingChanged();
}

/**
 * @brief Forwards the freeze toggle to the project model, the single owner of the stored
 *        flag; the effective state re-derives through the frozenChanged wiring.
 */
void UI::Dashboard::setFrozen(const bool frozen)
{
  if (m_appState->operationMode() != SerialStudio::ProjectFile)
    return;

  m_projectModel->setFrozen(frozen);
}

/**
 * @brief Sets the visible plot time window in seconds and notifies time-axis plots.
 */
void UI::Dashboard::setPlotTimeRange(const double seconds)
{
  const double clamped = qMax(0.001, seconds);
  if (qFuzzyCompare(m_plotTimeRange, clamped))
    return;

  m_plotTimeRange = clamped;

  if (m_appState->operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("Dashboard/PlotTimeRange", m_plotTimeRange);

  rebuildLineSeriesPreservingState();

  m_updateRequired = true;
  Q_EMIT plotTimeRangeChanged();
}

/**
 * @brief Toggles whether dashboard preference changes are written to QSettings.
 */
void UI::Dashboard::setSettingsPersistent(const bool persistent)
{
  m_tools.setSettingsPersistent(persistent);
  m_viewState.setSettingsPersistent(persistent);
}

//--------------------------------------------------------------------------------------------------
// Frame processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every per-source plot clock together with the display time they publish. The two
 *        are one state: a cleared clock restarts at the next publish, so a display time left
 *        behind stamps rings on a timeline that no longer exists, and their decimation grid
 *        clamps every later sample until wall time climbs back past it.
 */
void UI::Dashboard::resetPlotClocks()
{
  m_plotClocks.clear();
  m_plotDisplayTimeSec = 0.0;
}

//--------------------------------------------------------------------------------------------------
// Layout reconfiguration (the widget model itself is built by UI::WidgetMapBuilder)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a frame that does not match the widget model: one rebuild retry, then the
 *        (sourceId, structureGeneration) pair is quarantined and its frames dropped cheaply.
 *        The link, session, and players stay alive because a dashboard build failure is a
 *        rendering problem, never a reason to take the device away; a new generation retries.
 */
void UI::Dashboard::handleMissingDataset(const DataModel::Frame& frame)
{
  if (frame.groups.empty() || frame.sourceId < 0) [[unlikely]]
    return;

  const quint64 generation = m_sourceStructureGen.value(frame.sourceId);
  const auto qit           = m_quarantinedSources.constFind(frame.sourceId);
  if (qit != m_quarantinedSources.cend() && qit.value() == generation)
    return;

  if (m_updateRetryInProgress) {
    qWarning() << "[Dashboard] widget model build failed twice for source" << frame.sourceId
               << "-- dropping its blocks until the structure changes";
    m_quarantinedSources.insert(frame.sourceId, generation);
    m_updateRetryInProgress = false;
    return;
  }

  m_sourceRawFrames[frame.sourceId] = frame;
  reconfigureDashboard(combineSourceFrames(frame));
  m_updateRetryInProgress = true;
}

/**
 * @brief Merges every cached per-source frame into one layout frame. The reconfigure path must
 *        register the union of all sources because buildValuePushes() walks m_sourceRawFrames;
 *        a single-source frame would leave the other sources' push tables unresolved (INT_MIN)
 *        and trip handleMissingDataset() forever on multi-source projects.
 */
DataModel::Frame UI::Dashboard::combineSourceFrames(const DataModel::Frame& seed) const
{
  SS_ASSERT(seed.sourceId >= 0, return seed);

  DataModel::Frame combined;
  combined.title   = seed.title;
  combined.actions = seed.actions;

  // code-verify off
  for (const auto& sf : std::as_const(m_sourceRawFrames)) {
    combined.containsCommercialFeatures |= sf.containsCommercialFeatures;
    for (const auto& g : sf.groups)
      combined.groups.push_back(g);
  }
  // code-verify on

  return combined;
}

/**
 * @brief Reconfigures the dashboard layout and widgets based on the new frame. Time rings
 *        and the per-source cache are snapshotted before resetData() and restored after,
 *        so a cosmetic project edit that rebuilds the layout does not erase in-flight plot
 *        history.
 */
void UI::Dashboard::reconfigureDashboard(const DataModel::Frame& frame)
{
  SS_ASSERT(!frame.groups.empty(), return);
  SS_ASSERT(streamAvailable(), return);

  const bool pro = SerialStudio::activated();

  auto savedSourceFrames  = m_sourceRawFrames;
  auto savedClocks        = m_plotClocks;
  const double savedClock = m_plotDisplayTimeSec;

  auto savedPlotRings      = m_replaySeek.snapshotPlotTimeRings();
  auto savedMultiplotRings = m_replaySeek.snapshotMultiplotTimeRings();

  resetData(false);

  m_sourceRawFrames    = std::move(savedSourceFrames);
  m_plotClocks         = std::move(savedClocks);
  m_plotDisplayTimeSec = savedClock;

  m_lastFrame = frame;

  DataModel::Group terminal;
  terminal.widget   = "terminal";
  terminal.title    = tr("Console");
  terminal.groupId  = m_lastFrame.groups.size();
  terminal.uniqueId = DataModel::runtime_group_unique_id(terminal.groupId);
  m_lastFrame.groups.push_back(terminal);

#ifdef BUILD_COMMERCIAL
  DataModel::Group notif;
  notif.widget   = "notification-log";
  notif.title    = tr("Notifications");
  notif.groupId  = m_lastFrame.groups.size();
  notif.uniqueId = DataModel::runtime_group_unique_id(notif.groupId);
  m_lastFrame.groups.push_back(notif);
#endif

  DataModel::Group clock;
  clock.widget   = "clock";
  clock.title    = tr("Clock");
  clock.groupId  = m_lastFrame.groups.size();
  clock.uniqueId = DataModel::runtime_group_unique_id(clock.groupId);
  m_lastFrame.groups.push_back(clock);

  DataModel::Group stopwatch;
  stopwatch.widget   = "stopwatch";
  stopwatch.title    = tr("Stopwatch");
  stopwatch.groupId  = m_lastFrame.groups.size();
  stopwatch.uniqueId = DataModel::runtime_group_unique_id(stopwatch.groupId);
  m_lastFrame.groups.push_back(stopwatch);

  m_widgetMapBuilder.buildWidgetGroups(frame, pro);

  m_widgetMapBuilder.applyDisplayTitles();
  m_widgetMapBuilder.registerWidgets();

  m_widgetMapBuilder.buildDatasetReferences();
  m_widgetMapBuilder.buildValuePushes();

  m_ingest.updateDataSeries();
  m_tools.configureActions(frame);

  m_replaySeek.restorePlotTimeRings(savedPlotRings);
  m_replaySeek.restoreMultiplotTimeRings(savedMultiplotRings);

  m_layoutValid = true;

  Q_EMIT widgetCountChanged();
}

/**
 * @brief Re-applies display titles after an override edit without rebuilding the dashboard:
 *        patches the widget copies in place, pushes the new titles into the WidgetRegistry
 *        and notifies QML. Never emits widgetCountChanged (that would trigger a full
 *        delegate rebuild mid-stream).
 */
void UI::Dashboard::refreshDisplayTitles()
{
  if (m_widgetCount == 0 || m_lastFrame.groups.empty() || !m_layoutValid)
    return;

  m_widgetMapBuilder.applyDisplayTitles();
  rebuildPushTables();

  auto& registry = *m_widgetRegistry;
  for (auto i = m_widgetGroups.constBegin(); i != m_widgetGroups.constEnd(); ++i) {
    const auto key = i.key();
    for (int j = 0; j < i.value().size(); ++j)
      registry.updateWidget(registry.widgetIdByTypeAndIndex(key, j), i.value().at(j).title);
  }

  for (auto i = m_widgetDatasets.constBegin(); i != m_widgetDatasets.constEnd(); ++i) {
    const auto key = i.key();
    const int base = m_widgetMapBuilder.datasetBucketBase(key);
    for (int j = 0; j < i.value().size(); ++j)
      registry.updateWidget(registry.widgetIdByTypeAndIndex(key, base + j), i.value().at(j).title);
  }

  Q_EMIT displayTitlesChanged();
}

//--------------------------------------------------------------------------------------------------
// Data series configuration
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL
/**
 * @brief Arms or disarms the audio-recording tap on one FFT widget's ingest push. Taps are
 *        index-aligned with the FFT widget order and reset on every push-table rebuild, so a
 *        stale index can never fire against a re-indexed widget.
 */
void UI::Dashboard::setFftAudioTap(const int index, const bool enabled, const quint32 key)
{
  m_ingest.setFftAudioTap(index, enabled, key);
}
#endif

/**
 * @brief Initializes the GPS series structure for all GPS widgets.
 *        Two passes are deliberate: the push table stores raw pointers into m_gpsValues,
 *        and those addresses are stable only after that vector stops growing, so all
 *        series are allocated first and the pushes resolved in a second pass.
 */
void UI::Dashboard::configureGpsSeries()
{
  m_gpsValues.clear();
  m_gpsValues.squeeze();

  const int gpsCount = widgetCount(SerialStudio::DashboardGPS);
  for (int i = 0; i < gpsCount; ++i) {
    DSP::GpsSeries series;
    const auto& group = getGroupWidget(SerialStudio::DashboardGPS, i);
    const QMap<QString, DSP::FixedQueue<double>*> fieldMap = {
      {"lat",  &series.latitudes},
      {"lon", &series.longitudes},
      {"alt",  &series.altitudes}
    };

    for (size_t j = 0; j < group.datasets.size(); ++j) {
      const auto& dataset = group.datasets[j];
      if (fieldMap.contains(dataset.widget)) {
        auto* vector = fieldMap[dataset.widget];
        vector->resize(points() + 1);
        vector->fill(std::nan(""));
      }
    }

    m_gpsValues.append(series);
  }

  m_ingest.buildGpsPushes();
}

/**
 * @brief Configures the FFT series data structure. Ring capacity comes from the shared transform
 *        size contract, so it matches the plan FFTPlot allocates from the same dataset. Two passes
 *        are deliberate: the push table stores raw pointers into m_fftValues, stable only after
 *        that vector stops growing, so buffers are allocated first and pushes resolved second.
 */
void UI::Dashboard::configureFftSeries()
{
  m_fftValues.clear();
  m_fftValues.squeeze();
  m_activeFFTPlots.clear();

  const int fftCount = widgetCount(SerialStudio::DashboardFFT);
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    const int capacity  = Widgets::normalizedFftSize(dataset.fftSamples);
    m_fftValues.append(DSP::AxisData(capacity));
    m_activeFFTPlots.insert(i, true);
  }

  m_ingest.buildFftPushes();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Arms or disarms the audio-recording tap on one waterfall widget's ingest push. Taps
 *        are index-aligned with the waterfall widget order and reset on every push-table
 *        rebuild, so a stale index can never fire against a re-indexed widget.
 */
void UI::Dashboard::setWaterfallAudioTap(const int index, const bool enabled, const quint32 key)
{
  m_ingest.setWaterfallAudioTap(index, enabled, key);
}

/**
 * @brief Configures the waterfall series buffers. Ring capacity comes from the shared
 *        transform-size contract under the waterfall's own (lower) ceiling, so it matches the
 *        plan the widget allocates. Two passes are deliberate: the push table stores raw
 *        pointers into m_waterfallValues, stable only after that vector stops growing.
 */
void UI::Dashboard::configureWaterfallSeries()
{
  m_waterfallValues.clear();
  m_waterfallValues.squeeze();
  m_activeWaterfalls.clear();

  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);
    const int capacity =
      Widgets::normalizedFftSize(dataset.fftSamples, Widgets::kMaxWaterfallFftSize);
    m_waterfallValues.append(DSP::AxisData(capacity));
    m_activeWaterfalls.insert(i, true);
  }

  m_ingest.buildWaterfallPushes();
}
#endif

/**
 * @brief Rebuilds the plot and multiplot series after a point-count or time-range change while
 *        preserving what the rebuild would otherwise reset: the retained history, the sweep
 *        configuration with its captured segments, and each widget's run/pause flag. A point-count
 *        change used to drop all three (F4).
 */
void UI::Dashboard::rebuildLineSeriesPreservingState()
{
  auto savedPlotRings              = m_replaySeek.snapshotPlotTimeRings();
  auto savedMultiplotRings         = m_replaySeek.snapshotMultiplotTimeRings();
  const auto savedPlotSweep        = m_plotSweep;
  const auto savedMultiplotSweep   = m_multiplotSweep;
  const auto savedPlotRunning      = m_activePlots;
  const auto savedMultiplotRunning = m_activeMultiplots;

  configureLineSeries();
  configureMultiLineSeries();

  m_replaySeek.restorePlotTimeRings(savedPlotRings);
  m_replaySeek.restoreMultiplotTimeRings(savedMultiplotRings);
  m_plotControls.restorePlotSweepConfig(savedPlotSweep);
  m_plotControls.restoreMultiplotSweepConfig(savedMultiplotSweep);

  restoreRunFlags(savedPlotRunning, m_activePlots);
  restoreRunFlags(savedMultiplotRunning, m_activeMultiplots);
}

/**
 * @brief Re-resolves every pre-resolved push table against the current widget buckets. The tables
 *        hold Dataset pointers into those buckets, and a title edit writes into them: an
 *        implicitly-shared bucket detaches on that write and moves every dataset the tables
 *        address, so the tables must be resolved again afterwards (F10).
 */
void UI::Dashboard::rebuildPushTables()
{
  m_widgetMapBuilder.rebuildDatasetReferences();

  m_ingest.buildGpsPushes();
  m_ingest.buildFftPushes();
  m_ingest.buildLinePushes();
  m_ingest.buildMultiplotPushes();
#ifdef BUILD_COMMERCIAL
  m_ingest.buildPlot3DPushes();
  m_ingest.buildWaterfallPushes();
#endif
}

/**
 * @brief Registers an X-axis data buffer for a dataset's custom X source. The AxisData is
 *        left unfilled on purpose so the XY ring grows from empty; a seeded (0,0) point
 *        would draw a false first line segment.
 */
void UI::Dashboard::registerXAxisIfNeeded(const DataModel::Dataset& dataset)
{
  const int xSource = dataset.xAxisId;
  if (m_xAxisData.contains(xSource))
    return;

  if (!m_datasets.contains(xSource))
    return;

  DSP::AxisData xAxis(points() + 1);
  m_xAxisData.insert(xSource, xAxis);
}

/**
 * @brief Configures the line series data structures for all dashboard plots.
 */
void UI::Dashboard::configureLineSeries()
{
  SS_ASSERT(m_points > 0, {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  });

  m_xAxisData.clear();
  m_yAxisData.clear();
  m_plotTimeRings.clear();
  m_plotSweep.clear();
  m_pltValues.clear();
  m_pltValues.squeeze();
  m_activePlots.clear();

  m_pltXAxis = DSP::AxisData(points() + 1);
  m_pltXAxis.fillRange(0, 1);

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto& datasets = i.value();

    for (auto d = datasets.begin(); d != datasets.end(); ++d) {
      if (!d->plt)
        continue;

      if (useTimeXAxis(*d))
        continue;

      DSP::AxisData yAxis(points() + 1);
      m_yAxisData.insert(d->uniqueId, yAxis);

      registerXAxisIfNeeded(*d);
    }
  }

  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);

    if (useTimeXAxis(yDataset)) {
      const int cap = timeRingCapacity(m_plotTimeRange);
      m_plotTimeRings.insert(i,
                             makeHistoryRing(m_plotTimeRange, streamSampleRate(yDataset.sourceId)));

      DSP::SweepEngine sweep;
      sweep.configure(1, cap, m_plotTimeRange);
      m_plotSweep.insert(i, std::move(sweep));

      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_pltNullY;
      m_pltValues.append(series);
    }

    else if (m_datasets.contains(yDataset.xAxisId)) {
      DSP::LineSeries series;
      series.x = &m_xAxisData[yDataset.xAxisId];
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }

    else {
      DSP::LineSeries series;
      series.x = &m_pltXAxis;
      series.y = &m_yAxisData[yDataset.uniqueId];
      m_pltValues.append(series);
    }

    m_activePlots.insert(i, true);
  }

  m_ingest.buildLinePushes();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Initializes internal data structures for 3D trajectory plot widgets.
 *        Two passes are deliberate: the push table stores raw pointers into m_plot3DRings,
 *        and those addresses are stable only after that vector stops growing, so all rings
 *        are allocated first and the pushes resolved in a second pass.
 */
void UI::Dashboard::configurePlot3DSeries()
{
  SS_ASSERT(m_points > 0, {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  });

  const int plot3DCount = widgetCount(SerialStudio::DashboardPlot3D);

  m_plotData3D.clear();
  m_plotData3D.squeeze();
  m_plotData3D.resize(plot3DCount);

  m_plot3DRings.clear();
  m_plot3DRings.squeeze();

  m_plot3DRings.reserve(plot3DCount);
  for (int i = 0; i < plot3DCount; ++i) {
    m_plot3DRings.append(DSP::FixedQueue<QVector3D>(static_cast<std::size_t>(points())));
    m_plotData3D[i].reserve(static_cast<std::size_t>(points()));
  }

  m_ingest.buildPlot3DPushes();
}
#endif

/**
 * @brief Configures the multi-line series data structure for the dashboard.
 */
void UI::Dashboard::configureMultiLineSeries()
{
  SS_ASSERT(m_points > 0, {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  });

  m_multipltValues.clear();
  m_multipltValues.squeeze();
  m_activeMultiplots.clear();
  m_multiplotTimeRings.clear();
  m_multiplotSweep.clear();

  m_multipltXAxis = DSP::AxisData(points() + 1);
  m_multipltXAxis.fillRange(0, 1);

  for (int i = 0; i < widgetCount(SerialStudio::DashboardMultiPlot); ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    DSP::MultiLineSeries series;
    series.x = &m_multipltXAxis;
    for (size_t j = 0; j < group.datasets.size(); ++j)
      series.y.push_back(DSP::AxisData(points() + 1));

    m_multipltValues.append(series);
    m_activeMultiplots.insert(i, true);

    if (useTimeXAxisGroup(group)) {
      const int cap     = timeRingCapacity(m_plotTimeRange);
      const double rate = streamSampleRate(group.sourceId);
      std::vector<DSP::EnvelopeRing> rings;
      rings.reserve(group.datasets.size());
      for (size_t j = 0; j < group.datasets.size(); ++j)
        rings.push_back(makeHistoryRing(m_plotTimeRange, rate));

      m_multiplotTimeRings.insert(i, std::move(rings));

      DSP::SweepEngine sweep;
      sweep.configure(static_cast<int>(group.datasets.size()), cap, m_plotTimeRange);
      m_multiplotSweep.insert(i, std::move(sweep));
    }
  }

  m_ingest.buildMultiplotPushes();
}
