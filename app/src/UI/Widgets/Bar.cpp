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

#include "UI/Widgets/Bar.h"

#include <QVariantMap>

#include "DataModel/Frame.h"
#include "DSP.h"
#include "UI/Dashboard.h"
#include "UI/WidgetBands.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Bar widget.
 */
Widgets::Bar::Bar(const int index, QQuickItem* parent, bool autoInitFromBarDataset)
  : QQuickItem(parent)
  , m_index(index)
  , m_displayTickCount(5)
  , m_decimalPoints(-1)
  , m_value(0.0)
  , m_minValue(0.0)
  , m_maxValue(0.0)
  , m_extremesValid(false)
  , m_minSeen(0.0)
  , m_maxSeen(0.0)
  , m_activeBandIndex(-1)
  , m_lastBandHint(-1)
  , m_dashboard(UI::Dashboard::instance())
{
  if (autoInitFromBarDataset && VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);

    m_title            = dataset.title;
    m_units            = dataset.units;
    m_displayFormat    = dataset.displayFormat;
    m_displayTickCount = dataset.displayTickCount;
    m_decimalPoints    = dataset.decimalPoints;
    m_minValue         = qMin(dataset.wgtMin, dataset.wgtMax);
    m_maxValue         = qMax(dataset.wgtMin, dataset.wgtMax);
    buildBands(dataset.alarmBands);

    connect(&m_dashboard, &UI::Dashboard::updated, this, &Bar::updateData);
  }
}

//--------------------------------------------------------------------------------------------------
// Band construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Precomputes render data for the dataset's alarm bands.
 */
void Widgets::Bar::buildBands(const std::vector<DataModel::AlarmBand>& srcBands)
{
  m_bands.clear();
  m_bandsAsVariant.clear();
  m_bands.reserve(static_cast<int>(srcBands.size()));
  m_bandsAsVariant.reserve(static_cast<int>(srcBands.size()));

  const double range = m_maxValue - m_minValue;
  for (const auto& src : srcBands) {
    const double lo = qBound(m_minValue, qMin(src.min, src.max), m_maxValue);
    const double hi = qBound(m_minValue, qMax(src.min, src.max), m_maxValue);
    if (hi <= lo)
      continue;

    BarBand band;
    band.min         = lo;
    band.max         = hi;
    band.severity    = static_cast<int>(src.severity);
    band.blink       = src.blink;
    band.customColor = src.color;
    band.label       = src.label;
    band.fracMin     = DSP::isZero(range) ? 0.0 : qBound(0.0, (lo - m_minValue) / range, 1.0);
    band.fracMax     = DSP::isZero(range) ? 0.0 : qBound(0.0, (hi - m_minValue) / range, 1.0);
    m_bands.append(band);

    QVariantMap entry;
    entry.insert(QStringLiteral("min"), band.min);
    entry.insert(QStringLiteral("max"), band.max);
    entry.insert(QStringLiteral("fracMin"), band.fracMin);
    entry.insert(QStringLiteral("fracMax"), band.fracMax);
    entry.insert(QStringLiteral("severity"), band.severity);
    entry.insert(QStringLiteral("blink"), band.blink);
    entry.insert(QStringLiteral("customColor"), band.customColor);
    entry.insert(QStringLiteral("label"), band.label);
    m_bandsAsVariant.append(entry);
  }
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when the dataset declares one or more alarm bands.
 */
bool Widgets::Bar::alarmsDefined() const noexcept
{
  return !m_bands.isEmpty();
}

/**
 * @brief True when the current value sits in a band with Warning severity or worse.
 */
bool Widgets::Bar::alarmTriggered() const noexcept
{
  return activeBandSeverity() >= 2;
}

/**
 * @brief Returns the severity (0..3) of the band the value sits in, or -1 if none.
 */
int Widgets::Bar::activeBandSeverity() const noexcept
{
  if (m_activeBandIndex < 0 || m_activeBandIndex >= m_bands.size())
    return -1;

  return m_bands[m_activeBandIndex].severity;
}

/**
 * @brief Returns the label of the active band; empty when no band is active.
 */
const QString& Widgets::Bar::activeBandLabel() const noexcept
{
  if (m_activeBandIndex < 0 || m_activeBandIndex >= m_bands.size())
    return m_emptyLabel;

  return m_bands[m_activeBandIndex].label;
}

/**
 * @brief Returns the precomputed alarm-band list as a QML-accessible QVariantList.
 */
const QVariantList& Widgets::Bar::alarmBands() const noexcept
{
  return m_bandsAsVariant;
}

/**
 * @brief True when extreme-hold is on for the dataset and at least one sample was observed.
 */
bool Widgets::Bar::extremesValid() const noexcept
{
  return m_extremesValid;
}

/**
 * @brief Returns the lowest value observed since the last data reset.
 */
double Widgets::Bar::minSeen() const noexcept
{
  return m_minSeen;
}

/**
 * @brief Returns the highest value observed since the last data reset.
 */
double Widgets::Bar::maxSeen() const noexcept
{
  return m_maxSeen;
}

/**
 * @brief Returns the normalized scale position of the lowest observed value.
 */
double Widgets::Bar::minSeenFrac() const noexcept
{
  return computeFractional(m_minSeen);
}

/**
 * @brief Returns the normalized scale position of the highest observed value.
 */
double Widgets::Bar::maxSeenFrac() const noexcept
{
  return computeFractional(m_maxSeen);
}

//--------------------------------------------------------------------------------------------------
// Value getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the dataset title associated with the widget.
 */
const QString& Widgets::Bar::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the measurement units associated with the dataset.
 */
const QString& Widgets::Bar::units() const noexcept
{
  return m_units;
}

/**
 * @brief Returns the preferred major-tick count (0 = use widget's adaptive heuristic).
 */
int Widgets::Bar::displayTickCount() const noexcept
{
  return m_displayTickCount;
}

/**
 * @brief Returns the tick/value display format ("" = auto; "%.<n>f" or preset slugs accepted).
 */
const QString& Widgets::Bar::displayFormat() const noexcept
{
  return m_displayFormat;
}

/**
 * @brief Returns the fixed display decimal-place count, or -1 for range-driven auto.
 */
int Widgets::Bar::decimalPoints() const noexcept
{
  return m_decimalPoints;
}

/**
 * @brief Returns the current numeric value of the dataset.
 */
double Widgets::Bar::value() const noexcept
{
  return m_value;
}

/**
 * @brief Returns the minimum scale value for the bar.
 */
double Widgets::Bar::minValue() const noexcept
{
  return m_minValue;
}

/**
 * @brief Returns the maximum scale value for the bar.
 */
double Widgets::Bar::maxValue() const noexcept
{
  return m_maxValue;
}

/**
 * @brief Returns the normalized fractional position of the current value.
 */
double Widgets::Bar::normalizedValue() const noexcept
{
  return computeFractional(m_value);
}

//--------------------------------------------------------------------------------------------------
// Band lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the cached active band index from the current value; a value outside every
 *        band clamps to the nearest band so overrange data never renders as unclassified.
 */
void Widgets::Bar::recomputeActiveBand(double value)
{
  m_activeBandIndex = Bands::activeIndex(m_bands, value, m_lastBandHint);
  if (m_activeBandIndex >= 0)
    m_lastBandHint = m_activeBandIndex;
}

/**
 * @brief Refreshes the extreme-hold state from the dashboard store; returns true on change.
 */
bool Widgets::Bar::refreshExtremes(const DataModel::Dataset& dataset)
{
  const auto extremes = m_dashboard.datasetExtremes(dataset.uniqueId);
  const bool valid    = dataset.extremeHold && extremes.valid;
  bool changed        = (valid != m_extremesValid);
  if (valid && (DSP::notEqual(extremes.min, m_minSeen) || DSP::notEqual(extremes.max, m_maxSeen)))
    changed = true;

  m_extremesValid = valid;
  if (valid) {
    m_minSeen = extremes.min;
    m_maxSeen = extremes.max;
  }

  return changed;
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the current dataset value from the dashboard source.
 */
void Widgets::Bar::updateData()
{
  if (!isVisible())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardBar, m_index)) {
    const auto& dataset = GET_DATASET(SerialStudio::DashboardBar, m_index);
    if (!std::isfinite(dataset.numericValue))
      return;

    const bool extremesChanged = refreshExtremes(dataset);
    auto value                 = qMax(m_minValue, qMin(m_maxValue, dataset.numericValue));
    const bool valueChanged    = DSP::notEqual(value, m_value);
    if (valueChanged) {
      m_value = value;
      recomputeActiveBand(value);
    }

    if ((valueChanged || extremesChanged) && isEnabled())
      Q_EMIT updated();
  }
}

/**
 * @brief Pulls a fresh snapshot when the item becomes effectively visible again, so a widget
 *        on a just-activated workspace page never shows values from when it was last shown.
 */
void Widgets::Bar::itemChange(ItemChange change, const ItemChangeData& value)
{
  QQuickItem::itemChange(change, value);

  if (change == ItemVisibleHasChanged && value.boolValue)
    updateData();
}
