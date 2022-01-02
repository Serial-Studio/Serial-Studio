/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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
 * Constructor function, configures widget style & signal/slot connections.
 */
Widgets::FFTPlot::FFTPlot(const int index)
    : m_size(0)
    , m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

    // Initialize pointers to NULL
    m_fft = Q_NULLPTR;
    m_samples = Q_NULLPTR;

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->fftCount())
        return;

    // Set window palette
    QPalette palette;
    palette.setColor(QPalette::Base, theme->widgetWindowBackground());
    palette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(palette);

    // Set plot palette
    palette.setColor(QPalette::Base, theme->base());
    palette.setColor(QPalette::Highlight, QColor(255, 0, 0));
    palette.setColor(QPalette::Text, theme->widgetIndicator());
    palette.setColor(QPalette::Dark, theme->widgetIndicator());
    palette.setColor(QPalette::Light, theme->widgetIndicator());
    palette.setColor(QPalette::ButtonText, theme->widgetIndicator());
    palette.setColor(QPalette::WindowText, theme->widgetIndicator());
    m_plot.setPalette(palette);
    m_plot.setCanvasBackground(theme->base());
    m_plot.setFrameStyle(QFrame::Plain);

    // Configure layout
    m_layout.addWidget(&m_plot);
    m_layout.setContentsMargins(24, 24, 24, 24);
    setLayout(&m_layout);

    // Fit data horizontally
    auto xAxisEngine = m_plot.axisScaleEngine(QwtPlot::xBottom);
    xAxisEngine->setAttribute(QwtScaleEngine::Floating, true);

    // Create curve from data
    m_curve.attach(&m_plot);
    m_plot.replot();
    m_plot.show();

    // Get curve color
    QString color;
    const StringList colors = theme->widgetColors();
    if (colors.count() > m_index)
        color = colors.at(m_index);
    else
        color = colors.at(colors.count() % m_index);

    // Set curve color & plot style
    m_curve.setPen(QColor(color), 2, Qt::SolidLine);

    // Get dataset max freq. & calculate fft size
    auto dataset = UI::Dashboard::instance().getFFT(m_index);
    int size = qMax(8, dataset.fftSamples());

    // Ensure that FFT size is valid
    while (m_transformer.setSize(size) != QFourierTransformer::FixedSize)
        --size;

    // Set FFT size
    m_size = size;

    // Initialize samples & FFT arrays
    m_fft = (float *)calloc(m_size, sizeof(float));
    m_samples = (float *)calloc(m_size, sizeof(float));

    // Clear Y-axis data
    PlotData xData;
    PlotData yData;
    xData.reserve(m_size);
    yData.reserve(m_size);
    for (int i = 0; i < m_size; ++i)
    {
        yData.append(0);
        xData.append(i);
    }

    // Set y-scale from -1 to 1
    m_plot.setAxisScale(QwtPlot::yLeft, -1, 1);

    // Set axis titles
    m_plot.setAxisTitle(QwtPlot::xBottom, tr("Samples"));
    m_plot.setAxisTitle(QwtPlot::yLeft, tr("FFT of %1").arg(dataset.title()));

    // Set curve data & replot
    m_curve.setSamples(xData, yData);
    m_plot.replot();

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()), Qt::QueuedConnection);
}

/**
 * Destructor function
 */
Widgets::FFTPlot::~FFTPlot()
{
    free(m_fft);
    free(m_samples);
    m_fft = Q_NULLPTR;
    m_samples = Q_NULLPTR;
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the new data shall be saved to the plot
 * vector, but the widget shall not be redrawn.
 */
void Widgets::FFTPlot::updateData()
{
    // Verify that FFT sample arrays are valid
    if (!m_samples || !m_fft)
        return;

    // Replot
    auto plotData = UI::Dashboard::instance().fftPlotValues();
    if (plotData.count() > m_index)
    {
        // Copy data to samples array
        auto data = plotData.at(m_index);
        for (int i = 0; i < m_size; ++i)
            m_samples[i] = static_cast<float>(data[i]);

        // Execute FFT
        m_transformer.forwardTransform(m_samples, m_fft);
        m_transformer.rescale(m_fft);
        m_curve.setSamples(m_fft, m_size);
        m_plot.replot();

        // Repaint widget
        requestRepaint();
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_FFTPlot.cpp"
#endif
