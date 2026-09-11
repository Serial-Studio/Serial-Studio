# Architecture — SIMD Kernels & Hotpath Optimization Macros

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before adding a bulk numeric loop or annotating a function for the optimizer. The
> `ss-hotpath` skill re-states these at edit time.

## Portable SIMD Kernels (`core/Core/DSPSimd.h`, spec 0021, moved by spec 0076)

**Portable SIMD kernels live in `core/Core/DSPSimd.h`** (`namespace DSP`, spec 0021): x86-64-v2
+ NEON lanes + reference scalar fallback, per-lane bit-exact versus the scalar loop (full
contract in the header). New bulk loops reuse these — never inline intrinsics at call sites.

## Runtime Lanes (spec 0081)

One binary carries every lane and picks per call. `core/Core/SimdLevel.h` holds the
process-wide `DSP::SimdLevel` (Scalar, Sse4, Avx2, Neon) as one relaxed atomic:
`DSP::activeSimdLevel()` is the load every kernel does once at the top of its body,
`DSP::setActiveSimdLevel()` refuses anything `DSP::supportedSimdLevels()` did not list, and the
list comes from a one-time CPUID/XGETBV probe (the OS must have enabled YMM state, which a
translation layer or an old kernel may not). Inside `DSPSimd.h` the SSE4 and NEON loops live in
the `SimdSse4` / `SimdNeon` helper namespaces; the AVX2 bodies live in `core/Core/DSPSimdAvx2.h`
(`SimdAvx2`), and the scalar tail that closes every kernel is the selectable Scalar lane, so the
oracle in `app/tests/dsp_scalar_ref.cpp` and the shipped Scalar lane are the same source lines.
The x86 dispatch is a chain, not an either/or: the AVX2 body runs only when at least one whole
256-bit block exists (`len >= 32` bytes, `n >= 8` floats), then the inline SSE4 helper finishes the
residue, then the scalar tail. A short span on an AVX2 host therefore never pays the out-of-line
call, and the 16..31-byte residue keeps the 16-byte lane. The reductions restart their seed, so
they pick one lane (`n >= 8` AVX2, else `n >= 4` SSE4). `simdAsciiDots16` has no AVX2 body: a
fixed 16-byte kernel cannot amortize a call boundary, so the AVX2 level runs its 128-bit lane.

The rules that keep a wide lane legal on a machine that lacks it:

- Every AVX2 body is `SS_NEVER_INLINE SS_TARGET_AVX2`, so the level compare in the caller is a
  call boundary and no TU is ever compiled with `-mavx2` / `/arch:AVX2` (a whole-TU flag plus
  LTO's COMDAT pick would put wide code in baseline callers). `SS_TARGET_AVX2` is `avx2` alone:
  never add `fma`, which would let the compiler contract multiply-add.
- Every AVX2 body calls `_mm256_zeroupper()` exactly once, after every 256-bit value is dead
  (both accumulators of a two-sided reduction are split into 128-bit halves first) and before
  any 128-bit op or return; GCC and Clang insert one, cl.exe does not. A zeroupper while another
  YMM accumulator is live is only correct on compilers that model the clobber (GCC 12+, LLVM).
- A lane helper hands back the index the next lane resumes at (or updates `lo` / `hi` by
  reference): AVX2 to the inline SSE4 residue, SSE4 or NEON to the scalar tail. The reductions
  enter a wide body only for `n >= 8` (`n >= 4` on the 128-bit lanes), so a short span is always
  the scalar loop.
- The level is read once per kernel call into a local, never per element and never re-read,
  and reading it allocates nothing and takes no lock. Every lane is bit-identical, so a switch
  from another thread mid-span is never observable and the store needs no ordering.

Selection surface: `Misc::SimdSettings` (`core/Ui/Misc/`, root-owned, no `instance()`) persists
the choice under the `App/SimdLevel` settings key as a stable id (`auto`, `scalar`, `sse4`,
`avx2`, `neon`), feeds the Preferences → Startup → System combobox (Auto is labelled with the
level it resolves to), and `Misc::SimdSettings::applyConfiguredLevel()` in `main()` resolves
pin > preference > auto before `runApplication()`, so the benchmark, selftest and headless roots
inherit it. `--simd <level>` pins one run and is never saved; an unsupported id falls back to
auto, and only that rejection is logged (the maintainer removed the per-start level line on
2026-09-11; the combobox's Auto label is the report).

**Accumulator chains (spec 0082).** The four reductions (`minF64`, `maxF64`, `minMaxF64`,
`finiteMinMaxPointF`) in every lane namespace take a `kChains` template parameter and production
calls `<2>`: two independent accumulators per lane so the compare latency of one chain overlaps
the other, folded chain 1 into chain 0 with the loop's own operand order before the unchanged
horizontal tail. Only min and max may split, because their result is order-independent, a NaN
element never wins in any chain and a NaN seed poisons every chain alike; the windowing, power
and interleave kernels keep one chain, since a sum or product reassociated across chains changes
bits. `app/tests/tst_dsp_reduction_bench.cpp` instantiates `<1>` against `<2>` from the same
source and prints the ratio per lane; it asserts only bit-identity with the scalar oracle.

Mechanical check: `app/tests/tst_dsp_kernels.cpp` carries a `level` column on every data table,
so each kernel is compared bitwise against the scalar oracle at every level the machine
supports; CI additionally gates `--benchmark-hotpath --min-fps 256000 --simd sse4` on the x86-64
jobs, because the runners take the AVX2 lane on Auto and the baseline lane would otherwise never
be measured.

## Hotpath Optimization Macros (`core/Core/HotpathOptimization.h`)

**Hotpath optimization macros live in `core/Core/HotpathOptimization.h`**
(`SS_FORCE_INLINE`, `SS_FLATTEN`, `SS_HOT`/`SS_COLD`, `SS_RESTRICT`, `SS_ASSUME`, ...); the
header documents the toolchain cascade. Annotate `.h` declaration and `.cpp` definition in
lockstep. Never add a fast-math / no-unwind / GCC `optimize("...")` macro (breaks the
IEEE-stable + Lua-unwind invariants). `SS_ASSUME` must restate a guard that already ran,
never a precondition on a parsed frame. `datasets+publish` is ~70-80% of per-frame time —
gate any change with `--benchmark-hotpath`.

`SS_ASSUME` stays the zero-branch kernel spelling; on the per-frame/per-cell kernels use
`SS_ASSERT_HOTPATH(cond)` instead of `SS_ASSERT` — it compiles out of release (even the
pass-path branch is measurable at rate; the 2026-07 wholesale swap cost ~5% throughput),
admissible only where the condition restates a guard that provably already ran, never on
device bytes; the blocking `hotpath-assert-scope` lint pins it to the hotpath TUs. Full
assertion rules live in [../code-style.md](../code-style.md).
