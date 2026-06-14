/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <chrono>
#include <QFont>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QSettings>

#include "DSP.h"
#include "SerialStudio.h"
#include "UI/WidgetRegistry.h"

namespace UI {
/**
 * @brief Real-time dashboard manager for displaying data-driven widgets.
 */
class Dashboard : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool available
             READ available
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int  points
             READ  points
             WRITE setPoints
             NOTIFY pointsChanged)
  Q_PROPERTY(int  actionCount
             READ actionCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(int  totalWidgetCount
             READ totalWidgetCount
             NOTIFY widgetCountChanged)
  Q_PROPERTY(bool pointsWidgetVisible
             READ pointsWidgetVisible
             NOTIFY widgetCountChanged)
  Q_PROPERTY(bool showActionPanel
             READ  showActionPanel
             WRITE setShowActionPanel
             NOTIFY showActionPanelChanged)
  Q_PROPERTY(bool terminalEnabled
             READ  terminalEnabled
             WRITE setTerminalEnabled
             NOTIFY terminalEnabledChanged)
  Q_PROPERTY(bool notificationLogEnabled
             READ  notificationLogEnabled
             WRITE setNotificationLogEnabled
             NOTIFY notificationLogEnabledChanged)
  Q_PROPERTY(bool clockEnabled
             READ  clockEnabled
             WRITE setClockEnabled
             NOTIFY clockEnabledChanged)
  Q_PROPERTY(bool stopwatchEnabled
             READ  stopwatchEnabled
             WRITE setStopwatchEnabled
             NOTIFY stopwatchEnabledChanged)
  Q_PROPERTY(bool autoHideToolbar
             READ  autoHideToolbar
             WRITE setAutoHideToolbar
             NOTIFY autoHideToolbarChanged)
  Q_PROPERTY(double plotTimeRange
             READ  plotTimeRange
             WRITE setPlotTimeRange
             NOTIFY plotTimeRangeChanged)
  Q_PROPERTY(bool containsCommercialFeatures
             READ containsCommercialFeatures
             NOTIFY containsCommercialFeaturesChanged)
  Q_PROPERTY(QString title
             READ title
             NOTIFY widgetCountChanged)
  Q_PROPERTY(QVariantList actions
             READ actions
             NOTIFY actionStatusChanged)
  // clang-format on

signals:
  void updated();
  void dataReset();
  void pointsChanged();
  void widgetCountChanged();
  void actionStatusChanged();
  void showActionPanelChanged();
  void terminalEnabledChanged();
  void notificationLogEnabledChanged();
  void clockEnabledChanged();
  void stopwatchEnabledChanged();
  void autoHideToolbarChanged();
  void plotTimeRangeChanged();
  void containsCommercialFeaturesChanged();

private:
  explicit Dashboard();
  Dashboard(Dashboard&&)                 = delete;
  Dashboard(const Dashboard&)            = delete;
  Dashboard& operator=(Dashboard&&)      = delete;
  Dashboard& operator=(const Dashboard&) = delete;

public:
  [[nodiscard]] static Dashboard& instance();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool showActionPanel() const noexcept;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool terminalEnabled() const noexcept;
  [[nodiscard]] bool notificationLogEnabled() const noexcept;
  [[nodiscard]] bool clockEnabled() const noexcept;
  [[nodiscard]] bool stopwatchEnabled() const noexcept;
  [[nodiscard]] bool autoHideToolbar() const noexcept;
  [[nodiscard]] double plotTimeRange() const noexcept;
  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool containsCommercialFeatures() const noexcept;

  [[nodiscard]] int points() const noexcept;
  [[nodiscard]] int actionCount() const;
  [[nodiscard]] int totalWidgetCount() const noexcept;

  [[nodiscard]] Q_INVOKABLE bool frameValid() const;
  [[nodiscard]] Q_INVOKABLE int relativeIndex(const int widgetIndex) const;
  [[nodiscard]] Q_INVOKABLE QString formatValue(double val, double min, double max) const;
  [[nodiscard]] Q_INVOKABLE SerialStudio::DashboardWidget widgetType(const int widgetIndex) const;
  [[nodiscard]] Q_INVOKABLE int widgetCount(const SerialStudio::DashboardWidget widget) const;

  [[nodiscard]] const QString& title() const;
  [[nodiscard]] QVariantList actions() const;
  [[nodiscard]] int actionIndexForId(int actionId) const noexcept;
  [[nodiscard]] const SerialStudio::WidgetMap& widgetMap() const;

  [[nodiscard]] int groupIdForUniqueId(int uniqueId) const;
  [[nodiscard]] int groupUniqueIdForGroupId(int groupId) const;

  // clang-format off
  [[nodiscard]] const QMap<int, DataModel::Dataset> &datasets() const;
  [[nodiscard]] const DataModel::Group &getGroupWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  [[nodiscard]] const DataModel::Dataset &getDatasetWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] bool useTimeXAxis(const DataModel::Dataset& dataset) const;
  [[nodiscard]] bool useTimeXAxisGroup(const DataModel::Group& group) const;

  [[nodiscard]] const DataModel::Frame& rawFrame();
  [[nodiscard]] const DataModel::Frame& processedFrame();
  [[nodiscard]] const DSP::AxisData& fftData(const int index) const;
  [[nodiscard]] const DSP::GpsSeries& gpsSeries(const int index) const;
  [[nodiscard]] const DSP::LineSeries& plotData(const int index) const;
  [[nodiscard]] const DSP::MultiLineSeries& multiplotData(const int index) const;
  [[nodiscard]] const DSP::TimeRing& plotTimeRing(const int index) const;
  [[nodiscard]] const std::vector<DSP::TimeRing>& multiplotTimeRings(const int index) const;
  [[nodiscard]] const DSP::SweepEngine& plotSweep(const int index) const;
  [[nodiscard]] const DSP::SweepEngine& multiplotSweep(const int index) const;

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] const DSP::LineSeries3D& plotData3D(const int index) const;
  [[nodiscard]] const DSP::AxisData& waterfallData(const int index) const;
#endif

  [[nodiscard]] bool plotRunning(const int index);
  [[nodiscard]] bool fftPlotRunning(const int index);
  [[nodiscard]] bool multiplotRunning(const int index);
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] bool waterfallRunning(const int index);
#endif

public slots:
  void setPoints(const int points);
  void resetData(const bool notify = true);
  void clearPlotData();
  void setShowActionPanel(const bool enabled);
  void setTerminalEnabled(const bool enabled);
  void setNotificationLogEnabled(const bool enabled);
  void setClockEnabled(const bool enabled);
  void setStopwatchEnabled(const bool enabled);
  void setAutoHideToolbar(const bool enabled);
  void setPlotTimeRange(const double seconds);
  void setSettingsPersistent(const bool persistent);
  void activateAction(const int index, const bool guiTrigger = false);

  void setPlotRunning(const int index, const bool enabled);
  void setFFTPlotRunning(const int index, const bool enabled);
  void setMultiplotRunning(const int index, const bool enabled);
#ifdef BUILD_COMMERCIAL
  void setWaterfallRunning(const int index, const bool enabled);
#endif

  void setPlotSweep(const int index,
                    const bool enabled,
                    const double level,
                    const int edge,
                    const int mode,
                    const double holdoff,
                    const double timebase);
  void setMultiplotSweep(const int index,
                         const bool enabled,
                         const double level,
                         const int edge,
                         const int mode,
                         const double holdoff,
                         const int triggerCurve,
                         const double timebase);
  void armPlotSweep(const int index);
  void armMultiplotSweep(const int index);

  void hotpathRxFrame(const DataModel::TimestampedFramePtr& frame);
  void updateStreamAvailable();

private:
  void connectStreamAvailableInputs();
  void updateDashboardData(const DataModel::Frame& frame);
  void reconfigureDashboard(const DataModel::Frame& frame);
  [[nodiscard]] DataModel::Frame combineSourceFrames(const DataModel::Frame& seed) const;
  void processDatasetIntoWidgetMaps(const DataModel::Dataset& datasetIn,
                                    DataModel::Group& ledPanel);
  void handleMissingDataset(const DataModel::Frame& frame);
  void registerXAxisIfNeeded(const DataModel::Dataset& dataset);

  void updateDataSeries(int sourceId = -1);
  void updateFftSeries(int sourceId);
  void updateGpsSeries(int sourceId);
  void updatePlot3DSeries(int sourceId);
  void updateLineSeries(int sourceId);
#ifdef BUILD_COMMERCIAL
  void updateWaterfallSeries(int sourceId);
#endif

  void configureGpsSeries();
  void configureFftSeries();
  void configureLineSeries();
  void configurePlot3DSeries();
  void configureMultiLineSeries();
  void buildMultiplotPushes();
  void configureActions(const DataModel::Frame& frame);

  [[nodiscard]] QHash<qint64, DSP::TimeRing> snapshotPlotTimeRings() const;
  [[nodiscard]] QHash<qint64, std::vector<DSP::TimeRing>> snapshotMultiplotTimeRings() const;
  void restorePlotTimeRings(QHash<qint64, DSP::TimeRing>& snapshot);
  void restoreMultiplotTimeRings(QHash<qint64, std::vector<DSP::TimeRing>>& snapshot);
  void restorePlotSweepConfig(const QMap<int, DSP::SweepEngine>& saved);
  void restoreMultiplotSweepConfig(const QMap<int, DSP::SweepEngine>& saved);
#ifdef BUILD_COMMERCIAL
  void configureWaterfallSeries();
#endif

  void buildWidgetGroups(const DataModel::Frame& frame, bool pro);
  void registerWidgets();
  void buildDatasetReferences();
  void rebuildDatasetReferences();
  void buildValuePushes();
  void clearPushTables();
  void relabelGroupAsMultiplotFallback(int groupId, const QString& newTitle);

private:
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
   *        resolved at use time; only per-curve value pointers (stable across the build) are
   * cached.
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
   * @brief Pre-resolved value-propagation entry: one incoming dataset to its widget copies.
   *        stringTargets is the subset whose string value is observable (DataGrid rows and the
   *        API-serialized m_lastFrame); the rest only ever consume the numeric fields.
   */
  struct ValuePush {
    std::vector<DataModel::Dataset*> targets;
    std::vector<DataModel::Dataset*> stringTargets;
    int uniqueId;
  };

  [[nodiscard]] ValuePush makeValuePush(const DataModel::Dataset& dataset,
                                        const QSet<const DataModel::Dataset*>& stringTargets) const;

  /**
   * @brief Pre-resolved descriptor that pushes one dataset value into one sample ring.
   */
  struct SeriesPush {
    int sourceId;
    const bool* activeFlag;
    DSP::AxisData* buf;
    const double* value;
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
    std::chrono::steady_clock::time_point origin = {};
  };

  QSettings m_settings;
  int m_points;
  int m_widgetCount;
  bool m_updateRequired;
  bool m_showActionPanel;
  bool m_terminalEnabled;
  bool m_notificationLogEnabled;
  bool m_clockEnabled;
  bool m_stopwatchEnabled;
  bool m_autoHideToolbar;
  bool m_persistSettings;

  bool m_updateRetryInProgress;

  // False while cached Dataset*/ring pointers are stale (post-reset, pre-reconfigure)
  bool m_layoutValid;
  bool m_streamAvailable;

  double m_plotTimeRange;
  double m_plotDisplayTimeSec;
  QMap<int, PlotClock> m_plotClocks;

  DSP::AxisData m_pltXAxis;
  DSP::AxisData m_multipltXAxis;

  QMap<int, DSP::AxisData> m_xAxisData;
  QMap<int, DSP::AxisData> m_yAxisData;
  QMap<int, DSP::TimeRing> m_plotTimeRings;
  QMap<int, std::vector<DSP::TimeRing>> m_multiplotTimeRings;
  QMap<int, DSP::SweepEngine> m_plotSweep;
  QMap<int, DSP::SweepEngine> m_multiplotSweep;

  QMap<int, bool> m_activePlots;
  QMap<int, bool> m_activeFFTPlots;
  QMap<int, bool> m_activeMultiplots;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool> m_activeWaterfalls;
#endif

  QVector<DSP::GpsSeries> m_gpsValues;
  QVector<DSP::AxisData> m_fftValues;
  QVector<DSP::LineSeries> m_pltValues;
  QVector<DSP::MultiLineSeries> m_multipltValues;

  std::vector<LinePush> m_yLinePushes;
  std::vector<LinePush> m_xLinePushes;
  std::vector<TimePush> m_timePushes;
  std::vector<MultiPush> m_multiplotPushes;
  std::vector<SeriesPush> m_fftPushes;
  std::vector<GpsPush> m_gpsPushes;
#ifdef BUILD_COMMERCIAL
  std::vector<SeriesPush> m_waterfallPushes;
  std::vector<Plot3DPush> m_plot3DPushes;
  QVector<DSP::FixedQueue<QVector3D>> m_plot3DRings;

  // Ordered snapshot materialized from the ring at read (render) cadence, off the hotpath
  mutable QVector<DSP::LineSeries3D> m_plotData3D;
  QVector<DSP::AxisData> m_waterfallValues;
#endif

  QMap<int, QTimer*> m_timers;
  QMap<int, int> m_repeatCounters;
  QVector<DataModel::Action> m_actions;
  SerialStudio::WidgetMap m_widgetMap;
  QMap<int, DataModel::Dataset> m_datasets;

  QHash<int, QVector<DataModel::Dataset*>> m_datasetReferences;
  QHash<int, std::vector<ValuePush>> m_valuePushes;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>> m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>> m_widgetDatasets;

  DataModel::Frame m_lastFrame;
  QMap<int, DataModel::Frame> m_sourceRawFrames;

  // Subordinate to m_sourceRawFrames (validated by its contains(sid) check); never cleared alone.
  QHash<int, quint64> m_sourceStructureGen;
};
}  // namespace UI

inline QString FMT_VAL(double val, double min, double max)
{
  auto decPoints = [](double v) {
    double abs = std::abs(v);
    if (DSP::isZero(abs))
      return 2;

    if (abs >= 1e6)
      return 0;

    if (abs >= 1e5)
      return 0;

    if (abs >= 1e4)
      return 0;

    if (abs >= 1e3)
      return 1;

    if (abs >= 1e2)
      return 2;

    if (abs >= 1e1)
      return 2;

    if (abs >= 1.0)
      return 3;

    if (abs >= 1e-1)
      return 4;

    if (abs >= 1e-2)
      return 5;

    if (abs >= 1e-3)
      return 6;

    if (abs >= 1e-4)
      return 7;

    if (abs >= 1e-5)
      return 8;

    if (abs >= 1e-6)
      return 9;

    return 10;
  };

  if (DSP::isZero(min) && DSP::isZero(max))
    return QString::number(val, 'f', decPoints(val));

  else {
    const int p = std::max(decPoints(min), decPoints(max));
    return QString::number(val, 'f', p);
  }
}

inline QString FMT_VAL(double val, const DataModel::Dataset& dataset)
{
  return FMT_VAL(val, dataset.pltMin, dataset.pltMax);
}

inline const DataModel::Group& GET_GROUP(const SerialStudio::DashboardWidget type, int index)
{
  return UI::Dashboard::instance().getGroupWidget(type, index);
}

inline const DataModel::Dataset& GET_DATASET(const SerialStudio::DashboardWidget type, int index)
{
  return UI::Dashboard::instance().getDatasetWidget(type, index);
}

inline bool VALIDATE_WIDGET(const SerialStudio::DashboardWidget type, int index)
{
  return index >= 0 && index < UI::Dashboard::instance().widgetCount(type);
}
