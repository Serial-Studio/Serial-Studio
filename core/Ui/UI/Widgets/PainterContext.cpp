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

#  include "UI/Widgets/PainterContext.h"

#  include <cmath>
#  include <functional>
#  include <QFileInfo>
#  include <QFontMetricsF>
#  include <QImage>
#  include <QPainterPathStroker>
#  include <QPixmap>

#  include "Misc/CommonFonts.h"
#  include "UI/Widgets/Painter/PainterBlur.h"
#  include "UI/Widgets/Painter/PainterEnums.h"
#  include "UI/Widgets/Painter/PainterFont.h"
#  include "UI/Widgets/Painter/PainterGeometry.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initialises the context with sensible Canvas2D defaults.
 */
Widgets::PainterContext::PainterContext(QObject* parent)
  : QObject(parent)
  , m_commonFonts(Misc::CommonFonts::instance())
  , m_painter(nullptr)
  , m_width(0.0)
  , m_height(0.0)
{
  const QString defaultFam = m_commonFonts.widgetFontFamily();

  m_state.fillBrush             = QBrush(Qt::black);
  m_state.strokePen             = QPen(Qt::black, 1.0);
  m_state.fillSpec              = QStringLiteral("#000000");
  m_state.strokeSpec            = QStringLiteral("#000000");
  m_state.font                  = QFont(defaultFam, 10);
  m_state.fontSpecCached        = QStringLiteral("10px ") + defaultFam;
  m_state.textAlign             = QStringLiteral("start");
  m_state.textBaseline          = QStringLiteral("alphabetic");
  m_state.globalAlpha           = 1.0;
  m_state.compositionMode       = QPainter::CompositionMode_SourceOver;
  m_state.compositionModeName   = QStringLiteral("source-over");
  m_state.shadowColor           = QColor(0, 0, 0, 0);
  m_state.shadowBlur            = 0.0;
  m_state.shadowOffsetX         = 0.0;
  m_state.shadowOffsetY         = 0.0;
  m_state.lineDashOffset        = 0.0;
  m_state.imageSmoothing        = true;
  m_state.imageSmoothingQuality = QStringLiteral("low");
  m_state.miterLimit            = 10.0;
  m_state.strokePen.setMiterLimit(m_state.miterLimit);
}

//--------------------------------------------------------------------------------------------------
// Frame lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds a QPainter for the duration of a single user paint() call.
 */
void Widgets::PainterContext::beginFrame(QPainter* painter, qreal width, qreal height)
{
  if (!painter) [[unlikely]]
    return;

  m_painter = painter;
  m_width   = width;
  m_height  = height;
  m_path    = QPainterPath();

  m_painter->setRenderHint(QPainter::Antialiasing, true);
  m_painter->setRenderHint(QPainter::SmoothPixmapTransform, m_state.imageSmoothing);
  m_painter->setRenderHint(QPainter::TextAntialiasing, true);
  m_painter->setBrush(m_state.fillBrush);
  m_painter->setPen(m_state.strokePen);
  m_painter->setFont(m_state.font);
  m_painter->setOpacity(m_state.globalAlpha);
  m_painter->setCompositionMode(m_state.compositionMode);
}

/**
 * @brief Releases the active QPainter binding.
 */
void Widgets::PainterContext::endFrame()
{
  m_painter = nullptr;
  m_width   = 0.0;
  m_height  = 0.0;
  m_stateStack.clear();
}

/**
 * @brief Updates the project directory used to resolve relative drawImage paths.
 */
void Widgets::PainterContext::setProjectDirectory(const QString& dir)
{
  m_projectDir = dir;
}

//--------------------------------------------------------------------------------------------------
// Style getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current fill style as a JS value (string OR gradient/pattern object).
 */
QJSValue Widgets::PainterContext::fillStyle() const
{
  if (m_state.fillStyleValue.isUndefined() || m_state.fillStyleValue.isNull())
    return QJSValue(m_state.fillSpec);

  return m_state.fillStyleValue;
}

/**
 * @brief Returns the current stroke style as a JS value (string OR gradient/pattern object).
 */
QJSValue Widgets::PainterContext::strokeStyle() const
{
  if (m_state.strokeStyleValue.isUndefined() || m_state.strokeStyleValue.isNull())
    return QJSValue(m_state.strokeSpec);

  return m_state.strokeStyleValue;
}

/**
 * @brief Returns the current line width in pixels.
 */
qreal Widgets::PainterContext::lineWidth() const
{
  return m_state.strokePen.widthF();
}

/**
 * @brief Returns the current line cap style as a Canvas2D name.
 */
QString Widgets::PainterContext::lineCap() const
{
  return PainterEnums::unmapLineCap(m_state.strokePen.capStyle());
}

/**
 * @brief Returns the current line join style as a Canvas2D name.
 */
QString Widgets::PainterContext::lineJoin() const
{
  return PainterEnums::unmapLineJoin(m_state.strokePen.joinStyle());
}

/**
 * @brief Returns the current miter limit.
 */
qreal Widgets::PainterContext::miterLimit() const
{
  return m_state.miterLimit;
}

/**
 * @brief Returns the current line-dash phase offset.
 */
qreal Widgets::PainterContext::lineDashOffset() const
{
  return m_state.lineDashOffset;
}

/**
 * @brief Returns the cached font spec string.
 */
QString Widgets::PainterContext::fontSpec() const
{
  return m_state.fontSpecCached;
}

/**
 * @brief Returns the current text horizontal alignment.
 */
QString Widgets::PainterContext::textAlign() const
{
  return m_state.textAlign;
}

/**
 * @brief Returns the current text vertical baseline.
 */
QString Widgets::PainterContext::textBaseline() const
{
  return m_state.textBaseline;
}

/**
 * @brief Returns the current global alpha multiplier.
 */
qreal Widgets::PainterContext::globalAlpha() const
{
  return m_state.globalAlpha;
}

/**
 * @brief Returns the current Canvas2D composition operation name.
 */
QString Widgets::PainterContext::globalCompositeOperation() const
{
  return m_state.compositionModeName;
}

/**
 * @brief Returns the current shadow color spec.
 */
QString Widgets::PainterContext::shadowColor() const
{
  return m_state.shadowColor.name(QColor::HexArgb);
}

/**
 * @brief Returns the current shadow blur radius.
 */
qreal Widgets::PainterContext::shadowBlur() const
{
  return m_state.shadowBlur;
}

/**
 * @brief Returns the current shadow horizontal offset.
 */
qreal Widgets::PainterContext::shadowOffsetX() const
{
  return m_state.shadowOffsetX;
}

/**
 * @brief Returns the current shadow vertical offset.
 */
qreal Widgets::PainterContext::shadowOffsetY() const
{
  return m_state.shadowOffsetY;
}

/**
 * @brief Returns whether image smoothing is enabled.
 */
bool Widgets::PainterContext::imageSmoothingEnabled() const
{
  return m_state.imageSmoothing;
}

/**
 * @brief Returns the requested image-smoothing quality string.
 */
QString Widgets::PainterContext::imageSmoothingQuality() const
{
  return m_state.imageSmoothingQuality;
}

//--------------------------------------------------------------------------------------------------
// Style setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the fill style. Accepts a CSS color string OR a gradient/pattern object.
 */
void Widgets::PainterContext::setFillStyle(const QJSValue& value)
{
  if (value.isString()) {
    const QString spec = value.toString();
    const QColor c     = parseColor(spec);
    if (!c.isValid())
      return;

    m_state.fillSpec       = spec;
    m_state.fillStyleValue = value;
    m_state.fillBrush      = QBrush(c);
    rebindFillBrush();
    return;
  }

  if (value.isQObject()) {
    if (auto* g = qobject_cast<PainterGradient*>(value.toQObject())) {
      m_state.fillBrush      = g->brush();
      m_state.fillStyleValue = value;
      rebindFillBrush();
      return;
    }
    if (auto* p = qobject_cast<PainterPattern*>(value.toQObject())) {
      m_state.fillBrush      = p->brush();
      m_state.fillStyleValue = value;
      rebindFillBrush();
      return;
    }
  }
}

/**
 * @brief Updates the stroke style. Accepts a CSS color string OR a gradient/pattern object.
 */
void Widgets::PainterContext::setStrokeStyle(const QJSValue& value)
{
  if (value.isString()) {
    const QString spec = value.toString();
    const QColor c     = parseColor(spec);
    if (!c.isValid())
      return;

    m_state.strokeSpec       = spec;
    m_state.strokeStyleValue = value;
    m_state.strokePen.setColor(c);
    m_state.strokePen.setBrush(QBrush(c));
    rebindStrokeBrush();
    return;
  }

  if (value.isQObject()) {
    if (auto* g = qobject_cast<PainterGradient*>(value.toQObject())) {
      m_state.strokePen.setBrush(g->brush());
      m_state.strokeStyleValue = value;
      rebindStrokeBrush();
      return;
    }
    if (auto* p = qobject_cast<PainterPattern*>(value.toQObject())) {
      m_state.strokePen.setBrush(p->brush());
      m_state.strokeStyleValue = value;
      rebindStrokeBrush();
      return;
    }
  }
}

/**
 * @brief Updates the line width.
 */
void Widgets::PainterContext::setLineWidth(qreal w)
{
  if (w <= 0.0)
    return;

  m_state.strokePen.setWidthF(w);

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Updates the line cap style.
 */
void Widgets::PainterContext::setLineCap(const QString& cap)
{
  m_state.strokePen.setCapStyle(PainterEnums::mapLineCap(cap));

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Updates the line join style.
 */
void Widgets::PainterContext::setLineJoin(const QString& join)
{
  m_state.strokePen.setJoinStyle(PainterEnums::mapLineJoin(join));

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Updates the miter limit applied to mitered joins.
 */
void Widgets::PainterContext::setMiterLimit(qreal limit)
{
  if (limit <= 0.0)
    return;

  m_state.miterLimit = limit;
  m_state.strokePen.setMiterLimit(limit);

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Updates the dash phase offset.
 */
void Widgets::PainterContext::setLineDashOffset(qreal offset)
{
  m_state.lineDashOffset = offset;
  applyDashToPen();
}

/**
 * @brief Updates the active font from a CSS-like font shorthand.
 */
void Widgets::PainterContext::setFontSpec(const QString& spec)
{
  const QFont f          = PainterFont::parseFontSpec(spec, m_commonFonts);
  m_state.font           = f;
  m_state.fontSpecCached = spec;

  if (m_painter)
    m_painter->setFont(m_state.font);
}

/**
 * @brief Sets the horizontal anchor used by fillText/strokeText.
 */
void Widgets::PainterContext::setTextAlign(const QString& align)
{
  m_state.textAlign = align;
}

/**
 * @brief Sets the vertical baseline used by fillText/strokeText.
 */
void Widgets::PainterContext::setTextBaseline(const QString& baseline)
{
  m_state.textBaseline = baseline;
}

/**
 * @brief Updates the global alpha multiplier (clamped to [0, 1]).
 */
void Widgets::PainterContext::setGlobalAlpha(qreal a)
{
  m_state.globalAlpha = qBound(0.0, a, 1.0);

  if (m_painter)
    m_painter->setOpacity(m_state.globalAlpha);
}

/**
 * @brief Updates the Canvas2D composition operation.
 */
void Widgets::PainterContext::setGlobalCompositeOperation(const QString& op)
{
  m_state.compositionModeName = op;
  m_state.compositionMode     = PainterEnums::mapComposite(op);

  if (m_painter)
    m_painter->setCompositionMode(m_state.compositionMode);
}

/**
 * @brief Updates the shadow color spec.
 */
void Widgets::PainterContext::setShadowColor(const QString& spec)
{
  const QColor c = parseColor(spec);
  if (!c.isValid())
    return;

  m_state.shadowColor = c;
}

/**
 * @brief Updates the shadow blur radius.
 */
void Widgets::PainterContext::setShadowBlur(qreal blur)
{
  m_state.shadowBlur = qMax<qreal>(0.0, blur);
}

/**
 * @brief Updates the shadow horizontal offset.
 */
void Widgets::PainterContext::setShadowOffsetX(qreal dx)
{
  m_state.shadowOffsetX = dx;
}

/**
 * @brief Updates the shadow vertical offset.
 */
void Widgets::PainterContext::setShadowOffsetY(qreal dy)
{
  m_state.shadowOffsetY = dy;
}

/**
 * @brief Toggles bilinear image smoothing.
 */
void Widgets::PainterContext::setImageSmoothingEnabled(bool enabled)
{
  m_state.imageSmoothing = enabled;
  applyImageSmoothing();
}

/**
 * @brief Records the requested smoothing-quality hint (display-only; Qt has one filter).
 */
void Widgets::PainterContext::setImageSmoothingQuality(const QString& quality)
{
  m_state.imageSmoothingQuality = quality;
}

//--------------------------------------------------------------------------------------------------
// State stack + transforms
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes the full drawing state (painter + Canvas2D style) onto the stack.
 */
void Widgets::PainterContext::save()
{
  if (!active())
    return;

  m_painter->save();
  m_stateStack.push_back(m_state);
}

/**
 * @brief Pops the full drawing state, reverting both painter and Canvas2D style.
 */
void Widgets::PainterContext::restore()
{
  if (!active() || m_stateStack.empty())
    return;

  m_painter->restore();
  m_state = m_stateStack.back();
  m_stateStack.pop_back();

  m_painter->setRenderHint(QPainter::SmoothPixmapTransform, m_state.imageSmoothing);
  m_painter->setBrush(m_state.fillBrush);
  m_painter->setPen(m_state.strokePen);
  m_painter->setFont(m_state.font);
  m_painter->setOpacity(m_state.globalAlpha);
  m_painter->setCompositionMode(m_state.compositionMode);
}

/**
 * @brief Translates the painter origin by (x, y).
 */
void Widgets::PainterContext::translate(qreal x, qreal y)
{
  if (!active())
    return;

  m_painter->translate(x, y);
}

/**
 * @brief Rotates the painter by the given angle in radians.
 */
void Widgets::PainterContext::rotate(qreal radians)
{
  if (!active())
    return;

  m_painter->rotate(qRadiansToDegrees(radians));
}

/**
 * @brief Scales the painter by (sx, sy).
 */
void Widgets::PainterContext::scale(qreal sx, qreal sy)
{
  if (!active())
    return;

  m_painter->scale(sx, sy);
}

/**
 * @brief Multiplies the current transform by a Canvas2D 2x3 affine matrix.
 */
void Widgets::PainterContext::transform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
  if (!active())
    return;

  const QTransform t(a, b, c, d, e, f);
  m_painter->setWorldTransform(t * m_painter->worldTransform());
}

/**
 * @brief Replaces the current transform with the given Canvas2D 2x3 affine matrix.
 */
void Widgets::PainterContext::setTransform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
  if (!active())
    return;

  m_painter->setWorldTransform(QTransform(a, b, c, d, e, f));
}

/**
 * @brief Resets the painter transform to identity.
 */
void Widgets::PainterContext::resetTransform()
{
  if (!active())
    return;

  m_painter->resetTransform();
}

/**
 * @brief Returns the current world transform as a {a, b, c, d, e, f} map.
 */
QVariantMap Widgets::PainterContext::getTransform() const
{
  QVariantMap m;
  if (!active()) {
    m[QStringLiteral("a")] = 1.0;
    m[QStringLiteral("b")] = 0.0;
    m[QStringLiteral("c")] = 0.0;
    m[QStringLiteral("d")] = 1.0;
    m[QStringLiteral("e")] = 0.0;
    m[QStringLiteral("f")] = 0.0;
    return m;
  }

  const QTransform t     = m_painter->worldTransform();
  m[QStringLiteral("a")] = t.m11();
  m[QStringLiteral("b")] = t.m12();
  m[QStringLiteral("c")] = t.m21();
  m[QStringLiteral("d")] = t.m22();
  m[QStringLiteral("e")] = t.dx();
  m[QStringLiteral("f")] = t.dy();
  return m;
}

//--------------------------------------------------------------------------------------------------
// Paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts a new path, discarding any previous subpaths.
 */
void Widgets::PainterContext::beginPath()
{
  m_path = QPainterPath();
}

/**
 * @brief Closes the current subpath with a line to its starting point.
 */
void Widgets::PainterContext::closePath()
{
  m_path.closeSubpath();
}

/**
 * @brief Moves the path cursor to (x, y) without drawing.
 */
void Widgets::PainterContext::moveTo(qreal x, qreal y)
{
  m_path.moveTo(x, y);
}

/**
 * @brief Adds a straight line segment from the cursor to (x, y).
 */
void Widgets::PainterContext::lineTo(qreal x, qreal y)
{
  m_path.lineTo(x, y);
}

/**
 * @brief Adds a closed rectangle subpath at (x, y) with size (w, h).
 */
void Widgets::PainterContext::rect(qreal x, qreal y, qreal w, qreal h)
{
  m_path.addRect(x, y, w, h);
}

/**
 * @brief Parses a Canvas2D radii array into the four per-corner radii.
 */
static void parseRoundRectRadiiArray(
  const QJSValue& radii, qreal& tl, qreal& tr, qreal& br, qreal& bl)
{
  const int n = radii.property(QStringLiteral("length")).toInt();
  if (n == 1) {
    tl = tr = br = bl = radii.property(0).toNumber();
    return;
  }

  if (n == 2) {
    tl = br = radii.property(0).toNumber();
    tr = bl = radii.property(1).toNumber();
    return;
  }

  if (n == 3) {
    tl = radii.property(0).toNumber();
    tr = bl = radii.property(1).toNumber();
    br      = radii.property(2).toNumber();
    return;
  }

  if (n >= 4) {
    tl = radii.property(0).toNumber();
    tr = radii.property(1).toNumber();
    br = radii.property(2).toNumber();
    bl = radii.property(3).toNumber();
  }
}

/**
 * @brief Adds a rounded-rectangle subpath. radii may be a number or [tl, tr, br, bl].
 */
void Widgets::PainterContext::roundRect(qreal x, qreal y, qreal w, qreal h, const QJSValue& radii)
{
  qreal tl = 0.0, tr = 0.0, br = 0.0, bl = 0.0;
  if (radii.isNumber())
    tl = tr = br = bl = radii.toNumber();
  else if (radii.isArray())
    parseRoundRectRadiiArray(radii, tl, tr, br, bl);

  PainterGeometry::appendRoundRect(m_path, x, y, w, h, tl, tr, br, bl);
}

/**
 * @brief Adds a circular arc to the path (Canvas2D semantics).
 */
void Widgets::PainterContext::arc(
  qreal x, qreal y, qreal r, qreal startRad, qreal endRad, bool counterClockwise)
{
  PainterGeometry::appendArc(m_path, x, y, r, startRad, endRad, counterClockwise);
}

/**
 * @brief Adds a Canvas2D-style arcTo: tangent arc of radius r between two segments.
 */
void Widgets::PainterContext::arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal r)
{
  PainterGeometry::appendArcTo(m_path, x1, y1, x2, y2, r);
}

/**
 * @brief Adds an axis-rotated elliptical arc (Canvas2D semantics).
 */
void Widgets::PainterContext::ellipse(qreal x,
                                      qreal y,
                                      qreal rx,
                                      qreal ry,
                                      qreal rotation,
                                      qreal startRad,
                                      qreal endRad,
                                      bool counterClockwise)
{
  PainterGeometry::appendEllipse(
    m_path, x, y, rx, ry, rotation, startRad, endRad, counterClockwise);
}

/**
 * @brief Adds a quadratic Bezier segment from the cursor.
 */
void Widgets::PainterContext::quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y)
{
  m_path.quadTo(cpx, cpy, x, y);
}

/**
 * @brief Adds a cubic Bezier segment from the cursor.
 */
void Widgets::PainterContext::bezierCurveTo(
  qreal c1x, qreal c1y, qreal c2x, qreal c2y, qreal x, qreal y)
{
  m_path.cubicTo(c1x, c1y, c2x, c2y, x, y);
}

//--------------------------------------------------------------------------------------------------
// Path consumers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills the current path with the active fill style.
 */
void Widgets::PainterContext::fill()
{
  if (!active())
    return;

  if (shadowActive()) {
    QPainterPath path = m_path;
    QBrush brush      = m_state.fillBrush;
    renderWithShadow([path, brush](QPainter* p) { p->fillPath(path, brush); }, path.boundingRect());
  }

  m_painter->fillPath(m_path, m_state.fillBrush);
}

/**
 * @brief Strokes the current path with the active stroke style.
 */
void Widgets::PainterContext::stroke()
{
  if (!active())
    return;

  if (shadowActive()) {
    QPainterPath path = m_path;
    QPen pen          = m_state.strokePen;
    renderWithShadow(
      [path, pen](QPainter* p) { p->strokePath(path, pen); },
      path.boundingRect().adjusted(-pen.widthF(), -pen.widthF(), pen.widthF(), pen.widthF()));
  }

  m_painter->strokePath(m_path, m_state.strokePen);
}

/**
 * @brief Sets the current path as the clipping region.
 */
void Widgets::PainterContext::clip()
{
  if (!active())
    return;

  m_painter->setClipPath(m_path, Qt::IntersectClip);
}

/**
 * @brief Returns true if the point lies inside the current path (non-zero winding).
 */
bool Widgets::PainterContext::isPointInPath(qreal x, qreal y) const
{
  return m_path.contains(QPointF(x, y));
}

/**
 * @brief Returns true if the point lies inside the current path's stroke region.
 */
bool Widgets::PainterContext::isPointInStroke(qreal x, qreal y) const
{
  QPainterPathStroker stroker;
  stroker.setWidth(m_state.strokePen.widthF());
  stroker.setCapStyle(m_state.strokePen.capStyle());
  stroker.setJoinStyle(m_state.strokePen.joinStyle());
  stroker.setMiterLimit(m_state.strokePen.miterLimit());
  return stroker.createStroke(m_path).contains(QPointF(x, y));
}

//--------------------------------------------------------------------------------------------------
// Rectangle convenience
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills the rectangle (x, y, w, h) with the active fill style.
 */
void Widgets::PainterContext::fillRect(qreal x, qreal y, qreal w, qreal h)
{
  if (!active())
    return;

  if (shadowActive()) {
    QBrush brush = m_state.fillBrush;
    QRectF r(x, y, w, h);
    renderWithShadow([r, brush](QPainter* p) { p->fillRect(r, brush); }, r);
  }

  m_painter->fillRect(QRectF(x, y, w, h), m_state.fillBrush);
}

/**
 * @brief Strokes the rectangle (x, y, w, h) with the active stroke style.
 */
void Widgets::PainterContext::strokeRect(qreal x, qreal y, qreal w, qreal h)
{
  if (!active())
    return;

  QPainterPath p;
  p.addRect(x, y, w, h);

  if (shadowActive()) {
    QPen pen = m_state.strokePen;
    renderWithShadow(
      [p, pen](QPainter* qp) { qp->strokePath(p, pen); },
      p.boundingRect().adjusted(-pen.widthF(), -pen.widthF(), pen.widthF(), pen.widthF()));
  }

  m_painter->strokePath(p, m_state.strokePen);
}

/**
 * @brief Clears the rectangle (x, y, w, h) to transparent.
 */
void Widgets::PainterContext::clearRect(qreal x, qreal y, qreal w, qreal h)
{
  if (!active())
    return;

  const auto previous = m_painter->compositionMode();
  m_painter->setCompositionMode(QPainter::CompositionMode_Source);
  m_painter->fillRect(QRectF(x, y, w, h), Qt::transparent);
  m_painter->setCompositionMode(previous);
}

//--------------------------------------------------------------------------------------------------
// Text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws filled text at (x, y) using the active font and fill style.
 */
void Widgets::PainterContext::fillText(const QString& text, qreal x, qreal y)
{
  if (!active() || text.isEmpty())
    return;

  const QPointF origin = alignTextOrigin(text, x, y);

  if (shadowActive()) {
    QPainterPath p;
    p.addText(origin, m_state.font, text);
    QBrush brush = m_state.fillBrush;
    renderWithShadow([p, brush](QPainter* qp) { qp->fillPath(p, brush); }, p.boundingRect());
  }

  m_painter->save();
  m_painter->setPen(QPen(m_state.fillBrush, 0));
  m_painter->setBrush(m_state.fillBrush);
  m_painter->setFont(m_state.font);
  m_painter->drawText(origin, text);
  m_painter->restore();
}

/**
 * @brief Draws stroked text at (x, y) using the active font and stroke style.
 */
void Widgets::PainterContext::strokeText(const QString& text, qreal x, qreal y)
{
  if (!active() || text.isEmpty())
    return;

  QPainterPath path;
  const QPointF origin = alignTextOrigin(text, x, y);
  path.addText(origin, m_state.font, text);

  if (shadowActive()) {
    QPen pen = m_state.strokePen;
    renderWithShadow([path, pen](QPainter* p) { p->strokePath(path, pen); }, path.boundingRect());
  }

  m_painter->strokePath(path, m_state.strokePen);
}

/**
 * @brief Returns the rendered width of `text` under the current font.
 */
qreal Widgets::PainterContext::measureTextWidth(const QString& text) const
{
  return QFontMetricsF(m_state.font).horizontalAdvance(text);
}

/**
 * @brief Canvas-2D shaped measureText: returns { width, actualBoundingBoxAscent,
 * actualBoundingBoxDescent, fontBoundingBoxAscent, fontBoundingBoxDescent }.
 */
QVariantMap Widgets::PainterContext::measureText(const QString& text) const
{
  const QFontMetricsF fm(m_state.font);
  const QRectF tight = fm.tightBoundingRect(text);

  QVariantMap m;
  m[QStringLiteral("width")]                    = fm.horizontalAdvance(text);
  m[QStringLiteral("actualBoundingBoxAscent")]  = -tight.top();
  m[QStringLiteral("actualBoundingBoxDescent")] = tight.bottom();
  m[QStringLiteral("fontBoundingBoxAscent")]    = fm.ascent();
  m[QStringLiteral("fontBoundingBoxDescent")]   = fm.descent();
  return m;
}

//--------------------------------------------------------------------------------------------------
// Images
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws an image at (x, y) at its native size.
 */
void Widgets::PainterContext::drawImage(const QString& src, qreal x, qreal y)
{
  if (!active())
    return;

  const QString resolved = resolveImagePath(src);
  if (resolved.isEmpty())
    return;

  const QImage img(resolved);
  if (img.isNull())
    return;

  m_painter->drawImage(QPointF(x, y), img);
}

/**
 * @brief Draws an image scaled to (w, h) at (x, y).
 */
void Widgets::PainterContext::drawImageScaled(
  const QString& src, qreal x, qreal y, qreal w, qreal h)
{
  if (!active() || w <= 0.0 || h <= 0.0)
    return;

  const QString resolved = resolveImagePath(src);
  if (resolved.isEmpty())
    return;

  const QImage img(resolved);
  if (img.isNull())
    return;

  m_painter->drawImage(QRectF(x, y, w, h), img);
}

//--------------------------------------------------------------------------------------------------
// Line dashes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the dash pattern. Empty array means a solid line.
 */
void Widgets::PainterContext::setLineDash(const QJSValue& segments)
{
  m_state.lineDash.clear();
  if (segments.isArray()) {
    const int n = segments.property(QStringLiteral("length")).toInt();
    m_state.lineDash.reserve(n);
    for (int i = 0; i < n; ++i) {
      const qreal v = segments.property(static_cast<quint32>(i)).toNumber();
      if (v < 0.0)
        return;

      m_state.lineDash.push_back(v);
    }

    if ((n & 1) != 0) {
      const int dup = n;
      for (int i = 0; i < dup; ++i)
        m_state.lineDash.push_back(m_state.lineDash[i]);
    }
  }

  applyDashToPen();
}

/**
 * @brief Returns the active dash pattern as a JS-friendly variant list.
 */
QVariantList Widgets::PainterContext::getLineDash() const
{
  QVariantList out;
  out.reserve(m_state.lineDash.size());
  for (qreal v : m_state.lineDash)
    out.push_back(v);

  return out;
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

//--------------------------------------------------------------------------------------------------
// Geometry getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current canvas width in logical pixels.
 */
qreal Widgets::PainterContext::width() const noexcept
{
  return m_width;
}

/**
 * @brief Returns the current canvas height in logical pixels.
 */
qreal Widgets::PainterContext::height() const noexcept
{
  return m_height;
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a QPainter is bound for the current frame.
 */
bool Widgets::PainterContext::active() const noexcept
{
  return m_painter != nullptr;
}

/**
 * @brief Parses a CSS-like color spec into a QColor.
 */
QColor Widgets::PainterContext::parseColor(const QString& spec) const
{
  const QString trimmed = spec.trimmed();
  if (trimmed.isEmpty())
    return QColor();

  return QColor::fromString(trimmed);
}

/**
 * @brief Resolves a drawImage source path through the sandbox.
 */
QString Widgets::PainterContext::resolveImagePath(const QString& src) const
{
  if (src.isEmpty())
    return QString();

  if (src.startsWith(QLatin1String("qrc:/")) || src.startsWith(QLatin1String(":/")))
    return src.startsWith(QLatin1String("qrc:")) ? src.mid(3) : src;

  if (src.contains(QLatin1String("://")))
    return QString();

  if (m_projectDir.isEmpty())
    return QString();

  const QFileInfo info(src);
  const QString canonical = info.exists() ? info.canonicalFilePath() : QString();
  if (canonical.isEmpty())
    return QString();

  const QString projCanon = QFileInfo(m_projectDir).canonicalFilePath();
  if (projCanon.isEmpty())
    return QString();

  if (canonical == projCanon || canonical.startsWith(projCanon + QLatin1Char('/')))
    return canonical;

  return QString();
}

/**
 * @brief Translates Canvas2D textAlign/textBaseline into a QPainter origin.
 */
QPointF Widgets::PainterContext::alignTextOrigin(const QString& text, qreal x, qreal y) const
{
  qreal dx = 0.0;
  qreal dy = 0.0;

  const QFontMetricsF fm(m_state.font);
  const qreal w = fm.horizontalAdvance(text);

  if (m_state.textAlign == QLatin1String("center"))
    dx = -w * 0.5;
  else if (m_state.textAlign == QLatin1String("right") || m_state.textAlign == QLatin1String("end"))
    dx = -w;

  if (m_state.textBaseline == QLatin1String("top")
      || m_state.textBaseline == QLatin1String("hanging"))
    dy = fm.ascent();
  else if (m_state.textBaseline == QLatin1String("middle"))
    dy = fm.ascent() * 0.5 - fm.descent() * 0.5;
  else if (m_state.textBaseline == QLatin1String("bottom"))
    dy = -fm.descent();

  return QPointF(x + dx, y + dy);
}

/**
 * @brief Pushes the current dash list and offset onto the active pen.
 */
void Widgets::PainterContext::applyDashToPen()
{
  if (m_state.lineDash.isEmpty())
    m_state.strokePen.setStyle(Qt::SolidLine);
  else {
    QList<qreal> pattern;
    pattern.reserve(m_state.lineDash.size());
    const qreal w = qMax<qreal>(0.0001, m_state.strokePen.widthF());
    for (qreal v : m_state.lineDash)
      pattern.push_back(v / w);

    m_state.strokePen.setDashPattern(pattern);
    m_state.strokePen.setDashOffset(m_state.lineDashOffset / w);
  }

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Re-applies SmoothPixmapTransform per the current imageSmoothing flag.
 */
void Widgets::PainterContext::applyImageSmoothing()
{
  if (m_painter)
    m_painter->setRenderHint(QPainter::SmoothPixmapTransform, m_state.imageSmoothing);
}

/**
 * @brief Returns true if any shadow component is non-trivial enough to render.
 */
bool Widgets::PainterContext::shadowActive() const noexcept
{
  if (m_state.shadowColor.alpha() == 0)
    return false;

  return m_state.shadowBlur > 0.0 || m_state.shadowOffsetX != 0.0 || m_state.shadowOffsetY != 0.0;
}

/**
 * @brief Renders the given draw callback as a coloured drop shadow.
 */
void Widgets::PainterContext::renderWithShadow(const std::function<void(QPainter*)>& draw,
                                               const QRectF& bounds)
{
  if (!active() || bounds.isEmpty())
    return;

  const qreal pad = qMax<qreal>(2.0, m_state.shadowBlur);
  QRectF padded   = bounds.adjusted(-pad, -pad, pad, pad);
  const QSize sz(qMax(1, int(std::ceil(padded.width()))), qMax(1, int(std::ceil(padded.height()))));

  QImage shadow(sz, QImage::Format_ARGB32_Premultiplied);
  shadow.fill(Qt::transparent);

  {
    QPainter sp(&shadow);
    sp.setRenderHint(QPainter::Antialiasing, true);
    sp.translate(-padded.topLeft());
    draw(&sp);

    sp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    sp.fillRect(shadow.rect(), m_state.shadowColor);
  }

  if (m_state.shadowBlur > 0.0) {
    const int radius = qBound(1, int(std::ceil(m_state.shadowBlur)), PainterBlur::kMaxBlurRadius);
    PainterBlur::applyBoxBlur(shadow, radius);
  }

  m_painter->save();
  m_painter->setOpacity(m_state.globalAlpha);
  m_painter->drawImage(padded.topLeft() + QPointF(m_state.shadowOffsetX, m_state.shadowOffsetY),
                       shadow);
  m_painter->restore();
}

/**
 * @brief Pushes the current fill brush onto the active painter.
 */
void Widgets::PainterContext::rebindFillBrush()
{
  if (m_painter)
    m_painter->setBrush(m_state.fillBrush);
}

/**
 * @brief Pushes the current stroke pen onto the active painter.
 */
void Widgets::PainterContext::rebindStrokeBrush()
{
  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

#endif  // BUILD_COMMERCIAL
