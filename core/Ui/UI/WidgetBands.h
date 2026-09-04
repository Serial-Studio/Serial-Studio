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

#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>

/**
 * @brief Header-only alarm-band lookup shared by the instrument widgets (spec 0052). Works on
 *        any indexable list whose elements expose inclusive `min`/`max` bounds; keeps a caller
 *        hint for the repeat-lookup fast path and clamps out-of-band values to the nearest band
 *        so overrange data never renders as unclassified.
 */
namespace Widgets::Bands {

/**
 * @brief Returns the index of the band containing @a value; -1 if none. @a hint names the last
 *        matched band and is tested first.
 */
template<typename List>
[[nodiscard]] inline int indexFor(const List& bands, double value, int hint = -1) noexcept
{
  const int count = static_cast<int>(bands.size());
  if (hint >= 0 && hint < count) [[likely]] {
    const auto& b = bands[static_cast<std::size_t>(hint)];
    if (value >= b.min && value <= b.max)
      return hint;
  }

  for (int i = 0; i < count; ++i) {
    const auto& b = bands[static_cast<std::size_t>(i)];
    if (value >= b.min && value <= b.max)
      return i;
  }

  return -1;
}

/**
 * @brief Returns the index of the band closest to @a value; -1 only when no bands exist.
 */
template<typename List>
[[nodiscard]] inline int nearestIndex(const List& bands, double value) noexcept
{
  int best             = -1;
  double best_distance = std::numeric_limits<double>::infinity();
  const int count      = static_cast<int>(bands.size());
  for (int i = 0; i < count; ++i) {
    const auto& b         = bands[static_cast<std::size_t>(i)];
    const double distance = std::max({0.0, b.min - value, value - b.max});
    if (distance < best_distance) {
      best_distance = distance;
      best          = i;
    }
  }

  return best;
}

/**
 * @brief Containment lookup with the nearest-band clamp: a value outside every band takes the
 *        closest band's index, so severity never falls through to unclassified (-1 only when
 *        the list is empty).
 */
template<typename List>
[[nodiscard]] inline int activeIndex(const List& bands, double value, int hint = -1) noexcept
{
  const int contained = indexFor(bands, value, hint);
  if (contained >= 0 || bands.size() == 0)
    return contained;

  return nearestIndex(bands, value);
}

/**
 * @brief Severity an instrument reports for band @a activeIndex: unclassified (-1) whenever no
 *        sample has arrived, whatever the lookup resolved. The clamp above is right for overrange
 *        data and wrong for the placeholder 0.0 a widget shows before its first byte, which would
 *        otherwise alarm forever on a project whose bands sit above zero (spec 0075, N3).
 */
template<typename List>
[[nodiscard]] inline int reportedSeverity(const List& bands, int activeIndex, bool hasData) noexcept
{
  if (!hasData || activeIndex < 0 || activeIndex >= static_cast<int>(bands.size()))
    return -1;

  return static_cast<int>(bands[static_cast<std::size_t>(activeIndex)].severity);
}

}  // namespace Widgets::Bands
