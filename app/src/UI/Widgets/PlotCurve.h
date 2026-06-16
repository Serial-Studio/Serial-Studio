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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QColor>
#include <QList>
#include <QPointer>
#include <QPointF>
#include <QQuickItem>
#include <QSGGeometry>
#include <QXYSeries>

namespace Widgets {
/**
 * @brief GPU polyline for plot curves: renders a curve series as one connected mitered
 *        triangle strip per run with per-vertex feathered edges (anti-aliasing without
 *        MSAA), built directly on the scene graph. Replaces the QtGraphs LineSeries
 *        stroke, which re-triangulates a QQuickShape path on the CPU every update.
 */
class PlotCurve : public QQuickItem {
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
  Q_PROPERTY(double lineWidth
             READ lineWidth
             WRITE setLineWidth
             NOTIFY lineWidthChanged)
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
  void lineWidthChanged();

public:
  explicit PlotCurve(QQuickItem* parent = nullptr);

  [[nodiscard]] QXYSeries* source() const noexcept;
  [[nodiscard]] const QColor& color() const noexcept;
  [[nodiscard]] double lineWidth() const noexcept;
  [[nodiscard]] double xMin() const noexcept;
  [[nodiscard]] double xMax() const noexcept;
  [[nodiscard]] double yMin() const noexcept;
  [[nodiscard]] double yMax() const noexcept;

public slots:
  void setSource(QXYSeries* series);
  void setColor(const QColor& color);
  void setLineWidth(const double width);
  void setXMin(const double value);
  void setXMax(const double value);
  void setYMin(const double value);
  void setYMax(const double value);

protected:
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;

private:
  void projectToPixels(const QPointF* pts, const qsizetype count, const double w, const double h);
  [[nodiscard]] qsizetype runLength(const QPointF* pts,
                                    const qsizetype count,
                                    qsizetype& start) const;
  [[nodiscard]] bool runVisible(const QPointF* pts,
                                const qsizetype start,
                                const qsizetype len) const;
  void countRun(const QPointF* px,
                const qsizetype start,
                const qsizetype len,
                qsizetype& vertexCount,
                qsizetype& indexCount) const;
  void countRibbon(const QPointF* pts,
                   const QPointF* px,
                   const qsizetype count,
                   qsizetype& vertexCount,
                   qsizetype& indexCount) const;
  void emitRibbon(QSGGeometry::ColoredPoint2D* vertices,
                  quint32* indices,
                  const int vertexCount,
                  const int indexCount,
                  const QPointF* pts,
                  const QPointF* px,
                  const qsizetype count,
                  const double hw) const;
  void emitRun(QSGGeometry::ColoredPoint2D* vertices,
               quint32* indices,
               int& v,
               int& idx,
               const QPointF* px,
               const qsizetype start,
               const qsizetype len,
               const double hw) const;

private:
  QColor m_color;
  double m_lineWidth;
  double m_xMin;
  double m_xMax;
  double m_yMin;
  double m_yMax;

  QList<QPointF> m_px;
  QPointer<QXYSeries> m_source;
  QMetaObject::Connection m_sourceConnection;
};
}  // namespace Widgets
