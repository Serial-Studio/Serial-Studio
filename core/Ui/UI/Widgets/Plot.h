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

#include <QQuickItem>
#include <QVector>
#include <QXYSeries>

#include "DataModel/Frame.h"
#include "DSP.h"
#include "SerialStudio.h"
#include "UI/Widgets/PlotBase.h"

namespace UI {
class Dashboard;
}  // namespace UI

namespace Widgets {
/**
 * @brief Real-time 2D plotting widget for visualizing time-series data.
 */
class Plot : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool running
             READ running
             WRITE setRunning
             NOTIFY runningChanged)
  Q_PROPERTY(int dataW
             READ dataW
             WRITE setDataW
             NOTIFY dataSizeChanged)
  Q_PROPERTY(int dataH
             READ dataH
             WRITE setDataH
             NOTIFY dataSizeChanged)
  Q_PROPERTY(SerialStudio::InterpolationMode interpolationMode
             READ interpolationMode
             WRITE setInterpolationMode
             NOTIFY interpolationModeChanged)
  Q_PROPERTY(double minX
             READ minX
             NOTIFY rangeChanged)
  Q_PROPERTY(double maxX
             READ maxX
             NOTIFY rangeChanged)
  Q_PROPERTY(double minY
             READ minY
             NOTIFY rangeChanged)
  Q_PROPERTY(double maxY
             READ maxY
             NOTIFY rangeChanged)
  Q_PROPERTY(bool dataBipolar
             READ dataBipolar
             NOTIFY rangeChanged)
  Q_PROPERTY(double dataMaxY
             READ dataMaxY
             NOTIFY rangeChanged)
  Q_PROPERTY(bool dataFlatZero
             READ dataFlatZero
             NOTIFY rangeChanged)
  Q_PROPERTY(QString yLabel
             READ yLabel
             CONSTANT)
  Q_PROPERTY(QString xLabel
             READ xLabel
             CONSTANT)
  Q_PROPERTY(bool timeAxis
             READ timeAxis
             CONSTANT)
  Q_PROPERTY(bool xyPlot
             READ xyPlot
             CONSTANT)
  Q_PROPERTY(bool logX
             READ logX
             CONSTANT)
  Q_PROPERTY(bool logY
             READ logY
             CONSTANT)
  Q_PROPERTY(bool sweepEnabled
             READ sweepEnabled
             WRITE setSweepEnabled
             NOTIFY sweepChanged)
  Q_PROPERTY(double triggerLevel
             READ triggerLevel
             WRITE setTriggerLevel
             NOTIFY sweepChanged)
  Q_PROPERTY(double holdoff
             READ holdoff
             WRITE setHoldoff
             NOTIFY sweepChanged)
  Q_PROPERTY(double sweepTimebase
             READ sweepTimebase
             WRITE setSweepTimebase
             NOTIFY sweepChanged)
  Q_PROPERTY(SerialStudio::SweepMode sweepMode
             READ sweepMode
             WRITE setSweepMode
             NOTIFY sweepChanged)
  Q_PROPERTY(SerialStudio::TriggerEdge triggerEdge
             READ triggerEdge
             WRITE setTriggerEdge
             NOTIFY sweepChanged)
  Q_PROPERTY(int sweepRetention
             READ sweepRetention
             WRITE setSweepRetention
             NOTIFY sweepSegmentsChanged)
  Q_PROPERTY(int sweepSegmentCount
             READ sweepSegmentCount
             NOTIFY sweepSegmentsChanged)
  Q_PROPERTY(int sweepSegmentCapacity
             READ sweepSegmentCapacity
             NOTIFY sweepSegmentsChanged)
  // clang-format on

signals:
  void rangeChanged();
  void sweepChanged();
  void runningChanged();
  void dataSizeChanged();
  void sweepSegmentsChanged();
  void interpolationModeChanged();

public:
  explicit Plot(const int index = -1, QQuickItem* parent = nullptr);

  ~Plot()
  {
    m_data.clear();
    m_data.squeeze();
  }

  [[nodiscard]] int dataW() const noexcept;
  [[nodiscard]] int dataH() const noexcept;
  [[nodiscard]] double minX() const noexcept;
  [[nodiscard]] double maxX() const noexcept;
  [[nodiscard]] double minY() const noexcept;
  [[nodiscard]] double maxY() const noexcept;
  [[nodiscard]] bool dataBipolar() const noexcept;
  [[nodiscard]] double dataMaxY() const noexcept;
  [[nodiscard]] bool dataFlatZero() const noexcept;
  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] SerialStudio::InterpolationMode interpolationMode() const noexcept;
  [[nodiscard]] const QString& yLabel() const noexcept;
  [[nodiscard]] const QString& xLabel() const noexcept;
  [[nodiscard]] bool timeAxis() const noexcept;
  [[nodiscard]] bool xyPlot() const noexcept;
  [[nodiscard]] bool logX() const noexcept;
  [[nodiscard]] bool logY() const noexcept;
  [[nodiscard]] bool sweepEnabled() const noexcept;
  [[nodiscard]] double triggerLevel() const noexcept;
  [[nodiscard]] double holdoff() const noexcept;
  [[nodiscard]] double sweepTimebase() const noexcept;
  [[nodiscard]] SerialStudio::SweepMode sweepMode() const noexcept;
  [[nodiscard]] SerialStudio::TriggerEdge triggerEdge() const noexcept;
  [[nodiscard]] int sweepRetention() const noexcept;
  [[nodiscard]] int sweepSegmentCount() const;
  [[nodiscard]] int sweepSegmentCapacity() const;

public slots:
  void draw(QXYSeries* series);
  void drawSegment(QXYSeries* series, const int index);
  void setSweepRetention(const int count);
  void setDataW(const int width);
  void setDataH(const int height);
  void setRunning(const bool enabled);
  void setVisibleXWindow(const double lo, const double hi);
  void setInterpolationMode(SerialStudio::InterpolationMode mode);

  void armSweep();
  void setSweepEnabled(const bool enabled);
  void setTriggerLevel(const double level);
  void setHoldoff(const double milliseconds);
  void setSweepTimebase(const double milliseconds);
  void setSweepMode(const SerialStudio::SweepMode mode);
  void setTriggerEdge(const SerialStudio::TriggerEdge edge);

private slots:
  void updateData();
  void updateRange();
  void calculateAutoScaleRange();

private:
  void pushSweepConfig();
  bool updateDataExtremes(const DataModel::Dataset& dataset);
  void updateInterpolatedData();
  void applyLogYToRender();
  void applyDatasetXRange(const DataModel::Dataset& dx);
  void resolveXAxis(const DataModel::Dataset& yDataset);

  template<int kLane>
  bool computeMinMaxValues(double& min,
                           double& max,
                           const DataModel::Dataset& dataset,
                           const bool addPadding,
                           const bool logAxis);

  static void padDerivedRange(double& min, double& max, const bool addPadding, int& stepIndex);
  static void applyAxisPadding(double& min, double& max, const bool addPadding);

private:
  UI::Dashboard& m_dashboard;
  int m_index;
  int m_dataW;
  int m_dataH;
  int m_xStepIndex;
  int m_yStepIndex;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  double m_dataMinY;
  double m_dataMaxY;
  QString m_yLabel;
  QString m_xLabel;

  bool m_monotonicData;
  bool m_timeAxis;
  bool m_logX;
  bool m_logY;
  QList<QPointF> m_data;
  QList<QPointF> m_renderData;

  PlotBase m_base;

  int m_sweepRetention;
  int m_lastSegmentCount;
  QList<QPointF> m_segmentScratch;
};
}  // namespace Widgets
