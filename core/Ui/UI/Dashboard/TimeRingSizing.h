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

#include <optional>

#include "DSP.h"

namespace UI::TimeRingSizing {

[[nodiscard]] double historyWindowSec(double plotTimeRangeSec) noexcept;
[[nodiscard]] DSP::RingCapacity frameLaneCapacity(double windowSec) noexcept;
[[nodiscard]] DSP::RingCapacity streamLaneCapacity(double windowSec, double rateHz) noexcept;
[[nodiscard]] DSP::EnvelopeRing historyRing(double plotTimeRangeSec, double rateHz);
[[nodiscard]] std::optional<DSP::RingCapacity> grownCapacity(int currentSlots,
                                                             double windowSec,
                                                             double samplePeriodSec) noexcept;

}  // namespace UI::TimeRingSizing
