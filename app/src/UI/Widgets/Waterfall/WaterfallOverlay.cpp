/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <algorithm>
#include <QFontMetrics>
#include <QPainter>
#include <QQuickWindow>
#include <QtMath>

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/Waterfall.h"
#include "UI/Widgets/Waterfall/WaterfallMath.h"

//--------------------------------------------------------------------------------------------------
// Axis layer cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-renders the overlay layer -- border, axes, markers and hover cursor -- into
 *        m_axisLayer and clears the dirty flag. Rasterized only when one of its inputs
 *        changed, so the scrolling spectrogram underneath costs no CPU raster.
 */
void Widgets::Waterfall::renderAxisLayer()
{
  m_axisDirty     = false;
  m_overlayUpload = true;

  const QSize itemSize(qMax(1, qCeil(width())), qMax(1, qCeil(height())));
  const qreal dpr     = (window() ? window()->devicePixelRatio() : 1.0);
  const QSize bufSize = itemSize * dpr;
  if (bufSize.isEmpty())
    return;

  if (m_axisLayer.size() != bufSize) {
    m_axisLayer = QImage(bufSize, QImage::Format_ARGB32_Premultiplied);
    m_axisLayer.setDevicePixelRatio(dpr);
  }
  m_axisLayer.fill(Qt::transparent);

  QPainter painter(&m_axisLayer);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  static auto& fonts = Misc::CommonFonts::instance();
  painter.setFont(fonts.widgetFont(0.83, false));

  const QFontMetrics fm(painter.font());
  m_cachedPlotRect = computePlotRect(fm);
  if (m_cachedPlotRect.isEmpty())
    return;

  painter.setPen(QPen(m_borderColor, 1));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(m_cachedPlotRect);

  if (m_axisVisible && width() >= kMinAxisWidth && height() >= kMinAxisHeight) {
    drawXAxis(&painter, m_cachedPlotRect);
    drawYAxis(&painter, m_cachedPlotRect);
  }

  if (m_markersVisible && !m_markers.empty())
    drawMarkers(&painter, m_cachedPlotRect);

  if (m_cursorEnabled && m_cursorHovering)
    drawCursor(&painter, m_cachedPlotRect);
}

/**
 * @brief Marks the axis overlay as needing a re-render and schedules a repaint.
 */
void Widgets::Waterfall::markAxisDirty()
{
  m_axisDirty = true;
  polish();
  update();
}

/**
 * @brief Rasterizes the overlay layer on the GUI thread ahead of the scene-graph sync. The
 *        layer uses QPainter, fonts and application singletons, none of which may be touched
 *        from updatePaintNode's render-thread context.
 */
void Widgets::Waterfall::updatePolish()
{
  if (m_axisDirty)
    renderAxisLayer();
}

/**
 * @brief Draws the frequency axis (X) -- grid, tick marks, labels -- in world space, so the
 *        same loop renders both the linear and the log-frequency layout.
 */
void Widgets::Waterfall::drawXAxis(QPainter* painter, const QRectF& plotRect) const
{
  if (m_samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double wRange = wMax - wMin;
  if (wRange <= 0.0)
    return;

  const QFontMetrics fm(painter->font());
  const double tickTopY  = plotRect.bottom();
  const double tickBotY  = plotRect.bottom() + kAxisTickPx;
  const double labelY    = tickBotY + kAxisLabelPad;
  const double invWRange = 1.0 / wRange;
  const auto tickFreqs   = collectFreqTicks(wMin, wMax);

  for (const double v : tickFreqs) {
    const double t = (worldFromFreq(v) - wMin) * invWRange;
    if (t < 0.0 || t > 1.0)
      continue;

    const double x = plotRect.left() + t * plotRect.width();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(x, tickTopY), QPointF(x, tickBotY));

    const QString label  = formatFreqTick(v);
    const int labelWidth = fm.horizontalAdvance(label);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(x - labelWidth * 0.5, labelY + fm.ascent()), label);
  }
}

/**
 * @brief Draws the Y axis (time or Campbell-mode dataset value).
 */
void Widgets::Waterfall::drawYAxis(QPainter* painter, const QRectF& plotRect) const
{
  double dataMin = 0.0;
  double dataMax = 0.0;
  if (m_campbellMode) {
    dataMin = m_yMin;
    dataMax = m_yMax;
  } else {
    static auto& timer = Misc::TimerEvents::instance();
    const double fps   = timer.fps() > 0 ? timer.fps() : 24.0;
    dataMin            = 0.0;
    dataMax            = m_historySize / fps;
  }
  const double dataRange = dataMax - dataMin;
  if (dataRange <= 0.0)
    return;

  const double srcH    = dataRange / m_yZoom;
  const double maxPan  = qMax(0.0, (dataRange - srcH) * 0.5);
  const double centerD = (dataMin + dataMax) * 0.5 + qBound(-maxPan, m_yPan * dataRange, maxPan);
  const double yMin    = centerD - srcH * 0.5;
  const double yMax    = centerD + srcH * 0.5;

  const AxisTicks ticks = computeTimeTicks(srcH, kAxisTickCount);

  const QFontMetrics fm(painter->font());
  const double tickRightX = plotRect.left();
  const double tickLeftX  = plotRect.left() - kAxisTickPx;
  const double labelRight = tickLeftX - kAxisLabelPad;

  const double step      = ticks.step;
  const double invStep   = 1.0 / step;
  const double first     = std::ceil(yMin * invStep - 1e-9) * step;
  const double yRange    = yMax - yMin;
  const double invYRange = yRange > 0.0 ? 1.0 / yRange : 0.0;

  for (double v = first; v <= yMax + 1e-6; v += step) {
    const double t = (v - yMin) * invYRange;
    if (t < 0.0 || t > 1.0)
      continue;

    const double y = m_campbellMode ? plotRect.bottom() - t * plotRect.height()
                                    : plotRect.top() + t * plotRect.height();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(tickLeftX, y), QPointF(tickRightX, y));

    const QString label  = formatTimeTick(v, step);
    const int labelWidth = fm.horizontalAdvance(label);
    const double textCy  = qBound(plotRect.top() + fm.ascent() * 0.5,
                                 y + fm.ascent() * 0.5,
                                 plotRect.bottom() + fm.ascent() * 0.5);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(labelRight - labelWidth, textCy), label);
  }

  static auto& fonts = Misc::CommonFonts::instance();
  const QFont titleFont(fonts.widgetFont(0.91, true));
  const QFontMetrics titleFm(titleFont);

  const QString title = m_campbellMode ? m_yAxisTitle : QObject::tr("Time (s)");
  const double titleX =
    labelRight - fm.horizontalAdvance(QStringLiteral("00.00")) - 2 - titleFm.descent();
  const double titleY = plotRect.center().y();

  painter->save();
  painter->setFont(titleFont);
  painter->translate(titleX, titleY);
  painter->rotate(-90.0);
  painter->setPen(m_textColor);
  painter->drawText(QPointF(-titleFm.horizontalAdvance(title) * 0.5, 0), title);
  painter->restore();
}

/**
 * @brief Draws the frequency markers over the spectrogram: translucent band regions, point
 *        lines, and label chips with the live peak readout (spec 0019).
 */
void Widgets::Waterfall::drawMarkers(QPainter* painter, const QRectF& plotRect) const
{
  m_chipHitRects.clear();
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(!m_markers.empty(), return);
  if (plotRect.isEmpty() || m_samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double range = wMax - wMin;
  if (range <= 0.0)
    return;

  painter->save();
  painter->setClipRect(plotRect);
  painter->setFont(m_commonFonts.widgetFont(0.8, false));

  const QFontMetrics fm(painter->font());
  double rowEnd[kMarkerChipRows] = {-1e18, -1e18, -1e18};
  const double invRange          = 1.0 / range;
  for (std::size_t i = 0; i < m_markers.size(); ++i) {
    const auto& m      = m_markers[i];
    const int idx      = static_cast<int>(i);
    const bool band    = m.freqHi > m.freqLo;
    const bool spotlit = m_selectedMarker == idx;
    const bool dimmed  = m_selectedMarker >= 0 && !spotlit;
    const double xLo =
      plotRect.left() + (worldFromFreq(m.freqLo) - wMin) * invRange * plotRect.width();
    const double xHi =
      band ? plotRect.left() + (worldFromFreq(m.freqHi) - wMin) * invRange * plotRect.width() : xLo;
    if (xHi < plotRect.left() || xLo > plotRect.right())
      continue;

    QColor base = m.customColor.isValid() ? m.customColor : m_accentColor;
    if (m.state == 2)
      base = m_alarmColor;
    else if (m.state == 1)
      base = m_warningColor;

    painter->setOpacity(dimmed ? 0.22 : 1.0);

    if (band) {
      QColor fill = base;
      fill.setAlpha(spotlit ? 78 : (m.state > 0 ? 64 : 40));
      painter->fillRect(QRectF(xLo, plotRect.top(), xHi - xLo, plotRect.height()), fill);

      QColor edge = base;
      edge.setAlpha(spotlit ? 200 : 120);
      const double ew = spotlit ? 2.0 : 1.0;
      painter->fillRect(QRectF(xLo, plotRect.top(), ew, plotRect.height()), edge);
      painter->fillRect(QRectF(xHi - ew, plotRect.top(), ew, plotRect.height()), edge);
    } else {
      QColor line = base;
      line.setAlpha(spotlit ? 255 : (m.state > 0 ? 240 : 170));
      const double lw = spotlit ? 3.0 : 2.0;
      painter->fillRect(QRectF(xLo - lw * 0.5, plotRect.top(), lw, plotRect.height()), line);
    }

    const QString name = m.label.isEmpty() ? formatFreqTick(m.freqLo) : m.label;
    const QString text = QObject::tr("%1  %2 dB").arg(name, QString::number(m.peakDb, 'f', 1));
    drawMarkerChip(painter, plotRect, fm, rowEnd, idx, spotlit, (xLo + xHi) * 0.5, text, base);
  }

  painter->setOpacity(1.0);
  painter->restore();
}

/**
 * @brief Draws one marker's label chip near the top of the plot, dropping down a row when it
 *        would overlap a chip already placed on the current row; captures the chip rect for
 *        the click-to-spotlight hit test.
 */
void Widgets::Waterfall::drawMarkerChip(QPainter* painter,
                                        const QRectF& plotRect,
                                        const QFontMetrics& fm,
                                        double* rowEnd,
                                        const int markerIndex,
                                        const bool spotlit,
                                        double cx,
                                        const QString& text,
                                        const QColor& color) const
{
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(rowEnd != nullptr, return);

  const double w = fm.horizontalAdvance(text) + 8;
  const double h = fm.height() + 4;
  const double x =
    qBound(plotRect.left() + 2, cx - w * 0.5, qMax(plotRect.left() + 2, plotRect.right() - w - 2));

  int row = 0;
  while (row < kMarkerChipRows - 1 && rowEnd[row] > x - 4)
    ++row;

  rowEnd[row]    = qMax(rowEnd[row], x + w);
  const double y = plotRect.top() + 4 + row * (h + 2);
  const QRectF chipRect(x, y, w, h);
  m_chipHitRects.emplace_back(markerIndex, chipRect);

  QColor bg = color;
  bg.setAlpha(spotlit ? 255 : 230);
  painter->setPen(Qt::NoPen);
  painter->setBrush(bg);
  painter->drawRoundedRect(chipRect, 3, 3);

  if (spotlit) {
    painter->setPen(QPen(m_textColor, 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(chipRect, 3, 3);
  }

  painter->setPen(m_innerBg);
  painter->drawText(QPointF(x + 4, y + 2 + fm.ascent()), text);
}

/**
 * @brief Returns the marker index whose chip contains @a pos, or -1 (topmost chip wins).
 */
int Widgets::Waterfall::markerChipAt(const QPointF& pos) const
{
  for (auto it = m_chipHitRects.rbegin(); it != m_chipHitRects.rend(); ++it)
    if (it->second.contains(pos))
      return it->first;

  return -1;
}

/**
 * @brief Draws the live hover cursor -- vertical & horizontal crosshair lines clipped to the plot
 * rect, plus a small tooltip with the freq + time readings under the pointer (zoom/pan-aware).
 */
void Widgets::Waterfall::drawCursor(QPainter* painter, const QRectF& plotRect) const
{
  if (plotRect.isEmpty() || !plotRect.contains(m_cursorPos))
    return;

  static auto& fonts = Misc::CommonFonts::instance();
  painter->setFont(fonts.widgetFont(0.83, false));
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  const double cx = qBound(plotRect.left(), m_cursorPos.x(), plotRect.right());
  const double cy = qBound(plotRect.top(), m_cursorPos.y(), plotRect.bottom());

  painter->setPen(QPen(QColor(255, 255, 255, 178), 1));
  painter->drawLine(QPointF(cx, plotRect.top()), QPointF(cx, plotRect.bottom()));
  painter->drawLine(QPointF(plotRect.left(), cy), QPointF(plotRect.right(), cy));

  double freqHz = 0.0;
  double yVal   = 0.0;
  cursorReadoutValues(plotRect, cx, cy, freqHz, yVal);

  auto fmtFreq = [](double hz) -> QString {
    const double abs = std::fabs(hz);
    if (abs >= 1e6)
      return QString::number(hz / 1e6, 'f', 2) + QStringLiteral(" MHz");

    if (abs >= 1e3)
      return QString::number(hz / 1e3, 'f', 2) + QStringLiteral(" kHz");

    return QString::number(hz, 'f', 1) + QStringLiteral(" Hz");
  };
  auto fmtTime = [](double s) -> QString {
    if (s < 1.0)
      return QString::number(std::round(s * 1000.0), 'f', 0) + QStringLiteral(" ms");

    if (s >= 100.0)
      return QString::number(s, 'f', 0) + QStringLiteral(" s");

    return QString::number(s, 'f', 2) + QStringLiteral(" s");
  };

  const QString freqText = QObject::tr("Freq: %1").arg(fmtFreq(freqHz));
  const QString timeText =
    m_campbellMode ? QStringLiteral("%1: %2").arg(m_yAxisTitle, QString::number(yVal, 'f', 2))
                   : QObject::tr("Time: −%1").arg(fmtTime(yVal));

  drawCursorTooltip(painter, plotRect, cx, cy, freqText, timeText);
}

/**
 * @brief Maps cursor pixel position to the visible Hz axis and the Y axis value.
 */
void Widgets::Waterfall::cursorReadoutValues(
  const QRectF& plotRect, double cx, double cy, double& freqHz, double& yVal) const
{
  double wMinX = 0.0;
  double wMaxX = 0.0;
  visibleFreqWindow(wMinX, wMaxX);
  freqHz = freqFromWorld(wMinX + (cx - plotRect.left()) / plotRect.width() * (wMaxX - wMinX));

  double yMinAxis = 0.0;
  double yMaxAxis = 1.0;
  if (m_campbellMode) {
    yMinAxis = m_yMin;
    yMaxAxis = m_yMax;
  } else {
    static auto& timer = Misc::TimerEvents::instance();
    const double fps   = timer.fps() > 0 ? timer.fps() : 24.0;
    yMinAxis           = 0.0;
    yMaxAxis           = m_historySize / fps;
  }
  const double yRange  = yMaxAxis - yMinAxis;
  const double srcWY   = yRange / m_yZoom;
  const double maxPanY = qMax(0.0, (yRange - srcWY) * 0.5);
  const double centerY = (yMinAxis + yMaxAxis) * 0.5 + qBound(-maxPanY, m_yPan * yRange, maxPanY);
  const double yMinV   = centerY - srcWY * 0.5;
  const double yMaxV   = centerY + srcWY * 0.5;

  const double tY = (cy - plotRect.top()) / plotRect.height();
  yVal = m_campbellMode ? (yMaxV - tY * (yMaxV - yMinV)) : (yMinV + tY * (yMaxV - yMinV));
}

/**
 * @brief Renders the two-line crosshair tooltip box, flipping sides as needed.
 */
void Widgets::Waterfall::drawCursorTooltip(QPainter* painter,
                                           const QRectF& plotRect,
                                           double cx,
                                           double cy,
                                           const QString& freqText,
                                           const QString& timeText) const
{
  const QFontMetrics fm(painter->font());
  const int padX = 8;
  const int padY = 5;
  const int gap  = 2;
  const int w = std::max(fm.horizontalAdvance(freqText), fm.horizontalAdvance(timeText)) + padX * 2;
  const int h = fm.height() * 2 + gap + padY * 2;

  double tx = cx + 12;
  double ty = cy + 12;
  if (tx + w > plotRect.right())
    tx = cx - 12 - w;

  if (ty + h > plotRect.bottom())
    ty = cy - 12 - h;

  tx = qBound(plotRect.left() + 2, tx, plotRect.right() - w - 2);
  ty = qBound(plotRect.top() + 2, ty, plotRect.bottom() - h - 2);

  const QRectF tipRect(tx, ty, w, h);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 184));
  painter->drawRoundedRect(tipRect, 3, 3);

  painter->setPen(QColor(Qt::white));
  const double textBaseline = ty + padY + fm.ascent();
  painter->drawText(QPointF(tx + padX, textBaseline), freqText);
  painter->drawText(QPointF(tx + padX, textBaseline + fm.height() + gap), timeText);
}
