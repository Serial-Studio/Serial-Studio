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

#include "UI/Alarms/AnnunciatorSequence.h"

#include <algorithm>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty table running @p sequence.
 */
UI::Alarms::AnnunciatorSequence::AnnunciatorSequence(Sequence sequence)
  : m_sequence(sequence), m_ringbackSilenced(false)
{
  m_points.reserve(64);
}

/**
 * @brief The sequence in force.
 */
UI::Alarms::Sequence UI::Alarms::AnnunciatorSequence::sequence() const noexcept
{
  return m_sequence;
}

/**
 * @brief True while any point is in Alert (unacknowledged); drives the fast flash.
 */
bool UI::Alarms::AnnunciatorSequence::anyAlert() const noexcept
{
  return unacknowledgedCount() > 0;
}

/**
 * @brief True when sequence R has a Return-to-normal point whose ringback was not silenced.
 */
bool UI::Alarms::AnnunciatorSequence::ringbackPending() const noexcept
{
  if (baseSequence(m_sequence) != Sequence::R || m_ringbackSilenced)
    return false;

  return std::any_of(m_points.cbegin(), m_points.cend(), [](const Point& p) {
    return p.state == PointState::ReturnToNormal;
  });
}

/**
 * @brief Number of points in Alert.
 */
int UI::Alarms::AnnunciatorSequence::unacknowledgedCount() const noexcept
{
  int count = 0;
  for (const auto& point : m_points)
    count += point.state == PointState::Alert ? 1 : 0;

  return count;
}

/**
 * @brief Highest priority among points in Alert that were not silenced (spec R6); None when the
 *        alarm lane should be quiet (ringback is decided separately by ringbackPending()).
 */
UI::Alarms::Priority UI::Alarms::AnnunciatorSequence::soundingPriority() const noexcept
{
  Priority best = Priority::None;
  for (const auto& point : m_points) {
    if (point.state != PointState::Alert || point.silenced)
      continue;

    best = std::max(best, point.priority);
  }

  return best;
}

/**
 * @brief Highest priority among every point not yet Normal; what the master annunciator shows.
 */
UI::Alarms::Priority UI::Alarms::AnnunciatorSequence::highestActivePriority() const noexcept
{
  Priority best = Priority::None;
  for (const auto& point : m_points)
    best = std::max(best, point.priority);

  return best;
}

/**
 * @brief The point with @p key, or nullptr.
 */
const UI::Alarms::Point* UI::Alarms::AnnunciatorSequence::find(const PointKey& key) const noexcept
{
  for (const auto& point : m_points)
    if (point.key == key)
      return &point;

  return nullptr;
}

/**
 * @brief Every point not in Normal, in arrival order.
 */
const std::vector<UI::Alarms::Point>& UI::Alarms::AnnunciatorSequence::points() const noexcept
{
  return m_points;
}

//--------------------------------------------------------------------------------------------------
// Process side
//--------------------------------------------------------------------------------------------------

/**
 * @brief The process condition became (or stayed) abnormal at @p priority. Returns true when the
 *        audible must (re)start: a new point, a re-alert from Return-to-normal, or a priority
 *        rise, each of which also clears a Silence (reflash).
 */
bool UI::Alarms::AnnunciatorSequence::raise(const PointKey& key,
                                            Priority priority,
                                            const QString& title,
                                            const QString& channel,
                                            const QString& label,
                                            const QString& sound,
                                            qint64 nowMs)
{
  SS_ASSERT(priority != Priority::None, return false);

  Point* existing = mutableFind(key);
  if (!existing) {
    if (m_points.size() >= static_cast<std::size_t>(kMaxPoints))
      return false;

    m_points.push_back(
      Point{key, priority, PointState::Alert, true, false, nowMs, title, channel, label, sound});
    return true;
  }

  existing->active   = true;
  existing->label    = label;
  existing->sound    = sound;
  const bool rise    = priority > existing->priority;
  existing->priority = std::max(existing->priority, priority);
  if (existing->state == PointState::ReturnToNormal || rise) {
    existing->state    = PointState::Alert;
    existing->silenced = false;
    existing->sinceMs  = nowMs;
    return true;
  }

  return false;
}

/**
 * @brief The process condition returned to normal. Returns true when a ringback must start
 *        (sequence R, acknowledged point entering Return-to-normal).
 */
bool UI::Alarms::AnnunciatorSequence::clear(const PointKey& key, qint64 nowMs)
{
  Point* point = mutableFind(key);
  if (!point)
    return false;

  point->active       = false;
  const bool ringback = settle(*point, nowMs);
  dropNormal(baseSequence(m_sequence) == Sequence::A);
  return ringback;
}

/**
 * @brief The most recently raised point still in Alert at @p priority; what the audible plays
 *        when arbitration lands on that priority. nullptr when none.
 */
const UI::Alarms::Point* UI::Alarms::AnnunciatorSequence::latestAlert(
  Priority priority) const noexcept
{
  const Point* best = nullptr;
  for (const auto& point : m_points) {
    if (point.state != PointState::Alert || point.priority != priority)
      continue;

    if (!best || point.sinceMs >= best->sinceMs)
      best = &point;
  }

  return best;
}

/**
 * @brief Drops every point without ringback (disconnect, project reload, data reset).
 */
void UI::Alarms::AnnunciatorSequence::clearAll()
{
  m_points.clear();
  m_ringbackSilenced = false;
}

/**
 * @brief Drops every point of one kind (a tracker rebuild invalidates the band points only).
 */
void UI::Alarms::AnnunciatorSequence::clearKind(PointKind kind)
{
  m_points.erase(std::remove_if(m_points.begin(),
                                m_points.end(),
                                [kind](const Point& p) { return p.key.kind == kind; }),
                 m_points.end());
}

/**
 * @brief Drops every point of @p kind still in Alert (spec 0088 R3): a link drop discards
 *        unacknowledged notification points, whose raiser cannot re-assert or clear them across
 *        the outage, while acknowledged ones survive the hold. Returns how many were dropped.
 */
int UI::Alarms::AnnunciatorSequence::dropUnacknowledged(PointKind kind)
{
  int dropped = 0;
  m_points.erase(std::remove_if(m_points.begin(),
                                m_points.end(),
                                [kind, &dropped](const Point& p) {
                                  const bool drop =
                                    p.key.kind == kind && p.state == PointState::Alert;
                                  dropped += drop ? 1 : 0;
                                  return drop;
                                }),
                 m_points.end());
  return dropped;
}

/**
 * @brief Switches sequence; leaving R or M drops the Return-to-normal points A cannot hold.
 */
void UI::Alarms::AnnunciatorSequence::setSequence(Sequence sequence)
{
  if (m_sequence == sequence)
    return;

  m_sequence = sequence;
  dropNormal(baseSequence(sequence) == Sequence::A);
}

//--------------------------------------------------------------------------------------------------
// Operator side
//--------------------------------------------------------------------------------------------------

/**
 * @brief Acknowledge: every Alert point becomes Acknowledged, or settles further when its
 *        process condition already returned to normal. Returns how many points changed.
 */
int UI::Alarms::AnnunciatorSequence::acknowledge()
{
  int changed = 0;
  for (auto& point : m_points) {
    if (point.state != PointState::Alert)
      continue;

    point.state    = PointState::Acknowledged;
    point.silenced = false;
    ++changed;
    if (!point.active)
      (void)settle(point, point.sinceMs);
  }

  dropNormal(baseSequence(m_sequence) == Sequence::A);
  return changed;
}

/**
 * @brief Silence: the audible stops, points stay in Alert and keep flashing; a re-raise or a
 *        priority rise re-sounds. Also quiets a pending ringback until the next one arrives.
 */
int UI::Alarms::AnnunciatorSequence::silence()
{
  int changed = 0;
  for (auto& point : m_points) {
    if (point.state != PointState::Alert || point.silenced)
      continue;

    point.silenced = true;
    ++changed;
  }

  if (ringbackPending()) {
    m_ringbackSilenced = true;
    ++changed;
  }

  return changed;
}

/**
 * @brief Reset: Return-to-normal points go Normal (sequences M and R); no effect in A.
 */
int UI::Alarms::AnnunciatorSequence::reset()
{
  int changed = 0;
  for (const auto& point : m_points)
    changed += point.state == PointState::ReturnToNormal ? 1 : 0;

  dropNormal(true);
  m_ringbackSilenced = false;
  return changed;
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mutable lookup.
 */
UI::Alarms::Point* UI::Alarms::AnnunciatorSequence::mutableFind(const PointKey& key) noexcept
{
  for (auto& point : m_points)
    if (point.key == key)
      return &point;

  return nullptr;
}

/**
 * @brief Return-to-normal rule for a point whose condition is normal: a plain sequence keeps an
 *        unacknowledged point in Alert (lock-in), an option-4 sequence settles it at once; then
 *        A drops it, M and R park it in Return-to-normal, R rings back.
 */
bool UI::Alarms::AnnunciatorSequence::settle(Point& point, qint64 nowMs)
{
  SS_ASSERT(!point.active, return false);
  if (point.state == PointState::Alert && locksIn(m_sequence))
    return false;

  if (baseSequence(m_sequence) == Sequence::A) {
    point.state = PointState::Normal;
    return false;
  }

  if (point.state == PointState::ReturnToNormal)
    return false;

  point.state        = PointState::ReturnToNormal;
  point.sinceMs      = nowMs;
  m_ringbackSilenced = false;
  return baseSequence(m_sequence) == Sequence::R;
}

/**
 * @brief Removes every point in Normal, plus every Return-to-normal point when @p dropReturned.
 */
void UI::Alarms::AnnunciatorSequence::dropNormal(bool dropReturned)
{
  m_points.erase(std::remove_if(m_points.begin(),
                                m_points.end(),
                                [dropReturned](const Point& p) {
                                  return p.state == PointState::Normal
                                      || (dropReturned && p.state == PointState::ReturnToNormal);
                                }),
                 m_points.end());
}
