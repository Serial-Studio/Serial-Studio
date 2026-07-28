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
#include <QtGlobal>

#include "DataModel/HotpathOptimization.h"

/**
 * @file SSAssert.h
 * @brief Release-safe assertions that carry their own recovery action.
 *
 * Q_ASSERT compiles out under QT_NO_DEBUG, so every precondition it guards is unchecked in the
 * shipped binary. In an application that parses untrusted device bytes at 256 kHz that is the wrong
 * default: the assert reads as a guard while the release build performs the unchecked subscript,
 * shift, or divide anyway. SS_ASSERT keeps the debug abort (the developer signal) and adds a
 * release path that reports the violation once per source site and then executes a caller-supplied
 * recovery action.
 *
 * The recovery action is part of the assertion, not an afterthought: an assertion whose author
 * cannot name what the code should do when it fails is an assertion whose failure has no defined
 * behavior. SS_ASSERT_LOG is the explicit escape for invariants where no recovery is meaningful.
 * SS_ASSERT_HOTPATH is the debug-only tier for the per-frame/per-cell kernels, where even the
 * pass-path branch is measurable at rate (the 2026-07 campaign's wholesale swap cost ~5% of
 * hotpath throughput); it compiles out under QT_NO_DEBUG. SS_ASSUME (HotpathOptimization.h) stays
 * the spelling for a guard that provably already ran inside a kernel that must carry zero
 * branches.
 *
 * Contract, in order of how often it is violated:
 *
 * 1. The condition is evaluated in every configuration, so it MUST be side-effect free and cheap.
 *    A predicate that walks a container or allocates belongs behind a `// code-verify off` fence
 *    as a plain Q_ASSERT, not here.
 * 2. The action must be side-effect-complete on its own: it runs instead of, not before, the code
 *    the failed condition was guarding. Never write an SS_ASSERT whose action falls through into
 *    the statements the condition protects.
 * 3. `continue` and `break` are NOT valid actions. The macro wraps its body in a do/while(0), so a
 *    loop-control statement would bind to that wrapper and silently do nothing. Spell a loop skip
 *    as `SS_ASSERT_LOG(cond); if (!(cond)) continue;` instead, where the guard is visible.
 * 4. The action is a single statement and may not contain a top-level comma (no variadic macros
 *    per NASA Power of Ten rule 8). Wrap a multi-statement or comma-bearing recovery in braces:
 *    `SS_ASSERT(ok, { lua_pushnil(L); return 1; })`.
 *
 * Keying is on QT_NO_DEBUG, matching Q_ASSERT itself, so a plain Release build behaves the same
 * way the assert it replaced did. Setting SS_ASSERT_NONFATAL in the environment makes a debug
 * build take the recovery branch instead of aborting, which is how the recovery paths get
 * exercised under test rather than shipping unrun.
 */

namespace SSAssertDetail {
/**
 * @brief Reports a soft-assertion failure through the serialstudio.assert logging category. Kept
 *        out of line and cold so the failure path adds no instructions to a hot caller's body.
 */
SS_COLD SS_NEVER_INLINE void reportSoftAssert(const char* expr,
                                              const char* file,
                                              int line,
                                              const char* func);

/**
 * @brief Whether a failed assertion aborts the process: true in debug builds unless
 *        SS_ASSERT_NONFATAL is set in the environment, always false under QT_NO_DEBUG.
 */
[[nodiscard]] bool softAssertIsFatal();
}  // namespace SSAssertDetail

//--------------------------------------------------------------------------------------------------
// Reporting primitive
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports one failure per source site. The per-site latch is a relaxed atomic exchange so a
 *        256 kHz failure loop cannot flood the log and so ThreadSanitizer sees no race between the
 *        GUI thread and the loader / database / USB worker threads. It is constant-initialized, so
 *        the pass path touches neither the latch nor a guard variable.
 */
#define SS_ASSERT_REPORT(expr_text)                                                   \
  do {                                                                                \
    static std::atomic<bool> ss_assert_seen{false};                                   \
    if (!ss_assert_seen.exchange(true, std::memory_order_relaxed))                    \
      ::SSAssertDetail::reportSoftAssert(expr_text, __FILE__, __LINE__, Q_FUNC_INFO); \
  } while (0)

//--------------------------------------------------------------------------------------------------
// Assertions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks a precondition and runs @p action when it fails; aborts first in debug builds
 *        unless SS_ASSERT_NONFATAL is set. Cost when the condition holds is one correctly-predicted
 *        not-taken branch: no allocation, no atomic, no call.
 */
#define SS_ASSERT(cond, action)                  \
  do {                                           \
    if (SS_UNLIKELY(!(cond))) {                  \
      SS_ASSERT_REPORT(#cond);                   \
      if (::SSAssertDetail::softAssertIsFatal()) \
        qt_assert(#cond, __FILE__, __LINE__);    \
      action;                                    \
    }                                            \
  } while (0)

/**
 * @brief Checks an invariant that has no meaningful recovery: reports once and proceeds. Reach for
 *        this only after failing to name a recovery action; SS_ASSERT is the default.
 */
#define SS_ASSERT_LOG(cond)                      \
  do {                                           \
    if (SS_UNLIKELY(!(cond))) {                  \
      SS_ASSERT_REPORT(#cond);                   \
      if (::SSAssertDetail::softAssertIsFatal()) \
        qt_assert(#cond, __FILE__, __LINE__);    \
    }                                            \
  } while (0)

/**
 * @brief Debug-only invariant for the per-frame/per-cell kernels: compiles out entirely under
 *        QT_NO_DEBUG (Qt's own parsed-but-unevaluated idiom, so variables the condition names stay
 *        referenced). Admissible exactly where SS_ASSUME is admissible -- the condition restates a
 *        guard that provably already ran -- but without SS_ASSUME's optimizer promise, so a wrong
 *        one costs a missed debug abort instead of a miscompile. Never use it on a condition
 *        derived from device bytes: that class keeps SS_ASSERT and its release recovery. The
 *        hotpath-assert-scope lint pins this macro to the hotpath TUs.
 */
#ifdef QT_NO_DEBUG
#  define SS_ASSERT_HOTPATH(cond) static_cast<void>(false && (cond))
#else
#  define SS_ASSERT_HOTPATH(cond) SS_ASSERT_LOG(cond)
#endif
