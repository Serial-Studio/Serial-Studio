# Architecture — SIMD Kernels & Hotpath Optimization Macros

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before adding a bulk numeric loop or annotating a function for the optimizer. The
> `ss-hotpath` skill re-states these at edit time.

## Portable SIMD Kernels (`core/Core/DSPSimd.h`, spec 0021, moved by spec 0076)

**Portable SIMD kernels live in `core/Core/DSPSimd.h`** (`namespace DSP`, spec 0021): x86-64-v2
+ NEON lanes + reference scalar fallback, per-lane bit-exact versus the scalar loop (full
contract in the header). New bulk loops reuse these — never inline intrinsics at call sites.

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
