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
  // clang-format on

signals:
  void rangeChanged();
  void runningChanged();
  void dataSizeChanged();

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
  [[nodiscard]] const QString& yLabel() const noexcept;
  [[nodiscard]] const QString& xLabel() const noexcept;

public slots:
  void draw(QXYSeries* series);
  void setDataW(const int width);
  void setDataH(const int height);
  void setRunning(const bool enabled);

private slots:
  void updateData();
  void updateRange();
  void calculateAutoScaleRange();

private:
  template<typename Extractor>
  bool computeMinMaxValues(double& min,
                           double& max,
                           const DataModel::Dataset& dataset,
                           const bool addPadding,
                           Extractor extractor);

  /**
   * @brief Pads a data-derived [min, max] range and rounds to integer bounds.
   */
  static void padDerivedRange(double& min, double& max, const bool addPadding);

  /**
   * @brief Selects the padding strategy for a [min, max] pair before rounding.
   */
  static void applyAxisPadding(double& min, double& max, const bool addPadding);

private:
  int m_index;
  int m_dataW;
  int m_dataH;
  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;
  QString m_yLabel;
  QString m_xLabel;

  bool m_monotonicData;
  QList<QPointF> m_data;
};
}  // namespace Widgets
