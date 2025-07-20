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

#include "UI/Dashboard.h"
#include "UI/Widgets/FFTPlot.h"

/**
 * @brief Constructs a new FFTPlot widget.
 * @param index The index of the FFT plot in the Dashboard.
 * @param parent The parent QQuickItem.
 */
Widgets::FFTPlot::FFTPlot(const int index, QQuickItem *parent)
  : QQuickItem(parent)
  , m_size(0)
  , m_index(index)
  , m_samplingRate(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_center(0)
  , m_halfRange(1)
  , m_scaleIsValid(false)
  , m_transformer(0, QStringLiteral("Hann"))
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
  {
    // Get FFT dataset
    const auto &dataset = GET_DATASET(SerialStudio::DashboardFFT, m_index);

    // Set FFT window type
    if (!dataset.fftWindowFn().isEmpty())
      m_transformer.setWindowFunction(dataset.fftWindowFn());

    // Initialize FFT size
    int size = qMax(8, dataset.fftSamples());
    while (m_transformer.setSize(size) != QFourierTransformer::FixedSize)
      --size;

    // Set the size
    m_size = size;

    // Obtain sampling rate from dataset
    m_samplingRate = dataset.fftSamplingRate();

    // Allocate FFT and sample arrays
    m_fft.reset(new float[m_size]);
    m_samples.reset(new float[m_size]);

    // Set axis ranges
    m_minX = 0;
    m_maxY = 0;
    m_minY = -100;
    m_maxX = m_samplingRate / 2;

    // Obtain minimum and maximum values
    double minVal = dataset.min();
    double maxVal = dataset.max();

    // Fix inverted limits
    if (maxVal < minVal)
      std::swap(minVal, maxVal);

    // Accept only if the range is positive and non-zero
    if (maxVal - minVal > 0.0)
    {
      m_scaleIsValid = true;
      m_center = (maxVal + minVal) * 0.5;
      m_halfRange = qMax(1e-12, (maxVal - minVal) * 0.5);
    }
  }
}

/**
 * @brief Returns the minimum X-axis value.
 * @return The minimum X-axis value.
 */
double Widgets::FFTPlot::minX() const
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 * @return The maximum X-axis value.
 */
double Widgets::FFTPlot::maxX() const
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 * @return The minimum Y-axis value.
 */
double Widgets::FFTPlot::minY() const
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 * @return The maximum Y-axis value.
 */
double Widgets::FFTPlot::maxY() const
{
  return m_maxY;
}

/**
 * @brief Returns the X-axis tick interval.
 * @return The X-axis tick interval.
 */
double Widgets::FFTPlot::xTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minX, m_maxX);
}

/**
 * @brief Returns the Y-axis tick interval.
 * @return The Y-axis tick interval.
 */
double Widgets::FFTPlot::yTickInterval() const
{
  return UI::Dashboard::smartInterval(m_minY, m_maxY);
}

/**
 * @brief Draws the FFT data on the given QLineSeries.
 * @param series The QLineSeries to draw the data on.
 */
void Widgets::FFTPlot::draw(QLineSeries *series)
{
  if (series)
  {
    updateData();
    series->replace(m_data);
    Q_EMIT series->update();
  }
}

/**
 * @brief Updates the FFT data.
 */
void Widgets::FFTPlot::updateData()
{
  // Skip if widget is disabled
  if (!isEnabled())
    return;

  // Only work with valid data
  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
  {
    // Fetch time-domain data
    const auto &data = UI::Dashboard::instance().fftData(m_index);
    m_size = static_cast<int>(data.size());

    // Access the internal buffer and state of the circular queue
    const double *in = data.raw();
    std::size_t idx = data.frontIndex();
    const std::size_t cap = data.capacity();

    // Normalize time-domain input samples into [-1, 1] range
    const double offset = m_scaleIsValid ? -m_center : 0.0;
    const double scale = m_scaleIsValid ? (1.0 / m_halfRange) : 1.0;
    for (int i = 0; i < m_size; ++i)
    {
      m_samples[i] = static_cast<float>((in[idx] + offset) * scale);
      idx = (idx + 1) % cap;
    }

    // FFT processing
    m_transformer.forwardTransform(m_samples.data(), m_fft.data());
    m_transformer.rescale(m_fft.data());

    // Constants
    constexpr float floorDB = -100.0f;
    constexpr int smoothingWindow = 3;
    constexpr float eps_squared = 1e-24f;
    constexpr int halfWindow = smoothingWindow / 2;

    // Compute number of frequency bins (Nyquist rate)
    const int spectrumSize = m_size / 2;
    m_data.resize(spectrumSize);

    // Allocate dB cache only if needed
    const float *fft = m_fft.data();
    static thread_local std::vector<float> dbCache;
    if (dbCache.size() < static_cast<size_t>(spectrumSize))
      dbCache.resize(spectrumSize);

    // Precompute dB values
    for (int i = 0; i < spectrumSize; ++i)
    {
      const float re = fft[i];
      const float im = fft[i + spectrumSize];
      const float power = re * re + im * im;
      dbCache[i]
          = std::max(10.0f * std::log10(std::max(power, eps_squared)), floorDB);
    }

    // Smooth and output spectrum
    QPointF *out = m_data.data();
    for (int i = 0; i < spectrumSize; ++i)
    {
      const int minIdx = std::max(0, i - halfWindow);
      const int maxIdx = std::min(spectrumSize - 1, i + halfWindow);

      float sum = 0.0f;
      for (int k = minIdx; k <= maxIdx; ++k)
        sum += dbCache[k];

      const float smoothedDB = sum / (maxIdx - minIdx + 1);
      const float freq = static_cast<float>(i) * m_samplingRate / m_size;
      out[i].setX(freq);
      out[i].setY(smoothedDB);
    }
  }
}
