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

#include "DSP.h"

namespace UI::TimeRingSizing {

[[nodiscard]] const DSP::EnvelopeRing& emptyRing();
[[nodiscard]] double historyWindowSec(double plotTimeRangeSec) noexcept;
[[nodiscard]] DSP::RingCapacity initialCapacity(double windowSec, double rateHz) noexcept;
[[nodiscard]] DSP::RingMode initialMode(double windowSec, double rateHz) noexcept;
[[nodiscard]] DSP::EnvelopeRing historyRing(double plotTimeRangeSec,
                                            double rateHz,
                                            DSP::RingBudget& budget);
[[nodiscard]] DSP::RingCapacity sweepCapacity(const DSP::EnvelopeRing& history,
                                              double sweepWindowSec) noexcept;
void shapeSweep(DSP::SweepEngine& engine,
                int curves,
                const DSP::EnvelopeRing& densest,
                double sweepWindowSec,
                DSP::RingBudget& budget);
void growSweep(DSP::SweepEngine& engine,
               const DSP::EnvelopeRing& densest,
               double sweepWindowSec,
               DSP::RingBudget& budget);

}  // namespace UI::TimeRingSizing
