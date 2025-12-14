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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QVector>
#include <QQuickItem>
#include <QLineSeries>

#include <kiss_fft.h>

#include "DSP.h"

namespace Widgets
{
/**
 * @class Widgets::FFTPlot
 * @brief Fast Fourier Transform visualization widget for frequency analysis.
 *
 * The FFTPlot class performs real-time Fast Fourier Transform (FFT) analysis
 * on incoming time-domain data and visualizes the resulting frequency spectrum.
 * It uses the KissFFT library for efficient FFT computation and applies window
 * functions for improved spectral analysis.
 *
 * Key Features:
 * - **Real-time FFT**: Performs FFT computation on live data streams
 * - **Windowing**: Applies window functions to reduce spectral leakage
 * - **Configurable Resolution**: Adjustable FFT size and display resolution
 * - **Auto-scaling**: Automatically adjusts frequency and magnitude ranges
 * - **Pause/Resume**: Can freeze the display while continuing data collection
 * - **Frequency Axis**: Displays frequency in Hz based on sampling rate
 *
 * The widget automatically detects the sampling rate from the dataset
 * configuration and computes the appropriate frequency axis scaling.
 *
 * Typical Use Cases:
 * - Audio spectrum analysis
 * - Vibration frequency analysis
 * - Signal harmonic detection
 * - Noise characterization
 * - Periodic signal analysis
 *
 * @note Requires KissFFT library for FFT computation. The FFT size is
 *       automatically determined from the dataset configuration.
 */
class FFTPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double minX READ minX CONSTANT)
  Q_PROPERTY(double maxX READ maxX CONSTANT)
  Q_PROPERTY(double minY READ minY CONSTANT)
  Q_PROPERTY(double maxY READ maxY CONSTANT)
  Q_PROPERTY(int dataW READ dataW WRITE setDataW NOTIFY dataSizeChanged)
  Q_PROPERTY(int dataH READ dataH WRITE setDataH NOTIFY dataSizeChanged)
  Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

signals:
  void runningChanged();
  void dataSizeChanged();

public:
  explicit FFTPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~FFTPlot()
  {
    if (m_plan)
    {
      kiss_fft_free(m_plan);
      m_plan = nullptr;
    }
  }

  [[nodiscard]] int dataW() const;
  [[nodiscard]] int dataH() const;
  [[nodiscard]] double minX() const;
  [[nodiscard]] double maxX() const;
  [[nodiscard]] double minY() const;
  [[nodiscard]] double maxY() const;
  [[nodiscard]] bool running() const;

public slots:
  void draw(QLineSeries *series);
  void setDataW(const int width);
  void setDataH(const int height);
  void setRunning(const bool enabled);

private slots:
  void updateData();

private:
  int m_size;
  int m_index;
  int m_samplingRate;

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
  DSP::AxisData m_xData;
  DSP::AxisData m_yData;
  std::vector<float> m_window;

  kiss_fft_cfg m_plan;
  std::vector<kiss_fft_cpx> m_samples;
  std::vector<kiss_fft_cpx> m_fftOutput;
};
} // namespace Widgets
