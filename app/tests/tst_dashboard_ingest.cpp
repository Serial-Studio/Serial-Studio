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

#include "UI/Dashboard/DashboardIngest.h"

// The dashboard's block ingest (spec 0075, WP-E). The lane under test is the uniform grid one:
// a dense source publishes N samples of M datasets at a fixed step, and before this spec only the
// time rings, the FFT windows and the 3D rings were fed from it -- a Samples-axis plot, a
// Samples-mode multiplot and a GPS group stayed blank while the time plot beside them was live
// (F3). The other properties pinned here are the ones a stream source breaks first: the sample
// feed is bounded by the ring capacity rather than by the block length, GPS advances once per
// block (never once per column, which would triple its rate), the plot clock continues from the
// previous block's span instead of the smoothed cadence, and a block whose structure generation
// does not match the layout is dropped instead of written into the wrong widgets.

class DashboardIngestTest : public QObject {
  Q_OBJECT

private slots:
  void uniformBlockFeedsSamplesPlot();
  void uniformBlockFeedsMultiplotSampleRings();
  void gpsAdvancesOncePerBlock();
  void sampleFeedIsBoundedByRingCapacity();
  void stringsReachOnlyStringTargets();
  void plotClockContinuesFromBlockSpan();
  void staleGenerationBlockIsDropped();
  void columnMismatchHandsOffToTheHost();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

namespace {

constexpr int kPoints         = 8;
constexpr int kPlotUid        = 10;
constexpr int kCurveAUid      = 11;
constexpr int kCurveBUid      = 12;
constexpr int kLatUid         = 20;
constexpr int kLonUid         = 21;
constexpr int kAltUid         = 22;
constexpr quint64 kGeneration = 7;

/**
 * @brief Builds one dataset carrier the layout buckets hold by value.
 */
[[nodiscard]] DataModel::Dataset makeDataset(int uniqueId, const QString& widget, int xAxisId)
{
  DataModel::Dataset dataset;
  dataset.uniqueId = uniqueId;
  dataset.sourceId = 0;
  dataset.xAxisId  = xAxisId;
  dataset.plt      = true;
  dataset.widget   = widget;
  dataset.title    = QStringLiteral("ds%1").arg(uniqueId);
  return dataset;
}

/**
 * @brief Builds one uniform-grid column carrying @p values for @p uniqueId.
 */
[[nodiscard]] DataModel::BlockColumn makeColumn(int uniqueId, const std::vector<double>& values)
{
  DataModel::BlockColumn column;
  column.uniqueId = uniqueId;
  column.values   = values;
  return column;
}

/**
 * @brief The dashboard state an ingest binds to. Each test builds the layout by hand, exactly as
 *        the facade's configure* pass would leave it, then resolves the push tables from it.
 */
struct Stores {
  bool layoutValid           = true;
  bool streamAvailable       = true;
  bool updateRequired        = false;
  bool updateRetryInProgress = false;
  int widgetCount            = 3;
  int points                 = kPoints;
  double plotDisplayTimeSec  = 0.0;

  QMap<int, UI::PlotClock> plotClocks;
  SerialStudio::WidgetMap widgetMap;
  QMap<int, DSP::AxisData> xAxisData;
  QMap<int, DSP::AxisData> yAxisData;
  QMap<int, DSP::EnvelopeRing> plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>> multiplotTimeRings;
  QMap<int, DSP::SweepEngine> plotSweep;
  QMap<int, DSP::SweepEngine> multiplotSweep;
  QMap<int, bool> activePlots;
  QMap<int, bool> activeFFTPlots;
  QMap<int, bool> activeMultiplots;
  QVector<DSP::GpsSeries> gpsValues;
  QVector<DSP::AxisData> fftValues;
  QVector<DSP::LineSeries> pltValues;
  QVector<DSP::MultiLineSeries> multipltValues;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool> activeWaterfalls;
  QVector<DSP::AxisData> waterfallValues;
  QVector<DSP::FixedQueue<QVector3D>> plot3DRings;
  QVector<DSP::LineSeries3D> plotData3D;
#endif
  QMap<int, DataModel::Dataset> datasets;
  QMap<int, UI::DatasetExtremes> datasetExtremes;
  QHash<int, std::vector<UI::ValuePush>> valuePushes;
  QHash<int, std::vector<UI::ExtremePush>> extremePushes;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>> widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>> widgetDatasets;
  QMap<int, DataModel::Frame> sourceRawFrames;
  QHash<int, quint64> sourceStructureGen;

  [[nodiscard]] UI::IngestBindings bindings()
  {
    return UI::IngestBindings{.layoutValid           = layoutValid,
                              .streamAvailable       = streamAvailable,
                              .updateRequired        = updateRequired,
                              .updateRetryInProgress = updateRetryInProgress,
                              .widgetCount           = widgetCount,
                              .points                = points,
                              .plotDisplayTimeSec    = plotDisplayTimeSec,
                              .plotClocks            = plotClocks,
                              .widgetMap             = widgetMap,
                              .xAxisData             = xAxisData,
                              .yAxisData             = yAxisData,
                              .plotTimeRings         = plotTimeRings,
                              .multiplotTimeRings    = multiplotTimeRings,
                              .plotSweep             = plotSweep,
                              .multiplotSweep        = multiplotSweep,
                              .activePlots           = activePlots,
                              .activeFFTPlots        = activeFFTPlots,
                              .activeMultiplots      = activeMultiplots,
                              .gpsValues             = gpsValues,
                              .fftValues             = fftValues,
                              .pltValues             = pltValues,
                              .multipltValues        = multipltValues,
#ifdef BUILD_COMMERCIAL
                              .activeWaterfalls = activeWaterfalls,
                              .waterfallValues  = waterfallValues,
                              .plot3DRings      = plot3DRings,
                              .plotData3D       = plotData3D,
#endif
                              .datasets           = datasets,
                              .datasetExtremes    = datasetExtremes,
                              .valuePushes        = valuePushes,
                              .extremePushes      = extremePushes,
                              .widgetGroups       = widgetGroups,
                              .widgetDatasets     = widgetDatasets,
                              .sourceRawFrames    = sourceRawFrames,
                              .sourceStructureGen = sourceStructureGen};
  }
};

/**
 * @brief Records the facade calls the ingest makes and answers its widget lookups from the
 *        fixture's buckets, so a reconfigure request is observable instead of silent.
 */
class HostStub : public UI::IngestHost {
public:
  explicit HostStub(Stores& stores) : m_stores(stores) {}

  void configureGpsSeries() override { ++reconfigures; }

  void configureFftSeries() override { ++reconfigures; }

  void configureLineSeries() override { ++reconfigures; }

  void configureMultiLineSeries() override { ++reconfigures; }

#ifdef BUILD_COMMERCIAL
  void configurePlot3DSeries() override { ++reconfigures; }

  void configureWaterfallSeries() override { ++reconfigures; }
#endif

  void handleMissingDataset(const DataModel::Frame&) override { ++missingDatasets; }

  [[nodiscard]] bool useTimeXAxis(const DataModel::Dataset& dataset) const override
  {
    return dataset.xAxisId == DataModel::kXAxisTime;
  }

  [[nodiscard]] const DataModel::Group& getGroupWidget(const SerialStudio::DashboardWidget widget,
                                                       const int index) const override
  {
    static const DataModel::Group empty;
    const auto it = m_stores.widgetGroups.constFind(widget);
    if (it == m_stores.widgetGroups.cend() || index < 0 || index >= it->size())
      return empty;

    return it->at(index);
  }

  [[nodiscard]] const DataModel::Dataset& getDatasetWidget(
    const SerialStudio::DashboardWidget widget, const int index) const override
  {
    static const DataModel::Dataset empty;
    const auto it = m_stores.widgetDatasets.constFind(widget);
    if (it == m_stores.widgetDatasets.cend() || index < 0 || index >= it->size())
      return empty;

    return it->at(index);
  }

  int reconfigures    = 0;
  int missingDatasets = 0;

private:
  Stores& m_stores;
};

/**
 * @brief Lays out one Samples-axis plot, one Samples-mode multiplot of two curves and one GPS
 *        group, all on source 0, and resolves the value-push table over the widget copies. The
 *        buckets are filled before any pointer is taken: the tables address those datasets
 *        directly, so a later resize would move what they point at.
 */
void buildLayout(Stores& stores)
{
  QVector<DataModel::Dataset> plots;
  plots.append(makeDataset(kPlotUid, QStringLiteral("plot"), DataModel::kXAxisSamples));
  stores.widgetDatasets.insert(SerialStudio::DashboardPlot, plots);

  DataModel::Group multiplot;
  multiplot.groupId  = 0;
  multiplot.uniqueId = 100;
  multiplot.sourceId = 0;
  multiplot.widget   = QStringLiteral("multiplot");
  multiplot.datasets.push_back(
    makeDataset(kCurveAUid, QStringLiteral("multiplot"), DataModel::kXAxisSamples));
  multiplot.datasets.push_back(
    makeDataset(kCurveBUid, QStringLiteral("multiplot"), DataModel::kXAxisSamples));

  DataModel::Group gps;
  gps.groupId  = 1;
  gps.uniqueId = 101;
  gps.sourceId = 0;
  gps.widget   = QStringLiteral("map");
  gps.datasets.push_back(makeDataset(kLatUid, QStringLiteral("lat"), DataModel::kXAxisTime));
  gps.datasets.push_back(makeDataset(kLonUid, QStringLiteral("lon"), DataModel::kXAxisTime));
  gps.datasets.push_back(makeDataset(kAltUid, QStringLiteral("alt"), DataModel::kXAxisTime));

  stores.widgetGroups.insert(SerialStudio::DashboardMultiPlot, {multiplot});
  stores.widgetGroups.insert(SerialStudio::DashboardGPS, {gps});

  stores.yAxisData.insert(kPlotUid, DSP::AxisData(kPoints + 1));
  stores.activePlots.insert(0, true);
  stores.pltValues.append(DSP::LineSeries{});

  DSP::MultiLineSeries series;
  series.y.push_back(DSP::AxisData(kPoints + 1));
  series.y.push_back(DSP::AxisData(kPoints + 1));
  stores.multipltValues.append(series);
  stores.activeMultiplots.insert(0, true);

  DSP::GpsSeries fix;
  fix.latitudes.resize(kPoints + 1);
  fix.longitudes.resize(kPoints + 1);
  fix.altitudes.resize(kPoints + 1);
  stores.gpsValues.append(fix);

  DataModel::Frame frame;
  frame.sourceId = 0;
  frame.groups.push_back(multiplot);
  stores.sourceRawFrames.insert(0, frame);
  stores.sourceStructureGen.insert(0, kGeneration);
}

/**
 * @brief Resolves one value-push entry per column, in block order, over the widget copies the
 *        layout holds. @p stringTargetUid is the single dataset whose text is observable.
 */
void buildValuePushes(Stores& stores, int stringTargetUid = -1)
{
  const std::vector<int> order{kPlotUid, kCurveAUid, kCurveBUid, kLatUid, kLonUid, kAltUid};

  QHash<int, std::vector<DataModel::Dataset*>> byUid;
  for (auto it = stores.widgetGroups.begin(); it != stores.widgetGroups.end(); ++it)
    for (auto& group : it.value())
      for (auto& dataset : group.datasets)
        byUid[dataset.uniqueId].push_back(&dataset);

  for (auto it = stores.widgetDatasets.begin(); it != stores.widgetDatasets.end(); ++it)
    for (auto& dataset : it.value())
      byUid[dataset.uniqueId].push_back(&dataset);

  auto& table = stores.valuePushes[0];
  for (const int uid : order) {
    UI::ValuePush push;
    push.uniqueId = uid;
    push.targets  = byUid.value(uid);
    if (uid == stringTargetUid)
      push.stringTargets = push.targets;

    table.push_back(push);
  }
}

/**
 * @brief Builds one uniform-grid block of @p samples values per dataset. Column i of dataset d
 *        carries base(d) + i so a ring's contents identify both the dataset and the sample.
 */
[[nodiscard]] DataModel::DataBlockPtr makeBlock(qsizetype samples)
{
  auto block                 = std::make_shared<DataModel::DataBlock>();
  block->sourceId            = 0;
  block->structureGeneration = kGeneration;
  block->samples             = samples;
  block->t0                  = std::chrono::steady_clock::now();
  block->dt                  = std::chrono::milliseconds(1);

  const std::vector<int> order{kPlotUid, kCurveAUid, kCurveBUid, kLatUid, kLonUid, kAltUid};
  for (const int uid : order) {
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(samples));
    for (qsizetype i = 0; i < samples; ++i)
      values.push_back(static_cast<double>(uid * 1000) + static_cast<double>(i));

    block->columns.push_back(makeColumn(uid, values));
  }

  return block;
}

}  // namespace

//--------------------------------------------------------------------------------------------------
// Uniform-grid lane coverage (F3)
//--------------------------------------------------------------------------------------------------

/**
 * @brief A Samples-axis plot must receive one ring sample per block sample.
 */
void DashboardIngestTest::uniformBlockFeedsSamplesPlot()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  ingest.applyBlock(makeBlock(4));

  QCOMPARE(host.reconfigures, 0);
  QCOMPARE(stores.yAxisData[kPlotUid].size(), std::size_t(4));
  QCOMPARE(stores.yAxisData[kPlotUid].back(), double(kPlotUid * 1000 + 3));
  QVERIFY(stores.updateRequired);
}

/**
 * @brief A Samples-mode multiplot must receive every sample on every curve, in step.
 */
void DashboardIngestTest::uniformBlockFeedsMultiplotSampleRings()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  ingest.applyBlock(makeBlock(4));

  const auto& curves = stores.multipltValues[0].y;
  QCOMPARE(curves[0].size(), std::size_t(4));
  QCOMPARE(curves[1].size(), std::size_t(4));
  QCOMPARE(curves[0].back(), double(kCurveAUid * 1000 + 3));
  QCOMPARE(curves[1].back(), double(kCurveBUid * 1000 + 3));
}

/**
 * @brief GPS is a per-block consumer: three columns of one group must still advance the track by
 *        one fix, carrying the block's last sample.
 */
void DashboardIngestTest::gpsAdvancesOncePerBlock()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  stores.gpsValues[0].latitudes.clear();
  stores.gpsValues[0].longitudes.clear();
  stores.gpsValues[0].altitudes.clear();

  ingest.applyBlock(makeBlock(4));

  QCOMPARE(stores.gpsValues[0].latitudes.size(), std::size_t(1));
  QCOMPARE(stores.gpsValues[0].latitudes.back(), double(kLatUid * 1000 + 3));
  QCOMPARE(stores.gpsValues[0].longitudes.back(), double(kLonUid * 1000 + 3));
  QCOMPARE(stores.gpsValues[0].altitudes.back(), double(kAltUid * 1000 + 3));
}

/**
 * @brief A block longer than the ring writes only the samples the ring could have kept: a
 *        4096-sample stream block must not cost 4096 stores into a 9-slot plot.
 */
void DashboardIngestTest::sampleFeedIsBoundedByRingCapacity()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  ingest.applyBlock(makeBlock(100));

  const auto& ring = stores.yAxisData[kPlotUid];
  QCOMPARE(ring.size(), ring.capacity());
  QCOMPARE(ring.back(), double(kPlotUid * 1000 + 99));
  QCOMPARE(ring.front(), double(kPlotUid * 1000 + 100 - static_cast<int>(ring.capacity())));
}

/**
 * @brief A dense column carries no text, so the rendered string must reach the datasets that
 *        display it and nothing else (F9).
 */
void DashboardIngestTest::stringsReachOnlyStringTargets()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores, kPlotUid);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  ingest.applyBlock(makeBlock(4));

  const auto& plot  = stores.widgetDatasets[SerialStudio::DashboardPlot][0];
  const auto& curve = stores.widgetGroups[SerialStudio::DashboardMultiPlot][0].datasets[0];

  QCOMPARE(plot.numericValue, double(kPlotUid * 1000 + 3));
  QCOMPARE(plot.value, QString::number(double(kPlotUid * 1000 + 3), 'g', 10));
  QCOMPARE(curve.numericValue, double(kCurveAUid * 1000 + 3));
  QVERIFY(curve.value.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Plot clock and block gating
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consecutive uniform blocks continue from the previous block's span, so the display time
 *        advances by exactly one block per block and never rewinds into the retained history.
 */
void DashboardIngestTest::plotClockContinuesFromBlockSpan()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);

  const auto origin  = std::chrono::steady_clock::now();
  const double one   = ingest.advancePlotClock(0, origin, 0.5);
  const double two   = ingest.advancePlotClock(0, origin, 0.5);
  const double three = ingest.advancePlotClock(0, origin, 0.5);

  QVERIFY(qFuzzyIsNull(one));
  QVERIFY(two >= one + 0.5);
  QVERIFY(three >= two + 0.5);
  QCOMPARE(stores.plotDisplayTimeSec, three);
}

/**
 * @brief A block staged under a structure the dashboard no longer shows is dropped whole: it
 *        must not reach the push tables, which are indexed for the current layout.
 */
void DashboardIngestTest::staleGenerationBlockIsDropped()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  auto block                 = std::make_shared<DataModel::DataBlock>(*makeBlock(4));
  block->structureGeneration = kGeneration + 1;
  ingest.applyBlock(block);

  QCOMPARE(stores.yAxisData[kPlotUid].size(), std::size_t(0));
  QCOMPARE(host.missingDatasets, 0);
  QVERIFY(!stores.updateRequired);
}

/**
 * @brief A block whose column count no longer matches the value-push table is handed to the
 *        facade's rebuild-once-then-quarantine path instead of being written positionally.
 */
void DashboardIngestTest::columnMismatchHandsOffToTheHost()
{
  Stores stores;
  buildLayout(stores);
  buildValuePushes(stores);

  HostStub host(stores);
  UI::DashboardIngest ingest(stores.bindings(), host);
  ingest.buildLinePushes();
  ingest.buildMultiplotPushes();
  ingest.buildGpsPushes();

  auto block                 = std::make_shared<DataModel::DataBlock>();
  block->sourceId            = 0;
  block->structureGeneration = kGeneration;
  block->samples             = 2;
  block->t0                  = std::chrono::steady_clock::now();
  block->dt                  = std::chrono::milliseconds(1);
  block->columns.push_back(makeColumn(kPlotUid, {1.0, 2.0}));

  ingest.applyBlock(block);

  QCOMPARE(host.missingDatasets, 1);
  QVERIFY(!stores.updateRequired);
}

QTEST_APPLESS_MAIN(DashboardIngestTest)

#include "tst_dashboard_ingest.moc"
