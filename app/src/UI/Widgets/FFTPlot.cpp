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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/FFTPlot.h"

#include <algorithm>
#include <cmath>

#include "DSPSimd.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/FFTWindow.h"
#include "UI/Widgets/PlotLogScale.h"

#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/AudioExport.h"
#endif

//--------------------------------------------------------------------------------------------------
// Log-axis display constants
//--------------------------------------------------------------------------------------------------

static constexpr int kLogRenderPoints   = 2048;
static constexpr float kSpectrumFloorDb = -100.0f;
static constexpr float kSpectrumEpsSq   = 1e-24f;

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a new FFTPlot widget.
 */
Widgets::FFTPlot::FFTPlot(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_dashboard(UI::Dashboard::instance())
  , m_logX(false)
  , m_size(0)
  , m_index(index)
  , m_samplingRate(0)
  , m_ballistics(false)
  , m_releaseMs(300)
  , m_releaseAlpha(1.0f)
  , m_dataW(0)
  , m_dataH(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_center(0)
  , m_halfRange(1)
  , m_scaleIsValid(false)
  , m_windowType(SerialStudio::FFTWindowBlackmanHarris)
  , m_plan(nullptr)
#ifdef BUILD_COMMERCIAL
  , m_audioExport(Widgets::AudioExport::instance())
  , m_audioRecordingEnabled(false)
#endif
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardFFT, m_index);
    m_size              = Widgets::normalizedFftSize(dataset.fftSamples);
    m_samplingRate      = qMax(1, dataset.fftSamplingRate);
    m_windowType        = static_cast<SerialStudio::FFTWindow>(dataset.fftWindow);
    m_minX              = 0;
    m_maxY              = 0;
    m_minY              = -100;
    m_maxX              = m_samplingRate / 2;
    m_logX              = dataset.fftLogX;
    m_ballistics        = dataset.fftBallistics;
    m_releaseMs         = qBound(50, dataset.fftBallisticsRelease, 5000);
    if (m_logX) {
      rebuildLogBinTable();
      applyLogFrequencyBounds();
    }

    m_samples.resize(m_size);
    m_fftOutput.resize(m_size / 2 + 1);
    m_window.resize(m_size);
    Widgets::fillFftWindow(m_windowType, m_window.data(), static_cast<unsigned int>(m_size));

    loadMarkers();
    rebuildMarkerBins();

    m_plan = kiss_fftr_alloc(m_size, 0, nullptr, nullptr);
    if (!m_plan) {
      qWarning() << "FFT plan allocation failed for size:" << m_size;
      return;
    }

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

#ifdef BUILD_COMMERCIAL
  connect(&m_audioExport, &Widgets::AudioExport::sessionsClosed, this, [this] {
    if (!m_audioRecordingEnabled)
      return;

    m_audioRecordingEnabled = false;
    m_dashboard.setFftAudioTap(m_index, false, 0);
    Q_EMIT audioRecordingEnabledChanged();
  });

  connect(&m_audioExport, &Widgets::AudioExport::sessionClosed, this, [this](quint32 key) {
    if (key != Widgets::AudioExport::sessionKey(SerialStudio::DashboardFFT, m_index)
        || !m_audioRecordingEnabled)
      return;

    m_audioRecordingEnabled = false;
    m_dashboard.setFftAudioTap(m_index, false, 0);
    Q_EMIT audioRecordingEnabledChanged();
  });
#endif
}

/**
 * @brief Releases the FFT plan and, on commercial builds, finalises any active recording session.
 */
Widgets::FFTPlot::~FFTPlot()
{
#ifdef BUILD_COMMERCIAL
  if (m_audioRecordingEnabled) {
    m_dashboard.setFftAudioTap(m_index, false, 0);
    m_audioExport.closeSession(SerialStudio::DashboardFFT, m_index);
  }
#endif

  if (m_plan) {
    kiss_fftr_free(m_plan);
    m_plan = nullptr;
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

/**
 * @brief Returns true when the frequency axis renders in log10 space.
 */
bool Widgets::FFTPlot::logX() const noexcept
{
  return m_logX;
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether plot data updates are currently active.
 */
bool Widgets::FFTPlot::running() const noexcept
{
  return m_dashboard.fftPlotRunning(m_index);
}

/**
 * @brief Returns the current interpolation mode.
 */
SerialStudio::InterpolationMode Widgets::FFTPlot::interpolationMode() const noexcept
{
  return m_base.interpolationMode();
}

//--------------------------------------------------------------------------------------------------
// Frequency markers (spec 0019)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the static marker configuration list (built once at construction).
 */
QVariantList Widgets::FFTPlot::markers() const
{
  return m_markerConfig;
}

/**
 * @brief Returns the latest peak display dB measured inside the marker's bin window.
 */
double Widgets::FFTPlot::markerPeakDb(int index) const
{
  if (index < 0 || index >= static_cast<int>(m_markerRt.size()))
    return kSpectrumFloorDb;

  return m_markerRt[static_cast<std::size_t>(index)].peakDb;
}

/**
 * @brief Returns the marker's escalation state: 0 = normal, 1 = warning, 2 = alarm.
 */
int Widgets::FFTPlot::markerState(int index) const
{
  if (index < 0 || index >= static_cast<int>(m_markerRt.size()))
    return 0;

  return m_markerRt[static_cast<std::size_t>(index)].state;
}

/**
 * @brief Copies the dataset's frequency markers into runtime state and the QML config list;
 *        runs once at construction so the per-tick path never touches the project model.
 */
void Widgets::FFTPlot::loadMarkers()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
    return;

  const auto& dataset = GET_DATASET(SerialStudio::DashboardFFT, m_index);
  m_markerRt.clear();
  m_markerConfig.clear();
  m_markerRt.reserve(dataset.fftMarkers.size());
  m_markerConfig.reserve(static_cast<int>(dataset.fftMarkers.size()));
  for (const auto& m : dataset.fftMarkers) {
    MarkerRuntime rt;
    rt.freqLo    = m.frequency;
    rt.freqHi    = m.endFrequency;
    rt.warningDb = static_cast<float>(m.warningDb);
    rt.alarmDb   = static_cast<float>(m.alarmDb);
    rt.peakDb    = kSpectrumFloorDb;
    rt.state     = 0;
    rt.binLo     = 0;
    rt.binHi     = 0;
    m_markerRt.push_back(rt);

    QVariantMap entry;
    entry.insert(QStringLiteral("freq"), m.frequency);
    entry.insert(QStringLiteral("endFreq"), m.endFrequency);
    entry.insert(QStringLiteral("label"), m.label);
    entry.insert(QStringLiteral("color"), m.color);
    entry.insert(QStringLiteral("hasThresholds"),
                 std::isfinite(m.warningDb) || std::isfinite(m.alarmDb));
    m_markerConfig.append(entry);
  }
}

/**
 * @brief Resolves each marker's frequency span to a clamped FFT bin window; point markers get
 *        a +/- 2 bin neighborhood so slightly detuned lines still register. Re-run on every
 *        plan rebuild because the bin width follows the FFT size. Bin math clamps in the
 *        double domain BEFORE the int cast: casting an unrepresentable double is UB.
 */
void Widgets::FFTPlot::rebuildMarkerBins()
{
  constexpr double pointHalfWindow = 2.0;
  SS_ASSERT(m_size > 0, return);
  SS_ASSERT(m_samplingRate > 0, return);

  const int spectrumSize = m_size / 2;
  const double freqStep  = static_cast<double>(m_samplingRate) / qMax(1, m_size);
  const double lastBin   = qMax(0, spectrumSize - 1);
  for (auto& rt : m_markerRt) {
    double lo = 0.0;
    double hi = 0.0;
    if (rt.freqHi > rt.freqLo) {
      lo = std::floor(rt.freqLo / freqStep);
      hi = std::ceil(rt.freqHi / freqStep);
    } else {
      const double center = std::round(rt.freqLo / freqStep);
      lo                  = center - pointHalfWindow;
      hi                  = center + pointHalfWindow;
    }

    rt.binLo = static_cast<int>(qBound(0.0, lo, lastBin));
    rt.binHi = static_cast<int>(qBound(static_cast<double>(rt.binLo), hi, lastBin));
  }
}

/**
 * @brief Refreshes each marker's live peak (from the ballistics-processed display spectrum, so
 *        what is judged matches what is drawn) and its state. Returns whether the readout the
 *        chips render moved: the signal used to fire on every tick regardless (F15).
 */
bool Widgets::FFTPlot::updateMarkerValues(const int spectrumSize)
{
  SS_ASSERT(spectrumSize > 0, return false);
  SS_ASSERT(m_binDb.size() >= static_cast<std::size_t>(spectrumSize), return false);

  bool changed = false;
  for (auto& rt : m_markerRt) {
    const int hi = qMin(rt.binHi, spectrumSize - 1);
    float peak   = kSpectrumFloorDb;
    for (int i = qMin(rt.binLo, hi); i <= hi; ++i)
      peak = std::max(peak, m_binDb[static_cast<std::size_t>(i)]);

    const int previous_state = rt.state;
    const int previous_label = qRound(rt.peakDb * 10.0f);

    rt.peakDb = peak;
    if (std::isfinite(rt.alarmDb) && peak >= rt.alarmDb)
      rt.state = 2;
    else if (std::isfinite(rt.warningDb) && peak >= rt.warningDb)
      rt.state = 1;
    else
      rt.state = 0;

    changed |= rt.state != previous_state || qRound(rt.peakDb * 10.0f) != previous_label;
  }

  return changed;
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
    if (m_base.interpolationMode() == SerialStudio::InterpolationZoh
        || m_base.interpolationMode() == SerialStudio::InterpolationStem) {
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
  m_dashboard.setFFTPlotRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Updates the interpolation mode used by the FFT plot.
 */
void Widgets::FFTPlot::setInterpolationMode(SerialStudio::InterpolationMode mode)
{
  if (!m_base.setInterpolationMode(mode))
    return;

  Q_EMIT interpolationModeChanged();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Returns whether WAV recording of the widget's time-domain input is active (Pro).
 */
bool Widgets::FFTPlot::audioRecordingEnabled() const noexcept
{
  return m_audioRecordingEnabled;
}

/**
 * @brief Resolves the shared WAV output folder live from the current dataset title, so a dataset
 *        rename after construction still reveals the correct recordings folder.
 */
QString Widgets::FFTPlot::recordingsFolder() const
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
    return QString();

  return m_audioExport.audioPath(GET_DATASET(SerialStudio::DashboardFFT, m_index).title,
                                 m_dashboard.title());
}

/**
 * @brief Toggles WAV recording of the input signal; enabling is refused without a Pro license,
 *        disabling always disarms the Dashboard tap before finalising the session.
 */
void Widgets::FFTPlot::setAudioRecordingEnabled(const bool enabled)
{
  if (m_audioRecordingEnabled == enabled)
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
    return;

  if (enabled) {
    if (!SerialStudio::activated())
      return;

    const auto& dataset = GET_DATASET(SerialStudio::DashboardFFT, m_index);
    AudioSessionConfig cfg;
    cfg.sampleRate   = m_samplingRate;
    cfg.uniqueId     = dataset.uniqueId;
    cfg.useScale     = m_scaleIsValid;
    cfg.center       = m_center;
    cfg.halfRange    = m_halfRange;
    cfg.datasetTitle = dataset.title;
    cfg.projectTitle = m_dashboard.title();

    m_audioExport.openSession(SerialStudio::DashboardFFT, m_index, cfg);
    m_dashboard.setFftAudioTap(
      m_index, true, Widgets::AudioExport::sessionKey(SerialStudio::DashboardFFT, m_index));
  } else {
    m_dashboard.setFftAudioTap(m_index, false, 0);
    m_audioExport.closeSession(SerialStudio::DashboardFFT, m_index);
  }

  m_audioRecordingEnabled = enabled;
  Q_EMIT audioRecordingEnabledChanged();
}
#endif

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the FFT plan and window when the input size changes.
 */
bool Widgets::FFTPlot::rebuildFftPlan(int newSize)
{
  SS_ASSERT(newSize % 2 == 0, return false);

  m_size = newSize;

  m_window.resize(m_size);
  m_samples.resize(m_size);
  m_fftOutput.resize(m_size / 2 + 1);

  Widgets::fillFftWindow(m_windowType, m_window.data(), static_cast<unsigned int>(m_size));

  if (m_plan) {
    kiss_fftr_free(m_plan);
    m_plan = nullptr;
  }

  m_plan = kiss_fftr_alloc(m_size, 0, nullptr, nullptr);
  if (!m_plan) {
    qWarning() << "FFT plan allocation failed for size:" << m_size;
    return false;
  }

  if (m_logX) {
    rebuildLogBinTable();
    applyLogFrequencyBounds();
  }

  rebuildMarkerBins();
  return true;
}

/**
 * @brief Maps the frequency axis bounds into log10 space: the lower bound sits on the
 *        first FFT bin (the closest a log axis gets to 0 Hz, so nothing the analysis
 *        captures is cropped), the upper bound on Nyquist.
 */
void Widgets::FFTPlot::applyLogFrequencyBounds()
{
  SS_ASSERT(m_size > 0, return);
  SS_ASSERT(m_samplingRate > 0, return);

  const double freqStep = static_cast<double>(m_samplingRate) / qMax(1, m_size);
  m_minX                = LogScale::clampedLog10(freqStep);
  m_maxX                = LogScale::clampedLog10(m_samplingRate * 0.5, freqStep);
}

/**
 * @brief Rebuilds the cached log10 position of every FFT bin (bin 0 clamps onto bin 1's
 *        position, the closest a log axis gets to DC) and sizes the render buffers for
 *        the interpolated log curve -- steady state stays allocation-free.
 */
void Widgets::FFTPlot::rebuildLogBinTable()
{
  SS_ASSERT(m_size > 0, return);
  SS_ASSERT(m_samplingRate > 0, return);

  const int spectrumSize = m_size / 2;
  const double freqStep  = static_cast<double>(m_samplingRate) / m_size;
  m_logBinX.resize(static_cast<std::size_t>(spectrumSize));
  m_pchipSlope.resize(static_cast<std::size_t>(spectrumSize));
  for (int i = 0; i < spectrumSize; ++i) {
    const double freq = i * freqStep;
    m_logBinX[static_cast<std::size_t>(i)] =
      static_cast<float>(LogScale::clampedLog10(freq, freqStep));
  }

  if (m_xData.size() != static_cast<std::size_t>(kLogRenderPoints)) {
    m_xData.resize(kLogRenderPoints);
    m_xData.clear();
    m_yData.resize(kLogRenderPoints);
    m_yData.clear();
  }
}

/**
 * @brief Converts the FFT output to smoothed display dB per bin (shared 1/N^2 power
 *        norm, 3-bin boxcar, then the optional ballistics envelope) into m_binDb.
 */
void Widgets::FFTPlot::computeBinSpectrum(int spectrumSize)
{
  constexpr int halfWindow = 1;
  SS_ASSERT(spectrumSize > 0, return);
  SS_ASSERT(m_fftOutput.size() >= static_cast<std::size_t>(spectrumSize),
            spectrumSize = static_cast<int>(m_fftOutput.size()));

  static thread_local std::vector<float> dbCache;
  if (dbCache.size() < static_cast<size_t>(spectrumSize))
    dbCache.resize(spectrumSize);

  static_assert(sizeof(kiss_fft_cpx) == 2 * sizeof(float), "kiss_fft_cpx must pack two floats");

  const float normFactor = static_cast<float>(m_size) * static_cast<float>(m_size);
  const float invNorm    = 1.0f / normFactor;
  DSP::simdPowerSpectrumDb(reinterpret_cast<const float*>(m_fftOutput.data()),
                           dbCache.data(),
                           static_cast<std::size_t>(spectrumSize),
                           invNorm,
                           kSpectrumEpsSq,
                           kSpectrumFloorDb);

  if (m_binDb.size() != static_cast<std::size_t>(spectrumSize))
    m_binDb.resize(static_cast<std::size_t>(spectrumSize));

  if (m_ballistics && m_displayDb.size() != static_cast<std::size_t>(spectrumSize))
    resetBallistics(spectrumSize);

  updateBallisticsAlpha();

  for (int i = 0; i < spectrumSize; ++i) {
    const int minIdx = std::max(0, i - halfWindow);
    const int maxIdx = std::min(spectrumSize - 1, i + halfWindow);

    float sum = 0.0f;
    for (int k = minIdx; k <= maxIdx; ++k)
      sum += dbCache[k];

    const float smoothedDB               = sum / static_cast<float>(maxIdx - minIdx + 1);
    m_binDb[static_cast<std::size_t>(i)] = applyBallistics(static_cast<std::size_t>(i), smoothedDB);
  }
}

/**
 * @brief Pushes the per-bin display spectrum on the linear frequency axis (the
 *        pre-existing rendering, unchanged in shape).
 */
void Widgets::FFTPlot::emitLinearSpectrum(int spectrumSize)
{
  SS_ASSERT(spectrumSize > 0, return);
  SS_ASSERT(m_binDb.size() == static_cast<std::size_t>(spectrumSize),
            spectrumSize = static_cast<int>(m_binDb.size()));

  if (m_xData.size() != static_cast<size_t>(spectrumSize)) {
    m_xData.resize(spectrumSize);
    m_xData.clear();
    m_yData.resize(spectrumSize);
    m_yData.clear();
  }

  const float freqStep = static_cast<float>(m_samplingRate) / static_cast<float>(qMax(1, m_size));
  for (int i = 0; i < spectrumSize; ++i) {
    m_xData.push(static_cast<float>(i) * freqStep);
    m_yData.push(m_binDb[static_cast<std::size_t>(i)]);
  }
}

/**
 * @brief Renders the log-axis curve the studio-analyzer way: a monotone cubic (Fritsch-Carlson
 *        PCHIP) through the bins in log-x space, resampled on a uniform log grid, so the
 *        sparse low decades draw as smooth hills instead of angular segments. Monotone
 *        interpolation never overshoots, so peaks stay honest.
 */
void Widgets::FFTPlot::buildLogRenderCurve(const int spectrumSize)
{
  const int first = 1;
  const int last  = spectrumSize - 1;
  SS_ASSERT(spectrumSize >= 4, return);
  SS_ASSERT(m_logBinX.size() == static_cast<std::size_t>(spectrumSize)
              && m_pchipSlope.size() == static_cast<std::size_t>(spectrumSize)
              && m_binDb.size() >= static_cast<std::size_t>(spectrumSize),
            return);

  const float* xs = m_logBinX.data();
  const float* ys = m_binDb.data();
  float* slope    = m_pchipSlope.data();

  float hPrev  = xs[first + 1] - xs[first];
  float dPrev  = (ys[first + 1] - ys[first]) / hPrev;
  slope[first] = dPrev;
  for (int i = first + 1; i < last; ++i) {
    const float h    = xs[i + 1] - xs[i];
    const float dCur = (ys[i + 1] - ys[i]) / h;
    if (dPrev * dCur <= 0.0f)
      slope[i] = 0.0f;
    else {
      const float w1 = 2.0f * h + hPrev;
      const float w2 = h + 2.0f * hPrev;
      slope[i]       = (w1 + w2) / (w1 / dPrev + w2 / dCur);
    }

    hPrev = h;
    dPrev = dCur;
  }
  slope[last] = dPrev;

  const float x0 = xs[first];
  const float x1 = xs[last];
  const float dx = (x1 - x0) / static_cast<float>(kLogRenderPoints - 1);
  int seg        = first;
  for (int j = 0; j < kLogRenderPoints; ++j) {
    const float x = x0 + static_cast<float>(j) * dx;
    // code-verify off
    while (seg + 1 < last && xs[seg + 1] < x)
      ++seg;
    // code-verify on

    const float h   = xs[seg + 1] - xs[seg];
    const float t   = qBound(0.0f, (x - xs[seg]) / h, 1.0f);
    const float t2  = t * t;
    const float t3  = t2 * t;
    const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    const float h10 = t3 - 2.0f * t2 + t;
    const float h01 = -2.0f * t3 + 3.0f * t2;
    const float h11 = t3 - t2;
    const float y =
      h00 * ys[seg] + h10 * h * slope[seg] + h01 * ys[seg + 1] + h11 * h * slope[seg + 1];

    m_xData.push(x);
    m_yData.push(y);
  }
}

/**
 * @brief Refreshes the wall-clock release coefficient once per displayed frame; the
 *        first frame after a reset jumps straight to the fresh value (alpha = 1).
 */
void Widgets::FFTPlot::updateBallisticsAlpha()
{
  if (!m_ballistics)
    return;

  if (!m_ballisticsClock.isValid()) {
    m_ballisticsClock.start();
    m_releaseAlpha = 1.0f;
    return;
  }

  const double dt  = m_ballisticsClock.nsecsElapsed() * 1e-9;
  const double tau = m_releaseMs * 1e-3;
  m_ballisticsClock.restart();
  m_releaseAlpha = static_cast<float>(1.0 - std::exp(-dt / tau));
}

/**
 * @brief Resets the per-bin display state to the dB floor and invalidates the release
 *        clock, so the next frame attacks cleanly instead of decaying from stale bins.
 */
void Widgets::FFTPlot::resetBallistics(const int bins)
{
  SS_ASSERT(bins > 0, return);
  m_displayDb.assign(static_cast<std::size_t>(bins), kSpectrumFloorDb);
  m_ballisticsClock.invalidate();
}

/**
 * @brief Display-only envelope per emitted bin: instant attack (peaks never
 *        under-read), exponential wall-clock release toward lower fresh values.
 */
float Widgets::FFTPlot::applyBallistics(const std::size_t idx, const float freshDb)
{
  if (!m_ballistics)
    return freshDb;

  SS_ASSERT(idx < m_displayDb.size(), return freshDb);
  float& shown = m_displayDb[idx];
  shown        = freshDb >= shown ? freshDb : shown + (freshDb - shown) * m_releaseAlpha;
  return shown;
}

/**
 * @brief Updates the FFT data. The plan is sized from the ring's capacity (the configured FFT
 *        size), never its fill level, so a filling ring cannot thrash plan reallocation; the
 *        unfilled tail is zero-padded instead.
 */
void Widgets::FFTPlot::updateData()
{
  static thread_local DSP::DownsampleWorkspace ws;

  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardFFT, m_index))
    return;

  const auto& data  = m_dashboard.fftData(m_index);
  const int newSize = static_cast<int>(data.capacity());
  if (newSize != m_size && !rebuildFftPlan(newSize))
    return;

  if (newSize <= 0)
    return;

  if (!m_plan)
    return;

  const int avail = static_cast<int>(std::min(data.size(), static_cast<std::size_t>(m_size)));

  const double* in       = data.raw();
  const std::size_t mask = data.storageMask();
  const double offset    = m_scaleIsValid ? -m_center : 0.0;
  const double scale     = m_scaleIsValid ? (1.0 / m_halfRange) : 1.0;

  static_assert(sizeof(kiss_fft_scalar) == sizeof(float));

  if (avail > 0)
    DSP::simdWindowedRealFill(in,
                              data.frontIndex(),
                              mask,
                              static_cast<std::size_t>(avail),
                              offset,
                              scale,
                              m_window.data(),
                              m_samples.data());

  std::fill(m_samples.begin() + avail, m_samples.end(), 0.0f);

  kiss_fftr(m_plan, m_samples.data(), m_fftOutput.data());
  const int spectrumSize = m_size / 2;
  computeBinSpectrum(spectrumSize);
  if (m_logX && spectrumSize >= 4)
    buildLogRenderCurve(spectrumSize);
  else
    emitLinearSpectrum(spectrumSize);

  if (!m_markerRt.empty() && spectrumSize > 0 && updateMarkerValues(spectrumSize))
    Q_EMIT markerValuesChanged();

  DSP::downsampleMonotonic(m_xData, m_yData, m_dataW, m_dataH, m_data, &ws);
}

/**
 * @brief Rebuilds the render data for ZOH or stem interpolation modes.
 */
void Widgets::FFTPlot::updateInterpolatedData()
{
  const int n = m_data.size();

  if (m_base.interpolationMode() == SerialStudio::InterpolationZoh) {
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

  if (m_base.interpolationMode() == SerialStudio::InterpolationStem) {
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
