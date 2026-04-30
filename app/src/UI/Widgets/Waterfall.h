/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <kiss_fft.h>

#include <QImage>
#include <QVector>

#include "UI/QuickPaintedItemCompat.h"

namespace Widgets {

/**
 * @brief Pro waterfall (spectrogram) widget — scrolling time-frequency plot.
 *
 * Reuses the dataset's FFT settings (samples, sampling rate, min/max) and
 * maintains a history of FFT magnitude rows rendered as a color-mapped image.
 */
class Waterfall : public QuickPaintedItemCompat {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool running
             READ running
             WRITE setRunning
             NOTIFY runningChanged)
  Q_PROPERTY(int historySize
             READ historySize
             WRITE setHistorySize
             NOTIFY historySizeChanged)
  Q_PROPERTY(int colorMap
             READ colorMap
             WRITE setColorMap
             NOTIFY colorMapChanged)
  Q_PROPERTY(double minDb
             READ minDb
             WRITE setMinDb
             NOTIFY dynamicRangeChanged)
  Q_PROPERTY(double maxDb
             READ maxDb
             WRITE setMaxDb
             NOTIFY dynamicRangeChanged)
  Q_PROPERTY(double minFreq
             READ minFreq
             CONSTANT)
  Q_PROPERTY(double maxFreq
             READ maxFreq
             CONSTANT)
  Q_PROPERTY(int samplingRate
             READ samplingRate
             CONSTANT)
  Q_PROPERTY(int fftSize
             READ fftSize
             CONSTANT)
  Q_PROPERTY(int colorMapCount
             READ colorMapCount
             CONSTANT)
  Q_PROPERTY(bool axisVisible
             READ axisVisible
             WRITE setAxisVisible
             NOTIFY axisVisibleChanged)
  Q_PROPERTY(double xZoom
             READ xZoom
             NOTIFY viewChanged)
  Q_PROPERTY(double yZoom
             READ yZoom
             NOTIFY viewChanged)
  Q_PROPERTY(double xPan
             READ xPan
             NOTIFY viewChanged)
  Q_PROPERTY(double yPan
             READ yPan
             NOTIFY viewChanged)
  Q_PROPERTY(bool atDefaultView
             READ atDefaultView
             NOTIFY viewChanged)
  Q_PROPERTY(bool cursorEnabled
             READ cursorEnabled
             WRITE setCursorEnabled
             NOTIFY cursorEnabledChanged)
  // clang-format on

signals:
  void viewChanged();
  void runningChanged();
  void colorMapChanged();
  void historySizeChanged();
  void axisVisibleChanged();
  void cursorEnabledChanged();
  void dynamicRangeChanged();

public:
  /**
   * @brief Built-in color map identifiers.
   */
  enum ColorMap {
    Viridis = 0,
    Inferno,
    Magma,
    Plasma,
    Turbo,
    Jet,
    Hot,
    Grayscale,
    ColorMapCount,
  };
  Q_ENUM(ColorMap)

  explicit Waterfall(const int index = -1, QQuickItem* parent = nullptr);
  ~Waterfall() override;

  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] int colorMap() const noexcept;
  [[nodiscard]] int historySize() const noexcept;
  [[nodiscard]] int samplingRate() const noexcept;
  [[nodiscard]] int fftSize() const noexcept;
  [[nodiscard]] double minDb() const noexcept;
  [[nodiscard]] double maxDb() const noexcept;
  [[nodiscard]] double minFreq() const noexcept;
  [[nodiscard]] double maxFreq() const noexcept;
  [[nodiscard]] int colorMapCount() const noexcept;
  [[nodiscard]] bool axisVisible() const noexcept;
  [[nodiscard]] double xZoom() const noexcept;
  [[nodiscard]] double yZoom() const noexcept;
  [[nodiscard]] double xPan() const noexcept;
  [[nodiscard]] double yPan() const noexcept;
  [[nodiscard]] bool atDefaultView() const noexcept;
  [[nodiscard]] bool cursorEnabled() const noexcept;

  Q_INVOKABLE [[nodiscard]] QString colorMapName(int index) const;
  Q_INVOKABLE [[nodiscard]] QColor colorAt(double normalized) const;
  Q_INVOKABLE void zoomBy(double factor, double anchorX, double anchorY);
  Q_INVOKABLE void panBy(double normDx, double normDy);
  Q_INVOKABLE void resetView();

  void paint(QPainter* painter) override;

protected:
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void hoverEnterEvent(QHoverEvent* event) override;
  void hoverMoveEvent(QHoverEvent* event) override;
  void hoverLeaveEvent(QHoverEvent* event) override;
  void geometryChange(const QRectF& newGeom, const QRectF& oldGeom) override;

public slots:
  void setRunning(const bool enabled);
  void setColorMap(const int map);
  void setHistorySize(const int size);
  void setMinDb(const double value);
  void setMaxDb(const double value);
  void setAxisVisible(const bool enabled);
  void setCursorEnabled(const bool enabled);
  void clearHistory();

private slots:
  void updateData();
  void onThemeChanged();
  void onFontsChanged();

private:
  /**
   * @brief Result of axis tick generation — sampled values + step + display max.
   */
  struct AxisTicks {
    std::vector<double> values;
    double step;
    double displayMax;
  };

  void allocateFftPlan(int size);
  void releaseFftPlan();
  void rebuildHistoryImage();
  void writeRow(const float* dbValues, int bins);
  void writeRowAt(int row, const float* dbValues, int bins);
  void renderAxisLayer();
  void markAxisDirty();
  void drawXAxis(QPainter* painter, const QRectF& plotRect) const;
  void drawYAxis(QPainter* painter, const QRectF& plotRect) const;
  void drawCursor(QPainter* painter, const QRectF& plotRect) const;
  [[nodiscard]] QRectF computePlotRect(const QFontMetrics& fm) const;
  [[nodiscard]] QRectF computeSourceRect() const;
  static QRgb sampleColorMap(int map, double t);
  static AxisTicks computeFreqTicks(double maxFreq, int targetCount);
  static AxisTicks computeTimeTicks(double maxSeconds, int targetCount);
  static QString formatFreqTick(double hz);
  static QString formatTimeTick(double seconds, double step);

  int m_index;
  int m_size;
  int m_samplingRate;
  int m_historySize;
  int m_colorMap;
  int m_writeRow;
  bool m_filledOnce;
  bool m_axisVisible;

  double m_minDb;
  double m_maxDb;
  double m_center;
  double m_halfRange;
  double m_xZoom;
  double m_yZoom;
  double m_xPan;
  double m_yPan;
  bool m_scaleIsValid;

  // Mouse drag state
  bool m_dragging;
  QPointF m_lastMousePos;

  // Hover-cursor state (frequency / time readout under the pointer)
  bool m_cursorEnabled;
  bool m_cursorHovering;
  QPointF m_cursorPos;

  // Cached axis layer — rebuilt only when zoom/pan/size/theme changes
  bool m_axisDirty;
  QRectF m_cachedPlotRect;
  QImage m_axisLayer;

  // Cached theme colors (refreshed via onThemeChanged())
  QColor m_outerBg;
  QColor m_innerBg;
  QColor m_borderColor;
  QColor m_textColor;
  QColor m_gridColor;

  // Campbell-mode state — when m_campbellMode is true the Y axis is driven by
  // an external dataset's value (e.g. RPM) instead of elapsed time.
  bool m_campbellMode;
  int m_yDatasetIndex;
  QString m_yAxisTitle;
  double m_yMin;
  double m_yMax;

  QImage m_image;
  std::vector<float> m_window;
  std::vector<float> m_dbCache;
  std::vector<float> m_smoothed;
  std::vector<kiss_fft_cpx> m_samples;
  std::vector<kiss_fft_cpx> m_fftOutput;
  kiss_fft_cfg m_plan;
};

}  // namespace Widgets
