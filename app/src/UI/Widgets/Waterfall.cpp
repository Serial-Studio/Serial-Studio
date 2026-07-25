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

#include "UI/Widgets/Waterfall.h"

#include <algorithm>
#include <QCursor>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QtMath>
#include <QWheelEvent>

#include "DSPSimd.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/AudioExport.h"
#include "UI/Widgets/FFTWindow.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kMaxFftSamples    = 65536;
static constexpr int kDefaultHistory   = 256;
static constexpr int kMaxHistorySize   = 4096;
static constexpr float kFloorDb        = -100.0f;
static constexpr float kEpsSquared     = 1e-24f;
static constexpr int kSmoothingWindow  = 3;
static constexpr int kHalfSmoothWindow = kSmoothingWindow / 2;
static constexpr int kAxisTickPx       = 4;
static constexpr int kAxisLabelPad     = 4;
static constexpr int kAxisTickCount    = 6;
static constexpr int kMinAxisWidth     = 200;
static constexpr int kMinAxisHeight    = 160;
static constexpr double kMaxZoom       = 32.0;
static constexpr int kMarkerChipRows   = 3;
static constexpr double kLn10          = 2.302585092994046;

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the largest power-of-two not exceeding @a value (and >= 8).
 */
static inline int floorPow2Bounded(int value)
{
  const int clamped = qBound(8, value, kMaxFftSamples);
  return 1 << static_cast<int>(std::log2(clamped));
}

/**
 * @brief Integer-exponent 10^n via table lookup; falls back to std::pow if out of band.
 */
static inline double fastPow10(double exponent) noexcept
{
  static constexpr double kTable[] = {
    1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
    1e-4,  1e-3,  1e-2,  1e-1,  1e0,   1e1,   1e2,  1e3,  1e4,  1e5,  1e6,
    1e7,   1e8,   1e9,   1e10,  1e11,  1e12,  1e13, 1e14, 1e15,
  };
  const int idx = static_cast<int>(exponent) + 15;
  if (idx < 0 || idx >= static_cast<int>(sizeof(kTable) / sizeof(kTable[0]))) [[unlikely]]
    return std::pow(10.0, exponent);

  return kTable[idx];
}

/**
 * @brief Linearly interpolates a color from per-channel LUT arrays of length n.
 */
QRgb Widgets::Waterfall::interpolateLut(
  const double* r, const double* g, const double* b, int n, double t)
{
  const double f = t * (n - 1);
  const int i    = qBound(0, static_cast<int>(f), n - 2);
  const double s = f - i;
  const int rr   = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
  const int gg   = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
  const int bb   = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);

  return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
}

/**
 * @brief Returns the RGB color for a given color map and normalized magnitude.
 */
QRgb Widgets::Waterfall::sampleColorMap(int map, double t)
{
  t = qBound(0.0, t, 1.0);

  switch (map) {
    case Viridis: {
      static constexpr double r[] = {
        0.267, 0.282, 0.253, 0.207, 0.164, 0.135, 0.135, 0.267, 0.478, 0.741, 0.993};
      static constexpr double g[] = {
        0.005, 0.100, 0.265, 0.371, 0.471, 0.567, 0.659, 0.749, 0.821, 0.873, 0.906};
      static constexpr double b[] = {
        0.329, 0.529, 0.529, 0.553, 0.557, 0.553, 0.518, 0.440, 0.318, 0.150, 0.144};
      return interpolateLut(r, g, b, 11, t);
    }

    case Inferno: {
      static constexpr double r[] = {0.001, 0.099, 0.301, 0.527, 0.733, 0.882, 0.973, 0.988};
      static constexpr double g[] = {0.000, 0.034, 0.064, 0.117, 0.214, 0.388, 0.626, 0.998};
      static constexpr double b[] = {0.014, 0.299, 0.434, 0.395, 0.276, 0.118, 0.034, 0.645};
      return interpolateLut(r, g, b, 8, t);
    }

    case Magma: {
      static constexpr double r[] = {0.001, 0.146, 0.421, 0.715, 0.928, 0.987, 0.987};
      static constexpr double g[] = {0.000, 0.060, 0.139, 0.215, 0.473, 0.749, 0.991};
      static constexpr double b[] = {0.014, 0.347, 0.516, 0.475, 0.502, 0.622, 0.749};
      return interpolateLut(r, g, b, 7, t);
    }

    case Plasma: {
      static constexpr double r[] = {0.050, 0.286, 0.530, 0.741, 0.892, 0.969, 0.940};
      static constexpr double g[] = {0.030, 0.010, 0.140, 0.347, 0.560, 0.789, 0.975};
      static constexpr double b[] = {0.527, 0.629, 0.586, 0.415, 0.227, 0.105, 0.131};
      return interpolateLut(r, g, b, 7, t);
    }

    case Turbo: {
      static constexpr double r[] = {0.190, 0.275, 0.247, 0.085, 0.152, 0.617, 0.964, 0.974, 0.479};
      static constexpr double g[] = {0.072, 0.366, 0.703, 0.916, 0.988, 0.983, 0.787, 0.317, 0.016};
      static constexpr double b[] = {0.232, 0.804, 0.964, 0.757, 0.357, 0.141, 0.180, 0.108, 0.011};
      return interpolateLut(r, g, b, 9, t);
    }

    case Jet: {
      const double v = t;
      const double r = qBound(0.0, qMin(4.0 * v - 1.5, 4.5 - 4.0 * v), 1.0);
      const double g = qBound(0.0, qMin(4.0 * v - 0.5, 3.5 - 4.0 * v), 1.0);
      const double b = qBound(0.0, qMin(4.0 * v + 0.5, 2.5 - 4.0 * v), 1.0);
      return qRgb(
        static_cast<int>(r * 255.0), static_cast<int>(g * 255.0), static_cast<int>(b * 255.0));
    }

    case Hot: {
      const double v = t;
      const double r = qBound(0.0, 3.0 * v, 1.0);
      const double g = qBound(0.0, 3.0 * v - 1.0, 1.0);
      const double b = qBound(0.0, 3.0 * v - 2.0, 1.0);
      return qRgb(
        static_cast<int>(r * 255.0), static_cast<int>(g * 255.0), static_cast<int>(b * 255.0));
    }

    case Grayscale:
    default: {
      const int v = static_cast<int>(t * 255.0);
      return qRgb(v, v, v);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Waterfall widget bound to the dataset at @a index.
 */
Widgets::Waterfall::Waterfall(const int index, QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_index(index)
  , m_size(0)
  , m_samplingRate(0)
  , m_windowType(SerialStudio::FFTWindowBlackmanHarris)
  , m_historySize(kDefaultHistory)
  , m_colorMap(Turbo)
  , m_writeRow(0)
  , m_topRow(0)
  , m_filledOnce(false)
  , m_axisVisible(true)
  , m_markersVisible(true)
  , m_colorbarVisible(true)
  , m_minDb(-100.0)
  , m_maxDb(0.0)
  , m_center(0.0)
  , m_halfRange(1.0)
  , m_xZoom(1.0)
  , m_yZoom(1.0)
  , m_xPan(0.0)
  , m_yPan(0.0)
  , m_scaleIsValid(false)
  , m_dragging(false)
  , m_cursorEnabled(false)
  , m_cursorHovering(false)
  , m_axisDirty(true)
  , m_campbellMode(false)
  , m_yDatasetUniqueId(0)
  , m_yMin(0.0)
  , m_yMax(1.0)
  , m_logX(false)
  , m_logActive(false)
  , m_logMin(0.0)
  , m_logMax(1.0)
  , m_selectedMarker(-1)
  , m_plan(nullptr)
  , m_dashboard(UI::Dashboard::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_commonFonts(Misc::CommonFonts::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_audioExport(Widgets::AudioExport::instance())
  , m_audioRecordingEnabled(false)
{
  setAcceptedMouseButtons(Qt::LeftButton);
  setAcceptHoverEvents(true);

  if (VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
    m_size              = floorPow2Bounded(dataset.fftSamples);
    m_samplingRate      = qMax(1, dataset.fftSamplingRate);
    m_windowType        = static_cast<SerialStudio::FFTWindow>(dataset.fftWindow);
    m_logX              = dataset.fftLogX;

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

    m_yDatasetUniqueId = dataset.waterfallYAxis;
    if (m_yDatasetUniqueId > 0) {
      const auto& datasets = m_dashboard.datasets();
      const auto it        = datasets.find(m_yDatasetUniqueId);
      if (it != datasets.end() && it->pltMax > it->pltMin) {
        m_campbellMode = true;
        m_yMin         = it->pltMin;
        m_yMax         = it->pltMax;
        m_yAxisTitle =
          it->units.isEmpty() ? it->title : QStringLiteral("%1 (%2)").arg(it->title, it->units);
      }
    }

    allocateFftPlan(m_size);
    rebuildHistoryImage();
    loadMarkers();
  }

  onThemeChanged();

  connect(&m_dashboard, &UI::Dashboard::updated, this, &Widgets::Waterfall::updateData);

  connect(
    &m_themeManager, &Misc::ThemeManager::themeChanged, this, &Widgets::Waterfall::onThemeChanged);
  connect(
    &m_commonFonts, &Misc::CommonFonts::fontsChanged, this, &Widgets::Waterfall::onFontsChanged);

  connect(&m_dashboard,
          &UI::Dashboard::plotTimeRangeChanged,
          this,
          &Widgets::Waterfall::syncHistoryToTimeRange);
  connect(&m_timerEvents,
          &Misc::TimerEvents::fpsChanged,
          this,
          &Widgets::Waterfall::syncHistoryToTimeRange);
  syncHistoryToTimeRange();

  connect(&m_audioExport, &Widgets::AudioExport::sessionsClosed, this, [this] {
    if (!m_audioRecordingEnabled)
      return;

    m_audioRecordingEnabled = false;
    m_dashboard.setWaterfallAudioTap(m_index, false, 0);
    Q_EMIT audioRecordingEnabledChanged();
  });

  connect(&m_audioExport, &Widgets::AudioExport::sessionClosed, this, [this](quint32 key) {
    if (key != Widgets::AudioExport::sessionKey(SerialStudio::DashboardWaterfall, m_index)
        || !m_audioRecordingEnabled)
      return;

    m_audioRecordingEnabled = false;
    m_dashboard.setWaterfallAudioTap(m_index, false, 0);
    Q_EMIT audioRecordingEnabledChanged();
  });
}

/**
 * @brief Releases the FFT plan and finalises any active recording session.
 */
Widgets::Waterfall::~Waterfall()
{
  if (m_audioRecordingEnabled) {
    m_dashboard.setWaterfallAudioTap(m_index, false, 0);
    m_audioExport.closeSession(SerialStudio::DashboardWaterfall, m_index);
  }

  releaseFftPlan();
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether updates are currently active.
 */
bool Widgets::Waterfall::running() const noexcept
{
  return m_dashboard.waterfallRunning(m_index);
}

/**
 * @brief Returns the active color map identifier.
 */
int Widgets::Waterfall::colorMap() const noexcept
{
  return m_colorMap;
}

/**
 * @brief Returns the number of time slices stored in the history.
 */
int Widgets::Waterfall::historySize() const noexcept
{
  return m_historySize;
}

/**
 * @brief Returns the sampling rate from the dataset's FFT settings.
 */
int Widgets::Waterfall::samplingRate() const noexcept
{
  return m_samplingRate;
}

/**
 * @brief Returns the FFT window size in use.
 */
int Widgets::Waterfall::fftSize() const noexcept
{
  return m_size;
}

/**
 * @brief Returns the lower clip of the dB range mapped to the color map start.
 */
double Widgets::Waterfall::minDb() const noexcept
{
  return m_minDb;
}

/**
 * @brief Returns the upper clip of the dB range mapped to the color map end.
 */
double Widgets::Waterfall::maxDb() const noexcept
{
  return m_maxDb;
}

/**
 * @brief Returns the lowest displayed frequency (always 0 Hz).
 */
double Widgets::Waterfall::minFreq() const noexcept
{
  return 0.0;
}

/**
 * @brief Returns the highest displayed frequency (Nyquist limit).
 */
double Widgets::Waterfall::maxFreq() const noexcept
{
  return m_samplingRate * 0.5;
}

/**
 * @brief Returns the number of available built-in color maps.
 */
int Widgets::Waterfall::colorMapCount() const noexcept
{
  return static_cast<int>(ColorMapCount);
}

/**
 * @brief Returns a human-readable label for the requested color map.
 */
QString Widgets::Waterfall::colorMapName(int index) const
{
  switch (index) {
    case Viridis:
      return tr("Viridis");
    case Inferno:
      return tr("Inferno");
    case Magma:
      return tr("Magma");
    case Plasma:
      return tr("Plasma");
    case Turbo:
      return tr("Turbo");
    case Jet:
      return tr("Jet");
    case Hot:
      return tr("Hot");
    case Grayscale:
      return tr("Grayscale");
    default:
      return tr("Unknown");
  }
}

/**
 * @brief Returns the QColor for a normalized magnitude using the active map.
 */
QColor Widgets::Waterfall::colorAt(double normalized) const
{
  return QColor(sampleColorMap(m_colorMap, normalized));
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or pauses real-time updates.
 */
void Widgets::Waterfall::setRunning(const bool enabled)
{
  m_dashboard.setWaterfallRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Switches to a different color map and triggers a repaint.
 */
void Widgets::Waterfall::setColorMap(const int map)
{
  const int clamped = qBound(0, map, static_cast<int>(ColorMapCount) - 1);
  if (m_colorMap == clamped)
    return;

  m_colorMap = clamped;
  Q_EMIT colorMapChanged();
  update();
}

/**
 * @brief Resizes the time-history buffer (rows of the spectrogram).
 */
void Widgets::Waterfall::setHistorySize(const int size)
{
  const int clamped = qBound(16, size, kMaxHistorySize);
  if (m_historySize == clamped)
    return;

  m_historySize = clamped;
  rebuildHistoryImage();
  Q_EMIT historySizeChanged();
  markAxisDirty();
}

/**
 * @brief Sizes the time history to the dashboard/project Time Range (rows ~= range * fps).
 */
void Widgets::Waterfall::syncHistoryToTimeRange()
{
  static auto& timer = Misc::TimerEvents::instance();
  const double fps   = timer.fps() > 0 ? timer.fps() : 24.0;
  const double range = m_dashboard.plotTimeRange();
  setHistorySize(static_cast<int>(std::lround(range * fps)));
}

/**
 * @brief Sets the lower clip of the dB, color mapping.
 */
void Widgets::Waterfall::setMinDb(const double value)
{
  if (qFuzzyCompare(m_minDb, value))
    return;

  m_minDb = value;
  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Sets the upper clip of the dB, color mapping.
 */
void Widgets::Waterfall::setMaxDb(const double value)
{
  if (qFuzzyCompare(m_maxDb, value))
    return;

  m_maxDb = value;
  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Copies the dataset's frequency markers into runtime state (spec 0019); runs once at
 *        construction so the per-row path never touches the project model.
 */
void Widgets::Waterfall::loadMarkers()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return;

  const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
  m_markers.clear();
  m_markers.reserve(dataset.fftMarkers.size());
  for (const auto& m : dataset.fftMarkers) {
    MarkerData d;
    d.freqLo      = m.frequency;
    d.freqHi      = m.endFrequency;
    d.warningDb   = static_cast<float>(m.warningDb);
    d.alarmDb     = static_cast<float>(m.alarmDb);
    d.peakDb      = kFloorDb;
    d.state       = 0;
    d.customColor = m.color.isEmpty() ? QColor() : QColor::fromString(m.color);
    d.label       = m.label;
    m_markers.push_back(std::move(d));
  }
}

/**
 * @brief Refreshes each marker's peak and normal/warning/alarm state from the freshly
 *        smoothed spectrum row; point markers use a +/- 2 bin neighborhood. Bin math clamps
 *        in the double domain BEFORE the int cast: casting an unrepresentable double is UB.
 */
void Widgets::Waterfall::updateMarkerStates(const int spectrumSize)
{
  constexpr double pointHalfWindow = 2.0;
  Q_ASSERT(spectrumSize > 0);
  Q_ASSERT(m_smoothed.size() >= static_cast<std::size_t>(spectrumSize));

  const double freqStep = static_cast<double>(m_samplingRate) / qMax(1, m_size);
  const double lastBin  = qMax(0, spectrumSize - 1);
  for (auto& m : m_markers) {
    double loF = 0.0;
    double hiF = 0.0;
    if (m.freqHi > m.freqLo) {
      loF = std::floor(m.freqLo / freqStep);
      hiF = std::ceil(m.freqHi / freqStep);
    } else {
      const double center = std::round(m.freqLo / freqStep);
      loF                 = center - pointHalfWindow;
      hiF                 = center + pointHalfWindow;
    }

    const int lo = static_cast<int>(qBound(0.0, loF, lastBin));
    const int hi = static_cast<int>(qBound(static_cast<double>(lo), hiF, lastBin));

    float peak = kFloorDb;
    for (int i = lo; i <= hi; ++i)
      peak = std::max(peak, m_smoothed[i]);

    m.peakDb = peak;
    if (std::isfinite(m.alarmDb) && peak >= m.alarmDb)
      m.state = 2;
    else if (std::isfinite(m.warningDb) && peak >= m.warningDb)
      m.state = 1;
    else
      m.state = 0;
  }
}

/**
 * @brief Resets the time history to the floor color.
 */
void Widgets::Waterfall::clearHistory()
{
  m_topRow     = 0;
  m_writeRow   = 0;
  m_filledOnce = false;
  if (!m_image.isNull())
    m_image.fill(sampleColorMap(m_colorMap, 0.0));

  update();
}

//--------------------------------------------------------------------------------------------------
// FFT plan management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the FFT plan and pre-windowed buffers for a new size.
 */
void Widgets::Waterfall::allocateFftPlan(int size)
{
  releaseFftPlan();
  if (size <= 0)
    return;

  m_size = size;
  m_window.resize(m_size);
  m_samples.resize(m_size);
  m_fftOutput.resize(m_size);
  m_dbCache.assign(m_size / 2, kFloorDb);

  Widgets::fillFftWindow(m_windowType, m_window.data(), static_cast<unsigned int>(m_size));

  m_plan = kiss_fft_alloc(m_size, 0, nullptr, nullptr);
  if (!m_plan)
    qWarning() << "Waterfall FFT plan allocation failed for size:" << m_size;

  rebuildLogColumnTable();
}

/**
 * @brief Derives the log10 frequency domain (first bin to Nyquist, the FFTPlot convention)
 *        and the column-to-bin resample LUT; a degenerate domain falls back to linear.
 */
void Widgets::Waterfall::rebuildLogColumnTable()
{
  const int width       = qMax(1, m_size / 2);
  const double freqStep = static_cast<double>(m_samplingRate) / qMax(1, m_size);
  const double nyquist  = m_samplingRate * 0.5;

  m_logActive = m_logX && width >= 4 && nyquist > freqStep;
  markAxisDirty();
  if (!m_logActive)
    return;

  m_logMin = std::log10(freqStep);
  m_logMax = std::log10(nyquist);

  m_logRow.resize(static_cast<std::size_t>(width));
  m_logColBin.resize(static_cast<std::size_t>(width));
  m_logColFrac.resize(static_cast<std::size_t>(width));

  const double logRange = m_logMax - m_logMin;
  const int lastPair    = width - 2;
  for (int x = 0; x < width; ++x) {
    const double t   = (x + 0.5) / width;
    const double f   = std::exp((m_logMin + t * logRange) * kLn10);
    const double pos = qBound(1.0, f / freqStep, static_cast<double>(width - 1));
    const int idx    = qBound(1, static_cast<int>(pos), lastPair);
    m_logColBin[static_cast<std::size_t>(x)]  = idx;
    m_logColFrac[static_cast<std::size_t>(x)] = static_cast<float>(qBound(0.0, pos - idx, 1.0));
  }
}

/**
 * @brief Returns the row to blit for the current axis mode: the raw linear-bin spectrum, or
 *        the same data resampled onto the log-spaced column grid through the cached LUT.
 */
const float* Widgets::Waterfall::imageRow(const float* dbValues, int bins)
{
  Q_ASSERT(dbValues);
  if (!m_logActive || m_logColBin.size() != static_cast<std::size_t>(bins)
      || m_logRow.size() != static_cast<std::size_t>(bins))
    return dbValues;

  for (int x = 0; x < bins; ++x) {
    const int idx                         = m_logColBin[static_cast<std::size_t>(x)];
    const float fr                        = m_logColFrac[static_cast<std::size_t>(x)];
    const float lo                        = dbValues[idx];
    const float hi                        = dbValues[idx + 1];
    m_logRow[static_cast<std::size_t>(x)] = lo + (hi - lo) * fr;
  }

  return m_logRow.data();
}

/**
 * @brief Maps a frequency in Hz to the axis world domain (Hz linear, log10-Hz log).
 */
double Widgets::Waterfall::worldFromFreq(double hz) const
{
  if (!m_logActive)
    return hz;

  const double freqStep = static_cast<double>(m_samplingRate) / qMax(1, m_size);
  return std::log10(qMax(hz, freqStep));
}

/**
 * @brief Maps an axis world coordinate back to a frequency in Hz.
 */
double Widgets::Waterfall::freqFromWorld(double w) const
{
  if (!m_logActive)
    return w;

  return std::exp(w * kLn10);
}

/**
 * @brief Releases the FFT plan if one is currently allocated.
 */
void Widgets::Waterfall::releaseFftPlan()
{
  if (m_plan) {
    kiss_fft_free(m_plan);
    m_plan = nullptr;
  }
}

//--------------------------------------------------------------------------------------------------
// Image management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Allocates the spectrogram image based on FFT size and history depth.
 */
void Widgets::Waterfall::rebuildHistoryImage()
{
  const int width  = qMax(1, m_size / 2);
  const int height = qMax(1, m_historySize);
  m_image          = QImage(width, height, QImage::Format_RGB32);
  m_image.fill(sampleColorMap(m_colorMap, 0.0));
  m_topRow     = 0;
  m_writeRow   = 0;
  m_filledOnce = false;
}

/**
 * @brief Writes a new spectrum row into the ring: the newest row's physical position is
 *        m_topRow and paint() recomposes the logical order, so a row insert costs O(width)
 *        instead of the old O(width x height) full-image memmove.
 */
void Widgets::Waterfall::writeRow(const float* dbValues, int bins)
{
  if (m_image.isNull() || bins <= 0)
    return;

  const int imageWidth  = m_image.width();
  const int imageHeight = m_image.height();
  m_topRow              = (m_topRow + imageHeight - 1) % imageHeight;

  const float minDb      = static_cast<float>(m_minDb);
  const float maxDb      = static_cast<float>(m_maxDb);
  const float invDbRange = 1.0f / qMax(1e-6f, maxDb - minDb);
  const int writableBins = qMin(bins, imageWidth);
  QRgb* scan             = reinterpret_cast<QRgb*>(m_image.scanLine(m_topRow));

  for (int x = 0; x < writableBins; ++x) {
    const float v  = (dbValues[x] - minDb) * invDbRange;
    const double t = qBound(0.0, static_cast<double>(v), 1.0);
    scan[x]        = sampleColorMap(m_colorMap, t);
  }

  if (writableBins < imageWidth) {
    const QRgb floor = sampleColorMap(m_colorMap, 0.0);
    for (int x = writableBins; x < imageWidth; ++x)
      scan[x] = floor;
  }

  if (!m_filledOnce) {
    if (++m_writeRow >= imageHeight) {
      m_writeRow   = imageHeight;
      m_filledOnce = true;
    }
  }
}

/**
 * @brief Writes a spectrum row at a specific logical row without scrolling (Campbell-mode
 *        entry); the logical row maps through the ring origin so mode toggles stay coherent.
 */
void Widgets::Waterfall::writeRowAt(int row, const float* dbValues, int bins)
{
  if (m_image.isNull() || bins <= 0)
    return;

  const int imageHeight = m_image.height();
  if (row < 0 || row >= imageHeight)
    return;

  const int imageWidth   = m_image.width();
  const int physicalRow  = (row + m_topRow) % imageHeight;
  const float minDb      = static_cast<float>(m_minDb);
  const float maxDb      = static_cast<float>(m_maxDb);
  const float invDbRange = 1.0f / qMax(1e-6f, maxDb - minDb);
  const int writableBins = qMin(bins, imageWidth);
  QRgb* scan             = reinterpret_cast<QRgb*>(m_image.scanLine(physicalRow));

  for (int x = 0; x < writableBins; ++x) {
    const float v  = (dbValues[x] - minDb) * invDbRange;
    const double t = qBound(0.0, static_cast<double>(v), 1.0);
    scan[x]        = sampleColorMap(m_colorMap, t);
  }

  if (writableBins < imageWidth) {
    const QRgb floor = sampleColorMap(m_colorMap, 0.0);
    for (int x = writableBins; x < imageWidth; ++x)
      scan[x] = floor;
  }
}

//--------------------------------------------------------------------------------------------------
// Hotpath
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts the FFT output to smoothed display dB per bin (shared 1/N^2 power norm,
 *        3-bin boxcar) into m_smoothed.
 */
void Widgets::Waterfall::computeSmoothedRow(const int spectrumSize)
{
  Q_ASSERT(spectrumSize > 0);
  Q_ASSERT(m_fftOutput.size() >= static_cast<std::size_t>(spectrumSize));

  const float normFactor = static_cast<float>(m_size) * static_cast<float>(m_size);
  const float invNorm    = 1.0f / normFactor;
  for (int i = 0; i < spectrumSize; ++i) {
    const float re    = m_fftOutput[i].r;
    const float im    = m_fftOutput[i].i;
    const float power = std::max((re * re + im * im) * invNorm, kEpsSquared);
    m_dbCache[i]      = std::max(10.0f * std::log10(power), kFloorDb);
  }

  if (m_smoothed.size() < static_cast<size_t>(spectrumSize))
    m_smoothed.resize(spectrumSize);

  static constexpr float kInvSmoothingTaps[] = {
    0.0f,
    1.0f / 1.0f,
    1.0f / 2.0f,
    1.0f / 3.0f,
    1.0f / 4.0f,
    1.0f / 5.0f,
  };
  for (int i = 0; i < spectrumSize; ++i) {
    const int lo = std::max(0, i - kHalfSmoothWindow);
    const int hi = std::min(spectrumSize - 1, i + kHalfSmoothWindow);
    float sum    = 0.0f;
    for (int k = lo; k <= hi; ++k)
      sum += m_dbCache[k];

    const int taps = hi - lo + 1;
    m_smoothed[i]  = sum * kInvSmoothingTaps[taps];
  }
}

/**
 * @brief Pulls the latest time-domain samples, runs FFT, pushes a new row. The plan and history
 *        image are sized from the ring's capacity, never its fill level, so a filling ring cannot
 *        thrash the plan or wipe the spectrogram; the unfilled tail is zero-padded instead.
 */
void Widgets::Waterfall::updateData()
{
  if (!isEnabled() || !isVisible())
    return;

  if (!m_dashboard.waterfallRunning(m_index))
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return;

  const auto& data  = m_dashboard.waterfallData(m_index);
  const int newSize = static_cast<int>(data.capacity());
  if (newSize <= 0)
    return;

  if (newSize != m_size) {
    allocateFftPlan(newSize);
    rebuildHistoryImage();
    Q_EMIT historySizeChanged();
  }

  if (!m_plan)
    return;

  const int avail = static_cast<int>(std::min(data.size(), static_cast<std::size_t>(m_size)));

  const double* in       = data.raw();
  const std::size_t mask = data.storageMask();
  const double offset    = m_scaleIsValid ? -m_center : 0.0;
  const double scale     = m_scaleIsValid ? (1.0 / m_halfRange) : 1.0;

  static_assert(sizeof(kiss_fft_cpx) == 2 * sizeof(float));

  if (avail > 0)
    DSP::simdWindowedComplexFill(in,
                                 data.frontIndex(),
                                 mask,
                                 static_cast<std::size_t>(avail),
                                 offset,
                                 scale,
                                 m_window.data(),
                                 &m_samples[0].r);

  for (int i = avail; i < m_size; ++i) {
    m_samples[i].r = 0.0f;
    m_samples[i].i = 0.0f;
  }

  kiss_fft(m_plan, m_samples.data(), m_fftOutput.data());

  const int spectrumSize = m_size / 2;
  computeSmoothedRow(spectrumSize);

  if (!m_markers.empty() && spectrumSize > 0)
    updateMarkerStates(spectrumSize);

  const float* row_data = imageRow(m_smoothed.data(), spectrumSize);

  if (m_campbellMode && m_image.height() > 0) {
    const auto& datasets = m_dashboard.datasets();
    const auto it        = datasets.find(m_yDatasetUniqueId);
    if (it != datasets.end() && it->isNumeric) {
      const double v     = it->numericValue;
      const double range = m_yMax - m_yMin;
      if (range > 0.0) {
        const double invRange = 1.0 / range;
        const double t        = qBound(0.0, (v - m_yMin) * invRange, 1.0);
        const int row =
          qBound(0, static_cast<int>((1.0 - t) * (m_image.height() - 1)), m_image.height() - 1);
        writeRowAt(row, row_data, spectrumSize);
      }
    }
  } else {
    writeRow(row_data, spectrumSize);
  }

  update();
}

//--------------------------------------------------------------------------------------------------
// Painting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the spectrogram and composites it with the cached axis layer.
 */
void Widgets::Waterfall::paint(QPainter* painter)
{
  if (!painter)
    return;

  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  if (m_axisDirty)
    renderAxisLayer();

  const QRectF outerRect(0, 0, width(), height());
  painter->fillRect(outerRect, m_outerBg);

  const QRectF& plotRect = m_cachedPlotRect;
  if (plotRect.isEmpty())
    return;

  painter->fillRect(plotRect, m_innerBg);

  if (!m_image.isNull()) {
    painter->save();
    painter->setClipRect(plotRect);
    drawHistoryImage(painter, plotRect);
    painter->restore();
  }

  if (!m_axisLayer.isNull())
    painter->drawImage(outerRect, m_axisLayer, QRectF(m_axisLayer.rect()));

  if (m_markersVisible && !m_markers.empty())
    drawMarkers(painter, plotRect);

  if (m_cursorEnabled && m_cursorHovering)
    drawCursor(painter, plotRect);
}

//--------------------------------------------------------------------------------------------------
// Axis layer cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-renders the axis overlay into m_axisLayer and clears the dirty flag.
 */
void Widgets::Waterfall::renderAxisLayer()
{
  m_axisDirty = false;

  const QSize itemSize(qMax(1, qCeil(width())), qMax(1, qCeil(height())));
  const qreal dpr     = (window() ? window()->devicePixelRatio() : 1.0);
  const QSize bufSize = itemSize * dpr;
  if (bufSize.isEmpty())
    return;

  if (m_axisLayer.size() != bufSize) {
    m_axisLayer = QImage(bufSize, QImage::Format_ARGB32_Premultiplied);
    m_axisLayer.setDevicePixelRatio(dpr);
  }
  m_axisLayer.fill(Qt::transparent);

  QPainter painter(&m_axisLayer);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);

  static auto& fonts = Misc::CommonFonts::instance();
  painter.setFont(fonts.widgetFont(0.83, false));

  const QFontMetrics fm(painter.font());
  m_cachedPlotRect = computePlotRect(fm);
  if (m_cachedPlotRect.isEmpty())
    return;

  painter.setPen(QPen(m_borderColor, 1));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(m_cachedPlotRect);

  if (m_axisVisible && width() >= kMinAxisWidth && height() >= kMinAxisHeight) {
    drawXAxis(&painter, m_cachedPlotRect);
    drawYAxis(&painter, m_cachedPlotRect);
  }
}

/**
 * @brief Marks the axis overlay as needing a re-render and schedules a repaint.
 */
void Widgets::Waterfall::markAxisDirty()
{
  m_axisDirty = true;
  update();
}

//--------------------------------------------------------------------------------------------------
// Layout helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the inner plot rectangle after reserving axis-label margins.
 */
QRectF Widgets::Waterfall::computePlotRect(const QFontMetrics& fm) const
{
  if (!m_axisVisible || width() < kMinAxisWidth || height() < kMinAxisHeight)
    return QRectF(0.5, 0.5, qMax(0.0, width() - 1), qMax(0.0, height() - 1));

  static auto& fonts = Misc::CommonFonts::instance();
  const QFontMetrics titleFm(fonts.widgetFont(0.91, true));

  const int yTickWidth =
    fm.horizontalAdvance(QStringLiteral("00.00")) + kAxisTickPx + kAxisLabelPad;
  const int yTitleWidth = titleFm.height() + 2;

  const int leftMargin   = yTitleWidth + yTickWidth;
  const int rightMargin  = kAxisLabelPad;
  const int topMargin    = kAxisLabelPad;
  const int bottomMargin = fm.height() + kAxisTickPx + kAxisLabelPad * 2;

  return QRectF(leftMargin + 0.5,
                topMargin + 0.5,
                qMax(0.0, width() - leftMargin - rightMargin - 1),
                qMax(0.0, height() - topMargin - bottomMargin - 1));
}

/**
 * @brief Draws the ring-buffered history: the visible logical rows map to at most two
 *        physical slices (wrap at the ring origin), each drawn into its share of plotRect.
 */
void Widgets::Waterfall::drawHistoryImage(QPainter* painter, const QRectF& plotRect) const
{
  Q_ASSERT(painter);
  Q_ASSERT(!m_image.isNull());

  const QRectF src = computeSourceRect();
  if (src.isEmpty())
    return;

  const double h = m_image.height();
  if (m_topRow == 0) {
    painter->drawImage(plotRect, m_image, src);
    return;
  }

  const double wrapY = h - m_topRow;
  if (src.bottom() <= wrapY) {
    painter->drawImage(plotRect, m_image, src.translated(0.0, m_topRow));
    return;
  }

  if (src.top() >= wrapY) {
    painter->drawImage(plotRect, m_image, src.translated(0.0, m_topRow - h));
    return;
  }

  const double fracTop = (wrapY - src.top()) / src.height();
  const double splitY  = plotRect.top() + plotRect.height() * fracTop;
  const QRectF dstTop(plotRect.left(), plotRect.top(), plotRect.width(), splitY - plotRect.top());
  const QRectF dstBottom(plotRect.left(), splitY, plotRect.width(), plotRect.bottom() - splitY);
  const QRectF srcTop(src.left(), src.top() + m_topRow, src.width(), wrapY - src.top());
  const QRectF srcBottom(src.left(), 0.0, src.width(), src.bottom() - wrapY);
  painter->drawImage(dstTop, m_image, srcTop);
  painter->drawImage(dstBottom, m_image, srcBottom);
}

/**
 * @brief Returns the visible source rectangle inside m_image, given zoom/pan.
 */
QRectF Widgets::Waterfall::computeSourceRect() const
{
  if (m_image.isNull())
    return QRectF();

  const double iw   = m_image.width();
  const double ih   = m_image.height();
  const double srcW = iw / m_xZoom;
  const double srcH = ih / m_yZoom;

  const double maxPanX = qMax(0.0, (iw - srcW) * 0.5);
  const double maxPanY = qMax(0.0, (ih - srcH) * 0.5);
  const double cx      = iw * 0.5 + qBound(-maxPanX, m_xPan * iw, maxPanX);
  const double cy      = ih * 0.5 + qBound(-maxPanY, m_yPan * ih, maxPanY);

  return QRectF(cx - srcW * 0.5, cy - srcH * 0.5, srcW, srcH);
}

//--------------------------------------------------------------------------------------------------
// Axis rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the visible frequency window [wMin, wMax] in axis WORLD units (Hz linear,
 *        log10-Hz log) from the zoom/pan view state; single source of truth for the axis,
 *        hover cursor, and marker Hz-to-pixel mapping.
 */
void Widgets::Waterfall::visibleFreqWindow(double& wMin, double& wMax) const
{
  const double w0     = m_logActive ? m_logMin : 0.0;
  const double w1     = m_logActive ? m_logMax : m_samplingRate * 0.5;
  const double range  = w1 - w0;
  const double srcW   = range / m_xZoom;
  const double maxPan = qMax(0.0, (range - srcW) * 0.5);
  const double center = (w0 + w1) * 0.5 + qBound(-maxPan, m_xPan * range, maxPan);
  wMin                = center - srcW * 0.5;
  wMax                = center + srcW * 0.5;
}

/**
 * @brief Collects the tick frequencies (Hz) for the visible window: the {1,2,5} ladder on
 *        the linear axis, or per-decade {1,2,5} candidates thinned to decades on the log
 *        axis when they would crowd.
 */
std::vector<double> Widgets::Waterfall::collectFreqTicks(double wMin, double wMax) const
{
  std::vector<double> out;
  if (!m_logActive) {
    const AxisTicks ticks = computeFreqTicks(wMax - wMin, kAxisTickCount);
    const double step     = ticks.step;
    const double first    = std::ceil(wMin / step - 1e-9) * step;
    for (double v = first; v <= wMax + 1e-6; v += step)
      out.push_back(v);

    return out;
  }

  constexpr double mants[] = {1.0, 2.0, 5.0};
  const int dLo            = static_cast<int>(std::floor(wMin)) - 1;
  const int dHi            = static_cast<int>(std::ceil(wMax)) + 1;
  for (int d = dLo; d <= dHi; ++d) {
    for (const double m : mants) {
      const double w = d + std::log10(m);
      if (w >= wMin - 1e-9 && w <= wMax + 1e-9)
        out.push_back(m * fastPow10(d));
    }
  }

  if (static_cast<int>(out.size()) > kAxisTickCount + 2) {
    std::vector<double> decades;
    decades.reserve(out.size());
    for (const double v : out) {
      const double lg = std::log10(v);
      if (std::abs(lg - std::round(lg)) < 1e-9)
        decades.push_back(v);
    }

    if (!decades.empty())
      out = std::move(decades);
  }

  if (static_cast<int>(out.size()) < 2) {
    out.clear();
    const double fLo      = freqFromWorld(wMin);
    const double fHi      = freqFromWorld(wMax);
    const AxisTicks ticks = computeFreqTicks(fHi - fLo, kAxisTickCount);
    const double first    = std::ceil(fLo / ticks.step - 1e-9) * ticks.step;
    for (double v = first; v <= fHi + 1e-6; v += ticks.step)
      out.push_back(v);
  }

  return out;
}

/**
 * @brief Draws the frequency axis (X) -- grid, tick marks, labels -- in world space, so the
 *        same loop renders both the linear and the log-frequency layout.
 */
void Widgets::Waterfall::drawXAxis(QPainter* painter, const QRectF& plotRect) const
{
  if (m_samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double wRange = wMax - wMin;
  if (wRange <= 0.0)
    return;

  const QFontMetrics fm(painter->font());
  const double tickTopY  = plotRect.bottom();
  const double tickBotY  = plotRect.bottom() + kAxisTickPx;
  const double labelY    = tickBotY + kAxisLabelPad;
  const double invWRange = 1.0 / wRange;
  const auto tickFreqs   = collectFreqTicks(wMin, wMax);

  for (const double v : tickFreqs) {
    const double t = (worldFromFreq(v) - wMin) * invWRange;
    if (t < 0.0 || t > 1.0)
      continue;

    const double x = plotRect.left() + t * plotRect.width();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(x, tickTopY), QPointF(x, tickBotY));

    const QString label  = formatFreqTick(v);
    const int labelWidth = fm.horizontalAdvance(label);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(x - labelWidth * 0.5, labelY + fm.ascent()), label);
  }
}

/**
 * @brief Draws the Y axis (time or Campbell-mode dataset value).
 */
void Widgets::Waterfall::drawYAxis(QPainter* painter, const QRectF& plotRect) const
{
  double dataMin = 0.0;
  double dataMax = 0.0;
  if (m_campbellMode) {
    dataMin = m_yMin;
    dataMax = m_yMax;
  } else {
    static auto& timer = Misc::TimerEvents::instance();
    const double fps   = timer.fps() > 0 ? timer.fps() : 24.0;
    dataMin            = 0.0;
    dataMax            = m_historySize / fps;
  }
  const double dataRange = dataMax - dataMin;
  if (dataRange <= 0.0)
    return;

  const double srcH    = dataRange / m_yZoom;
  const double maxPan  = qMax(0.0, (dataRange - srcH) * 0.5);
  const double centerD = (dataMin + dataMax) * 0.5 + qBound(-maxPan, m_yPan * dataRange, maxPan);
  const double yMin    = centerD - srcH * 0.5;
  const double yMax    = centerD + srcH * 0.5;

  const AxisTicks ticks = computeTimeTicks(srcH, kAxisTickCount);

  const QFontMetrics fm(painter->font());
  const double tickRightX = plotRect.left();
  const double tickLeftX  = plotRect.left() - kAxisTickPx;
  const double labelRight = tickLeftX - kAxisLabelPad;

  const double step      = ticks.step;
  const double invStep   = 1.0 / step;
  const double first     = std::ceil(yMin * invStep - 1e-9) * step;
  const double yRange    = yMax - yMin;
  const double invYRange = yRange > 0.0 ? 1.0 / yRange : 0.0;

  for (double v = first; v <= yMax + 1e-6; v += step) {
    const double t = (v - yMin) * invYRange;
    if (t < 0.0 || t > 1.0)
      continue;

    const double y = m_campbellMode ? plotRect.bottom() - t * plotRect.height()
                                    : plotRect.top() + t * plotRect.height();

    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));

    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(tickLeftX, y), QPointF(tickRightX, y));

    const QString label  = formatTimeTick(v, step);
    const int labelWidth = fm.horizontalAdvance(label);
    const double textCy  = qBound(plotRect.top() + fm.ascent() * 0.5,
                                 y + fm.ascent() * 0.5,
                                 plotRect.bottom() + fm.ascent() * 0.5);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(labelRight - labelWidth, textCy), label);
  }

  static auto& fonts = Misc::CommonFonts::instance();
  const QFont titleFont(fonts.widgetFont(0.91, true));
  const QFontMetrics titleFm(titleFont);

  const QString title = m_campbellMode ? m_yAxisTitle : QObject::tr("Time (s)");
  const double titleX =
    labelRight - fm.horizontalAdvance(QStringLiteral("00.00")) - 2 - titleFm.descent();
  const double titleY = plotRect.center().y();

  painter->save();
  painter->setFont(titleFont);
  painter->translate(titleX, titleY);
  painter->rotate(-90.0);
  painter->setPen(m_textColor);
  painter->drawText(QPointF(-titleFm.horizontalAdvance(title) * 0.5, 0), title);
  painter->restore();
}

/**
 * @brief Draws the frequency markers over the spectrogram: translucent band regions, point
 *        lines, and label chips with the live peak readout (spec 0019).
 */
void Widgets::Waterfall::drawMarkers(QPainter* painter, const QRectF& plotRect) const
{
  Q_ASSERT(painter);
  Q_ASSERT(!m_markers.empty());
  m_chipHitRects.clear();
  if (plotRect.isEmpty() || m_samplingRate <= 0)
    return;

  double wMin = 0.0;
  double wMax = 0.0;
  visibleFreqWindow(wMin, wMax);
  const double range = wMax - wMin;
  if (range <= 0.0)
    return;

  painter->save();
  painter->setClipRect(plotRect);
  painter->setFont(m_commonFonts.widgetFont(0.8, false));

  const QFontMetrics fm(painter->font());
  double rowEnd[kMarkerChipRows] = {-1e18, -1e18, -1e18};
  const double invRange          = 1.0 / range;
  for (std::size_t i = 0; i < m_markers.size(); ++i) {
    const auto& m      = m_markers[i];
    const int idx      = static_cast<int>(i);
    const bool band    = m.freqHi > m.freqLo;
    const bool spotlit = m_selectedMarker == idx;
    const bool dimmed  = m_selectedMarker >= 0 && !spotlit;
    const double xLo =
      plotRect.left() + (worldFromFreq(m.freqLo) - wMin) * invRange * plotRect.width();
    const double xHi =
      band ? plotRect.left() + (worldFromFreq(m.freqHi) - wMin) * invRange * plotRect.width() : xLo;
    if (xHi < plotRect.left() || xLo > plotRect.right())
      continue;

    QColor base = m.customColor.isValid() ? m.customColor : m_accentColor;
    if (m.state == 2)
      base = m_alarmColor;
    else if (m.state == 1)
      base = m_warningColor;

    painter->setOpacity(dimmed ? 0.22 : 1.0);

    if (band) {
      QColor fill = base;
      fill.setAlpha(spotlit ? 78 : (m.state > 0 ? 64 : 40));
      painter->fillRect(QRectF(xLo, plotRect.top(), xHi - xLo, plotRect.height()), fill);

      QColor edge = base;
      edge.setAlpha(spotlit ? 200 : 120);
      const double ew = spotlit ? 2.0 : 1.0;
      painter->fillRect(QRectF(xLo, plotRect.top(), ew, plotRect.height()), edge);
      painter->fillRect(QRectF(xHi - ew, plotRect.top(), ew, plotRect.height()), edge);
    } else {
      QColor line = base;
      line.setAlpha(spotlit ? 255 : (m.state > 0 ? 240 : 170));
      const double lw = spotlit ? 3.0 : 2.0;
      painter->fillRect(QRectF(xLo - lw * 0.5, plotRect.top(), lw, plotRect.height()), line);
    }

    const QString name = m.label.isEmpty() ? formatFreqTick(m.freqLo) : m.label;
    const QString text = QObject::tr("%1  %2 dB").arg(name, QString::number(m.peakDb, 'f', 1));
    drawMarkerChip(painter, plotRect, fm, rowEnd, idx, spotlit, (xLo + xHi) * 0.5, text, base);
  }

  painter->setOpacity(1.0);
  painter->restore();
}

/**
 * @brief Draws one marker's label chip near the top of the plot, dropping down a row when it
 *        would overlap a chip already placed on the current row; captures the chip rect for
 *        the click-to-spotlight hit test.
 */
void Widgets::Waterfall::drawMarkerChip(QPainter* painter,
                                        const QRectF& plotRect,
                                        const QFontMetrics& fm,
                                        double* rowEnd,
                                        const int markerIndex,
                                        const bool spotlit,
                                        double cx,
                                        const QString& text,
                                        const QColor& color) const
{
  Q_ASSERT(painter);
  Q_ASSERT(rowEnd);

  const double w = fm.horizontalAdvance(text) + 8;
  const double h = fm.height() + 4;
  const double x =
    qBound(plotRect.left() + 2, cx - w * 0.5, qMax(plotRect.left() + 2, plotRect.right() - w - 2));

  int row = 0;
  while (row < kMarkerChipRows - 1 && rowEnd[row] > x - 4)
    ++row;

  rowEnd[row]    = qMax(rowEnd[row], x + w);
  const double y = plotRect.top() + 4 + row * (h + 2);
  const QRectF chipRect(x, y, w, h);
  m_chipHitRects.emplace_back(markerIndex, chipRect);

  QColor bg = color;
  bg.setAlpha(spotlit ? 255 : 230);
  painter->setPen(Qt::NoPen);
  painter->setBrush(bg);
  painter->drawRoundedRect(chipRect, 3, 3);

  if (spotlit) {
    painter->setPen(QPen(m_textColor, 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(chipRect, 3, 3);
  }

  painter->setPen(m_innerBg);
  painter->drawText(QPointF(x + 4, y + 2 + fm.ascent()), text);
}

/**
 * @brief Returns the marker index whose chip contains @a pos, or -1 (topmost chip wins).
 */
int Widgets::Waterfall::markerChipAt(const QPointF& pos) const
{
  for (auto it = m_chipHitRects.rbegin(); it != m_chipHitRects.rend(); ++it)
    if (it->second.contains(pos))
      return it->first;

  return -1;
}

/**
 * @brief Draws the live hover cursor -- vertical & horizontal crosshair lines clipped to the plot
 * rect, plus a small tooltip with the freq + time readings under the pointer (zoom/pan-aware).
 */
void Widgets::Waterfall::drawCursor(QPainter* painter, const QRectF& plotRect) const
{
  if (plotRect.isEmpty() || !plotRect.contains(m_cursorPos))
    return;

  static auto& fonts = Misc::CommonFonts::instance();
  painter->setFont(fonts.widgetFont(0.83, false));
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  const double cx = qBound(plotRect.left(), m_cursorPos.x(), plotRect.right());
  const double cy = qBound(plotRect.top(), m_cursorPos.y(), plotRect.bottom());

  painter->setPen(QPen(QColor(255, 255, 255, 178), 1));
  painter->drawLine(QPointF(cx, plotRect.top()), QPointF(cx, plotRect.bottom()));
  painter->drawLine(QPointF(plotRect.left(), cy), QPointF(plotRect.right(), cy));

  double freqHz = 0.0;
  double yVal   = 0.0;
  cursorReadoutValues(plotRect, cx, cy, freqHz, yVal);

  auto fmtFreq = [](double hz) -> QString {
    const double abs = std::fabs(hz);
    if (abs >= 1e6)
      return QString::number(hz / 1e6, 'f', 2) + QStringLiteral(" MHz");

    if (abs >= 1e3)
      return QString::number(hz / 1e3, 'f', 2) + QStringLiteral(" kHz");

    return QString::number(hz, 'f', 1) + QStringLiteral(" Hz");
  };
  auto fmtTime = [](double s) -> QString {
    if (s < 1.0)
      return QString::number(std::round(s * 1000.0), 'f', 0) + QStringLiteral(" ms");

    if (s >= 100.0)
      return QString::number(s, 'f', 0) + QStringLiteral(" s");

    return QString::number(s, 'f', 2) + QStringLiteral(" s");
  };

  const QString freqText = QObject::tr("Freq: %1").arg(fmtFreq(freqHz));
  const QString timeText =
    m_campbellMode ? QStringLiteral("%1: %2").arg(m_yAxisTitle, QString::number(yVal, 'f', 2))
                   : QObject::tr("Time: −%1").arg(fmtTime(yVal));

  drawCursorTooltip(painter, plotRect, cx, cy, freqText, timeText);
}

/**
 * @brief Maps cursor pixel position to the visible Hz axis and the Y axis value.
 */
void Widgets::Waterfall::cursorReadoutValues(
  const QRectF& plotRect, double cx, double cy, double& freqHz, double& yVal) const
{
  double wMinX = 0.0;
  double wMaxX = 0.0;
  visibleFreqWindow(wMinX, wMaxX);
  freqHz = freqFromWorld(wMinX + (cx - plotRect.left()) / plotRect.width() * (wMaxX - wMinX));

  double yMinAxis = 0.0;
  double yMaxAxis = 1.0;
  if (m_campbellMode) {
    yMinAxis = m_yMin;
    yMaxAxis = m_yMax;
  } else {
    static auto& timer = Misc::TimerEvents::instance();
    const double fps   = timer.fps() > 0 ? timer.fps() : 24.0;
    yMinAxis           = 0.0;
    yMaxAxis           = m_historySize / fps;
  }
  const double yRange  = yMaxAxis - yMinAxis;
  const double srcWY   = yRange / m_yZoom;
  const double maxPanY = qMax(0.0, (yRange - srcWY) * 0.5);
  const double centerY = (yMinAxis + yMaxAxis) * 0.5 + qBound(-maxPanY, m_yPan * yRange, maxPanY);
  const double yMinV   = centerY - srcWY * 0.5;
  const double yMaxV   = centerY + srcWY * 0.5;

  const double tY = (cy - plotRect.top()) / plotRect.height();
  yVal = m_campbellMode ? (yMaxV - tY * (yMaxV - yMinV)) : (yMinV + tY * (yMaxV - yMinV));
}

/**
 * @brief Renders the two-line crosshair tooltip box, flipping sides as needed.
 */
void Widgets::Waterfall::drawCursorTooltip(QPainter* painter,
                                           const QRectF& plotRect,
                                           double cx,
                                           double cy,
                                           const QString& freqText,
                                           const QString& timeText) const
{
  const QFontMetrics fm(painter->font());
  const int padX = 8;
  const int padY = 5;
  const int gap  = 2;
  const int w = std::max(fm.horizontalAdvance(freqText), fm.horizontalAdvance(timeText)) + padX * 2;
  const int h = fm.height() * 2 + gap + padY * 2;

  double tx = cx + 12;
  double ty = cy + 12;
  if (tx + w > plotRect.right())
    tx = cx - 12 - w;

  if (ty + h > plotRect.bottom())
    ty = cy - 12 - h;

  tx = qBound(plotRect.left() + 2, tx, plotRect.right() - w - 2);
  ty = qBound(plotRect.top() + 2, ty, plotRect.bottom() - h - 2);

  const QRectF tipRect(tx, ty, w, h);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 184));
  painter->drawRoundedRect(tipRect, 3, 3);

  painter->setPen(QColor(Qt::white));
  const double textBaseline = ty + padY + fm.ascent();
  painter->drawText(QPointF(tx + padX, textBaseline), freqText);
  painter->drawText(QPointF(tx + padX, textBaseline + fm.height() + gap), timeText);
}

//--------------------------------------------------------------------------------------------------
// Tick generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a {1,2,5}*10^n step for a given range and target tick count.
 */
Widgets::Waterfall::AxisTicks Widgets::Waterfall::computeFreqTicks(double maxFreq, int targetCount)
{
  AxisTicks out{{}, 1.0, maxFreq};
  if (!std::isfinite(maxFreq) || maxFreq <= 0.0)
    return out;

  const double target  = std::max(2, targetCount);
  const double raw     = maxFreq / target;
  const double base    = fastPow10(std::floor(std::log10(raw)));
  const double cands[] = {1.0, 2.0, 5.0, 10.0};

  double step = base;
  for (double c : cands) {
    if (raw <= c * base) {
      step = c * base;
      break;
    }
  }

  out.step       = step;
  out.displayMax = std::ceil(maxFreq / step) * step;
  for (double v = 0.0; v <= out.displayMax + 1e-6; v += step)
    out.values.push_back(v);

  return out;
}

/**
 * @brief Same algorithm as computeFreqTicks but for the seconds axis.
 */
Widgets::Waterfall::AxisTicks Widgets::Waterfall::computeTimeTicks(double maxSeconds,
                                                                   int targetCount)
{
  return computeFreqTicks(maxSeconds, targetCount);
}

//--------------------------------------------------------------------------------------------------
// Format helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a frequency value as Hz / kHz / MHz with one decimal at most.
 */
QString Widgets::Waterfall::formatFreqTick(double hz)
{
  const double abs = std::fabs(hz);
  if (abs >= 1e6)
    return QString::number(hz / 1e6, 'g', 3) + QStringLiteral(" MHz");

  if (abs >= 1e3)
    return QString::number(hz / 1e3, 'g', 3) + QStringLiteral(" kHz");

  return QString::number(hz, 'g', 3) + QStringLiteral(" Hz");
}

/**
 * @brief Formats a time value -- integer seconds when step >= 1, decimals otherwise.
 */
QString Widgets::Waterfall::formatTimeTick(double seconds, double step)
{
  if (step >= 1.0)
    return QString::number(std::round(seconds), 'f', 0);

  const int decimals = std::max(0, -static_cast<int>(std::floor(std::log10(step))));
  return QString::number(seconds, 'f', decimals);
}

//--------------------------------------------------------------------------------------------------
// View state (zoom / pan / axis visibility)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the axes are user-enabled.
 */
bool Widgets::Waterfall::axisVisible() const noexcept
{
  return m_axisVisible;
}

/**
 * @brief Returns the current X (frequency) zoom factor -- 1.0 = full Nyquist sweep.
 */
double Widgets::Waterfall::xZoom() const noexcept
{
  return m_xZoom;
}

/**
 * @brief Returns the current Y (time) zoom factor -- 1.0 = full history visible.
 */
double Widgets::Waterfall::yZoom() const noexcept
{
  return m_yZoom;
}

/**
 * @brief Returns the X pan offset in [-0.5, 0.5] of the data range.
 */
double Widgets::Waterfall::xPan() const noexcept
{
  return m_xPan;
}

/**
 * @brief Returns the Y pan offset in [-0.5, 0.5] of the history range.
 */
double Widgets::Waterfall::yPan() const noexcept
{
  return m_yPan;
}

/**
 * @brief Convenience flag -- true when zoom is unity and pan is zero on both axes.
 */
bool Widgets::Waterfall::atDefaultView() const noexcept
{
  return qFuzzyCompare(m_xZoom, 1.0) && qFuzzyCompare(m_yZoom, 1.0) && qFuzzyIsNull(m_xPan)
      && qFuzzyIsNull(m_yPan);
}

/**
 * @brief Toggles the axes (ticks, grid, labels) on/off. Triggers a repaint.
 */
void Widgets::Waterfall::setAxisVisible(const bool enabled)
{
  if (m_axisVisible == enabled)
    return;

  m_axisVisible = enabled;
  Q_EMIT axisVisibleChanged();
  markAxisDirty();
}

/**
 * @brief Returns whether the freq/time hover cursor is enabled.
 */
bool Widgets::Waterfall::cursorEnabled() const noexcept
{
  return m_cursorEnabled;
}

/**
 * @brief Enables or disables the freq/time hover cursor overlay.
 */
void Widgets::Waterfall::setCursorEnabled(const bool enabled)
{
  if (m_cursorEnabled == enabled)
    return;

  m_cursorEnabled = enabled;
  Q_EMIT cursorEnabledChanged();
  update();
}

/**
 * @brief Returns whether the frequency-marker overlay is visible.
 */
bool Widgets::Waterfall::markersVisible() const noexcept
{
  return m_markersVisible;
}

/**
 * @brief Toggles the frequency-marker overlay on/off; hiding also clears the spotlight and
 *        the stale chip hit rects.
 */
void Widgets::Waterfall::setMarkersVisible(const bool enabled)
{
  if (m_markersVisible == enabled)
    return;

  m_markersVisible = enabled;
  if (!enabled) {
    m_selectedMarker = -1;
    m_chipHitRects.clear();
  }

  Q_EMIT markersVisibleChanged();
  update();
}

/**
 * @brief Returns whether the side colorbar legend is visible.
 */
bool Widgets::Waterfall::colorbarVisible() const noexcept
{
  return m_colorbarVisible;
}

/**
 * @brief Toggles the colorbar legend on/off.
 */
void Widgets::Waterfall::setColorbarVisible(const bool enabled)
{
  if (m_colorbarVisible == enabled)
    return;

  m_colorbarVisible = enabled;
  Q_EMIT colorbarVisibleChanged();
}

/**
 * @brief Returns whether WAV recording of the widget's time-domain input is active (Pro).
 */
bool Widgets::Waterfall::audioRecordingEnabled() const noexcept
{
  return m_audioRecordingEnabled;
}

/**
 * @brief Resolves the shared WAV output folder live from the current dataset title, so a dataset
 *        rename after construction still reveals the correct recordings folder.
 */
QString Widgets::Waterfall::recordingsFolder() const
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return QString();

  return m_audioExport.audioPath(GET_DATASET(SerialStudio::DashboardWaterfall, m_index).title,
                                 m_dashboard.title());
}

/**
 * @brief Toggles WAV recording of the input signal; enabling is refused without a Pro license,
 *        disabling always disarms the Dashboard tap before finalising the session.
 */
void Widgets::Waterfall::setAudioRecordingEnabled(const bool enabled)
{
  if (m_audioRecordingEnabled == enabled)
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return;

  if (enabled) {
    if (!SerialStudio::activated())
      return;

    const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
    AudioSessionConfig cfg;
    cfg.sampleRate   = m_samplingRate;
    cfg.useScale     = m_scaleIsValid;
    cfg.center       = m_center;
    cfg.halfRange    = m_halfRange;
    cfg.datasetTitle = dataset.title;
    cfg.projectTitle = m_dashboard.title();

    m_audioExport.openSession(SerialStudio::DashboardWaterfall, m_index, cfg);
    m_dashboard.setWaterfallAudioTap(
      m_index, true, Widgets::AudioExport::sessionKey(SerialStudio::DashboardWaterfall, m_index));
  } else {
    m_dashboard.setWaterfallAudioTap(m_index, false, 0);
    m_audioExport.closeSession(SerialStudio::DashboardWaterfall, m_index);
  }

  m_audioRecordingEnabled = enabled;
  Q_EMIT audioRecordingEnabledChanged();
}

/**
 * @brief Multiplies both axis zooms by factor, anchored at (anchorX,anchorY) in [0,1] coordinates.
 */
void Widgets::Waterfall::zoomBy(double factor, double anchorX, double anchorY)
{
  if (!std::isfinite(factor) || factor <= 0.0)
    return;

  const double newX = qBound(1.0, m_xZoom * factor, kMaxZoom);
  const double newY = qBound(1.0, m_yZoom * factor, kMaxZoom);

  const double ax = qBound(0.0, anchorX, 1.0) - 0.5;
  const double ay = qBound(0.0, anchorY, 1.0) - 0.5;
  m_xPan          = m_xPan + ax * (1.0 / m_xZoom - 1.0 / newX);
  m_yPan          = m_yPan + ay * (1.0 / m_yZoom - 1.0 / newY);
  m_xZoom         = newX;
  m_yZoom         = newY;

  const double maxPanX = (1.0 - 1.0 / m_xZoom) * 0.5;
  const double maxPanY = (1.0 - 1.0 / m_yZoom) * 0.5;
  m_xPan               = qBound(-maxPanX, m_xPan, maxPanX);
  m_yPan               = qBound(-maxPanY, m_yPan, maxPanY);

  Q_EMIT viewChanged();
  markAxisDirty();
}

/**
 * @brief Translates the view by (normDx, normDy) -- both in normalized item-rect
 *        coordinates (e.g. 0.1 = 10% of the visible plot width/height).
 */
void Widgets::Waterfall::panBy(double normDx, double normDy)
{
  if (!std::isfinite(normDx) || !std::isfinite(normDy))
    return;

  m_xPan -= normDx / m_xZoom;
  m_yPan -= normDy / m_yZoom;

  const double maxPanX = (1.0 - 1.0 / m_xZoom) * 0.5;
  const double maxPanY = (1.0 - 1.0 / m_yZoom) * 0.5;
  m_xPan               = qBound(-maxPanX, m_xPan, maxPanX);
  m_yPan               = qBound(-maxPanY, m_yPan, maxPanY);

  Q_EMIT viewChanged();
  markAxisDirty();
}

/**
 * @brief Restores zoom = 1.0 and pan = 0 on both axes.
 */
void Widgets::Waterfall::resetView()
{
  if (atDefaultView())
    return;

  m_xZoom = 1.0;
  m_yZoom = 1.0;
  m_xPan  = 0.0;
  m_yPan  = 0.0;

  Q_EMIT viewChanged();
  markAxisDirty();
}

//--------------------------------------------------------------------------------------------------
// Mouse / wheel event handling (matches Plot3D's interaction pattern)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wheel = zoom toward the cursor; touch-pad gestures zoom more gently.
 */
void Widgets::Waterfall::wheelEvent(QWheelEvent* event)
{
  if (event->angleDelta().y() == 0) {
    event->ignore();
    return;
  }

  const bool isTouchpad =
    !event->pixelDelta().isNull() || event->source() == Qt::MouseEventSynthesizedBySystem;
  const double zoomBase    = isTouchpad ? 1.05 : 1.15;
  constexpr double kInv120 = 1.0 / 120.0;
  const double delta       = event->angleDelta().y() * kInv120;
  const double factor      = std::pow(zoomBase, delta);

  const QPointF p = event->position();
  const double w  = qMax(1.0, width());
  const double h  = qMax(1.0, height());
  zoomBy(factor, p.x() / w, p.y() / h);
  event->accept();
}

/**
 * @brief Mouse-press toggles the marker spotlight when it lands on a chip; anywhere else it
 *        starts the drag-to-pan interaction.
 */
void Widgets::Waterfall::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    event->ignore();
    return;
  }

  if (m_markersVisible && !m_markers.empty()) {
    const int hit = markerChipAt(event->position());
    if (hit >= 0) {
      m_selectedMarker = (m_selectedMarker == hit) ? -1 : hit;
      update();
      event->accept();
      return;
    }
  }

  m_dragging     = true;
  m_lastMousePos = event->position();
  setCursor(Qt::ClosedHandCursor);
  grabMouse();
  event->accept();
}

/**
 * @brief While dragging, translate the view by the cursor delta.
 */
void Widgets::Waterfall::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_dragging) {
    event->ignore();
    return;
  }

  const QPointF p = event->position();
  const double dx = (p.x() - m_lastMousePos.x()) / qMax(1.0, width());
  const double dy = (p.y() - m_lastMousePos.y()) / qMax(1.0, height());
  m_lastMousePos  = p;

  panBy(dx, dy);
  event->accept();
}

/**
 * @brief Mouse-release ends the drag and restores the cursor.
 */
void Widgets::Waterfall::mouseReleaseEvent(QMouseEvent* event)
{
  if (!m_dragging) {
    event->ignore();
    return;
  }

  m_dragging = false;
  unsetCursor();
  ungrabMouse();
  event->accept();
}

/**
 * @brief Re-renders the axis overlay when the widget is resized.
 */
void Widgets::Waterfall::geometryChange(const QRectF& newGeom, const QRectF& oldGeom)
{
  QQuickPaintedItem::geometryChange(newGeom, oldGeom);
  if (newGeom.size() != oldGeom.size())
    markAxisDirty();
}

/**
 * @brief Records the entering pointer position so the cursor draws immediately.
 */
void Widgets::Waterfall::hoverEnterEvent(QHoverEvent* event)
{
  m_cursorHovering = true;
  m_cursorPos      = event->position();
  if (m_cursorEnabled)
    update();

  event->accept();
}

/**
 * @brief Tracks the pointer, hints chip clickability with a pointing-hand cursor, and only
 *        schedules a repaint when the readout cursor is on.
 */
void Widgets::Waterfall::hoverMoveEvent(QHoverEvent* event)
{
  m_cursorHovering = true;
  m_cursorPos      = event->position();

  if (!m_dragging) {
    if (m_markersVisible && markerChipAt(m_cursorPos) >= 0)
      setCursor(Qt::PointingHandCursor);
    else
      unsetCursor();
  }

  if (m_cursorEnabled)
    update();

  event->accept();
}

/**
 * @brief Clears the cursor when the pointer leaves the item.
 */
void Widgets::Waterfall::hoverLeaveEvent(QHoverEvent* event)
{
  m_cursorHovering = false;
  if (m_cursorEnabled)
    update();

  event->accept();
}

//--------------------------------------------------------------------------------------------------
// Theme + font reactive hooks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes cached theme colors and forces an axis re-render.
 */
void Widgets::Waterfall::onThemeChanged()
{
  m_outerBg      = m_themeManager.getColor(QStringLiteral("widget_window"));
  m_innerBg      = m_themeManager.getColor(QStringLiteral("widget_base"));
  m_borderColor  = m_themeManager.getColor(QStringLiteral("widget_border"));
  m_textColor    = m_themeManager.getColor(QStringLiteral("widget_text"));
  m_gridColor    = QColor(m_borderColor.red(), m_borderColor.green(), m_borderColor.blue(), 80);
  m_accentColor  = m_themeManager.getColor(QStringLiteral("highlight"));
  m_warningColor = m_themeManager.alarmColorForSeverity(2);
  m_alarmColor   = m_themeManager.alarmColorForSeverity(3);

  markAxisDirty();
}

/**
 * @brief Forces an axis re-render when the dashboard widget-font scale changes.
 */
void Widgets::Waterfall::onFontsChanged()
{
  markAxisDirty();
}
