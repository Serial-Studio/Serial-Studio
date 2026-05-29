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

#include "UI/Widgets/MultiPlot.h"

#include "DSP.h"
#include "Misc/ThemeManager.h"
#include "UI/Dashboard.h"

/**
 * @brief Returns the shared simplified unit of every dataset in @p group, or
 *        an empty string when datasets disagree or the first unit is empty.
 */
static QString sharedDatasetUnit(const DataModel::Group& group)
{
  if (group.datasets.empty())
    return {};

  const auto firstUnit = group.datasets[0].units.simplified();
  if (firstUnit.isEmpty())
    return {};

  for (size_t i = 1; i < group.datasets.size(); ++i)
    if (group.datasets[i].units.simplified() != firstUnit)
      return {};

  return firstUnit;
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a MultiPlot widget.
 */
Widgets::MultiPlot::MultiPlot(const int index, QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(index)
  , m_dataW(0)
  , m_dataH(0)
  , m_minX(0)
  , m_maxX(0)
  , m_minY(0)
  , m_maxY(0)
  , m_timeAxis(false)
  , m_interpolationMode(SerialStudio::InterpolationLinear)
  , m_sweepEnabled(false)
  , m_triggerLevel(0)
  , m_holdoffMs(0)
  , m_timebaseMs(0)
  , m_triggerSource(0)
  , m_sweepMode(SerialStudio::SweepAuto)
  , m_triggerEdge(SerialStudio::TriggerRising)
{
  // Validate dashboard configuration
  if (!VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
    return;

  // Obtain min/max values from datasets
  const auto& group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);
  m_minY            = std::numeric_limits<double>::max();
  m_maxY            = std::numeric_limits<double>::lowest();

  // Populate data from datasets
  for (size_t i = 0; i < group.datasets.size(); ++i) {
    const auto& dataset = group.datasets[i];

    m_drawOrders.append(i);
    m_visibleCurves.append(true);
    m_labels.append(dataset.title);
    m_minY = qMin(m_minY, qMin(dataset.pltMin, dataset.pltMax));
    m_maxY = qMax(m_maxY, qMax(dataset.pltMin, dataset.pltMax));
  }

  // Obtain group title, appending the shared unit if all datasets agree
  m_yLabel                 = group.title;
  const QString sharedUnit = sharedDatasetUnit(group);
  if (!sharedUnit.isEmpty())
    m_yLabel += " (" + sharedUnit + ")";

  // Resolve X-axis label (Quick Plot time preference or a Time-set dataset)
  m_timeAxis = UI::Dashboard::instance().useTimeXAxisGroup(group);
  m_xLabel   = m_timeAxis ? tr("Time (s)") : tr("Samples");

  // Resize data container to fit curves
  m_data.resize(group.datasets.size());

  // Connect to the dashboard signals
  connect(&UI::Dashboard::instance(), &UI::Dashboard::pointsChanged, this, &MultiPlot::updateRange);
  connect(&UI::Dashboard::instance(),
          &UI::Dashboard::plotTimeRangeChanged,
          this,
          &MultiPlot::updateRange);

  // Connect to the theme manager to update the curve colors
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &MultiPlot::onThemeChanged);

  calculateAutoScaleRange();
  updateRange();
}

//--------------------------------------------------------------------------------------------------
// Dimension getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of datasets in the multiplot.
 */
int Widgets::MultiPlot::count() const noexcept
{
  return m_data.count();
}

/**
 * @brief Returns the size of the down-sampled X axis data.
 */
int Widgets::MultiPlot::dataW() const noexcept
{
  return m_dataW;
}

/**
 * @brief Returns the size of the down-sampled Y axis data.
 */
int Widgets::MultiPlot::dataH() const noexcept
{
  return m_dataH;
}

//--------------------------------------------------------------------------------------------------
// Axis range getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the minimum X-axis value.
 */
double Widgets::MultiPlot::minX() const noexcept
{
  return m_minX;
}

/**
 * @brief Returns the maximum X-axis value.
 */
double Widgets::MultiPlot::maxX() const noexcept
{
  return m_maxX;
}

/**
 * @brief Returns the minimum Y-axis value.
 */
double Widgets::MultiPlot::minY() const noexcept
{
  return m_minY;
}

/**
 * @brief Returns the maximum Y-axis value.
 */
double Widgets::MultiPlot::maxY() const noexcept
{
  return m_maxY;
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks whether plot data updates are currently active.
 */
bool Widgets::MultiPlot::running() const noexcept
{
  return UI::Dashboard::instance().multiplotRunning(m_index);
}

/**
 * @brief Returns the current interpolation mode.
 */
SerialStudio::InterpolationMode Widgets::MultiPlot::interpolationMode() const noexcept
{
  return m_interpolationMode;
}

//--------------------------------------------------------------------------------------------------
// Metadata getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Y-axis label.
 */
const QString& Widgets::MultiPlot::yLabel() const noexcept
{
  return m_yLabel;
}

/**
 * @brief Returns the X-axis label ("Samples" or "Time (s)").
 */
const QString& Widgets::MultiPlot::xLabel() const noexcept
{
  return m_xLabel;
}

/**
 * @brief Returns true when this multiplot renders against a time (seconds-ago) X-axis.
 */
bool Widgets::MultiPlot::timeAxis() const noexcept
{
  return m_timeAxis;
}

/**
 * @brief Returns the colors of the datasets.
 */
const QStringList& Widgets::MultiPlot::colors() const noexcept
{
  return m_colors;
}

/**
 * @brief Returns the labels of the datasets.
 */
const QStringList& Widgets::MultiPlot::labels() const noexcept
{
  return m_labels;
}

/**
 * @brief Returns the visibility state of all curves.
 */
const QList<bool>& Widgets::MultiPlot::visibleCurves() const noexcept
{
  return m_visibleCurves;
}

//--------------------------------------------------------------------------------------------------
// Sweep / trigger getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether sweep/trigger mode is active.
 */
bool Widgets::MultiPlot::sweepEnabled() const noexcept
{
  return m_sweepEnabled;
}

/**
 * @brief Returns the trigger level on the trigger-source curve.
 */
double Widgets::MultiPlot::triggerLevel() const noexcept
{
  return m_triggerLevel;
}

/**
 * @brief Returns the trigger holdoff in milliseconds.
 */
double Widgets::MultiPlot::holdoff() const noexcept
{
  return m_holdoffMs;
}

/**
 * @brief Returns the per-sweep timebase in milliseconds; 0 means match time range.
 */
double Widgets::MultiPlot::sweepTimebase() const noexcept
{
  return m_timebaseMs;
}

/**
 * @brief Returns the curve index used as the trigger source.
 */
int Widgets::MultiPlot::triggerSource() const noexcept
{
  return m_triggerSource;
}

/**
 * @brief Returns the active sweep mode (auto/normal/single).
 */
SerialStudio::SweepMode Widgets::MultiPlot::sweepMode() const noexcept
{
  return m_sweepMode;
}

/**
 * @brief Returns the trigger edge polarity.
 */
SerialStudio::TriggerEdge Widgets::MultiPlot::triggerEdge() const noexcept
{
  return m_triggerEdge;
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the data on the given QLineSeries.
 */
void Widgets::MultiPlot::draw(QXYSeries* series, const int index)
{
  if (!series || index < 0 || index >= count() || !m_visibleCurves[index])
    return;

  const auto& source         = m_data[index];
  const QList<QPointF>* data = &source;
  const int n                = source.size();

  if (m_interpolationMode == SerialStudio::InterpolationZoh && n >= 2) {
    m_renderData.resize(2 * n - 1);
    QPointF* out      = m_renderData.data();
    const QPointF* in = source.constData();
    out[0]            = in[0];
    for (int i = 1; i < n; ++i) {
      out[2 * i - 1] = QPointF(in[i].x(), in[i - 1].y());
      out[2 * i]     = in[i];
    }
    data = &m_renderData;
  }

  else if (m_interpolationMode == SerialStudio::InterpolationStem && n > 0) {
    constexpr double kNan = std::numeric_limits<double>::quiet_NaN();
    const double base     = (m_minY < 0.0 && m_maxY > 0.0) ? 0.0 : m_minY;

    m_renderData.resize(3 * n);
    QPointF* out      = m_renderData.data();
    const QPointF* in = source.constData();
    for (int i = 0; i < n; ++i) {
      out[3 * i]     = in[i];
      out[3 * i + 1] = QPointF(in[i].x(), base);
      out[3 * i + 2] = QPointF(kNan, kNan);
    }
    data = &m_renderData;
  }

  series->replace(*data);
  Q_EMIT series->update();
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the size of the down-sampled X axis data.
 */
void Widgets::MultiPlot::setDataW(const int width)
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
void Widgets::MultiPlot::setDataH(const int height)
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
void Widgets::MultiPlot::setRunning(const bool enabled)
{
  UI::Dashboard::instance().setMultiplotRunning(m_index, enabled);
  Q_EMIT runningChanged();
}

/**
 * @brief Updates the interpolation mode used by the multiplot.
 */
void Widgets::MultiPlot::setInterpolationMode(SerialStudio::InterpolationMode mode)
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
void Widgets::MultiPlot::setSweepEnabled(const bool enabled)
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
void Widgets::MultiPlot::setTriggerLevel(const double level)
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
void Widgets::MultiPlot::setHoldoff(const double milliseconds)
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
void Widgets::MultiPlot::setSweepTimebase(const double milliseconds)
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
 * @brief Selects which curve drives the trigger.
 */
void Widgets::MultiPlot::setTriggerSource(const int curve)
{
  const int clamped = (curve < 0) ? 0 : (curve >= count() ? count() - 1 : curve);
  if (m_triggerSource == clamped)
    return;

  m_triggerSource = clamped;
  pushSweepConfig();
  Q_EMIT sweepChanged();
}

/**
 * @brief Updates the sweep mode (auto/normal/single).
 */
void Widgets::MultiPlot::setSweepMode(const SerialStudio::SweepMode mode)
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
void Widgets::MultiPlot::setTriggerEdge(const SerialStudio::TriggerEdge edge)
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
void Widgets::MultiPlot::armSweep()
{
  UI::Dashboard::instance().armMultiplotSweep(m_index);
}

/**
 * @brief Pushes the current trigger configuration into the Dashboard engine.
 */
void Widgets::MultiPlot::pushSweepConfig()
{
  UI::Dashboard::instance().setMultiplotSweep(m_index,
                                              m_sweepEnabled,
                                              m_triggerLevel,
                                              static_cast<int>(m_triggerEdge),
                                              static_cast<int>(m_sweepMode),
                                              m_holdoffMs * 0.001,
                                              m_triggerSource,
                                              m_timebaseMs * 0.001);
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the data of the multiplot.
 */
void Widgets::MultiPlot::updateData()
{
  // Share workspace data
  static thread_local DSP::DownsampleWorkspace ws;

  // Stop if widget is disabled or invalid
  if (!isEnabled() || !VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
    return;

  // Sweep mode: decimate each curve's held sweep over the shared [0, T] window
  if (m_timeAxis && m_sweepEnabled) {
    const auto& engine        = UI::Dashboard::instance().multiplotSweep(m_index);
    const qsizetype plotCount = static_cast<qsizetype>(engine.front.size());
    if (m_data.size() != plotCount)
      m_data.resize(plotCount);

    for (qsizetype i = 0; i < plotCount; ++i) {
      if (i >= m_visibleCurves.size() || !m_visibleCurves[i])
        continue;

      const auto& ring = engine.display(static_cast<size_t>(i));
      (void)DSP::downsampleWindowAbsolute(
        ring.time, ring.value, m_minX, m_maxX, m_dataW, m_dataH, m_data[i], &ws);
    }

    calculateAutoScaleRange();
    return;
  }

  // Time axis: decimate each curve's visible window from its decimating ring
  if (m_timeAxis) {
    const auto& rings         = UI::Dashboard::instance().multiplotTimeRings(m_index);
    const qsizetype plotCount = static_cast<qsizetype>(rings.size());
    if (m_data.size() != plotCount)
      m_data.resize(plotCount);

    for (qsizetype i = 0; i < plotCount; ++i) {
      if (i >= m_visibleCurves.size() || !m_visibleCurves[i])
        continue;

      const auto& ring = rings[static_cast<size_t>(i)];
      (void)DSP::downsampleTimeWindow(
        ring.time, ring.value, m_minX, m_maxX, m_dataW, m_dataH, m_data[i], &ws);
    }

    calculateAutoScaleRange();
    return;
  }

  // Fetch multiplot source data (shared X axis, multiple Y series)
  const auto& data = UI::Dashboard::instance().multiplotData(m_index);
  const auto& X    = *data.x;

  // One QVector<QPointF> per series; resize only when count changes
  const qsizetype plotCount = data.y.size();
  if (m_data.size() != plotCount) {
    // Squeeze only when shrinking by >20% to avoid thrashing
    if (m_data.size() > plotCount && m_data.size() > plotCount * 1.2) {
      m_data.clear();
      m_data.squeeze();
    }
    m_data.resize(plotCount);
  }

  // Populate data for each plot
  for (qsizetype i = 0; i < plotCount; ++i) {
    // Skip if curve is not visible or out of bounds
    if (i >= m_visibleCurves.size() || !m_visibleCurves[i])
      continue;

    DSP::downsampleMonotonic(X, data.y[i], m_dataW, m_dataH, m_data[i], &ws);
  }

  // Calculate auto scale range
  calculateAutoScaleRange();
}

/**
 * @brief Updates the range of the multiplot.
 */
void Widgets::MultiPlot::updateRange()
{
  // Validate widget and reset X-axis range for the new point count
  if (!VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
    return;

  // Reset data containers and update X-axis range
  const auto& data = UI::Dashboard::instance().multiplotData(m_index);
  m_data.clear();
  m_data.squeeze();
  m_data.resize(data.y.size());

  if (m_timeAxis && m_sweepEnabled) {
    const double range    = UI::Dashboard::instance().plotTimeRange();
    const double timebase = m_timebaseMs * 0.001;
    m_minX                = 0;
    m_maxX                = (timebase > 0 && timebase < range) ? timebase : range;
  }

  else if (m_timeAxis) {
    m_minX = -UI::Dashboard::instance().plotTimeRange();
    m_maxX = 0;
  }

  else {
    m_minX = 0;
    m_maxX = UI::Dashboard::instance().points();
  }

  Q_EMIT rangeChanged();
}

/**
 * @brief Calculates the auto scale range of the multiplot.
 */
void Widgets::MultiPlot::calculateAutoScaleRange()
{
  const auto prevMinY = m_minY;
  const auto prevMaxY = m_maxY;

  // If the data is empty, set the range to 0-1
  if (m_data.isEmpty()) {
    m_minY = 0;
    m_maxY = 1;
  }

  // Try dataset-declared bounds; fall back to scanning curves on failure
  else if (!computeRangeFromDatasets()) {
    scanCurvesForRange();
    padDerivedRange();
  }

  if (DSP::notEqual(prevMinY, m_minY) || DSP::notEqual(prevMaxY, m_maxY))
    Q_EMIT rangeChanged();
}

/**
 * @brief Computes Y range from dataset pltMin/pltMax bounds.
 */
bool Widgets::MultiPlot::computeRangeFromDatasets()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index))
    return false;

  const auto& group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);
  m_minY            = std::numeric_limits<double>::max();
  m_maxY            = std::numeric_limits<double>::lowest();

  int index = 0;
  for (const auto& dataset : group.datasets) {
    const bool curveOk = DSP::notEqual(dataset.pltMin, dataset.pltMax)
                      && index < m_visibleCurves.size() && m_visibleCurves[index];
    if (!curveOk)
      return false;

    m_minY = qMin(m_minY, qMin(dataset.pltMin, dataset.pltMax));
    m_maxY = qMax(m_maxY, qMax(dataset.pltMin, dataset.pltMax));
    ++index;
  }

  return true;
}

/**
 * @brief Scans visible curves for finite min/max values.
 */
void Widgets::MultiPlot::scanCurvesForRange()
{
  m_minY = std::numeric_limits<double>::max();
  m_maxY = std::numeric_limits<double>::lowest();

  auto accumulate = [this](const QList<QPointF>& curve) {
    for (auto i = 0; i < curve.count(); ++i) {
      const double value = curve[i].y();
      if (!std::isfinite(value))
        continue;

      m_minY = qMin(m_minY, value);
      m_maxY = qMax(m_maxY, value);
    }
  };

  int index = 0;
  for (const auto& curve : std::as_const(m_data)) {
    if (index < m_visibleCurves.size() && m_visibleCurves[index])
      accumulate(curve);

    ++index;
  }
}

/**
 * @brief Adjusts m_minY/m_maxY when data-derived bounds need padding.
 */
void Widgets::MultiPlot::padDerivedRange()
{
  applyDerivedYBounds();

  // Round to integer numbers
  m_maxY = std::ceil(m_maxY);
  m_minY = std::floor(m_minY);
  if (DSP::almostEqual(m_maxY, m_minY)) {
    m_minY -= 1;
    m_maxY += 1;
  }
}

/**
 * @brief Selects the padding strategy for the current m_minY/m_maxY pair.
 */
void Widgets::MultiPlot::applyDerivedYBounds()
{
  // If no finite values found, use default range
  if (!std::isfinite(m_minY) || !std::isfinite(m_maxY)) {
    m_minY = 0;
    m_maxY = 1;
    return;
  }

  // If the min and max are the same at zero, fall back to [-1, 1]
  if (DSP::almostEqual(m_minY, m_maxY) && DSP::isZero(m_minY)) {
    m_minY = -1;
    m_maxY = 1;
    return;
  }

  // If min and max are equal but non-zero, expand by 10% of |min|
  if (DSP::almostEqual(m_minY, m_maxY)) {
    const double absValue = qAbs(m_minY);
    m_minY                = m_minY - absValue * 0.1;
    m_maxY                = m_maxY + absValue * 0.1;
    return;
  }

  // Expand range symmetrically around midY, with a 10% padding
  const double midY      = (m_minY + m_maxY) * 0.5;
  const double halfRange = (m_maxY - m_minY) * 0.5;

  double paddedRange = halfRange * 1.1;
  if (DSP::isZero(paddedRange))
    paddedRange = 1;

  m_minY = std::floor(midY - paddedRange);
  m_maxY = std::ceil(midY + paddedRange);

  // Safety check to avoid zero-range
  if (DSP::almostEqual(m_minY, m_maxY)) {
    m_minY -= 1;
    m_maxY += 1;
  }
}

//--------------------------------------------------------------------------------------------------
// Curve management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Modifies the visibility state of a specific curve in the multi-plot.
 */
void Widgets::MultiPlot::modifyCurveVisibility(const int index, const bool visible)
{
  // Update visibility flag and move the curve to the end of the draw order
  if (index >= 0 && index < m_visibleCurves.count()) {
    m_visibleCurves[index] = visible;
    if (visible) {
      m_drawOrders.removeAll(index);
      m_drawOrders.append(index);
    }

    Q_EMIT curvesChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Theme management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the theme of the multiplot.
 */
void Widgets::MultiPlot::onThemeChanged()
{
  // Rebuild per-dataset colors from the current theme
  if (VALIDATE_WIDGET(SerialStudio::DashboardMultiPlot, m_index)) {
    const auto& group = GET_GROUP(SerialStudio::DashboardMultiPlot, m_index);

    m_colors.clear();
    m_colors.resize(group.datasets.size());
    for (size_t i = 0; i < group.datasets.size(); ++i) {
      const auto& dataset = group.datasets[i];
      const auto color    = SerialStudio::getDatasetColor(dataset.index);
      m_colors[i]         = color.name();
    }

    Q_EMIT themeChanged();
    Q_EMIT curvesChanged();
  }
}
