/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player/ReplayClock.h"

/**
 * @brief Constructs an unanchored clock: the epoch of the steady clock, row zero.
 */
Sessions::ReplayClock::ReplayClock() : m_baseRowSeconds(0.0) {}

/**
 * @brief Returns the recorded instant, in seconds, the anchor was taken at.
 */
double Sessions::ReplayClock::baseRowSeconds() const noexcept
{
  return m_baseRowSeconds;
}

/**
 * @brief Returns the steady-clock instant the anchor was taken at.
 */
std::chrono::steady_clock::time_point Sessions::ReplayClock::base() const noexcept
{
  return m_base;
}

/**
 * @brief Steady timestamp for @p timestampNs: the anchor advanced by the recorded delta.
 */
std::chrono::steady_clock::time_point Sessions::ReplayClock::timestampFor(qint64 timestampNs) const
{
  const auto delta = std::chrono::duration<double>(timestampNs / 1e9 - m_baseRowSeconds);
  return m_base + std::chrono::duration_cast<std::chrono::steady_clock::duration>(delta);
}

/**
 * @brief Anchors @p rowSeconds of the recording to now.
 */
void Sessions::ReplayClock::anchor(double rowSeconds)
{
  anchorAt(std::chrono::steady_clock::now(), rowSeconds);
}

/**
 * @brief Anchors @p rowSeconds of the recording to an explicit steady instant.
 */
void Sessions::ReplayClock::anchorAt(const std::chrono::steady_clock::time_point& base,
                                     double rowSeconds)
{
  m_base           = base;
  m_baseRowSeconds = rowSeconds;
}

#endif
