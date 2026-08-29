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

#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <utility>
#include <vector>

#include "UI/Widgets/Waterfall/WaterfallViewState.h"

QT_FORWARD_DECLARE_CLASS(QFontMetrics)
QT_FORWARD_DECLARE_CLASS(QPainter)

namespace Misc {
class CommonFonts;
class ThemeManager;
class TimerEvents;
}  // namespace Misc

namespace Widgets {

/**
 * @brief Cached overlay layer of the waterfall widget: border, axes, frequency markers and the
 *        hover cursor, rasterized into one ARGB image and re-rendered only when an input changes,
 *        so the scrolling spectrogram underneath costs no CPU raster. Owns no widget and no
 *        singleton: geometry arrives per render call and every collaborator by constructor.
 */
class WaterfallOverlay {
public:
  /**
   * @brief Per-marker runtime state: configured span, thresholds, and live readout.
   */
  struct MarkerData {
    double freqLo;
    double freqHi;
    float warningDb;
    float alarmDb;
    float peakDb;
    int state;
    QColor customColor;
    QString label;
  };

  /**
   * @brief Axis domain the overlay draws against; the widget pushes it whenever the dataset,
   *        the FFT plan or the history depth changes.
   */
  struct AxisModel {
    int samplingRate;
    int fftSize;
    int historySize;
    bool campbellMode;
    bool logActive;
    double logMin;
    double logMax;
    double yMin;
    double yMax;
    QString yAxisTitle;
  };

  WaterfallOverlay(const WaterfallViewState& view,
                   Misc::ThemeManager& themeManager,
                   Misc::CommonFonts& commonFonts,
                   Misc::TimerEvents& timerEvents);

  [[nodiscard]] bool dirty() const noexcept { return m_dirty; }

  [[nodiscard]] bool hasMarkers() const noexcept { return !m_markers.empty(); }

  [[nodiscard]] const QImage& layer() const noexcept { return m_layer; }

  [[nodiscard]] const QRectF& plotRect() const noexcept { return m_plotRect; }

  [[nodiscard]] const QColor& outerBackground() const noexcept { return m_outerBg; }

  [[nodiscard]] const QColor& innerBackground() const noexcept { return m_innerBg; }

  [[nodiscard]] bool consumeUpload() noexcept;
  [[nodiscard]] bool toggleSpotlight(const int markerIndex) noexcept;
  [[nodiscard]] int markerChipAt(const QPointF& pos) const;

  void markDirty() noexcept;
  void refreshTheme();
  void clearSpotlight() noexcept;
  void setMarkers(std::vector<MarkerData>&& markers);
  void setAxisModel(const AxisModel& axis);
  void setCursorPosition(const QPointF& pos, const bool hovering) noexcept;
  void updateMarkerStates(const float* smoothed, const int spectrumSize);
  void render(const QSizeF& itemSize, const qreal devicePixelRatio);

private:
  [[nodiscard]] double worldFromFreq(const double hz) const;
  [[nodiscard]] double freqFromWorld(const double w) const;
  [[nodiscard]] std::vector<double> collectFreqTicks(const double wMin, const double wMax) const;
  [[nodiscard]] QRectF computePlotRect(const QFontMetrics& fm, const QSizeF& itemSize) const;

  void visibleFreqWindow(double& wMin, double& wMax) const;
  void valueRange(double& lo, double& hi) const;
  void drawXAxis(QPainter* painter);
  void drawYAxis(QPainter* painter);
  void drawMarkers(QPainter* painter);
  void drawCursor(QPainter* painter);
  void drawMarkerChip(QPainter* painter,
                      const QFontMetrics& fm,
                      double* rowEnd,
                      const int markerIndex,
                      const bool spotlit,
                      const double cx,
                      const QString& text,
                      const QColor& color);
  void cursorReadoutValues(const double cx, const double cy, double& freqHz, double& yVal) const;
  void drawCursorTooltip(QPainter* painter,
                         const double cx,
                         const double cy,
                         const QString& freqText,
                         const QString& timeText) const;

  const WaterfallViewState& m_view;
  Misc::ThemeManager& m_themeManager;
  Misc::CommonFonts& m_commonFonts;
  Misc::TimerEvents& m_timerEvents;

  AxisModel m_axis;
  std::vector<MarkerData> m_markers;
  std::vector<std::pair<int, QRectF>> m_chipHitRects;
  int m_selectedMarker;

  bool m_dirty;
  bool m_upload;
  bool m_cursorHovering;
  QPointF m_cursorPos;

  QRectF m_plotRect;
  QImage m_layer;

  QColor m_outerBg;
  QColor m_innerBg;
  QColor m_borderColor;
  QColor m_textColor;
  QColor m_gridColor;
  QColor m_accentColor;
  QColor m_warningColor;
  QColor m_alarmColor;
};

}  // namespace Widgets
