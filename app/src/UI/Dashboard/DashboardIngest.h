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

#include <chrono>
#include <QHash>
#include <QMap>
#include <QVector>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/Frame.h"
#include "DSP.h"
#include "SerialStudio.h"
#include "UI/Dashboard/WidgetMapBuilder.h"

namespace UI {
/**
 * @brief Pre-resolved descriptor that pushes one value into one ring buffer.
 */
struct LinePush {
  struct Consumer {
    int sourceId;
    const bool* activeFlag;
  };

  std::vector<Consumer> consumers;
  DSP::AxisData* buf;
  const double* value;
  int uniqueId;
};

/**
 * @brief Pre-resolved descriptor that appends one (time, value) into a decimating ring. The
 *        ring and sweep are addressed by plotIndex (key into m_plotTimeRings / m_plotSweep) and
 *        resolved at use time, never cached as raw pointers: a layout rebuild that reallocates
 *        those QMap nodes would dangle a cached pointer, so the index must be re-looked-up.
 */
struct TimePush {
  std::vector<LinePush::Consumer> consumers;
  int plotIndex;
  const double* value;
};

/**
 * @brief Pre-resolved descriptor for one multiplot, time-ring or sample mode. The time rings and
 *        sweep are addressed by groupIndex (key into m_multiplotTimeRings / m_multiplotSweep) and
 *        resolved at use time; only per-curve value pointers (stable across the build) are cached.
 */
struct MultiPush {
  struct TimeCurve {
    int curveIndex;
    const double* value;
  };

  int sourceId;
  int groupIndex;
  const bool* activeFlag;
  std::vector<TimeCurve> timeCurves;
  std::vector<std::pair<DSP::AxisData*, const double*>> samples;
};

/**
 * @brief Pre-resolved descriptor that pushes one dataset value into one sample ring.
 */
struct SeriesPush {
  int sourceId;
  const bool* activeFlag;
  DSP::AxisData* buf;
  const double* value;
#ifdef BUILD_COMMERCIAL
  bool record;
  quint32 sessionKey;
#endif
};

/**
 * @brief Pre-resolved GPS coordinate sources: numeric gate + value pointer per axis.
 */
struct GpsPush {
  struct Field {
    const double* value;
    const bool* numeric;
  };

  int sourceId;
  DSP::GpsSeries* series;
  Field lat;
  Field lon;
  Field alt;
};

#ifdef BUILD_COMMERCIAL
/**
 * @brief Pre-resolved 3D trajectory sources feeding an O(1) overwrite ring.
 */
struct Plot3DPush {
  int sourceId;
  DSP::FixedQueue<QVector3D>* ring;
  const double* x;
  const double* y;
  const double* z;
};
#endif

/**
 * @brief Pre-resolved stream ingest targets for one dataset uniqueId: widget and push-table
 *        indexes only (never raw ring pointers -- a layout rebuild reallocates those), rebuilt
 *        lazily after every reconfigure (spec 0051 T25). The sample-ring entries let a uniform-grid
 *        block reach a Samples-axis plot, a dataset-X plot and a Samples-mode multiplot (F3).
 */
struct StreamTargets {
  std::vector<int> plotIndexes;
  std::vector<std::pair<int, int>> multiplotCurves;
  std::vector<int> fftIndexes;
  std::vector<int> yLinePushIndexes;
  std::vector<int> xLinePushIndexes;
  std::vector<std::pair<int, int>> multiSampleIndexes;
#ifdef BUILD_COMMERCIAL
  std::vector<int> waterfallIndexes;
#endif
};

/**
 * @brief Per-source plot clock: origin, smoothed sample period and forward-only display
 *        time. Each source owns its own clock so interleaved frames from one source never
 *        advance or rewind another source's plot rings.
 */
struct PlotClock {
  bool originSet                               = false;
  int groupCount                               = 0;
  double relativeFrameTimeSec                  = 0.0;
  double displayTimeSec                        = 0.0;
  double groupStartSec                         = 0.0;
  double samplePeriodSec                       = 0.0;
  double blockSpanSec                          = 0.0;
  std::chrono::steady_clock::time_point origin = {};
};

/**
 * @brief The dashboard state the ingest path reads and writes. Every entry stays owned by
 *        UI::Dashboard and is bound here by reference: the push tables hold raw pointers into
 *        these containers, so handing the ingest its own copies would move what they address.
 */
struct IngestBindings {
  const bool& layoutValid;
  const bool& streamAvailable;
  bool& updateRequired;
  bool& updateRetryInProgress;
  const int& widgetCount;
  const int& points;
  double& plotDisplayTimeSec;
  QMap<int, PlotClock>& plotClocks;
  const SerialStudio::WidgetMap& widgetMap;
  QMap<int, DSP::AxisData>& xAxisData;
  QMap<int, DSP::AxisData>& yAxisData;
  QMap<int, DSP::EnvelopeRing>& plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>>& multiplotTimeRings;
  QMap<int, DSP::SweepEngine>& plotSweep;
  QMap<int, DSP::SweepEngine>& multiplotSweep;
  QMap<int, bool>& activePlots;
  QMap<int, bool>& activeFFTPlots;
  QMap<int, bool>& activeMultiplots;
  QVector<DSP::GpsSeries>& gpsValues;
  QVector<DSP::AxisData>& fftValues;
  QVector<DSP::LineSeries>& pltValues;
  QVector<DSP::MultiLineSeries>& multipltValues;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool>& activeWaterfalls;
  QVector<DSP::AxisData>& waterfallValues;
  QVector<DSP::FixedQueue<QVector3D>>& plot3DRings;
  QVector<DSP::LineSeries3D>& plotData3D;
#endif
  QMap<int, DataModel::Dataset>& datasets;
  QMap<int, DatasetExtremes>& datasetExtremes;
  QHash<int, std::vector<ValuePush>>& valuePushes;
  QHash<int, std::vector<ExtremePush>>& extremePushes;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& widgetDatasets;
  QMap<int, DataModel::Frame>& sourceRawFrames;
  QHash<int, quint64>& sourceStructureGen;
};

/**
 * @brief The facade services the ingest path calls back into: the series allocators it cannot
 *        own (they size buffers the push tables then point at) and the layout-repair entry a
 *        block that no longer matches the widget model hands off to.
 */
class IngestHost {
public:
  virtual ~IngestHost() = default;

  virtual void configureGpsSeries()       = 0;
  virtual void configureFftSeries()       = 0;
  virtual void configureLineSeries()      = 0;
  virtual void configureMultiLineSeries() = 0;
#ifdef BUILD_COMMERCIAL
  virtual void configurePlot3DSeries()    = 0;
  virtual void configureWaterfallSeries() = 0;
#endif
  virtual void handleMissingDataset(const DataModel::Frame& frame) = 0;

  [[nodiscard]] virtual bool useTimeXAxis(const DataModel::Dataset& dataset) const = 0;
  [[nodiscard]] virtual const DataModel::Group& getGroupWidget(
    const SerialStudio::DashboardWidget widget, const int index) const = 0;
  [[nodiscard]] virtual const DataModel::Dataset& getDatasetWidget(
    const SerialStudio::DashboardWidget widget, const int index) const = 0;
};

/**
 * @brief The dashboard's GUI-thread ingest path: block application, the per-source plot clocks,
 *        the sweep feeds and the pre-resolved push tables the per-block walk runs on. Hotpath:
 *        no container lookup per sample, no allocation, indexes rather than ring pointers, and
 *        the tables share the facade's m_layoutValid staleness contract.
 */
class DashboardIngest {
public:
  DashboardIngest(const IngestBindings& bindings, IngestHost& host);
  DashboardIngest(DashboardIngest&&)                 = delete;
  DashboardIngest(const DashboardIngest&)            = delete;
  DashboardIngest& operator=(DashboardIngest&&)      = delete;
  DashboardIngest& operator=(const DashboardIngest&) = delete;

  void clearPushTables();
  void buildGpsPushes();
  void buildFftPushes();
  void buildLinePushes();
  void buildMultiplotPushes();
#ifdef BUILD_COMMERCIAL
  void buildPlot3DPushes();
  void buildWaterfallPushes();
#endif

  void applyBlock(const DataModel::DataBlockPtr& block);
  void updateDataSeries(int sourceId = -1);

  double advancePlotClock(int sourceId,
                          const std::chrono::steady_clock::time_point& ts,
                          double blockSpanSec = 0.0);

#ifdef BUILD_COMMERCIAL
  void setFftAudioTap(const int index, const bool enabled, const quint32 key);
  void setWaterfallAudioTap(const int index, const bool enabled, const quint32 key);
#endif

private:
  [[nodiscard]] int groupWidgetCount(const SerialStudio::DashboardWidget widget) const;
  [[nodiscard]] int datasetWidgetCount(const SerialStudio::DashboardWidget widget) const;

  [[nodiscard]] const StreamTargets& streamTargetsFor(int uniqueId);
  [[nodiscard]] bool applyBlockValues(const DataModel::DataBlock& block, qsizetype index);
  void applyBlockColumn(const DataModel::BlockColumn& column,
                        const DataModel::DataBlock& block,
                        double baseSec);
  void feedFftFromSamples(const DataModel::BlockColumn& column,
                          const StreamTargets& targets,
                          std::size_t count);
  void feedSampleRings(const DataModel::BlockColumn& column,
                       const StreamTargets& targets,
                       int sourceId,
                       std::size_t count);
  void feedPlotBlockSweep(int plotIndex,
                          const DataModel::BlockColumn& column,
                          const DataModel::DataBlock& block,
                          double baseSec);
  void feedMultiplotBlockSweep(int groupIndex, const DataModel::DataBlock& block, double baseSec);

  void foldExtremes(int sourceId);
  void updateFftSeries(int sourceId);
  void updateGpsSeries(int sourceId);
  void updateLineSeries(int sourceId);
  void updatePlot3DSeries(int sourceId);
#ifdef BUILD_COMMERCIAL
  void updateWaterfallSeries(int sourceId);
#endif

private:
  IngestHost& m_host;

  const bool& m_layoutValid;
  const bool& m_streamAvailable;
  bool& m_updateRequired;
  bool& m_updateRetryInProgress;
  const int& m_widgetCount;
#ifdef BUILD_COMMERCIAL
  const int& m_points;
#endif
  double& m_plotDisplayTimeSec;
  QMap<int, PlotClock>& m_plotClocks;
  const SerialStudio::WidgetMap& m_widgetMap;
  QMap<int, DSP::AxisData>& m_xAxisData;
  QMap<int, DSP::AxisData>& m_yAxisData;
  QMap<int, DSP::EnvelopeRing>& m_plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>>& m_multiplotTimeRings;
  QMap<int, DSP::SweepEngine>& m_plotSweep;
  QMap<int, DSP::SweepEngine>& m_multiplotSweep;
  QMap<int, bool>& m_activePlots;
  QMap<int, bool>& m_activeFFTPlots;
  QMap<int, bool>& m_activeMultiplots;
  QVector<DSP::GpsSeries>& m_gpsValues;
  QVector<DSP::AxisData>& m_fftValues;
  QVector<DSP::LineSeries>& m_pltValues;
  QVector<DSP::MultiLineSeries>& m_multipltValues;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool>& m_activeWaterfalls;
  QVector<DSP::AxisData>& m_waterfallValues;
  QVector<DSP::FixedQueue<QVector3D>>& m_plot3DRings;
  QVector<DSP::LineSeries3D>& m_plotData3D;
#endif
  QMap<int, DataModel::Dataset>& m_datasets;
  QMap<int, DatasetExtremes>& m_datasetExtremes;
  QHash<int, std::vector<ValuePush>>& m_valuePushes;
  QHash<int, std::vector<ExtremePush>>& m_extremePushes;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& m_widgetDatasets;
  QMap<int, DataModel::Frame>& m_sourceRawFrames;
  QHash<int, quint64>& m_sourceStructureGen;

  std::vector<LinePush> m_yLinePushes;
  std::vector<LinePush> m_xLinePushes;
  std::vector<TimePush> m_timePushes;
  std::vector<MultiPush> m_multiplotPushes;
  std::vector<SeriesPush> m_fftPushes;
  std::vector<GpsPush> m_gpsPushes;
#ifdef BUILD_COMMERCIAL
  std::vector<SeriesPush> m_waterfallPushes;
  std::vector<Plot3DPush> m_plot3DPushes;
#endif

  QHash<int, StreamTargets> m_streamTargets;

  // Curve-index -> column scratch for one multiplot sweep; reused so a tick allocates nothing
  std::vector<const DataModel::BlockColumn*> m_streamSweepCurves;
};
}  // namespace UI
