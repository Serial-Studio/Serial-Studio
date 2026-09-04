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

#include <QHash>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>
#include <utility>
#include <vector>

#include "DataModel/Frame.h"
#include "DSP.h"
#include "SerialStudio.h"

namespace UI {

/**
 * @brief The plot stores a replay seek rewrites, bound by reference from UI::Dashboard: the
 *        decimating rings, the XY sample rings and the multiplot series share one timeline, so a
 *        seek or a layout rebuild must move them together. The plot clocks are deliberately NOT
 *        here: they stay Dashboard state, cleared only by Dashboard::resetPlotClocks().
 */
struct ReplaySeekBindings {
  QMap<int, DSP::AxisData>& xAxisData;
  QMap<int, DSP::AxisData>& yAxisData;
  QMap<int, DSP::EnvelopeRing>& plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>>& multiplotTimeRings;
  QVector<DSP::MultiLineSeries>& multiplotValues;
  const QMap<int, DataModel::Dataset>& datasets;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& widgetGroups;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& widgetDatasets;
};

/**
 * @brief Bulk history rewriter for the dashboard plot rings (spec 0020): the replay-seek window
 *        fill a player pushes after a scrub, and the snapshot/restore pair that carries ring
 *        contents across a layout rebuild or a points/time-range change. All of it runs at seek or
 *        reconfigure rate, never per block, and it writes rings only, never the plot clocks.
 */
class ReplaySeekEngine {
public:
  explicit ReplaySeekEngine(const ReplaySeekBindings& bindings);
  ReplaySeekEngine(ReplaySeekEngine&&)                 = delete;
  ReplaySeekEngine(const ReplaySeekEngine&)            = delete;
  ReplaySeekEngine& operator=(ReplaySeekEngine&&)      = delete;
  ReplaySeekEngine& operator=(const ReplaySeekEngine&) = delete;

  [[nodiscard]] static qint64 seekKey(int sourceId, int uniqueId) noexcept;

  [[nodiscard]] QList<std::pair<int, int>> seekSeries() const;
  [[nodiscard]] bool bulkLoadPlotWindow(const QVector<double>& timesSec,
                                        const QHash<qint64, QVector<double>>& series);

  [[nodiscard]] QHash<qint64, DSP::EnvelopeRing> snapshotPlotTimeRings() const;
  [[nodiscard]] QHash<qint64, std::vector<DSP::EnvelopeRing>> snapshotMultiplotTimeRings() const;

  void restorePlotTimeRings(QHash<qint64, DSP::EnvelopeRing>& snapshot);
  void restoreMultiplotTimeRings(QHash<qint64, std::vector<DSP::EnvelopeRing>>& snapshot);

private:
  [[nodiscard]] int plotCount() const;
  [[nodiscard]] int multiplotCount() const;
  [[nodiscard]] const DataModel::Group& multiplotGroup(const int index) const;
  [[nodiscard]] const DataModel::Dataset& plotDataset(const int index) const;

  void fillSeekPlotSingle(int index,
                          const QVector<double>& timesSec,
                          const QHash<qint64, QVector<double>>& series,
                          double timeOffset,
                          QSet<const DSP::AxisData*>& filled);
  void fillSeekPlotMulti(int index,
                         const QVector<double>& timesSec,
                         const QHash<qint64, QVector<double>>& series,
                         double timeOffset);

private:
  QMap<int, DSP::AxisData>& m_xAxisData;
  QMap<int, DSP::AxisData>& m_yAxisData;
  QMap<int, DSP::EnvelopeRing>& m_plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>>& m_multiplotTimeRings;
  QVector<DSP::MultiLineSeries>& m_multiplotValues;
  const QMap<int, DataModel::Dataset>& m_datasets;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& m_widgetGroups;
  const QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& m_widgetDatasets;
};

}  // namespace UI
