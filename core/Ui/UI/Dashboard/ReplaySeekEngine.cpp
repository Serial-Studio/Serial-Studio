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

#include "UI/Dashboard/ReplaySeekEngine.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the plot stores owned by UI::Dashboard; the engine allocates no state of its own.
 */
UI::ReplaySeekEngine::ReplaySeekEngine(const ReplaySeekBindings& bindings)
  : m_xAxisData(bindings.xAxisData)
  , m_yAxisData(bindings.yAxisData)
  , m_plotTimeRings(bindings.plotTimeRings)
  , m_multiplotTimeRings(bindings.multiplotTimeRings)
  , m_multiplotValues(bindings.multiplotValues)
  , m_datasets(bindings.datasets)
  , m_widgetGroups(bindings.widgetGroups)
  , m_widgetDatasets(bindings.widgetDatasets)
{}

//--------------------------------------------------------------------------------------------------
// Widget-model lookups
//--------------------------------------------------------------------------------------------------

/**
 * @brief Packs a (sourceId, uniqueId) pair into one hash key. Both the replay window and the ring
 *        snapshot key on it: uniqueIds repeat across sources, so a sourceId-free key collapses two
 *        plots onto one entry and the second restore move-assigns a moved-from (null-buffer) ring.
 */
qint64 UI::ReplaySeekEngine::seekKey(int sourceId, int uniqueId) noexcept
{
  return (static_cast<qint64>(sourceId) << 32) | static_cast<quint32>(uniqueId);
}

/**
 * @brief Number of single-plot widgets in the live layout.
 */
int UI::ReplaySeekEngine::plotCount() const
{
  const auto it = m_widgetDatasets.constFind(SerialStudio::DashboardPlot);
  return it != m_widgetDatasets.cend() ? it->count() : 0;
}

/**
 * @brief Number of multiplot widgets in the live layout.
 */
int UI::ReplaySeekEngine::multiplotCount() const
{
  const auto it = m_widgetGroups.constFind(SerialStudio::DashboardMultiPlot);
  return it != m_widgetGroups.cend() ? it->count() : 0;
}

/**
 * @brief Plot dataset at @p index, or an empty dataset when the layout changed under the caller.
 */
const DataModel::Dataset& UI::ReplaySeekEngine::plotDataset(const int index) const
{
  static const DataModel::Dataset kEmpty;
  const auto it = m_widgetDatasets.constFind(SerialStudio::DashboardPlot);
  if (it == m_widgetDatasets.cend()) [[unlikely]]
    return kEmpty;

  if (index < 0 || index >= it->size()) [[unlikely]]
    return kEmpty;

  return it->at(index);
}

/**
 * @brief Multiplot group at @p index, or an empty group when the layout changed under the caller.
 */
const DataModel::Group& UI::ReplaySeekEngine::multiplotGroup(const int index) const
{
  static const DataModel::Group kEmpty;
  const auto it = m_widgetGroups.constFind(SerialStudio::DashboardMultiPlot);
  if (it == m_widgetGroups.cend()) [[unlikely]]
    return kEmpty;

  if (index < 0 || index >= it->size()) [[unlikely]]
    return kEmpty;

  return it->at(index);
}

//--------------------------------------------------------------------------------------------------
// Replay seek bulk fill (spec 0020)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Lists every (sourceId, uniqueId) pair the plot widgets consume, including dataset-X
 *        axis sources and multiplot curves, so a replay player knows which columns to sample
 *        for bulkLoadPlotWindow().
 */
QList<std::pair<int, int>> UI::ReplaySeekEngine::seekSeries() const
{
  QList<std::pair<int, int>> out;
  QSet<qint64> seen;

  auto add = [&](int sourceId, int uniqueId) {
    const qint64 key = seekKey(sourceId, uniqueId);
    if (seen.contains(key))
      return;

    seen.insert(key);
    out.append({sourceId, uniqueId});
  };

  const int plots = plotCount();
  for (int i = 0; i < plots; ++i) {
    const auto& ds = plotDataset(i);
    add(ds.sourceId, ds.uniqueId);

    if (ds.xAxisId == DataModel::kXAxisTime)
      continue;

    const auto xIt = m_datasets.constFind(ds.xAxisId);
    if (xIt != m_datasets.constEnd())
      add(xIt.value().sourceId, ds.xAxisId);
  }

  const int multis = multiplotCount();
  for (int i = 0; i < multis; ++i) {
    const auto& group = multiplotGroup(i);
    for (const auto& ds : group.datasets)
      add(group.sourceId, ds.uniqueId);
  }

  return out;
}

/**
 * @brief Fills one single-plot widget's ring from the seek window: decimating TimeRing for
 *        time plots, y/x sample rings otherwise. @p filled dedups shared sample rings.
 */
void UI::ReplaySeekEngine::fillSeekPlotSingle(int index,
                                              const QVector<double>& timesSec,
                                              const QHash<qint64, QVector<double>>& series,
                                              double timeOffset,
                                              QSet<const DSP::AxisData*>& filled)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT(index < plotCount(), return);

  const auto& ds     = plotDataset(index);
  const auto& values = series.value(seekKey(ds.sourceId, ds.uniqueId));
  const int count    = qMin(timesSec.size(), values.size());

  if (ds.xAxisId == DataModel::kXAxisTime) {
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

  const auto& xValues = series.value(seekKey(xDsIt.value().sourceId, ds.xAxisId));
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
void UI::ReplaySeekEngine::fillSeekPlotMulti(int index,
                                             const QVector<double>& timesSec,
                                             const QHash<qint64, QVector<double>>& series,
                                             double timeOffset)
{
  SS_ASSERT(index >= 0, return);
  SS_ASSERT(index < multiplotCount(), return);

  const auto& group = multiplotGroup(index);

  const auto rIt = m_multiplotTimeRings.find(index);
  if (rIt != m_multiplotTimeRings.end()) {
    auto& rings        = rIt.value();
    const size_t count = std::min(group.datasets.size(), rings.size());
    for (size_t j = 0; j < count; ++j) {
      const auto& ds     = group.datasets[j];
      const auto& values = series.value(seekKey(group.sourceId, ds.uniqueId));
      const int n        = qMin(timesSec.size(), values.size());
      rings[j].clear();
      for (int k = 0; k < n; ++k)
        rings[j].appendDecimated(timesSec[k] + timeOffset, values[k]);
    }

    return;
  }

  if (index >= m_multiplotValues.size()) [[unlikely]]
    return;

  auto& multiSeries  = m_multiplotValues[index];
  const size_t count = std::min(group.datasets.size(), multiSeries.y.size());
  for (size_t j = 0; j < count; ++j) {
    const auto& ds     = group.datasets[j];
    const auto& values = series.value(seekKey(group.sourceId, ds.uniqueId));
    const int n        = qMin(timesSec.size(), values.size());
    multiSeries.y[j].clear();
    for (int k = 0; k < n; ++k)
      multiSeries.y[j].push(values[k]);
  }
}

/**
 * @brief Rebuilds every plot ring from a replay seek window (spec 0020): ascending recorded
 *        seconds + series keyed by seekKey, normalized to end at 0 so the decimation grid and
 *        later live appends stay monotonic. Writes rings only; the caller resets the sweep
 *        states and the plot clocks, which is what re-anchors play-after-scrub.
 */
bool UI::ReplaySeekEngine::bulkLoadPlotWindow(const QVector<double>& timesSec,
                                              const QHash<qint64, QVector<double>>& series)
{
  // code-verify off
  // Debug-only ordering check: is_sorted is O(n) over the whole seek window, so a release
  // evaluation would walk every sample on every scrub.
  Q_ASSERT(std::is_sorted(timesSec.cbegin(), timesSec.cend()));
  // code-verify on

  if (timesSec.isEmpty()) [[unlikely]]
    return false;

  const double timeOffset = -timesSec.last();

  QSet<const DSP::AxisData*> filled;
  const int plots = plotCount();
  for (int i = 0; i < plots; ++i)
    fillSeekPlotSingle(i, timesSec, series, timeOffset, filled);

  const int multis = multiplotCount();
  for (int i = 0; i < multis; ++i)
    fillSeekPlotMulti(i, timesSec, series, timeOffset);

  return true;
}

//--------------------------------------------------------------------------------------------------
// Time-ring snapshot / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots plot time-ring contents keyed by (sourceId, dataset uniqueId).
 */
QHash<qint64, DSP::EnvelopeRing> UI::ReplaySeekEngine::snapshotPlotTimeRings() const
{
  QHash<qint64, DSP::EnvelopeRing> out;
  const int n = plotCount();
  for (int i = 0; i < n; ++i) {
    const auto it = m_plotTimeRings.constFind(i);
    if (it == m_plotTimeRings.cend())
      continue;

    const auto& d = plotDataset(i);
    out.insert(seekKey(d.sourceId, d.uniqueId), it.value());
  }

  return out;
}

/**
 * @brief Snapshots multiplot time-ring contents keyed by (sourceId, group uniqueId).
 */
QHash<qint64, std::vector<DSP::EnvelopeRing>> UI::ReplaySeekEngine::snapshotMultiplotTimeRings()
  const
{
  QHash<qint64, std::vector<DSP::EnvelopeRing>> out;
  const int n = multiplotCount();
  for (int i = 0; i < n; ++i) {
    const auto it = m_multiplotTimeRings.constFind(i);
    if (it == m_multiplotTimeRings.cend())
      continue;

    const auto& g = multiplotGroup(i);
    out.insert(seekKey(g.sourceId, g.uniqueId), it.value());
  }

  return out;
}

/**
 * @brief Replays a saved ring's level-0 samples into @p target via appendDecimated, which rebuilds
 *        the coarse levels as a side effect. Used when the new ring has a different capacity /
 *        interval than the saved one.
 */
static void replayTimeRing(const DSP::EnvelopeRing& saved, DSP::EnvelopeRing& target)
{
  const std::size_t n = std::min(saved.level0.time.size(), saved.level0.value.size());
  for (std::size_t k = 0; k < n; ++k)
    target.appendDecimated(saved.level0.time[k], saved.level0.value[k]);
}

/**
 * @brief Restores saved plot rings into the currently configured widget slots. Splices when
 *        the new ring shape matches the saved one, replays through appendDecimated otherwise.
 */
void UI::ReplaySeekEngine::restorePlotTimeRings(QHash<qint64, DSP::EnvelopeRing>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = plotCount();
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_plotTimeRings.find(i);
    if (ringIt == m_plotTimeRings.end())
      continue;

    const auto& d = plotDataset(i);
    auto savedIt  = snapshot.find(seekKey(d.sourceId, d.uniqueId));
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();
    if (kept.level0.time.raw() == nullptr)
      continue;

    if (live.level0.time.capacity() == kept.level0.time.capacity()
        && qFuzzyCompare(live.level0.interval, kept.level0.interval))
      live = std::move(kept);
    else
      replayTimeRing(kept, live);

    snapshot.erase(savedIt);
  }
}

/**
 * @brief Restores saved multiplot rings; matches by group uniqueId and per-curve shape.
 */
void UI::ReplaySeekEngine::restoreMultiplotTimeRings(
  QHash<qint64, std::vector<DSP::EnvelopeRing>>& snapshot)
{
  if (snapshot.isEmpty())
    return;

  const int n = multiplotCount();
  for (int i = 0; i < n; ++i) {
    auto ringIt = m_multiplotTimeRings.find(i);
    if (ringIt == m_multiplotTimeRings.end())
      continue;

    const auto& g = multiplotGroup(i);
    auto savedIt  = snapshot.find(seekKey(g.sourceId, g.uniqueId));
    if (savedIt == snapshot.end())
      continue;

    auto& live = ringIt.value();
    auto& kept = savedIt.value();

    const std::size_t count = std::min(live.size(), kept.size());
    for (std::size_t j = 0; j < count; ++j) {
      if (kept[j].level0.time.raw() == nullptr)
        continue;

      if (live[j].level0.time.capacity() == kept[j].level0.time.capacity()
          && qFuzzyCompare(live[j].level0.interval, kept[j].level0.interval))
        live[j] = std::move(kept[j]);
      else
        replayTimeRing(kept[j], live[j]);
    }

    snapshot.erase(savedIt);
  }
}
