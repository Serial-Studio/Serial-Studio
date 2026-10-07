/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "UI/AlarmMonitor.h"

#include <cmath>
#include <QDateTime>
#include <QSet>

#include "Core/Bus/Messages.h"
#include "Core/DataModel/Frame.h"
#include "Core/SSAssert.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/WidgetResolution.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr qint64 kMinNotifyIntervalMs = 3000;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the AlarmMonitor.
 */
UI::AlarmMonitor::AlarmMonitor() : m_dashboard(nullptr), m_notificationCenter(nullptr) {}

/**
 * @brief Retrieves the singleton instance of the AlarmMonitor.
 */
UI::AlarmMonitor& UI::AlarmMonitor::instance()
{
  static AlarmMonitor instance;
  return instance;
}

/**
 * @brief Wires the monitor to the Dashboard's layout and data-update signals.
 */
void UI::AlarmMonitor::setupExternalConnections()
{
  m_dashboard          = &UI::Dashboard::instance();
  m_notificationCenter = &DataModel::pipelineModules().notifications;

  auto& dashboard = UI::Dashboard::instance();
  connect(&dashboard, &UI::Dashboard::widgetCountChanged, this, &AlarmMonitor::rebuildTrackers);
  connect(&dashboard, &UI::Dashboard::dataReset, this, &AlarmMonitor::rebuildTrackers);
  connect(&dashboard, &UI::Dashboard::updated, this, &AlarmMonitor::evaluateAlarms);
}

//--------------------------------------------------------------------------------------------------
// Tracker construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the per-dataset tracker list from the Dashboard's dataset map, one tracker per
 *        dataset whose bands raise alarms: drawn by a widget and not suppressed (spec 0093); the
 *        baseline band is re-captured on the next evaluation so layout rebuilds never re-fire.
 */
void UI::AlarmMonitor::rebuildTrackers()
{
  m_trackers.clear();
  SS_ASSERT(m_dashboard != nullptr, return);

  QSet<int> applicable;
  for (const auto& group : m_dashboard->rawFrame().groups)
    for (const auto& dataset : group.datasets)
      if (SerialStudio::datasetRaisesBandAlarms(dataset, group))
        applicable.insert(dataset.uniqueId);

  const auto& datasets = m_dashboard->datasets();
  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it) {
    const auto& dataset = it.value();
    if (dataset.alarmBands.empty() || !applicable.contains(it.key()))
      continue;

    Tracker tracker;
    tracker.uniqueId      = it.key();
    tracker.hint          = -1;
    tracker.lastFiredBand = -1;
    tracker.initialized   = false;
    tracker.rangeMin      = qMin(dataset.wgtMin, dataset.wgtMax);
    tracker.rangeMax      = qMax(dataset.wgtMin, dataset.wgtMax);
    tracker.title         = dataset.title;
    tracker.units         = dataset.units;
    tracker.lastFireMs    = {0, 0, 0, 0};

    tracker.bands.reserve(dataset.alarmBands.size());
    for (const auto& band : dataset.alarmBands) {
      Band b;
      b.min      = qMin(band.min, band.max);
      b.max      = qMax(band.min, band.max);
      b.severity = static_cast<int>(band.severity);
      b.label    = band.label;
      b.sound    = band.sound;
      tracker.bands.push_back(std::move(b));
    }

    for (const auto& band : tracker.bands) {
      tracker.rangeMin = qMin(tracker.rangeMin, band.min);
      tracker.rangeMax = qMax(tracker.rangeMax, band.max);
    }

    m_trackers.push_back(std::move(tracker));
  }

  Q_EMIT trackersRebuilt();
}

/**
 * @brief Whether a tracker exists for @p uniqueId; the annunciator reaps the band points whose
 *        dataset this stops answering for.
 */
bool UI::AlarmMonitor::tracks(int uniqueId) const noexcept
{
  for (const auto& tracker : m_trackers)
    if (tracker.uniqueId == uniqueId)
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Alarm evaluation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Evaluates every tracked dataset against its bands; datasets are resolved by uniqueId
 *        on each pass so a Dashboard reset between signals can never dangle a tracker.
 */
void UI::AlarmMonitor::evaluateAlarms()
{
  SS_ASSERT(m_dashboard != nullptr, return);

  if (m_trackers.empty())
    return;

  const auto& datasets = m_dashboard->datasets();
  for (auto& tracker : m_trackers) {
    const auto it = datasets.constFind(tracker.uniqueId);
    if (it == datasets.cend())
      continue;

    const auto& dataset = it.value();
    if (!dataset.isNumeric || !std::isfinite(dataset.numericValue))
      continue;

    processValue(tracker, dataset.numericValue);
  }
}

/**
 * @brief Returns the index of the band that contains @a value; -1 if none.
 */
int UI::AlarmMonitor::bandIndexFor(const Tracker& tracker, double value) noexcept
{
  const int count = static_cast<int>(tracker.bands.size());
  if (tracker.hint >= 0 && tracker.hint < count) [[likely]] {
    const auto& b = tracker.bands[tracker.hint];
    if (value >= b.min && value <= b.max)
      return tracker.hint;
  }

  for (int i = 0; i < count; ++i) {
    const auto& b = tracker.bands[i];
    if (value >= b.min && value <= b.max)
      return i;
  }

  return -1;
}

/**
 * @brief Posts a notification on transition into a Warning or Critical band; the value is first
 *        clamped to the widget range widened to the band extents. Every band change, baseline
 *        capture included, is emitted as bandTransition (spec 0087) ahead of the notification
 *        cooldown, so a rebuilt tracker re-seeds its point without a notification.
 */
void UI::AlarmMonitor::processValue(Tracker& tracker, double value)
{
  SS_ASSERT(m_notificationCenter != nullptr, return);

  if (tracker.rangeMax > tracker.rangeMin)
    value = qBound(tracker.rangeMin, value, tracker.rangeMax);

  const int idx = bandIndexFor(tracker, value);
  if (idx >= 0)
    tracker.hint = idx;

  if (!tracker.initialized) {
    tracker.initialized    = true;
    tracker.lastFiredBand  = idx;
    const int seedSeverity = idx < 0 ? -1 : tracker.bands[idx].severity;
    const QString seedLbl  = idx < 0 ? QString() : tracker.bands[idx].label;
    const QString seedSnd  = idx < 0 ? QString() : tracker.bands[idx].sound;
    Q_EMIT bandTransition(tracker.uniqueId, seedSeverity, tracker.title, seedLbl, seedSnd, value);
    return;
  }

  if (idx == tracker.lastFiredBand)
    return;

  tracker.lastFiredBand = idx;
  const int severity    = idx < 0 ? -1 : tracker.bands[idx].severity;
  const QString label   = idx < 0 ? QString() : tracker.bands[idx].label;
  const QString sound   = idx < 0 ? QString() : tracker.bands[idx].sound;
  Q_EMIT bandTransition(tracker.uniqueId, severity, tracker.title, label, sound, value);
  if (idx < 0)
    return;

  const auto& band = tracker.bands[idx];
  if (band.severity < 2)
    return;

  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  for (int s = band.severity; s <= 3; ++s)
    if (now - tracker.lastFireMs[s] < kMinNotifyIntervalMs)
      return;

  tracker.lastFireMs[band.severity] = now;

  const QString unit = tracker.units.isEmpty() ? QString() : QStringLiteral(" ") + tracker.units;
  const QString name = tracker.title.isEmpty() ? tr("Alarm") : tracker.title;
  const QString tier = band.severity >= 3 ? tr("critical") : tr("warning");
  const QString lbl  = band.label.isEmpty() ? tier : band.label;

  const QString subtitle =
    tr("Value %1%2 entered the %3 band (%4–%5).")
      .arg(QString::number(value), unit, lbl, QString::number(band.min), QString::number(band.max));

  const auto level = band.severity >= 3 ? DataModel::NotificationCenter::Critical
                                        : DataModel::NotificationCenter::Warning;
  m_notificationCenter->post(
    level, tr("Alarms"), name, subtitle, Core::Bus::kNotificationOriginAlarmMonitor);
}
