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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QtGlobal>

/**
 * @file HotpathOptimization.h
 * @brief Per-function and per-loop optimization hints for the data hotpath.
 *
 * Each macro lowers to the correct spelling on every production toolchain
 * (cmake/Optimization.cmake) and degrades to a clean no-op where a compiler has no equivalent.
 *
 * Detection order is load-bearing: __clang__ is tested first because clang-cl (Windows MSVC ABI)
 * and IntelLLVM (oneAPI icx) both define __clang__ as well as _MSC_VER / __INTEL_LLVM_COMPILER, so
 * testing _MSC_VER first would route clang-cl to the MSVC no-op and drop the GNU attribute. Only
 * genuine MSVC cl.exe reaches the _MSC_VER branch.
 *
 * Hard rule: nothing here may touch floating-point semantics or unwind tables. No fast-math, no
 * -ffp-contract=fast, no float_control(fast), no nounwind, no GCC optimize("...") strings:
 * telemetry output must stay bit-stable, and Lua throws across the VM so every frame needs unwind
 * metadata.
 */

//--------------------------------------------------------------------------------------------------
// Inlining control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Forces inlining of a tiny leaf. Use only on small, hot, header-defined helpers; applied to
 *        anything large it bloats the I-cache. Never combine with SS_FLATTEN on the same function.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define SS_FORCE_INLINE __forceinline
#else
#  define SS_FORCE_INLINE inline
#endif

/**
 * @brief Keeps a cold helper out of a hot caller's body. Placement tool for rarely-taken paths
 *        (pool-exhaustion warning, watchdog-timeout notification); never on the per-dataset path.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_NEVER_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define SS_NEVER_INLINE __declspec(noinline)
#else
#  define SS_NEVER_INLINE
#endif

/**
 * @brief Inlines a function's entire callee tree into its body in one pass (caller-side inlining).
 *        Use on shallow, leaf-heavy hot functions only; flatten is transitive and bloats deep
 * trees. MSVC cl.exe has no equivalent (its /Ob3 + /GL already inline aggressively) -- no-op there.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_FLATTEN __attribute__((flatten))
#else
#  define SS_FLATTEN
#endif

//--------------------------------------------------------------------------------------------------
// Section placement hints
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks a function hot: packs it into the hot text section for I-cache locality and biases
 *        block layout in its favor. A hint, not a correctness lever; PGO supersedes it where it has
 *        samples, so the shipped binary leans on profiles and this mainly helps non-PGO builds.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_HOT __attribute__((hot))
#else
#  define SS_HOT
#endif

/**
 * @brief Marks a function cold: exiles it to the cold section away from hot code and biases branch
 *        prediction against entering it. Use only on provably rare helpers; pairs with
 * [[unlikely]].
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_COLD __attribute__((cold))
#else
#  define SS_COLD
#endif

//--------------------------------------------------------------------------------------------------
// Aliasing
//--------------------------------------------------------------------------------------------------

/**
 * @brief No-alias promise on a pointer parameter, letting the optimizer hoist loads across a loop.
 *        This is a correctness contract, not a hint: undefined behavior if the buffers can overlap.
 *        Apply only where the no-alias invariant is provable at the call site.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define SS_RESTRICT __restrict
#else
#  define SS_RESTRICT
#endif

//--------------------------------------------------------------------------------------------------
// Optimizer assumptions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Feeds an invariant to the optimizer in release; asserts it in debug. The predicate must be
 *        side-effect free (it is not evaluated under NDEBUG) and must restate a fact a preceding
 *        runtime guard already enforced -- never a precondition on untrusted input.
 */
#if defined(NDEBUG)
#  if defined(__clang__)
#    define SS_ASSUME(expr) __builtin_assume(expr)
#  elif defined(_MSC_VER)
#    define SS_ASSUME(expr) __assume(expr)
#  elif defined(__GNUC__) && (__GNUC__ >= 13)
#    define SS_ASSUME(expr) [[assume(expr)]]
#  elif defined(__GNUC__)
#    define SS_ASSUME(expr)        \
      do {                         \
        if (!(expr))               \
          __builtin_unreachable(); \
      } while (0)
#  else
#    define SS_ASSUME(expr) ((void)0)
#  endif
#else
#  define SS_ASSUME(expr) Q_ASSERT(expr)
#endif

//--------------------------------------------------------------------------------------------------
// Branch hints (expression context)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Expression-context "usually true" hint for ternaries and bare guards the C++20 [[likely]]
 *        statement attribute cannot reach. Prefer [[likely]] where a statement fits (PGO supersedes
 *        both on the shipped binary); never double-annotate one branch with both forms.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#else
#  define SS_LIKELY(expr) (expr)
#endif

/**
 * @brief Expression-context "usually false" twin of SS_LIKELY; prefer the [[unlikely]] statement
 *        attribute where a statement fits, and never combine the two on one branch.
 */
#if defined(__clang__) || defined(__GNUC__)
#  define SS_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#else
#  define SS_UNLIKELY(expr) (expr)
#endif

//--------------------------------------------------------------------------------------------------
// Loop control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Disables unrolling for the following loop. The per-dataset publish loops have short trip
 *        counts with large inlined bodies, where -O3 unrolling only bloats I-cache (more so after
 *        SS_FLATTEN fattens the body). __clang__ is tested first so clang-cl / IntelLLVM keep the
 *        clang loop pragma instead of falling to the no-op.
 */
#if defined(__clang__)
#  define SS_NO_UNROLL _Pragma("clang loop unroll(disable)")
#elif defined(__GNUC__)
#  define SS_NO_UNROLL _Pragma("GCC unroll 1")
#else
#  define SS_NO_UNROLL
#endif
