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

/**
 * @file sanitizer_features.h
 * @brief Compile-time detection of the sanitizer a suite is built under.
 *
 * SS_TSAN_ACTIVE is 1 under ThreadSanitizer (GCC's __SANITIZE_THREAD__, Clang's __has_feature)
 * and 0 otherwise. The suites that replace the global operator new to count allocations gate the
 * replacement on it: TSan's runtime archive carries the whole new/delete family in one member,
 * and the moment a link set references an overload the suite does not replace (aligned new, from
 * any alignas member) lld pulls that member and reports the plain operator new as a duplicate
 * symbol (tst_script_cells, 2026-09-12). An allocation count is not a threading property, so those
 * suites QSKIP the counting case under TSan instead of linking the replacement.
 */

#if defined(__SANITIZE_THREAD__)
#  define SS_TSAN_ACTIVE 1
#elif defined(__has_feature)
#  if __has_feature(thread_sanitizer)
#    define SS_TSAN_ACTIVE 1
#  endif
#endif

#ifndef SS_TSAN_ACTIVE
#  define SS_TSAN_ACTIVE 0
#endif
