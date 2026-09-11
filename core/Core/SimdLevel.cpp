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

#include "Core/SimdLevel.h"

#include <array>
#include <cstddef>

#include "Core/SSAssert.h"

#if !defined(SS_SIMD_DISABLE) && (defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64))
#  define SS_SIMD_LEVEL_X86 1
#  if defined(_MSC_VER) && !defined(__clang__)
#    include <intrin.h>
#  else
#    include <cpuid.h>
#  endif
#elif !defined(SS_SIMD_DISABLE) && (defined(__aarch64__) || defined(_M_ARM64))
#  define SS_SIMD_LEVEL_NEON 1
#endif

static constexpr std::size_t kMaxLevels = 4;

namespace DSP::SimdLevelDetail {
struct SupportedLevels {
  std::array<SimdLevel, kMaxLevels> levels;
  std::size_t count;
};

struct CpuidRegs {
  quint32 eax;
  quint32 ebx;
  quint32 ecx;
  quint32 edx;
};
}  // namespace DSP::SimdLevelDetail

using DSP::SimdLevelDetail::CpuidRegs;
using DSP::SimdLevelDetail::SupportedLevels;

#if defined(SS_SIMD_LEVEL_X86)

/**
 * @brief CPUID leaf/subleaf through the one intrinsic each x86 toolchain guarantees without flags.
 */
[[nodiscard]] static CpuidRegs cpuid(const quint32 leaf, const quint32 subleaf) noexcept
{
#  if defined(_MSC_VER) && !defined(__clang__)
  int regs[4] = {0, 0, 0, 0};
  __cpuidex(regs, static_cast<int>(leaf), static_cast<int>(subleaf));
  return {static_cast<quint32>(regs[0]),
          static_cast<quint32>(regs[1]),
          static_cast<quint32>(regs[2]),
          static_cast<quint32>(regs[3])};
#  else
  unsigned int eax = 0;
  unsigned int ebx = 0;
  unsigned int ecx = 0;
  unsigned int edx = 0;
  __cpuid_count(leaf, subleaf, eax, ebx, ecx, edx);
  return {eax, ebx, ecx, edx};
#  endif
}

/**
 * @brief XCR0, the OS-enabled register-state mask. Only legal once CPUID.1:ECX.OSXSAVE is set.
 */
[[nodiscard]] static quint64 xgetbv0() noexcept
{
#  if defined(_MSC_VER) && !defined(__clang__)
  return static_cast<quint64>(_xgetbv(0));
#  else
  quint32 eax = 0;
  quint32 edx = 0;
  __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
  return (static_cast<quint64>(edx) << 32) | eax;
#  endif
}

/**
 * @brief AVX2 counts as usable only when the CPU advertises AVX and AVX2 and the OS has enabled
 *        the XMM and YMM state (XCR0 bits 1 and 2), which is what a translation layer or an old
 *        kernel would leave unset.
 */
[[nodiscard]] static bool detectAvx2() noexcept
{
  constexpr quint32 kExtendedFeaturesLeaf = 7;
  constexpr quint32 kOsxsaveBit           = 1u << 27;
  constexpr quint32 kAvxBit               = 1u << 28;
  constexpr quint32 kAvx2Bit              = 1u << 5;
  constexpr quint64 kYmmState             = 0x6;

  const CpuidRegs leaf0 = cpuid(0, 0);
  if (leaf0.eax < kExtendedFeaturesLeaf)
    return false;

  const CpuidRegs leaf1 = cpuid(1, 0);
  if ((leaf1.ecx & kOsxsaveBit) == 0 || (leaf1.ecx & kAvxBit) == 0)
    return false;

  if ((xgetbv0() & kYmmState) != kYmmState)
    return false;

  return (cpuid(kExtendedFeaturesLeaf, 0).ebx & kAvx2Bit) != 0;
}

#endif

/**
 * @brief Runs the probe once; the list is ascending so the last entry is the best level.
 */
[[nodiscard]] static SupportedLevels probeSupportedLevels() noexcept
{
  SupportedLevels result{};
  result.levels[result.count++] = DSP::SimdLevel::Scalar;
#if defined(SS_SIMD_LEVEL_X86)
  result.levels[result.count++] = DSP::SimdLevel::Sse4;
  if (detectAvx2())
    result.levels[result.count++] = DSP::SimdLevel::Avx2;
#elif defined(SS_SIMD_LEVEL_NEON)
  result.levels[result.count++] = DSP::SimdLevel::Neon;
#endif
  SS_ASSERT(result.count >= 1 && result.count <= kMaxLevels, result.count = 1);
  return result;
}

/**
 * @brief The probe result, computed on first use and never on a kernel path.
 */
[[nodiscard]] static const SupportedLevels& supported() noexcept
{
  static const SupportedLevels s_supported = probeSupportedLevels();
  return s_supported;
}

//--------------------------------------------------------------------------------------------------
// Selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs @p level for every subsequent kernel call; refuses and leaves the active level
 *        untouched when the machine does not support it.
 */
bool DSP::setActiveSimdLevel(const SimdLevel level) noexcept
{
  if (!isSimdLevelSupported(level))
    return false;

  SimdLevelDetail::s_active_level.store(static_cast<quint8>(level), std::memory_order_relaxed);
  return true;
}

/**
 * @brief Every level this machine can run, ascending; Scalar is always first.
 */
std::span<const DSP::SimdLevel> DSP::supportedSimdLevels() noexcept
{
  const SupportedLevels& list = supported();
  const std::span<const SimdLevel> scalar_only(list.levels.data(), 1);
  SS_ASSERT(list.count >= 1 && list.count <= kMaxLevels, return scalar_only);
  SS_ASSERT(list.levels[0] == SimdLevel::Scalar, return scalar_only);
  return {list.levels.data(), list.count};
}

/**
 * @brief The widest supported level, which is what Auto resolves to.
 */
DSP::SimdLevel DSP::bestSupportedSimdLevel() noexcept
{
  const SupportedLevels& list = supported();
  SS_ASSERT(list.count >= 1, return SimdLevel::Scalar);
  return list.levels[list.count - 1];
}

/**
 * @brief Whether @p level is in the supported list (CPU and OS both agree).
 */
bool DSP::isSimdLevelSupported(const SimdLevel level) noexcept
{
  const SupportedLevels& list = supported();
  SS_ASSERT(list.count <= kMaxLevels, return level == SimdLevel::Scalar);
  for (std::size_t i = 0; i < list.count; ++i)
    if (list.levels[i] == level)
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Stable ids (persisted preference, --simd pin)
//--------------------------------------------------------------------------------------------------

/**
 * @brief The stable lowercase id a level is persisted and pinned as: settings files, the --simd
 *        pin and the CI gates carry these tokens, and the settings object lowercases its input
 *        before matching, so they must stay lowercase. User-facing spelling is the label.
 */
std::string_view DSP::simdLevelId(const SimdLevel level) noexcept
{
  constexpr std::array<std::string_view, kMaxLevels> kIds = {"scalar", "sse4", "avx2", "neon"};
  const auto index                                        = static_cast<std::size_t>(level);
  SS_ASSERT(index < kIds.size(), return kIds[0]);
  return kIds[index];
}

/**
 * @brief Inverse of simdLevelId(); an unknown id parses to nothing rather than to a default.
 */
std::optional<DSP::SimdLevel> DSP::parseSimdLevelId(const std::string_view id) noexcept
{
  constexpr std::array<SimdLevel, kMaxLevels> kAll = {
    SimdLevel::Scalar, SimdLevel::Sse4, SimdLevel::Avx2, SimdLevel::Neon};
  static_assert(kAll.size() == kMaxLevels, "every level needs an id");
  for (const SimdLevel level : kAll)
    if (simdLevelId(level) == id)
      return level;

  return std::nullopt;
}
