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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <chrono>
#  include <QtGlobal>

namespace Sessions {

/**
 * @brief Maps recorded timestamps onto the steady clock the pipeline stamps frames with. One
 *        anchor pins a recorded instant to a real one, and every later row is that anchor plus its
 *        own recorded delta, so the recording owns replay time (spec 0020). Re-anchoring on each
 *        play/seek keeps a scrubbed session from replaying its rows into the past.
 */
class ReplayClock {
public:
  explicit ReplayClock();

  [[nodiscard]] double baseRowSeconds() const noexcept;
  [[nodiscard]] std::chrono::steady_clock::time_point base() const noexcept;
  [[nodiscard]] std::chrono::steady_clock::time_point timestampFor(qint64 timestampNs) const;

  void anchor(double rowSeconds);
  void anchorAt(const std::chrono::steady_clock::time_point& base, double rowSeconds);

private:
  double m_baseRowSeconds;
  std::chrono::steady_clock::time_point m_base;
};

}  // namespace Sessions

#endif
