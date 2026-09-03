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
#include <cmath>
#include <QCursor>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QWheelEvent>

#include "DSPSimd.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/AudioExport.h"
#include "UI/Widgets/FFTWindow.h"
#include "UI/Widgets/Waterfall/WaterfallRingTexture.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kDefaultHistory   = 256;
static constexpr int kMaxHistorySize   = 4096;
static constexpr float kFloorDb        = -100.0f;
static constexpr float kEpsSquared     = 1e-24f;
static constexpr int kSmoothingWindow  = 3;
static constexpr int kHalfSmoothWindow = kSmoothingWindow / 2;
static constexpr double kLn10          = 2.302585092994046;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Waterfall widget bound to the dataset at @a index.
 */
Widgets::Waterfall::Waterfall(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_size(0)
  , m_samplingRate(0)
  , m_windowType(SerialStudio::FFTWindowBlackmanHarris)
  , m_historySize(kDefaultHistory)
  , m_writeRow(0)
  , m_topRow(0)
  , m_filledOnce(false)
  , m_center(0.0)
  , m_halfRange(1.0)
  , m_scaleIsValid(false)
  , m_dragging(false)
  , m_releaseRenderResources(false)
  , m_imageReleased(false)
  , m_outerBgNode(nullptr)
  , m_innerBgNode(nullptr)
  , m_overlayNode(nullptr)
  , m_campbellMode(false)
  , m_yDatasetUniqueId(0)
  , m_yMin(0.0)
  , m_yMax(1.0)
  , m_logX(false)
  , m_logActive(false)
  , m_logMin(0.0)
  , m_logMax(1.0)
  , m_plan(nullptr)
  , m_dashboard(UI::Dashboard::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_commonFonts(Misc::CommonFonts::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_audioExport(Widgets::AudioExport::instance())
  , m_view(static_cast<int>(ColorMapCount), static_cast<int>(Turbo))
  , m_overlay(m_view, m_themeManager, m_commonFonts, m_timerEvents)
  , m_audioRecordingEnabled(false)
{
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::LeftButton);
  setAcceptHoverEvents(true);
  rebuildColorLut();

  if (VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
    m_size              = Widgets::normalizedFftSize(dataset.fftSamples, kMaxWaterfallFftSize);
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

  pushAxisModel();
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
  return m_view.colorMap();
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
  return m_view.minDb();
}

/**
 * @brief Returns the upper clip of the dB range mapped to the color map end.
 */
double Widgets::Waterfall::maxDb() const noexcept
{
  return m_view.maxDb();
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
  return QColor(WaterfallColorMap::sample(m_view.colorMap(), normalized));
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
  if (!m_view.setColorMap(map))
    return;

  rebuildColorLut();
  if (!m_image.isNull() && !m_filledOnce && m_writeRow == 0)
    m_image.fill(m_colorLut[0]);

  m_spectrogram.markAll();
  Q_EMIT colorMapChanged();
  update();
}

/**
 * @brief Bakes the active color map into the 256-entry table the row colorizer indexes.
 */
void Widgets::Waterfall::rebuildColorLut()
{
  m_colorLut = WaterfallColorMap::buildLut(m_view.colorMap());
  SS_ASSERT(
    m_colorLut.size() == static_cast<std::size_t>(WaterfallColorMap::kLutSize),
    m_colorLut.assign(static_cast<std::size_t>(WaterfallColorMap::kLutSize), qRgb(0, 0, 0)));
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
  pushAxisModel();
}

/**
 * @brief Sizes the time history to the dashboard/project Time Range (rows ~= range * fps).
 */
void Widgets::Waterfall::syncHistoryToTimeRange()
{
  const double fps   = m_timerEvents.fps() > 0 ? m_timerEvents.fps() : 24.0;
  const double range = m_dashboard.plotTimeRange();
  setHistorySize(static_cast<int>(std::lround(range * fps)));
}

/**
 * @brief Sets the lower clip of the dB, color mapping.
 */
void Widgets::Waterfall::setMinDb(const double value)
{
  if (!m_view.setMinDb(value))
    return;

  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Sets the upper clip of the dB, color mapping.
 */
void Widgets::Waterfall::setMaxDb(const double value)
{
  if (!m_view.setMaxDb(value))
    return;

  Q_EMIT dynamicRangeChanged();
}

/**
 * @brief Copies the dataset's frequency markers into the overlay (spec 0019); runs once at
 *        construction so the per-row path never touches the project model.
 */
void Widgets::Waterfall::loadMarkers()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardWaterfall, m_index))
    return;

  const auto& dataset = GET_DATASET(SerialStudio::DashboardWaterfall, m_index);
  std::vector<WaterfallOverlay::MarkerData> markers;
  markers.reserve(dataset.fftMarkers.size());
  for (const auto& m : dataset.fftMarkers) {
    WaterfallOverlay::MarkerData md;
    md.freqLo      = m.frequency;
    md.freqHi      = m.endFrequency;
    md.warningDb   = static_cast<float>(m.warningDb);
    md.alarmDb     = static_cast<float>(m.alarmDb);
    md.peakDb      = kFloorDb;
    md.state       = 0;
    md.customColor = m.color.isEmpty() ? QColor() : QColor::fromString(m.color);
    md.label       = m.label;
    markers.push_back(std::move(md));
  }

  m_overlay.setMarkers(std::move(markers));
}

/**
 * @brief Republishes the axis domain the overlay draws against; called whenever the FFT plan,
 *        the history depth or the Campbell binding changes, since the overlay caches it.
 */
void Widgets::Waterfall::pushAxisModel()
{
  WaterfallOverlay::AxisModel axis;
  axis.samplingRate = m_samplingRate;
  axis.fftSize      = m_size;
  axis.historySize  = m_historySize;
  axis.campbellMode = m_campbellMode;
  axis.logActive    = m_logActive;
  axis.logMin       = m_logMin;
  axis.logMax       = m_logMax;
  axis.yMin         = m_yMin;
  axis.yMax         = m_yMax;
  axis.yAxisTitle   = m_yAxisTitle;

  m_overlay.setAxisModel(axis);
  markAxisDirty();
}

/**
 * @brief Resets the time history to the floor color.
 */
void Widgets::Waterfall::clearHistory()
{
  m_topRow     = 0;
  m_writeRow   = 0;
  m_filledOnce = false;
  if (!m_image.isNull()) {
    m_image.fill(m_colorLut[0]);
    m_spectrogram.markAll();
  }

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

  SS_ASSERT(size % 2 == 0, return);

  m_size = size;
  m_window.resize(m_size);
  m_samples.resize(m_size);
  m_fftOutput.resize(m_size / 2 + 1);
  m_dbCache.assign(m_size / 2, kFloorDb);

  Widgets::fillFftWindow(m_windowType, m_window.data(), static_cast<unsigned int>(m_size));

  m_plan = kiss_fftr_alloc(m_size, 0, nullptr, nullptr);
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
  if (!m_logActive) {
    pushAxisModel();
    return;
  }

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

  pushAxisModel();
}

/**
 * @brief Returns the row to blit for the current axis mode: the raw linear-bin spectrum, or
 *        the same data resampled onto the log-spaced column grid through the cached LUT.
 */
const float* Widgets::Waterfall::imageRow(const float* dbValues, int bins)
{
  SS_ASSERT(dbValues != nullptr, return nullptr);
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
 * @brief Releases the FFT plan if one is currently allocated.
 */
void Widgets::Waterfall::releaseFftPlan()
{
  if (m_plan) {
    kiss_fftr_free(m_plan);
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
  m_image.fill(m_colorLut[0]);
  m_topRow        = 0;
  m_writeRow      = 0;
  m_filledOnce    = false;
  m_imageReleased = false;

  m_lastRow.clear();
  m_spectrogram.reset(height);
}

/**
 * @brief Drops the spectrogram image and the row bookkeeping when the widget goes off screen. A
 *        hidden waterfall stops receiving rows anyway (updateData early-returns), so holding the
 *        image only pinned megabytes per widget; the next show rebuilds it from the floor color.
 */
void Widgets::Waterfall::releaseHistoryImage()
{
  m_image = QImage();
  m_lastRow.clear();
  m_lastRow.shrink_to_fit();
  m_spectrogram.release();

  m_topRow        = 0;
  m_writeRow      = 0;
  m_filledOnce    = false;
  m_imageReleased = true;
}

/**
 * @brief Writes a new spectrum row into the ring: the newest row's physical position is m_topRow
 *        and the quads recompose the logical order, so a row insert costs O(width) instead of the
 *        old O(width x height) full-image memmove.
 */
void Widgets::Waterfall::writeRow(const float* dbValues, int bins)
{
  if (m_image.isNull() || bins <= 0)
    return;

  const int imageHeight = m_image.height();
  m_topRow              = (m_topRow + imageHeight - 1) % imageHeight;

  paintRowInto(m_topRow, dbValues, bins);

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

  paintRowInto((row + m_topRow) % imageHeight, dbValues, bins);
}

/**
 * @brief Colorizes one spectrum row into a physical scan line: dB values normalize onto the
 *        current display range and the bins past the spectrum take the colormap floor.
 */
void Widgets::Waterfall::paintRowInto(int physicalRow, const float* dbValues, int bins)
{
  SS_ASSERT(dbValues != nullptr, return);
  SS_ASSERT(physicalRow >= 0 && physicalRow < m_image.height(), return);

  const int imageWidth   = m_image.width();
  const float minDb      = static_cast<float>(m_view.minDb());
  const float invDbRange = m_view.invDbRange();
  const int writableBins = qMin(bins, imageWidth);
  const int lastEntry    = WaterfallColorMap::kLutSize - 1;
  const QRgb* lut        = m_colorLut.data();
  QRgb* scan             = reinterpret_cast<QRgb*>(m_image.scanLine(physicalRow));
  m_spectrogram.markRow(physicalRow);

  for (int x = 0; x < writableBins; ++x) {
    const float v = (dbValues[x] - minDb) * invDbRange;
    scan[x]       = lut[qBound(0, static_cast<int>(v * lastEntry + 0.5f), lastEntry)];
  }

  if (writableBins < imageWidth) {
    const QRgb floor = lut[0];
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
void Widgets::Waterfall::computeSmoothedRow(int spectrumSize)
{
  SS_ASSERT(spectrumSize > 0, return);
  SS_ASSERT(m_fftOutput.size() >= static_cast<std::size_t>(spectrumSize)
              && m_dbCache.size() >= static_cast<std::size_t>(spectrumSize),
            spectrumSize = static_cast<int>(std::min(m_fftOutput.size(), m_dbCache.size())));

  static_assert(sizeof(kiss_fft_cpx) == 2 * sizeof(float), "kiss_fft_cpx must pack two floats");

  const float normFactor = static_cast<float>(m_size) * static_cast<float>(m_size);
  const float invNorm    = 1.0f / normFactor;
  DSP::simdPowerSpectrumDb(reinterpret_cast<const float*>(m_fftOutput.data()),
                           m_dbCache.data(),
                           static_cast<std::size_t>(spectrumSize),
                           invNorm,
                           kEpsSquared,
                           kFloorDb);

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
 * @brief Pulls the latest time-domain samples, runs FFT, pushes a new row. Plan and image size
 *        from the ring's capacity, never its fill level. A spectrum identical to the previous
 *        tick's writes no row at all -- an idle source's row is bit-identical (R15.1); Campbell
 *        mode is exempt, another dataset's value places its rows.
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
  } else if (m_imageReleased) {
    rebuildHistoryImage();
  }

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
  computeSmoothedRow(spectrumSize);

  bool overlay_changed = false;
  if (m_overlay.hasMarkers() && spectrumSize > 0)
    overlay_changed =
      m_overlay.updateMarkerStates(m_smoothed.data(), spectrumSize) && m_view.markersVisible();

  const float* row_data = imageRow(m_smoothed.data(), spectrumSize);
  if (!row_data)
    return;

  const bool row_changed =
    WaterfallRingTexture::captureRowIfChanged(row_data, spectrumSize, m_lastRow);
  const bool write_row = row_changed || m_campbellMode;
  if (!write_row && !overlay_changed)
    return;

  if (!write_row) {
    markAxisDirty();
    return;
  }

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

  if (overlay_changed)
    markAxisDirty();
  else
    update();
}

//--------------------------------------------------------------------------------------------------
// Painting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the widget's scene-graph node tree. Runs on the render thread with the GUI
 *        thread blocked in the synchronization phase, which is what makes reading item state
 *        here safe; the cached node pointers are never touched outside this call.
 */
QSGNode* Widgets::Waterfall::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
  Q_UNUSED(data)

  const QRectF outerRect(0, 0, width(), height());
  if (outerRect.isEmpty() || !window() || m_releaseRenderResources) {
    delete oldNode;
    releaseRenderResources();
    return nullptr;
  }

  auto* root = oldNode;
  if (!root) {
    root = new QSGNode;
    releaseRenderResources();
  }

  SS_ASSERT(root != nullptr, return nullptr);

  root->removeAllChildNodes();
  syncBackgroundNodes(root, m_overlay.plotRect());
  m_spectrogram.sync(root, window(), m_image, computeSourceRect(), m_overlay.plotRect(), m_topRow);
  syncOverlayNode(root);

  return root;
}

/**
 * @brief Refreshes the outer and inner background quads, replacing the two fillRect calls.
 */
void Widgets::Waterfall::syncBackgroundNodes(QSGNode* root, const QRectF& plotRect)
{
  SS_ASSERT(root != nullptr, return);

  const QRectF outerRect(0, 0, width(), height());
  if (!m_outerBgNode) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending transfers ownership to
    // the root; the slots are re-nulled whenever updatePaintNode is handed a null oldNode.
    m_outerBgNode = new QSGSimpleRectNode;
    // code-verify on
  }

  m_outerBgNode->setRect(outerRect);
  m_outerBgNode->setColor(m_overlay.outerBackground());
  root->appendChildNode(m_outerBgNode);

  if (plotRect.isEmpty())
    return;

  if (!m_innerBgNode) {
    // code-verify off
    // Owned by the root once appended, exactly as the outer background node is.
    m_innerBgNode = new QSGSimpleRectNode;
    // code-verify on
  }

  m_innerBgNode->setRect(plotRect);
  m_innerBgNode->setColor(m_overlay.innerBackground());
  root->appendChildNode(m_innerBgNode);
}

/**
 * @brief Forgets every cached scene-graph pointer after the tree they belonged to was destroyed,
 *        and re-arms a full upload. Never deletes: the nodes are the root's children, so the root
 *        already took them with it.
 */
void Widgets::Waterfall::releaseRenderResources()
{
  m_outerBgNode = nullptr;
  m_innerBgNode = nullptr;
  m_overlayNode = nullptr;
  m_spectrogram.forgetNodes();
  m_releaseRenderResources = false;
}

/**
 * @brief Draws the cached overlay layer as one textured quad, re-uploading it only when the
 *        layer was rasterized again.
 */
void Widgets::Waterfall::syncOverlayNode(QSGNode* root)
{
  SS_ASSERT(root != nullptr, return);

  bool upload = m_overlay.consumeUpload();
  if (m_overlay.layer().isNull()) {
    delete m_overlayNode;
    m_overlayNode = nullptr;
    return;
  }

  if (!m_overlayNode) {
    m_overlayNode = new QSGSimpleTextureNode;
    m_overlayNode->setOwnsTexture(true);
    m_overlayNode->setFiltering(QSGTexture::Linear);
    upload = true;
  }

  if (upload)
    m_overlayNode->setTexture(window()->createTextureFromImage(m_overlay.layer()));

  m_overlayNode->setRect(QRectF(0, 0, width(), height()));
  root->appendChildNode(m_overlayNode);
}

/**
 * @brief Rasterizes the overlay layer on the GUI thread ahead of the scene-graph sync. The
 *        layer uses QPainter, fonts and application singletons, none of which may be touched
 *        from updatePaintNode's render-thread context.
 */
void Widgets::Waterfall::updatePolish()
{
  if (!m_overlay.dirty())
    return;

  const qreal dpr = (window() ? window()->devicePixelRatio() : 1.0);
  m_overlay.render(QSizeF(width(), height()), dpr);
}

/**
 * @brief Marks the axis overlay as needing a re-render and schedules a repaint.
 */
void Widgets::Waterfall::markAxisDirty()
{
  m_overlay.markDirty();
  polish();
  update();
}

//--------------------------------------------------------------------------------------------------
// Layout helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the visible source rectangle inside m_image, given zoom/pan.
 */
QRectF Widgets::Waterfall::computeSourceRect() const
{
  if (m_image.isNull())
    return QRectF();

  return m_view.sourceRect(m_image.width(), m_image.height());
}

//--------------------------------------------------------------------------------------------------
// View state (zoom / pan / axis visibility)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the axes are user-enabled.
 */
bool Widgets::Waterfall::axisVisible() const noexcept
{
  return m_view.axisVisible();
}

/**
 * @brief Returns the current X (frequency) zoom factor -- 1.0 = full Nyquist sweep.
 */
double Widgets::Waterfall::xZoom() const noexcept
{
  return m_view.xZoom();
}

/**
 * @brief Returns the current Y (time) zoom factor -- 1.0 = full history visible.
 */
double Widgets::Waterfall::yZoom() const noexcept
{
  return m_view.yZoom();
}

/**
 * @brief Returns the X pan offset in [-0.5, 0.5] of the data range.
 */
double Widgets::Waterfall::xPan() const noexcept
{
  return m_view.xPan();
}

/**
 * @brief Returns the Y pan offset in [-0.5, 0.5] of the history range.
 */
double Widgets::Waterfall::yPan() const noexcept
{
  return m_view.yPan();
}

/**
 * @brief Convenience flag -- true when zoom is unity and pan is zero on both axes.
 */
bool Widgets::Waterfall::atDefaultView() const noexcept
{
  return m_view.atDefaultView();
}

/**
 * @brief Toggles the axes (ticks, grid, labels) on/off. Triggers a repaint.
 */
void Widgets::Waterfall::setAxisVisible(const bool enabled)
{
  if (!m_view.setAxisVisible(enabled))
    return;

  Q_EMIT axisVisibleChanged();
  markAxisDirty();
}

/**
 * @brief Returns whether the freq/time hover cursor is enabled.
 */
bool Widgets::Waterfall::cursorEnabled() const noexcept
{
  return m_view.cursorEnabled();
}

/**
 * @brief Enables or disables the freq/time hover cursor overlay.
 */
void Widgets::Waterfall::setCursorEnabled(const bool enabled)
{
  if (!m_view.setCursorEnabled(enabled))
    return;

  Q_EMIT cursorEnabledChanged();
  markAxisDirty();
}

/**
 * @brief Returns whether the frequency-marker overlay is visible.
 */
bool Widgets::Waterfall::markersVisible() const noexcept
{
  return m_view.markersVisible();
}

/**
 * @brief Toggles the frequency-marker overlay on/off; hiding also clears the spotlight and
 *        the stale chip hit rects.
 */
void Widgets::Waterfall::setMarkersVisible(const bool enabled)
{
  if (!m_view.setMarkersVisible(enabled))
    return;

  if (!enabled)
    m_overlay.clearSpotlight();

  Q_EMIT markersVisibleChanged();
  markAxisDirty();
}

/**
 * @brief Returns whether the side colorbar legend is visible.
 */
bool Widgets::Waterfall::colorbarVisible() const noexcept
{
  return m_view.colorbarVisible();
}

/**
 * @brief Toggles the colorbar legend on/off.
 */
void Widgets::Waterfall::setColorbarVisible(const bool enabled)
{
  if (!m_view.setColorbarVisible(enabled))
    return;

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
    cfg.uniqueId     = dataset.uniqueId;
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
  if (!m_view.zoomBy(factor, anchorX, anchorY))
    return;

  Q_EMIT viewChanged();
  markAxisDirty();
}

/**
 * @brief Translates the view by (normDx, normDy) -- both in normalized item-rect
 *        coordinates (e.g. 0.1 = 10% of the visible plot width/height).
 */
void Widgets::Waterfall::panBy(double normDx, double normDy)
{
  if (!m_view.panBy(normDx, normDy))
    return;

  Q_EMIT viewChanged();
  markAxisDirty();
}

/**
 * @brief Restores zoom = 1.0 and pan = 0 on both axes.
 */
void Widgets::Waterfall::resetView()
{
  if (!m_view.resetView())
    return;

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

  if (m_view.markersVisible() && m_overlay.hasMarkers()) {
    const int hit = m_overlay.markerChipAt(event->position());
    if (hit >= 0 && m_overlay.toggleSpotlight(hit)) {
      markAxisDirty();
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
  QQuickItem::geometryChange(newGeom, oldGeom);
  if (newGeom.size() != oldGeom.size())
    markAxisDirty();
}

/**
 * @brief Releases the spectrogram image and every GPU resource when the widget leaves the screen,
 *        rebuilding both lazily when it returns (R15.1). A hidden waterfall already skipped its
 *        FFT but kept a multi-megabyte image and a live texture; the scene graph frees the texture
 *        on the render thread at the updatePaintNode this update() schedules.
 */
void Widgets::Waterfall::itemChange(ItemChange change, const ItemChangeData& value)
{
  QQuickItem::itemChange(change, value);

  if (change != ItemVisibleHasChanged)
    return;

  if (!value.boolValue) {
    releaseHistoryImage();
    m_releaseRenderResources = true;
  }

  update();
}

/**
 * @brief Records the entering pointer position so the cursor draws immediately.
 */
void Widgets::Waterfall::hoverEnterEvent(QHoverEvent* event)
{
  m_overlay.setCursorPosition(event->position(), true);
  if (m_view.cursorEnabled())
    markAxisDirty();

  event->accept();
}

/**
 * @brief Tracks the pointer, hints chip clickability with a pointing-hand cursor, and only
 *        schedules a repaint when the readout cursor is on.
 */
void Widgets::Waterfall::hoverMoveEvent(QHoverEvent* event)
{
  const QPointF pos = event->position();
  m_overlay.setCursorPosition(pos, true);

  if (!m_dragging) {
    if (m_view.markersVisible() && m_overlay.markerChipAt(pos) >= 0)
      setCursor(Qt::PointingHandCursor);
    else
      unsetCursor();
  }

  if (m_view.cursorEnabled())
    markAxisDirty();

  event->accept();
}

/**
 * @brief Clears the cursor when the pointer leaves the item.
 */
void Widgets::Waterfall::hoverLeaveEvent(QHoverEvent* event)
{
  m_overlay.setCursorPosition(QPointF(), false);
  if (m_view.cursorEnabled())
    markAxisDirty();

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
  m_overlay.refreshTheme();
  markAxisDirty();
}

/**
 * @brief Forces an axis re-render when the dashboard widget-font scale changes.
 */
void Widgets::Waterfall::onFontsChanged()
{
  markAxisDirty();
}
