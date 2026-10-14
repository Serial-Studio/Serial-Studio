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

#include "UI/Dashboard/PlotMemoryLedger.h"

#include <utility>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Hard ceiling on all time-plot history (spec 0097 R7); binds only in pathological projects
constexpr std::size_t kPlotHistoryRailBytes = 512u * 1024u * 1024u;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the plot stores owned by UI::Dashboard; the ledger owns only the budget.
 */
UI::PlotMemoryLedger::PlotMemoryLedger(const PlotMemoryBindings& bindings)
  : m_plotTimeRings(bindings.plotTimeRings)
  , m_multiplotTimeRings(bindings.multiplotTimeRings)
  , m_plotSweep(bindings.plotSweep)
  , m_multiplotSweep(bindings.multiplotSweep)
  , m_widgetGroups(bindings.widgetGroups)
  , m_widgetDatasets(bindings.widgetDatasets)
  , m_budget(kPlotHistoryRailBytes)
{}

//--------------------------------------------------------------------------------------------------
// Accounting
//--------------------------------------------------------------------------------------------------

/**
 * @brief On-demand report: walks every time ring and sweep engine once, O(curves). Pulled by the
 *        API, never pushed per frame.
 */
UI::PlotMemoryReport UI::PlotMemoryLedger::report() const
{
  SS_ASSERT_LOG(m_budget.limit > 0);
  SS_ASSERT_LOG(m_budget.used <= m_budget.limit || m_budget.binding);

  PlotMemoryReport out{};
  out.railBytes   = m_budget.limit;
  out.railBinding = m_budget.binding;

  const auto addRing = [&out](const DSP::EnvelopeRing& ring, const int uniqueId) {
    out.historyBytes += ring.bytes();
    ++out.timeCurves;
    if (ring.exact())
      return;

    ++out.inexactCurves;
    out.inexact.append({uniqueId, ring.retainedRateHz(), ring.bytes(), ring.limit});
  };

  for (auto it = m_plotTimeRings.cbegin(); it != m_plotTimeRings.cend(); ++it)
    addRing(it.value(), plotUniqueId(it.key()));

  for (auto it = m_multiplotTimeRings.cbegin(); it != m_multiplotTimeRings.cend(); ++it)
    for (std::size_t j = 0; j < it.value().size(); ++j)
      addRing(it.value()[j], multiplotUniqueId(it.key(), j));

  for (const auto& engine : m_plotSweep)
    out.historyBytes += engine.bytes();

  for (const auto& engine : m_multiplotSweep)
    out.historyBytes += engine.bytes();

  return out;
}

/**
 * @brief Recounts the budget from the live rings and sweep engines: every rebuild, restore and
 *        bulk load replaces rings wholesale, so the total is recounted rather than patched.
 *        Binding follows any ring the rail refused, or a total that ran past the rail.
 */
void UI::PlotMemoryLedger::refresh()
{
  SS_ASSERT_LOG(m_budget.limit > 0);

  std::size_t used = 0;
  bool binding     = false;
  const auto count = [&used, &binding](const DSP::EnvelopeRing& ring) {
    used    += ring.bytes();
    binding  = binding || ring.limit == DSP::RingLimit::Rail;
  };

  for (const auto& ring : std::as_const(m_plotTimeRings))
    count(ring);

  for (const auto& rings : std::as_const(m_multiplotTimeRings))
    for (const auto& ring : rings)
      count(ring);

  for (const auto& engine : std::as_const(m_plotSweep))
    used += engine.bytes();

  for (const auto& engine : std::as_const(m_multiplotSweep))
    used += engine.bytes();

  m_budget.used    = used;
  m_budget.binding = binding || used > m_budget.limit;
}

//--------------------------------------------------------------------------------------------------
// Curve identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dataset uniqueId of the single plot at @p index, or -1 when the layout changed.
 */
int UI::PlotMemoryLedger::plotUniqueId(const int index) const
{
  const auto it = m_widgetDatasets.constFind(SerialStudio::DashboardPlot);
  if (it == m_widgetDatasets.cend() || index < 0 || index >= it->size())
    return -1;

  return it->at(index).uniqueId;
}

/**
 * @brief Dataset uniqueId of curve @p curve in the multiplot at @p index, or -1.
 */
int UI::PlotMemoryLedger::multiplotUniqueId(const int index, const std::size_t curve) const
{
  const auto it = m_widgetGroups.constFind(SerialStudio::DashboardMultiPlot);
  if (it == m_widgetGroups.cend() || index < 0 || index >= it->size())
    return -1;

  const auto& datasets = it->at(index).datasets;
  return curve < datasets.size() ? datasets[curve].uniqueId : -1;
}
