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
#include <QXYSeries>

namespace Widgets {
/**
 * @brief GPU area-under-curve fill: renders the region between a curve series and a
 *        value-space baseline as a single triangle strip with per-vertex gradient
 *        colors. Replaces the QtGraphs AreaSeries, which re-triangulates a shape
 *        path on the CPU every update and stalls on dense (audio-rate) curves.
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
  QColor m_color;
  double m_baseline;
  double m_xMin;
  double m_xMax;
  double m_yMin;
  double m_yMax;

  QList<QPointF> m_points;
  QPointer<QXYSeries> m_source;
  QMetaObject::Connection m_sourceConnection;
};
}  // namespace Widgets
