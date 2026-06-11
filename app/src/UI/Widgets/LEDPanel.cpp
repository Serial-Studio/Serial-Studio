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

#include "UI/Widgets/LEDPanel.h"

#include <limits>

#include "Misc/ThemeManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an LEDPanel widget.
 */
Widgets::LEDPanel::LEDPanel(const int index, QQuickItem* parent)
  : QQuickItem(parent), m_index(index)
{
  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index)) {
    buildLeds();

    connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &LEDPanel::updateData);

    onThemeChanged();
    connect(&Misc::ThemeManager::instance(),
            &Misc::ThemeManager::themeChanged,
            this,
            &Widgets::LEDPanel::onThemeChanged);
  }
}

/**
 * @brief Builds the per-LED band tables from the group's datasets; a dataset without alarm
 *        bands gets one synthetic ledHigh band (severity -1 = use the dataset color).
 */
void Widgets::LEDPanel::buildLeds()
{
  const auto& group = GET_GROUP(SerialStudio::DashboardLED, m_index);
  const auto n      = static_cast<int>(group.datasets.size());

  m_leds.resize(group.datasets.size());
  m_states.resize(n);
  m_blinks.resize(n);
  m_titles.resize(n);
  m_colors.resize(n);
  m_labels.resize(n);

  for (size_t i = 0; i < group.datasets.size(); ++i) {
    const auto& dataset = group.datasets[i];
    const auto idx      = static_cast<int>(i);

    m_states[idx] = false;
    m_blinks[idx] = false;
    m_titles[idx] = dataset.title;

    auto& led      = m_leds[i];
    led.hint       = -1;
    led.activeBand = -1;

    if (dataset.alarmBands.empty()) {
      LedBand band;
      band.min      = dataset.ledHigh;
      band.max      = std::numeric_limits<double>::infinity();
      band.severity = -1;
      band.blink    = false;
      led.bands.push_back(std::move(band));
      continue;
    }

    led.bands.reserve(dataset.alarmBands.size());
    for (const auto& src : dataset.alarmBands) {
      LedBand band;
      band.min         = qMin(src.min, src.max);
      band.max         = qMax(src.min, src.max);
      band.severity    = static_cast<int>(src.severity);
      band.blink       = src.blink;
      band.customColor = src.color;
      band.label       = src.label;
      led.bands.push_back(std::move(band));
    }
  }
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of LEDs in the panel.
 */
int Widgets::LEDPanel::count() const noexcept
{
  return m_titles.count();
}

//--------------------------------------------------------------------------------------------------
// State getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the lit/unlit states of the LEDs in the panel.
 */
const QList<bool>& Widgets::LEDPanel::states() const noexcept
{
  return m_states;
}

/**
 * @brief Returns the blink flags of the LEDs' active bands.
 */
const QList<bool>& Widgets::LEDPanel::blinks() const noexcept
{
  return m_blinks;
}

/**
 * @brief Returns the display colors of the LEDs (active band color, dataset color when off).
 */
const QStringList& Widgets::LEDPanel::colors() const noexcept
{
  return m_colors;
}

/**
 * @brief Returns the labels of the LEDs' active bands (empty when off or unlabeled).
 */
const QStringList& Widgets::LEDPanel::labels() const noexcept
{
  return m_labels;
}

/**
 * @brief Returns the titles of the LEDs in the panel.
 */
const QStringList& Widgets::LEDPanel::titles() const noexcept
{
  return m_titles;
}

//--------------------------------------------------------------------------------------------------
// Band lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the band that contains @a value; -1 if none.
 */
int Widgets::LEDPanel::bandIndexFor(const Led& led, double value) noexcept
{
  const int count = static_cast<int>(led.bands.size());
  if (led.hint >= 0 && led.hint < count) [[likely]] {
    const auto& b = led.bands[led.hint];
    if (value >= b.min && value <= b.max)
      return led.hint;
  }

  for (int i = 0; i < count; ++i) {
    const auto& b = led.bands[i];
    if (value >= b.min && value <= b.max)
      return i;
  }

  return -1;
}

/**
 * @brief Re-derives the QML-facing display arrays for one LED from its active band.
 */
void Widgets::LEDPanel::refreshDisplayState(int index)
{
  const auto& led = m_leds[static_cast<size_t>(index)];
  const int band  = led.activeBand;

  m_states[index] = band >= 0;
  m_blinks[index] = band >= 0 && led.bands[static_cast<size_t>(band)].blink;
  m_colors[index] = band >= 0 ? led.bands[static_cast<size_t>(band)].resolvedColor : led.offColor;
  m_labels[index] = band >= 0 ? led.bands[static_cast<size_t>(band)].label : QString();
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the LED panel data from the Dashboard.
 */
void Widgets::LEDPanel::updateData()
{
  if (!isEnabled())
    return;

  if (VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index)) {
    bool changed      = false;
    const auto& group = GET_GROUP(SerialStudio::DashboardLED, m_index);
    const size_t n    = qMin(m_leds.size(), group.datasets.size());
    for (size_t i = 0; i < n; ++i) {
      auto& led     = m_leds[i];
      const int idx = bandIndexFor(led, group.datasets[i].numericValue);
      if (idx >= 0)
        led.hint = idx;

      if (led.activeBand != idx) {
        led.activeBand = idx;
        refreshDisplayState(static_cast<int>(i));
        changed = true;
      }
    }

    if (changed)
      Q_EMIT updated();
  }
}

//--------------------------------------------------------------------------------------------------
// Theme management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a band's display color: custom override, severity theme color, or the
 *        dataset color for legacy ledHigh bands (severity -1).
 */
QString Widgets::LEDPanel::resolveBandColor(const LedBand& band, const QString& datasetColor)
{
  if (!band.customColor.isEmpty())
    return band.customColor;

  if (band.severity >= 0)
    return Misc::ThemeManager::instance().alarmColorForSeverity(band.severity).name();

  return datasetColor;
}

/**
 * @brief Re-resolves band and off-state colors from the current theme, then refreshes every
 *        LED's display arrays.
 */
void Widgets::LEDPanel::onThemeChanged()
{
  if (!VALIDATE_WIDGET(SerialStudio::DashboardLED, m_index))
    return;

  const auto& group = GET_GROUP(SerialStudio::DashboardLED, m_index);
  const size_t n    = qMin(m_leds.size(), group.datasets.size());
  for (size_t i = 0; i < n; ++i) {
    auto& led        = m_leds[i];
    const auto color = SerialStudio::getDatasetColor(group.datasets[i].index).name();

    led.offColor = color;
    for (auto& band : led.bands)
      band.resolvedColor = resolveBandColor(band, color);

    refreshDisplayState(static_cast<int>(i));
  }

  Q_EMIT updated();
}
