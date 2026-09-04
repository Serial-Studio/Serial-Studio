/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <kiss_fftr.h>

#include <QElapsedTimer>
#include <QQuickItem>
#include <QVariant>
#include <QVector>
#include <QXYSeries>

#include "DSP.h"
#include "SerialStudio.h"
#include "UI/Widgets/PlotBase.h"

namespace UI {
class Dashboard;
}  // namespace UI

namespace Widgets {
#ifdef BUILD_COMMERCIAL
class AudioExport;
#endif

/**
 * @brief Fast Fourier Transform visualization widget for frequency analysis.
 */
class FFTPlot : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool running
             READ running
             WRITE setRunning
             NOTIFY runningChanged)
  Q_PROPERTY(int dataW
             READ dataW
             WRITE setDataW
             NOTIFY dataSizeChanged)
  Q_PROPERTY(int dataH
             READ dataH
             WRITE setDataH
             NOTIFY dataSizeChanged)
  Q_PROPERTY(SerialStudio::InterpolationMode interpolationMode
             READ interpolationMode
             WRITE setInterpolationMode
             NOTIFY interpolationModeChanged)
  Q_PROPERTY(double minX
             READ minX
             CONSTANT)
  Q_PROPERTY(double maxX
             READ maxX
             CONSTANT)
  Q_PROPERTY(double minY
             READ minY
             CONSTANT)
  Q_PROPERTY(double maxY
             READ maxY
             CONSTANT)
  Q_PROPERTY(bool logX
             READ logX
             CONSTANT)
  Q_PROPERTY(QVariantList markers
             READ markers
             CONSTANT)
#ifdef BUILD_COMMERCIAL
  Q_PROPERTY(bool audioRecordingEnabled
             READ audioRecordingEnabled
             WRITE setAudioRecordingEnabled
             NOTIFY audioRecordingEnabledChanged)
#endif
  // clang-format on

signals:
  void runningChanged();
  void dataSizeChanged();
  void markerValuesChanged();
  void interpolationModeChanged();
#ifdef BUILD_COMMERCIAL
  void audioRecordingEnabledChanged();
#endif

public:
  explicit FFTPlot(const int index = -1, QQuickItem* parent = nullptr);
  ~FFTPlot();

  [[nodiscard]] int dataW() const noexcept;
  [[nodiscard]] int dataH() const noexcept;
  [[nodiscard]] double minX() const noexcept;
  [[nodiscard]] double maxX() const noexcept;
  [[nodiscard]] double minY() const noexcept;
  [[nodiscard]] double maxY() const noexcept;
  [[nodiscard]] bool logX() const noexcept;
  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] QVariantList markers() const;
  [[nodiscard]] SerialStudio::InterpolationMode interpolationMode() const noexcept;
#ifdef BUILD_COMMERCIAL
  [[nodiscard]] bool audioRecordingEnabled() const noexcept;
  Q_INVOKABLE [[nodiscard]] QString recordingsFolder() const;
#endif

  Q_INVOKABLE [[nodiscard]] double markerPeakDb(int index) const;
  Q_INVOKABLE [[nodiscard]] int markerState(int index) const;

public slots:
  void draw(QXYSeries* series);
  void setDataW(const int width);
  void setDataH(const int height);
  void setRunning(const bool enabled);
  void setInterpolationMode(SerialStudio::InterpolationMode mode);
#ifdef BUILD_COMMERCIAL
  void setAudioRecordingEnabled(const bool enabled);
#endif

private slots:
  void updateData();
  void updateInterpolatedData();

private:
  /**
   * @brief Per-marker runtime state: resolved bin window, thresholds, and live readout.
   */
  struct MarkerRuntime {
    double freqLo;
    double freqHi;
    float warningDb;
    float alarmDb;
    float peakDb;
    int state;
    int binLo;
    int binHi;
  };

  bool rebuildFftPlan(int newSize);
  void loadMarkers();
  void rebuildMarkerBins();
  [[nodiscard]] bool updateMarkerValues(const int spectrumSize);
  void applyLogFrequencyBounds();
  void rebuildLogBinTable();
  void computeBinSpectrum(int spectrumSize);
  void emitLinearSpectrum(int spectrumSize);
  void buildLogRenderCurve(const int spectrumSize);
  void updateBallisticsAlpha();
  void resetBallistics(const int bins);
  [[nodiscard]] float applyBallistics(const std::size_t idx, const float freshDb);

  UI::Dashboard& m_dashboard;

  bool m_logX;
  int m_size;
  int m_index;
  int m_samplingRate;
  bool m_ballistics;
  int m_releaseMs;
  float m_releaseAlpha;
  QElapsedTimer m_ballisticsClock;
  std::vector<float> m_displayDb;
  std::vector<float> m_binDb;
  std::vector<float> m_logBinX;
  std::vector<float> m_pchipSlope;

  int m_dataW;
  int m_dataH;

  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;

  double m_center;
  double m_halfRange;
  bool m_scaleIsValid;

  QList<QPointF> m_data;
  QList<QPointF> m_renderData;
  DSP::AxisData m_xData;
  DSP::AxisData m_yData;
  std::vector<float> m_window;
  SerialStudio::FFTWindow m_windowType;

  PlotBase m_base;

  kiss_fftr_cfg m_plan;
  std::vector<kiss_fft_scalar> m_samples;
  std::vector<kiss_fft_cpx> m_fftOutput;

  QVariantList m_markerConfig;
  std::vector<MarkerRuntime> m_markerRt;

#ifdef BUILD_COMMERCIAL
  AudioExport& m_audioExport;
  bool m_audioRecordingEnabled;
#endif
};
}  // namespace Widgets
