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

#pragma once

#include <QRectF>
#include <QtGlobal>

namespace Widgets {

/**
 * @brief Waterfall view/window state: pan, zoom, dB window, color map and overlay visibility. Qt
 *        value types only, no QQuickItem and no singletons, so the window arithmetic is testable
 *        in isolation. Every mutator answers whether it changed something; the facade owns the
 *        matching property-change signal and never emits one this class did not earn.
 */
class WaterfallViewState {
public:
  static constexpr double kMaxZoom = 32.0;

  WaterfallViewState(const int colorMapCount, const int initialColorMap);

  [[nodiscard]] int colorMap() const noexcept { return m_colorMap; }

  [[nodiscard]] double minDb() const noexcept { return m_minDb; }

  [[nodiscard]] double maxDb() const noexcept { return m_maxDb; }

  [[nodiscard]] double xZoom() const noexcept { return m_xZoom; }

  [[nodiscard]] double yZoom() const noexcept { return m_yZoom; }

  [[nodiscard]] double xPan() const noexcept { return m_xPan; }

  [[nodiscard]] double yPan() const noexcept { return m_yPan; }

  [[nodiscard]] bool axisVisible() const noexcept { return m_axisVisible; }

  [[nodiscard]] bool cursorEnabled() const noexcept { return m_cursorEnabled; }

  [[nodiscard]] bool markersVisible() const noexcept { return m_markersVisible; }

  [[nodiscard]] bool colorbarVisible() const noexcept { return m_colorbarVisible; }

  /**
   * @brief Reciprocal of the dB window, floored so an inverted or empty window cannot divide
   *        by zero on the per-row colorization path; inline to stay usable from the caller's TU.
   */
  [[nodiscard]] float invDbRange() const noexcept
  {
    const float span = static_cast<float>(m_maxDb) - static_cast<float>(m_minDb);
    return 1.0f / qMax(1e-6f, span);
  }

  [[nodiscard]] bool atDefaultView() const noexcept;

  [[nodiscard]] bool setColorMap(const int map) noexcept;
  [[nodiscard]] bool setMinDb(const double value) noexcept;
  [[nodiscard]] bool setMaxDb(const double value) noexcept;
  [[nodiscard]] bool setAxisVisible(const bool enabled) noexcept;
  [[nodiscard]] bool setCursorEnabled(const bool enabled) noexcept;
  [[nodiscard]] bool setMarkersVisible(const bool enabled) noexcept;
  [[nodiscard]] bool setColorbarVisible(const bool enabled) noexcept;

  [[nodiscard]] bool zoomBy(const double factor,
                            const double anchorX,
                            const double anchorY) noexcept;
  [[nodiscard]] bool panBy(const double normDx, const double normDy) noexcept;
  [[nodiscard]] bool resetView() noexcept;

  void freqWindow(const double w0, const double w1, double& wMin, double& wMax) const noexcept;
  void timeWindow(const double y0, const double y1, double& yMin, double& yMax) const noexcept;
  [[nodiscard]] QRectF sourceRect(const double imageWidth, const double imageHeight) const noexcept;

  static void axisWindow(const double lo,
                         const double hi,
                         const double zoom,
                         const double pan,
                         double& outMin,
                         double& outMax) noexcept;

private:
  [[nodiscard]] bool applyView(const double xZoom,
                               const double yZoom,
                               const double xPan,
                               const double yPan) noexcept;

  int m_colorMapCount;
  int m_colorMap;

  double m_minDb;
  double m_maxDb;

  double m_xZoom;
  double m_yZoom;
  double m_xPan;
  double m_yPan;

  bool m_axisVisible;
  bool m_cursorEnabled;
  bool m_markersVisible;
  bool m_colorbarVisible;
};

}  // namespace Widgets
