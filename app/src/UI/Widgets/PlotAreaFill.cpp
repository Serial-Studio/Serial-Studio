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
#include <limits>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

// Alpha ramp for the fill gradient (strong at the data extreme, visible at the baseline)
constexpr double kMinAlpha = 0.12;
constexpr double kMaxAlpha = 0.50;

// Caps: columns track item width; the budget bounds the column walk for non-monotonic curves
constexpr int kMaxColumns        = 8192;
constexpr int kMaxBridgedColumns = 262144;

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
             static_cast<unsigned char>(std::lround(color.redF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(color.greenF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(color.blueF() * alpha * 255.0)),
             static_cast<unsigned char>(std::lround(alpha * 255.0)));
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
 * @brief Deepens the saturation of the curve color (hue preserved, achromatic colors
 *        pass through) so soft pastel line colors still produce a vivid fill.
 */
QColor Widgets::PlotAreaFill::vividFillColor(const QColor& color)
{
  float h = 0.0f;
  float s = 0.0f;
  float v = 0.0f;
  float a = 0.0f;
  color.toHsv().getHsvF(&h, &s, &v, &a);
  if (h < 0.0f || s <= 0.0f)
    return color;

  const float deepened = 1.0f - (1.0f - s) * (1.0f - s);
  return QColor::fromHsvF(h, deepened, v, a);
}

/**
 * @brief Sets the fill base color; the rendered fill uses a saturation-deepened variant.
 */
void Widgets::PlotAreaFill::setColor(const QColor& color)
{
  if (m_color == color)
    return;

  m_color     = color;
  m_fillColor = vividFillColor(color);
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
 * @brief Folds one curve sample into the column envelope at the given column.
 */
void Widgets::PlotAreaFill::accumulatePoint(const int col, const double y)
{
  Q_ASSERT(col >= 0);
  Q_ASSERT(static_cast<std::size_t>(col) < m_colMin.size());

  const auto c = static_cast<std::size_t>(col);
  m_colMin[c]  = std::min(m_colMin[c], y);
  m_colMax[c]  = std::max(m_colMax[c], y);
}

/**
 * @brief Folds one linear segment into every column it crosses, clipped to the visible
 *        window. A segment is linear in X, so its extremes per column sit on the column
 *        boundaries; once the budget is spent only the endpoints are accumulated.
 */
void Widgets::PlotAreaFill::bridgeSegment(const double px0,
                                          const double y0,
                                          const double px1,
                                          const double y1,
                                          const double w,
                                          const int cols,
                                          int& budget)
{
  Q_ASSERT(cols >= 1);
  Q_ASSERT(w > 0.0);

  double xa = px0;
  double ya = y0;
  double xb = px1;
  double yb = y1;
  if (xb < xa) {
    std::swap(xa, xb);
    std::swap(ya, yb);
  }

  if (xb < 0.0 || xa >= w)
    return;

  const double dx = xb - xa;
  const double dy = yb - ya;
  if (xa < 0.0) {
    ya += dy * ((0.0 - xa) / dx);
    xa  = 0.0;
  }

  if (xb > w) {
    yb -= dy * ((xb - w) / dx);
    xb  = w;
  }

  const int ca = std::clamp(static_cast<int>(xa), 0, cols - 1);
  const int cb = std::clamp(static_cast<int>(xb), 0, cols - 1);
  if (ca == cb || budget <= 0) {
    accumulatePoint(ca, ya);
    accumulatePoint(cb, yb);
    --budget;
    return;
  }

  const double inv_dx = 1.0 / (xb - xa);
  for (int c = ca; c <= cb; ++c) {
    const double x_enter = std::max(xa, static_cast<double>(c));
    const double x_exit  = std::min(xb, static_cast<double>(c + 1));
    const double y_enter = ya + dy * ((x_enter - xa) * inv_dx);
    const double y_exit  = ya + dy * ((x_exit - xa) * inv_dx);
    accumulatePoint(c, y_enter);
    accumulatePoint(c, y_exit);
  }

  budget -= cb - ca + 1;
}

/**
 * @brief Rasterizes the curve into per-column min/max envelopes. Consecutive finite
 *        points bridge through every column their segment crosses (so sparse curves
 *        stay solid); non-finite points break the run and leave a real gap.
 */
void Widgets::PlotAreaFill::accumulateColumns(const QList<QPointF>& points,
                                              const int cols,
                                              const double w)
{
  Q_ASSERT(cols >= 1);
  Q_ASSERT(cols <= kMaxColumns);

  constexpr double kInf = std::numeric_limits<double>::infinity();
  m_colMin.assign(static_cast<std::size_t>(cols), kInf);
  m_colMax.assign(static_cast<std::size_t>(cols), -kInf);

  const double sx = w / (m_xMax - m_xMin);

  int budget     = kMaxBridgedColumns;
  bool have_prev = false;
  double prev_px = 0.0;
  double prev_y  = 0.0;

  const QPointF* pts    = points.constData();
  const qsizetype count = points.size();
  for (qsizetype i = 0; i < count; ++i) {
    const double x = pts[i].x();
    const double y = pts[i].y();
    if (!std::isfinite(x) || !std::isfinite(y)) {
      have_prev = false;
      continue;
    }

    const double px = (x - m_xMin) * sx;
    if (have_prev)
      bridgeSegment(prev_px, prev_y, px, y, w, cols, budget);
    else if (px >= 0.0 && px < w)
      accumulatePoint(std::min(static_cast<int>(px), cols - 1), y);

    have_prev = true;
    prev_px   = px;
    prev_y    = y;
  }
}

/**
 * @brief Counts the filled columns and finds the per-sign deviation extremes that
 *        anchor the alpha ramp (bipolar fills get a full ramp on each half).
 */
int Widgets::PlotAreaFill::scanColumns(double& refPositive, double& refNegative) const
{
  Q_ASSERT(m_colMin.size() == m_colMax.size());
  Q_ASSERT(m_colMin.size() <= static_cast<std::size_t>(kMaxColumns));

  int filled  = 0;
  refPositive = 0.0;
  refNegative = 0.0;

  const std::size_t cols = m_colMin.size();
  for (std::size_t c = 0; c < cols; ++c) {
    if (m_colMax[c] < m_colMin[c])
      continue;

    ++filled;
    refPositive = std::max(refPositive, m_colMax[c] - m_baseline);
    refNegative = std::max(refNegative, m_baseline - m_colMin[c]);
  }

  refPositive = std::max(refPositive, 1e-12);
  refNegative = std::max(refNegative, 1e-12);
  return filled;
}

/**
 * @brief Emits one six-vertex column per filled column (peak quad above the baseline,
 *        valley quad below, sharing the baseline edge where the alpha pinches to
 *        kMinAlpha), stitched into a single strip with two degenerate vertices.
 */
void Widgets::PlotAreaFill::emitColumns(QSGGeometry::ColoredPoint2D* vertices,
                                        const int vertexCount,
                                        const double w,
                                        const double h,
                                        const double refPositive,
                                        const double refNegative) const
{
  Q_ASSERT(vertices != nullptr);
  Q_ASSERT(refPositive > 0.0);
  Q_ASSERT(refNegative > 0.0);

  const double sy    = h / (m_yMax - m_yMin);
  const float base_y = static_cast<float>((m_yMax - m_baseline) * sy);

  auto ramp = [](const double dev, const double ref) -> double {
    return kMinAlpha + (kMaxAlpha - kMinAlpha) * std::min(1.0, dev / ref);
  };

  int j                  = 0;
  const std::size_t cols = m_colMin.size();
  for (std::size_t c = 0; c < cols; ++c) {
    const double lo = m_colMin[c];
    const double hi = m_colMax[c];
    if (hi < lo)
      continue;

    const double a_top = hi > m_baseline ? ramp(hi - m_baseline, refPositive) : kMinAlpha;
    const double a_bot = lo < m_baseline ? ramp(m_baseline - lo, refNegative) : kMinAlpha;

    const float x0    = static_cast<float>(c);
    const float x1    = static_cast<float>(std::min(static_cast<double>(c + 1), w));
    const float y_top = static_cast<float>((m_yMax - std::max(hi, m_baseline)) * sy);
    const float y_bot = static_cast<float>((m_yMax - std::min(lo, m_baseline)) * sy);

    if (j > 0) {
      vertices[j] = vertices[j - 1];
      ++j;
      setVertexColor(vertices[j++], x0, y_top, m_fillColor, a_top);
    }

    setVertexColor(vertices[j++], x0, y_top, m_fillColor, a_top);
    setVertexColor(vertices[j++], x1, y_top, m_fillColor, a_top);
    setVertexColor(vertices[j++], x0, base_y, m_fillColor, kMinAlpha);
    setVertexColor(vertices[j++], x1, base_y, m_fillColor, kMinAlpha);
    setVertexColor(vertices[j++], x0, y_bot, m_fillColor, a_bot);
    setVertexColor(vertices[j++], x1, y_bot, m_fillColor, a_bot);
  }

  Q_ASSERT(j == vertexCount);
  Q_UNUSED(vertexCount)
}

/**
 * @brief Rebuilds the fill from the per-column envelopes: one O(points) accumulation
 *        pass, then O(item width) vertex emission, so geometry cost is independent of
 *        data density. The per-vertex alpha ramps from kMaxAlpha at the largest
 *        per-sign deviation down to kMinAlpha at the baseline, anchoring the gradient
 *        to the data instead of the axis range.
 */
QSGNode* Widgets::PlotAreaFill::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
  Q_UNUSED(data)

  const double w      = width();
  const double h      = height();
  const double xRange = m_xMax - m_xMin;
  const double yRange = m_yMax - m_yMin;

  const QList<QPointF> points = m_source ? m_source->points() : QList<QPointF>();

  const bool valid =
    points.size() >= 2 && std::isfinite(m_baseline) && xRange > 0 && yRange > 0 && w > 0 && h > 0;
  if (!valid) {
    delete oldNode;
    return nullptr;
  }

  const int cols = std::min(kMaxColumns, static_cast<int>(std::ceil(w)));
  accumulateColumns(points, cols, w);

  double refPositive = 0;
  double refNegative = 0;
  const int filled   = scanColumns(refPositive, refNegative);
  if (filled == 0) {
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

  const int vertexCount = 8 * filled - 2;
  auto* geometry        = node->geometry();
  Q_ASSERT(geometry != nullptr);
  if (geometry->vertexCount() != vertexCount)
    geometry->allocate(vertexCount);

  emitColumns(geometry->vertexDataAsColoredPoint2D(), vertexCount, w, h, refPositive, refNegative);

  node->markDirty(QSGNode::DirtyGeometry);
  return node;
}
