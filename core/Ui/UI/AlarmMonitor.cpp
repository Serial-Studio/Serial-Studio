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
#include "UI/AlarmDeadband.h"
#include "UI/Dashboard.h"
#include "UI/WidgetVisibility.h"

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
UI::AlarmMonitor::AlarmMonitor()
  : m_dashboard(nullptr), m_notificationCenter(nullptr), m_visibility(nullptr)
{}

/**
 * @brief Out of line so the owned WidgetVisibility is complete where it is destroyed.
 */
UI::AlarmMonitor::~AlarmMonitor() = default;

/**
 * @brief Retrieves the singleton instance of the AlarmMonitor.
 */
UI::AlarmMonitor& UI::AlarmMonitor::instance()
{
  static AlarmMonitor instance;
  return instance;
}

/**
 * @brief Wires the monitor to the Dashboard's layout and data-update signals and builds the
 *        visibility fold the opted-in datasets are gated on, over the taskbars @p registry lists
 *        (spec 0105). A --headless run has no taskbars, so an opted-in dataset is hidden there by
 *        the rule's own terms; the re-record root never calls this at all.
 */
void UI::AlarmMonitor::setupExternalConnections(UISessionRegistry& registry)
{
  m_dashboard          = &UI::Dashboard::instance();
  m_notificationCenter = &DataModel::pipelineModules().notifications;

  auto& dashboard = UI::Dashboard::instance();
  connect(&dashboard, &UI::Dashboard::widgetCountChanged, this, &AlarmMonitor::rebuildTrackers);
  connect(&dashboard, &UI::Dashboard::dataReset, this, &AlarmMonitor::rebuildTrackers);
  connect(&dashboard, &UI::Dashboard::updated, this, &AlarmMonitor::evaluateAlarms);

  m_visibility = std::make_unique<WidgetVisibility>(dashboard, registry);
  connect(
    m_visibility.get(), &WidgetVisibility::shownChanged, this, &AlarmMonitor::applyVisibility);
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
    tracker.whileVisible  = dataset.alarmsWhileVisible;
    tracker.shown         = !m_visibility || m_visibility->isShown(it.key());
    tracker.revealPending = false;
    tracker.rangeMin      = qMin(dataset.wgtMin, dataset.wgtMax);
    tracker.rangeMax      = qMax(dataset.wgtMin, dataset.wgtMax);
    tracker.deadband      = qMax(0.0, dataset.alarmDeadband);
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
 * @brief Whether @p uniqueId alarms right now: a tracker exists and, for a dataset that opted
 *        into Alarms only while visible, a widget showing it is on screen (the runtime home of
 *        SerialStudio::datasetAlarmsNow); the annunciator reaps the band points whose dataset this
 *        stops answering for.
 */
bool UI::AlarmMonitor::tracks(int uniqueId) const noexcept
{
  for (const auto& tracker : m_trackers)
    if (tracker.uniqueId == uniqueId)
      return !tracker.whileVisible || tracker.shown;

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
    if (tracker.whileVisible && !tracker.shown)
      continue;

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
 * @brief Applies a change of the shown set to the opted-in trackers (spec 0105 R4): a hidden one
 *        stops being tracked, so the annunciator reaps its point through trackersRebuilt, and
 *        forgets its band; a revealed one is evaluated at once from the dataset's current value
 *        as a band entry under the usual cooldown, whose stamps survive the hide on purpose.
 */
void UI::AlarmMonitor::applyVisibility()
{
  SS_ASSERT(m_dashboard != nullptr, return);
  SS_ASSERT(m_visibility != nullptr, return);

  bool changed = false;
  std::vector<size_t> revealed;
  for (size_t i = 0; i < m_trackers.size(); ++i) {
    auto& tracker = m_trackers[i];
    if (!tracker.whileVisible)
      continue;

    const bool shown = m_visibility->isShown(tracker.uniqueId);
    if (shown == tracker.shown)
      continue;

    changed       = true;
    tracker.shown = shown;
    if (shown) {
      tracker.revealPending = true;
      revealed.push_back(i);
      continue;
    }

    tracker.initialized   = false;
    tracker.revealPending = false;
    tracker.hint          = -1;
    tracker.lastFiredBand = -1;
  }

  if (!changed)
    return;

  Q_EMIT trackersRebuilt();

  const auto& datasets = m_dashboard->datasets();
  for (const size_t i : revealed) {
    auto& tracker = m_trackers[i];
    const auto it = datasets.constFind(tracker.uniqueId);
    if (it == datasets.cend() || !it->isNumeric || !std::isfinite(it->numericValue))
      continue;

    processValue(tracker, it->numericValue);
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
 * @brief The band the tracker reports for @a value: the containing band, except that the band
 *        last reported holds on while the value sits within the dataset's deadband of its edges
 *        and the move would not escalate (UI::AlarmDeadband::holdsActiveBand).
 */
int UI::AlarmMonitor::resolveBand(const Tracker& tracker, double value) noexcept
{
  const int candidate = bandIndexFor(tracker, value);
  const int active    = tracker.lastFiredBand;
  if (!tracker.initialized || active < 0 || candidate == active)
    return candidate;

  SS_ASSERT(active < static_cast<int>(tracker.bands.size()), return candidate);
  const auto& band            = tracker.bands[active];
  const int candidateSeverity = candidate < 0 ? -1 : tracker.bands[candidate].severity;
  if (AlarmDeadband::holdsActiveBand(
        value, band.min, band.max, band.severity, candidateSeverity, tracker.deadband))
    return active;

  return candidate;
}

/**
 * @brief Posts a notification on transition into a Warning or Critical band after clamping the
 *        value to the widget range widened to the band extents and the deadband hold. Every band
 *        change is emitted as bandTransition (spec 0087) ahead of the cooldown: a rebuilt tracker
 *        re-seeds silently, a revealed one seeds and then notifies (spec 0105: reveal = entry).
 */
void UI::AlarmMonitor::processValue(Tracker& tracker, double value)
{
  SS_ASSERT(m_notificationCenter != nullptr, return);

  if (tracker.rangeMax > tracker.rangeMin)
    value = qBound(tracker.rangeMin, value, tracker.rangeMax);

  const int idx = resolveBand(tracker, value);
  if (idx >= 0)
    tracker.hint = idx;

  const bool seeding = !tracker.initialized;
  if (!seeding && idx == tracker.lastFiredBand)
    return;

  tracker.initialized   = true;
  tracker.lastFiredBand = idx;
  const int severity    = idx < 0 ? -1 : tracker.bands[idx].severity;
  const QString label   = idx < 0 ? QString() : tracker.bands[idx].label;
  const QString sound   = idx < 0 ? QString() : tracker.bands[idx].sound;
  Q_EMIT bandTransition(tracker.uniqueId, severity, tracker.title, label, sound, value);

  const bool reveal     = tracker.revealPending;
  tracker.revealPending = false;
  if (idx < 0 || (seeding && !reveal))
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
