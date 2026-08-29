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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <chrono>
#include <QElapsedTimer>
#include <QFont>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <utility>

#include "DataModel/DataBlock.h"
#include "DSP.h"
#include "IO/StreamWorker.h"
#include "SerialStudio.h"
#include "UI/Dashboard/DashboardTools.h"
#include "UI/Dashboard/DashboardViewState.h"
#include "UI/Dashboard/PlotControlBank.h"
#include "UI/Dashboard/ReplaySeekEngine.h"
#include "UI/Dashboard/WidgetMapBuilder.h"
#include "UI/WidgetRegistry.h"

class AppState;
class SessionContext;

namespace CSV {
class Player;
}  // namespace CSV

namespace MDF4 {
class Player;
}  // namespace MDF4

namespace IO {
class PipelineHost;
class ConnectionManager;
}  // namespace IO

namespace DataModel {
class FrameBuilder;
class ProjectModel;
}  // namespace DataModel

#ifdef BUILD_COMMERCIAL
namespace Sessions {
class Player;
}  // namespace Sessions
#endif

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
  Q_PROPERTY(bool showAlignmentGuides
             READ  showAlignmentGuides
             WRITE setShowAlignmentGuides
             NOTIFY showAlignmentGuidesChanged)
  Q_PROPERTY(int layoutMargin
             READ  layoutMargin
             WRITE setLayoutMargin
             NOTIFY layoutMarginChanged)
  Q_PROPERTY(int layoutSpacing
             READ  layoutSpacing
             WRITE setLayoutSpacing
             NOTIFY layoutSpacingChanged)
  Q_PROPERTY(double plotTimeRange
             READ  plotTimeRange
             WRITE setPlotTimeRange
             NOTIFY plotTimeRangeChanged)
  Q_PROPERTY(bool frozen
             READ frozen
             NOTIFY frozenChanged)
  Q_PROPERTY(bool thinningActive
             READ thinningActive
             NOTIFY thinningActiveChanged)
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
  void showAlignmentGuidesChanged();
  void layoutMarginChanged();
  void layoutSpacingChanged();
  void plotTimeRangeChanged();
  void frozenChanged();
  void thinningActiveChanged();
  void displayTitlesChanged();
  void containsCommercialFeaturesChanged();
  void viewStateChanged();

private:
  friend class ::SessionContext;
  explicit Dashboard();
  Dashboard(Dashboard&&)                 = delete;
  Dashboard(const Dashboard&)            = delete;
  Dashboard& operator=(Dashboard&&)      = delete;
  Dashboard& operator=(const Dashboard&) = delete;

public:
  /**
   * @brief Resolved identity of one slot in the DashboardExtension bucket. Every third-party
   *        package shares that single enum value, so the scope that tells a group widget from a
   *        dataset widget comes from the package descriptor, never from the enum: group-scope
   *        slots occupy the first positions, dataset-scope slots follow, and @c bucketIndex
   *        addresses the matching m_widgetGroups / m_widgetDatasets entry.
   */
  struct ExtensionSlot {
    bool valid      = false;
    bool group      = false;
    int bucketIndex = -1;
    QString extensionId;
  };

  // Owned by UI::WidgetMapBuilder, which resolves the fold table pointing into this store
  using DatasetExtremes = UI::DatasetExtremes;

  [[nodiscard]] static Dashboard& instance();

  [[nodiscard]] bool available() const;
  [[nodiscard]] bool streamAvailable() const;
  [[nodiscard]] bool frozen() const;

  [[nodiscard]] bool thinningActive() const noexcept { return m_thinningActive; }

  [[nodiscard]] double plotTimeRange() const noexcept { return m_plotTimeRange; }

  [[nodiscard]] bool pointsWidgetVisible() const;
  [[nodiscard]] bool containsCommercialFeatures() const noexcept;

  [[nodiscard]] int points() const noexcept { return m_points; }

  [[nodiscard]] int totalWidgetCount() const noexcept { return m_widgetCount; }

  [[nodiscard]] bool clockEnabled() const noexcept { return m_tools.clockEnabled(); }

  [[nodiscard]] bool terminalEnabled() const noexcept { return m_tools.terminalEnabled(); }

  [[nodiscard]] bool stopwatchEnabled() const noexcept { return m_tools.stopwatchEnabled(); }

  [[nodiscard]] bool notificationLogEnabled() const noexcept
  {
    return m_tools.notificationLogEnabled();
  }

  [[nodiscard]] int actionCount() const { return m_tools.actionCount(); }

  [[nodiscard]] QVariantList actions() const { return m_tools.actions(); }

  [[nodiscard]] int actionIndexForId(int actionId) const noexcept
  {
    return m_tools.actionIndexForId(actionId);
  }

  [[nodiscard]] int layoutMargin() const noexcept { return m_viewState.layoutMargin(); }

  [[nodiscard]] int layoutSpacing() const noexcept { return m_viewState.layoutSpacing(); }

  [[nodiscard]] bool showActionPanel() const noexcept { return m_viewState.showActionPanel(); }

  [[nodiscard]] bool autoHideToolbar() const noexcept { return m_viewState.autoHideToolbar(); }

  [[nodiscard]] bool showAlignmentGuides() const noexcept
  {
    return m_viewState.showAlignmentGuides();
  }

  [[nodiscard]] QString viewStateJson() const { return m_viewState.viewStateJson(); }

  [[nodiscard]] Q_INVOKABLE QJsonObject globalViewState() const
  {
    return m_viewState.globalViewState();
  }

  [[nodiscard]] Q_INVOKABLE QJsonObject widgetViewState(const QString& widgetId) const
  {
    return m_viewState.widgetViewState(widgetId);
  }

  [[nodiscard]] Q_INVOKABLE bool frameValid() const { return m_lastFrame.groups.size() > 0; }

  [[nodiscard]] Q_INVOKABLE QString formatValue(double val, double min, double max) const;

  [[nodiscard]] Q_INVOKABLE int relativeIndex(const int widgetIndex) const
  {
    const auto it = m_widgetMap.constFind(widgetIndex);
    return it != m_widgetMap.cend() ? it->second : -1;
  }

  [[nodiscard]] Q_INVOKABLE SerialStudio::DashboardWidget widgetType(const int widgetIndex) const
  {
    const auto it = m_widgetMap.constFind(widgetIndex);
    return it != m_widgetMap.cend() ? it->first : SerialStudio::DashboardNoWidget;
  }

  [[nodiscard]] Q_INVOKABLE int widgetCount(const SerialStudio::DashboardWidget widget) const;

  [[nodiscard]] Q_INVOKABLE QString extensionIdAt(const bool group, const int bucketIndex) const
  {
    return m_widgetMapBuilder.extensionIdAt(group, bucketIndex);
  }

  [[nodiscard]] ExtensionSlot extensionSlot(const int relativeIndex) const;
  [[nodiscard]] ExtensionSlot widgetSlot(const SerialStudio::DashboardWidget type,
                                         const int relativeIndex) const;

  [[nodiscard]] const QString& title() const { return m_lastFrame.title; }

  [[nodiscard]] const SerialStudio::WidgetMap& widgetMap() const { return m_widgetMap; }

  [[nodiscard]] int groupIdForUniqueId(int uniqueId) const;
  [[nodiscard]] int groupUniqueIdForGroupId(int groupId) const;

  [[nodiscard]] DatasetExtremes datasetExtremes(int uniqueId) const
  {
    return m_datasetExtremes.value(uniqueId);
  }

  // clang-format off
  [[nodiscard]] const QMap<int, DataModel::Dataset> &datasets() const { return m_datasets; }
  [[nodiscard]] const DataModel::Group &getGroupWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  [[nodiscard]] const DataModel::Dataset &getDatasetWidget(const SerialStudio::DashboardWidget widget, const int index) const;
  // clang-format on

  [[nodiscard]] bool useTimeXAxis(const DataModel::Dataset& dataset) const;
  [[nodiscard]] bool useTimeXAxisGroup(const DataModel::Group& group) const;

  [[nodiscard]] const DataModel::Frame& rawFrame() { return m_lastFrame; }

  [[nodiscard]] const DataModel::Frame& processedFrame() { return m_lastFrame; }

  [[nodiscard]] const DSP::AxisData& fftData(const int index) const;
  [[nodiscard]] const DSP::GpsSeries& gpsSeries(const int index) const;
  [[nodiscard]] const DSP::LineSeries& plotData(const int index) const;
  [[nodiscard]] const DSP::MultiLineSeries& multiplotData(const int index) const;
  [[nodiscard]] const DSP::EnvelopeRing& plotTimeRing(const int index) const;
  [[nodiscard]] const std::vector<DSP::EnvelopeRing>& multiplotTimeRings(const int index) const;

  [[nodiscard]] const DSP::SweepEngine& plotSweep(const int index) const
  {
    return m_plotControls.plotSweep(index);
  }

  [[nodiscard]] const DSP::SweepEngine& multiplotSweep(const int index) const
  {
    return m_plotControls.multiplotSweep(index);
  }

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] const DSP::LineSeries3D& plotData3D(const int index) const;
  [[nodiscard]] const DSP::AxisData& waterfallData(const int index) const;
#endif

  [[nodiscard]] static qint64 replaySeekKey(int sourceId, int uniqueId) noexcept
  {
    return ReplaySeekEngine::seekKey(sourceId, uniqueId);
  }

  [[nodiscard]] QList<std::pair<int, int>> replaySeekSeries() const
  {
    return m_replaySeek.seekSeries();
  }

  void bulkLoadPlotWindow(const QVector<double>& timesSec,
                          const QHash<qint64, QVector<double>>& series);

  [[nodiscard]] bool plotRunning(const int index) const
  {
    return m_plotControls.plotRunning(index);
  }

  [[nodiscard]] bool fftPlotRunning(const int index) const
  {
    return m_plotControls.fftPlotRunning(index);
  }

  [[nodiscard]] bool multiplotRunning(const int index) const
  {
    return m_plotControls.multiplotRunning(index);
  }
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] bool waterfallRunning(const int index) const
  {
    return m_plotControls.waterfallRunning(index);
  }

  void setFftAudioTap(const int index, const bool enabled, const quint32 key);
  void setWaterfallAudioTap(const int index, const bool enabled, const quint32 key);
#endif

public slots:
  void setPoints(const int points);
  void resetData(const bool notify = true);
  void clearPlotData();
  void setShowActionPanel(const bool enabled);
  void setAutoHideToolbar(const bool enabled);
  void setShowAlignmentGuides(const bool enabled);
  void setLayoutMargin(const int margin);
  void setLayoutSpacing(const int spacing);
  void setFrozen(const bool frozen);
  void setPlotTimeRange(const double seconds);
  void setSettingsPersistent(const bool persistent);

  void setClockEnabled(const bool enabled) { m_tools.setClockEnabled(enabled); }

  void setTerminalEnabled(const bool enabled) { m_tools.setTerminalEnabled(enabled); }

  void setStopwatchEnabled(const bool enabled) { m_tools.setStopwatchEnabled(enabled); }

  void setNotificationLogEnabled(const bool enabled) { m_tools.setNotificationLogEnabled(enabled); }

  void activateAction(const int index, const bool guiTrigger = false)
  {
    m_tools.activateAction(index, guiTrigger);
  }

  void setPlotRunning(const int index, const bool enabled)
  {
    m_plotControls.setPlotRunning(index, enabled);
  }

  void setFFTPlotRunning(const int index, const bool enabled)
  {
    m_plotControls.setFFTPlotRunning(index, enabled);
  }

  void setMultiplotRunning(const int index, const bool enabled)
  {
    m_plotControls.setMultiplotRunning(index, enabled);
  }
#ifdef BUILD_COMMERCIAL
  void setWaterfallRunning(const int index, const bool enabled)
  {
    m_plotControls.setWaterfallRunning(index, enabled);
  }
#endif

  void setPlotSweep(const int index,
                    const bool enabled,
                    const double level,
                    const int edge,
                    const int mode,
                    const double holdoff,
                    const double timebase)
  {
    m_plotControls.setPlotSweep(index, enabled, level, edge, mode, holdoff, timebase);
  }

  void setMultiplotSweep(const int index,
                         const bool enabled,
                         const double level,
                         const int edge,
                         const int mode,
                         const double holdoff,
                         const int triggerCurve,
                         const double timebase)
  {
    m_plotControls.setMultiplotSweep(
      index, enabled, level, edge, mode, holdoff, triggerCurve, timebase);
  }

  void armPlotSweep(const int index) { m_plotControls.armPlotSweep(index); }

  void armMultiplotSweep(const int index) { m_plotControls.armMultiplotSweep(index); }

  void setPlotSweepRetention(const int index, const int count)
  {
    m_plotControls.setPlotSweepRetention(index, count);
  }

  void saveWidgetViewState(const QString& widgetId, const QString& key, const QVariant& value);
  void saveGlobalViewState(const QString& key, const QVariant& value);
  void setViewStateJson(const QString& json);
  void clearViewState();

  void applyBlock(const DataModel::DataBlockPtr& block);
  void applyStructureSnapshot(const DataModel::StructureSnapshotPtr& snapshot);
  void updateStreamAvailable();
  void pollThinningState();
  void refreshDisplayTitles();
  void onDisplayTick();

private:
  void connectToolSignals();
  void connectDisplayTimers();
  void connectSessionResets();
  void applyOperationModeDefaults();
  void restorePersistedSettings();
  void connectStreamAvailableInputs();
  void connectViewStateResets(AppState& appState);
  void reconfigureDashboard(const DataModel::Frame& frame);
  [[nodiscard]] DataModel::Frame combineSourceFrames(const DataModel::Frame& seed) const;
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
  void buildLinePushes();
  void configurePlot3DSeries();
  void configureMultiLineSeries();
  void buildMultiplotPushes();

#ifdef BUILD_COMMERCIAL
  void configureWaterfallSeries();
#endif

  void foldExtremes(int sourceId);
  void clearPushTables();

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
   * @brief Pre-resolved stream ingest targets for one dataset uniqueId: widget indexes only
   *        (never raw ring pointers -- a layout rebuild reallocates those), rebuilt lazily
   *        after every reconfigure (spec 0051 T25).
   */
  struct StreamTargets {
    std::vector<int> plotIndexes;
    std::vector<std::pair<int, int>> multiplotCurves;
    std::vector<int> fftIndexes;
#ifdef BUILD_COMMERCIAL
    std::vector<int> waterfallIndexes;
#endif
  };

  [[nodiscard]] const StreamTargets& streamTargetsFor(int uniqueId);
  void applyBlockColumn(const DataModel::BlockColumn& column,
                        const DataModel::DataBlock& block,
                        double baseSec);
  [[nodiscard]] bool applyBlockValues(const DataModel::DataBlock& block, qsizetype index);
  void feedFftFromSamples(const DataModel::BlockColumn& column,
                          const StreamTargets& targets,
                          std::size_t count);
  void feedPlotBlockSweep(int plotIndex,
                          const DataModel::BlockColumn& column,
                          const DataModel::DataBlock& block,
                          double baseSec);
  void feedMultiplotBlockSweep(int groupIndex, const DataModel::DataBlock& block, double baseSec);
  void growTimeRings();
  void resetPlotClocks();
  void growTimeRing(DSP::EnvelopeRing& ring, int sourceId, double windowSec);
  void drainStructureSnapshots();
  void drainBlockRing(const QElapsedTimer& clock, qint64 budgetNs);
  double advancePlotClock(int sourceId,
                          const std::chrono::steady_clock::time_point& ts,
                          double blockSpanSec = 0.0);

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

  // Resolved once at construction: Dashboard is built last, so no method body resolves a singleton.
  AppState* m_appState;
  WidgetRegistry* m_widgetRegistry;
  DataModel::FrameBuilder* m_frameBuilder;
  DataModel::ProjectModel* m_projectModel;

  QSettings m_settings;
  qint64 m_drainBudgetNs;
  int m_points;
  int m_widgetCount;
  bool m_updateRequired;
  bool m_thinningActive;

  bool m_updateRetryInProgress;

  // False while cached Dataset*/ring pointers are stale (post-reset, pre-reconfigure)
  bool m_layoutValid;
  bool m_streamAvailable;

  double m_plotTimeRange;
  double m_plotDisplayTimeSec;
  QMap<int, PlotClock> m_plotClocks;

  DSP::AxisData m_pltXAxis;
  DSP::AxisData m_pltNullY;
  DSP::AxisData m_multipltXAxis;

  QMap<int, DSP::AxisData> m_xAxisData;
  QMap<int, DSP::AxisData> m_yAxisData;
  QMap<int, DSP::EnvelopeRing> m_plotTimeRings;
  QMap<int, std::vector<DSP::EnvelopeRing>> m_multiplotTimeRings;
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

  SerialStudio::WidgetMap m_widgetMap;
  QMap<int, DataModel::Dataset> m_datasets;

  QHash<int, QVector<DataModel::Dataset*>> m_datasetReferences;
  QHash<int, std::vector<ValuePush>> m_valuePushes;
  QHash<int, std::vector<ExtremePush>> m_extremePushes;
  QMap<int, DatasetExtremes> m_datasetExtremes;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>> m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>> m_widgetDatasets;

  // Owning package ids, index-aligned with the DashboardExtension entries of the two maps above
  QVector<QString> m_extensionGroupIds;
  QVector<QString> m_extensionDatasetIds;

  DataModel::Frame m_lastFrame;
  QMap<int, DataModel::Frame> m_sourceRawFrames;
  QHash<int, StreamTargets> m_streamTargets;

  // Curve-index -> column scratch for one multiplot sweep; reused so a tick allocates nothing
  std::vector<const DataModel::BlockColumn*> m_streamSweepCurves;

  // Subordinate to m_sourceRawFrames (validated by its contains(sid) check); never cleared alone.
  QHash<int, quint64> m_sourceStructureGen;
  QHash<int, quint64> m_quarantinedSources;

  // Declared last: these bind the members above by reference, in declaration order
  DashboardTools m_tools;
  DashboardViewState m_viewState;
  PlotControlBank m_plotControls;
  ReplaySeekEngine m_replaySeek;
  WidgetMapBuilder m_widgetMapBuilder;
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
  static auto& dashboard = UI::Dashboard::instance();
  return dashboard.getGroupWidget(type, index);
}

inline const DataModel::Dataset& GET_DATASET(const SerialStudio::DashboardWidget type, int index)
{
  static auto& dashboard = UI::Dashboard::instance();
  return dashboard.getDatasetWidget(type, index);
}

inline bool VALIDATE_WIDGET(const SerialStudio::DashboardWidget type, int index)
{
  static auto& dashboard = UI::Dashboard::instance();
  return index >= 0 && index < dashboard.widgetCount(type);
}
