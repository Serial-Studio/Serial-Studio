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

#ifdef BUILD_COMMERCIAL

#  include <array>
#  include <cmath>
#  include <cstdint>
#  include <functional>
#  include <QConicalGradient>
#  include <QDir>
#  include <QFileInfo>
#  include <QFontMetricsF>
#  include <QImage>
#  include <QImageReader>
#  include <QLinearGradient>
#  include <QPainterPathStroker>
#  include <QPixmap>
#  include <QRadialGradient>
#  include <QRegularExpression>
#  include <QStringList>

#  include "Misc/CommonFonts.h"
#  include "SerialStudio.h"
#  include "UI/Widgets/PainterContext.h"

//--------------------------------------------------------------------------------------------------
// PainterGradient
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty gradient of the given kind.
 */
Widgets::PainterGradient::PainterGradient(Kind kind, QObject* parent)
  : QObject(parent)
  , m_kind(kind)
  , m_x0(0)
  , m_y0(0)
  , m_x1(0)
  , m_y1(0)
  , m_r0(0)
  , m_r1(0)
  , m_startRad(0)
{}

/**
 * @brief Stores the linear-gradient axis endpoints.
 */
void Widgets::PainterGradient::setLinear(qreal x0, qreal y0, qreal x1, qreal y1)
{
  m_x0 = x0;
  m_y0 = y0;
  m_x1 = x1;
  m_y1 = y1;
}

/**
 * @brief Stores the radial-gradient inner and outer circles.
 */
void Widgets::PainterGradient::setRadial(qreal x0, qreal y0, qreal r0, qreal x1, qreal y1, qreal r1)
{
  m_x0 = x0;
  m_y0 = y0;
  m_r0 = r0;
  m_x1 = x1;
  m_y1 = y1;
  m_r1 = r1;
}

/**
 * @brief Stores the conic-gradient centre and start angle.
 */
void Widgets::PainterGradient::setConic(qreal cx, qreal cy, qreal startRad)
{
  m_x0       = cx;
  m_y0       = cy;
  m_startRad = startRad;
}

/**
 * @brief Appends or replaces a (offset, color) stop, sorted by offset.
 */
void Widgets::PainterGradient::addColorStop(qreal offset, const QString& color)
{
  const QColor c = QColor::fromString(color.trimmed());
  if (!c.isValid())
    return;

  const qreal clamped = qBound(0.0, offset, 1.0);
  for (auto& stop : m_stops) {
    if (qFuzzyCompare(stop.first, clamped)) {
      stop.second = c;
      return;
    }
  }

  m_stops.push_back(QGradientStop(clamped, c));
  std::sort(m_stops.begin(), m_stops.end(), [](const QGradientStop& a, const QGradientStop& b) {
    return a.first < b.first;
  });
}

/**
 * @brief Materialises the gradient into a QBrush ready for QPainter binding.
 */
QBrush Widgets::PainterGradient::brush() const
{
  QGradient* g = nullptr;
  if (m_kind == Kind::Linear)
    g = new QLinearGradient(m_x0, m_y0, m_x1, m_y1);
  else if (m_kind == Kind::Radial)
    g = new QRadialGradient(QPointF(m_x1, m_y1), m_r1, QPointF(m_x0, m_y0), m_r0);
  else {
    auto* cg = new QConicalGradient(m_x0, m_y0, qRadiansToDegrees(m_startRad));
    g        = cg;
  }

  g->setStops(m_stops);
  QBrush b(*g);
  delete g;
  return b;
}

//--------------------------------------------------------------------------------------------------
// Gradients / patterns
//--------------------------------------------------------------------------------------------------

/**
 * @brief Allocates a JS-visible linear-gradient handle parented to the context.
 */
Widgets::PainterGradient* Widgets::PainterContext::createLinearGradient(qreal x0,
                                                                        qreal y0,
                                                                        qreal x1,
                                                                        qreal y1)
{
  auto* g = new PainterGradient(PainterGradient::Kind::Linear, this);
  g->setLinear(x0, y0, x1, y1);
  return g;
}

/**
 * @brief Allocates a JS-visible radial-gradient handle parented to the context.
 */
Widgets::PainterGradient* Widgets::PainterContext::createRadialGradient(
  qreal x0, qreal y0, qreal r0, qreal x1, qreal y1, qreal r1)
{
  auto* g = new PainterGradient(PainterGradient::Kind::Radial, this);
  g->setRadial(x0, y0, r0, x1, y1, r1);
  return g;
}

/**
 * @brief Allocates a JS-visible conic-gradient handle parented to the context.
 */
Widgets::PainterGradient* Widgets::PainterContext::createConicGradient(qreal startRad,
                                                                       qreal cx,
                                                                       qreal cy)
{
  auto* g = new PainterGradient(PainterGradient::Kind::Conic, this);
  g->setConic(cx, cy, startRad);
  return g;
}

/**
 * @brief Allocates a JS-visible pattern handle from a sandbox-resolved image path.
 */
Widgets::PainterPattern* Widgets::PainterContext::createPattern(const QString& src,
                                                                const QString& repetition)
{
  const QString resolved = resolveImagePath(src);
  if (resolved.isEmpty())
    return new PainterPattern(QPixmap(), repetition, this);

  return new PainterPattern(QPixmap(resolved), repetition, this);
}

#endif
