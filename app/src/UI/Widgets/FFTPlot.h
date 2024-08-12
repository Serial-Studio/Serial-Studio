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

#pragma once

#include <QWidget>
#include <QwtPlot>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QwtPlotCurve>
#include <QwtScaleEngine>
#include <QScopedArrayPointer>
#include <qfouriertransformer.h>
#include <UI/DashboardWidget.h>

namespace Widgets
{

/**
 * @class FFTPlot
 * @brief Class to plot FFT data using QwtPlot and QFourierTransformer.
 */
class FFTPlot : public DashboardWidgetBase
{
  Q_OBJECT

public:
  FFTPlot(int index);

private slots:
  void updateData();
  void onThemeChanged();

private:
  int m_size;                       ///< Size of the FFT data array.
  int m_index;                      ///< Index of the FFT plot data.
  float m_samplingRate;             ///< Sampling rate calculated dynamically.
  QElapsedTimer m_timer;            ///< Timer to measure time between updates.
  QwtPlot m_plot;                   ///< Plot widget for FFT.
  QwtPlotCurve m_curve;             ///< Curve for the FFT data.
  QVBoxLayout m_layout;             ///< Layout for the plot widget.
  QScopedArrayPointer<float> m_fft; ///< FFT data array.
  QScopedArrayPointer<float> m_samples; ///< Sample data array.
  QFourierTransformer m_transformer;    ///< Fourier transformer for FFT.
};

} // namespace Widgets
