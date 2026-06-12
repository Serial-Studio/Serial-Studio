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

#include "UI/Widgets/PlotAreaFill.h"

#include <algorithm>
#include <cmath>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

// Alpha ramp matching the retired AreaSeries gradients (strong at the data extreme)
constexpr double kMinAlpha = 0.04;
constexpr double kMaxAlpha = 0.40;

/**
 * @brief Encodes a premultiplied vertex color for QSGVertexColorMaterial.
 */
static void setVertexColor(QSGGeometry::ColoredPoint2D& vertex,
                           const float x,
                           const float y,
                           const QColor& color,
                           const double alpha)
{
  Q_ASSERT(alpha >= 0.0);
  Q_ASSERT(alpha <= 1.0);

  vertex.set(x,
             y,
             static_cast<unsigned char>(color.redF() * alpha * 255.0),
             static_cast<unsigned char>(color.greenF() * alpha * 255.0),
             static_cast<unsigned char>(color.blueF() * alpha * 255.0),
             static_cast<unsigned char>(alpha * 255.0));
}

/**
 * @brief Constructs the fill item and enables scene-graph content.
 */
Widgets::PlotAreaFill::PlotAreaFill(QQuickItem* parent)
  : QQuickItem(parent), m_baseline(0), m_xMin(0), m_xMax(1), m_yMin(0), m_yMax(1)
{
  setFlag(ItemHasContents, true);
}

/**
 * @brief Returns the curve series the fill shadows.
 */
QXYSeries* Widgets::PlotAreaFill::source() const noexcept
{
  return m_source.data();
}

/**
 * @brief Returns the fill base color (alpha is supplied by the gradient ramp).
 */
const QColor& Widgets::PlotAreaFill::color() const noexcept
{
  return m_color;
}

/**
 * @brief Returns the value-space baseline the fill hangs from. Named baselineValue
 *        because QQuickItem's baseline anchor line is FINAL and cannot be shadowed.
 */
double Widgets::PlotAreaFill::baselineValue() const noexcept
{
  return m_baseline;
}

/**
 * @brief Returns the lower bound of the visible X window.
 */
double Widgets::PlotAreaFill::xMin() const noexcept
{
  return m_xMin;
}

/**
 * @brief Returns the upper bound of the visible X window.
 */
double Widgets::PlotAreaFill::xMax() const noexcept
{
  return m_xMax;
}

/**
 * @brief Returns the lower bound of the visible Y window.
 */
double Widgets::PlotAreaFill::yMin() const noexcept
{
  return m_yMin;
}

/**
 * @brief Returns the upper bound of the visible Y window.
 */
double Widgets::PlotAreaFill::yMax() const noexcept
{
  return m_yMax;
}

/**
 * @brief Follows the given series; the fill regenerates on every series update().
 */
void Widgets::PlotAreaFill::setSource(QXYSeries* series)
{
  if (m_source.data() == series)
    return;

  if (m_sourceConnection)
    disconnect(m_sourceConnection);

  m_source = series;
  if (m_source)
    m_sourceConnection =
      connect(m_source.data(), &QXYSeries::update, this, &QQuickItem::update, Qt::DirectConnection);

  Q_EMIT sourceChanged();
  update();
}

/**
 * @brief Sets the fill base color.
 */
void Widgets::PlotAreaFill::setColor(const QColor& color)
{
  if (m_color == color)
    return;

  m_color = color;
  Q_EMIT colorChanged();
  update();
}

/**
 * @brief Sets the value-space baseline (range minimum, range maximum or zero).
 */
void Widgets::PlotAreaFill::setBaselineValue(const double value)
{
  if (m_baseline == value)
    return;

  m_baseline = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the lower bound of the visible X window.
 */
void Widgets::PlotAreaFill::setXMin(const double value)
{
  if (m_xMin == value)
    return;

  m_xMin = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the upper bound of the visible X window.
 */
void Widgets::PlotAreaFill::setXMax(const double value)
{
  if (m_xMax == value)
    return;

  m_xMax = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the lower bound of the visible Y window.
 */
void Widgets::PlotAreaFill::setYMin(const double value)
{
  if (m_yMin == value)
    return;

  m_yMin = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Sets the upper bound of the visible Y window.
 */
void Widgets::PlotAreaFill::setYMax(const double value)
{
  if (m_yMax == value)
    return;

  m_yMax = value;
  Q_EMIT rangeChanged();
  update();
}

/**
 * @brief Rebuilds the fill as one triangle strip (two vertices per curve point: the
 *        curve sample and its baseline projection). The per-vertex alpha ramps from
 *        kMaxAlpha at the largest data deviation from the baseline (per sign, so
 *        bipolar fills get a full ramp on each half) down to kMinAlpha at the
 *        baseline, which anchors the gradient to the data instead of the axis range.
 */
QSGNode* Widgets::PlotAreaFill::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
  Q_UNUSED(data)

  const double w      = width();
  const double h      = height();
  const double xRange = m_xMax - m_xMin;
  const double yRange = m_yMax - m_yMin;

  m_points              = m_source ? m_source->points() : QList<QPointF>();
  const qsizetype count = m_points.size();

  const bool valid = count >= 2 && xRange > 0 && yRange > 0 && w > 0 && h > 0;
  if (!valid) {
    delete oldNode;
    return nullptr;
  }

  auto* node = static_cast<QSGGeometryNode*>(oldNode);
  if (!node) {
    node           = new QSGGeometryNode;
    auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new QSGVertexColorMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
  }

  double refPositive = 0;
  double refNegative = 0;
  const QPointF* pts = m_points.constData();
  for (qsizetype i = 0; i < count; ++i) {
    const double d = pts[i].y() - m_baseline;
    if (std::isnan(d))
      continue;

    refPositive = std::max(refPositive, d);
    refNegative = std::max(refNegative, -d);
  }

  refPositive = std::max(refPositive, 1e-12);
  refNegative = std::max(refNegative, 1e-12);

  auto* geometry = node->geometry();
  Q_ASSERT(geometry != nullptr);
  if (geometry->vertexCount() != static_cast<int>(2 * count))
    geometry->allocate(static_cast<int>(2 * count));

  const float baseY = static_cast<float>((1.0 - (m_baseline - m_yMin) / yRange) * h);

  auto* vertices = geometry->vertexDataAsColoredPoint2D();
  Q_ASSERT(vertices != nullptr);
  for (qsizetype i = 0; i < count; ++i) {
    double y       = pts[i].y();
    const double x = pts[i].x();
    if (std::isnan(y))
      y = m_baseline;

    const double d     = y - m_baseline;
    const double ref   = d >= 0 ? refPositive : refNegative;
    const double t     = std::min(1.0, std::abs(d) / ref);
    const double alpha = kMinAlpha + (kMaxAlpha - kMinAlpha) * t;

    const float px = static_cast<float>((x - m_xMin) / xRange * w);
    const float py = static_cast<float>((1.0 - (y - m_yMin) / yRange) * h);

    setVertexColor(vertices[2 * i], px, py, m_color, alpha);
    setVertexColor(vertices[2 * i + 1], px, baseY, m_color, kMinAlpha);
  }

  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
