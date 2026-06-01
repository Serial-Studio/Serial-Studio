/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "UI/Widgets/FFTPlot.h"

#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Static helper functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes a single coefficient of the 4-term Blackman-Harris window.
 */
inline float blackman_harris_coeff(unsigned int i, unsigned int N)
{
  if (N <= 1)
    return 1.0f;

  constexpr float a0 = 0.35875f;
  constexpr float a1 = 0.48829f;
  constexpr float a2 = 0.14128f;
  constexpr float a3 = 0.01168f;

  const float two_pi = 6.28318530717958647692f;
  const float k      = two_pi / static_cast<float>(N - 1);
  const float x      = k * static_cast<float>(i);

  return a0 - a1 * std::cos(x) + a2 * std::cos(2.0f * x) - a3 * std::cos(3.0f * x);
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a new FFTPlot widget.
 */
Widgets::FFTPlot::FFTPlot(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_size(0)
  , m_index(index)
  , m_samplingRate(0)
  , m_dataW(0)
  , m_dataH(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_center(0)
  , m_halfRange(1)
  , m_scaleIsValid(false)
  , m_interpolationMode(SerialStudio::InterpolationLinear)
  , m_plan(nullptr)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index)) {
    // Clamp fftSamples: project JSON is user-controlled, unbounded would OOM
    static constexpr int kMaxFftSamples = 65536;
    const auto& dataset                 = GET_DATASET(SerialStudio::DashboardFFT, m_index);
    const int clampedSamples            = qBound(8, dataset.fftSamples, kMaxFftSamples);
    m_size                              = 1 << static_cast<int>(std::log2(clampedSamples));
    m_samplingRate                      = qMax(1, dataset.fftSamplingRate);
    m_minX                              = 0;
    m_maxY                              = 0;
    m_minY                              = -100;
    m_maxX                              = m_samplingRate / 2;

    // Allocate FFT buffers and build Blackman-Harris window
    m_samples.resize(m_size);
    m_fftOutput.resize(m_size);
    m_window.resize(m_size);
    const auto windowSize = static_cast<unsigned int>(m_size);
    for (unsigned int i = 0; i < windowSize; ++i)
      m_window[i] = blackman_harris_coeff(i, windowSize);

    // Create FFT plan
    m_plan = kiss_fft_alloc(m_size, 0, nullptr, nullptr);
    if (!m_plan) {
      qWarning() << "FFT plan allocation failed for size:" << m_size;
      return;
    }

    // Configure input normalization from user-defined scale
    double minVal = dataset.fftMin;
    double maxVal = dataset.fftMax;
    if (std::isfinite(minVal) && std::isfinite(maxVal)) {
      if (maxVal < minVal)
        std::swap(minVal, maxVal);

      if (maxVal - minVal > 0.0) {
        m_scaleIsValid = true;
        m_center       = (maxVal + minVal) * 0.5;
        m_halfRange    = qMax(1e-12, (maxVal - minVal) * 0.5);
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Data dimension getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the size of the down-sampled X axis data.
 */
int Widgets::FFTPlot::dataW() const noexcept
{
  return m_dataW;
}

/**
 * @brief Returns the size of the down-sampled Y axis data.
 */
int Widgets::FFTPlot::dataH() const noexcept
{
  return m_dataH;
}

//--------------------------------------------------------------------------------------------------
// Axis range getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the minimum X-axis value.
 */
double Widgets::FFTPlot::minX() const noexcept
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 */
double Widgets::FFTPlot::maxX() const noexcept
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 */
double Widgets::FFTPlot::minY() const noexcept
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 */
double Widgets::FFTPlot::maxY() const noexcept
{
  return m_maxY;
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether plot data updates are currently active.
 */
bool Widgets::FFTPlot::running() const noexcept
{
  return UI::Dashboard::instance().fftPlotRunning(m_index);
}

/**
 * @brief Returns the current interpolation mode.
 */
SerialStudio::InterpolationMode Widgets::FFTPlot::interpolationMode() const noexcept
{
  return m_interpolationMode;
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the FFT data on the given QLineSeries.
 */
void Widgets::FFTPlot::draw(QXYSeries* series)
{
  if (series) {
    updateData();
    const auto* data = &m_data;
    if (m_interpolationMode == SerialStudio::InterpolationZoh
        || m_interpolationMode == SerialStudio::InterpolationStem) {
      updateInterpolatedData();
      data = &m_renderData;
    }

    series->replace(*data);
    Q_EMIT series->update();
  }
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the size of the down-sampled X axis data.
 */
void Widgets::FFTPlot::setDataW(const int width)
{
  if (m_dataW != width) {
    m_dataW = width;
    updateData();

    Q_EMIT dataSizeChanged();
  }
}

/**
 * @brief Updates the size of the down-sampled Y axis data.
 */
void Widgets::FFTPlot::setDataH(const int height)
{
  if (m_dataH != height) {
    m_dataH = height;
    updateData();

    Q_EMIT dataSizeChanged();
  }
}

/**
 * @brief Enables or disables plot data updates.
 */
void Widgets::FFTPlot::setRunning(const bool enabled)
{
  UI::Dashboard::instance().setFFTPlotRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Updates the interpolation mode used by the FFT plot.
 */
void Widgets::FFTPlot::setInterpolationMode(SerialStudio::InterpolationMode mode)
{
  SerialStudio::InterpolationMode resolved;
  switch (mode) {
    case SerialStudio::InterpolationNone:
    case SerialStudio::InterpolationLinear:
    case SerialStudio::InterpolationZoh:
    case SerialStudio::InterpolationStem:
      resolved = mode;
      break;
    default:
      resolved = SerialStudio::InterpolationLinear;
      break;
  }

  if (m_interpolationMode == resolved)
    return;

  m_interpolationMode = resolved;
  Q_EMIT interpolationModeChanged();
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the FFT plan and window when the input size changes.
 */
bool Widgets::FFTPlot::rebuildFftPlan(int newSize)
{
  m_size = newSize;

  m_window.resize(m_size);
  m_samples.resize(m_size);
  m_fftOutput.resize(m_size);

  const auto windowSize = static_cast<unsigned int>(m_size);
  for (unsigned int i = 0; i < windowSize; ++i)
    m_window[i] = blackman_harris_coeff(i, windowSize);

  if (m_plan) {
    kiss_fft_free(m_plan);
    m_plan = nullptr;
  }

  m_plan = kiss_fft_alloc(m_size, 0, nullptr, nullptr);
  if (!m_plan) {
    qWarning() << "FFT plan allocation failed for size:" << m_size;
    return false;
  }

  return true;
}

/**
 * @brief Converts the FFT output to dB, smooths the spectrum, and fills the
 *        x/y axis buffers used by the downsampler.
 */
void Widgets::FFTPlot::computeSmoothedSpectrum(int spectrumSize)
{
  constexpr float floorDB       = -100.0f;
  constexpr int smoothingWindow = 3;
  constexpr float eps_squared   = 1e-24f;
  constexpr int halfWindow      = smoothingWindow / 2;

  static thread_local std::vector<float> dbCache;
  if (dbCache.size() < static_cast<size_t>(spectrumSize))
    dbCache.resize(spectrumSize);

  const float normFactor = static_cast<float>(m_size) * static_cast<float>(m_size);
  const float invNorm    = 1.0f / normFactor;
  for (int i = 0; i < spectrumSize; ++i) {
    const float re    = m_fftOutput[i].r;
    const float im    = m_fftOutput[i].i;
    const float power = std::max((re * re + im * im) * invNorm, eps_squared);
    dbCache[i]        = std::max(10.0f * std::log10(power), floorDB);
  }

  if (m_xData.size() != static_cast<size_t>(spectrumSize)) {
    m_xData.resize(spectrumSize);
    m_xData.clear();
  }

  if (m_yData.size() != static_cast<size_t>(spectrumSize)) {
    m_yData.resize(spectrumSize);
    m_yData.clear();
  }

  static constexpr float kInvSmoothingTaps[] = {
    0.0f,
    1.0f / 1.0f,
    1.0f / 2.0f,
    1.0f / 3.0f,
    1.0f / 4.0f,
    1.0f / 5.0f,
  };
  const float invSize  = m_size > 0 ? 1.0f / static_cast<float>(m_size) : 0.0f;
  const float freqStep = m_samplingRate * invSize;
  for (int i = 0; i < spectrumSize; ++i) {
    const int minIdx = std::max(0, i - halfWindow);
    const int maxIdx = std::min(spectrumSize - 1, i + halfWindow);

    float sum = 0.0f;
    for (int k = minIdx; k <= maxIdx; ++k)
      sum += dbCache[k];

    const float smoothedDB = sum * kInvSmoothingTaps[maxIdx - minIdx + 1];
    const float freq       = static_cast<float>(i) * freqStep;

    m_xData.push(freq);
    m_yData.push(smoothedDB);
  }
}

/**
 * @brief Updates the FFT data.
 */
void Widgets::FFTPlot::updateData()
{
  static thread_local DSP::DownsampleWorkspace ws;

  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
    return;

  // Fetch time-domain data
  const auto& data  = UI::Dashboard::instance().fftData(m_index);
  const int newSize = static_cast<int>(data.size());
  if (newSize != m_size && !rebuildFftPlan(newSize))
    return;

  if (newSize <= 0)
    return;

  // Normalize time-domain input samples into [-1, 1] range
  const double* in       = data.raw();
  std::size_t idx        = data.frontIndex();
  const std::size_t mask = data.storageMask();
  const double offset    = m_scaleIsValid ? -m_center : 0.0;
  const double scale     = m_scaleIsValid ? (1.0 / m_halfRange) : 1.0;
  for (int i = 0; i < m_size; ++i) {
    const double raw = in[idx];
    const float v    = std::isfinite(raw) ? static_cast<float>((raw + offset) * scale) : 0.0f;
    m_samples[i].r   = v * m_window[i];
    m_samples[i].i   = 0.0f;
    idx              = (idx + 1) & mask;
  }

  // Run FFT, smooth the resulting spectrum, and downsample for the line series
  kiss_fft(m_plan, m_samples.data(), m_fftOutput.data());
  const int spectrumSize = static_cast<size_t>(m_size) / 2;
  computeSmoothedSpectrum(spectrumSize);
  DSP::downsampleMonotonic(m_xData, m_yData, m_dataW, m_dataH, m_data, &ws);
}

/**
 * @brief Rebuilds the render data for ZOH or stem interpolation modes.
 */
void Widgets::FFTPlot::updateInterpolatedData()
{
  const int n = m_data.size();

  if (m_interpolationMode == SerialStudio::InterpolationZoh) {
    if (n < 2) {
      m_renderData.resize(n);
      if (n == 1)
        m_renderData.data()[0] = m_data.constData()[0];

      return;
    }

    m_renderData.resize(2 * n - 1);
    QPointF* out      = m_renderData.data();
    const QPointF* in = m_data.constData();
    out[0]            = in[0];
    for (int i = 1; i < n; ++i) {
      out[2 * i - 1] = QPointF(in[i].x(), in[i - 1].y());
      out[2 * i]     = in[i];
    }
    return;
  }

  if (m_interpolationMode == SerialStudio::InterpolationStem) {
    constexpr double kNan = std::numeric_limits<double>::quiet_NaN();
    const double base     = m_minY;

    m_renderData.resize(3 * n);
    QPointF* out      = m_renderData.data();
    const QPointF* in = m_data.constData();
    for (int i = 0; i < n; ++i) {
      out[3 * i]     = in[i];
      out[3 * i + 1] = QPointF(in[i].x(), base);
      out[3 * i + 2] = QPointF(kNan, kNan);
    }
  }
}
