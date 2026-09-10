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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/BarPanel.h"

#include <cmath>
#include <QVariantMap>

#include "Core/DataModel/Frame.h"
#include "Core/SSAssert.h"
#include "DSP.h"
#include "UI/Dashboard.h"
#include "UI/SerialStudioHelpers.h"
#include "UI/WidgetBands.h"
#include "UI/Widgets/DatasetWidgetLinks.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a BarPanel widget.
 */
Widgets::BarPanel::BarPanel(const int index, QQuickItem* parent)
  : QQuickItem(parent), m_index(index), m_revision(0), m_dashboard(UI::Dashboard::instance())
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardBarPanel, m_index)) {
    buildRows();

    connect(&m_dashboard, &UI::Dashboard::updated, this, &BarPanel::updateData);
  }
}

/**
 * @brief Returns the decimal places used for a row's value text: the dataset's explicit
 *        setting when given, else a span-driven default (wide ranges drop the decimals).
 */
[[nodiscard]] static int rowDecimals(const DataModel::Dataset& dataset, double span)
{
  if (dataset.decimalPoints >= 0)
    return qMin(dataset.decimalPoints, 6);

  if (span >= 100.0)
    return 0;

  return span >= 10.0 ? 1 : 2;
}

/**
 * @brief Builds the per-row static snapshot (titles, units, ranges, normalized band tables)
 *        from the owning group's datasets.
 */
void Widgets::BarPanel::buildRows()
{
  const auto& group = GET_GROUP(SerialStudio::DashboardBarPanel, m_index);
  const auto n      = static_cast<int>(group.datasets.size());

  m_styleMode = group.barPanelStyle;
  m_rows.resize(group.datasets.size());
  m_titles.resize(n);
  m_units.resize(n);
  m_valueTexts.resize(n);
  m_ranged.resize(n);
  m_numeric.resize(n);
  m_extremesOk.resize(n);
  m_severities.resize(n);
  m_fracs.resize(n);
  m_minSeenFracs.resize(n);
  m_maxSeenFracs.resize(n);
  m_bandsAsVariant.clear();
  m_bandsAsVariant.reserve(n);
  m_widgets.clear();
  m_widgets.reserve(n);

  for (size_t i = 0; i < group.datasets.size(); ++i) {
    const auto& dataset = group.datasets[i];
    const auto idx      = static_cast<int>(i);

    auto& row       = m_rows[i];
    row.hint        = -1;
    row.uniqueId    = dataset.uniqueId;
    row.extremeHold = dataset.extremeHold;
    row.min         = qMin(dataset.wgtMin, dataset.wgtMax);
    row.max         = qMax(dataset.wgtMin, dataset.wgtMax);
    row.ranged      = !DSP::isZero(row.max - row.min);
    row.decimals    = rowDecimals(dataset, row.max - row.min);

    m_titles[idx]       = dataset.title;
    m_units[idx]        = dataset.units;
    m_valueTexts[idx]   = QStringLiteral("--");
    m_ranged[idx]       = row.ranged;
    m_numeric[idx]      = false;
    m_extremesOk[idx]   = false;
    m_severities[idx]   = -1;
    m_fracs[idx]        = 0.0;
    m_minSeenFracs[idx] = 0.0;
    m_maxSeenFracs[idx] = 0.0;

    const double span = row.ranged ? (row.max - row.min) : 1.0;
    QVariantList rowBands;
    row.bands.reserve(dataset.alarmBands.size());
    for (const auto& src : dataset.alarmBands) {
      const double lo = qBound(row.min, qMin(src.min, src.max), row.max);
      const double hi = qBound(row.min, qMax(src.min, src.max), row.max);
      if (hi <= lo)
        continue;

      RowBand band;
      band.min      = lo;
      band.max      = hi;
      band.severity = static_cast<int>(src.severity);
      row.bands.push_back(band);

      QVariantMap entry;
      entry.insert(QStringLiteral("fracMin"), qBound(0.0, (lo - row.min) / span, 1.0));
      entry.insert(QStringLiteral("fracMax"), qBound(0.0, (hi - row.min) / span, 1.0));
      entry.insert(QStringLiteral("severity"), band.severity);
      entry.insert(QStringLiteral("customColor"), src.color);
      rowBands.append(entry);
    }

    m_bandsAsVariant.append(QVariant(rowBands));
    m_widgets.append(QVariant(datasetWidgetLinks(m_dashboard, dataset)));
  }
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of bar rows in the panel.
 */
int Widgets::BarPanel::count() const noexcept
{
  return m_titles.count();
}

/**
 * @brief Returns the persisted orientation mode ("" = auto, "horizontal", "vertical").
 */
const QString& Widgets::BarPanel::styleMode() const noexcept
{
  return m_styleMode;
}

/**
 * @brief Returns the dataset titles, one per row.
 */
const QStringList& Widgets::BarPanel::titles() const noexcept
{
  return m_titles;
}

/**
 * @brief Returns the dataset units, one per row.
 */
const QStringList& Widgets::BarPanel::units() const noexcept
{
  return m_units;
}

/**
 * @brief Returns per-row flags: true when the widget range is usable and the track renders.
 */
const QVector<bool>& Widgets::BarPanel::ranged() const noexcept
{
  return m_ranged;
}

/**
 * @brief Returns the per-row alarm-band lists (fracMin/fracMax/severity/customColor maps).
 */
const QVariantList& Widgets::BarPanel::bands() const noexcept
{
  return m_bandsAsVariant;
}

/**
 * @brief Returns, per row, the {windowId, icon, title} list of the other dashboard widgets that
 *        show that row's dataset (plot, gauge, ...), for the row's pop-out buttons.
 */
const QVariantList& Widgets::BarPanel::widgets() const noexcept
{
  return m_widgets;
}

/**
 * @brief Monotonic change counter bumped on every applied update. QML bindings reference it as
 *        their sole notify dependency and read row state through the scalar accessors below, so
 *        a tick converts a handful of scalars instead of every per-row list.
 */
int Widgets::BarPanel::revision() const noexcept
{
  return m_revision;
}

/**
 * @brief Returns the normalized fill fraction of one row; 0.0 when out of range.
 */
double Widgets::BarPanel::frac(int row) const
{
  return m_fracs.value(row, 0.0);
}

/**
 * @brief The fill colour a row uses outside any alarm band: the dataset's accent (an explicit
 *        override wins, else the theme palette), resolved on demand so a theme switch re-reads it.
 */
QString Widgets::BarPanel::rowColor(const int row) const
{
  SS_ASSERT(VALIDATE_WIDGET(SerialStudio::DashboardBarPanel, m_index), return QString());
  const auto& group = GET_GROUP(SerialStudio::DashboardBarPanel, m_index);
  SS_ASSERT(row >= 0 && row < static_cast<int>(group.datasets.size()), return QString());
  const auto& dataset = group.datasets[static_cast<size_t>(row)];
  return UI::SerialStudioHelpers::getDatasetAccentColor(dataset).name();
}

/**
 * @brief Returns one row's active band severity (-1 = no bands defined or unknown row).
 */
int Widgets::BarPanel::severity(int row) const
{
  return m_severities.value(row, -1);
}

/**
 * @brief Returns true while one row's latest value parsed as a finite number.
 */
bool Widgets::BarPanel::isNumeric(int row) const
{
  return m_numeric.value(row, false);
}

/**
 * @brief Returns one row's formatted value text ("--" before data arrives).
 */
QString Widgets::BarPanel::valueText(int row) const
{
  if (row < 0 || row >= m_valueTexts.size())
    return QStringLiteral("--");

  return m_valueTexts.at(row);
}

/**
 * @brief Returns true when one row's min/max hold markers hold at least one sample.
 */
bool Widgets::BarPanel::hasExtremes(int row) const
{
  return m_extremesOk.value(row, false);
}

/**
 * @brief Returns the normalized position of one row's lowest observed value.
 */
double Widgets::BarPanel::minSeenFrac(int row) const
{
  return m_minSeenFracs.value(row, 0.0);
}

/**
 * @brief Returns the normalized position of one row's highest observed value.
 */
double Widgets::BarPanel::maxSeenFrac(int row) const
{
  return m_maxSeenFracs.value(row, 0.0);
}

//--------------------------------------------------------------------------------------------------
// Band lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the band that contains @a value; -1 if none.
 */
int Widgets::BarPanel::bandIndexFor(const Row& row, double value) noexcept
{
  return Bands::indexFor(row.bands, value, row.hint);
}

/**
 * @brief Returns the index of the band closest to @a value; -1 only when no bands exist.
 */
int Widgets::BarPanel::nearestBandIndex(const Row& row, double value) noexcept
{
  return Bands::nearestIndex(row.bands, value);
}

/**
 * @brief Returns the normalized scale position of @a value on a row's track.
 */
double Widgets::BarPanel::rowFraction(const Row& row, double value) const noexcept
{
  if (!row.ranged)
    return 0.0;

  const double clamped = qBound(row.min, value, row.max);
  return qBound(0.0, (clamped - row.min) / (row.max - row.min), 1.0);
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-derives one row's QML-facing arrays from its dataset; a value outside every band
 *        clamps to the nearest band's severity so overrange data never renders unclassified.
 */
bool Widgets::BarPanel::refreshRow(int index, const DataModel::Dataset& dataset)
{
  auto& row          = m_rows[static_cast<size_t>(index)];
  const bool numeric = dataset.isNumeric && std::isfinite(dataset.numericValue);

  int severity = -1;
  double frac  = 0.0;
  QString text = QStringLiteral("--");
  if (numeric) {
    frac     = rowFraction(row, dataset.numericValue);
    int band = bandIndexFor(row, dataset.numericValue);
    if (band < 0 && !row.bands.empty())
      band = nearestBandIndex(row, dataset.numericValue);

    if (band >= 0) {
      row.hint = band;
      severity = row.bands[static_cast<size_t>(band)].severity;
    }

    text = QString::number(dataset.numericValue, 'f', row.decimals);
    if (!dataset.units.isEmpty())
      text += QChar(' ') + dataset.units;
  } else {
    severity = row.bands.empty() ? -1 : 2;
    if (!dataset.value.isEmpty())
      text = dataset.value;
  }

  bool extremesOk    = false;
  double minSeenFrac = 0.0;
  double maxSeenFrac = 0.0;
  if (row.extremeHold) {
    const auto extremes = m_dashboard.datasetExtremes(row.uniqueId);
    extremesOk          = extremes.valid;
    if (extremesOk) {
      minSeenFrac = rowFraction(row, extremes.min);
      maxSeenFrac = rowFraction(row, extremes.max);
    }
  }

  const bool changed = (numeric != m_numeric[index]) || (severity != m_severities[index])
                    || (extremesOk != m_extremesOk[index]) || (text != m_valueTexts[index])
                    || DSP::notEqual(frac, m_fracs[index])
                    || DSP::notEqual(minSeenFrac, m_minSeenFracs[index])
                    || DSP::notEqual(maxSeenFrac, m_maxSeenFracs[index]);
  m_numeric[index]      = numeric;
  m_severities[index]   = severity;
  m_extremesOk[index]   = extremesOk;
  m_valueTexts[index]   = text;
  m_fracs[index]        = frac;
  m_minSeenFracs[index] = minSeenFrac;
  m_maxSeenFracs[index] = maxSeenFrac;
  return changed;
}

/**
 * @brief Updates the bar panel data from the Dashboard; hidden panels (any inactive workspace
 *        page) skip the pass entirely and itemChange() refreshes on the next show.
 */
void Widgets::BarPanel::updateData()
{
  if (!isEnabled() || !isVisible())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardBarPanel, m_index)) {
    bool changed      = false;
    const auto& group = GET_GROUP(SerialStudio::DashboardBarPanel, m_index);
    const size_t n    = qMin(m_rows.size(), group.datasets.size());
    for (size_t i = 0; i < n; ++i)
      changed |= refreshRow(static_cast<int>(i), group.datasets[i]);

    if (changed) {
      ++m_revision;
      Q_EMIT updated();
    }
  }
}

/**
 * @brief Pulls a fresh snapshot when the item becomes effectively visible again, so a panel on
 *        a just-activated workspace page never shows the values from when it was last shown.
 */
void Widgets::BarPanel::itemChange(ItemChange change, const ItemChangeData& value)
{
  QQuickItem::itemChange(change, value);

  if (change == ItemVisibleHasChanged && value.boolValue)
    updateData();
}
