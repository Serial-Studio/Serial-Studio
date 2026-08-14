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
#include "CSV/Player.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/PipelineHost.h"
#include "MDF4/Player.h"
#include "Misc/IconEngine.h"
#include "Misc/TimerEvents.h"
#include "SessionContext.h"
#include "SSAssert.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"
#include "UI/Widgets/FFTWindow.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#  include "Licensing/LemonSqueezy.h"
#  include "Sessions/Player.h"
#  include "UI/Widgets/AudioExport.h"
#endif

#include <algorithm>
#include <cmath>
#include <limits>
#include <QSet>
#include <QTimer>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kDefaultPlotPoints      = 1000;
constexpr int kDefaultPlotBuckets     = 1024;
constexpr int kMaxTimeRingSamples     = 262144;
constexpr double kAssumedMaxRateHz    = 1024000.0;
constexpr double kTimeRingHeadroom    = 1.25;
constexpr double kSmoothMaxPeriodSec  = 0.002;
constexpr double kSmoothMaxForwardSec = 0.050;

// Ring-drain budget per display tick: kDrainBudgetNs / fps == 40% of the tick period
constexpr qint64 kDrainBudgetNs = 400000000LL;
constexpr int kMaxDisplayFps    = 240;
constexpr int kBudgetCheckMask  = 7;

// Pre-resolved push fallbacks for GPS / 3D groups missing an axis dataset
static constexpr double kNoGpsFix   = std::numeric_limits<double>::quiet_NaN();
static constexpr bool kNeverNumeric = false;
#ifdef BUILD_COMMERCIAL
static constexpr double kZeroAxisSource = 0.0;
#endif

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
 * @brief Builds a scrolling-history ring for the visible window plus headroom, so a
 *        saturated min/max source (two slots per decimation cell) still spans the full
 *        axis instead of erasing at the left edge; the surplus samples sit off-screen.
 */
static DSP::TimeRing makeHistoryRing(const double plotTimeRangeSec)
{
  const double window = plotTimeRangeSec * kTimeRingHeadroom;
  return DSP::TimeRing(timeRingCapacity(window), window);
}

/**
 * @brief Composite snapshot key: dataset uniqueIds repeat across sources, so the time-ring
 *        snapshot must include the sourceId or two plots collapse onto one entry and the second
 *        restore move-assigns a moved-from (null-buffer) ring, crashing appendDecimated.
 */
static qint64 ringSnapshotKey(const int sourceId, const int uniqueId)
{
  return (static_cast<qint64>(sourceId) << 32) | static_cast<quint32>(uniqueId);
}

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decrements a RepeatNTimes counter and stops the timer when it hits zero.
 */
static void tickRepeatTimer(int index, QMap<int, QTimer*>& timers, QMap<int, int>& counters)
{
  const auto it = counters.find(index);
  if (it == counters.end())
    return;

  if (--it.value() > 0)
    return;

  const auto timerIt = timers.find(index);
  if (timerIt != timers.end() && timerIt.value())
    timerIt.value()->stop();

  counters.erase(it);
}

/**
 * @brief Applies a non-RepeatNTimes timer mode to an action's QTimer.
 */
static void applyTimerMode(QTimer* timer,
                           DataModel::TimerMode mode,
                           bool guiTrigger,
                           const QString& actionTitle)
{
  if (!timer) {
    qWarning() << "Invalid timer pointer for action" << actionTitle;
    return;
  }

  if (mode == DataModel::TimerMode::StartOnTrigger && !timer->isActive())
    timer->start();

  else if (mode == DataModel::TimerMode::ToggleOnTrigger && guiTrigger) {
    if (timer->isActive())
      timer->stop();
    else
      timer->start();
  }
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Dashboard, wires reset signals and loads persisted settings.
 */
UI::Dashboard::Dashboard()
  : m_drainBudgetNs(kDrainBudgetNs / kMaxDisplayFps)
  , m_points(kDefaultPlotPoints)
  , m_widgetCount(0)
  , m_updateRequired(false)
  , m_thinningActive(false)
  , m_showActionPanel(true)
  , m_terminalEnabled(false)
  , m_notificationLogEnabled(false)
  , m_clockEnabled(false)
  , m_stopwatchEnabled(false)
  , m_autoHideToolbar(false)
  , m_showAlignmentGuides(false)
  , m_persistSettings(true)
  , m_autoLayoutMargin(0)
  , m_autoLayoutSpacing(-1)
  , m_manualLayoutSpacing(-1)
  , m_updateRetryInProgress(false)
  , m_layoutValid(false)
  , m_streamAvailable(false)
  , m_plotTimeRange(10.0)
  , m_plotDisplayTimeSec(0)
  , m_pltXAxis(kDefaultPlotPoints)
  , m_multipltXAxis(kDefaultPlotPoints)
{
  static auto& csvPlayer    = CSV::Player::instance();
  static auto& mdf4Player   = MDF4::Player::instance();
  static auto& ioManager    = IO::ConnectionManager::instance();
  static auto& appState     = AppState::instance();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();

  // clang-format off
  connect(&csvPlayer, &CSV::Player::openChanged, this, [=, this] { resetData(true); }, Qt::QueuedConnection);
  connect(&mdf4Player, &MDF4::Player::openChanged, this, [=, this] { resetData(true); }, Qt::QueuedConnection);
  connect(&ioManager, &IO::ConnectionManager::connectedChanged, this, [=, this] {
    if (!ioManager.isConnected() && !streamAvailable())
      resetData(true);
  }, Qt::QueuedConnection);
  connect(&appState, &AppState::projectFileChanged, this, [=, this] { resetData(); }, Qt::QueuedConnection);
  connect(&frameBuilder, &DataModel::FrameBuilder::jsonFileMapChanged, this, [this] {
    m_sourceRawFrames.clear();
    m_datasetReferences.clear();
    m_valuePushes.clear();
  }, Qt::QueuedConnection);
  connect(&appState, &AppState::operationModeChanged, this, [=, this] {
    const auto mode = appState.operationMode();
    if (mode == SerialStudio::ProjectFile) {
      const int project_pts = projectModel.pointCount();
      if (project_pts > 0 && m_points != project_pts) {
        m_points = project_pts;
        Q_EMIT pointsChanged();
      }

      const double project_range = projectModel.plotTimeRange();
      if (project_range > 0 && !qFuzzyCompare(m_plotTimeRange, project_range)) {
        m_plotTimeRange = project_range;
        Q_EMIT plotTimeRangeChanged();
      }
    } else {
      if (m_points != kDefaultPlotPoints) {
        m_points = kDefaultPlotPoints;
        Q_EMIT pointsChanged();
      }

      const double saved
        = qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));
      if (!qFuzzyCompare(m_plotTimeRange, saved)) {
        m_plotTimeRange = saved;
        Q_EMIT plotTimeRangeChanged();
      }
    }

    if (mode != SerialStudio::ProjectFile)
      resetData(true);

    Q_EMIT frozenChanged();
  }, Qt::QueuedConnection);
  // clang-format on

#ifdef BUILD_COMMERCIAL
  static auto& sessPlayer = Sessions::Player::instance();
  connect(
    &sessPlayer,
    &Sessions::Player::openChanged,
    this,
    [=, this] { resetData(true); },
    Qt::QueuedConnection);
#endif

  connectStreamAvailableInputs();

  static auto& timerEvents = Misc::TimerEvents::instance();
  connect(&timerEvents, &Misc::TimerEvents::uiTimeout, this, &UI::Dashboard::onDisplayTick);

  const auto refreshDrainBudget = [this] {
    m_drainBudgetNs = kDrainBudgetNs / qBound(1, timerEvents.fps(), kMaxDisplayFps);
  };
  connect(&timerEvents, &Misc::TimerEvents::fpsChanged, this, refreshDrainBudget);
  refreshDrainBudget();

  connect(&timerEvents, &Misc::TimerEvents::timeout1Hz, this, &UI::Dashboard::pollThinningState);

  connect(this, &UI::Dashboard::widgetCountChanged, this, &UI::Dashboard::actionStatusChanged);

  connect(
    &projectModel, &DataModel::ProjectModel::frozenChanged, this, &UI::Dashboard::frozenChanged);
  connect(&projectModel,
          &DataModel::ProjectModel::widgetDisplayChanged,
          this,
          &UI::Dashboard::refreshDisplayTitles);
#ifdef BUILD_COMMERCIAL
  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  connect(
    &lemonSqueezy, &Licensing::LemonSqueezy::activatedChanged, this, &UI::Dashboard::frozenChanged);
#endif

  updateStreamAvailable();
  restorePersistedSettings();
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

  drainDashboardRing(clock, m_drainBudgetNs);
  drainStreamWorkers(clock, m_drainBudgetNs);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.drainTableSnapshot();
  frameBuilder.drainLatestFrameSnapshot();

  if (m_updateRequired) {
    m_updateRequired = false;
    Q_EMIT updated();
  }
}

/**
 * @brief Drains the dashboard ring under @p budgetNs of @p clock, hard-bounded by the ring
 *        capacity: the producer is a live thread, so draining "until empty" livelocks the GUI.
 *        Frames past the budget are discarded except the newest, so the display stays current;
 *        exports are untouched, they fan out on the pipeline thread.
 */
void UI::Dashboard::drainDashboardRing(const QElapsedTimer& clock, const qint64 budgetNs)
{
  static auto& pipeline = IO::PipelineHost::instance();

  const int max_drain = pipeline.dashboardRingCapacity();
  SS_ASSERT(max_drain > 0, return);
  SS_ASSERT(budgetNs > 0, return);

  quint64 discarded = 0;
  bool over_budget  = false;
  DataModel::TimestampedFramePtr frame;
  DataModel::TimestampedFramePtr newest;

  for (int drained = 0; drained < max_drain && pipeline.dequeueDashboardFrame(frame); ++drained) {
    if (over_budget) [[unlikely]] {
      newest = frame;
      ++discarded;
      continue;
    }

    hotpathRxFrame(frame);
    if ((drained & kBudgetCheckMask) == kBudgetCheckMask)
      over_budget = clock.nsecsElapsed() >= budgetNs;
  }

  if (newest) [[unlikely]] {
    hotpathRxFrame(newest);
    --discarded;
    newest.reset();
  }

  frame.reset();
  pipeline.noteDisplayDrops(discarded);
}

/**
 * @brief Drains every stream worker's display ring and applies the updates, publishing the
 *        current display budget (points/window) to the workers' resize atomics on the way
 *        (GUI-written, worker-read, eventually consistent by design; spec 0051 T25).
 */
void UI::Dashboard::drainStreamWorkers(const QElapsedTimer& clock, const qint64 budgetNs)
{
  static auto& ioManager = IO::ConnectionManager::instance();
  const auto& workers    = ioManager.streamWorkers();
  if (workers.empty()) [[likely]]
    return;

  for (const auto& worker : workers) {
    if (!worker)
      continue;

    worker->setPixelWidth(qMax(1, m_points / 2));
    worker->setWindowSec(m_plotTimeRange);
    drainStreamWorker(*worker, clock, budgetNs);
  }
}

/**
 * @brief Drains one worker's display ring, hard-bounded by the ring capacity and stopped by the
 *        shared tick budget. Leftovers stay queued for the next tick; a ring that then fills is
 *        dropped and counted worker-side, which is the reported signal.
 */
void UI::Dashboard::drainStreamWorker(IO::StreamWorker& worker,
                                      const QElapsedTimer& clock,
                                      const qint64 budgetNs)
{
  const int max_drain = worker.displayRingCapacity();
  SS_ASSERT(max_drain > 0, return);
  SS_ASSERT(budgetNs > 0, return);

  IO::StreamDisplayUpdatePtr update;

  for (int drained = 0; drained < max_drain && worker.dequeueDisplayUpdate(update); ++drained) {
    if (update)
      applyStreamUpdate(*update);

    if ((drained & kBudgetCheckMask) == kBudgetCheckMask && clock.nsecsElapsed() >= budgetNs)
      break;
  }

  update.reset();
}

/**
 * @brief Ingests one bounded stream display update: latest values into the widget dataset
 *        copies, envelope pairs into the plot/multiplot time rings (per-source clock advanced
 *        from block t0, never cleared), FFT window into the FFT series. All work is O(pixels +
 *        fftSize + datasets), independent of the stream's sample rate (spec 0051 R7/R11).
 */
void UI::Dashboard::applyStreamUpdate(const IO::StreamDisplayUpdate& update)
{
  if (!m_layoutValid || !m_streamAvailable) [[unlikely]]
    return;

  const double baseSec = advancePlotClock(update.sourceId, update.t0);

  for (const auto& channel : update.channels)
    applyStreamChannel(channel, baseSec);

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
    if (it.value().enabled && m_activeMultiplots.value(it.key(), false))
      feedMultiplotStreamSweep(it.key(), update, baseSec);

  m_updateRequired = true;
}

/**
 * @brief Applies one channel's display payload: latest value into every widget dataset copy,
 *        envelope pairs into the plot/multiplot rings, FFT window into the FFT and waterfall
 *        series (both hold time-domain samples and transform themselves).
 */
void UI::Dashboard::applyStreamChannel(const IO::StreamDisplayUpdate::ChannelUpdate& channel,
                                       double baseSec)
{
  const auto refs = m_datasetReferences.constFind(channel.uniqueId);
  if (refs != m_datasetReferences.cend()) {
    const QString text = QString::number(channel.latest, 'g', 10);
    for (auto* dataset : refs.value()) {
      dataset->isNumeric    = true;
      dataset->numericValue = channel.latest;
      dataset->value        = text;
    }
  }

  const auto ext_it = m_datasetExtremes.find(channel.uniqueId);
  if (ext_it != m_datasetExtremes.end()) [[unlikely]] {
    auto& slot = ext_it.value();
    if (std::isfinite(channel.latest)) {
      slot.min   = slot.valid ? qMin(slot.min, channel.latest) : channel.latest;
      slot.max   = slot.valid ? qMax(slot.max, channel.latest) : channel.latest;
      slot.valid = true;
    }

    for (const auto& pair : channel.envelope) {
      if (!std::isfinite(pair.second))
        continue;

      slot.min   = slot.valid ? qMin(slot.min, pair.second) : pair.second;
      slot.max   = slot.valid ? qMax(slot.max, pair.second) : pair.second;
      slot.valid = true;
    }
  }

  const StreamTargets& targets = streamTargetsFor(channel.uniqueId);

  for (const int plotIndex : targets.plotIndexes) {
    if (!m_activePlots.value(plotIndex, false))
      continue;

    auto ringIt = m_plotTimeRings.find(plotIndex);
    if (ringIt == m_plotTimeRings.end()) [[unlikely]]
      continue;

    for (const auto& pair : channel.envelope)
      ringIt.value().appendDecimated(baseSec + pair.first, pair.second);

    feedPlotStreamSweep(plotIndex, channel, baseSec);
  }

  for (const auto& [groupIndex, curveIndex] : targets.multiplotCurves) {
    if (!m_activeMultiplots.value(groupIndex, false))
      continue;

    auto ringsIt = m_multiplotTimeRings.find(groupIndex);
    if (ringsIt == m_multiplotTimeRings.end()) [[unlikely]]
      continue;

    auto& rings = ringsIt.value();
    if (curveIndex < 0 || static_cast<std::size_t>(curveIndex) >= rings.size()) [[unlikely]]
      continue;

    for (const auto& pair : channel.envelope)
      rings[curveIndex].appendDecimated(baseSec + pair.first, pair.second);
  }

  if (!channel.hasFft)
    return;

  for (const int fftIndex : targets.fftIndexes) {
    if (!m_activeFFTPlots.value(fftIndex, false))
      continue;

    if (fftIndex < 0 || fftIndex >= m_fftValues.size()) [[unlikely]]
      continue;

    auto& series = m_fftValues[fftIndex];
    series.clear();
    for (const double sample : channel.fftWindow)
      series.push(sample);
  }

#ifdef BUILD_COMMERCIAL
  for (const int fallIndex : targets.waterfallIndexes) {
    if (!m_activeWaterfalls.value(fallIndex, false))
      continue;

    if (fallIndex < 0 || fallIndex >= m_waterfallValues.size()) [[unlikely]]
      continue;

    auto& series = m_waterfallValues[fallIndex];
    series.clear();
    for (const double sample : channel.fftWindow)
      series.push(sample);
  }
#endif
}

/**
 * @brief Feeds the stream lane's envelope into one plot's sweep engine, the counterpart of the
 *        frame lane's feedSweep (spec 0051 M4: stream sources never reach that path). Trigger
 *        detection runs at the envelope's bucket resolution, which is the resolution the trace
 *        is drawn at, so precision degrades in step with the display and never below it.
 */
void UI::Dashboard::feedPlotStreamSweep(int plotIndex,
                                        const IO::StreamDisplayUpdate::ChannelUpdate& channel,
                                        double baseSec)
{
  auto sweepIt = m_plotSweep.find(plotIndex);
  if (sweepIt == m_plotSweep.end())
    return;

  DSP::SweepEngine& sweep = sweepIt.value();
  if (!sweep.enabled || sweep.back.empty())
    return;

  for (const auto& pair : channel.envelope) {
    const double st = sweep.advance(baseSec + pair.first, pair.second);
    if (st >= 0)
      sweep.back[0].appendDecimated(st, pair.second);
  }
}

/**
 * @brief Feeds one multiplot's sweep engine from a stream update: the trigger curve alone drives
 *        advance(), every curve is appended at the time it returns, and curves pair by envelope
 *        index because one source's channels share a bucket grid. A group this update feeds no
 *        curve of resolves to no trigger, which keeps frame-fed multiplots out of this path.
 */
void UI::Dashboard::feedMultiplotStreamSweep(int groupIndex,
                                             const IO::StreamDisplayUpdate& update,
                                             double baseSec)
{
  auto sweepIt = m_multiplotSweep.find(groupIndex);
  if (sweepIt == m_multiplotSweep.end())
    return;

  DSP::SweepEngine& sweep = sweepIt.value();
  if (!sweep.enabled || sweep.back.empty())
    return;

  m_streamSweepCurves.assign(sweep.back.size(), nullptr);
  for (const auto& channel : update.channels) {
    for (const auto& [group, curve] : streamTargetsFor(channel.uniqueId).multiplotCurves) {
      if (group != groupIndex || curve < 0)
        continue;

      if (static_cast<std::size_t>(curve) < m_streamSweepCurves.size())
        m_streamSweepCurves[static_cast<std::size_t>(curve)] = &channel;
    }
  }

  const int last      = static_cast<int>(m_streamSweepCurves.size()) - 1;
  const int trigCurve = qBound(0, sweep.triggerCurve, last);
  const auto* trigger = m_streamSweepCurves[static_cast<std::size_t>(trigCurve)];
  if (!trigger)
    return;

  std::size_t count = trigger->envelope.size();
  for (const auto* curve : m_streamSweepCurves)
    if (curve)
      count = std::min(count, curve->envelope.size());

  for (std::size_t i = 0; i < count; ++i) {
    const double st =
      sweep.advance(baseSec + trigger->envelope[i].first, trigger->envelope[i].second);
    if (st < 0)
      continue;

    for (std::size_t j = 0; j < m_streamSweepCurves.size(); ++j)
      if (m_streamSweepCurves[j])
        sweep.back[j].appendDecimated(st, m_streamSweepCurves[j]->envelope[i].second);
  }
}

/**
 * @brief Lazily resolves (and caches) the widget indexes fed by one stream dataset. The cache
 *        holds indexes only -- ring pointers would dangle across a layout rebuild -- and is
 *        cleared with the push tables on every reconfigure.
 */
const UI::Dashboard::StreamTargets& UI::Dashboard::streamTargetsFor(int uniqueId)
{
  auto it = m_streamTargets.find(uniqueId);
  if (it != m_streamTargets.end()) [[likely]]
    return it.value();

  StreamTargets targets;

  const auto plots = m_widgetDatasets.constFind(SerialStudio::DashboardPlot);
  if (plots != m_widgetDatasets.cend())
    for (int i = 0; i < plots.value().size(); ++i)
      if (plots.value()[i].uniqueId == uniqueId)
        targets.plotIndexes.push_back(i);

  const auto ffts = m_widgetDatasets.constFind(SerialStudio::DashboardFFT);
  if (ffts != m_widgetDatasets.cend())
    for (int i = 0; i < ffts.value().size(); ++i)
      if (ffts.value()[i].uniqueId == uniqueId)
        targets.fftIndexes.push_back(i);

#ifdef BUILD_COMMERCIAL
  const auto falls = m_widgetDatasets.constFind(SerialStudio::DashboardWaterfall);
  if (falls != m_widgetDatasets.cend())
    for (int i = 0; i < falls.value().size(); ++i)
      if (falls.value()[i].uniqueId == uniqueId)
        targets.waterfallIndexes.push_back(i);
#endif

  const auto groups    = m_widgetGroups.constFind(SerialStudio::DashboardMultiPlot);
  const int groupCount = (groups != m_widgetGroups.cend()) ? groups.value().size() : 0;
  for (int g = 0; g < groupCount; ++g) {
    const auto& datasets = groups.value()[g].datasets;
    for (std::size_t c = 0; c < datasets.size(); ++c)
      if (datasets[c].uniqueId == uniqueId)
        targets.multiplotCurves.emplace_back(g, static_cast<int>(c));
  }

  return m_streamTargets.insert(uniqueId, std::move(targets)).value();
}

/**
 * @brief Restores the persisted view-state flags and layout preferences from QSettings.
 */
void UI::Dashboard::restorePersistedSettings()
{
  m_autoHideToolbar        = m_settings.value("Dashboard/AutoHideToolbar", false).toBool();
  m_showActionPanel        = m_settings.value("Dashboard/ShowActionPanel", true).toBool();
  m_showAlignmentGuides    = m_settings.value("Dashboard/ShowAlignmentGuides", false).toBool();
  m_terminalEnabled        = m_settings.value("Dashboard/TerminalEnabled", false).toBool();
  m_notificationLogEnabled = m_settings.value("Dashboard/NotificationLogEnabled", false).toBool();
  m_clockEnabled           = m_settings.value("Dashboard/ClockEnabled", false).toBool();
  m_stopwatchEnabled       = m_settings.value("Dashboard/StopwatchEnabled", false).toBool();
  m_plotTimeRange =
    qMax(0.001, SerialStudio::toDouble(m_settings.value("Dashboard/PlotTimeRange", 10.0)));

  m_autoLayoutMargin    = qMax(0, m_settings.value("Dashboard/AutoLayoutMargin", 0).toInt());
  m_autoLayoutSpacing   = qMax(-1, m_settings.value("Dashboard/AutoLayoutSpacing", -1).toInt());
  m_manualLayoutSpacing = qMax(-1, m_settings.value("Dashboard/ManualLayoutSpacing", -1).toInt());
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
 * @brief Returns true if a rectangle with a list of actions should be displayed alongside the
 * dashboard.
 */
bool UI::Dashboard::showActionPanel() const noexcept
{
  return m_showActionPanel;
}

/**
 * @brief Returns true if the toolbar should automatically hide when the dashboard is visible.
 */
bool UI::Dashboard::autoHideToolbar() const noexcept
{
  return m_autoHideToolbar;
}

/**
 * @brief Returns true if smart alignment guides are shown during manual-mode gestures.
 */
bool UI::Dashboard::showAlignmentGuides() const noexcept
{
  return m_showAlignmentGuides;
}

/**
 * @brief Returns the effective dashboard freeze state: the project's stored flag gated on an
 *        active Pro/Trial license and ProjectFile mode; QML-binding-time only, never read on
 *        the frame path. QuickPlot/ConsoleOnly is always live so a frozen project cannot lock
 *        the clean-slate dashboard.
 */
bool UI::Dashboard::frozen() const
{
  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();
  return projectModel.frozen() && SerialStudio::activated()
      && appState.operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Returns whether the parse governor is thinning any source, polled at 1 Hz for the
 *        taskbar badge; never read or refreshed on the frame path.
 */
bool UI::Dashboard::thinningActive() const noexcept
{
  return m_thinningActive;
}

/**
 * @brief 1 Hz poll of the parse governor's thinning latch; emits only on transitions so QML
 *        bindings stay quiet while the state is stable.
 */
void UI::Dashboard::pollThinningState()
{
  static auto& builder    = DataModel::FrameBuilder::instance();
  const bool now_thinning = builder.parseBudgetThinning();
  if (now_thinning != m_thinningActive) {
    m_thinningActive = now_thinning;
    Q_EMIT thinningActiveChanged();
  }
}

/**
 * @brief Returns the visible plot time window in seconds (newest sample at 0).
 */
double UI::Dashboard::plotTimeRange() const noexcept
{
  return m_plotTimeRange;
}

/**
 * @brief Returns the auto-layout margin (px) reserved between tiled windows and the canvas edges.
 */
int UI::Dashboard::autoLayoutMargin() const noexcept
{
  return m_autoLayoutMargin;
}

/**
 * @brief Returns the auto-layout spacing (px) between adjacent tiled windows (-1 = flush borders).
 */
int UI::Dashboard::autoLayoutSpacing() const noexcept
{
  return m_autoLayoutSpacing;
}

/**
 * @brief Returns the border spacing manual layouts snap and weld to (-1 = shared border).
 */
int UI::Dashboard::manualLayoutSpacing() const noexcept
{
  return m_manualLayoutSpacing;
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
 *        as a stream: its snapshots enter through hotpathRxFrame like any local frame, and the
 *        flag read there is what decides whether they are drawn. The query is a plain flag read
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
 * @brief Refreshes the cached stream flag read by hotpathRxFrame; wired to every input that
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
 * @brief Returns true if a terminal widget should be displayed within the dashboard.
 */
bool UI::Dashboard::terminalEnabled() const noexcept
{
  return m_terminalEnabled;
}

/**
 * @brief Returns true if the notification log widget should be displayed within the dashboard.
 */
bool UI::Dashboard::notificationLogEnabled() const noexcept
{
  return m_notificationLogEnabled;
}

/**
 * @brief Returns true if the clock widget should be displayed within the dashboard.
 */
bool UI::Dashboard::clockEnabled() const noexcept
{
  return m_clockEnabled;
}

/**
 * @brief Returns true if the stopwatch widget should be displayed within the dashboard.
 */
bool UI::Dashboard::stopwatchEnabled() const noexcept
{
  return m_stopwatchEnabled;
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
// UI configuration queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current point/sample count setting for the dashboard plots.
 */
int UI::Dashboard::points() const noexcept
{
  return m_points;
}

/**
 * @brief Retrieves the count of actions available within the dashboard.
 */
int UI::Dashboard::actionCount() const
{
  return m_actions.count();
}

/**
 * @brief Gets the total count of widgets currently available on the dashboard.
 */
int UI::Dashboard::totalWidgetCount() const noexcept
{
  return m_widgetCount;
}

//--------------------------------------------------------------------------------------------------
// Data & widget queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks if the current frame is valid for processing.
 */
bool UI::Dashboard::frameValid() const
{
  return m_lastFrame.groups.size() > 0;
}

/**
 * @brief Retrieves the relative index of a widget within its type group.
 */
int UI::Dashboard::relativeIndex(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->second : -1;
}

/**
 * @brief Formats a numerical value according to its context range.
 */
QString UI::Dashboard::formatValue(double val, double min, double max) const
{
  return FMT_VAL(val, min, max);
}

/**
 * @brief Retrieves the type of widget associated with a given widget index.
 */
SerialStudio::DashboardWidget UI::Dashboard::widgetType(const int widgetIndex) const
{
  const auto it = m_widgetMap.constFind(widgetIndex);
  return it != m_widgetMap.cend() ? it->first : SerialStudio::DashboardNoWidget;
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
 * @brief Returns the package id owning one entry of the extension bucket, empty when out of range.
 */
QString UI::Dashboard::extensionIdAt(const bool group, const int bucketIndex) const
{
  const auto& ids = group ? m_extensionGroupIds : m_extensionDatasetIds;
  if (bucketIndex < 0 || bucketIndex >= ids.count())
    return {};

  return ids.at(bucketIndex);
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
 * @brief Retrieves the title of the current frame in the dashboard.
 */
const QString& UI::Dashboard::title() const
{
  return m_lastFrame.title;
}

/**
 * @brief Returns a list of available dashboard actions with their metadata.
 */
QVariantList UI::Dashboard::actions() const
{
  QVariantList actions;
  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];

    QVariantMap m;
    m["id"]      = i;
    m["checked"] = false;
    m["text"]    = action.title;
    m["icon"]    = Misc::IconEngine::resolveActionIconSource(action.icon);
    if (action.timerMode == DataModel::TimerMode::ToggleOnTrigger) {
      if (m_timers.contains(i) && m_timers[i] && m_timers[i]->isActive())
        m["checked"] = true;
    }

    actions.append(m);
  }

  return actions;
}

/**
 * @brief Returns the runtime index of the action with the given public @p actionId, or -1.
 */
int UI::Dashboard::actionIndexForId(int actionId) const noexcept
{
  for (int i = 0; i < m_actions.count(); ++i)
    if (m_actions.at(i).actionId == actionId)
      return i;

  return -1;
}

/**
 * @brief Retrieves a map of all widgets/windows in the dashboard.
 */
const SerialStudio::WidgetMap& UI::Dashboard::widgetMap() const
{
  return m_widgetMap;
}

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

/**
 * @brief Returns the min/max hold state for one extreme-hold dataset; invalid when the dataset
 *        never opted in or no finite sample arrived since the last data reset.
 */
UI::Dashboard::DatasetExtremes UI::Dashboard::datasetExtremes(int uniqueId) const
{
  return m_datasetExtremes.value(uniqueId);
}

//--------------------------------------------------------------------------------------------------
// Dataset & group access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Provides access to the map of dataset objects.
 */
const QMap<int, DataModel::Dataset>& UI::Dashboard::datasets() const
{
  return m_datasets;
}

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
// Frame access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the last unmodified DataModel frame for the dashboard.
 */
const DataModel::Frame& UI::Dashboard::rawFrame()
{
  return m_lastFrame;
}

/**
 * @brief Retrieves the processed DataModel frame for the dashboard.
 */
const DataModel::Frame& UI::Dashboard::processedFrame()
{
  return m_lastFrame;
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
const DSP::TimeRing& UI::Dashboard::plotTimeRing(const int index) const
{
  const auto it = m_plotTimeRings.find(index);
  if (it == m_plotTimeRings.end()) [[unlikely]] {
    static const DSP::TimeRing kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the per-curve decimating time rings for a time-axis multiplot widget.
 */
const std::vector<DSP::TimeRing>& UI::Dashboard::multiplotTimeRings(const int index) const
{
  const auto it = m_multiplotTimeRings.find(index);
  if (it == m_multiplotTimeRings.end()) [[unlikely]] {
    static const std::vector<DSP::TimeRing> kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the sweep/trigger engine for a time-axis plot widget.
 */
const DSP::SweepEngine& UI::Dashboard::plotSweep(const int index) const
{
  const auto it = m_plotSweep.find(index);
  if (it == m_plotSweep.end()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the sweep/trigger engine for a time-axis multiplot widget.
 */
const DSP::SweepEngine& UI::Dashboard::multiplotSweep(const int index) const
{
  const auto it = m_multiplotSweep.find(index);
  if (it == m_multiplotSweep.end()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
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
// Plot active status getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether a plot is currently active.
 */
bool UI::Dashboard::plotRunning(const int index)
{
  if (m_activePlots.contains(index))
    return m_activePlots[index];

  return false;
}

/**
 * @brief Checks whether an FFT plot is currently active.
 */
bool UI::Dashboard::fftPlotRunning(const int index)
{
  if (m_activeFFTPlots.contains(index))
    return m_activeFFTPlots[index];

  return false;
}

/**
 * @brief Checks whether a multiplot is currently active.
 */
bool UI::Dashboard::multiplotRunning(const int index)
{
  if (m_activeMultiplots.contains(index))
    return m_activeMultiplots[index];

  return false;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Checks whether a waterfall plot is currently active.
 */
bool UI::Dashboard::waterfallRunning(const int index)
{
  if (m_activeWaterfalls.contains(index))
    return m_activeWaterfalls[index];

  return false;
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

    static auto& appState = AppState::instance();
    if (appState.operationMode() != SerialStudio::ProjectFile)
      m_settings.setValue("Dashboard/Points", m_points);

    auto savedPlotRings      = snapshotPlotTimeRings();
    auto savedMultiplotRings = snapshotMultiplotTimeRings();

    configureLineSeries();
    configureMultiLineSeries();

    restorePlotTimeRings(savedPlotRings);
    restoreMultiplotTimeRings(savedMultiplotRings);

    Q_EMIT pointsChanged();
  }
}

/**
 * @brief Drops every pre-resolved hotpath push table (their pointers follow the layout).
 */
void UI::Dashboard::clearPushTables()
{
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();
  m_fftPushes.clear();
  m_gpsPushes.clear();
  m_valuePushes.clear();
  m_extremePushes.clear();
  m_streamTargets.clear();
  m_multiplotPushes.clear();
  m_yLinePushes.shrink_to_fit();
  m_xLinePushes.shrink_to_fit();
  m_timePushes.shrink_to_fit();
  m_fftPushes.shrink_to_fit();
  m_gpsPushes.shrink_to_fit();
  m_multiplotPushes.shrink_to_fit();
#ifdef BUILD_COMMERCIAL
  m_waterfallPushes.clear();
  m_plot3DPushes.clear();
  m_waterfallPushes.shrink_to_fit();
  m_plot3DPushes.shrink_to_fit();
#endif
}

/**
 * @brief Resets all data in the dashboard, including plot values, widget structures, and actions.
 */
void UI::Dashboard::resetData(const bool notify)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile && m_points != kDefaultPlotPoints) {
    m_points = kDefaultPlotPoints;
    Q_EMIT pointsChanged();
  }

  static auto& widgetRegistry = WidgetRegistry::instance();
  widgetRegistry.clear();

  m_fftValues.clear();
  m_pltValues.clear();
  m_multipltValues.clear();

  m_fftValues.squeeze();
  m_pltValues.squeeze();
  m_multipltValues.squeeze();

  m_layoutValid = false;

  clearPushTables();

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
  m_plotClocks.clear();

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

  if (appState.operationMode() == SerialStudio::ProjectFile) {
    static auto& frameBuilder = DataModel::FrameBuilder::instance();
    DataModel::Frame templateFrame;
    frameBuilder.invokeOnBuilderThreadBlocking(
      [&templateFrame] { templateFrame = frameBuilder.frame(); });
    configureActions(templateFrame);
  }

  if (notify) {
    m_datasetExtremes.clear();

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

  for (auto it = m_plotSweep.begin(); it != m_plotSweep.end(); ++it)
    it.value().resetState();

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
    it.value().resetState();

  m_plotClocks.clear();

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
 * @brief Packs a (sourceId, uniqueId) pair into one hash key; uniqueIds alone are not unique
 *        across sources.
 */
qint64 UI::Dashboard::replaySeekKey(int sourceId, int uniqueId) noexcept
{
  return (static_cast<qint64>(sourceId) << 32) | static_cast<quint32>(uniqueId);
}

/**
 * @brief Lists every (sourceId, uniqueId) pair the plot widgets consume, including dataset-X
 *        axis sources and multiplot curves, so a replay player knows which columns to sample
 *        for bulkLoadPlotWindow().
 */
QList<std::pair<int, int>> UI::Dashboard::replaySeekSeries() const
{
  QList<std::pair<int, int>> out;
  QSet<qint64> seen;

  auto add = [&](int sourceId, int uniqueId) {
    const qint64 key = replaySeekKey(sourceId, uniqueId);
    if (seen.contains(key))
      return;

    seen.insert(key);
    out.append({sourceId, uniqueId});
  };

  const int plotCount = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < plotCount; ++i) {
    const auto& ds = getDatasetWidget(SerialStudio::DashboardPlot, i);
    add(ds.sourceId, ds.uniqueId);

    if (useTimeXAxis(ds))
      continue;

    const auto xIt = m_datasets.constFind(ds.xAxisId);
    if (xIt != m_datasets.constEnd())
      add(xIt.value().sourceId, ds.xAxisId);
  }

  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < multiCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    for (const auto& ds : group.datasets)
      add(group.sourceId, ds.uniqueId);
  }

  return out;
}

/**
 * @brief Fills one single-plot widget's ring from the seek window: decimating TimeRing for
 *        time plots, y/x sample rings otherwise. @p filled dedups shared sample rings.
 */
void UI::Dashboard::fillSeekPlotSingle(int index,
                                       const QVector<double>& timesSec,
                                       const QHash<qint64, QVector<double>>& series,
                                       double timeOffset,
                                       QSet<const DSP::AxisData*>& filled)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT(index < widgetCount(SerialStudio::DashboardPlot), return);

  const auto& ds     = getDatasetWidget(SerialStudio::DashboardPlot, index);
  const auto& values = series.value(replaySeekKey(ds.sourceId, ds.uniqueId));
  const int count    = qMin(timesSec.size(), values.size());

  if (useTimeXAxis(ds)) {
    const auto rIt = m_plotTimeRings.find(index);
    if (rIt == m_plotTimeRings.end()) [[unlikely]]
      return;

    rIt.value().clear();
    for (int k = 0; k < count; ++k)
      rIt.value().appendDecimated(timesSec[k] + timeOffset, values[k]);

    return;
  }

  const auto yIt = m_yAxisData.find(ds.uniqueId);
  if (yIt != m_yAxisData.end() && !filled.contains(&yIt.value())) {
    filled.insert(&yIt.value());
    yIt.value().clear();
    for (int k = 0; k < count; ++k)
      yIt.value().push(values[k]);
  }

  const auto xDsIt = m_datasets.constFind(ds.xAxisId);
  if (xDsIt == m_datasets.constEnd())
    return;

  const auto xIt = m_xAxisData.find(ds.xAxisId);
  if (xIt == m_xAxisData.end() || filled.contains(&xIt.value()))
    return;

  const auto& xValues = series.value(replaySeekKey(xDsIt.value().sourceId, ds.xAxisId));
  const int xCount    = qMin(timesSec.size(), xValues.size());
  filled.insert(&xIt.value());
  xIt.value().clear();
  for (int k = 0; k < xCount; ++k)
    xIt.value().push(xValues[k]);
}

/**
 * @brief Fills one multiplot widget's curves from the seek window: per-curve TimeRings for
 *        time-mode groups, per-curve y sample rings otherwise.
 */
void UI::Dashboard::fillSeekPlotMulti(int index,
                                      const QVector<double>& timesSec,
                                      const QHash<qint64, QVector<double>>& series,
                                      double timeOffset)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT(index < widgetCount(SerialStudio::DashboardMultiPlot), return);

  const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, index);

  const auto rIt = m_multiplotTimeRings.find(index);
  if (rIt != m_multiplotTimeRings.end()) {
    auto& rings        = rIt.value();
    const size_t count = std::min(group.datasets.size(), rings.size());
    for (size_t j = 0; j < count; ++j) {
      const auto& ds     = group.datasets[j];
      const auto& values = series.value(replaySeekKey(group.sourceId, ds.uniqueId));
      const int n        = qMin(timesSec.size(), values.size());
      rings[j].clear();
      for (int k = 0; k < n; ++k)
        rings[j].appendDecimated(timesSec[k] + timeOffset, values[k]);
    }

    return;
  }

  if (index >= m_multipltValues.size()) [[unlikely]]
    return;

  auto& multiSeries  = m_multipltValues[index];
  const size_t count = std::min(group.datasets.size(), multiSeries.y.size());
  for (size_t j = 0; j < count; ++j) {
    const auto& ds     = group.datasets[j];
    const auto& values = series.value(replaySeekKey(group.sourceId, ds.uniqueId));
    const int n        = qMin(timesSec.size(), values.size());
    multiSeries.y[j].clear();
    for (int k = 0; k < n; ++k)
      multiSeries.y[j].push(values[k]);
  }
}

/**
 * @brief Rebuilds every plot ring from a replay seek window (spec 0020): ascending recorded
 *        seconds + series keyed by replaySeekKey, normalized to end at 0 so the decimation
 *        grid and later live appends stay monotonic. Writes rings only and resets the plot
 *        clocks so play-after-scrub re-anchors; FFT/GPS/3D/waterfall settle at rest.
 */
void UI::Dashboard::bulkLoadPlotWindow(const QVector<double>& timesSec,
                                       const QHash<qint64, QVector<double>>& series)
{
  // code-verify off
  // Debug-only ordering check: is_sorted is O(n) over the whole seek window, so a release
  // evaluation would walk every sample on every scrub.
  Q_ASSERT(std::is_sorted(timesSec.cbegin(), timesSec.cend()));
  // code-verify on

  if (!m_layoutValid || timesSec.isEmpty()) [[unlikely]]
    return;

  const double timeOffset = -timesSec.last();

  QSet<const DSP::AxisData*> filled;
  const int plotCount = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < plotCount; ++i)
    fillSeekPlotSingle(i, timesSec, series, timeOffset, filled);

  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < multiCount; ++i)
    fillSeekPlotMulti(i, timesSec, series, timeOffset);

  for (auto it = m_plotSweep.begin(); it != m_plotSweep.end(); ++it)
    it.value().resetState();

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
    it.value().resetState();

  m_plotClocks.clear();
  m_plotDisplayTimeSec = 0.0;
  m_updateRequired     = true;
}

/**
 * @brief Enables/disables the action panel.
 */
void UI::Dashboard::setShowActionPanel(const bool enabled)
{
  if (m_showActionPanel != enabled) {
    m_showActionPanel = enabled;
    if (m_persistSettings)
      m_settings.setValue("Dashboard/ShowActionPanel", m_showActionPanel);

    Q_EMIT showActionPanelChanged();
  }
}

/**
 * @brief Enables or disables auto-hiding the toolbar when the dashboard is shown.
 */
void UI::Dashboard::setAutoHideToolbar(const bool enabled)
{
  if (m_autoHideToolbar != enabled) {
    m_autoHideToolbar = enabled;
    m_settings.setValue("Dashboard/AutoHideToolbar", m_autoHideToolbar);
    Q_EMIT autoHideToolbarChanged();
  }
}

/**
 * @brief Shows or hides the smart alignment guides drawn during manual-mode gestures.
 */
void UI::Dashboard::setShowAlignmentGuides(const bool enabled)
{
  if (m_showAlignmentGuides != enabled) {
    m_showAlignmentGuides = enabled;
    if (m_persistSettings)
      m_settings.setValue("Dashboard/ShowAlignmentGuides", m_showAlignmentGuides);

    Q_EMIT showAlignmentGuidesChanged();
  }
}

/**
 * @brief Sets the auto-layout edge margin (px); clamped to >= 0 and persisted.
 */
void UI::Dashboard::setAutoLayoutMargin(const int margin)
{
  const int clamped = qMax(0, margin);
  if (m_autoLayoutMargin == clamped)
    return;

  m_autoLayoutMargin = clamped;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/AutoLayoutMargin", m_autoLayoutMargin);

  Q_EMIT autoLayoutMarginChanged();
}

/**
 * @brief Sets the auto-layout inter-window spacing (px); clamped to >= -1 and persisted.
 */
void UI::Dashboard::setAutoLayoutSpacing(const int spacing)
{
  const int clamped = qMax(-1, spacing);
  if (m_autoLayoutSpacing == clamped)
    return;

  m_autoLayoutSpacing = clamped;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/AutoLayoutSpacing", m_autoLayoutSpacing);

  Q_EMIT autoLayoutSpacingChanged();
}

/**
 * @brief Sets the border spacing manual layouts snap and weld to; clamped to >= -1 (the
 *        default, which overlaps two borders into one shared line) and persisted.
 */
void UI::Dashboard::setManualLayoutSpacing(const int spacing)
{
  const int clamped = qMax(-1, spacing);
  if (m_manualLayoutSpacing == clamped)
    return;

  m_manualLayoutSpacing = clamped;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ManualLayoutSpacing", m_manualLayoutSpacing);

  Q_EMIT manualLayoutSpacingChanged();
}

/**
 * @brief Forwards the freeze toggle to the project model, the single owner of the stored
 *        flag; the effective state re-derives through the frozenChanged wiring.
 */
void UI::Dashboard::setFrozen(const bool frozen)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setFrozen(frozen);
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

  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    m_settings.setValue("Dashboard/PlotTimeRange", m_plotTimeRange);

  auto savedPlotRings            = snapshotPlotTimeRings();
  auto savedMultiplotRings       = snapshotMultiplotTimeRings();
  const auto savedPlotSweep      = m_plotSweep;
  const auto savedMultiplotSweep = m_multiplotSweep;

  configureLineSeries();
  configureMultiLineSeries();

  restorePlotTimeRings(savedPlotRings);
  restoreMultiplotTimeRings(savedMultiplotRings);
  restorePlotSweepConfig(savedPlotSweep);
  restoreMultiplotSweepConfig(savedMultiplotSweep);

  m_updateRequired = true;
  Q_EMIT plotTimeRangeChanged();
}

/**
 * @brief Toggles whether dashboard preference changes are written to QSettings.
 */
void UI::Dashboard::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

//--------------------------------------------------------------------------------------------------
// Dashboard tools (terminal, notification log, clock, stopwatch)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Shows or hides the terminal tool window. The widget itself is always registered;
 *        the flag only drives external-window visibility, so no rebuild occurs.
 */
void UI::Dashboard::setTerminalEnabled(const bool enabled)
{
  if (m_terminalEnabled == enabled)
    return;

  m_terminalEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/TerminalEnabled", m_terminalEnabled);

  Q_EMIT terminalEnabledChanged();
}

/**
 * @brief Shows or hides the notification log tool window (Pro-only widget).
 */
void UI::Dashboard::setNotificationLogEnabled(const bool enabled)
{
  if (m_notificationLogEnabled == enabled)
    return;

  m_notificationLogEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/NotificationLogEnabled", m_notificationLogEnabled);

  Q_EMIT notificationLogEnabledChanged();
}

/**
 * @brief Shows or hides the clock tool window.
 */
void UI::Dashboard::setClockEnabled(const bool enabled)
{
  if (m_clockEnabled == enabled)
    return;

  m_clockEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ClockEnabled", m_clockEnabled);

  Q_EMIT clockEnabledChanged();
}

/**
 * @brief Shows or hides the stopwatch tool window.
 */
void UI::Dashboard::setStopwatchEnabled(const bool enabled)
{
  if (m_stopwatchEnabled == enabled)
    return;

  m_stopwatchEnabled = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/StopwatchEnabled", m_stopwatchEnabled);

  Q_EMIT stopwatchEnabledChanged();
}

//--------------------------------------------------------------------------------------------------
// Action handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Activates a dashboard action by transmitting its associated data and handling timer
 *        logic. actionStatusChanged makes QML rebuild the whole actions list, so it only fires
 *        when a timer's activity flips; per-tick transmissions emit nothing.
 */
void UI::Dashboard::activateAction(const int index, const bool guiTrigger)
{
  if (index < 0 || index >= m_actions.count()) {
    qWarning() << "Invalid action index:" << index;
    return;
  }

  const auto& action = m_actions[index];

  static auto& ioManager = IO::ConnectionManager::instance();

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && guiTrigger) {
    if (m_timers.contains(index) && m_timers[index]) {
      m_repeatCounters[index] = qMax(1, action.repeatCount);
      m_timers[index]->start();
    }

    if (!ioManager.paused())
      (void)ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);
    return;
  }

  if (action.timerMode == DataModel::TimerMode::RepeatNTimes && !guiTrigger) {
    if (!ioManager.paused())
      (void)ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

    tickRepeatTimer(index, m_timers, m_repeatCounters);
    return;
  }

  bool timerFlipped  = false;
  const auto timerIt = m_timers.find(index);
  if (timerIt != m_timers.end()) {
    const bool wasActive = timerIt.value() && timerIt.value()->isActive();
    applyTimerMode(timerIt.value(), action.timerMode, guiTrigger, action.title);
    const bool isActive = timerIt.value() && timerIt.value()->isActive();
    timerFlipped        = (wasActive != isActive);
  }

  if (!ioManager.paused())
    (void)ioManager.writeDataToDevice(action.sourceId, DataModel::get_tx_bytes(action));

  if (timerFlipped)
    Q_EMIT actionStatusChanged();
}

//--------------------------------------------------------------------------------------------------
// Plot active state setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active state of a plot.
 */
void UI::Dashboard::setPlotRunning(const int index, const bool enabled)
{
  if (m_activePlots.contains(index))
    m_activePlots[index] = enabled;
}

/**
 * @brief Sets the active state of an FFT plot.
 */
void UI::Dashboard::setFFTPlotRunning(const int index, const bool enabled)
{
  if (m_activeFFTPlots.contains(index))
    m_activeFFTPlots[index] = enabled;
}

/**
 * @brief Sets the active state of a multiplot.
 */
void UI::Dashboard::setMultiplotRunning(const int index, const bool enabled)
{
  if (m_activeMultiplots.contains(index))
    m_activeMultiplots[index] = enabled;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Sets the active state of a waterfall plot.
 */
void UI::Dashboard::setWaterfallRunning(const int index, const bool enabled)
{
  if (m_activeWaterfalls.contains(index))
    m_activeWaterfalls[index] = enabled;
}
#endif

/**
 * @brief Configures sweep/trigger mode for a plot; gated to commercial tiers.
 */
void UI::Dashboard::setPlotSweep(const int index,
                                 const bool enabled,
                                 const double level,
                                 const int edge,
                                 const int mode,
                                 const double holdoff,
                                 const double timebase)
{
  auto it = m_plotSweep.find(index);
  if (it == m_plotSweep.end())
    return;

#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  const bool ok  = enabled && tk.isValid() && SS_LICENSE_GUARD();
#else
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, 0);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Configures sweep/trigger mode for a multiplot; gated to commercial tiers.
 */
void UI::Dashboard::setMultiplotSweep(const int index,
                                      const bool enabled,
                                      const double level,
                                      const int edge,
                                      const int mode,
                                      const double holdoff,
                                      const int triggerCurve,
                                      const double timebase)
{
  auto it = m_multiplotSweep.find(index);
  if (it == m_multiplotSweep.end())
    return;

#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  const bool ok  = enabled && tk.isValid() && SS_LICENSE_GUARD();
#else
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, triggerCurve);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Re-arms a single-shot plot sweep capture.
 */
void UI::Dashboard::armPlotSweep(const int index)
{
  auto it = m_plotSweep.find(index);
  if (it != m_plotSweep.end())
    it.value().arm();
}

/**
 * @brief Re-arms a single-shot multiplot sweep capture.
 */
void UI::Dashboard::armMultiplotSweep(const int index)
{
  auto it = m_multiplotSweep.find(index);
  if (it != m_multiplotSweep.end())
    it.value().arm();
}

//--------------------------------------------------------------------------------------------------
// Frame processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a frame and updates the dashboard. Each source owns its plot clock
 *        (m_plotClocks[sid]); the per-source display clock never runs backwards and is published
 *        to the m_plotDisplayTimeSec scalar the append/advance sites read. A matching cached
 *        publisher generation skips the per-frame compare_frames() walk (changes only on rebuild).
 */
void UI::Dashboard::hotpathRxFrame(const DataModel::TimestampedFramePtr& frame)
{
  SS_ASSERT_HOTPATH(frame);
  SS_ASSERT_HOTPATH(frame->data.sourceId >= 0);

  const auto& payload = frame->data;

  if (payload.groups.size() <= 0 || !m_streamAvailable) [[unlikely]]
    return;

  const int sid = payload.sourceId;
  (void)advancePlotClock(sid, frame->timestamp);

  const auto genIt = m_sourceStructureGen.constFind(sid);
  const bool genKnown =
    genIt != m_sourceStructureGen.cend() && genIt.value() == frame->structureGeneration;

  const bool structureChanged =
    !genKnown || !m_sourceRawFrames.contains(sid) || m_valuePushes.isEmpty();

  if (structureChanged) [[unlikely]] {
    const bool hadProFeatures = containsCommercialFeatures();
    m_sourceStructureGen[sid] = frame->structureGeneration;
    m_sourceRawFrames[sid]    = payload;

    reconfigureDashboard(combineSourceFrames(payload));

    if (hadProFeatures != containsCommercialFeatures())
      Q_EMIT containsCommercialFeaturesChanged();
  }

  updateDashboardData(payload);

  m_updateRequired = true;
}

/**
 * @brief Advances the per-source plot clock for one publish (frame or stream block) and returns
 *        the new forward-only display time, also published to m_plotDisplayTimeSec. Each source
 *        owns its clock so interleaved publishes never rewind another source's rings.
 */
double UI::Dashboard::advancePlotClock(int sourceId,
                                       const std::chrono::steady_clock::time_point& ts)
{
  // code-verify off
  // Scoped tight: reconfigureDashboard move-assigns m_plotClocks, so this reference must not
  // survive past this function.
  PlotClock& clk = m_plotClocks[sourceId];
  // code-verify on

  if (!clk.originSet) [[unlikely]] {
    clk.origin          = ts;
    clk.originSet       = true;
    clk.groupCount      = 0;
    clk.groupStartSec   = 0;
    clk.displayTimeSec  = 0;
    clk.samplePeriodSec = 0;
  }
  clk.relativeFrameTimeSec = std::chrono::duration<double>(ts - clk.origin).count();

  ++clk.groupCount;
  if (clk.relativeFrameTimeSec > clk.groupStartSec) {
    const int n      = (clk.groupCount > 1) ? (clk.groupCount - 1) : 1;
    const double gap = clk.relativeFrameTimeSec - clk.groupStartSec;

    // code-verify off
    // Divide only for rare multi-sample coarse-clock groups; fine-timestamp sources hit n == 1.
    const double period = (n > 1) ? (gap / n) : gap;
    // code-verify on

    clk.samplePeriodSec =
      (clk.samplePeriodSec > 0) ? (0.8 * clk.samplePeriodSec + 0.2 * period) : period;
    clk.groupStartSec = clk.relativeFrameTimeSec;
    clk.groupCount    = 1;
  }
  const double expected = clk.displayTimeSec + clk.samplePeriodSec;

  double displayNext = qMax(expected, clk.relativeFrameTimeSec);

  const double forwardError = clk.relativeFrameTimeSec - expected;
  if (clk.samplePeriodSec > 0 && clk.samplePeriodSec < kSmoothMaxPeriodSec && forwardError > 0
      && forwardError < kSmoothMaxForwardSec)
    displayNext = expected;

  clk.displayTimeSec   = displayNext;
  m_plotDisplayTimeSec = displayNext;
  return displayNext;
}

//--------------------------------------------------------------------------------------------------
// Widget map population
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a frame that does not match the widget model: one rebuild retry, then the
 *        (sourceId, structureGeneration) pair is quarantined and its frames dropped cheaply.
 *        The link, session, and players stay alive because a dashboard build failure is a
 *        rendering problem, never a reason to take the device away; a new generation retries.
 */
void UI::Dashboard::handleMissingDataset(const DataModel::Frame& frame)
{
  SS_ASSERT(!frame.groups.empty(), return);
  SS_ASSERT(frame.sourceId >= 0, return);

  const quint64 generation = m_sourceStructureGen.value(frame.sourceId);
  const auto qit           = m_quarantinedSources.constFind(frame.sourceId);
  if (qit != m_quarantinedSources.cend() && qit.value() == generation)
    return;

  if (m_updateRetryInProgress) {
    qWarning() << "[Dashboard] widget model build failed twice for source" << frame.sourceId
               << "-- dropping its frames until the structure changes";
    m_quarantinedSources.insert(frame.sourceId, generation);
    return;
  }

  m_sourceRawFrames[frame.sourceId] = frame;
  reconfigureDashboard(combineSourceFrames(frame));

  m_updateRetryInProgress = true;
  updateDashboardData(frame);
  m_updateRetryInProgress = false;
}

/**
 * @brief Updates dataset values and plot data based on the given frame.
 */
void UI::Dashboard::updateDashboardData(const DataModel::Frame& frame)
{
  SS_ASSERT_HOTPATH(!frame.groups.empty());
  SS_ASSERT_HOTPATH(frame.sourceId >= 0);

  if (!m_layoutValid) [[unlikely]]
    return;

  const auto pit = m_valuePushes.constFind(frame.sourceId);
  if (pit == m_valuePushes.cend()) [[unlikely]] {
    handleMissingDataset(frame);
    return;
  }

  const auto& table         = pit.value();
  const std::size_t entries = table.size();

  std::size_t i = 0;
  for (const auto& group : frame.groups) {
    for (const auto& dataset : group.datasets) {
      if (i >= entries || table[i].uniqueId != dataset.uniqueId) [[unlikely]] {
        handleMissingDataset(frame);
        return;
      }

      const auto& push = table[i++];
      for (auto* ptr : push.targets) {
        ptr->isNumeric    = dataset.isNumeric;
        ptr->numericValue = dataset.numericValue;
      }

      const auto& string_targets = dataset.isNumeric ? push.stringTargets : push.targets;
      for (auto* ptr : string_targets)
        ptr->value = dataset.value;
    }
  }

  foldExtremes(frame.sourceId);
  updateDataSeries(frame.sourceId);
}

/**
 * @brief Folds the just-propagated values of one source's extreme-hold datasets into their
 *        min/max slots (spec 0052). The table holds only opted-in datasets, so the common case
 *        is one failed hash lookup; non-finite and non-numeric samples never move the hold.
 */
void UI::Dashboard::foldExtremes(int sourceId)
{
  const auto it = m_extremePushes.constFind(sourceId);
  if (it == m_extremePushes.cend()) [[likely]]
    return;

  for (const auto& push : it.value()) {
    if (!*push.numeric || !std::isfinite(*push.value))
      continue;

    const double value = *push.value;
    if (!push.slot->valid) {
      push.slot->min   = value;
      push.slot->max   = value;
      push.slot->valid = true;
      continue;
    }

    push.slot->min = qMin(push.slot->min, value);
    push.slot->max = qMax(push.slot->max, value);
  }
}

/**
 * @brief Registers a dataset's index and per-widget-key mappings.
 */
void UI::Dashboard::processDatasetIntoWidgetMaps(const DataModel::Dataset& datasetIn,
                                                 DataModel::Group& ledPanel)
{
  SS_ASSERT(datasetIn.index >= 0, return);
  SS_ASSERT(datasetIn.uniqueId >= 0, return);

  DataModel::Dataset dataset = datasetIn;
  if (DSP::almostEqual(dataset.wgtMin, dataset.wgtMax)) {
    dataset.wgtMin = dataset.pltMin;
    dataset.wgtMax = dataset.pltMax;
  }

  if (DSP::almostEqual(dataset.fftMin, dataset.fftMax)) {
    dataset.fftMin = dataset.pltMin;
    dataset.fftMax = dataset.pltMax;
  }

  if (!m_datasets.contains(dataset.uniqueId)) {
    m_datasets.insert(dataset.uniqueId, dataset);
  } else {
    auto prev     = m_datasets.value(dataset.uniqueId);
    double newMin = qMin(prev.pltMin, dataset.pltMin);
    double newMax = qMax(prev.pltMax, dataset.pltMax);

    auto d   = dataset;
    d.pltMin = newMin;
    d.pltMax = newMax;
    m_datasets.insert(dataset.uniqueId, d);
  }

  if (dataset.hideOnDashboard)
    return;

  auto keys = SerialStudio::getDashboardWidgets(dataset);
  for (const auto& widgetKey : std::as_const(keys)) {
    if (widgetKey == SerialStudio::DashboardLED) {
      ledPanel.datasets.push_back(dataset);
      continue;
    }
    if (widgetKey == SerialStudio::DashboardExtension)
      m_extensionDatasetIds.append(dataset.widget);

    if (widgetKey != SerialStudio::DashboardNoWidget)
      m_widgetDatasets[widgetKey].append(dataset);
  }
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

  auto savedSourceFrames = m_sourceRawFrames;
  auto savedClocks       = m_plotClocks;

  auto savedPlotRings      = snapshotPlotTimeRings();
  auto savedMultiplotRings = snapshotMultiplotTimeRings();

  resetData(false);

  m_sourceRawFrames = std::move(savedSourceFrames);
  m_plotClocks      = std::move(savedClocks);

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

  buildWidgetGroups(frame, pro);

  applyDisplayTitles();
  registerWidgets();

  buildDatasetReferences();
  buildValuePushes();

  updateDataSeries();
  configureActions(frame);

  restorePlotTimeRings(savedPlotRings);
  restoreMultiplotTimeRings(savedMultiplotRings);

  m_layoutValid = true;

  Q_EMIT widgetCountChanged();
}

/**
 * @brief Populates m_widgetGroups and m_widgetDatasets from the current frame. A datasetless data
 *        grid materialises as an empty table on purpose: dropping it would shift every later
 *        widget's relativeIndex and orphan saved workspace references. Extension entries bucket
 *        under DashboardExtension in walk order, ids index-aligned in m_extensionGroupIds.
 */
void UI::Dashboard::buildWidgetGroups(const DataModel::Frame& frame, bool pro)
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT(!frame.groups.empty(), return);
  (void)frame;

  for (const auto& group : m_lastFrame.groups) {
    const auto key = SerialStudio::getDashboardWidget(group);

    if (key == SerialStudio::DashboardExtension)
      m_extensionGroupIds.append(group.widget);

    if (key != SerialStudio::DashboardNoWidget)
      m_widgetGroups[key].append(group);

    if (key == SerialStudio::DashboardPlot3D && !pro) {
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      relabelGroupAsMultiplotFallback(group.groupId, copy.title);
    }

#ifdef BUILD_COMMERCIAL
    if (key == SerialStudio::DashboardPainter && !pro) {
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardDataGrid].append(copy);
    }
#endif

    if (key == SerialStudio::DashboardAccelerometer) {
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);
      if (pro)
        m_widgetGroups[SerialStudio::DashboardPlot3D].append(group);
    }

    if (key == SerialStudio::DashboardGyroscope)
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);

    DataModel::Group ledPanel;
    for (const auto& dataset : group.datasets)
      processDatasetIntoWidgetMaps(dataset, ledPanel);

    if (ledPanel.datasets.size() > 0) {
      ledPanel.widget   = "led-panel";
      ledPanel.groupId  = group.groupId;
      ledPanel.uniqueId = group.uniqueId;
      ledPanel.title    = tr("LED Panel (%1)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardLED].append(ledPanel);
    }
  }
}

/**
 * @brief Rewrites the matching group entry in m_lastFrame as a multiplot fallback.
 */
void UI::Dashboard::relabelGroupAsMultiplotFallback(int groupId, const QString& newTitle)
{
  for (size_t i = 0; i < m_lastFrame.groups.size(); ++i) {
    if (m_lastFrame.groups[i].groupId != groupId)
      continue;

    m_lastFrame.groups[i].title  = newTitle;
    m_lastFrame.groups[i].widget = "multiplot";
    return;
  }
}

/**
 * @brief Applies display-title overrides to the widget copies in m_widgetGroups and
 *        m_widgetDatasets: widget-level entries ("type:uid") beat entity-level ones ("uid"),
 *        canonical titles resolve from m_lastFrame so a removed override restores the original
 *        text; extension widgets key off "ext:&lt;id&gt;" instead of the numeric type.
 */
void UI::Dashboard::applyDisplayTitles()
{
  static auto& appState     = AppState::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  const auto overrides = projectModel.displayTitles();

  QHash<int, QString> canonical;
  for (const auto& group : m_lastFrame.groups) {
    canonical.insert(group.uniqueId, group.title);
    for (const auto& dataset : group.datasets)
      canonical.insert(dataset.uniqueId, dataset.title);
  }

  const auto widgetOverride = [&](const QString& token, int uniqueId) {
    return overrides.value(token + QLatin1Char(':') + QString::number(uniqueId)).toString();
  };

  const auto entityResolve = [&](int uniqueId, const QString& current) {
    const auto over = overrides.value(QString::number(uniqueId)).toString();
    if (!over.isEmpty())
      return over;

    return canonical.value(uniqueId, current);
  };

  const auto resolve = [&](const QString& token, int uniqueId, const QString& current) {
    const auto scoped = widgetOverride(token, uniqueId);
    return scoped.isEmpty() ? entityResolve(uniqueId, current) : scoped;
  };

  const auto typeToken = [this](SerialStudio::DashboardWidget key, bool group, int index) {
    if (key != SerialStudio::DashboardExtension)
      return QString::number(static_cast<int>(key));

    return UI::WidgetExtensions::persistedTypeToken(extensionIdAt(group, index));
  };

  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    for (int j = 0; j < i.value().count(); ++j) {
      auto& group      = i.value()[j];
      const auto token = typeToken(i.key(), true, j);
      if (group.widget != QLatin1String("led-panel")) {
        group.title = resolve(token, group.uniqueId, group.title);
        continue;
      }

      const auto scoped = widgetOverride(token, group.uniqueId);
      group.title       = scoped.isEmpty()
                          ? tr("LED Panel (%1)").arg(entityResolve(group.uniqueId, group.title))
                          : scoped;
    }
  }

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    for (int j = 0; j < i.value().count(); ++j) {
      auto& dataset    = i.value()[j];
      const auto token = typeToken(i.key(), false, j);
      dataset.title    = resolve(token, dataset.uniqueId, dataset.title);
    }
  }
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

  applyDisplayTitles();

  static auto& registry = WidgetRegistry::instance();
  for (auto i = m_widgetGroups.constBegin(); i != m_widgetGroups.constEnd(); ++i) {
    const auto key = i.key();
    for (int j = 0; j < i.value().size(); ++j)
      registry.updateWidget(registry.widgetIdByTypeAndIndex(key, j), i.value().at(j).title);
  }

  for (auto i = m_widgetDatasets.constBegin(); i != m_widgetDatasets.constEnd(); ++i) {
    const auto key = i.key();
    const int base = datasetBucketBase(key);
    for (int j = 0; j < i.value().size(); ++j)
      registry.updateWidget(registry.widgetIdByTypeAndIndex(key, base + j), i.value().at(j).title);
  }

  Q_EMIT displayTitlesChanged();
}

/**
 * @brief Registers all group and dataset widgets with the WidgetRegistry. Registry ids are handed
 *        out in creation order per type, so the dataset pass offsets its relative indices for the
 *        extension bucket (whose first slots belong to the group-scope packages registered above).
 */
void UI::Dashboard::registerWidgets()
{
  SS_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty(), return);
  SS_ASSERT(m_widgetCount == 0, m_widgetCount = 0);

  static auto& registry = WidgetRegistry::instance();
  registry.beginBatchUpdate();

  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto key   = i.key();
    const auto count = i.value().count();
    for (int j = 0; j < count; ++j) {
      const auto& group = i.value().at(j);
      (void)registry.createWidget(key, group.title, group.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto key   = i.key();
    const int base   = datasetBucketBase(key);
    const auto count = i.value().count();
    for (int j = 0; j < count; ++j) {
      const auto& dataset = i.value().at(j);
      (void)registry.createWidget(key, dataset.title, dataset.groupId, dataset.index, false);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, base + j));
    }
  }

  registry.endBatchUpdate();
}

/**
 * @brief Builds the m_datasetReferences map from all widget and frame sources.
 */
void UI::Dashboard::buildDatasetReferences()
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty(), return);

  for (auto& groupList : m_widgetGroups) {
    for (auto& group : groupList)
      for (auto& dataset : group.datasets)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
  }

  for (auto& datasetList : m_widgetDatasets)
    for (auto& dataset : datasetList)
      m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& dataset : m_datasets)
    m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& group : m_lastFrame.groups) {
    for (auto& dataset : group.datasets) {
      auto& list = m_datasetReferences[dataset.uniqueId];
      if (!list.contains(&dataset))
        list.append(&dataset);
    }
  }
}

/**
 * @brief Rebuilds the dataset reference map after the frame layout has changed.
 *        Any push_back/erase on m_lastFrame.groups shifts elements and dangles the
 *        &dataset pointers stored here, so every such mutation must call this; the
 *        early-out guards buildDatasetReferences(), which asserts on an empty frame.
 */
void UI::Dashboard::rebuildDatasetReferences()
{
  m_datasetReferences.clear();
  m_valuePushes.clear();

  if (m_lastFrame.groups.empty())
    return;

  buildDatasetReferences();
  buildValuePushes();
}

/**
 * @brief Resolves one dataset's propagation targets from m_datasetReferences.
 */
UI::Dashboard::ValuePush UI::Dashboard::makeValuePush(
  const DataModel::Dataset& dataset, const QSet<const DataModel::Dataset*>& stringTargets) const
{
  SS_ASSERT_LOG(!m_datasetReferences.isEmpty());

  ValuePush push;
  push.uniqueId = dataset.uniqueId;

  const auto ref_it = m_datasetReferences.constFind(dataset.uniqueId);
  if (ref_it == m_datasetReferences.cend()) {
    push.uniqueId = std::numeric_limits<int>::min();
    return push;
  }

  for (auto* target : ref_it.value()) {
    push.targets.push_back(target);
    if (stringTargets.contains(target))
      push.stringTargets.push_back(target);
  }

  return push;
}

/**
 * @brief Pre-resolves the per-source value-propagation tables from m_datasetReferences. A
 *        zero-dataset layout (image/painter-only) still registers an empty table per source:
 *        a legitimate datasetless frame must find its table instead of tripping the
 *        missing-dataset quarantine on every frame.
 */
void UI::Dashboard::buildValuePushes()
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT_LOG(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty());

  m_valuePushes.clear();

  QSet<const DataModel::Dataset*> string_targets;
  for (auto& group : m_lastFrame.groups)
    for (auto& dataset : group.datasets)
      string_targets.insert(&dataset);

  const auto grid_it = m_widgetGroups.constFind(SerialStudio::DashboardDataGrid);
  if (grid_it != m_widgetGroups.cend()) {
    for (const auto& group : grid_it.value())
      for (const auto& dataset : group.datasets)
        string_targets.insert(&dataset);
  }

  const auto panel_it = m_widgetGroups.constFind(SerialStudio::DashboardBarPanel);
  if (panel_it != m_widgetGroups.cend()) {
    for (const auto& group : panel_it.value())
      for (const auto& dataset : group.datasets)
        string_targets.insert(&dataset);
  }

  addExtensionStringTargets(string_targets);

  for (auto it = m_sourceRawFrames.cbegin(); it != m_sourceRawFrames.cend(); ++it) {
    auto& table = m_valuePushes[it.key()];
    for (const auto& group : it.value().groups)
      for (const auto& dataset : group.datasets)
        table.push_back(makeValuePush(dataset, string_targets));
  }

  buildExtremePushes();
}

/**
 * @brief Pre-resolves the per-source extreme-hold fold tables (spec 0052): one entry per opted-in
 *        dataset, pointing at the address-stable m_datasets copy and its m_datasetExtremes slot.
 *        Datasets that never opt in contribute no entry, so the per-frame fold walks nothing.
 */
void UI::Dashboard::buildExtremePushes()
{
  m_extremePushes.clear();

  for (auto it = m_sourceRawFrames.cbegin(); it != m_sourceRawFrames.cend(); ++it)
    for (const auto& group : it.value().groups)
      for (const auto& dataset : group.datasets)
        appendExtremePush(it.key(), dataset);
}

/**
 * @brief Appends one extreme-hold fold entry when @a dataset opted in and resolves in m_datasets.
 */
void UI::Dashboard::appendExtremePush(int sourceId, const DataModel::Dataset& dataset)
{
  if (!dataset.extremeHold)
    return;

  const auto ds_it = m_datasets.constFind(dataset.uniqueId);
  if (ds_it == m_datasets.cend())
    return;

  ExtremePush push;
  push.slot    = &m_datasetExtremes[dataset.uniqueId];
  push.value   = &ds_it.value().numericValue;
  push.numeric = &ds_it.value().isNumeric;
  m_extremePushes[sourceId].push_back(push);
}

/**
 * @brief Adds the widget copies of every extension package that declared readsStringValues to the
 *        string-target set, which is what keeps a package that renders Dataset::value from reading
 *        a stale string. Reconfigure-time only: the per-frame walk is untouched, and a package
 *        that never declares the flag contributes no target and therefore no work.
 */
void UI::Dashboard::addExtensionStringTargets(QSet<const DataModel::Dataset*>& targets) const
{
  static auto& catalog = UI::WidgetExtensions::instance();

  const auto groups = m_widgetGroups.constFind(SerialStudio::DashboardExtension);
  if (groups != m_widgetGroups.cend()) {
    for (int i = 0; i < groups->count(); ++i) {
      if (!catalog.descriptor(extensionIdAt(true, i)).readsStringValues)
        continue;

      for (const auto& dataset : groups->at(i).datasets)
        targets.insert(&dataset);
    }
  }

  const auto datasets = m_widgetDatasets.constFind(SerialStudio::DashboardExtension);
  if (datasets != m_widgetDatasets.cend()) {
    for (int i = 0; i < datasets->count(); ++i)
      if (catalog.descriptor(extensionIdAt(false, i)).readsStringValues)
        targets.insert(&datasets->at(i));
  }
}

/**
 * @brief Returns the relative-index offset of one dataset bucket. Extension widgets share a single
 *        enum value with the group-scope packages that occupy the bucket's first slots, so their
 *        dataset copies start after them; every built-in type owns its bucket alone.
 */
int UI::Dashboard::datasetBucketBase(const SerialStudio::DashboardWidget key) const noexcept
{
  return key == SerialStudio::DashboardExtension ? m_extensionGroupIds.count() : 0;
}

//--------------------------------------------------------------------------------------------------
// Data series configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates time-series data for all dashboard widgets that require historical tracking.
 */
void UI::Dashboard::updateDataSeries(int sourceId)
{
  SS_ASSERT_LOG(m_widgetCount > 0 || m_widgetMap.isEmpty());
  SS_ASSERT(!m_sourceRawFrames.isEmpty(), return);

  const int gpsCount   = widgetCount(SerialStudio::DashboardGPS);
  const int fftCount   = widgetCount(SerialStudio::DashboardFFT);
  const int plotCount  = widgetCount(SerialStudio::DashboardPlot);
  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
#ifdef BUILD_COMMERCIAL
  const int plot3DCount    = widgetCount(SerialStudio::DashboardPlot3D);
  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
#endif

  if (m_gpsValues.size() != gpsCount) [[unlikely]]
    configureGpsSeries();
  if (m_fftValues.size() != fftCount) [[unlikely]]
    configureFftSeries();
  if (m_pltValues.size() != plotCount) [[unlikely]]
    configureLineSeries();
  if (m_multipltValues.size() != multiCount) [[unlikely]]
    configureMultiLineSeries();
#ifdef BUILD_COMMERCIAL
  if (m_plotData3D.size() != plot3DCount) [[unlikely]]
    configurePlot3DSeries();
  if (m_waterfallValues.size() != waterfallCount) [[unlikely]]
    configureWaterfallSeries();
#endif

  updateGpsSeries(sourceId);
  updateFftSeries(sourceId);
  updateLineSeries(sourceId);
#ifdef BUILD_COMMERCIAL
  updateWaterfallSeries(sourceId);
#endif

  auto feedMultiRings = [this](const MultiPush& p) {
    auto rIt = m_multiplotTimeRings.find(p.groupIndex);
    if (rIt == m_multiplotTimeRings.end())
      return;

    auto& rings = rIt.value();
    for (const auto& tc : p.timeCurves)
      if (tc.curveIndex >= 0 && static_cast<std::size_t>(tc.curveIndex) < rings.size())
        rings[tc.curveIndex].appendDecimated(m_plotDisplayTimeSec, *tc.value);
  };

  auto feedMultiSweep = [this](const MultiPush& p) {
    if (p.timeCurves.empty())
      return;

    auto sIt = m_multiplotSweep.find(p.groupIndex);
    if (sIt == m_multiplotSweep.end())
      return;

    DSP::SweepEngine& sweep = sIt.value();
    if (!sweep.enabled)
      return;

    const int last  = static_cast<int>(p.timeCurves.size()) - 1;
    const int tc    = qBound(0, sweep.triggerCurve, last);
    const double st = sweep.advance(m_plotDisplayTimeSec, *p.timeCurves[tc].value);
    if (st < 0)
      return;

    const std::size_t n = std::min(sweep.back.size(), p.timeCurves.size());
    for (std::size_t j = 0; j < n; ++j)
      sweep.back[j].appendDecimated(st, *p.timeCurves[j].value);
  };

  SS_ASSERT_LOG(static_cast<int>(m_multiplotPushes.size()) == multiCount);
  for (const auto& p : m_multiplotPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    feedMultiRings(p);
    feedMultiSweep(p);

    for (const auto& s : p.samples)
      s.first->push(*s.second);
  }

#ifdef BUILD_COMMERCIAL
  updatePlot3DSeries(sourceId);
#endif
}

/**
 * @brief Updates FFT data series for all active FFT plot widgets.
 */
void UI::Dashboard::updateFftSeries(int sourceId)
{
  SS_ASSERT_LOG(static_cast<int>(m_fftPushes.size()) == m_fftValues.size());
  SS_ASSERT_LOG(m_activeFFTPlots.size() == m_fftValues.size());

#ifdef BUILD_COMMERCIAL
  static auto& audioExport = Widgets::AudioExport::instance();
#endif

  for (const auto& p : m_fftPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
#ifdef BUILD_COMMERCIAL
    if (p.record) [[unlikely]]
      audioExport.enqueueSample(p.sessionKey, *p.value);
#endif
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Arms or disarms the audio-recording tap on one FFT widget's ingest push. Taps are
 *        index-aligned with the FFT widget order and reset on every push-table rebuild, so a
 *        stale index can never fire against a re-indexed widget.
 */
void UI::Dashboard::setFftAudioTap(const int index, const bool enabled, const quint32 key)
{
  SS_ASSERT_LOG(static_cast<int>(m_fftPushes.size()) == m_fftValues.size());
  if (index < 0 || index >= static_cast<int>(m_fftPushes.size()))
    return;

  m_fftPushes[index].record     = enabled;
  m_fftPushes[index].sessionKey = key;
}
#endif

/**
 * @brief Updates GPS trajectory series for all GPS widgets.
 */
void UI::Dashboard::updateGpsSeries(int sourceId)
{
  SS_ASSERT_LOG(static_cast<int>(m_gpsPushes.size()) == m_gpsValues.size());

  // code-verify off
  // Debug-only layout parity check: contains() hashes into the widget-group map on every frame.
  Q_ASSERT(m_widgetGroups.contains(SerialStudio::DashboardGPS) || m_gpsPushes.empty());
  // code-verify on

  for (const auto& p : m_gpsPushes) {
    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    const double lat = *p.lat.numeric ? *p.lat.value : kNoGpsFix;
    const double lon = *p.lon.numeric ? *p.lon.value : kNoGpsFix;
    const double alt = *p.alt.numeric ? *p.alt.value : kNoGpsFix;

    p.series->latitudes.push(lat);
    p.series->longitudes.push(lon);
    p.series->altitudes.push(alt);
  }
}

/**
 * @brief Updates 3D trajectory plot series for all 3D plot widgets.
 */
void UI::Dashboard::updatePlot3DSeries(int sourceId)
{
#ifdef BUILD_COMMERCIAL
  SS_ASSERT_LOG(static_cast<int>(m_plot3DPushes.size()) == m_plot3DRings.size());
  SS_ASSERT(m_points > 0, return);

  const auto maxPoints = static_cast<std::size_t>(points());
  for (const auto& p : m_plot3DPushes) {
    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    if (p.ring->capacity() != maxPoints) [[unlikely]]
      p.ring->resize(maxPoints);

    p.ring->push(
      QVector3D(static_cast<float>(*p.x), static_cast<float>(*p.y), static_cast<float>(*p.z)));
  }
#else
  (void)sourceId;
#endif
}

/**
 * @brief Updates linear plot data series for all active plot widgets.
 */
void UI::Dashboard::updateLineSeries(int sourceId)
{
  // code-verify off
  // Debug-only layout parity check: widgetCount() runs two map lookups per call, per frame.
  Q_ASSERT(m_pltValues.size() == widgetCount(SerialStudio::DashboardPlot));
  Q_ASSERT(m_activePlots.size() == widgetCount(SerialStudio::DashboardPlot));
  // code-verify on

  auto fire = [sourceId](const LinePush& p) {
    for (const auto& c : p.consumers) {
      if (sourceId >= 0 && c.sourceId != sourceId)
        continue;

      if (*c.activeFlag) {
        p.buf->push(*p.value);
        return;
      }
    }
  };

  for (const auto& p : m_yLinePushes)
    fire(p);

  for (const auto& p : m_xLinePushes)
    fire(p);

  auto feedSweep = [this](const TimePush& p) {
    auto sIt = m_plotSweep.find(p.plotIndex);
    if (sIt == m_plotSweep.end())
      return;

    DSP::SweepEngine& sweep = sIt.value();
    if (!sweep.enabled || sweep.back.empty())
      return;

    const double st = sweep.advance(m_plotDisplayTimeSec, *p.value);
    if (st >= 0)
      sweep.back[0].appendDecimated(st, *p.value);
  };

  for (const auto& p : m_timePushes) {
    auto rIt = m_plotTimeRings.find(p.plotIndex);
    if (rIt == m_plotTimeRings.end()) [[unlikely]]
      continue;

    for (const auto& c : p.consumers) {
      if (sourceId >= 0 && c.sourceId != sourceId)
        continue;

      if (*c.activeFlag) {
        rIt.value().appendDecimated(m_plotDisplayTimeSec, *p.value);
        feedSweep(p);
        break;
      }
    }
  }
}

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
  m_gpsPushes.clear();
  m_gpsPushes.shrink_to_fit();

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

  m_gpsPushes.reserve(static_cast<std::size_t>(gpsCount));
  for (int i = 0; i < gpsCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardGPS, i);

    GpsPush push;
    push.sourceId = group.sourceId;
    push.series   = &m_gpsValues[i];
    push.lat      = {&kNoGpsFix, &kNeverNumeric};
    push.lon      = {&kNoGpsFix, &kNeverNumeric};
    push.alt      = {&kNoGpsFix, &kNeverNumeric};

    for (const auto& dataset : group.datasets) {
      const GpsPush::Field field{&dataset.numericValue, &dataset.isNumeric};
      if (dataset.widget == QStringLiteral("lat"))
        push.lat = field;

      if (dataset.widget == QStringLiteral("lon"))
        push.lon = field;

      if (dataset.widget == QStringLiteral("alt"))
        push.alt = field;
    }

    m_gpsPushes.push_back(push);
  }
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
  m_fftPushes.clear();
  m_fftPushes.shrink_to_fit();

  const int fftCount = widgetCount(SerialStudio::DashboardFFT);
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);
    const int capacity  = Widgets::normalizedFftSize(dataset.fftSamples);
    m_fftValues.append(DSP::AxisData(capacity));
    m_activeFFTPlots.insert(i, true);
  }

  m_fftPushes.reserve(static_cast<std::size_t>(fftCount));
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardFFT, i);

    SeriesPush push;
    push.sourceId   = dataset.sourceId;
    push.activeFlag = &m_activeFFTPlots[i];
    push.buf        = &m_fftValues[i];
    push.value      = &dataset.numericValue;
#ifdef BUILD_COMMERCIAL
    push.record     = false;
    push.sessionKey = 0;
#endif
    m_fftPushes.push_back(push);
  }
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Updates time-domain ring buffers feeding all active waterfall widgets.
 */
void UI::Dashboard::updateWaterfallSeries(int sourceId)
{
  SS_ASSERT_LOG(static_cast<int>(m_waterfallPushes.size()) == m_waterfallValues.size());
  SS_ASSERT_LOG(m_activeWaterfalls.size() == m_waterfallValues.size());

  static auto& audioExport = Widgets::AudioExport::instance();

  for (const auto& p : m_waterfallPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
    if (p.record) [[unlikely]]
      audioExport.enqueueSample(p.sessionKey, *p.value);
  }
}

/**
 * @brief Arms or disarms the audio-recording tap on one waterfall widget's ingest push. Taps
 *        are index-aligned with the waterfall widget order and reset on every push-table
 *        rebuild, so a stale index can never fire against a re-indexed widget.
 */
void UI::Dashboard::setWaterfallAudioTap(const int index, const bool enabled, const quint32 key)
{
  SS_ASSERT_LOG(static_cast<int>(m_waterfallPushes.size()) == m_waterfallValues.size());
  if (index < 0 || index >= static_cast<int>(m_waterfallPushes.size()))
    return;

  m_waterfallPushes[index].record     = enabled;
  m_waterfallPushes[index].sessionKey = key;
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
  m_waterfallPushes.clear();
  m_waterfallPushes.shrink_to_fit();

  const int waterfallCount = widgetCount(SerialStudio::DashboardWaterfall);
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);
    const int capacity =
      Widgets::normalizedFftSize(dataset.fftSamples, Widgets::kMaxWaterfallFftSize);
    m_waterfallValues.append(DSP::AxisData(capacity));
    m_activeWaterfalls.insert(i, true);
  }

  m_waterfallPushes.reserve(static_cast<std::size_t>(waterfallCount));
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = getDatasetWidget(SerialStudio::DashboardWaterfall, i);

    SeriesPush push;
    push.sourceId   = dataset.sourceId;
    push.activeFlag = &m_activeWaterfalls[i];
    push.buf        = &m_waterfallValues[i];
    push.value      = &dataset.numericValue;
    push.record     = false;
    push.sessionKey = 0;
    m_waterfallPushes.push_back(push);
  }
}
#endif

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

//--------------------------------------------------------------------------------------------------
// Time-ring snapshot / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots plot time-ring contents keyed by dataset uniqueId.
 */
QHash<qint64, DSP::TimeRing> UI::Dashboard::snapshotPlotTimeRings() const
{
  QHash<qint64, DSP::TimeRing> out;
  const int n = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < n; ++i) {
    const auto it = m_plotTimeRings.find(i);
    if (it == m_plotTimeRings.end())
      continue;

    const auto& d = getDatasetWidget(SerialStudio::DashboardPlot, i);
    out.insert(ringSnapshotKey(d.sourceId, d.uniqueId), it.value());
  }

  return out;
}

/**
 * @brief Snapshots multiplot time-ring contents keyed by group uniqueId.
 */
QHash<qint64, std::vector<DSP::TimeRing>> UI::Dashboard::snapshotMultiplotTimeRings() const
{
  QHash<qint64, std::vector<DSP::TimeRing>> out;
  const int n = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < n; ++i) {
    const auto it = m_multiplotTimeRings.find(i);
    if (it == m_multiplotTimeRings.end())
      continue;

    const auto& g = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    out.insert(ringSnapshotKey(g.sourceId, g.uniqueId), it.value());
  }

  return out;
}

/**
 * @brief Replays a saved ring's samples into @p target via appendDecimated. Used when
 *        the new ring has a different capacity / interval than the saved one.
 */
static void replayTimeRing(const DSP::TimeRing& saved, DSP::TimeRing& target)
{
  const std::size_t n = saved.time.size();
  for (std::size_t k = 0; k < n; ++k)
    target.appendDecimated(saved.time[k], saved.value[k]);
}

/**
 * @brief Restores saved plot rings into the currently configured widget slots. Splices when
 *        the new ring shape matches the saved one, replays through appendDecimated otherwise.
 */
void UI::Dashboard::restorePlotTimeRings(QHash<qint64, DSP::TimeRing>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = widgetCount(SerialStudio::DashboardPlot);
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_plotTimeRings.find(i);
    if (ringIt == m_plotTimeRings.end())
      continue;

    const auto& d = getDatasetWidget(SerialStudio::DashboardPlot, i);
    auto savedIt  = snapshot.find(ringSnapshotKey(d.sourceId, d.uniqueId));
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();
    if (kept.time.raw() == nullptr)
      continue;

    if (live.time.capacity() == kept.time.capacity() && qFuzzyCompare(live.interval, kept.interval))
      live = std::move(kept);
    else
      replayTimeRing(kept, live);

    snapshot.erase(savedIt);
  }
}

/**
 * @brief Restores saved multiplot rings; matches by group uniqueId and per-curve shape.
 */
void UI::Dashboard::restoreMultiplotTimeRings(QHash<qint64, std::vector<DSP::TimeRing>>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = widgetCount(SerialStudio::DashboardMultiPlot);
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_multiplotTimeRings.find(i);
    if (ringIt == m_multiplotTimeRings.end())
      continue;

    const auto& g = getGroupWidget(SerialStudio::DashboardMultiPlot, i);
    auto savedIt  = snapshot.find(ringSnapshotKey(g.sourceId, g.uniqueId));
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();

    const std::size_t count = std::min(live.size(), kept.size());
    for (std::size_t j = 0; j < count; ++j) {
      if (kept[j].time.raw() == nullptr)
        continue;

      if (live[j].time.capacity() == kept[j].time.capacity()
          && qFuzzyCompare(live[j].interval, kept[j].interval))
        live[j] = std::move(kept[j]);
      else
        replayTimeRing(kept[j], live[j]);
    }

    snapshot.erase(savedIt);
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured plot engines.
 */
void UI::Dashboard::restorePlotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_plotSweep.find(it.key());
    if (live == m_plotSweep.end())
      continue;

    const auto& src = it.value();
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    live->enabled = src.enabled;
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured multiplot engines.
 */
void UI::Dashboard::restoreMultiplotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_multiplotSweep.find(it.key());
    if (live == m_multiplotSweep.end())
      continue;

    const auto& src = it.value();
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    live->enabled = src.enabled;
  }
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
      m_plotTimeRings.insert(i, makeHistoryRing(m_plotTimeRange));

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

  buildLinePushes();
}

/**
 * @brief Resolves the per-plot y/x/time push tables from the configured buffers, deduplicating
 *        shared Y and X sources so each buffer is pushed once per frame.
 */
void UI::Dashboard::buildLinePushes()
{
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();

  QHash<int, std::size_t> yByUid;
  QHash<int, std::size_t> xByXAxisId;
  for (int i = 0; i < widgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = getDatasetWidget(SerialStudio::DashboardPlot, i);
    const LinePush::Consumer consumer{yDataset.sourceId, &m_activePlots[i]};

    if (useTimeXAxis(yDataset)) {
      if (m_plotTimeRings.contains(i)) {
        TimePush tp;
        tp.consumers.push_back(consumer);
        tp.plotIndex = i;
        tp.value     = &yDataset.numericValue;
        m_timePushes.push_back(std::move(tp));
      }

      continue;
    }

    auto yIt = m_yAxisData.find(yDataset.uniqueId);
    if (yIt != m_yAxisData.end()) {
      auto cacheIt = yByUid.find(yDataset.uniqueId);
      if (cacheIt == yByUid.end()) {
        LinePush push;
        push.consumers.push_back(consumer);
        push.buf   = &yIt.value();
        push.value = &yDataset.numericValue;
        yByUid.insert(yDataset.uniqueId, m_yLinePushes.size());
        m_yLinePushes.push_back(std::move(push));
      } else {
        m_yLinePushes[cacheIt.value()].consumers.push_back(consumer);
      }
    }

    const int xAxisId = yDataset.xAxisId;

    auto xDsIt = m_datasets.find(xAxisId);
    if (xDsIt == m_datasets.end())
      continue;

    auto& xBuf   = m_xAxisData[xAxisId];
    auto cacheIt = xByXAxisId.find(xAxisId);
    if (cacheIt == xByXAxisId.end()) {
      LinePush push;
      push.consumers.push_back(consumer);
      push.buf   = &xBuf;
      push.value = &xDsIt.value().numericValue;
      xByXAxisId.insert(xAxisId, m_xLinePushes.size());
      m_xLinePushes.push_back(std::move(push));
    } else {
      m_xLinePushes[cacheIt.value()].consumers.push_back(consumer);
    }
  }
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
  m_plot3DPushes.clear();
  m_plot3DPushes.shrink_to_fit();

  m_plot3DRings.reserve(plot3DCount);
  for (int i = 0; i < plot3DCount; ++i) {
    m_plot3DRings.append(DSP::FixedQueue<QVector3D>(static_cast<std::size_t>(points())));
    m_plotData3D[i].reserve(static_cast<std::size_t>(points()));
  }

  m_plot3DPushes.reserve(static_cast<std::size_t>(plot3DCount));
  for (int i = 0; i < plot3DCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardPlot3D, i);

    Plot3DPush push;
    push.sourceId = group.sourceId;
    push.ring     = &m_plot3DRings[i];
    push.x        = &kZeroAxisSource;
    push.y        = &kZeroAxisSource;
    push.z        = &kZeroAxisSource;

    for (const auto& dataset : group.datasets) {
      const QString& id = dataset.widget;
      if (id == QStringLiteral("x") || id == QStringLiteral("X"))
        push.x = &dataset.numericValue;

      if (id == QStringLiteral("y") || id == QStringLiteral("Y"))
        push.y = &dataset.numericValue;

      if (id == QStringLiteral("z") || id == QStringLiteral("Z"))
        push.z = &dataset.numericValue;
    }

    m_plot3DPushes.push_back(push);
  }
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
      const int cap = timeRingCapacity(m_plotTimeRange);
      std::vector<DSP::TimeRing> rings;
      rings.reserve(group.datasets.size());
      for (size_t j = 0; j < group.datasets.size(); ++j)
        rings.push_back(makeHistoryRing(m_plotTimeRange));

      m_multiplotTimeRings.insert(i, std::move(rings));

      DSP::SweepEngine sweep;
      sweep.configure(static_cast<int>(group.datasets.size()), cap, m_plotTimeRange);
      m_multiplotSweep.insert(i, std::move(sweep));
    }
  }

  buildMultiplotPushes();
}

/**
 * @brief Resolves the per-tick multiplot push table from the configured buffers.
 */
void UI::Dashboard::buildMultiplotPushes()
{
  m_multiplotPushes.clear();
  m_multiplotPushes.shrink_to_fit();

  const int multiCount = widgetCount(SerialStudio::DashboardMultiPlot);
  m_multiplotPushes.reserve(static_cast<std::size_t>(multiCount));

  for (int i = 0; i < multiCount; ++i) {
    const auto& group = getGroupWidget(SerialStudio::DashboardMultiPlot, i);

    MultiPush push;
    push.sourceId   = group.sourceId;
    push.groupIndex = i;
    push.activeFlag = &m_activeMultiplots[i];

    auto rIt = m_multiplotTimeRings.find(i);
    if (rIt != m_multiplotTimeRings.end()) {
      auto& rings        = rIt.value();
      const size_t count = std::min(group.datasets.size(), rings.size());
      for (size_t j = 0; j < count; ++j)
        push.timeCurves.push_back({static_cast<int>(j), &group.datasets[j].numericValue});
    }

    else {
      auto& multiSeries   = m_multipltValues[i];
      const size_t yCount = multiSeries.y.size();
      const size_t count  = std::min(group.datasets.size(), yCount);
      for (size_t j = 0; j < count; ++j)
        push.samples.emplace_back(&multiSeries.y[j], &group.datasets[j].numericValue);
    }

    m_multiplotPushes.push_back(std::move(push));
  }
}

//--------------------------------------------------------------------------------------------------
// Action configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures dashboard actions and associated timers from the given DataModel frame.
 */
void UI::Dashboard::configureActions(const DataModel::Frame& frame)
{
  if (frame.groups.size() <= 0)
    return;

  m_actions.clear();
  m_actions.squeeze();

  for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
    if (it.value()) {
      disconnect(it.value());
      it.value()->stop();
      delete it.value();
    }
  }

  m_timers.clear();
  m_repeatCounters.clear();

  for (const auto& action : frame.actions)
    m_actions.append(action);

  static auto& ioManager = IO::ConnectionManager::instance();
  if (!ioManager.isConnected()) {
    Q_EMIT actionStatusChanged();
    return;
  }

  for (int i = 0; i < m_actions.count(); ++i) {
    const auto& action = m_actions[i];
    if (action.timerMode == DataModel::TimerMode::Off)
      continue;

    const auto interval = action.timerIntervalMs;
    if (interval <= 0) {
      qWarning() << "Interval for action" << action.title << "must be greater than 0!";
      continue;
    }

    auto* timer = new QTimer(this);
    timer->setInterval(interval);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, [this, i]() { activateAction(i, false); });

    const bool isRepeat = action.timerMode == DataModel::TimerMode::RepeatNTimes;
    if (isRepeat && action.autoExecuteOnConnect) {
      m_repeatCounters[i] = qMax(1, action.repeatCount);
      timer->start();
    }

    else if (!isRepeat
             && (action.timerMode == DataModel::TimerMode::AutoStart
                 || action.autoExecuteOnConnect))
      timer->start();

    m_timers.insert(i, timer);
  }

  Q_EMIT actionStatusChanged();
}
