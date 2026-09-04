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

#include "UI/Widgets/Waterfall/WaterfallViewState.h"

#include <cmath>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the default view: unity zoom, centered pan, full -100..0 dB window.
 */
Widgets::WaterfallViewState::WaterfallViewState(const int colorMapCount, const int initialColorMap)
  : m_colorMapCount(qMax(1, colorMapCount))
  , m_colorMap(qBound(0, initialColorMap, qMax(1, colorMapCount) - 1))
  , m_minDb(-100.0)
  , m_maxDb(0.0)
  , m_xZoom(1.0)
  , m_yZoom(1.0)
  , m_xPan(0.0)
  , m_yPan(0.0)
  , m_axisVisible(true)
  , m_cursorEnabled(false)
  , m_markersVisible(true)
  , m_colorbarVisible(true)
{
  SS_ASSERT(colorMapCount > 0, m_colorMapCount = 1);
  SS_ASSERT(m_colorMap >= 0 && m_colorMap < m_colorMapCount, m_colorMap = 0);
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Convenience flag -- true when zoom is unity and pan is zero on both axes.
 */
bool Widgets::WaterfallViewState::atDefaultView() const noexcept
{
  return qFuzzyCompare(m_xZoom, 1.0) && qFuzzyCompare(m_yZoom, 1.0) && qFuzzyIsNull(m_xPan)
      && qFuzzyIsNull(m_yPan);
}

//--------------------------------------------------------------------------------------------------
// Display window setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a color map, clamped to the map table; answers whether it changed.
 */
bool Widgets::WaterfallViewState::setColorMap(const int map) noexcept
{
  const int clamped = qBound(0, map, m_colorMapCount - 1);
  if (m_colorMap == clamped)
    return false;

  m_colorMap = clamped;
  return true;
}

/**
 * @brief Sets the lower clip of the dB color mapping.
 */
bool Widgets::WaterfallViewState::setMinDb(const double value) noexcept
{
  if (qFuzzyCompare(m_minDb, value))
    return false;

  m_minDb = value;
  return true;
}

/**
 * @brief Sets the upper clip of the dB color mapping.
 */
bool Widgets::WaterfallViewState::setMaxDb(const double value) noexcept
{
  if (qFuzzyCompare(m_maxDb, value))
    return false;

  m_maxDb = value;
  return true;
}

/**
 * @brief Toggles the axes (ticks, grid, labels).
 */
bool Widgets::WaterfallViewState::setAxisVisible(const bool enabled) noexcept
{
  if (m_axisVisible == enabled)
    return false;

  m_axisVisible = enabled;
  return true;
}

/**
 * @brief Toggles the freq/time hover cursor overlay.
 */
bool Widgets::WaterfallViewState::setCursorEnabled(const bool enabled) noexcept
{
  if (m_cursorEnabled == enabled)
    return false;

  m_cursorEnabled = enabled;
  return true;
}

/**
 * @brief Toggles the frequency-marker overlay.
 */
bool Widgets::WaterfallViewState::setMarkersVisible(const bool enabled) noexcept
{
  if (m_markersVisible == enabled)
    return false;

  m_markersVisible = enabled;
  return true;
}

/**
 * @brief Toggles the side colorbar legend.
 */
bool Widgets::WaterfallViewState::setColorbarVisible(const bool enabled) noexcept
{
  if (m_colorbarVisible == enabled)
    return false;

  m_colorbarVisible = enabled;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Zoom / pan
//--------------------------------------------------------------------------------------------------

/**
 * @brief Commits a candidate view, clamping pan to what the new zoom can actually reach; the
 *        clamp is what keeps the visible window inside the data on every entry point.
 */
bool Widgets::WaterfallViewState::applyView(const double xZoom,
                                            const double yZoom,
                                            const double xPan,
                                            const double yPan) noexcept
{
  SS_ASSERT(std::isfinite(xZoom) && std::isfinite(yZoom), return false);
  SS_ASSERT(std::isfinite(xPan) && std::isfinite(yPan), return false);

  const double zx      = qBound(1.0, xZoom, kMaxZoom);
  const double zy      = qBound(1.0, yZoom, kMaxZoom);
  const double maxPanX = (1.0 - 1.0 / zx) * 0.5;
  const double maxPanY = (1.0 - 1.0 / zy) * 0.5;
  const double px      = qBound(-maxPanX, xPan, maxPanX);
  const double py      = qBound(-maxPanY, yPan, maxPanY);

  if (zx == m_xZoom && zy == m_yZoom && px == m_xPan && py == m_yPan)
    return false;

  m_xZoom = zx;
  m_yZoom = zy;
  m_xPan  = px;
  m_yPan  = py;
  return true;
}

/**
 * @brief Multiplies both axis zooms by factor, anchored at (anchorX,anchorY) in [0,1] coordinates.
 */
bool Widgets::WaterfallViewState::zoomBy(const double factor,
                                         const double anchorX,
                                         const double anchorY) noexcept
{
  if (!std::isfinite(factor) || factor <= 0.0)
    return false;

  const double newX = qBound(1.0, m_xZoom * factor, kMaxZoom);
  const double newY = qBound(1.0, m_yZoom * factor, kMaxZoom);
  const double ax   = qBound(0.0, anchorX, 1.0) - 0.5;
  const double ay   = qBound(0.0, anchorY, 1.0) - 0.5;

  return applyView(newX,
                   newY,
                   m_xPan + ax * (1.0 / m_xZoom - 1.0 / newX),
                   m_yPan + ay * (1.0 / m_yZoom - 1.0 / newY));
}

/**
 * @brief Translates the view by (normDx, normDy) -- both in normalized item-rect coordinates
 *        (e.g. 0.1 = 10% of the visible plot width/height).
 */
bool Widgets::WaterfallViewState::panBy(const double normDx, const double normDy) noexcept
{
  if (!std::isfinite(normDx) || !std::isfinite(normDy))
    return false;

  return applyView(m_xZoom, m_yZoom, m_xPan - normDx / m_xZoom, m_yPan - normDy / m_yZoom);
}

/**
 * @brief Restores zoom = 1.0 and pan = 0 on both axes.
 */
bool Widgets::WaterfallViewState::resetView() noexcept
{
  if (atDefaultView())
    return false;

  return applyView(1.0, 1.0, 0.0, 0.0);
}

//--------------------------------------------------------------------------------------------------
// Window mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a data range plus a zoom/pan pair onto the visible sub-range; the single source of
 *        truth behind the axes, the hover cursor, the marker placement and the texture source
 *        rect, so those four can never disagree about what is on screen.
 */
void Widgets::WaterfallViewState::axisWindow(const double lo,
                                             const double hi,
                                             const double zoom,
                                             const double pan,
                                             double& outMin,
                                             double& outMax) noexcept
{
  double z = zoom;
  SS_ASSERT(std::isfinite(z) && z > 0.0, z = 1.0);

  const double range  = hi - lo;
  const double srcW   = range / z;
  const double maxPan = qMax(0.0, (range - srcW) * 0.5);
  const double center = (lo + hi) * 0.5 + qBound(-maxPan, pan * range, maxPan);

  outMin = center - srcW * 0.5;
  outMax = center + srcW * 0.5;
}

/**
 * @brief Visible horizontal window over the axis world domain [w0, w1] (Hz linear, log10-Hz log).
 */
void Widgets::WaterfallViewState::freqWindow(const double w0,
                                             const double w1,
                                             double& wMin,
                                             double& wMax) const noexcept
{
  axisWindow(w0, w1, m_xZoom, m_xPan, wMin, wMax);
}

/**
 * @brief Visible vertical window over [y0, y1] -- seconds of history, or the Campbell dataset.
 */
void Widgets::WaterfallViewState::timeWindow(const double y0,
                                             const double y1,
                                             double& yMin,
                                             double& yMax) const noexcept
{
  axisWindow(y0, y1, m_yZoom, m_yPan, yMin, yMax);
}

/**
 * @brief Visible source rectangle inside a history image of the given size, given zoom/pan.
 */
QRectF Widgets::WaterfallViewState::sourceRect(const double imageWidth,
                                               const double imageHeight) const noexcept
{
  double left   = 0.0;
  double right  = 0.0;
  double top    = 0.0;
  double bottom = 0.0;
  axisWindow(0.0, imageWidth, m_xZoom, m_xPan, left, right);
  axisWindow(0.0, imageHeight, m_yZoom, m_yPan, top, bottom);

  return QRectF(left, top, right - left, bottom - top);
}
