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

#  include <functional>
#  include <memory>
#  include <QBrush>
#  include <QColor>
#  include <QFont>
#  include <QGradient>
#  include <QImage>
#  include <QJSValue>
#  include <QObject>
#  include <QPainter>
#  include <QPainterPath>
#  include <QPen>
#  include <QPixmap>
#  include <QString>
#  include <QStringList>
#  include <QTransform>
#  include <QVariantList>
#  include <QVariantMap>
#  include <vector>

#  include "UI/Widgets/Painter/PainterGradient.h"
#  include "UI/Widgets/Painter/PainterPattern.h"

namespace Misc {
class CommonFonts;
}  // namespace Misc

namespace Widgets {

/**
 * @brief Canvas2D-shaped facade over QPainter exposed to the painter widget JS.
 */
class PainterContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QJSValue fillStyle READ fillStyle WRITE setFillStyle)
  Q_PROPERTY(QJSValue strokeStyle READ strokeStyle WRITE setStrokeStyle)
  Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth)
  Q_PROPERTY(QString lineCap READ lineCap WRITE setLineCap)
  Q_PROPERTY(QString lineJoin READ lineJoin WRITE setLineJoin)
  Q_PROPERTY(qreal miterLimit READ miterLimit WRITE setMiterLimit)
  Q_PROPERTY(qreal lineDashOffset READ lineDashOffset WRITE setLineDashOffset)
  Q_PROPERTY(QString font READ fontSpec WRITE setFontSpec)
  Q_PROPERTY(QString textAlign READ textAlign WRITE setTextAlign)
  Q_PROPERTY(QString textBaseline READ textBaseline WRITE setTextBaseline)
  Q_PROPERTY(qreal globalAlpha READ globalAlpha WRITE setGlobalAlpha)
  Q_PROPERTY(QString globalCompositeOperation READ globalCompositeOperation WRITE
               setGlobalCompositeOperation)
  Q_PROPERTY(QString shadowColor READ shadowColor WRITE setShadowColor)
  Q_PROPERTY(qreal shadowBlur READ shadowBlur WRITE setShadowBlur)
  Q_PROPERTY(qreal shadowOffsetX READ shadowOffsetX WRITE setShadowOffsetX)
  Q_PROPERTY(qreal shadowOffsetY READ shadowOffsetY WRITE setShadowOffsetY)
  Q_PROPERTY(bool imageSmoothingEnabled READ imageSmoothingEnabled WRITE setImageSmoothingEnabled)
  Q_PROPERTY(
    QString imageSmoothingQuality READ imageSmoothingQuality WRITE setImageSmoothingQuality)

public:
  explicit PainterContext(QObject* parent = nullptr);
  ~PainterContext() override = default;

  void beginFrame(QPainter* painter, qreal width, qreal height);
  void endFrame();

  void setProjectDirectory(const QString& dir);

  [[nodiscard]] QJSValue fillStyle() const;
  [[nodiscard]] QJSValue strokeStyle() const;
  void setFillStyle(const QJSValue& value);
  void setStrokeStyle(const QJSValue& value);

  [[nodiscard]] qreal lineWidth() const;
  [[nodiscard]] QString lineCap() const;
  [[nodiscard]] QString lineJoin() const;
  [[nodiscard]] qreal miterLimit() const;
  [[nodiscard]] qreal lineDashOffset() const;
  [[nodiscard]] QString fontSpec() const;
  [[nodiscard]] QString textAlign() const;
  [[nodiscard]] QString textBaseline() const;
  [[nodiscard]] qreal globalAlpha() const;
  [[nodiscard]] QString globalCompositeOperation() const;
  [[nodiscard]] QString shadowColor() const;
  [[nodiscard]] qreal shadowBlur() const;
  [[nodiscard]] qreal shadowOffsetX() const;
  [[nodiscard]] qreal shadowOffsetY() const;
  [[nodiscard]] bool imageSmoothingEnabled() const;
  [[nodiscard]] QString imageSmoothingQuality() const;

  void setLineWidth(qreal w);
  void setLineCap(const QString& cap);
  void setLineJoin(const QString& join);
  void setMiterLimit(qreal limit);
  void setLineDashOffset(qreal offset);
  void setFontSpec(const QString& spec);
  void setTextAlign(const QString& align);
  void setTextBaseline(const QString& baseline);
  void setGlobalAlpha(qreal a);
  void setGlobalCompositeOperation(const QString& op);
  void setShadowColor(const QString& spec);
  void setShadowBlur(qreal blur);
  void setShadowOffsetX(qreal dx);
  void setShadowOffsetY(qreal dy);
  void setImageSmoothingEnabled(bool enabled);
  void setImageSmoothingQuality(const QString& quality);

public slots:
  void save();
  void restore();
  void translate(qreal x, qreal y);
  void rotate(qreal radians);
  void scale(qreal sx, qreal sy);
  void transform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);
  void setTransform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);
  void resetTransform();
  [[nodiscard]] QVariantMap getTransform() const;

  void beginPath();
  void closePath();
  void moveTo(qreal x, qreal y);
  void lineTo(qreal x, qreal y);
  void rect(qreal x, qreal y, qreal w, qreal h);
  void roundRect(qreal x, qreal y, qreal w, qreal h, const QJSValue& radii);
  void arc(qreal x, qreal y, qreal r, qreal startRad, qreal endRad, bool counterClockwise = false);
  void arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal r);
  void ellipse(qreal x,
               qreal y,
               qreal rx,
               qreal ry,
               qreal rotation,
               qreal startRad,
               qreal endRad,
               bool counterClockwise = false);
  void quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y);
  void bezierCurveTo(qreal c1x, qreal c1y, qreal c2x, qreal c2y, qreal x, qreal y);

  void fill();
  void stroke();
  void clip();
  [[nodiscard]] bool isPointInPath(qreal x, qreal y) const;
  [[nodiscard]] bool isPointInStroke(qreal x, qreal y) const;

  void fillRect(qreal x, qreal y, qreal w, qreal h);
  void strokeRect(qreal x, qreal y, qreal w, qreal h);
  void clearRect(qreal x, qreal y, qreal w, qreal h);

  void fillText(const QString& text, qreal x, qreal y);
  void strokeText(const QString& text, qreal x, qreal y);
  [[nodiscard]] qreal measureTextWidth(const QString& text) const;
  [[nodiscard]] QVariantMap measureText(const QString& text) const;

  void drawImage(const QString& src, qreal x, qreal y);
  void drawImageScaled(const QString& src, qreal x, qreal y, qreal w, qreal h);

  void setLineDash(const QJSValue& segments);
  [[nodiscard]] QVariantList getLineDash() const;

  [[nodiscard]] PainterGradient* createLinearGradient(qreal x0, qreal y0, qreal x1, qreal y1);
  [[nodiscard]] PainterGradient* createRadialGradient(
    qreal x0, qreal y0, qreal r0, qreal x1, qreal y1, qreal r1);
  [[nodiscard]] PainterGradient* createConicGradient(qreal startRad, qreal cx, qreal cy);
  [[nodiscard]] PainterPattern* createPattern(const QString& src, const QString& repetition);

  [[nodiscard]] qreal width() const noexcept;
  [[nodiscard]] qreal height() const noexcept;

private:
  struct State {
    QBrush fillBrush;
    QPen strokePen;
    QFont font;
    QJSValue fillStyleValue;
    QJSValue strokeStyleValue;
    QString fillSpec;
    QString strokeSpec;
    QString fontSpecCached;
    QString textAlign;
    QString textBaseline;
    qreal globalAlpha;
    QPainter::CompositionMode compositionMode;
    QString compositionModeName;
    QColor shadowColor;
    qreal shadowBlur;
    qreal shadowOffsetX;
    qreal shadowOffsetY;
    qreal lineDashOffset;
    QVector<qreal> lineDash;
    bool imageSmoothing;
    QString imageSmoothingQuality;
    qreal miterLimit;
  };

  [[nodiscard]] bool active() const noexcept;
  [[nodiscard]] QColor parseColor(const QString& spec) const;
  [[nodiscard]] QString resolveImagePath(const QString& src) const;
  [[nodiscard]] QPointF alignTextOrigin(const QString& text, qreal x, qreal y) const;
  void applyDashToPen();
  void applyImageSmoothing();
  [[nodiscard]] bool shadowActive() const noexcept;
  void renderWithShadow(const std::function<void(QPainter*)>& draw, const QRectF& bounds);
  void rebindFillBrush();
  void rebindStrokeBrush();

  Misc::CommonFonts& m_commonFonts;

  QPainter* m_painter;
  qreal m_width;
  qreal m_height;
  QPainterPath m_path;
  State m_state;
  std::vector<State> m_stateStack;
  QString m_projectDir;
};

}  // namespace Widgets

#endif  // BUILD_COMMERCIAL
