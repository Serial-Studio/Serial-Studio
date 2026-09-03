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

#include <kiss_fftr.h>

#include <QImage>
#include <QPointF>
#include <QQuickItem>
#include <vector>

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/Waterfall/WaterfallColorMap.h"
#include "UI/Widgets/Waterfall/WaterfallOverlay.h"
#include "UI/Widgets/Waterfall/WaterfallSpectrogramNodes.h"
#include "UI/Widgets/Waterfall/WaterfallViewState.h"

QT_FORWARD_DECLARE_CLASS(QSGSimpleRectNode)
QT_FORWARD_DECLARE_CLASS(QSGSimpleTextureNode)

namespace Widgets {

class AudioExport;

/**
 * @brief Pro waterfall (spectrogram) widget -- scrolling time-frequency plot.
 */
class Waterfall : public QQuickItem {
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
  Q_PROPERTY(bool colorbarVisible
             READ colorbarVisible
             WRITE setColorbarVisible
             NOTIFY colorbarVisibleChanged)
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
  Q_PROPERTY(bool markersVisible
             READ markersVisible
             WRITE setMarkersVisible
             NOTIFY markersVisibleChanged)
  Q_PROPERTY(bool audioRecordingEnabled
             READ audioRecordingEnabled
             WRITE setAudioRecordingEnabled
             NOTIFY audioRecordingEnabledChanged)
  // clang-format on

signals:
  void viewChanged();
  void runningChanged();
  void colorMapChanged();
  void historySizeChanged();
  void axisVisibleChanged();
  void cursorEnabledChanged();
  void dynamicRangeChanged();
  void markersVisibleChanged();
  void colorbarVisibleChanged();
  void audioRecordingEnabledChanged();

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
  [[nodiscard]] bool markersVisible() const noexcept;
  [[nodiscard]] bool colorbarVisible() const noexcept;
  [[nodiscard]] bool audioRecordingEnabled() const noexcept;

  Q_INVOKABLE [[nodiscard]] QString colorMapName(int index) const;
  Q_INVOKABLE [[nodiscard]] QColor colorAt(double normalized) const;
  Q_INVOKABLE [[nodiscard]] QString recordingsFolder() const;

protected:
  QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
  void updatePolish() override;
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void hoverEnterEvent(QHoverEvent* event) override;
  void hoverMoveEvent(QHoverEvent* event) override;
  void hoverLeaveEvent(QHoverEvent* event) override;
  void geometryChange(const QRectF& newGeom, const QRectF& oldGeom) override;
  void itemChange(ItemChange change, const ItemChangeData& value) override;

public slots:
  void zoomBy(double factor, double anchorX, double anchorY);
  void panBy(double normDx, double normDy);
  void resetView();
  void setRunning(const bool enabled);
  void setColorMap(const int map);
  void setHistorySize(const int size);
  void setMinDb(const double value);
  void setMaxDb(const double value);
  void setAxisVisible(const bool enabled);
  void setCursorEnabled(const bool enabled);
  void setMarkersVisible(const bool enabled);
  void setColorbarVisible(const bool enabled);
  void setAudioRecordingEnabled(const bool enabled);
  void clearHistory();

private slots:
  void updateData();
  void onThemeChanged();
  void onFontsChanged();
  void syncHistoryToTimeRange();

private:
  void loadMarkers();
  void pushAxisModel();
  void computeSmoothedRow(int spectrumSize);
  void rebuildLogColumnTable();
  [[nodiscard]] const float* imageRow(const float* dbValues, int bins);
  void allocateFftPlan(int size);
  void releaseFftPlan();
  void rebuildHistoryImage();
  void rebuildColorLut();
  void writeRow(const float* dbValues, int bins);
  void writeRowAt(int row, const float* dbValues, int bins);
  void paintRowInto(int physicalRow, const float* dbValues, int bins);
  void syncBackgroundNodes(QSGNode* root, const QRectF& plotRect);
  void syncOverlayNode(QSGNode* root);
  void releaseRenderResources();
  void releaseHistoryImage();
  void markAxisDirty();
  [[nodiscard]] QRectF computeSourceRect() const;

  int m_index;
  int m_size;
  int m_samplingRate;
  SerialStudio::FFTWindow m_windowType;
  int m_historySize;
  int m_writeRow;
  int m_topRow;
  bool m_filledOnce;

  double m_center;
  double m_halfRange;
  bool m_scaleIsValid;

  // Mouse drag state
  bool m_dragging;
  QPointF m_lastMousePos;

  bool m_releaseRenderResources;
  bool m_imageReleased;

  QSGSimpleRectNode* m_outerBgNode;
  QSGSimpleRectNode* m_innerBgNode;
  QSGSimpleTextureNode* m_overlayNode;

  std::vector<QRgb> m_colorLut;
  std::vector<float> m_lastRow;

  bool m_campbellMode;
  int m_yDatasetUniqueId;
  QString m_yAxisTitle;
  double m_yMin;
  double m_yMax;

  // Log-frequency display (dataset fftLogX): active flag, log10 domain, column LUT
  bool m_logX;
  bool m_logActive;
  double m_logMin;
  double m_logMax;
  std::vector<int> m_logColBin;
  std::vector<float> m_logColFrac;
  std::vector<float> m_logRow;

  QImage m_image;
  std::vector<float> m_window;
  std::vector<float> m_dbCache;
  std::vector<float> m_smoothed;
  std::vector<kiss_fft_scalar> m_samples;
  std::vector<kiss_fft_cpx> m_fftOutput;
  kiss_fftr_cfg m_plan;

  UI::Dashboard& m_dashboard;
  Misc::ThemeManager& m_themeManager;
  Misc::CommonFonts& m_commonFonts;
  Misc::TimerEvents& m_timerEvents;
  AudioExport& m_audioExport;

  WaterfallViewState m_view;
  WaterfallOverlay m_overlay;
  WaterfallSpectrogramNodes m_spectrogram;

  bool m_audioRecordingEnabled;
};

}  // namespace Widgets
