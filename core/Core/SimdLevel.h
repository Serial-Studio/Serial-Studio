/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <atomic>
#include <optional>
#include <QtGlobal>
#include <span>
#include <string_view>

/**
 * @file SimdLevel.h
 * @brief The process-wide kernel lane selector behind DSPSimd.h (spec 0081). One shipped binary
 *        carries a Scalar, an SSE4 and an AVX2 body per kernel on x86-64 (Scalar and NEON on
 *        aarch64); the machine is probed once, the user may pin any supported level, and every
 *        kernel reads the choice once per call. Every level is bit-identical by contract, so a
 *        stale read on another thread is never observable and the store needs no ordering.
 */

namespace DSP {

/**
 * @brief Kernel lanes in ascending width. The numeric values are private to the process; the
 *        persisted and command-line form is the id returned by simdLevelId().
 */
enum class SimdLevel : quint8 {
  Scalar = 0,
  Sse4   = 1,
  Avx2   = 2,
  Neon   = 3
};

namespace SimdLevelDetail {
inline std::atomic<quint8> s_active_level{static_cast<quint8>(SimdLevel::Scalar)};
}  // namespace SimdLevelDetail

/**
 * @brief The lane every kernel selects on its next call. One relaxed load, safe from any thread.
 */
[[nodiscard]] inline SimdLevel activeSimdLevel() noexcept
{
  return static_cast<SimdLevel>(SimdLevelDetail::s_active_level.load(std::memory_order_relaxed));
}

[[nodiscard]] bool setActiveSimdLevel(SimdLevel level) noexcept;
[[nodiscard]] std::span<const SimdLevel> supportedSimdLevels() noexcept;
[[nodiscard]] SimdLevel bestSupportedSimdLevel() noexcept;
[[nodiscard]] bool isSimdLevelSupported(SimdLevel level) noexcept;
[[nodiscard]] std::string_view simdLevelId(SimdLevel level) noexcept;
[[nodiscard]] std::optional<SimdLevel> parseSimdLevelId(std::string_view id) noexcept;

}  // namespace DSP
