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

#pragma once

#include <cstddef>
#include <QMap>
#include <QVector>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DSP.h"

namespace UI {

/**
 * @brief The plot stores the ledger accounts for, bound by reference from UI::Dashboard, plus the
 *        widget buckets that name each curve by its dataset uniqueId.
 */
struct PlotMemoryBindings {
  const QMap<int, DSP::EnvelopeRing>& plotTimeRings;
  const QMap<int, std::vector<DSP::EnvelopeRing>>& multiplotTimeRings;
  const QMap<int, DSP::SweepEngine>& plotSweep;
  const QMap<int, DSP::SweepEngine>& multiplotSweep;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& widgetGroups;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& widgetDatasets;
};

/**
 * @brief One time-plot curve that stopped keeping every sample (spec 0097 R8).
 */
struct PlotMemoryCurve {
  int uniqueId;
  double retainedRateHz;
  std::size_t bytes;
  DSP::RingLimit limit;
};

/**
 * @brief On-demand plot-memory report (spec 0097 R8): history and sweep bytes, the rail and
 *        whether it binds, and the curves that are no longer exact.
 */
struct PlotMemoryReport {
  std::size_t historyBytes;
  std::size_t railBytes;
  bool railBinding;
  int timeCurves;
  int inexactCurves;
  QVector<PlotMemoryCurve> inexact;
};

/**
 * @brief Owns the plot-history byte budget (spec 0097 R7) and answers the memory report (R8).
 *        Rings and sweeps grow against budget() at block or toggle rate; refresh() recounts it
 *        from the live stores after anything replaces rings wholesale. Never on the per-frame path.
 */
class PlotMemoryLedger {
public:
  explicit PlotMemoryLedger(const PlotMemoryBindings& bindings);
  PlotMemoryLedger(PlotMemoryLedger&&)                 = delete;
  PlotMemoryLedger(const PlotMemoryLedger&)            = delete;
  PlotMemoryLedger& operator=(PlotMemoryLedger&&)      = delete;
  PlotMemoryLedger& operator=(const PlotMemoryLedger&) = delete;

  [[nodiscard]] DSP::RingBudget& budget() noexcept { return m_budget; }

  [[nodiscard]] PlotMemoryReport report() const;

  void refresh();

private:
  [[nodiscard]] int plotUniqueId(int index) const;
  [[nodiscard]] int multiplotUniqueId(int index, std::size_t curve) const;

private:
  const QMap<int, DSP::EnvelopeRing>& m_plotTimeRings;
  const QMap<int, std::vector<DSP::EnvelopeRing>>& m_multiplotTimeRings;
  const QMap<int, DSP::SweepEngine>& m_plotSweep;
  const QMap<int, DSP::SweepEngine>& m_multiplotSweep;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& m_widgetGroups;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& m_widgetDatasets;
  DSP::RingBudget m_budget;
};

}  // namespace UI
