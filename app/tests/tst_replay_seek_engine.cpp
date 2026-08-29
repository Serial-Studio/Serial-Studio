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

#include <QTest>

#include "UI/Dashboard/ReplaySeekEngine.h"

// Replay-seek bulk fill (spec 0020) and the ring snapshot/restore pair, extracted from
// UI::Dashboard (spec 0070). Three properties carry the weight. First, every key is
// source-qualified: dataset uniqueIds repeat across sources, and an unqualified key collapses two
// plots onto one snapshot entry -- the second restore then move-assigns a moved-from ring and
// crashes appendDecimated. Second, a seek window is normalized to end at 0, so no retained sample
// sits in the future of the live timeline the plot clocks resume from. Third, a fill clears the
// widget's ring first: a scrub replaces history, it never appends to it.

class ReplaySeekEngineTest : public QObject {
  Q_OBJECT

private slots:
  void keysAreSourceQualified();
  void seriesListsPlotsXSourcesAndCurves();
  void emptyWindowIsRejected();
  void timeRingsAreClearedAndNormalized();
  void sampleRingsCarryXAndYSources();
  void multiplotCurvesFillTheirOwnRings();
  void restoreReturnsRingsToTheirOwnPlot();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Owns the plot stores an engine binds to, so each test builds a layout by hand.
 */
struct Stores {
  QMap<int, DSP::AxisData> xAxisData;
  QMap<int, DSP::AxisData> yAxisData;
  QMap<int, DSP::EnvelopeRing> plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>> multiplotTimeRings;
  QVector<DSP::MultiLineSeries> multiplotValues;
  QMap<int, DataModel::Dataset> datasets;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>> widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>> widgetDatasets;

  [[nodiscard]] UI::ReplaySeekBindings bindings()
  {
    return UI::ReplaySeekBindings{.xAxisData          = xAxisData,
                                  .yAxisData          = yAxisData,
                                  .plotTimeRings      = plotTimeRings,
                                  .multiplotTimeRings = multiplotTimeRings,
                                  .multiplotValues    = multiplotValues,
                                  .datasets           = datasets,
                                  .widgetGroups       = widgetGroups,
                                  .widgetDatasets     = widgetDatasets};
  }
};

/**
 * @brief Builds one plot dataset; a negative @p xAxisId keeps the default time axis.
 */
DataModel::Dataset makeDataset(int sourceId, int uniqueId, int xAxisId = DataModel::kXAxisTime)
{
  DataModel::Dataset ds;
  ds.sourceId = sourceId;
  ds.uniqueId = uniqueId;
  ds.xAxisId  = xAxisId;
  return ds;
}

/**
 * @brief Builds one multiplot group over the given dataset uniqueIds.
 */
DataModel::Group makeGroup(int sourceId, int uniqueId, const std::vector<int>& datasetIds)
{
  DataModel::Group group;
  group.sourceId = sourceId;
  group.uniqueId = uniqueId;
  for (const int id : datasetIds)
    group.datasets.push_back(makeDataset(sourceId, id));

  return group;
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Keys and series enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief One uniqueId under two sources must never share a key.
 */
void ReplaySeekEngineTest::keysAreSourceQualified()
{
  QCOMPARE(UI::ReplaySeekEngine::seekKey(0, 7), static_cast<qint64>(7));
  QVERIFY(UI::ReplaySeekEngine::seekKey(0, 7) != UI::ReplaySeekEngine::seekKey(1, 7));
  QVERIFY(UI::ReplaySeekEngine::seekKey(2, -1) != UI::ReplaySeekEngine::seekKey(3, -1));
}

/**
 * @brief The player is told about plot datasets, the datasets behind a custom X axis, and every
 *        multiplot curve -- each pair once.
 */
void ReplaySeekEngineTest::seriesListsPlotsXSourcesAndCurves()
{
  Stores stores;
  stores.datasets.insert(12, makeDataset(1, 12));
  stores.widgetDatasets[SerialStudio::DashboardPlot] = {
    makeDataset(0, 10), makeDataset(0, 11, 12), makeDataset(0, 10)};
  stores.widgetGroups[SerialStudio::DashboardMultiPlot] = {makeGroup(2, 30, {20, 21})};

  UI::ReplaySeekEngine engine(stores.bindings());
  const auto pairs = engine.seekSeries();

  const QList<std::pair<int, int>> expected{
    {0, 10},
    {0, 11},
    {1, 12},
    {2, 20},
    {2, 21}
  };

  QCOMPARE(pairs.size(), 5);
  QVERIFY(pairs == expected);
}

//--------------------------------------------------------------------------------------------------
// Bulk window fill
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty window loads nothing, and says so: the caller resets clocks off that answer.
 */
void ReplaySeekEngineTest::emptyWindowIsRejected()
{
  Stores stores;
  UI::ReplaySeekEngine engine(stores.bindings());

  QVERIFY(!engine.bulkLoadPlotWindow({}, {}));
}

/**
 * @brief A fill replaces the ring: nothing from before the scrub survives, and every retained
 *        stamp is at or behind 0, the end of the loaded window.
 */
void ReplaySeekEngineTest::timeRingsAreClearedAndNormalized()
{
  Stores stores;
  stores.widgetDatasets[SerialStudio::DashboardPlot] = {makeDataset(0, 10)};
  stores.plotTimeRings.insert(0, DSP::EnvelopeRing(64, 1.0));
  stores.plotTimeRings[0].appendDecimated(500.0, 1.0);
  stores.plotTimeRings[0].appendDecimated(501.0, 2.0);

  QHash<qint64, QVector<double>> series;
  series.insert(UI::ReplaySeekEngine::seekKey(0, 10), {5.0, 6.0, 7.0, 8.0});

  UI::ReplaySeekEngine engine(stores.bindings());
  QVERIFY(engine.bulkLoadPlotWindow({10.0, 11.0, 12.0, 13.0}, series));

  const auto& ring = stores.plotTimeRings[0];
  QVERIFY(ring.level0.time.size() > 0);
  for (std::size_t i = 0; i < ring.level0.time.size(); ++i) {
    QVERIFY(ring.level0.time[i] <= 0.0);
    QVERIFY(ring.level0.time[i] >= -3.0);
  }
}

/**
 * @brief An XY plot fills both sample rings, each from its own source's column.
 */
void ReplaySeekEngineTest::sampleRingsCarryXAndYSources()
{
  Stores stores;
  stores.datasets.insert(12, makeDataset(1, 12));
  stores.widgetDatasets[SerialStudio::DashboardPlot] = {makeDataset(0, 11, 12)};
  stores.yAxisData.insert(11, DSP::AxisData(16));
  stores.xAxisData.insert(12, DSP::AxisData(16));

  QHash<qint64, QVector<double>> series;
  series.insert(UI::ReplaySeekEngine::seekKey(0, 11), {1.0, 2.0, 3.0});
  series.insert(UI::ReplaySeekEngine::seekKey(1, 12), {7.0, 8.0, 9.0});

  UI::ReplaySeekEngine engine(stores.bindings());
  QVERIFY(engine.bulkLoadPlotWindow({0.0, 1.0, 2.0}, series));

  QCOMPARE(stores.yAxisData[11].size(), static_cast<std::size_t>(3));
  QCOMPARE(stores.xAxisData[12].size(), static_cast<std::size_t>(3));
  QCOMPARE(stores.yAxisData[11][2], 3.0);
  QCOMPARE(stores.xAxisData[12][0], 7.0);
}

/**
 * @brief Every curve of a time-mode multiplot is filled from its own column.
 */
void ReplaySeekEngineTest::multiplotCurvesFillTheirOwnRings()
{
  Stores stores;
  stores.widgetGroups[SerialStudio::DashboardMultiPlot] = {makeGroup(3, 30, {20, 21})};

  std::vector<DSP::EnvelopeRing> rings;
  rings.push_back(DSP::EnvelopeRing(64, 1.0));
  rings.push_back(DSP::EnvelopeRing(64, 1.0));
  stores.multiplotTimeRings.insert(0, std::move(rings));

  QHash<qint64, QVector<double>> series;
  series.insert(UI::ReplaySeekEngine::seekKey(3, 20), {1.0, 2.0, 3.0, 4.0});
  series.insert(UI::ReplaySeekEngine::seekKey(3, 21), {5.0, 6.0, 7.0, 8.0});

  UI::ReplaySeekEngine engine(stores.bindings());
  QVERIFY(engine.bulkLoadPlotWindow({0.0, 1.0, 2.0, 3.0}, series));

  QCOMPARE(stores.multiplotTimeRings[0].size(), static_cast<std::size_t>(2));
  QVERIFY(stores.multiplotTimeRings[0][0].level0.time.size() > 0);
  QVERIFY(stores.multiplotTimeRings[0][1].level0.time.size() > 0);
}

//--------------------------------------------------------------------------------------------------
// Snapshot / restore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two plots of the same uniqueId under different sources keep their own history across a
 *        rebuild: the snapshot key carries the source, so neither restore reads the other's ring.
 */
void ReplaySeekEngineTest::restoreReturnsRingsToTheirOwnPlot()
{
  Stores stores;
  stores.widgetDatasets[SerialStudio::DashboardPlot] = {makeDataset(0, 10), makeDataset(1, 10)};
  stores.plotTimeRings.insert(0, DSP::EnvelopeRing(64, 1.0));
  stores.plotTimeRings.insert(1, DSP::EnvelopeRing(64, 1.0));

  QHash<qint64, QVector<double>> series;
  series.insert(UI::ReplaySeekEngine::seekKey(0, 10), {1.0, 1.0, 1.0, 1.0});
  series.insert(UI::ReplaySeekEngine::seekKey(1, 10), {9.0, 9.0, 9.0, 9.0});

  UI::ReplaySeekEngine engine(stores.bindings());
  QVERIFY(engine.bulkLoadPlotWindow({0.0, 1.0, 2.0, 3.0}, series));

  auto snapshot = engine.snapshotPlotTimeRings();
  QCOMPARE(snapshot.size(), 2);

  stores.plotTimeRings[0] = DSP::EnvelopeRing(64, 1.0);
  stores.plotTimeRings[1] = DSP::EnvelopeRing(64, 1.0);
  QCOMPARE(stores.plotTimeRings[0].level0.time.size(), static_cast<std::size_t>(0));

  engine.restorePlotTimeRings(snapshot);

  QVERIFY(snapshot.isEmpty());
  QVERIFY(stores.plotTimeRings[0].level0.value.size() > 0);
  QVERIFY(stores.plotTimeRings[1].level0.value.size() > 0);
  QCOMPARE(stores.plotTimeRings[0].level0.value[0], 1.0);
  QCOMPARE(stores.plotTimeRings[1].level0.value[0], 9.0);
}

QTEST_APPLESS_MAIN(ReplaySeekEngineTest)

#include "tst_replay_seek_engine.moc"
