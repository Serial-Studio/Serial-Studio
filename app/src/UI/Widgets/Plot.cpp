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

#include "UI/Widgets/Plot.h"

#include "DSP.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Plot widget.
 */
Widgets::Plot::Plot(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_dataW(0)
  , m_dataH(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_dataMinY(0)
  , m_dataMaxY(0)
  , m_visLoX(std::numeric_limits<double>::quiet_NaN())
  , m_visHiX(std::numeric_limits<double>::quiet_NaN())
  , m_monotonicData(true)
  , m_timeAxis(false)
  , m_interpolationMode(SerialStudio::InterpolationLinear)
  , m_sweepEnabled(false)
  , m_triggerLevel(0)
  , m_holdoffMs(0)
  , m_timebaseMs(0)
  , m_sweepMode(SerialStudio::SweepAuto)
  , m_triggerEdge(SerialStudio::TriggerRising)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index)) {
    const auto& yDataset = GET_DATASET(SerialStudio::DashboardPlot, m_index);

    m_minY = qMin(yDataset.pltMin, yDataset.pltMax);
    m_maxY = qMax(yDataset.pltMin, yDataset.pltMax);

    resolveXAxis(yDataset);

    m_yLabel = yDataset.title;
    if (!yDataset.units.isEmpty())
      m_yLabel += " (" + yDataset.units + ")";

    connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this, &Plot::updateRange);
    connect(
      &UI::Dashboard::instance(), &UI::Dashboard::plotTimeRangeChanged, this, &Plot::updateRange);

    calculateAutoScaleRange();
    updateRange();
  }
}

/**
 * @brief Resolves the X-axis mode, label, and monotonicity for the plot.
 */
void Widgets::Plot::resolveXAxis(const DataModel::Dataset& yDataset)
{
  if (UI::Dashboard::instance().useTimeXAxis(yDataset)) {
    m_timeAxis      = true;
    m_monotonicData = true;
    m_xLabel        = tr("Time (s)");
    return;
  }

#ifdef BUILD_COMMERCIAL
  const auto& tk = Licensing::CommercialToken::current();
  const auto xAxisId =
    (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Trial)
      ? yDataset.xAxisId
      : -1;
#else
  const auto xAxisId = -1;
#endif

  const auto& datasets = UI::Dashboard::instance().datasets();
  if (datasets.contains(xAxisId)) {
    m_monotonicData      = false;
    const auto& xDataset = datasets[xAxisId];
    m_xLabel             = xDataset.title;
    if (!xDataset.units.isEmpty())
      m_xLabel += " (" + xDataset.units + ")";

    return;
  }

  m_monotonicData = true;
  m_xLabel        = tr("Samples");
}

//--------------------------------------------------------------------------------------------------
// Data dimension getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the size of the down-sampled X axis data.
 */
int Widgets::Plot::dataW() const noexcept
{
  return m_dataW;
}

/**
 * @brief Returns the size of the down-sampled Y axis data.
 */
int Widgets::Plot::dataH() const noexcept
{
  return m_dataH;
}

//--------------------------------------------------------------------------------------------------
// Axis range getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the minimum X-axis value.
 */
double Widgets::Plot::minX() const noexcept
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 */
double Widgets::Plot::maxX() const noexcept
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 */
double Widgets::Plot::minY() const noexcept
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 */
double Widgets::Plot::maxY() const noexcept
{
  return m_maxY;
}

/**
 * @brief Reports whether the real samples straddle zero, so the area fill anchors at
 *        the zero line; a configured axis range that merely spans zero does not count.
 */
bool Widgets::Plot::dataBipolar() const noexcept
{
  return m_dataMinY < 0.0 && m_dataMaxY > 0.0;
}

/**
 * @brief Returns the largest finite sample value, used to anchor a strictly-negative
 *        fill from the range top instead of the range floor.
 */
double Widgets::Plot::dataMaxY() const noexcept
{
  return m_dataMaxY;
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether plot data updates are currently active.
 */
bool Widgets::Plot::running() const noexcept
{
  return UI::Dashboard::instance().plotRunning(m_index);
}

/**
 * @brief Returns the current interpolation mode.
 */
SerialStudio::InterpolationMode Widgets::Plot::interpolationMode() const noexcept
{
  return m_interpolationMode;
}

//--------------------------------------------------------------------------------------------------
// Label getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Y-axis label.
 */
const QString& Widgets::Plot::yLabel() const noexcept
{
  return m_yLabel;
}

/**
 * @brief Returns the X-axis label.
 */
const QString& Widgets::Plot::xLabel() const noexcept
{
  return m_xLabel;
}

/**
 * @brief Returns true when this plot renders against a time (seconds-ago) X-axis.
 */
bool Widgets::Plot::timeAxis() const noexcept
{
  return m_timeAxis;
}

/**
 * @brief Returns true when this plot renders against an X dataset (XY mode).
 */
bool Widgets::Plot::xyPlot() const noexcept
{
  return !m_timeAxis && !m_monotonicData;
}

//--------------------------------------------------------------------------------------------------
// Sweep / trigger getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether sweep/trigger mode is active.
 */
bool Widgets::Plot::sweepEnabled() const noexcept
{
  return m_sweepEnabled;
}

/**
 * @brief Returns the trigger level on the plotted value.
 */
double Widgets::Plot::triggerLevel() const noexcept
{
  return m_triggerLevel;
}

/**
 * @brief Returns the trigger holdoff in milliseconds.
 */
double Widgets::Plot::holdoff() const noexcept
{
  return m_holdoffMs;
}

/**
 * @brief Returns the per-sweep timebase in milliseconds; 0 means match time range.
 */
double Widgets::Plot::sweepTimebase() const noexcept
{
  return m_timebaseMs;
}

/**
 * @brief Returns the active sweep mode (auto/normal/single).
 */
SerialStudio::SweepMode Widgets::Plot::sweepMode() const noexcept
{
  return m_sweepMode;
}

/**
 * @brief Returns the trigger edge polarity.
 */
SerialStudio::TriggerEdge Widgets::Plot::triggerEdge() const noexcept
{
  return m_triggerEdge;
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the data on the given QLineSeries.
 */
void Widgets::Plot::draw(QXYSeries* series)
{
  if (series) {
    updateData();
    calculateAutoScaleRange();

    const auto* data = &m_data;
    if (m_monotonicData
        && (m_interpolationMode == SerialStudio::InterpolationZoh
            || m_interpolationMode == SerialStudio::InterpolationStem)) {
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
void Widgets::Plot::setDataW(const int width)
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
void Widgets::Plot::setDataH(const int height)
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
void Widgets::Plot::setRunning(const bool enabled)
{
  UI::Dashboard::instance().setPlotRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Stores the visible X window pushed by the view (zoom/pan slice). Time-axis
 *        draws downsample only this window at screen resolution, so zooming in scans
 *        fewer samples instead of re-bucketing the full range at zoom resolution.
 */
void Widgets::Plot::setVisibleXWindow(const double lo, const double hi)
{
  m_visLoX = lo;
  m_visHiX = hi;
}

/**
 * @brief Updates the interpolation mode used by the plot.
 */
void Widgets::Plot::setInterpolationMode(SerialStudio::InterpolationMode mode)
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
// Sweep / trigger setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables sweep/trigger mode and flips the X-axis window.
 */
void Widgets::Plot::setSweepEnabled(const bool enabled)
{
  if (m_sweepEnabled == enabled)
    return;

  m_sweepEnabled = enabled;
  pushSweepConfig();
  updateRange();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the trigger level.
 */
void Widgets::Plot::setTriggerLevel(const double level)
{
  if (qFuzzyCompare(m_triggerLevel, level))
    return;

  m_triggerLevel = level;
  pushSweepConfig();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the trigger holdoff in milliseconds.
 */
void Widgets::Plot::setHoldoff(const double milliseconds)
{
  const double clamped = milliseconds < 0 ? 0 : milliseconds;
  if (qFuzzyCompare(m_holdoffMs, clamped))
    return;

  m_holdoffMs = clamped;
  pushSweepConfig();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the per-sweep timebase in milliseconds and reflows the X-axis.
 */
void Widgets::Plot::setSweepTimebase(const double milliseconds)
{
  const double clamped = milliseconds < 0 ? 0 : milliseconds;
  if (qFuzzyCompare(m_timebaseMs, clamped))
    return;

  m_timebaseMs = clamped;
  pushSweepConfig();
  updateRange();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the sweep mode (auto/normal/single).
 */
void Widgets::Plot::setSweepMode(const SerialStudio::SweepMode mode)
{
  if (m_sweepMode == mode)
    return;

  m_sweepMode = mode;
  pushSweepConfig();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the trigger edge polarity.
 */
void Widgets::Plot::setTriggerEdge(const SerialStudio::TriggerEdge edge)
{
  if (m_triggerEdge == edge)
    return;

  m_triggerEdge = edge;
  pushSweepConfig();
  Q_EMIT sweepChanged();
}

/**
 * @brief Re-arms a single-shot capture.
 */
void Widgets::Plot::armSweep()
{
  UI::Dashboard::instance().armPlotSweep(m_index);
}

/**
 * @brief Pushes the current trigger configuration into the Dashboard engine.
 */
void Widgets::Plot::pushSweepConfig()
{
  UI::Dashboard::instance().setPlotSweep(m_index,
                                         m_sweepEnabled,
                                         m_triggerLevel,
                                         static_cast<int>(m_triggerEdge),
                                         static_cast<int>(m_sweepMode),
                                         m_holdoffMs * 0.001,
                                         m_timebaseMs * 0.001);
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the plot data from the Dashboard.
 */
void Widgets::Plot::updateData()
{
  static thread_local DSP::DownsampleWorkspace ws;

  if (!isEnabled())
    return;

  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
    return;

  if (m_timeAxis && m_sweepEnabled) {
    double xLo = m_minX;
    double xHi = m_maxX;
    clampToVisibleX(xLo, xHi);
    const auto& ring = UI::Dashboard::instance().plotSweep(m_index).display(0);
    (void)DSP::downsampleWindowAbsolute(
      ring.time, ring.value, xLo, xHi, m_dataW, m_dataH, m_data, &ws);
    return;
  }

  if (m_timeAxis) {
    double xLo = m_minX;
    double xHi = m_maxX;
    clampToVisibleX(xLo, xHi);
    const auto& ring = UI::Dashboard::instance().plotTimeRing(m_index);
    (void)DSP::downsampleTimeWindow(ring.time, ring.value, xLo, xHi, m_dataW, m_dataH, m_data, &ws);
    return;
  }

  const auto& plotData = UI::Dashboard::instance().plotData(m_index);

  if (m_monotonicData) {
    (void)DSP::downsampleMonotonic(plotData, m_dataW, m_dataH, m_data, &ws);
    return;
  }

  const auto& X = *plotData.x;
  const auto& Y = *plotData.y;

  const qsizetype count = std::min(X.size(), Y.size());
  if (m_data.size() != count)
    m_data.resize(count);

  if (X.capacity() == 0 || Y.capacity() == 0)
    return;

  QPointF* out            = m_data.data();
  const auto* xData       = X.raw();
  const auto* yData       = Y.raw();
  const std::size_t xMask = X.storageMask();
  const std::size_t yMask = Y.storageMask();
  std::size_t xIdx        = X.frontIndex();
  std::size_t yIdx        = Y.frontIndex();
  for (qsizetype i = 0; i < count; ++i) {
    out[i].setX(xData[xIdx]);
    out[i].setY(yData[yIdx]);
    xIdx = (xIdx + 1) & xMask;
    yIdx = (yIdx + 1) & yMask;
  }
}

/**
 * @brief Intersects [lo, hi] with the view-pushed visible X window when it is valid.
 *        The window grows by two render columns per side so edge-crossing segments
 *        survive the cut and the curve runs through the viewport edges.
 */
void Widgets::Plot::clampToVisibleX(double& lo, double& hi) const
{
  Q_ASSERT(lo <= hi);

  if (!std::isfinite(m_visLoX) || !std::isfinite(m_visHiX) || !(m_visLoX < m_visHiX))
    return;

  const double margin = (m_visHiX - m_visLoX) * (2.0 / std::max(2, m_dataW));
  lo                  = std::max(lo, m_visLoX - margin);
  hi                  = std::min(hi, m_visHiX + margin);
}

/**
 * @brief Rebuilds the render data for ZOH or stem interpolation modes.
 */
void Widgets::Plot::updateInterpolatedData()
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
    const double base     = dataBipolar() ? 0.0 : m_minY;

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

/**
 * @brief Updates the range of the X-axis values.
 */
void Widgets::Plot::updateRange()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index)) {
    Q_EMIT rangeChanged();
    return;
  }

  if (m_timeAxis) {
    const double range    = UI::Dashboard::instance().plotTimeRange();
    const double timebase = m_timebaseMs * 0.001;
    const double window   = (timebase > 0 && timebase < range) ? timebase : range;
    m_minX                = m_sweepEnabled ? 0 : -range;
    m_maxX                = m_sweepEnabled ? window : 0;
    Q_EMIT rangeChanged();
    return;
  }

  const auto& yD = GET_DATASET(SerialStudio::DashboardPlot, m_index);
  if (yD.xAxisId >= 0) {
    const auto& datasets = UI::Dashboard::instance().datasets();
    auto it              = datasets.find(yD.xAxisId);
    if (it != datasets.end()) {
      m_minX = it->pltMin;
      m_maxX = it->pltMax;
    }
  }

  else {
    m_minX = 0;
    m_maxX = UI::Dashboard::instance().points();
  }

  Q_EMIT rangeChanged();
}

/**
 * @brief Calculates the auto-scale range for both X and Y axes of the plot.
 */
void Widgets::Plot::calculateAutoScaleRange()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardPlot, m_index))
    return;

  bool xChanged = false;
  bool yChanged = false;

  const auto& dy = GET_DATASET(SerialStudio::DashboardPlot, m_index);
  yChanged  = computeMinMaxValues(m_minY, m_maxY, dy, true, [](const QPointF& p) { return p.y(); });
  yChanged |= updateDataExtremes(dy);

#ifdef BUILD_COMMERCIAL
  if (const auto& tk2 = Licensing::CommercialToken::current();
      !m_timeAxis && tk2.isValid() && SS_LICENSE_GUARD()
      && tk2.featureTier() >= Licensing::FeatureTier::Trial) {
    if (UI::Dashboard::instance().datasets().contains(dy.xAxisId)) {
      const auto& dx = UI::Dashboard::instance().datasets()[dy.xAxisId];
      xChanged =
        computeMinMaxValues(m_minX, m_maxX, dx, false, [](const QPointF& p) { return p.x(); });
    }
  }
#else
  if (false) {
  }
#endif

  else if (!m_timeAxis) {
    const auto points = UI::Dashboard::instance().points();

    if (m_minX != 0 || m_maxX != points) {
      m_minX   = 0;
      m_maxX   = points;
      xChanged = true;
    }
  }

  if (xChanged || yChanged)
    Q_EMIT rangeChanged();
}

/**
 * @brief Reports whether a configured plot range hard-clamps the data into one sign,
 *        so the samples cannot be bipolar and the per-frame scan can be skipped. A
 *        degenerate range (pltMin == pltMax, including both zero) auto-scales instead,
 *        so it is not a clamp.
 */
static bool signClampedRange(const DataModel::Dataset& dataset)
{
  if (!DSP::notEqual(dataset.pltMin, dataset.pltMax))
    return false;

  return (dataset.pltMin >= 0.0 && dataset.pltMax >= 0.0)
      || (dataset.pltMin <= 0.0 && dataset.pltMax <= 0.0);
}

/**
 * @brief Refreshes the finite Y extremes that drive the area fill's bipolarity from
 *        real data. A sign-clamped configured range short-circuits the per-frame scan;
 *        otherwise the current samples are swept. Returns true when the bipolar or
 *        all-negative verdict flips, so the caller refreshes the baseline binding.
 */
bool Widgets::Plot::updateDataExtremes(const DataModel::Dataset& dataset)
{
  const bool wasBipolar     = dataBipolar();
  const bool wasAllNegative = m_dataMaxY <= 0.0;

  if (signClampedRange(dataset)) {
    m_dataMinY = qMin(dataset.pltMin, dataset.pltMax);
    m_dataMaxY = qMax(dataset.pltMin, dataset.pltMax);
    return wasBipolar != dataBipolar() || wasAllNegative != (m_dataMaxY <= 0.0);
  }

  double dataMin = std::numeric_limits<double>::max();
  double dataMax = std::numeric_limits<double>::lowest();
  for (auto i = 0; i < m_data.size(); ++i) {
    const double value = m_data[i].y();
    if (std::isfinite(value)) {
      dataMin = qMin(dataMin, value);
      dataMax = qMax(dataMax, value);
    }
  }

  if (dataMin > dataMax) {
    m_dataMinY = 0.0;
    m_dataMaxY = 0.0;
  } else {
    m_dataMinY = dataMin;
    m_dataMaxY = dataMax;
  }

  return wasBipolar != dataBipolar() || wasAllNegative != (m_dataMaxY <= 0.0);
}

/**
 * @brief Computes the minimum and maximum values for a given axis of the plot.
 */
template<typename Extractor>
bool Widgets::Plot::computeMinMaxValues(double& min,
                                        double& max,
                                        const DataModel::Dataset& dataset,
                                        const bool addPadding,
                                        Extractor extractor)
{
  bool ok             = true;
  const auto prevMinY = min;
  const auto prevMaxY = max;

  if (m_data.isEmpty()) {
    min = 0;
    max = 1;
  }

  else {
    ok &= DSP::notEqual(dataset.pltMin, dataset.pltMax);
    if (ok) {
      min = qMin(dataset.pltMin, dataset.pltMax);
      max = qMax(dataset.pltMin, dataset.pltMax);
    }
  }

  if (!ok) {
    min = std::numeric_limits<double>::max();
    max = std::numeric_limits<double>::lowest();

    for (auto i = 0; i < m_data.size(); ++i) {
      const double value = extractor(m_data[i]);
      if (std::isfinite(value)) {
        min = qMin(min, value);
        max = qMax(max, value);
      }
    }

    padDerivedRange(min, max, addPadding);
  }

  return DSP::notEqual(prevMinY, min) || DSP::notEqual(prevMaxY, max);
}

/**
 * @brief Pads a data-derived [min, max] range and rounds to integer bounds.
 */
void Widgets::Plot::padDerivedRange(double& min, double& max, const bool addPadding)
{
  applyAxisPadding(min, max, addPadding);
  max = std::ceil(max);
  min = std::floor(min);
  if (DSP::almostEqual(max, min) && addPadding) {
    min -= 1;
    max += 1;
  }
}

/**
 * @brief Selects the padding strategy for a [min, max] pair before rounding.
 */
void Widgets::Plot::applyAxisPadding(double& min, double& max, const bool addPadding)
{
  if (!std::isfinite(min) || !std::isfinite(max)) {
    min = 0;
    max = 1;
    return;
  }

  if (DSP::almostEqual(min, max) && DSP::isZero(min)) {
    min = -1;
    max = 1;
    return;
  }

  if (DSP::almostEqual(min, max)) {
    const double absValue = qAbs(min);
    min                   = min - absValue * 0.1;
    max                   = max + absValue * 0.1;
    return;
  }

  if (addPadding) {
    double range  = max - min;
    min          -= range * 0.1;
    max          += range * 0.1;
  }
}
