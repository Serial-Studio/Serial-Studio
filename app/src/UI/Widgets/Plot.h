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

#include <QQuickItem>
#include <QVector>
#include <QXYSeries>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

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
  // clang-format on

signals:
  void rangeChanged();
  void sweepChanged();
  void runningChanged();
  void dataSizeChanged();
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
  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] SerialStudio::InterpolationMode interpolationMode() const noexcept;
  [[nodiscard]] const QString& yLabel() const noexcept;
  [[nodiscard]] const QString& xLabel() const noexcept;
  [[nodiscard]] bool timeAxis() const noexcept;
  [[nodiscard]] bool xyPlot() const noexcept;
  [[nodiscard]] bool sweepEnabled() const noexcept;
  [[nodiscard]] double triggerLevel() const noexcept;
  [[nodiscard]] double holdoff() const noexcept;
  [[nodiscard]] double sweepTimebase() const noexcept;
  [[nodiscard]] SerialStudio::SweepMode sweepMode() const noexcept;
  [[nodiscard]] SerialStudio::TriggerEdge triggerEdge() const noexcept;

public slots:
  void draw(QXYSeries* series);
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
  void updateInterpolatedData();
  void clampToVisibleX(double& lo, double& hi) const;
  void resolveXAxis(const DataModel::Dataset& yDataset);

  template<typename Extractor>
  bool computeMinMaxValues(double& min,
                           double& max,
                           const DataModel::Dataset& dataset,
                           const bool addPadding,
                           Extractor extractor);

  static void padDerivedRange(double& min, double& max, const bool addPadding);
  static void applyAxisPadding(double& min, double& max, const bool addPadding);

private:
  int m_index;
  int m_dataW;
  int m_dataH;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  double m_visLoX;
  double m_visHiX;
  QString m_yLabel;
  QString m_xLabel;

  bool m_monotonicData;
  bool m_timeAxis;
  QList<QPointF> m_data;
  QList<QPointF> m_renderData;
  SerialStudio::InterpolationMode m_interpolationMode;

  bool m_sweepEnabled;
  double m_triggerLevel;
  double m_holdoffMs;
  double m_timebaseMs;
  SerialStudio::SweepMode m_sweepMode;
  SerialStudio::TriggerEdge m_triggerEdge;
};
}  // namespace Widgets
