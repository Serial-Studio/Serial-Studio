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

#include "UI/Dashboard/DashboardIngest.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr double kSmoothMaxPeriodSec  = 0.002;
constexpr double kSmoothMaxForwardSec = 0.050;

// Pre-resolved push fallbacks for GPS / 3D groups missing an axis dataset
static constexpr double kNoGpsFix   = std::numeric_limits<double>::quiet_NaN();
static constexpr bool kNeverNumeric = false;
#ifdef BUILD_COMMERCIAL
static constexpr double kZeroAxisSource = 0.0;
#endif

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when at least one plot consuming @p push from @p sourceId is running; mirrors the
 *        per-frame consumer gate so a block feeds exactly the rings a frame would have.
 */
static bool push_is_active(const UI::LinePush& push, const int sourceId)
{
  for (const auto& consumer : push.consumers)
    if (consumer.sourceId == sourceId && *consumer.activeFlag)
      return true;

  return false;
}

/**
 * @brief Renders a numeric sample into the datasets that actually display Dataset::value. A dense
 *        column carries no text, and rendering it for every widget copy cost one heap allocation
 *        per column per block for strings nothing could read (F9).
 */
static void write_rendered_strings(const UI::ValuePush& push, const double value)
{
  if (push.stringTargets.empty()) [[likely]]
    return;

  const QString text = QString::number(value, 'g', 10);
  for (auto* ptr : push.stringTargets)
    ptr->value = text;
}

/**
 * @brief Appends the newest @p count samples of @p column into a sample ring, dropping whatever
 *        the ring could not have kept anyway. Bounded twice over: by the block's sample cap and
 *        by the ring's capacity, so a 4096-sample block into a 1000-point plot costs 1000 stores.
 */
static void push_ring_tail(DSP::AxisData& ring,
                           const DataModel::BlockColumn& column,
                           const std::size_t count)
{
  const std::size_t capacity = ring.capacity();
  const std::size_t first    = (count > capacity) ? (count - capacity) : 0;
  for (std::size_t i = first; i < count; ++i)
    ring.push(column.values[i]);
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the dashboard state the ingest path walks. Nothing is copied: the push tables
 *        resolved here hold raw pointers into the bound containers, so the facade stays the
 *        owner and a rebuild of those containers rebuilds the tables with them.
 */
UI::DashboardIngest::DashboardIngest(const IngestBindings& bindings, IngestHost& host)
  : m_host(host)
  , m_layoutValid(bindings.layoutValid)
  , m_streamAvailable(bindings.streamAvailable)
  , m_updateRequired(bindings.updateRequired)
  , m_updateRetryInProgress(bindings.updateRetryInProgress)
  , m_widgetCount(bindings.widgetCount)
#ifdef BUILD_COMMERCIAL
  , m_points(bindings.points)
#endif
  , m_plotDisplayTimeSec(bindings.plotDisplayTimeSec)
  , m_plotClocks(bindings.plotClocks)
  , m_widgetMap(bindings.widgetMap)
  , m_xAxisData(bindings.xAxisData)
  , m_yAxisData(bindings.yAxisData)
  , m_plotTimeRings(bindings.plotTimeRings)
  , m_multiplotTimeRings(bindings.multiplotTimeRings)
  , m_plotSweep(bindings.plotSweep)
  , m_multiplotSweep(bindings.multiplotSweep)
  , m_activePlots(bindings.activePlots)
  , m_activeFFTPlots(bindings.activeFFTPlots)
  , m_activeMultiplots(bindings.activeMultiplots)
  , m_gpsValues(bindings.gpsValues)
  , m_fftValues(bindings.fftValues)
  , m_pltValues(bindings.pltValues)
  , m_multipltValues(bindings.multipltValues)
#ifdef BUILD_COMMERCIAL
  , m_activeWaterfalls(bindings.activeWaterfalls)
  , m_waterfallValues(bindings.waterfallValues)
  , m_plot3DRings(bindings.plot3DRings)
  , m_plotData3D(bindings.plotData3D)
#endif
  , m_datasets(bindings.datasets)
  , m_datasetExtremes(bindings.datasetExtremes)
  , m_valuePushes(bindings.valuePushes)
  , m_extremePushes(bindings.extremePushes)
  , m_widgetGroups(bindings.widgetGroups)
  , m_widgetDatasets(bindings.widgetDatasets)
  , m_sourceRawFrames(bindings.sourceRawFrames)
  , m_sourceStructureGen(bindings.sourceStructureGen)
{}

//--------------------------------------------------------------------------------------------------
// Widget bucket sizes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number of group-scope widgets of @p widget in the current layout. Built-in group types
 *        only, which is every type the ingest path addresses; extension buckets are counted by
 *        the facade, which owns the package id vectors.
 */
int UI::DashboardIngest::groupWidgetCount(const SerialStudio::DashboardWidget widget) const
{
  const auto it = m_widgetGroups.constFind(widget);
  return it != m_widgetGroups.cend() ? it->count() : 0;
}

/**
 * @brief Number of dataset-scope widgets of @p widget in the current layout (see
 *        groupWidgetCount).
 */
int UI::DashboardIngest::datasetWidgetCount(const SerialStudio::DashboardWidget widget) const
{
  const auto it = m_widgetDatasets.constFind(widget);
  return it != m_widgetDatasets.cend() ? it->count() : 0;
}

//--------------------------------------------------------------------------------------------------
// Block ingestion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Ingests one published block. A uniform-grid block feeds its rings column-wise off one
 *        clock advance, then steps the per-block consumers (GPS, 3D) once; an irregular block
 *        replays each sample as a frame did before spec 0055. A generation mismatch is stale and
 *        dropped; generation 0 means an unversioned producer and gates on its template.
 */
void UI::DashboardIngest::applyBlock(const DataModel::DataBlockPtr& block)
{
  SS_ASSERT(block != nullptr, return);

  if (!m_layoutValid || !m_streamAvailable) [[unlikely]]
    return;

  if (block->samples <= 0 || block->columns.empty()) [[unlikely]]
    return;

  const int sid = block->sourceId;
  if (block->structureGeneration != 0) [[likely]] {
    const auto genIt = m_sourceStructureGen.constFind(sid);
    if (genIt == m_sourceStructureGen.cend() || genIt.value() != block->structureGeneration)
      [[unlikely]]
      return;

  } else if (!m_sourceRawFrames.contains(sid)) [[unlikely]]
    return;

  if (DataModel::uniform_grid(*block)) {
    const double spanSec =
      std::chrono::duration<double>(block->dt).count() * static_cast<double>(block->samples);
    const double baseSec = advancePlotClock(sid, block->t0, spanSec);

    for (const auto& column : block->columns)
      applyBlockColumn(column, *block, baseSec);

    for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
      if (it.value().enabled && m_activeMultiplots.value(it.key(), false))
        feedMultiplotBlockSweep(it.key(), *block, baseSec);

    if (!applyBlockValues(*block, block->samples - 1)) [[unlikely]]
      return;

    if (m_gpsValues.size() != groupWidgetCount(SerialStudio::DashboardGPS)) [[unlikely]]
      m_host.configureGpsSeries();

    updateGpsSeries(sid);

#ifdef BUILD_COMMERCIAL
    if (m_plotData3D.size() != groupWidgetCount(SerialStudio::DashboardPlot3D)) [[unlikely]]
      m_host.configurePlot3DSeries();

    if (m_waterfallValues.size() != datasetWidgetCount(SerialStudio::DashboardWaterfall))
      [[unlikely]]
      m_host.configureWaterfallSeries();

    updatePlot3DSeries(sid);
    updateWaterfallSeries(sid);
#endif

  } else {
    for (qsizetype i = 0; i < block->samples; ++i) {
      (void)advancePlotClock(sid, DataModel::sample_time(*block, i));
      if (!applyBlockValues(*block, i)) [[unlikely]]
        return;

      foldExtremes(sid);
      updateDataSeries(sid);
    }
  }

  m_updateRequired = true;
}

/**
 * @brief Propagates sample @p index of every column into its widget copies. Returns false when the
 *        push table no longer lines up with the block, which hands the source to the rebuild-once
 *        then quarantine path rather than writing values into the wrong widgets.
 */
bool UI::DashboardIngest::applyBlockValues(const DataModel::DataBlock& block, qsizetype index)
{
  const auto pit = m_valuePushes.constFind(block.sourceId);
  if (pit == m_valuePushes.cend()) [[unlikely]] {
    m_host.handleMissingDataset(m_sourceRawFrames.value(block.sourceId));
    return false;
  }

  const auto& table         = pit.value();
  const std::size_t entries = table.size();
  const std::size_t columns = block.columns.size();

  if (entries != columns) [[unlikely]] {
    m_host.handleMissingDataset(m_sourceRawFrames.value(block.sourceId));
    return false;
  }

  m_updateRetryInProgress = false;

  const auto slot = static_cast<std::size_t>(index);
  for (std::size_t c = 0; c < columns; ++c) {
    const auto& column = block.columns[c];
    const auto& push   = table[c];
    if (push.uniqueId != column.uniqueId) [[unlikely]] {
      m_host.handleMissingDataset(m_sourceRawFrames.value(block.sourceId));
      return false;
    }

    const bool numeric = DataModel::sample_is_numeric(column, index);
    for (auto* ptr : push.targets) {
      ptr->isNumeric    = numeric;
      ptr->numericValue = column.values[slot];
    }

    if (!column.hasText) {
      write_rendered_strings(push, column.values[slot]);
      continue;
    }

    const auto& string_targets = numeric ? push.stringTargets : push.targets;
    for (auto* ptr : string_targets)
      ptr->value = column.text[slot];
  }

  return true;
}

/**
 * @brief Applies one column of a uniform-grid block: samples into the plot/multiplot rings at
 *        t = baseSec + i * dtSec, into the sample-count rings, and the producer's FFT window into
 *        the FFT and waterfall series. The widget dataset copies are written once per block by
 *        applyBlockValues instead of once per column here (F9).
 */
void UI::DashboardIngest::applyBlockColumn(const DataModel::BlockColumn& column,
                                           const DataModel::DataBlock& block,
                                           double baseSec)
{
  const double dtSec = std::chrono::duration<double>(block.dt).count();
  const auto count   = static_cast<std::size_t>(block.samples);

  const auto ext_it = m_datasetExtremes.find(column.uniqueId);
  if (ext_it != m_datasetExtremes.end()) [[unlikely]] {
    auto& slot = ext_it.value();
    for (std::size_t i = 0; i < count; ++i) {
      const double value = column.values[i];
      if (!std::isfinite(value))
        continue;

      slot.min   = slot.valid ? qMin(slot.min, value) : value;
      slot.max   = slot.valid ? qMax(slot.max, value) : value;
      slot.valid = true;
    }
  }

  const StreamTargets& targets = streamTargetsFor(column.uniqueId);

  for (const int plotIndex : targets.plotIndexes) {
    if (!m_activePlots.value(plotIndex, false))
      continue;

    auto ringIt = m_plotTimeRings.find(plotIndex);
    if (ringIt == m_plotTimeRings.end()) [[unlikely]]
      continue;

    auto& ring = ringIt.value();
    for (std::size_t i = 0; i < count; ++i)
      ring.appendDecimated(baseSec + static_cast<double>(i) * dtSec, column.values[i]);

    feedPlotBlockSweep(plotIndex, column, block, baseSec);
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

    auto& ring = rings[curveIndex];
    for (std::size_t i = 0; i < count; ++i)
      ring.appendDecimated(baseSec + static_cast<double>(i) * dtSec, column.values[i]);
  }

  feedSampleRings(column, targets, block.sourceId, count);

  if (column.fftWindow.empty()) {
    feedFftFromSamples(column, targets, count);
    return;
  }

  for (const int fftIndex : targets.fftIndexes) {
    if (!m_activeFFTPlots.value(fftIndex, false))
      continue;

    if (fftIndex < 0 || fftIndex >= m_fftValues.size()) [[unlikely]]
      continue;

    auto& series = m_fftValues[fftIndex];
    series.clear();
    for (const double sample : column.fftWindow)
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
    for (const double sample : column.fftWindow)
      series.push(sample);
  }
#endif
}

/**
 * @brief Feeds the FFT and waterfall series from a column's raw samples. Used when the producer
 *        computed no window -- a replayed session, which has no stream worker in front of it. The
 *        series hold time-domain samples and transform themselves, so pushing samples is exactly
 *        what the frame lane does per frame.
 */
void UI::DashboardIngest::feedFftFromSamples(const DataModel::BlockColumn& column,
                                             const StreamTargets& targets,
                                             std::size_t count)
{
  for (const int fftIndex : targets.fftIndexes) {
    if (!m_activeFFTPlots.value(fftIndex, false))
      continue;

    if (fftIndex < 0 || fftIndex >= m_fftValues.size()) [[unlikely]]
      continue;

    auto& series = m_fftValues[fftIndex];
    for (std::size_t i = 0; i < count; ++i)
      series.push(column.values[i]);
  }

#ifdef BUILD_COMMERCIAL
  for (const int fallIndex : targets.waterfallIndexes) {
    if (!m_activeWaterfalls.value(fallIndex, false))
      continue;

    if (fallIndex < 0 || fallIndex >= m_waterfallValues.size()) [[unlikely]]
      continue;

    auto& series = m_waterfallValues[fallIndex];
    for (std::size_t i = 0; i < count; ++i)
      series.push(column.values[i]);
  }
#endif
}

/**
 * @brief Feeds one uniform-grid column's samples into the sample-count rings a Samples-axis plot,
 *        a dataset-X plot and a Samples-mode multiplot render from. Before spec 0075 only the
 *        irregular lane wrote them, so those widgets stayed blank on a stream source while the
 *        time plots beside them were live (F3).
 */
void UI::DashboardIngest::feedSampleRings(const DataModel::BlockColumn& column,
                                          const StreamTargets& targets,
                                          const int sourceId,
                                          const std::size_t count)
{
  for (const int index : targets.yLinePushIndexes) {
    const auto slot = static_cast<std::size_t>(index);
    if (slot >= m_yLinePushes.size()) [[unlikely]]
      continue;

    const LinePush& push = m_yLinePushes[slot];
    if (push_is_active(push, sourceId))
      push_ring_tail(*push.buf, column, count);
  }

  for (const int index : targets.xLinePushIndexes) {
    const auto slot = static_cast<std::size_t>(index);
    if (slot >= m_xLinePushes.size()) [[unlikely]]
      continue;

    const LinePush& push = m_xLinePushes[slot];
    if (push_is_active(push, sourceId))
      push_ring_tail(*push.buf, column, count);
  }

  for (const auto& [groupIndex, curveIndex] : targets.multiSampleIndexes) {
    const auto group = static_cast<std::size_t>(groupIndex);
    const auto curve = static_cast<std::size_t>(curveIndex);
    if (group >= m_multiplotPushes.size()) [[unlikely]]
      continue;

    const MultiPush& push = m_multiplotPushes[group];
    if (push.sourceId != sourceId || !*push.activeFlag)
      continue;

    if (curve >= push.samples.size()) [[unlikely]]
      continue;

    push_ring_tail(*push.samples[curve].first, column, count);
  }
}

/**
 * @brief Feeds one uniform-grid column's samples into a plot's sweep engine. The trigger steps per
 *        sample, so its resolution is the source's rather than the display's.
 */
void UI::DashboardIngest::feedPlotBlockSweep(int plotIndex,
                                             const DataModel::BlockColumn& column,
                                             const DataModel::DataBlock& block,
                                             double baseSec)
{
  auto sweepIt = m_plotSweep.find(plotIndex);
  if (sweepIt == m_plotSweep.end())
    return;

  DSP::SweepEngine& sweep = sweepIt.value();
  if (!sweep.enabled || sweep.back.empty())
    return;

  const double dtSec = std::chrono::duration<double>(block.dt).count();
  const auto count   = static_cast<std::size_t>(block.samples);

  for (std::size_t i = 0; i < count; ++i) {
    const double value = column.values[i];
    const double st    = sweep.advance(baseSec + static_cast<double>(i) * dtSec, value);
    if (st >= 0)
      sweep.back[0].appendDecimated(st, value);
  }
}

/**
 * @brief Feeds one multiplot's sweep engine from a uniform-grid block: the trigger curve alone
 *        drives advance(), every curve is appended at the time it returns, and curves pair by
 *        sample index because one source's columns share a block grid. A group this block feeds no
 *        curve of resolves to no trigger, which keeps frame-fed multiplots out of this path.
 */
void UI::DashboardIngest::feedMultiplotBlockSweep(int groupIndex,
                                                  const DataModel::DataBlock& block,
                                                  double baseSec)
{
  auto sweepIt = m_multiplotSweep.find(groupIndex);
  if (sweepIt == m_multiplotSweep.end())
    return;

  DSP::SweepEngine& sweep = sweepIt.value();
  if (!sweep.enabled || sweep.back.empty())
    return;

  m_streamSweepCurves.assign(sweep.back.size(), nullptr);
  for (const auto& column : block.columns) {
    for (const auto& [group, curve] : streamTargetsFor(column.uniqueId).multiplotCurves) {
      if (group != groupIndex || curve < 0)
        continue;

      if (static_cast<std::size_t>(curve) < m_streamSweepCurves.size())
        m_streamSweepCurves[static_cast<std::size_t>(curve)] = &column;
    }
  }

  const int last      = static_cast<int>(m_streamSweepCurves.size()) - 1;
  const int trigCurve = qBound(0, sweep.triggerCurve, last);
  const auto* trigger = m_streamSweepCurves[static_cast<std::size_t>(trigCurve)];
  if (!trigger)
    return;

  const double dtSec = std::chrono::duration<double>(block.dt).count();
  const auto count   = static_cast<std::size_t>(block.samples);

  for (std::size_t i = 0; i < count; ++i) {
    const double st = sweep.advance(baseSec + static_cast<double>(i) * dtSec, trigger->values[i]);
    if (st < 0)
      continue;

    for (std::size_t j = 0; j < m_streamSweepCurves.size(); ++j)
      if (m_streamSweepCurves[j])
        sweep.back[j].appendDecimated(st, m_streamSweepCurves[j]->values[i]);
  }
}

/**
 * @brief Lazily resolves (and caches) the widget indexes fed by one stream dataset. The cache
 *        holds indexes only -- ring pointers would dangle across a layout rebuild -- and is
 *        cleared with the push tables on every reconfigure.
 */
const UI::StreamTargets& UI::DashboardIngest::streamTargetsFor(int uniqueId)
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
    for (std::size_t c = 0; c < datasets.size(); ++c) {
      if (datasets[c].uniqueId != uniqueId)
        continue;

      targets.multiplotCurves.emplace_back(g, static_cast<int>(c));

      const auto slot = static_cast<std::size_t>(g);
      if (slot < m_multiplotPushes.size() && c < m_multiplotPushes[slot].samples.size())
        targets.multiSampleIndexes.emplace_back(g, static_cast<int>(c));
    }
  }

  for (std::size_t i = 0; i < m_yLinePushes.size(); ++i)
    if (m_yLinePushes[i].uniqueId == uniqueId)
      targets.yLinePushIndexes.push_back(static_cast<int>(i));

  for (std::size_t i = 0; i < m_xLinePushes.size(); ++i)
    if (m_xLinePushes[i].uniqueId == uniqueId)
      targets.xLinePushIndexes.push_back(static_cast<int>(i));

  return m_streamTargets.insert(uniqueId, std::move(targets)).value();
}

//--------------------------------------------------------------------------------------------------
// Plot clocks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Advances the per-source plot clock for one publish and returns the new forward-only
 *        display time. A uniform-grid block continues from the span the previous block published,
 *        never from the smoothed per-sample cadence: that cadence averages over block sizes, so it
 *        undershoots a long block (rewinding the rings) and ratchets past real time on a short one.
 */
double UI::DashboardIngest::advancePlotClock(int sourceId,
                                             const std::chrono::steady_clock::time_point& ts,
                                             const double blockSpanSec)
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
    clk.blockSpanSec    = 0;
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
  if (blockSpanSec > 0) {
    const double continued = clk.displayTimeSec + clk.blockSpanSec;
    const double blockNext = qMax(clk.relativeFrameTimeSec, continued);
    clk.blockSpanSec       = blockSpanSec;
    clk.displayTimeSec     = blockNext;
    m_plotDisplayTimeSec   = blockNext;
    return blockNext;
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
// Extreme-hold folding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Folds the just-propagated values of one source's extreme-hold datasets into their
 *        min/max slots (spec 0052). The table holds only opted-in datasets, so the common case
 *        is one failed hash lookup; non-finite and non-numeric samples never move the hold.
 */
void UI::DashboardIngest::foldExtremes(int sourceId)
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

//--------------------------------------------------------------------------------------------------
// Per-sample series updates (irregular lane)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates time-series data for all dashboard widgets that require historical tracking. A
 *        layout rebuild passes no source and refills the sample-count series only: time rings are
 *        stamped with the global display clock, which belongs to whichever source published last,
 *        so seeding every source's rings from it rewinds all but that one.
 */
void UI::DashboardIngest::updateDataSeries(int sourceId)
{
  SS_ASSERT_LOG(m_widgetCount > 0 || m_widgetMap.isEmpty());
  SS_ASSERT(!m_sourceRawFrames.isEmpty(), return);

  const int gpsCount   = groupWidgetCount(SerialStudio::DashboardGPS);
  const int fftCount   = datasetWidgetCount(SerialStudio::DashboardFFT);
  const int plotCount  = datasetWidgetCount(SerialStudio::DashboardPlot);
  const int multiCount = groupWidgetCount(SerialStudio::DashboardMultiPlot);
#ifdef BUILD_COMMERCIAL
  const int plot3DCount    = groupWidgetCount(SerialStudio::DashboardPlot3D);
  const int waterfallCount = datasetWidgetCount(SerialStudio::DashboardWaterfall);
#endif

  if (m_gpsValues.size() != gpsCount) [[unlikely]]
    m_host.configureGpsSeries();
  if (m_fftValues.size() != fftCount) [[unlikely]]
    m_host.configureFftSeries();
  if (m_pltValues.size() != plotCount) [[unlikely]]
    m_host.configureLineSeries();
  if (m_multipltValues.size() != multiCount) [[unlikely]]
    m_host.configureMultiLineSeries();
#ifdef BUILD_COMMERCIAL
  if (m_plotData3D.size() != plot3DCount) [[unlikely]]
    m_host.configurePlot3DSeries();
  if (m_waterfallValues.size() != waterfallCount) [[unlikely]]
    m_host.configureWaterfallSeries();
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

    if (sourceId >= 0) {
      feedMultiRings(p);
      feedMultiSweep(p);
    }

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
void UI::DashboardIngest::updateFftSeries(int sourceId)
{
  SS_ASSERT_LOG(static_cast<int>(m_fftPushes.size()) == m_fftValues.size());
  SS_ASSERT_LOG(m_activeFFTPlots.size() == m_fftValues.size());

  for (const auto& p : m_fftPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
  }
}

/**
 * @brief Updates GPS trajectory series for all GPS widgets.
 */
void UI::DashboardIngest::updateGpsSeries(int sourceId)
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
void UI::DashboardIngest::updatePlot3DSeries(int sourceId)
{
#ifdef BUILD_COMMERCIAL
  SS_ASSERT_LOG(static_cast<int>(m_plot3DPushes.size()) == m_plot3DRings.size());
  SS_ASSERT(m_points > 0, return);

  const auto maxPoints = static_cast<std::size_t>(m_points);
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
 * @brief Updates linear plot data series for all active plot widgets. A layout rebuild passes no
 *        source and refills the sample-count rings only: the time rings are stamped with the
 *        global display clock, which belongs to whichever source published last, so feeding them
 *        for every source rewinds the rings of all the others.
 */
void UI::DashboardIngest::updateLineSeries(int sourceId)
{
  // code-verify off
  // Debug-only layout parity check: widgetCount() runs two map lookups per call, per frame.
  Q_ASSERT(m_pltValues.size() == datasetWidgetCount(SerialStudio::DashboardPlot));
  Q_ASSERT(m_activePlots.size() == datasetWidgetCount(SerialStudio::DashboardPlot));
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

  if (sourceId < 0)
    return;

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

#ifdef BUILD_COMMERCIAL
/**
 * @brief Updates time-domain ring buffers feeding all active waterfall widgets.
 */
void UI::DashboardIngest::updateWaterfallSeries(int sourceId)
{
  SS_ASSERT_LOG(static_cast<int>(m_waterfallPushes.size()) == m_waterfallValues.size());
  SS_ASSERT_LOG(m_activeWaterfalls.size() == m_waterfallValues.size());

  for (const auto& p : m_waterfallPushes) {
    if (!*p.activeFlag)
      continue;

    if (sourceId >= 0 && p.sourceId != sourceId)
      continue;

    p.buf->push(*p.value);
  }
}

/**
 * @brief Arms or disarms the audio-recording tap on one FFT widget's ingest push. Taps are
 *        index-aligned with the FFT widget order and reset on every push-table rebuild, so a
 *        stale index can never fire against a re-indexed widget.
 */
void UI::DashboardIngest::setFftAudioTap(const int index, const bool enabled, const quint32 key)
{
  SS_ASSERT_LOG(static_cast<int>(m_fftPushes.size()) == m_fftValues.size());
  if (index < 0 || index >= static_cast<int>(m_fftPushes.size()))
    return;

  m_fftPushes[index].record     = enabled;
  m_fftPushes[index].sessionKey = key;
}

/**
 * @brief Arms or disarms the audio-recording tap on one waterfall widget's ingest push. Taps
 *        are index-aligned with the waterfall widget order and reset on every push-table
 *        rebuild, so a stale index can never fire against a re-indexed widget.
 */
void UI::DashboardIngest::setWaterfallAudioTap(const int index,
                                               const bool enabled,
                                               const quint32 key)
{
  SS_ASSERT_LOG(static_cast<int>(m_waterfallPushes.size()) == m_waterfallValues.size());
  if (index < 0 || index >= static_cast<int>(m_waterfallPushes.size()))
    return;

  m_waterfallPushes[index].record     = enabled;
  m_waterfallPushes[index].sessionKey = key;
}
#endif

//--------------------------------------------------------------------------------------------------
// Push-table construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every pre-resolved hotpath push table (their pointers follow the layout).
 */
void UI::DashboardIngest::clearPushTables()
{
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();
  m_fftPushes.clear();
  m_gpsPushes.clear();
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
 * @brief Resolves the GPS push table against the series the facade just allocated. Runs as the
 *        second pass of configureGpsSeries: the table holds raw pointers into m_gpsValues, whose
 *        addresses are stable only once that vector has stopped growing.
 */
void UI::DashboardIngest::buildGpsPushes()
{
  m_gpsPushes.clear();
  m_gpsPushes.shrink_to_fit();

  const int gpsCount = groupWidgetCount(SerialStudio::DashboardGPS);
  SS_ASSERT(m_gpsValues.size() == gpsCount, return);

  m_gpsPushes.reserve(static_cast<std::size_t>(gpsCount));
  for (int i = 0; i < gpsCount; ++i) {
    const auto& group = m_host.getGroupWidget(SerialStudio::DashboardGPS, i);

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
 * @brief Resolves the FFT push table against the buffers the facade just allocated (second pass
 *        of configureFftSeries; see buildGpsPushes for why the pass is split).
 */
void UI::DashboardIngest::buildFftPushes()
{
  m_fftPushes.clear();
  m_fftPushes.shrink_to_fit();

  const int fftCount = datasetWidgetCount(SerialStudio::DashboardFFT);
  SS_ASSERT(m_fftValues.size() == fftCount, return);

  m_fftPushes.reserve(static_cast<std::size_t>(fftCount));
  for (int i = 0; i < fftCount; ++i) {
    const auto& dataset = m_host.getDatasetWidget(SerialStudio::DashboardFFT, i);

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
 * @brief Resolves the waterfall push table against the buffers the facade just allocated (second
 *        pass of configureWaterfallSeries; see buildGpsPushes).
 */
void UI::DashboardIngest::buildWaterfallPushes()
{
  m_waterfallPushes.clear();
  m_waterfallPushes.shrink_to_fit();

  const int waterfallCount = datasetWidgetCount(SerialStudio::DashboardWaterfall);
  SS_ASSERT(m_waterfallValues.size() == waterfallCount, return);

  m_waterfallPushes.reserve(static_cast<std::size_t>(waterfallCount));
  for (int i = 0; i < waterfallCount; ++i) {
    const auto& dataset = m_host.getDatasetWidget(SerialStudio::DashboardWaterfall, i);

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

/**
 * @brief Resolves the 3D push table against the rings the facade just allocated (second pass of
 *        configurePlot3DSeries; see buildGpsPushes).
 */
void UI::DashboardIngest::buildPlot3DPushes()
{
  m_plot3DPushes.clear();
  m_plot3DPushes.shrink_to_fit();

  const int plot3DCount = groupWidgetCount(SerialStudio::DashboardPlot3D);
  SS_ASSERT(m_plot3DRings.size() == plot3DCount, return);

  m_plot3DPushes.reserve(static_cast<std::size_t>(plot3DCount));
  for (int i = 0; i < plot3DCount; ++i) {
    const auto& group = m_host.getGroupWidget(SerialStudio::DashboardPlot3D, i);

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
 * @brief Resolves the per-plot y/x/time push tables from the configured buffers, deduplicating
 *        shared Y and X sources so each buffer is pushed once per frame.
 */
void UI::DashboardIngest::buildLinePushes()
{
  m_yLinePushes.clear();
  m_xLinePushes.clear();
  m_timePushes.clear();
  m_streamTargets.clear();

  QHash<int, std::size_t> yByUid;
  QHash<int, std::size_t> xByXAxisId;
  for (int i = 0; i < datasetWidgetCount(SerialStudio::DashboardPlot); ++i) {
    const auto& yDataset = m_host.getDatasetWidget(SerialStudio::DashboardPlot, i);
    const LinePush::Consumer consumer{yDataset.sourceId, &m_activePlots[i]};

    if (m_host.useTimeXAxis(yDataset)) {
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
        push.buf      = &yIt.value();
        push.value    = &yDataset.numericValue;
        push.uniqueId = yDataset.uniqueId;
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
      push.buf      = &xBuf;
      push.value    = &xDsIt.value().numericValue;
      push.uniqueId = xAxisId;
      xByXAxisId.insert(xAxisId, m_xLinePushes.size());
      m_xLinePushes.push_back(std::move(push));
    } else {
      m_xLinePushes[cacheIt.value()].consumers.push_back(consumer);
    }
  }
}

/**
 * @brief Resolves the per-tick multiplot push table from the configured buffers.
 */
void UI::DashboardIngest::buildMultiplotPushes()
{
  m_multiplotPushes.clear();
  m_multiplotPushes.shrink_to_fit();
  m_streamTargets.clear();

  const int multiCount = groupWidgetCount(SerialStudio::DashboardMultiPlot);
  m_multiplotPushes.reserve(static_cast<std::size_t>(multiCount));

  for (int i = 0; i < multiCount; ++i) {
    const auto& group = m_host.getGroupWidget(SerialStudio::DashboardMultiPlot, i);

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
