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

#pragma once

#include <QString>
#include <QtGlobal>
#include <vector>

namespace UI {
namespace Alarms {

/**
 * @brief Alert priorities in the flight-deck vocabulary (spec 0087 R1); ordinals rank them.
 */
enum class Priority : int {
  None     = -1,
  Advisory = 0,
  Caution  = 1,
  Warning  = 2,
};

/**
 * @brief ISA-18.1 point states.
 */
enum class PointState : int {
  Normal         = 0,
  Alert          = 1,
  Acknowledged   = 2,
  ReturnToNormal = 3,
};

/**
 * @brief ISA-18.1 sequences: automatic reset, manual reset, ringback, each plain (locks in a
 *        momentary alarm until acknowledged) or with option 4 (no lock-in: a point whose
 *        condition clears before Acknowledge settles at once). A4 is the default.
 */
enum class Sequence : int {
  A  = 0,
  M  = 1,
  R  = 2,
  A4 = 3,
  M4 = 4,
  R4 = 5,
};

/**
 * @brief The plain sequence behind an option-4 variant.
 */
[[nodiscard]] constexpr Sequence baseSequence(Sequence sequence) noexcept
{
  switch (sequence) {
    case Sequence::A4:
      return Sequence::A;
    case Sequence::M4:
      return Sequence::M;
    case Sequence::R4:
      return Sequence::R;
    default:
      return sequence;
  }
}

/**
 * @brief True for the plain sequences, which keep an unacknowledged point alerting after its
 *        condition clears.
 */
[[nodiscard]] constexpr bool locksIn(Sequence sequence) noexcept
{
  return sequence == baseSequence(sequence);
}

/**
 * @brief What an alarm point is keyed on: a dataset's band state or a notification pair.
 */
enum class PointKind : int {
  Band         = 0,
  Notification = 1,
};

/**
 * @brief Identity of one alarm point; id is the dataset uniqueId or a hash of channel + title.
 */
struct PointKey {
  PointKind kind;
  int id;

  [[nodiscard]] bool operator==(const PointKey& other) const noexcept
  {
    return kind == other.kind && id == other.id;
  }
};

/**
 * @brief One alarm point: identity, priority, ISA-18.1 state, and the display strings the
 *        annunciator and the API show. active is the process condition (abnormal or not).
 */
struct Point {
  PointKey key;
  Priority priority;
  PointState state;
  bool active;
  bool silenced;
  qint64 sinceMs;
  QString title;
  QString channel;
  QString label;
  QString sound;
};

/**
 * @brief The ISA-18.1 point table (spec 0087 R2 to R7): raise/clear drive the process side,
 *        acknowledge/silence/reset the operator side, and the queries answer what the master
 *        annunciator and the audible arbitration need. Pure C++: no Qt signals, no allocation
 *        on the query paths, a fixed point cap.
 */
class AnnunciatorSequence {
public:
  static constexpr int kMaxPoints = 512;

  explicit AnnunciatorSequence(Sequence sequence);

  [[nodiscard]] Sequence sequence() const noexcept;
  [[nodiscard]] bool anyAlert() const noexcept;
  [[nodiscard]] bool ringbackPending() const noexcept;
  [[nodiscard]] int unacknowledgedCount() const noexcept;
  [[nodiscard]] Priority soundingPriority() const noexcept;
  [[nodiscard]] Priority highestActivePriority() const noexcept;
  [[nodiscard]] const Point* find(const PointKey& key) const noexcept;
  [[nodiscard]] const std::vector<Point>& points() const noexcept;

  [[nodiscard]] bool raise(const PointKey& key,
                           Priority priority,
                           const QString& title,
                           const QString& channel,
                           const QString& label,
                           const QString& sound,
                           qint64 nowMs);
  [[nodiscard]] bool clear(const PointKey& key, qint64 nowMs);
  [[nodiscard]] const Point* latestAlert(Priority priority) const noexcept;

  int reset();
  int silence();
  int acknowledge();
  void clearAll();
  void clearKind(PointKind kind);
  void setSequence(Sequence sequence);

private:
  [[nodiscard]] Point* mutableFind(const PointKey& key) noexcept;
  [[nodiscard]] bool settle(Point& point, qint64 nowMs);
  void dropNormal(bool dropReturned);

  Sequence m_sequence;
  bool m_ringbackSilenced;
  std::vector<Point> m_points;
};

}  // namespace Alarms
}  // namespace UI
