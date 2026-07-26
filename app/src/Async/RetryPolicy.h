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

namespace Async {
/**
 * @brief The one retry-with-backoff policy every flow shares: geometric schedule, ceiling,
 *        attempt cap, and the reset rule. Copied by value into each tree, so two runners on
 *        two threads never share state.
 */
class RetryPolicy {
public:
  RetryPolicy();
  RetryPolicy(int max_attempts, int initial_delay_msec, int max_delay_msec, double multiplier);

  [[nodiscard]] double multiplier() const noexcept;
  [[nodiscard]] int maxAttempts() const noexcept;
  [[nodiscard]] int maxDelayMsec() const noexcept;
  [[nodiscard]] int initialDelayMsec() const noexcept;
  [[nodiscard]] bool shouldRetry(int completed_attempts) const noexcept;
  [[nodiscard]] int delayForAttempt(int completed_attempts) const noexcept;

  [[nodiscard]] static RetryPolicy autoReconnect();
  [[nodiscard]] static RetryPolicy initialConnect();

private:
  double m_multiplier;
  int m_maxAttempts;
  int m_maxDelayMsec;
  int m_initialDelayMsec;
};
}  // namespace Async
