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

#include "UI/Dashboard/PlotControlBank.h"

#include <algorithm>

#include "Core/SSAssert.h"
#include "UI/Dashboard/TimeRingSizing.h"

#ifdef BUILD_COMMERCIAL
#  include "Core/Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the run-flag and sweep maps owned by UI::Dashboard; holds no state of its own.
 */
UI::PlotControlBank::PlotControlBank(const PlotControlBindings& bindings)
  : m_activePlots(bindings.activePlots)
  , m_activeFFTPlots(bindings.activeFFTPlots)
  , m_activeMultiplots(bindings.activeMultiplots)
#ifdef BUILD_COMMERCIAL
  , m_activeWaterfalls(bindings.activeWaterfalls)
#endif
  , m_plotSweep(bindings.plotSweep)
  , m_multiplotSweep(bindings.multiplotSweep)
  , m_plotTimeRings(bindings.plotTimeRings)
  , m_multiplotTimeRings(bindings.multiplotTimeRings)
  , m_plotTimeRange(bindings.plotTimeRange)
  , m_plotBudget(bindings.plotBudget)
{}

//--------------------------------------------------------------------------------------------------
// Run-state queries and toggles
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether a plot is currently active.
 */
bool UI::PlotControlBank::plotRunning(const int index) const
{
  return m_activePlots.value(index, false);
}

/**
 * @brief Checks whether an FFT plot is currently active.
 */
bool UI::PlotControlBank::fftPlotRunning(const int index) const
{
  return m_activeFFTPlots.value(index, false);
}

/**
 * @brief Checks whether a multiplot is currently active.
 */
bool UI::PlotControlBank::multiplotRunning(const int index) const
{
  return m_activeMultiplots.value(index, false);
}

/**
 * @brief Sets the active state of a plot; unknown indexes are ignored so a stale widget id
 *        never inserts a flag the push tables did not resolve a pointer to.
 */
void UI::PlotControlBank::setPlotRunning(const int index, const bool enabled)
{
  auto it = m_activePlots.find(index);
  if (it != m_activePlots.end())
    it.value() = enabled;
}

/**
 * @brief Sets the active state of an FFT plot.
 */
void UI::PlotControlBank::setFFTPlotRunning(const int index, const bool enabled)
{
  auto it = m_activeFFTPlots.find(index);
  if (it != m_activeFFTPlots.end())
    it.value() = enabled;
}

/**
 * @brief Sets the active state of a multiplot.
 */
void UI::PlotControlBank::setMultiplotRunning(const int index, const bool enabled)
{
  auto it = m_activeMultiplots.find(index);
  if (it != m_activeMultiplots.end())
    it.value() = enabled;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Checks whether a waterfall plot is currently active.
 */
bool UI::PlotControlBank::waterfallRunning(const int index) const
{
  return m_activeWaterfalls.value(index, false);
}

/**
 * @brief Sets the active state of a waterfall plot.
 */
void UI::PlotControlBank::setWaterfallRunning(const int index, const bool enabled)
{
  auto it = m_activeWaterfalls.find(index);
  if (it != m_activeWaterfalls.end())
    it.value() = enabled;
}
#endif

//--------------------------------------------------------------------------------------------------
// Sweep / trigger control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the sweep/trigger engine for a time-axis plot widget.
 */
const DSP::SweepEngine& UI::PlotControlBank::plotSweep(const int index) const
{
  const auto it = m_plotSweep.constFind(index);
  if (it == m_plotSweep.cend()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Returns the sweep/trigger engine for a time-axis multiplot widget.
 */
const DSP::SweepEngine& UI::PlotControlBank::multiplotSweep(const int index) const
{
  const auto it = m_multiplotSweep.constFind(index);
  if (it == m_multiplotSweep.cend()) [[unlikely]] {
    static const DSP::SweepEngine kEmpty{};
    return kEmpty;
  }

  return it.value();
}

/**
 * @brief Configures sweep/trigger mode for a plot; gated to commercial tiers.
 */
void UI::PlotControlBank::setPlotSweep(const int index,
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
  Q_UNUSED(enabled)
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, 0);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    shapeOrRelease(engine, ok, plotHistory(index), 1);
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Configures sweep/trigger mode for a multiplot; gated to commercial tiers.
 */
void UI::PlotControlBank::setMultiplotSweep(const int index,
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
  Q_UNUSED(enabled)
  const bool ok = false;
#endif

  auto& engine = it.value();
  engine.setTrigger(level, edge, mode, holdoff, triggerCurve);
  engine.setTimebase(timebase);
  if (engine.enabled != ok) {
    int curves          = 0;
    const auto* densest = multiplotHistory(index, curves);
    shapeOrRelease(engine, ok, densest, curves);
    engine.enabled = ok;
    engine.resetState();
  }
}

/**
 * @brief Re-arms a single-shot plot sweep capture.
 */
void UI::PlotControlBank::armPlotSweep(const int index)
{
  auto it = m_plotSweep.find(index);
  if (it != m_plotSweep.end())
    it.value().arm();
}

/**
 * @brief Re-arms a single-shot multiplot sweep capture.
 */
void UI::PlotControlBank::armMultiplotSweep(const int index)
{
  auto it = m_multiplotSweep.find(index);
  if (it != m_multiplotSweep.end())
    it.value().arm();
}

/**
 * @brief Sets how many completed sweeps a plot retains (spec 0061; 0 = off, byte-budget clamped).
 */
void UI::PlotControlBank::setPlotSweepRetention(const int index, const int count)
{
  auto it = m_plotSweep.find(index);
  if (it == m_plotSweep.end())
    return;

  const std::size_t before = it.value().bytes();
  it.value().setSegmentRetention(count);
  m_plotBudget.charge(before, it.value().bytes());
}

/**
 * @brief Drops the capture state of every sweep engine, keeping their trigger configuration.
 *        Called wherever the timeline the engines advance against is rewritten.
 */
void UI::PlotControlBank::resetSweepStates()
{
  for (auto it = m_plotSweep.begin(); it != m_plotSweep.end(); ++it)
    it.value().resetState();

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it)
    it.value().resetState();
}

/**
 * @brief Disarms every sweep engine and drops its capture state (spec 0092): the entitlement
 *        that allowed the sweep is gone, and the saved-config restore path must not resurrect
 *        it on the next reconfigure.
 */
void UI::PlotControlBank::disableSweeps()
{
  for (auto it = m_plotSweep.begin(); it != m_plotSweep.end(); ++it) {
    if (it.value().enabled) {
      it.value().enabled = false;
      releaseSweep(it.value());
    }
  }

  for (auto it = m_multiplotSweep.begin(); it != m_multiplotSweep.end(); ++it) {
    if (it.value().enabled) {
      it.value().enabled = false;
      releaseSweep(it.value());
    }
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured plot engines.
 */
void UI::PlotControlBank::restorePlotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_plotSweep.find(it.key());
    if (live == m_plotSweep.end())
      continue;

    const auto& src = it.value();
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    shapeOrRelease(*live, src.enabled, plotHistory(it.key()), 1);
    live->enabled = src.enabled;
    live->takeSegmentsFrom(src);
  }
}

/**
 * @brief Re-applies saved sweep trigger settings onto freshly configured multiplot engines.
 */
void UI::PlotControlBank::restoreMultiplotSweepConfig(const QMap<int, DSP::SweepEngine>& saved)
{
  for (auto it = saved.begin(); it != saved.end(); ++it) {
    auto live = m_multiplotSweep.find(it.key());
    if (live == m_multiplotSweep.end())
      continue;

    const auto& src     = it.value();
    int curves          = 0;
    const auto* densest = multiplotHistory(it.key(), curves);
    live->setTrigger(src.level, src.edge, src.mode, src.holdoffSec, src.triggerCurve);
    live->setTimebase(src.timebaseSec);
    shapeOrRelease(*live, src.enabled, densest, curves);
    live->enabled = src.enabled;
    live->takeSegmentsFrom(src);
  }
}

//--------------------------------------------------------------------------------------------------
// Lazy sweep storage (spec 0097 R4)
//--------------------------------------------------------------------------------------------------

/**
 * @brief History ring of the plot at @p index, or nullptr when the layout has none.
 */
const DSP::EnvelopeRing* UI::PlotControlBank::plotHistory(const int index) const
{
  const auto it = m_plotTimeRings.constFind(index);
  return it != m_plotTimeRings.cend() ? &it.value() : nullptr;
}

/**
 * @brief Densest history ring of the multiplot at @p index (its curve count into @p curves), or
 *        nullptr when the layout has none.
 */
const DSP::EnvelopeRing* UI::PlotControlBank::multiplotHistory(const int index, int& curves) const
{
  curves        = 0;
  const auto it = m_multiplotTimeRings.constFind(index);
  if (it == m_multiplotTimeRings.cend() || it.value().empty())
    return nullptr;

  const auto& rings = it.value();
  curves            = static_cast<int>(rings.size());
  return &*std::max_element(rings.cbegin(), rings.cend(), [](const auto& a, const auto& b) {
    return a.level0.time.capacity() < b.level0.time.capacity();
  });
}

/**
 * @brief Frees @p engine's ring storage and returns its bytes to the plot-history budget; the
 *        trigger configuration stays.
 */
void UI::PlotControlBank::releaseSweep(DSP::SweepEngine& engine)
{
  SS_ASSERT_LOG(m_plotBudget.limit > 0);
  SS_ASSERT_LOG(m_plotTimeRange > 0.0);

  m_plotBudget.charge(engine.bytes(), 0);
  engine.release();
}

/**
 * @brief Allocates @p engine at the density of @p densest when @p on (and not yet allocated), or
 *        releases it when off, keeping the plot-history budget in step. A sweep that is off holds
 *        no ring storage; the trigger configuration survives either way.
 */
void UI::PlotControlBank::shapeOrRelease(DSP::SweepEngine& engine,
                                         const bool on,
                                         const DSP::EnvelopeRing* densest,
                                         const int curves)
{
  SS_ASSERT(!on || curves >= 0, return);
  SS_ASSERT(m_plotTimeRange > 0.0, return);

  if (!on) {
    releaseSweep(engine);
    return;
  }

  if (densest && curves > 0 && !engine.allocated())
    TimeRingSizing::shapeSweep(engine, curves, *densest, m_plotTimeRange, m_plotBudget);
}
