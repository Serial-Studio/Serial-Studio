/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <QCursor>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cstring>

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "UI/Dashboard.h"

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

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes a single coefficient of the 4-term Blackman-Harris window.
 */
static inline float blackmanHarris(unsigned int i, unsigned int N)
{
  if (N <= 1)
    return 1.0f;

  constexpr float a0    = 0.35875f;
  constexpr float a1    = 0.48829f;
  constexpr float a2    = 0.14128f;
  constexpr float a3    = 0.01168f;
  constexpr float twoPi = 6.28318530717958647692f;
  const float k         = twoPi / static_cast<float>(N - 1);
  const float x         = k * static_cast<float>(i);

  return a0 - a1 * std::cos(x) + a2 * std::cos(2.0f * x) - a3 * std::cos(3.0f * x);
}

/**
 * @brief Returns the largest power-of-two not exceeding @a value (and ≥ 8).
 */
static inline int floorPow2Bounded(int value)
{
  const int clamped = qBound(8, value, kMaxFftSamples);
  return 1 << static_cast<int>(std::log2(clamped));
}

/**
 * @brief Returns the RGB color for a given color map and normalized magnitude.
 */
QRgb Widgets::Waterfall::sampleColorMap(int map, double t)
{
  // Clamp to 0..1 for safety
  t = qBound(0.0, t, 1.0);

  switch (map) {
    // Viridis — perceptually-uniform sequential map (Matplotlib standard)
    case Viridis: {
      static constexpr double r[] = {0.267, 0.282, 0.253, 0.207, 0.164, 0.135, 0.135, 0.267,
                                     0.478, 0.741, 0.993};
      static constexpr double g[] = {0.005, 0.100, 0.265, 0.371, 0.471, 0.567, 0.659, 0.749,
                                     0.821, 0.873, 0.906};
      static constexpr double b[] = {0.329, 0.529, 0.529, 0.553, 0.557, 0.553, 0.518, 0.440,
                                     0.318, 0.150, 0.144};
      const int n                 = 11;
      const double f              = t * (n - 1);
      const int i                 = qBound(0, static_cast<int>(f), n - 2);
      const double s              = f - i;
      const int rr                = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
      const int gg                = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
      const int bb                = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);
      return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
    }

    // Inferno — black → red → orange → yellow
    case Inferno: {
      static constexpr double r[] = {0.001, 0.099, 0.301, 0.527, 0.733, 0.882, 0.973, 0.988};
      static constexpr double g[] = {0.000, 0.034, 0.064, 0.117, 0.214, 0.388, 0.626, 0.998};
      static constexpr double b[] = {0.014, 0.299, 0.434, 0.395, 0.276, 0.118, 0.034, 0.645};
      const int n                 = 8;
      const double f              = t * (n - 1);
      const int i                 = qBound(0, static_cast<int>(f), n - 2);
      const double s              = f - i;
      const int rr                = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
      const int gg                = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
      const int bb                = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);
      return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
    }

    // Magma — black → purple → pink → cream
    case Magma: {
      static constexpr double r[] = {0.001, 0.146, 0.421, 0.715, 0.928, 0.987, 0.987};
      static constexpr double g[] = {0.000, 0.060, 0.139, 0.215, 0.473, 0.749, 0.991};
      static constexpr double b[] = {0.014, 0.347, 0.516, 0.475, 0.502, 0.622, 0.749};
      const int n                 = 7;
      const double f              = t * (n - 1);
      const int i                 = qBound(0, static_cast<int>(f), n - 2);
      const double s              = f - i;
      const int rr                = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
      const int gg                = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
      const int bb                = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);
      return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
    }

    // Plasma — purple → pink → orange → yellow
    case Plasma: {
      static constexpr double r[] = {0.050, 0.286, 0.530, 0.741, 0.892, 0.969, 0.940};
      static constexpr double g[] = {0.030, 0.010, 0.140, 0.347, 0.560, 0.789, 0.975};
      static constexpr double b[] = {0.527, 0.629, 0.586, 0.415, 0.227, 0.105, 0.131};
      const int n                 = 7;
      const double f              = t * (n - 1);
      const int i                 = qBound(0, static_cast<int>(f), n - 2);
      const double s              = f - i;
      const int rr                = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
      const int gg                = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
      const int bb                = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);
      return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
    }

    // Turbo — Google's improved Jet replacement
    case Turbo: {
      static constexpr double r[] = {0.190, 0.275, 0.247, 0.085, 0.152, 0.617, 0.964, 0.974, 0.479};
      static constexpr double g[] = {0.072, 0.366, 0.703, 0.916, 0.988, 0.983, 0.787, 0.317, 0.016};
      static constexpr double b[] = {0.232, 0.804, 0.964, 0.757, 0.357, 0.141, 0.180, 0.108, 0.011};
      const int n                 = 9;
      const double f              = t * (n - 1);
      const int i                 = qBound(0, static_cast<int>(f), n - 2);
      const double s              = f - i;
      const int rr                = static_cast<int>((r[i] + (r[i + 1] - r[i]) * s) * 255.0);
      const int gg                = static_cast<int>((g[i] + (g[i + 1] - g[i]) * s) * 255.0);
      const int bb                = static_cast<int>((b[i] + (b[i + 1] - b[i]) * s) * 255.0);
      return qRgb(qBound(0, rr, 255), qBound(0, gg, 255), qBound(0, bb, 255));
    }

    // Jet — classic blue → cyan → green → yellow → red
    case Jet: {
      const double v   = t;
      const double r   = qBound(0.0, qMin(4.0 * v - 1.5, 4.5 - 4.0 * v), 1.0);
      const double g   = qBound(0.0, qMin(4.0 * v - 0.5, 3.5 - 4.0 * v), 1.0);
      const double b   = qBound(0.0, qMin(4.0 * v + 0.5, 2.5 - 4.0 * v), 1.0);
      return qRgb(static_cast<int>(r * 255.0), static_cast<int>(g * 255.0),
                  static_cast<int>(b * 255.0));
    }

    // Hot — black → red → yellow → white
    case Hot: {
      const double v = t;
      const double r = qBound(0.0, 3.0 * v, 1.0);
      const double g = qBound(0.0, 3.0 * v - 1.0, 1.0);
      const double b = qBound(0.0, 3.0 * v - 2.0, 1.0);
      return qRgb(static_cast<int>(r * 255.0), static_cast<int>(g * 255.0),
                  static_cast<int>(b * 255.0));
    }

    // Grayscale
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
  : QuickPaintedItemCompat(parent)
  , m_index(index)
  , m_size(0)
  , m_samplingRate(0)
  , m_historySize(kDefaultHistory)
  , m_colorMap(Turbo)
  , m_writeRow(0)
  , m_filledOnce(false)
  , m_axisVisible(true)
  , m_minDb(-90.0)
  , m_maxDb(0.0)
  , m_center(0.0)
  , m_halfRange(1.0)
  , m_xZoom(1.0)
  , m_yZoom(1.0)
  , m_xPan(0.0)
  , m_yPan(0.0)
  , m_scaleIsValid(false)
  , m_dragging(false)
  , m_axisDirty(true)
  , m_cursorEnabled(false)
  , m_cursorHovering(false)
  , m_campbellMode(false)
  , m_yDatasetIndex(0)
  , m_yMin(0.0)
  , m_yMax(1.0)
  , m_plan(nullptr)
{
  // Accept mouse + wheel events directly — zoom/pan handled in C++. Hover
  // events drive the freq/time cursor when enabled.
  setAcceptedMouseButtons(Qt::LeftButton);
  setAcceptHoverEvents(true);

  // Pull dataset config — clamp samples since project JSON is user-controlled
  if (VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
    m_size              = floorPow2Bounded(dataset.fftSamples);
    m_samplingRate      = qMax(1, dataset.fftSamplingRate);

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

    // Resolve Campbell-mode binding — the project model stores the Y dataset's
    // index (0 = Time/scrolling default). Look it up in the dashboard's flat
    // dataset map and harvest title/units/range. Falls back to time mode if the
    // referenced dataset isn't found or its plot range is degenerate.
    m_yDatasetIndex = dataset.waterfallYAxis;
    if (m_yDatasetIndex > 0) {
      const auto& datasets = UI::Dashboard::instance().datasets();
      const auto it        = datasets.find(m_yDatasetIndex);
      if (it != datasets.end() && it->pltMax > it->pltMin) {
        m_campbellMode = true;
        m_yMin         = it->pltMin;
        m_yMax         = it->pltMax;
        m_yAxisTitle   = it->units.isEmpty()
                         ? it->title
                         : QStringLiteral("%1 (%2)").arg(it->title, it->units);
      }
    }

    allocateFftPlan(m_size);
    rebuildHistoryImage();
  }

  // Initial theme/font snapshot
  onThemeChanged();

  // Repaint at the dashboard UI tick rate
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this,
          &Widgets::Waterfall::updateData);

  // React to theme + dashboard-font changes (axis labels follow the global scale)
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged, this,
          &Widgets::Waterfall::onThemeChanged);
  connect(&Misc::CommonFonts::instance(), &Misc::CommonFonts::fontsChanged, this,
          &Widgets::Waterfall::onFontsChanged);
}

/**
 * @brief Releases the FFT plan and any owned buffers.
 */
Widgets::Waterfall::~Waterfall()
{
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
  return UI::Dashboard::instance().waterfallRunning(m_index);
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
  UI::Dashboard::instance().setWaterfallRunning(m_index, enabled);
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
 * @brief Sets the lower clip of the dB → color mapping.
 */
void Widgets::Waterfall::setMinDb(const double value)
{
  if (qFuzzyCompare(m_minDb, value))
    return;

  m_minDb = value;
  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Sets the upper clip of the dB → color mapping.
 */
void Widgets::Waterfall::setMaxDb(const double value)
{
  if (qFuzzyCompare(m_maxDb, value))
    return;

  m_maxDb = value;
  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Resets the time history to the floor color.
 */
void Widgets::Waterfall::clearHistory()
{
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

  const auto windowSize = static_cast<unsigned int>(m_size);
  for (unsigned int i = 0; i < windowSize; ++i)
    m_window[i] = blackmanHarris(i, windowSize);

  m_plan = kiss_fft_alloc(m_size, 0, nullptr, nullptr);
  if (!m_plan)
    qWarning() << "Waterfall FFT plan allocation failed for size:" << m_size;
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
 *
 * Newest spectrum rows are inserted at row 0 (top); older rows are scrolled
 * down each tick. m_writeRow tracks how many rows currently hold valid data
 * (capped at image height once fully populated).
 */
void Widgets::Waterfall::rebuildHistoryImage()
{
  const int width  = qMax(1, m_size / 2);
  const int height = qMax(1, m_historySize);
  m_image          = QImage(width, height, QImage::Format_RGB32);
  m_image.fill(sampleColorMap(m_colorMap, 0.0));
  m_writeRow   = 0;
  m_filledOnce = false;
}

/**
 * @brief Writes a new spectrum row at the top of the image, scrolling down.
 */
void Widgets::Waterfall::writeRow(const float* dbValues, int bins)
{
  if (m_image.isNull() || bins <= 0)
    return;

  // Detach explicitly before the in-place memmove. QImage uses copy-on-write,
  // so a single mutating call (bits() / scanLine()) would silently allocate a
  // brand-new pixel buffer if the previous frame's drawImage() left a shared
  // reference behind. Doing the detach up front means we never make the runtime
  // do it implicitly halfway through the row update — keeps memory churn flat.
  m_image.detach();

  // Scroll all existing rows down by one pixel via a single memmove. The
  // overlapping copy is safe because std::memmove handles direction correctly.
  const int imageWidth  = m_image.width();
  const int imageHeight = m_image.height();
  if (imageHeight > 1) {
    const qsizetype bpl = m_image.bytesPerLine();
    uchar* base         = m_image.bits();
    std::memmove(base + bpl, base, static_cast<size_t>(bpl) * (imageHeight - 1));
  }

  // Map each bin's dB into a normalized 0..1 value and resolve a color map entry
  const float minDb       = static_cast<float>(m_minDb);
  const float maxDb       = static_cast<float>(m_maxDb);
  const float invDbRange  = 1.0f / qMax(1e-6f, maxDb - minDb);
  const int writableBins  = qMin(bins, imageWidth);
  QRgb* scan              = reinterpret_cast<QRgb*>(m_image.scanLine(0));

  for (int x = 0; x < writableBins; ++x) {
    const float v  = (dbValues[x] - minDb) * invDbRange;
    const double t = qBound(0.0, static_cast<double>(v), 1.0);
    scan[x]        = sampleColorMap(m_colorMap, t);
  }

  // Pad with floor color if the spectrum has fewer columns than the image
  if (writableBins < imageWidth) {
    const QRgb floor = sampleColorMap(m_colorMap, 0.0);
    for (int x = writableBins; x < imageWidth; ++x)
      scan[x] = floor;
  }

  // Track filled rows for partial-fill painting until the buffer wraps once
  if (!m_filledOnce) {
    if (++m_writeRow >= imageHeight) {
      m_writeRow   = imageHeight;
      m_filledOnce = true;
    }
  }
}

/**
 * @brief Writes a spectrum row at a specific image row without scrolling.
 *
 * Used by Campbell mode: the target row is computed from the bound dataset's
 * value (e.g. RPM) so the same Y position is overwritten when that value
 * recurs, building up a value-vs-frequency surface instead of a time history.
 */
void Widgets::Waterfall::writeRowAt(int row, const float* dbValues, int bins)
{
  if (m_image.isNull() || bins <= 0)
    return;

  const int imageHeight = m_image.height();
  if (row < 0 || row >= imageHeight)
    return;

  m_image.detach();

  const int imageWidth   = m_image.width();
  const float minDb      = static_cast<float>(m_minDb);
  const float maxDb      = static_cast<float>(m_maxDb);
  const float invDbRange = 1.0f / qMax(1e-6f, maxDb - minDb);
  const int writableBins = qMin(bins, imageWidth);
  QRgb* scan             = reinterpret_cast<QRgb*>(m_image.scanLine(row));

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
 * @brief Pulls the latest time-domain samples, runs FFT, pushes a new row.
 */
void Widgets::Waterfall::updateData()
{
  if (!isEnabled() || !isVisible())
    return;

  if (!UI::Dashboard::instance().waterfallRunning(m_index))
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return;

  // Fetch time-domain ring buffer (newest sample at end)
  const auto& data  = UI::Dashboard::instance().waterfallData(m_index);
  const int newSize = static_cast<int>(data.size());
  if (newSize <= 0)
    return;

  // Re-init FFT structures if the dataset's FFT window size changed mid-run
  if (newSize != m_size) {
    allocateFftPlan(newSize);
    rebuildHistoryImage();
    Q_EMIT historySizeChanged();
  }

  if (!m_plan)
    return;

  // Window time-domain input + normalize against user-configured min/max
  const double* in      = data.raw();
  std::size_t idx       = data.frontIndex();
  const std::size_t cap = data.capacity();
  const double offset   = m_scaleIsValid ? -m_center : 0.0;
  const double scale    = m_scaleIsValid ? (1.0 / m_halfRange) : 1.0;

  for (int i = 0; i < m_size; ++i) {
    const double raw = in[idx];
    const float v    = std::isfinite(raw) ? static_cast<float>((raw + offset) * scale) : 0.0f;
    m_samples[i].r   = v * m_window[i];
    m_samples[i].i   = 0.0f;
    idx              = (idx + 1) % cap;
  }

  // Run FFT
  kiss_fft(m_plan, m_samples.data(), m_fftOutput.data());

  // Convert to dB
  const int spectrumSize    = m_size / 2;
  const float normFactor    = static_cast<float>(m_size) * static_cast<float>(m_size);
  for (int i = 0; i < spectrumSize; ++i) {
    const float re    = m_fftOutput[i].r;
    const float im    = m_fftOutput[i].i;
    const float power = std::max((re * re + im * im) / normFactor, kEpsSquared);
    m_dbCache[i]      = std::max(10.0f * std::log10(power), kFloorDb);
  }

  // Light frequency-axis smoothing (3-bin) to reduce single-bin sparkle. Buffer
  // is owned by the widget so it tracks the instance lifetime — a thread_local
  // here would persist across widget destruction and grow with the largest
  // FFT size ever seen process-wide.
  if (m_smoothed.size() < static_cast<size_t>(spectrumSize))
    m_smoothed.resize(spectrumSize);

  for (int i = 0; i < spectrumSize; ++i) {
    const int lo = std::max(0, i - kHalfSmoothWindow);
    const int hi = std::min(spectrumSize - 1, i + kHalfSmoothWindow);
    float sum    = 0.0f;
    for (int k = lo; k <= hi; ++k)
      sum += m_dbCache[k];

    m_smoothed[i] = sum / static_cast<float>(hi - lo + 1);
  }

  // Push the new row into the history image. In time mode we scroll all rows
  // down and write the freshest spectrum at the top; in Campbell mode we look
  // up the Y dataset's current value and write into the row indexed by it.
  if (m_campbellMode && m_image.height() > 0) {
    const auto& datasets = UI::Dashboard::instance().datasets();
    const auto it        = datasets.find(m_yDatasetIndex);
    if (it != datasets.end() && it->isNumeric) {
      const double v     = it->numericValue;
      const double range = m_yMax - m_yMin;
      if (range > 0.0) {
        // High Y value at top, low at bottom (standard Campbell convention)
        const double t = qBound(0.0, (v - m_yMin) / range, 1.0);
        const int row  = qBound(0, static_cast<int>((1.0 - t) * (m_image.height() - 1)),
                                m_image.height() - 1);
        writeRowAt(row, m_smoothed.data(), spectrumSize);
      }
    }
  } else {
    writeRow(m_smoothed.data(), spectrumSize);
  }
  update();
}

//--------------------------------------------------------------------------------------------------
// Painting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Renders the spectrogram and composites it with the cached axis layer.
 *
 * The axis layer (frame, ticks, grid lines, labels, rotated title) only needs
 * to be re-rendered when the widget size, zoom/pan, theme, or axis-visibility
 * changes — markAxisDirty() invalidates the cache. Every UI tick (24 Hz) the
 * spectrogram changes via writeRow(), so only the per-frame work below runs:
 * a fillRect + drawImage + drawImage of the cached overlay.
 */
void Widgets::Waterfall::paint(QPainter* painter)
{
  if (!painter)
    return;

  // Antialiased lines + crisp text apply to anything drawn here (including the
  // cursor overlay, which lives outside the cached axis layer)
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  // Rebuild the axis overlay if invalidated since the previous paint
  if (m_axisDirty)
    renderAxisLayer();

  // Outer background — fills axis-margin areas with the theme's window color so
  // the painted item never shows the scenegraph default (which renders black).
  const QRectF outerRect(0, 0, width(), height());
  painter->fillRect(outerRect, m_outerBg);

  // Inner plot area background (theme widget_base) — visible until enough rows
  // have arrived to fill the spectrogram, and behind any color-mapped floor.
  const QRectF& plotRect = m_cachedPlotRect;
  if (plotRect.isEmpty())
    return;

  painter->fillRect(plotRect, m_innerBg);

  // Spectrogram image, clipped to plotRect; source rect derives from zoom/pan
  if (!m_image.isNull()) {
    painter->save();
    painter->setClipRect(plotRect);
    painter->drawImage(plotRect, m_image, computeSourceRect());
    painter->restore();
  }

  // Composite the cached axis overlay (border, ticks, grid, labels, title)
  if (!m_axisLayer.isNull())
    painter->drawImage(outerRect, m_axisLayer, QRectF(m_axisLayer.rect()));

  // Cursor overlay (live; not part of the cached axis layer)
  if (m_cursorEnabled && m_cursorHovering)
    drawCursor(painter, plotRect);
}

//--------------------------------------------------------------------------------------------------
// Axis layer cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-renders the axis overlay into m_axisLayer and clears the dirty flag.
 *
 * Drawing axes (ticks, labels, rotated title) involves QFontMetrics queries,
 * QPainter text layout, and grid-line passes — all cheap individually but
 * wasteful at 24 Hz when nothing about the axes has changed. Caching the
 * result into a single QImage means paint() reduces to two drawImage calls.
 */
void Widgets::Waterfall::renderAxisLayer()
{
  m_axisDirty = false;

  // Determine the device-pixel-aware buffer size
  const QSize itemSize(qMax(1, qCeil(width())), qMax(1, qCeil(height())));
  const qreal dpr      = (window() ? window()->devicePixelRatio() : 1.0);
  const QSize bufSize  = itemSize * dpr;
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

  // Use the dashboard's widget font so axis labels track the global scale
  static auto& fonts = Misc::CommonFonts::instance();
  painter.setFont(fonts.widgetFont(0.83, false));

  // Compute (and cache) the plot rect; paint() reuses it without recomputation
  const QFontMetrics fm(painter.font());
  m_cachedPlotRect = computePlotRect(fm);
  if (m_cachedPlotRect.isEmpty())
    return;

  // Plot frame (cached border color)
  painter.setPen(QPen(m_borderColor, 1));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(m_cachedPlotRect);

  // Axes (ticks + grid + labels) when the widget is large enough
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
  // No axes — plot fills the whole item (border included)
  if (!m_axisVisible || width() < kMinAxisWidth || height() < kMinAxisHeight)
    return QRectF(0.5, 0.5, qMax(0.0, width() - 1), qMax(0.0, height() - 1));

  // Reserve room for the rotated "Time (s)" Y-axis title + tick column on the
  // left, and the X tick row on the bottom. Title is rendered at 1.0× scale
  // (vs 0.85× for ticks) so we use the title-font metrics to size its column.
  static auto& fonts        = Misc::CommonFonts::instance();
  const QFontMetrics titleFm(fonts.widgetFont(0.91, true));

  const int yTickWidth  = fm.horizontalAdvance(QStringLiteral("00.00"))
                        + kAxisTickPx + kAxisLabelPad;
  const int yTitleWidth = titleFm.height() + 2;  // tight title column

  const int leftMargin   = yTitleWidth + yTickWidth;
  const int rightMargin  = kAxisLabelPad;
  const int topMargin    = kAxisLabelPad;
  const int bottomMargin = fm.height() + kAxisTickPx + kAxisLabelPad * 2;

  return QRectF(leftMargin + 0.5, topMargin + 0.5,
                qMax(0.0, width() - leftMargin - rightMargin - 1),
                qMax(0.0, height() - topMargin - bottomMargin - 1));
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

  // Pan range is constrained so the source rect never falls outside the image
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
 * @brief Draws the frequency axis (X) — grid, tick marks, labels.
 */
void Widgets::Waterfall::drawXAxis(QPainter* painter, const QRectF& plotRect) const
{
  if (m_samplingRate <= 0)
    return;

  // Map zoom/pan into the visible Hz range: at zoom=1 the full Nyquist sweep is
  // visible; at zoom=k a 1/k slice is shown, panned by m_xPan along the band.
  const double nyquist = m_samplingRate * 0.5;
  const double srcW    = nyquist / m_xZoom;
  const double maxPan  = qMax(0.0, (nyquist - srcW) * 0.5);
  const double center  = nyquist * 0.5 + qBound(-maxPan, m_xPan * nyquist, maxPan);
  const double xMinHz  = center - srcW * 0.5;
  const double xMaxHz  = center + srcW * 0.5;

  const AxisTicks ticks = computeFreqTicks(srcW, kAxisTickCount);

  const QFontMetrics fm(painter->font());
  const double tickTopY = plotRect.bottom();
  const double tickBotY = plotRect.bottom() + kAxisTickPx;
  const double labelY   = tickBotY + kAxisLabelPad;

  // Pick a starting tick value aligned to the grid (snap to nearest step)
  const double step  = ticks.step;
  const double first = std::ceil(xMinHz / step - 1e-9) * step;

  for (double v = first; v <= xMaxHz + 1e-6; v += step) {
    const double t = (v - xMinHz) / (xMaxHz - xMinHz);
    if (t < 0.0 || t > 1.0)
      continue;

    const double x = plotRect.left() + t * plotRect.width();

    // Vertical grid line through the plot
    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));

    // Tick mark
    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(x, tickTopY), QPointF(x, tickBotY));

    // Label centered under the tick
    const QString label = formatFreqTick(v);
    const int labelWidth = fm.horizontalAdvance(label);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(x - labelWidth * 0.5, labelY + fm.ascent()), label);
  }
}

/**
 * @brief Draws the Y axis (time or Campbell-mode dataset value).
 *
 * - Time mode: top = newest (t=0), bottom = oldest (t=maxSeconds). The Y range
 *   derives from `historySize / fps`.
 * - Campbell mode: top = highest dataset value, bottom = lowest. The Y range
 *   comes from the bound dataset's pltMin/pltMax. Title shows "Title (units)".
 */
void Widgets::Waterfall::drawYAxis(QPainter* painter, const QRectF& plotRect) const
{
  // Resolve the axis data range based on mode
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

  // Apply Y zoom/pan around the data center
  const double srcH    = dataRange / m_yZoom;
  const double maxPan  = qMax(0.0, (dataRange - srcH) * 0.5);
  const double centerD = (dataMin + dataMax) * 0.5
                       + qBound(-maxPan, m_yPan * dataRange, maxPan);
  const double yMin    = centerD - srcH * 0.5;
  const double yMax    = centerD + srcH * 0.5;

  const AxisTicks ticks = computeTimeTicks(srcH, kAxisTickCount);

  const QFontMetrics fm(painter->font());
  const double tickRightX = plotRect.left();
  const double tickLeftX  = plotRect.left() - kAxisTickPx;
  const double labelRight = tickLeftX - kAxisLabelPad;

  const double step  = ticks.step;
  const double first = std::ceil(yMin / step - 1e-9) * step;

  for (double v = first; v <= yMax + 1e-6; v += step) {
    const double t = (v - yMin) / (yMax - yMin);
    if (t < 0.0 || t > 1.0)
      continue;

    // Time mode: t=0 (newest) at top → y = top + t*h.
    // Campbell mode: high value at top → y = bottom - t*h (flipped).
    const double y = m_campbellMode
                       ? plotRect.bottom() - t * plotRect.height()
                       : plotRect.top()    + t * plotRect.height();

    // Horizontal grid line across the plot
    painter->setPen(QPen(m_gridColor, 1, Qt::DotLine));
    painter->drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));

    // Tick mark
    painter->setPen(QPen(m_borderColor, 1));
    painter->drawLine(QPointF(tickLeftX, y), QPointF(tickRightX, y));

    // Label clamped vertically so first/last labels don't get cut by the frame
    const QString label = formatTimeTick(v, step);
    const int labelWidth = fm.horizontalAdvance(label);
    const double textCy  = qBound(plotRect.top() + fm.ascent() * 0.5,
                                  y + fm.ascent() * 0.5,
                                  plotRect.bottom() + fm.ascent() * 0.5);
    painter->setPen(m_textColor);
    painter->drawText(QPointF(labelRight - labelWidth, textCy), label);
  }

  // Rotated axis title — "Time (s)" by default, dataset title (+ units) when in
  // Campbell mode. Drawn at 0.91× bold scale, matching PlotWidget's axis titles.
  static auto& fonts = Misc::CommonFonts::instance();
  const QFont titleFont(fonts.widgetFont(0.91, true));
  const QFontMetrics titleFm(titleFont);

  const QString title = m_campbellMode ? m_yAxisTitle : QObject::tr("Time (s)");
  const double titleX = labelRight - fm.horizontalAdvance(QStringLiteral("00.00"))
                      - 2 - titleFm.descent();
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
 * @brief Draws the live hover cursor — vertical & horizontal crosshair lines
 *        clipped to the plot rect, plus a small tooltip with the freq + time
 *        readings under the pointer (zoom/pan-aware).
 */
void Widgets::Waterfall::drawCursor(QPainter* painter, const QRectF& plotRect) const
{
  if (plotRect.isEmpty() || !plotRect.contains(m_cursorPos))
    return;

  // Set the dashboard widget font for the tooltip text
  static auto& fonts = Misc::CommonFonts::instance();
  painter->setFont(fonts.widgetFont(0.83, false));
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  // Crosshair lines — clamped within plotRect to avoid overdrawing the axes
  const double cx = qBound(plotRect.left(), m_cursorPos.x(), plotRect.right());
  const double cy = qBound(plotRect.top(), m_cursorPos.y(), plotRect.bottom());

  painter->setPen(QPen(QColor(255, 255, 255, 178), 1));
  painter->drawLine(QPointF(cx, plotRect.top()), QPointF(cx, plotRect.bottom()));
  painter->drawLine(QPointF(plotRect.left(), cy), QPointF(plotRect.right(), cy));

  // Convert cursor position into the visible X (Hz) and Y (seconds OR Campbell
  // dataset value) windows
  const double nyquist = m_samplingRate * 0.5;
  const double srcWHz  = nyquist / m_xZoom;
  const double maxPanX = qMax(0.0, (nyquist - srcWHz) * 0.5);
  const double centerH = nyquist * 0.5 + qBound(-maxPanX, m_xPan * nyquist, maxPanX);
  const double xMinHz  = centerH - srcWHz * 0.5;
  const double freqHz  = xMinHz + (cx - plotRect.left()) / plotRect.width() * srcWHz;

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
  const double centerY = (yMinAxis + yMaxAxis) * 0.5
                       + qBound(-maxPanY, m_yPan * yRange, maxPanY);
  const double yMinV   = centerY - srcWY * 0.5;
  const double yMaxV   = centerY + srcWY * 0.5;

  // Time mode is top→bottom (newest→oldest); Campbell mode is bottom→top
  // (low→high). Mirror the direction the data was written in.
  const double tY = (cy - plotRect.top()) / plotRect.height();
  const double yVal =
    m_campbellMode ? (yMaxV - tY * (yMaxV - yMinV)) : (yMinV + tY * (yMaxV - yMinV));

  // Format readout strings
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
  // Campbell readout uses the dataset's own title for the line label, leaving
  // the value formatted in plain numerics. Time mode keeps the "Time: −Xs" form.
  const QString timeText =
    m_campbellMode
      ? QStringLiteral("%1: %2").arg(m_yAxisTitle, QString::number(yVal, 'f', 2))
      : QObject::tr("Time: −%1").arg(fmtTime(yVal));

  // Tooltip box dimensions
  const QFontMetrics fm(painter->font());
  const int padX = 8;
  const int padY = 5;
  const int gap  = 2;
  const int w = std::max(fm.horizontalAdvance(freqText), fm.horizontalAdvance(timeText)) + padX * 2;
  const int h = fm.height() * 2 + gap + padY * 2;

  // Position next to the cursor; flip sides if it would overflow the plot
  double tx = cx + 12;
  double ty = cy + 12;
  if (tx + w > plotRect.right())
    tx = cx - 12 - w;
  if (ty + h > plotRect.bottom())
    ty = cy - 12 - h;
  tx = qBound(plotRect.left() + 2, tx, plotRect.right() - w - 2);
  ty = qBound(plotRect.top() + 2, ty, plotRect.bottom() - h - 2);

  // Draw the tooltip background
  const QRectF tipRect(tx, ty, w, h);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(0, 0, 0, 184));
  painter->drawRoundedRect(tipRect, 3, 3);

  // Two-line readout
  painter->setPen(QColor(Qt::white));
  const double textBaseline = ty + padY + fm.ascent();
  painter->drawText(QPointF(tx + padX, textBaseline), freqText);
  painter->drawText(QPointF(tx + padX, textBaseline + fm.height() + gap), timeText);
}

//--------------------------------------------------------------------------------------------------
// Tick generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a {1,2,5}×10ⁿ step for a given range and target tick count.
 */
Widgets::Waterfall::AxisTicks Widgets::Waterfall::computeFreqTicks(double maxFreq,
                                                                   int targetCount)
{
  AxisTicks out{ {}, 1.0, maxFreq };
  if (!std::isfinite(maxFreq) || maxFreq <= 0.0)
    return out;

  const double target = std::max(2, targetCount);
  const double raw    = maxFreq / target;
  const double base   = std::pow(10.0, std::floor(std::log10(raw)));
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
 * @brief Formats a time value — integer seconds when step ≥ 1, decimals otherwise.
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
 * @brief Returns the current X (frequency) zoom factor — 1.0 = full Nyquist sweep.
 */
double Widgets::Waterfall::xZoom() const noexcept
{
  return m_xZoom;
}

/**
 * @brief Returns the current Y (time) zoom factor — 1.0 = full history visible.
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
 * @brief Convenience flag — true when zoom is unity and pan is zero on both axes.
 */
bool Widgets::Waterfall::atDefaultView() const noexcept
{
  return qFuzzyCompare(m_xZoom, 1.0) && qFuzzyCompare(m_yZoom, 1.0)
      && qFuzzyIsNull(m_xPan) && qFuzzyIsNull(m_yPan);
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
 * @brief Multiplies both axis zooms by @a factor, anchored at (anchorX,anchorY)
 *        in normalized [0,1] item coordinates. Mirrors the wheel-zoom UX of the
 *        plot widgets — zooming centers on the cursor, not the plot midpoint.
 */
void Widgets::Waterfall::zoomBy(double factor, double anchorX, double anchorY)
{
  if (!std::isfinite(factor) || factor <= 0.0)
    return;

  const double newX = qBound(1.0, m_xZoom * factor, kMaxZoom);
  const double newY = qBound(1.0, m_yZoom * factor, kMaxZoom);

  // Re-anchor pan so the data point under the cursor stays visually fixed
  const double ax = qBound(0.0, anchorX, 1.0) - 0.5;
  const double ay = qBound(0.0, anchorY, 1.0) - 0.5;
  m_xPan          = m_xPan + ax * (1.0 / m_xZoom - 1.0 / newX);
  m_yPan          = m_yPan + ay * (1.0 / m_yZoom - 1.0 / newY);
  m_xZoom         = newX;
  m_yZoom         = newY;

  // Clamp pan to keep the visible window inside the data range
  const double maxPanX = (1.0 - 1.0 / m_xZoom) * 0.5;
  const double maxPanY = (1.0 - 1.0 / m_yZoom) * 0.5;
  m_xPan               = qBound(-maxPanX, m_xPan, maxPanX);
  m_yPan               = qBound(-maxPanY, m_yPan, maxPanY);

  Q_EMIT viewChanged();
  markAxisDirty();
}

/**
 * @brief Translates the view by (normDx, normDy) — both in normalized item-rect
 *        coordinates (e.g. 0.1 = 10% of the visible plot width/height).
 */
void Widgets::Waterfall::panBy(double normDx, double normDy)
{
  if (!std::isfinite(normDx) || !std::isfinite(normDy))
    return;

  // Drag direction is opposite to pan direction (drag right → data scrolls right)
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

  const bool isTouchpad = !event->pixelDelta().isNull()
                       || event->source() == Qt::MouseEventSynthesizedBySystem;
  const double zoomBase = isTouchpad ? 1.05 : 1.15;
  const double delta    = event->angleDelta().y() / 120.0;
  const double factor   = std::pow(zoomBase, delta);

  const QPointF p = event->position();
  const double w  = qMax(1.0, width());
  const double h  = qMax(1.0, height());
  zoomBy(factor, p.x() / w, p.y() / h);
  event->accept();
}

/**
 * @brief Mouse-press starts a drag-to-pan interaction.
 */
void Widgets::Waterfall::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    event->ignore();
    return;
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

  const QPointF p  = event->position();
  const double dx  = (p.x() - m_lastMousePos.x()) / qMax(1.0, width());
  const double dy  = (p.y() - m_lastMousePos.y()) / qMax(1.0, height());
  m_lastMousePos   = p;

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
  QuickPaintedItemCompat::geometryChange(newGeom, oldGeom);
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
 * @brief Tracks the pointer; only schedules a repaint when the cursor is on.
 */
void Widgets::Waterfall::hoverMoveEvent(QHoverEvent* event)
{
  m_cursorHovering = true;
  m_cursorPos      = event->position();
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
 *
 * Lookups via ThemeManager::getColor are not free at 24 Hz — caching the values
 * here means paint() and the axis-overlay builder hit member fields only.
 */
void Widgets::Waterfall::onThemeChanged()
{
  const auto& theme = Misc::ThemeManager::instance();
  m_outerBg     = theme.getColor(QStringLiteral("widget_window"));
  m_innerBg     = theme.getColor(QStringLiteral("widget_base"));
  m_borderColor = theme.getColor(QStringLiteral("widget_border"));
  m_textColor   = theme.getColor(QStringLiteral("widget_text"));

  // Grid lines = border color at low alpha so they don't overpower the data
  m_gridColor = QColor(m_borderColor.red(), m_borderColor.green(),
                       m_borderColor.blue(), 80);

  markAxisDirty();
}

/**
 * @brief Forces an axis re-render when the dashboard widget-font scale changes.
 */
void Widgets::Waterfall::onFontsChanged()
{
  markAxisDirty();
}
