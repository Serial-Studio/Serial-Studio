/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <UI/Dashboard.h>
#include <Misc/ThemeManager.h>
#include <UI/Widgets/FFTPlot.h>

/**
 * @brief Constructor to initialize the FFTPlot.
 * @param index Index of the FFT plot data.
 */
Widgets::FFTPlot::FFTPlot(int index)
  : m_size(0)
  , m_index(index)
  , m_samplingRate(0.0)
  , m_transformer(0, QStringLiteral("Hann"))
{
  // Get pointers to Serial Studio modules
  auto dash = &UI::Dashboard::instance();
  auto theme = &Misc::ThemeManager::instance();

  // Validate index
  if (m_index < 0 || m_index >= dash->fftCount())
    return;

  // Set widget palette
  QPalette palette;
  palette.setColor(QPalette::Base, theme->widgetWindowBackground());
  palette.setColor(QPalette::Window, theme->widgetWindowBackground());
  palette.setColor(QPalette::Base, theme->base());
  palette.setColor(QPalette::Highlight, QColor(255, 0, 0));
  palette.setColor(QPalette::Text, theme->widgetIndicator());
  palette.setColor(QPalette::Dark, theme->widgetIndicator());
  palette.setColor(QPalette::Light, theme->widgetIndicator());
  palette.setColor(QPalette::ButtonText, theme->widgetIndicator());
  palette.setColor(QPalette::WindowText, theme->widgetIndicator());
  setPalette(palette);

  // Configure plot appearance
  m_plot.setPalette(palette);
  m_plot.setCanvasBackground(theme->base());
  m_plot.setFrameStyle(QFrame::Plain);
  m_layout.addWidget(&m_plot);
  m_layout.setContentsMargins(8, 8, 8, 8);
  setLayout(&m_layout);

  // Configure x-axis
  auto xAxisEngine = m_plot.axisScaleEngine(QwtPlot::xBottom);
  xAxisEngine->setAttribute(QwtScaleEngine::Floating, true);

  // Attach curve to plot
  m_curve.attach(&m_plot);

  // Set curve color
  QString color;
  const auto colors = theme->widgetColors();
  color = colors.count() > m_index ? colors.at(m_index)
                                   : colors.at(colors.count() % m_index);
  m_curve.setPen(QColor(color), 2, Qt::SolidLine);

  // Initialize FFT size
  auto dataset = dash->getFFT(m_index);
  int size = qMax(8, dataset.fftSamples());
  while (m_transformer.setSize(size) != QFourierTransformer::FixedSize)
    --size;
  m_size = size;

  // Allocate FFT and sample arrays
  m_fft.reset(new float[m_size]);
  m_samples.reset(new float[m_size]);
  m_curve.setSamples(QVector<QPointF>(m_size, QPointF(0, 0)));

  // Configure plot axes and titles
  m_plot.setAxisScale(QwtPlot::yLeft, -100, 0);
  m_plot.setAxisTitle(QwtPlot::xBottom, tr("Frequency (Hz)"));
  m_plot.setAxisTitle(QwtPlot::yLeft, tr("Magnitude (dB)"));
  m_plot.replot();

  // Start timer
  m_timer.start();

  // Connect update signal
  connect(dash, &UI::Dashboard::updated, this, &FFTPlot::updateData,
          Qt::QueuedConnection);
}

/**
 * @brief Slot to update the FFT data and replot the graph.
 */
void Widgets::FFTPlot::updateData()
{
  // Measure the time elapsed since the last call
  qint64 elapsedTime = m_timer.restart();
  if (elapsedTime > 0)
    m_samplingRate = 1000.0 / static_cast<float>(elapsedTime);

  // Update FFT data and plot
  auto plotData = UI::Dashboard::instance().fftPlotValues();
  if (plotData.count() > m_index)
  {
    // Obtain samples from data
    auto data = plotData.at(m_index);
    for (int i = 0; i < m_size; ++i)
      m_samples[i] = static_cast<float>(data[i]);

    // Obtain FFT transformation
    m_transformer.forwardTransform(m_samples.data(), m_fft.data());
    m_transformer.rescale(m_fft.data());

    // Create a final array with the magnitudes for each point
    qreal maxMagnitude = 0;
    QVector<QPointF> points(m_size / 2);
    for (int i = 0; i < m_size / 2; ++i)
    {
      const qreal re = m_fft[i];
      const qreal im = m_fft[m_size / 2 + i];
      const qreal magnitude = sqrt(re * re + im * im);
      const qreal frequency = i * m_samplingRate / m_size;
      points[i] = QPointF(frequency, magnitude);
      if (magnitude > maxMagnitude)
        maxMagnitude = magnitude;
    }

    // Convert to decibels
    for (int i = 0; i < m_size / 2; ++i)
    {
      const qreal re = m_fft[i];
      const qreal im = m_fft[m_size / 2 + i];
      const qreal magnitude = sqrt(re * re + im * im) / maxMagnitude;
      const qreal dB = 20 * log10(magnitude);
      const qreal frequency = i * m_samplingRate / m_size;
      points[i] = QPointF(frequency, dB);
    }

    // Plot obtained data, remember Nyquist's theorem? :)
    m_curve.setSamples(points);
    m_plot.setAxisScale(QwtPlot::xBottom, 0, m_samplingRate / 2);
    m_plot.replot();

    // Redraw the widget
    requestRepaint();
  }
}
