/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QBrush>
#  include <QColor>
#  include <QGradient>
#  include <QObject>
#  include <QString>

namespace Widgets {

/**
 * @brief Canvas2D-shaped gradient handle exposed to JS.
 */
class PainterGradient : public QObject {
  Q_OBJECT

public:
  enum class Kind {
    Linear,
    Radial,
    Conic
  };

  explicit PainterGradient(Kind kind, QObject* parent = nullptr);
  ~PainterGradient() override = default;

  void setLinear(qreal x0, qreal y0, qreal x1, qreal y1);
  void setRadial(qreal x0, qreal y0, qreal r0, qreal x1, qreal y1, qreal r1);
  void setConic(qreal cx, qreal cy, qreal startRad);

  [[nodiscard]] QBrush brush() const;

  [[nodiscard]] Kind kind() const noexcept { return m_kind; }

public slots:
  void addColorStop(qreal offset, const QString& color);

private:
  Kind m_kind;
  QGradientStops m_stops;
  qreal m_x0, m_y0, m_x1, m_y1;
  qreal m_r0, m_r1;
  qreal m_startRad;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
