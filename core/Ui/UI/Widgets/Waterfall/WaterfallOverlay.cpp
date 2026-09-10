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

#include "UI/Widgets/Waterfall/WaterfallOverlay.h"

#include <algorithm>
#include <cmath>
#include <QFontMetrics>
#include <QObject>
#include <QPainter>
#include <QtMath>

#include "Core/SSAssert.h"
#include "Core/TimerEvents.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/Waterfall/WaterfallMath.h"
#include "UI/Widgets/Waterfall/WaterfallTicks.h"

using namespace UI::Widgets::WaterfallDetail;
using Widgets::WaterfallTicks::AxisTicks;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr float kMarkerFloorDb = -100.0f;
static constexpr double kFallbackFps  = 24.0;
static constexpr double kOverlayLn10  = 2.302585092994046;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the overlay to the widget's view state and its three UI collaborators; every
 *        dependency arrives here so the layer itself never resolves a singleton.
 */
Widgets::WaterfallOverlay::WaterfallOverlay(const WaterfallViewState& view,
                                            Misc::ThemeManager& themeManager,
                                            Misc::CommonFonts& commonFonts,
                                            Misc::TimerEvents& timerEvents)
  : m_view(view)
  , m_themeManager(themeManager)
  , m_commonFonts(commonFonts)
  , m_timerEvents(timerEvents)
  , m_axis()
  , m_selectedMarker(-1)
  , m_dirty(true)
  , m_upload(true)
  , m_cursorHovering(false)
{
  m_axis.logMax = 1.0;
  m_axis.yMax   = 1.0;

  refreshTheme();
}

//--------------------------------------------------------------------------------------------------
// Layer state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the overlay as needing a re-render; the widget still owns polish scheduling.
 */
void Widgets::WaterfallOverlay::markDirty() noexcept
{
  m_dirty = true;
}

/**
 * @brief Answers whether the layer changed since the last scene-graph sync, clearing the flag so
 *        the texture is uploaded exactly once per rasterization.
 */
bool Widgets::WaterfallOverlay::consumeUpload() noexcept
{
  const bool pending = m_upload;
  m_upload           = false;
  return pending;
}

/**
 * @brief Refreshes the cached theme colors and forces a re-render.
 */
void Widgets::WaterfallOverlay::refreshTheme()
{
  m_outerBg      = m_themeManager.getColor(QStringLiteral("widget_window"));
  m_innerBg      = m_themeManager.getColor(QStringLiteral("widget_base"));
  m_borderColor  = m_themeManager.getColor(QStringLiteral("widget_border"));
  m_textColor    = m_themeManager.getColor(QStringLiteral("widget_text"));
  m_gridColor    = QColor(m_borderColor.red(), m_borderColor.green(), m_borderColor.blue(), 80);
  m_accentColor  = m_themeManager.getColor(QStringLiteral("highlight"));
  m_warningColor = m_themeManager.alarmColorForSeverity(2);
  m_alarmColor   = m_themeManager.alarmColorForSeverity(3);

  markDirty();
}

//--------------------------------------------------------------------------------------------------
// Model inputs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces the axis domain the overlay draws against.
 */
void Widgets::WaterfallOverlay::setAxisModel(const AxisModel& axis)
{
  m_axis = axis;
  markDirty();
}

/**
 * @brief Adopts a new marker set, dropping the spotlight and the stale chip hit rects with it.
 */
void Widgets::WaterfallOverlay::setMarkers(std::vector<MarkerData>&& markers)
{
  m_markers = std::move(markers);
  clearSpotlight();
  markDirty();
}

/**
 * @brief Records the pointer position for the hover readout.
 */
void Widgets::WaterfallOverlay::setCursorPosition(const QPointF& pos, const bool hovering) noexcept
{
  m_cursorPos      = pos;
  m_cursorHovering = hovering;
}

/**
 * @brief Drops the marker spotlight and the chip hit rects captured for it.
 */
void Widgets::WaterfallOverlay::clearSpotlight() noexcept
{
  m_selectedMarker = -1;
  m_chipHitRects.clear();
}

/**
 * @brief Spotlights a marker, or clears the spotlight when it is already on that marker.
 */
bool Widgets::WaterfallOverlay::toggleSpotlight(const int markerIndex) noexcept
{
  SS_ASSERT(markerIndex >= 0 && (markerIndex < static_cast<int>(m_markers.size())), return false);

  m_selectedMarker = (m_selectedMarker == markerIndex) ? -1 : markerIndex;
  return true;
}

/**
 * @brief Refreshes each marker's peak and state from the freshly smoothed spectrum row; point
 *        markers use a +/- 2 bin neighborhood. Bin math clamps in the double domain BEFORE the
 *        int cast: casting an unrepresentable double is UB. Returns whether the drawn readout
 *        changed -- chips render one decimal, and a smaller move must not re-rasterize (F7).
 */
bool Widgets::WaterfallOverlay::updateMarkerStates(const float* smoothed, const int spectrumSize)
{
  constexpr double pointHalfWindow = 2.0;
  SS_ASSERT(smoothed != nullptr, return false);
  SS_ASSERT(spectrumSize > 0, return false);

  bool changed = false;

  const double freqStep = static_cast<double>(m_axis.samplingRate) / qMax(1, m_axis.fftSize);
  const double lastBin  = qMax(0, spectrumSize - 1);
  for (auto& m : m_markers) {
    double loF = 0.0;
    double hiF = 0.0;
    if (m.freqHi > m.freqLo) {
      loF = std::floor(m.freqLo / freqStep);
      hiF = std::ceil(m.freqHi / freqStep);
    } else {
      const double center = std::round(m.freqLo / freqStep);
      loF                 = center - pointHalfWindow;
      hiF                 = center + pointHalfWindow;
    }

    const int lo = static_cast<int>(qBound(0.0, loF, lastBin));
    const int hi = static_cast<int>(qBound(static_cast<double>(lo), hiF, lastBin));

    float peak = kMarkerFloorDb;
    for (int i = lo; i <= hi; ++i)
      peak = std::max(peak, smoothed[i]);

    const int previousState = m.state;
    const int previousLabel = qRound(m.peakDb * 10.0f);

    m.peakDb = peak;
    if (std::isfinite(m.alarmDb) && peak >= m.alarmDb)
      m.state = 2;
    else if (std::isfinite(m.warningDb) && peak >= m.warningDb)
      m.state = 1;
    else
      m.state = 0;

    changed |= m.state != previousState || qRound(m.peakDb * 10.0f) != previousLabel;
  }

  return changed;
}

//--------------------------------------------------------------------------------------------------
// Axis domain mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a frequency in Hz to the axis world domain (Hz linear, log10-Hz log).
 */
double Widgets::WaterfallOverlay::worldFromFreq(const double hz) const
{
  if (!m_axis.logActive)
    return hz;

  const double freqStep = static_cast<double>(m_axis.samplingRate) / qMax(1, m_axis.fftSize);
  return std::log10(qMax(hz, freqStep));
}

/**
 * @brief Maps an axis world coordinate back to a frequency in Hz.
 */
double Widgets::WaterfallOverlay::freqFromWorld(const double w) const
{
  if (!m_axis.logActive)
    return w;

  return std::exp(w * kOverlayLn10);
}

/**
 * @brief Computes the visible frequency window in axis WORLD units from the zoom/pan view state;
 *        single source of truth for the axis, hover cursor, and marker Hz-to-pixel mapping.
 */
void Widgets::WaterfallOverlay::visibleFreqWindow(double& wMin, double& wMax) const
{
  const double w0 = m_axis.logActive ? m_axis.logMin : 0.0;
  const double w1 = m_axis.logActive ? m_axis.logMax : m_axis.samplingRate * 0.5;
  m_view.freqWindow(w0, w1, wMin, wMax);
}

/**
 * @brief Full vertical data range: the Campbell dataset bounds, or seconds of stored history.
 */
void Widgets::WaterfallOverlay::valueRange(double& lo, double& hi) const
{
  if (m_axis.campbellMode) {
    lo = m_axis.yMin;
    hi = m_axis.yMax;
    return;
  }

  const double fps = m_timerEvents.fps() > 0 ? m_timerEvents.fps() : kFallbackFps;
  lo               = 0.0;
  hi               = m_axis.historySize / fps;
}

/**
 * @brief Collects the tick frequencies (Hz) for the visible window: the {1,2,5} ladder on the
 *        linear axis, or per-decade {1,2,5} candidates thinned to decades on the log axis when
 *        they would crowd.
 */
std::vector<double> Widgets::WaterfallOverlay::collectFreqTicks(const double wMin,
                                                                const double wMax) const
{
  std::vector<double> out;
  if (!m_axis.logActive) {
    const AxisTicks ticks = WaterfallTicks::computeFreqTicks(wMax - wMin, kAxisTickCount);
    const double step     = ticks.step;
    const double first    = std::ceil(wMin / step - 1e-9) * step;
    for (double v = first; v <= wMax + 1e-6; v += step)
      out.push_back(v);

    return out;
  }

  constexpr double mants[] = {1.0, 2.0, 5.0};
  const int dLo            = static_cast<int>(std::floor(wMin)) - 1;
  const int dHi            = static_cast<int>(std::ceil(wMax)) + 1;
  for (int dec = dLo; dec <= dHi; ++dec) {
    for (const double m : mants) {
      const double w = dec + std::log10(m);
      if (w >= wMin - 1e-9 && w <= wMax + 1e-9)
        out.push_back(m * waterfallFastPow10(dec));
    }
  }

  if (static_cast<int>(out.size()) > kAxisTickCount + 2) {
    std::vector<double> decades;
    decades.reserve(out.size());
    for (const double v : out) {
      const double lg = std::log10(v);
      if (std::abs(lg - std::round(lg)) < 1e-9)
        decades.push_back(v);
    }

    if (!decades.empty())
      out = std::move(decades);
  }

  if (static_cast<int>(out.size()) < 2) {
    out.clear();
    const double fLo      = freqFromWorld(wMin);
    const double fHi      = freqFromWorld(wMax);
    const AxisTicks ticks = WaterfallTicks::computeFreqTicks(fHi - fLo, kAxisTickCount);
    const double first    = std::ceil(fLo / ticks.step - 1e-9) * ticks.step;
    for (double v = first; v <= fHi + 1e-6; v += ticks.step)
      out.push_back(v);
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Layout
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the inner plot rectangle after reserving axis-label margins.
 */
QRectF Widgets::WaterfallOverlay::computePlotRect(const QFontMetrics& fm,
                                                  const QSizeF& itemSize) const
{
  const double w = itemSize.width();
  const double h = itemSize.height();
  if (!m_view.axisVisible() || w < kMinAxisWidth || h < kMinAxisHeight)
    return QRectF(0.5, 0.5, qMax(0.0, w - 1), qMax(0.0, h - 1));

  const QFontMetrics titleFm(m_commonFonts.widgetFont(0.91, true));

  const int yTickWidth =
    fm.horizontalAdvance(QStringLiteral("00.00")) + kAxisTickPx + kAxisLabelPad;
  const int yTitleWidth = titleFm.height() + 2;

  const int leftMargin   = yTitleWidth + yTickWidth;
  const int rightMargin  = kAxisLabelPad;
  const int topMargin    = kAxisLabelPad;
  const int bottomMargin = fm.height() + kAxisTickPx + kAxisLabelPad * 2;

  return QRectF(leftMargin + 0.5,
                topMargin + 0.5,
                qMax(0.0, w - leftMargin - rightMargin - 1),
                qMax(0.0, h - topMargin - bottomMargin - 1));
}

//--------------------------------------------------------------------------------------------------
// Rasterization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-renders the overlay (border, axes, markers and hover cursor) into the cached layer
 *        and clears the dirty flag. Must run on the GUI thread, never the render thread: it uses
 *        QPainter and application fonts.
 */
void Widgets::WaterfallOverlay::render(const QSizeF& itemSize, const qreal devicePixelRatio)
{
  m_dirty  = false;
  m_upload = true;

  const QSize logicalSize(qMax(1, qCeil(itemSize.width())), qMax(1, qCeil(itemSize.height())));
  const qreal dpr     = devicePixelRatio > 0.0 ? devicePixelRatio : 1.0;
  const QSize bufSize = logicalSize * dpr;
  if (bufSize.isEmpty())
    return;

  if (m_layer.size() != bufSize) {
    m_layer = QImage(bufSize, QImage::Format_ARGB32_Premultiplied);
    m_layer.setDevicePixelRatio(dpr);
  }
  m_layer.fill(Qt::transparent);

  QPainter painter(&m_layer);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setFont(m_commonFonts.widgetFont(0.83, false));

  const QFontMetrics fm(painter.font());
  m_plotRect = computePlotRect(fm, itemSize);
  if (m_plotRect.isEmpty())
    return;

  painter.setPen(QPen(m_borderColor, 1));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(m_plotRect);

  if (m_view.axisVisible() && itemSize.width() >= kMinAxisWidth
      && itemSize.height() >= kMinAxisHeight) {
    drawXAxis(&painter);
    drawYAxis(&painter);
  }

  if (m_view.markersVisible() && !m_markers.empty())
    drawMarkers(&painter);

  if (m_view.cursorEnabled() && m_cursorHovering)
    drawCursor(&painter);
}

/**
 * @brief Draws the frequency axis (X) -- grid, tick marks, labels -- in world space, so the same
 *        loop renders both the linear and the log-frequency layout.
 */
void Widgets::WaterfallOverlay::drawXAxis(QPainter* painter)
{
  SS_ASSERT(painter != nullptr, return);
  if (m_axis.samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double wRange = wMax - wMin;
  if (wRange <= 0.0)
    return;

  const QFontMetrics fm(painter->font());
  const double tickTopY  = m_plotRect.bottom();
  const double tickBotY  = m_plotRect.bottom() + kAxisTickPx;
  const double labelY    = tickBotY + kAxisLabelPad;
  const double invWRange = 1.0 / wRange;
  const auto tickFreqs   = collectFreqTicks(wMin, wMax);

  for (const double v : tickFreqs) {
    const double t = (worldFromFreq(v) - wMin) * invWRange;
    if (t < 0.0 || t > 1.0)
      continue;

    const double x = m_plotRect.left() + t * m_plotRect.width();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(x, m_plotRect.top()), QPointF(x, m_plotRect.bottom()));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(x, tickTopY), QPointF(x, tickBotY));

    const QString label  = WaterfallTicks::formatFreqTick(v);
    const int labelWidth = fm.horizontalAdvance(label);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(x - labelWidth * 0.5, labelY + fm.ascent()), label);
  }
}

/**
 * @brief Draws the Y axis (time or Campbell-mode dataset value).
 */
void Widgets::WaterfallOverlay::drawYAxis(QPainter* painter)
{
  SS_ASSERT(painter != nullptr, return);

  double dataMin = 0.0;
  double dataMax = 0.0;
  valueRange(dataMin, dataMax);
  if (dataMax - dataMin <= 0.0)
    return;

  double yMin = 0.0;
  double yMax = 0.0;
  m_view.timeWindow(dataMin, dataMax, yMin, yMax);

  const AxisTicks ticks = WaterfallTicks::computeTimeTicks(yMax - yMin, kAxisTickCount);

  const QFontMetrics fm(painter->font());
  const double tickRightX = m_plotRect.left();
  const double tickLeftX  = m_plotRect.left() - kAxisTickPx;
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

    const double y = m_axis.campbellMode ? m_plotRect.bottom() - t * m_plotRect.height()
                                         : m_plotRect.top() + t * m_plotRect.height();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(m_plotRect.left(), y), QPointF(m_plotRect.right(), y));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(tickLeftX, y), QPointF(tickRightX, y));

    const QString label  = WaterfallTicks::formatTimeTick(v, step);
    const int labelWidth = fm.horizontalAdvance(label);
    const double textCy  = qBound(m_plotRect.top() + fm.ascent() * 0.5,
                                 y + fm.ascent() * 0.5,
                                 m_plotRect.bottom() + fm.ascent() * 0.5);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(labelRight - labelWidth, textCy), label);
  }

  const QFont titleFont(m_commonFonts.widgetFont(0.91, true));
  const QFontMetrics titleFm(titleFont);

  const QString title = m_axis.campbellMode ? m_axis.yAxisTitle : QObject::tr("Time (s)");
  const double titleX =
    labelRight - fm.horizontalAdvance(QStringLiteral("00.00")) - 2 - titleFm.descent();
  const double titleY = m_plotRect.center().y();

  painter->save();
  painter->setFont(titleFont);
  painter->translate(titleX, titleY);
  painter->rotate(-90.0);
  painter->setPen(m_textColor);
  painter->drawText(QPointF(-titleFm.horizontalAdvance(title) * 0.5, 0), title);
  painter->restore();
}

//--------------------------------------------------------------------------------------------------
// Markers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the frequency markers over the spectrogram: translucent band regions, point lines,
 *        and label chips with the live peak readout (spec 0019).
 */
void Widgets::WaterfallOverlay::drawMarkers(QPainter* painter)
{
  m_chipHitRects.clear();
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(!m_markers.empty(), return);
  if (m_plotRect.isEmpty() || m_axis.samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double range = wMax - wMin;
  if (range <= 0.0)
    return;

  painter->save();
  painter->setClipRect(m_plotRect);
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
      m_plotRect.left() + (worldFromFreq(m.freqLo) - wMin) * invRange * m_plotRect.width();
    const double xHi =
      band ? m_plotRect.left() + (worldFromFreq(m.freqHi) - wMin) * invRange * m_plotRect.width()
           : xLo;
    if (xHi < m_plotRect.left() || xLo > m_plotRect.right())
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
      painter->fillRect(QRectF(xLo, m_plotRect.top(), xHi - xLo, m_plotRect.height()), fill);

      QColor edge = base;
      edge.setAlpha(spotlit ? 200 : 120);
      const double ew = spotlit ? 2.0 : 1.0;
      painter->fillRect(QRectF(xLo, m_plotRect.top(), ew, m_plotRect.height()), edge);
      painter->fillRect(QRectF(xHi - ew, m_plotRect.top(), ew, m_plotRect.height()), edge);
    } else {
      QColor line = base;
      line.setAlpha(spotlit ? 255 : (m.state > 0 ? 240 : 170));
      const double lw = spotlit ? 3.0 : 2.0;
      painter->fillRect(QRectF(xLo - lw * 0.5, m_plotRect.top(), lw, m_plotRect.height()), line);
    }

    const QString name = m.label.isEmpty() ? WaterfallTicks::formatFreqTick(m.freqLo) : m.label;
    const QString text = QObject::tr("%1  %2 dB").arg(name, QString::number(m.peakDb, 'f', 1));
    drawMarkerChip(painter, fm, rowEnd, idx, spotlit, (xLo + xHi) * 0.5, text, base);
  }

  painter->setOpacity(1.0);
  painter->restore();
}

/**
 * @brief Draws one marker's label chip near the top of the plot, dropping down a row when it
 *        would overlap a chip already placed on the current row; captures the chip rect for the
 *        click-to-spotlight hit test.
 */
void Widgets::WaterfallOverlay::drawMarkerChip(QPainter* painter,
                                               const QFontMetrics& fm,
                                               double* rowEnd,
                                               const int markerIndex,
                                               const bool spotlit,
                                               const double cx,
                                               const QString& text,
                                               const QColor& color)
{
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(rowEnd != nullptr, return);

  const double w = fm.horizontalAdvance(text) + 8;
  const double h = fm.height() + 4;
  const double x = qBound(
    m_plotRect.left() + 2, cx - w * 0.5, qMax(m_plotRect.left() + 2, m_plotRect.right() - w - 2));

  int row = 0;
  while (row < kMarkerChipRows - 1 && rowEnd[row] > x - 4)
    ++row;

  rowEnd[row]    = qMax(rowEnd[row], x + w);
  const double y = m_plotRect.top() + 4 + row * (h + 2);
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
int Widgets::WaterfallOverlay::markerChipAt(const QPointF& pos) const
{
  for (auto it = m_chipHitRects.rbegin(); it != m_chipHitRects.rend(); ++it)
    if (it->second.contains(pos))
      return it->first;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Hover cursor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the live hover cursor -- crosshair lines clipped to the plot rect, plus a small
 *        tooltip with the freq + time readings under the pointer (zoom/pan-aware).
 */
void Widgets::WaterfallOverlay::drawCursor(QPainter* painter)
{
  SS_ASSERT(painter != nullptr, return);
  if (m_plotRect.isEmpty() || !m_plotRect.contains(m_cursorPos))
    return;

  painter->setFont(m_commonFonts.widgetFont(0.83, false));
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  const double cx = qBound(m_plotRect.left(), m_cursorPos.x(), m_plotRect.right());
  const double cy = qBound(m_plotRect.top(), m_cursorPos.y(), m_plotRect.bottom());

  painter->setPen(QPen(QColor(255, 255, 255, 178), 1));
  painter->drawLine(QPointF(cx, m_plotRect.top()), QPointF(cx, m_plotRect.bottom()));
  painter->drawLine(QPointF(m_plotRect.left(), cy), QPointF(m_plotRect.right(), cy));

  double freqHz = 0.0;
  double yVal   = 0.0;
  cursorReadoutValues(cx, cy, freqHz, yVal);

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
    m_axis.campbellMode
      ? QStringLiteral("%1: %2").arg(m_axis.yAxisTitle, QString::number(yVal, 'f', 2))
      : QObject::tr("Time: −%1").arg(fmtTime(yVal));

  drawCursorTooltip(painter, cx, cy, freqText, timeText);
}

/**
 * @brief Maps cursor pixel position to the visible Hz axis and the Y axis value.
 */
void Widgets::WaterfallOverlay::cursorReadoutValues(const double cx,
                                                    const double cy,
                                                    double& freqHz,
                                                    double& yVal) const
{
  double wMinX = 0.0;
  double wMaxX = 0.0;
  visibleFreqWindow(wMinX, wMaxX);
  freqHz = freqFromWorld(wMinX + (cx - m_plotRect.left()) / m_plotRect.width() * (wMaxX - wMinX));

  double dataMin = 0.0;
  double dataMax = 1.0;
  valueRange(dataMin, dataMax);

  double yMinV = 0.0;
  double yMaxV = 0.0;
  m_view.timeWindow(dataMin, dataMax, yMinV, yMaxV);

  const double tY = (cy - m_plotRect.top()) / m_plotRect.height();
  yVal = m_axis.campbellMode ? (yMaxV - tY * (yMaxV - yMinV)) : (yMinV + tY * (yMaxV - yMinV));
}

/**
 * @brief Renders the two-line crosshair tooltip box, flipping sides as needed.
 */
void Widgets::WaterfallOverlay::drawCursorTooltip(QPainter* painter,
                                                  const double cx,
                                                  const double cy,
                                                  const QString& freqText,
                                                  const QString& timeText) const
{
  SS_ASSERT(painter != nullptr, return);

  const QFontMetrics fm(painter->font());
  const int padX = 8;
  const int padY = 5;
  const int gap  = 2;
  const int w = std::max(fm.horizontalAdvance(freqText), fm.horizontalAdvance(timeText)) + padX * 2;
  const int h = fm.height() * 2 + gap + padY * 2;

  double tx = cx + 12;
  double ty = cy + 12;
  if (tx + w > m_plotRect.right())
    tx = cx - 12 - w;

  if (ty + h > m_plotRect.bottom())
    ty = cy - 12 - h;

  tx = qBound(m_plotRect.left() + 2, tx, m_plotRect.right() - w - 2);
  ty = qBound(m_plotRect.top() + 2, ty, m_plotRect.bottom() - h - 2);

  const QRectF tipRect(tx, ty, w, h);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 184));
  painter->drawRoundedRect(tipRect, 3, 3);

  painter->setPen(QColor(Qt::white));
  const double textBaseline = ty + padY + fm.ascent();
  painter->drawText(QPointF(tx + padX, textBaseline), freqText);
  painter->drawText(QPointF(tx + padX, textBaseline + fm.height() + gap), timeText);
}
