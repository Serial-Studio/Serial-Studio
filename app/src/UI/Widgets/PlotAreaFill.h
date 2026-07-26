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

#include <QColor>
#include <QList>
#include <QPointer>
#include <QPointF>
#include <QQuickItem>
#include <QSGGeometry>
#include <QXYSeries>
#include <vector>

namespace Widgets {
/**
 * @brief GPU area-under-curve fill: rasterizes the curve into per-pixel-column min/max
 *        envelopes, rendered as one baseline-anchored column pair per filled column in
 *        a single per-vertex-gradient triangle strip. Geometry is O(item width), not
 *        O(points), so audio-rate curves cost the same as sparse ones.
 */
class PlotAreaFill : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QXYSeries* source
             READ source
             WRITE setSource
             NOTIFY sourceChanged)
  Q_PROPERTY(QColor color
             READ color
             WRITE setColor
             NOTIFY colorChanged)
  Q_PROPERTY(double baselineValue
             READ baselineValue
             WRITE setBaselineValue
             NOTIFY rangeChanged)
  Q_PROPERTY(double xMin
             READ xMin
             WRITE setXMin
             NOTIFY rangeChanged)
  Q_PROPERTY(double xMax
             READ xMax
             WRITE setXMax
             NOTIFY rangeChanged)
  Q_PROPERTY(double yMin
             READ yMin
             WRITE setYMin
             NOTIFY rangeChanged)
  Q_PROPERTY(double yMax
             READ yMax
             WRITE setYMax
             NOTIFY rangeChanged)
  // clang-format on

signals:
  void rangeChanged();
  void colorChanged();
  void sourceChanged();

public:
  explicit PlotAreaFill(QQuickItem* parent = nullptr);

  [[nodiscard]] QXYSeries* source() const noexcept;
  [[nodiscard]] const QColor& color() const noexcept;
  [[nodiscard]] double baselineValue() const noexcept;
  [[nodiscard]] double xMin() const noexcept;
  [[nodiscard]] double xMax() const noexcept;
  [[nodiscard]] double yMin() const noexcept;
  [[nodiscard]] double yMax() const noexcept;

public slots:
  void setSource(QXYSeries* series);
  void setColor(const QColor& color);
  void setBaselineValue(const double value);
  void setXMin(const double value);
  void setXMax(const double value);
  void setYMin(const double value);
  void setYMax(const double value);

protected:
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;

private:
  [[nodiscard]] static QColor vividFillColor(const QColor& color);

  void accumulatePoint(const int col, const double y);
  [[nodiscard]] int scanColumns(double& refPositive, double& refNegative) const;
  void accumulateColumns(const QList<QPointF>& points, int cols, const double w);
  void bridgeSegment(const double px0,
                     const double y0,
                     const double px1,
                     const double y1,
                     const double w,
                     const int cols,
                     int& budget);
  void emitColumns(QSGGeometry::ColoredPoint2D* vertices,
                   const int vertexCount,
                   const double w,
                   const double h,
                   double refPositive,
                   double refNegative) const;

private:
  QColor m_color;
  QColor m_fillColor;
  double m_baseline;
  double m_xMin;
  double m_xMax;
  double m_yMin;
  double m_yMax;

  std::vector<double> m_colMin;
  std::vector<double> m_colMax;
  QPointer<QXYSeries> m_source;
  QMetaObject::Connection m_sourceConnection;
};
}  // namespace Widgets
