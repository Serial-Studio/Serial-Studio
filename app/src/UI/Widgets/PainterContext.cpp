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

/**
 * @brief Maps Canvas2D line-cap names to Qt::PenCapStyle.
 */
[[nodiscard]] static Qt::PenCapStyle mapLineCap(const QString& cap)
{
  if (cap == QLatin1String("round"))
    return Qt::RoundCap;

  if (cap == QLatin1String("square"))
    return Qt::SquareCap;

  return Qt::FlatCap;
}

/**
 * @brief Maps Qt::PenCapStyle back to its Canvas2D name.
 */
[[nodiscard]] static QString unmapLineCap(Qt::PenCapStyle cap)
{
  switch (cap) {
    case Qt::RoundCap:
      return QStringLiteral("round");
    case Qt::SquareCap:
      return QStringLiteral("square");
    case Qt::FlatCap:
    default:
      return QStringLiteral("butt");
  }
}

/**
 * @brief Maps Canvas2D line-join names to Qt::PenJoinStyle.
 */
[[nodiscard]] static Qt::PenJoinStyle mapLineJoin(const QString& join)
{
  if (join == QLatin1String("round"))
    return Qt::RoundJoin;

  if (join == QLatin1String("bevel"))
    return Qt::BevelJoin;

  return Qt::MiterJoin;
}

/**
 * @brief Maps Qt::PenJoinStyle back to its Canvas2D name.
 */
[[nodiscard]] static QString unmapLineJoin(Qt::PenJoinStyle join)
{
  switch (join) {
    case Qt::RoundJoin:
      return QStringLiteral("round");
    case Qt::BevelJoin:
      return QStringLiteral("bevel");
    case Qt::MiterJoin:
    default:
      return QStringLiteral("miter");
  }
}

/**
 * @brief Maps a Canvas2D globalCompositeOperation name to QPainter::CompositionMode.
 */
[[nodiscard]] static QPainter::CompositionMode mapComposite(const QString& op)
{
  static const struct {
    QLatin1String name;
    QPainter::CompositionMode mode;
  } table[] = {
    {     QLatin1String("source-over"),      QPainter::CompositionMode_SourceOver},
    {       QLatin1String("source-in"),        QPainter::CompositionMode_SourceIn},
    {      QLatin1String("source-out"),       QPainter::CompositionMode_SourceOut},
    {     QLatin1String("source-atop"),      QPainter::CompositionMode_SourceAtop},
    {QLatin1String("destination-over"), QPainter::CompositionMode_DestinationOver},
    {  QLatin1String("destination-in"),   QPainter::CompositionMode_DestinationIn},
    { QLatin1String("destination-out"),  QPainter::CompositionMode_DestinationOut},
    {QLatin1String("destination-atop"), QPainter::CompositionMode_DestinationAtop},
    {         QLatin1String("lighter"),            QPainter::CompositionMode_Plus},
    {            QLatin1String("copy"),          QPainter::CompositionMode_Source},
    {             QLatin1String("xor"),             QPainter::CompositionMode_Xor},
    {        QLatin1String("multiply"),        QPainter::CompositionMode_Multiply},
    {          QLatin1String("screen"),          QPainter::CompositionMode_Screen},
    {         QLatin1String("overlay"),         QPainter::CompositionMode_Overlay},
    {          QLatin1String("darken"),          QPainter::CompositionMode_Darken},
    {         QLatin1String("lighten"),         QPainter::CompositionMode_Lighten},
    {     QLatin1String("color-dodge"),      QPainter::CompositionMode_ColorDodge},
    {      QLatin1String("color-burn"),       QPainter::CompositionMode_ColorBurn},
    {      QLatin1String("hard-light"),       QPainter::CompositionMode_HardLight},
    {      QLatin1String("soft-light"),       QPainter::CompositionMode_SoftLight},
    {      QLatin1String("difference"),      QPainter::CompositionMode_Difference},
    {       QLatin1String("exclusion"),       QPainter::CompositionMode_Exclusion},
  };

  for (const auto& e : table)
    if (op == e.name)
      return e.mode;

  return QPainter::CompositionMode_SourceOver;
}

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
// PainterPattern
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the tile pixmap and repetition string.
 */
Widgets::PainterPattern::PainterPattern(const QPixmap& tile,
                                        const QString& repetition,
                                        QObject* parent)
  : QObject(parent), m_tile(tile), m_repetition(repetition)
{}

/**
 * @brief Returns a tiled QBrush. Non-repeat modes blank the off-axis bands.
 */
QBrush Widgets::PainterPattern::brush() const
{
  if (m_tile.isNull())
    return QBrush();

  if (m_repetition == QLatin1String("repeat") || m_repetition.isEmpty())
    return QBrush(m_tile);

  // Build padded tile so brush paints only inside requested band
  const int tw = m_tile.width();
  const int th = m_tile.height();
  if (tw <= 0 || th <= 0)
    return QBrush();

  const bool noX =
    (m_repetition == QLatin1String("no-repeat") || m_repetition == QLatin1String("repeat-y"));
  const bool noY =
    (m_repetition == QLatin1String("no-repeat") || m_repetition == QLatin1String("repeat-x"));

  const int outW = noX ? tw * 2 : tw;
  const int outH = noY ? th * 2 : th;
  QImage img(outW, outH, QImage::Format_ARGB32_Premultiplied);
  img.fill(Qt::transparent);
  QPainter p(&img);
  p.drawPixmap(0, 0, m_tile);
  p.end();
  return QBrush(QPixmap::fromImage(img));
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves a CSS-style font family to a real installed family.
 */
[[nodiscard]] static QString resolveFontFamily(const QString& family)
{
  const QString trimmed = family.trimmed();
  if (trimmed.isEmpty())
    return Misc::CommonFonts::instance().widgetFontFamily();

  const QString lower = trimmed.toLower();
  if (lower == QLatin1String("sans-serif") || lower == QLatin1String("system-ui")
      || lower == QLatin1String("ui-sans-serif"))
    return Misc::CommonFonts::instance().widgetFontFamily();

  if (lower == QLatin1String("monospace") || lower == QLatin1String("ui-monospace"))
    return Misc::CommonFonts::instance().monoFont().family();

  if (lower == QLatin1String("serif") || lower == QLatin1String("ui-serif")
      || lower == QLatin1String("cursive") || lower == QLatin1String("fantasy"))
    return Misc::CommonFonts::instance().uiFont().family();

  return trimmed;
}

/**
 * @brief Initialises the context with sensible Canvas2D defaults.
 */
Widgets::PainterContext::PainterContext(QObject* parent)
  : QObject(parent), m_painter(nullptr), m_width(0.0), m_height(0.0)
{
  const QString defaultFam = Misc::CommonFonts::instance().widgetFontFamily();

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
  return unmapLineCap(m_state.strokePen.capStyle());
}

/**
 * @brief Returns the current line join style as a Canvas2D name.
 */
QString Widgets::PainterContext::lineJoin() const
{
  return unmapLineJoin(m_state.strokePen.joinStyle());
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
  m_state.strokePen.setCapStyle(mapLineCap(cap));

  if (m_painter)
    m_painter->setPen(m_state.strokePen);
}

/**
 * @brief Updates the line join style.
 */
void Widgets::PainterContext::setLineJoin(const QString& join)
{
  m_state.strokePen.setJoinStyle(mapLineJoin(join));

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
  const QFont f          = parseFontSpec(spec);
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
  m_state.compositionMode     = mapComposite(op);

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
 * @brief Pushes the current QPainter state onto its stack.
 */
void Widgets::PainterContext::save()
{
  if (!active())
    return;

  m_painter->save();
}

/**
 * @brief Pops the QPainter state stack.
 */
void Widgets::PainterContext::restore()
{
  if (!active())
    return;

  m_painter->restore();
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

  // Clamp each radius to half the smaller side
  const qreal maxR = qMin(std::abs(w), std::abs(h)) * 0.5;
  tl               = qBound<qreal>(0.0, tl, maxR);
  tr               = qBound<qreal>(0.0, tr, maxR);
  br               = qBound<qreal>(0.0, br, maxR);
  bl               = qBound<qreal>(0.0, bl, maxR);

  if (qFuzzyIsNull(tl) && qFuzzyIsNull(tr) && qFuzzyIsNull(br) && qFuzzyIsNull(bl)) {
    m_path.addRect(x, y, w, h);
    return;
  }

  m_path.moveTo(x + tl, y);
  m_path.lineTo(x + w - tr, y);
  m_path.arcTo(x + w - 2 * tr, y, 2 * tr, 2 * tr, 90, -90);
  m_path.lineTo(x + w, y + h - br);
  m_path.arcTo(x + w - 2 * br, y + h - 2 * br, 2 * br, 2 * br, 0, -90);
  m_path.lineTo(x + bl, y + h);
  m_path.arcTo(x, y + h - 2 * bl, 2 * bl, 2 * bl, -90, -90);
  m_path.lineTo(x, y + tl);
  m_path.arcTo(x, y, 2 * tl, 2 * tl, 180, -90);
  m_path.closeSubpath();
}

/**
 * @brief Adds a circular arc to the path (Canvas2D semantics).
 */
void Widgets::PainterContext::arc(
  qreal x, qreal y, qreal r, qreal startRad, qreal endRad, bool counterClockwise)
{
  if (r <= 0.0)
    return;

  // Wrap into Canvas2D direction (CW = positive, CCW = negative); Qt sweep is the negation.
  constexpr qreal kTau = 2.0 * M_PI;
  const qreal raw      = endRad - startRad;
  qreal sweepRad;
  if (std::abs(raw) >= kTau)
    sweepRad = counterClockwise ? -kTau : kTau;
  else {
    sweepRad = std::fmod(raw, kTau);
    if (!counterClockwise && sweepRad < 0.0)
      sweepRad += kTau;
    else if (counterClockwise && sweepRad > 0.0)
      sweepRad -= kTau;
  }

  const qreal startDeg = -qRadiansToDegrees(startRad);
  const qreal sweepDeg = -qRadiansToDegrees(sweepRad);
  const QRectF box(x - r, y - r, 2.0 * r, 2.0 * r);
  m_path.arcTo(box, startDeg, sweepDeg);
}

/**
 * @brief Adds a Canvas2D-style arcTo: tangent arc of radius r between two segments.
 */
void Widgets::PainterContext::arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal r)
{
  if (r < 0.0)
    return;

  if (m_path.isEmpty()) {
    m_path.moveTo(x1, y1);
    return;
  }

  const QPointF p0 = m_path.currentPosition();
  const qreal v1x  = p0.x() - x1;
  const qreal v1y  = p0.y() - y1;
  const qreal v2x  = x2 - x1;
  const qreal v2y  = y2 - y1;
  const qreal len1 = std::hypot(v1x, v1y);
  const qreal len2 = std::hypot(v2x, v2y);
  if (qFuzzyIsNull(len1) || qFuzzyIsNull(len2) || qFuzzyIsNull(r)) {
    m_path.lineTo(x1, y1);
    return;
  }

  // Angle between the two segments
  const qreal cos_a = (v1x * v2x + v1y * v2y) / (len1 * len2);
  const qreal a     = std::acos(qBound<qreal>(-1.0, cos_a, 1.0));
  if (qFuzzyCompare(a, M_PI) || qFuzzyIsNull(a)) {
    m_path.lineTo(x1, y1);
    return;
  }

  // Tangent points on the two segments and arc centre at distance r/sin(a/2) along the bisector
  const qreal d       = r / std::tan(a * 0.5);
  const qreal invLen1 = 1.0 / len1;
  const qreal invLen2 = 1.0 / len2;
  const qreal n1x     = v1x * invLen1;
  const qreal n1y     = v1y * invLen1;
  const qreal n2x     = v2x * invLen2;
  const qreal n2y     = v2y * invLen2;
  const qreal tx1     = x1 + n1x * d;
  const qreal ty1     = y1 + n1y * d;
  const qreal tx2     = x1 + n2x * d;
  const qreal ty2     = y1 + n2y * d;

  const qreal sx = n1x + n2x;
  const qreal sy = n1y + n2y;
  const qreal sl = std::hypot(sx, sy);
  if (qFuzzyIsNull(sl)) {
    m_path.lineTo(x1, y1);
    return;
  }

  const qreal cR    = r / std::sin(a * 0.5);
  const qreal invSl = 1.0 / sl;
  const qreal cX    = x1 + (sx * invSl) * cR;
  const qreal cY    = y1 + (sy * invSl) * cR;
  const qreal cross = v1x * v2y - v1y * v2x;

  m_path.lineTo(tx1, ty1);

  const qreal startAng = std::atan2(ty1 - cY, tx1 - cX);
  const qreal endAng   = std::atan2(ty2 - cY, tx2 - cX);
  qreal sweep          = endAng - startAng;
  if (cross < 0.0 && sweep < 0.0)
    sweep += 2.0 * M_PI;
  else if (cross > 0.0 && sweep > 0.0)
    sweep -= 2.0 * M_PI;

  const QRectF box(cX - r, cY - r, 2.0 * r, 2.0 * r);
  m_path.arcTo(box, -qRadiansToDegrees(startAng), -qRadiansToDegrees(sweep));
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
  if (rx <= 0.0 || ry <= 0.0)
    return;

  constexpr qreal kTau = 2.0 * M_PI;
  const qreal raw      = endRad - startRad;
  qreal sweepRad;
  if (std::abs(raw) >= kTau)
    sweepRad = counterClockwise ? -kTau : kTau;
  else {
    sweepRad = std::fmod(raw, kTau);
    if (!counterClockwise && sweepRad < 0.0)
      sweepRad += kTau;
    else if (counterClockwise && sweepRad > 0.0)
      sweepRad -= kTau;
  }

  // Build arc on unrotated ellipse, then apply rotation transform
  QPainterPath sub;
  const QRectF box(-rx, -ry, 2.0 * rx, 2.0 * ry);
  sub.arcMoveTo(box, -qRadiansToDegrees(startRad));
  sub.arcTo(box, -qRadiansToDegrees(startRad), -qRadiansToDegrees(sweepRad));

  QTransform t;
  t.translate(x, y);
  t.rotateRadians(rotation);
  m_path.connectPath(t.map(sub));
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

    // Canvas spec: odd-length arrays are doubled to make them even.
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
 * @brief Parses a "<size>px <family>" font shorthand into a QFont.
 */
QFont Widgets::PainterContext::parseFontSpec(const QString& spec) const
{
  static const QRegularExpression sizeRe(QStringLiteral("(\\d+(?:\\.\\d+)?)\\s*px"));

  bool bold   = false;
  bool italic = false;
  qreal size  = 10.0;

  const auto sizeMatch = sizeRe.match(spec);
  if (sizeMatch.hasMatch())
    size = SerialStudio::toDouble(sizeMatch.captured(1));

  const QString lower = spec.toLower();
  if (lower.contains(QLatin1String("italic")))
    italic = true;

  if (lower.contains(QLatin1String("bold")) || lower.contains(QLatin1String(" 700"))
      || lower.contains(QLatin1String(" 800")) || lower.contains(QLatin1String(" 900")))
    bold = true;

  // Family is whatever comes after the px size token; empty falls back to the dashboard widget font
  QString family;
  if (sizeMatch.hasMatch()) {
    const int sizeEnd  = sizeMatch.capturedEnd();
    const QString tail = spec.mid(sizeEnd).trimmed();
    static const QRegularExpression kSep(QStringLiteral("[,\\s]+"));
    const QStringList tokens = tail.split(kSep, Qt::SkipEmptyParts);
    if (!tokens.isEmpty())
      family = tokens.first();
  }

  QFont f(resolveFontFamily(family), 10);
  f.setPointSizeF(size * 0.75);
  f.setBold(bold);
  f.setItalic(italic);

  return f;
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
      pattern.push_back(v / w);  // Qt expresses dashes in pen-width units

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

// Shadow blur radius is clamped to [1, 32] in renderWithShadow; max tap count is 2*32+1
static constexpr int kMaxBlurTaps         = 65;
static constexpr int kBlurReciprocalShift = 24;

/**
 * @brief Builds the Q24 ceiling-reciprocal table so `(sum * inv) >> 24 == sum / n` exactly.
 */
static const std::array<int64_t, kMaxBlurTaps + 1>& blurReciprocalTable()
{
  static const auto table = []() {
    std::array<int64_t, kMaxBlurTaps + 1> t{};
    const int64_t one = int64_t(1) << kBlurReciprocalShift;
    for (int n = 1; n <= kMaxBlurTaps; ++n)
      t[n] = (one + n - 1) / n;

    return t;
  }();
  return table;
}

/**
 * @brief Horizontal pass of a box-blur with the given radius.
 */
static void boxBlurHorizontal(const QImage& src, QImage& dst, int radius)
{
  const int w     = src.width();
  const int h     = src.height();
  const auto& inv = blurReciprocalTable();
  for (int y = 0; y < h; ++y) {
    const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(y));
    QRgb* drow       = reinterpret_cast<QRgb*>(dst.scanLine(y));
    for (int x = 0; x < w; ++x) {
      int r = 0, g = 0, b = 0, a = 0, n = 0;
      const int x0 = qMax(0, x - radius);
      const int x1 = qMin(w - 1, x + radius);
      for (int k = x0; k <= x1; ++k) {
        r += qRed(srow[k]);
        g += qGreen(srow[k]);
        b += qBlue(srow[k]);
        a += qAlpha(srow[k]);
        ++n;
      }
      const int64_t recip = inv[n];
      drow[x]             = qRgba(static_cast<int>((r * recip) >> kBlurReciprocalShift),
                      static_cast<int>((g * recip) >> kBlurReciprocalShift),
                      static_cast<int>((b * recip) >> kBlurReciprocalShift),
                      static_cast<int>((a * recip) >> kBlurReciprocalShift));
    }
  }
}

/**
 * @brief Vertical pass of a box-blur with the given radius.
 */
static void boxBlurVertical(const QImage& src, QImage& dst, int radius)
{
  const int w     = src.width();
  const int h     = src.height();
  const auto& inv = blurReciprocalTable();
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      int r = 0, g = 0, b = 0, a = 0, n = 0;
      const int y0 = qMax(0, y - radius);
      const int y1 = qMin(h - 1, y + radius);
      for (int k = y0; k <= y1; ++k) {
        const QRgb px  = reinterpret_cast<const QRgb*>(src.constScanLine(k))[x];
        r             += qRed(px);
        g             += qGreen(px);
        b             += qBlue(px);
        a             += qAlpha(px);
        ++n;
      }
      const int64_t recip = inv[n];
      reinterpret_cast<QRgb*>(dst.scanLine(y))[x] =
        qRgba(static_cast<int>((r * recip) >> kBlurReciprocalShift),
              static_cast<int>((g * recip) >> kBlurReciprocalShift),
              static_cast<int>((b * recip) >> kBlurReciprocalShift),
              static_cast<int>((a * recip) >> kBlurReciprocalShift));
    }
  }
}

/**
 * @brief Three-pass separable box-blur applied in place to the given image.
 */
static void applyBoxBlur(QImage& image, int radius)
{
  for (int pass = 0; pass < 3; ++pass) {
    QImage blurred(image.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(Qt::transparent);
    boxBlurHorizontal(image, blurred, radius);
    image.fill(Qt::transparent);
    boxBlurVertical(blurred, image, radius);
  }
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

    // Recolour the result to the shadow colour by source-in compositing
    sp.setCompositionMode(QPainter::CompositionMode_SourceIn);
    sp.fillRect(shadow.rect(), m_state.shadowColor);
  }

  // Cheap separable box-blur for shadowBlur > 0. Iterating 3x approximates a Gaussian.
  if (m_state.shadowBlur > 0.0) {
    const int radius = qBound(1, int(std::ceil(m_state.shadowBlur)), 32);
    applyBoxBlur(shadow, radius);
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
