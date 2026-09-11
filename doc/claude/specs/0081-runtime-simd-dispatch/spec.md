---
spec: 0081-runtime-simd-dispatch
title: Runtime SIMD Dispatch
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-09-11
author: Alex Spataru
---

# Spec 0081 — Runtime SIMD Dispatch

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio ships one x86-64 binary per platform, and that binary has to start on every
machine a user might own: a 2024 workstation and a fanless Celeron box on a test bench alike.
The shipped instruction-set baseline is therefore the conservative one (SSE4-class, the
x86-64-v2 level), and every vectorized kernel in the app is written to it. A machine with AVX2,
which is nearly every desktop and laptop CPU sold since 2013, runs the same 128-bit code as the
oldest supported one. Half of each vector register sits idle on the majority of installs.

The tempting fix, raising the baseline to AVX2 for everyone, is not available to this product.
Every Pentium Silver, Celeron and Atom sold through 2021 lacks AVX entirely, and those are
exactly the industrial and lab PCs that run a telemetry dashboard next to the equipment. A
binary that faults on launch there is a support incident, not an optimization. The only shape
that serves both audiences from one download is to carry more than one body per kernel and pick
at runtime.

There is also a support angle the maintainer cannot serve today. When a user reports a plot or
FFT that looks wrong, or a stall on a particular machine, there is no way to ask them to rule the
vector path in or out: the lane is fixed at compile time, and the scalar reference the kernels
are held bit-exact against is unreachable in a shipped build. A user-selectable level turns "try
it without SIMD" from a rebuild into a dropdown.

## Goals

- One shipped binary per platform uses the widest vector instruction set the running machine
  supports, and still starts and runs correctly on a machine that supports none of the wide
  ones.
- Every value a kernel produces is identical at every level. A user changing the level sees
  no difference in any plot, export, parse result or statistic, only in speed.
- The user can see which level is in use and can force a lower one, down to plain scalar code,
  from the Preferences dialog.
- The scalar reference lane the kernels are verified against is selectable in a shipped build,
  so it is available for support triage and as a live oracle.
- The choice can also be pinned for a single run from the command line, so headless and CI
  runs are deterministic regardless of the machine or the saved preference.

## Non-Goals

- No AVX-512 lane. The install base is small, mixed-generation frequency behaviour makes the
  win uncertain, and the mask-register model would be a third x86 idiom to hold bit-exact.
- No ARM levels beyond NEON. On 64-bit ARM, NEON is the baseline and SVE is out of scope; the
  ARM list is Scalar and NEON only.
- No per-kernel or per-widget selection. The level is one global setting.
- No change to what the kernels compute, to the set of kernels, or to the "reuse the kernels,
  never inline intrinsics at call sites" rule. This adds bodies, not behaviours.
- No promise about the hotpath benchmark number going up. The acquisition gate is dominated
  by work that is not kernel time; the expected wins are in plot, FFT and stream staging cost.
- No second build variant, no fat binary, no separate download per CPU generation.

## Requirements

1. **R1** — On x86-64 the application offers these kernel levels, from lowest to highest:
   **Scalar**, **SSE4** (the shipped baseline), **AVX2**. On 64-bit ARM it offers **Scalar**
   and **NEON**. Every shared vector kernel has a genuine body at every level its platform
   lists; no level is an alias of a lower one.
2. **R2** — At startup the application determines which levels the running machine supports. A
   level counts as supported only when both the CPU advertises it and the operating system has
   enabled the register state it needs. Scalar is always supported.
3. **R3** — The Preferences dialog, Startup tab, System section, gains a combobox labelled for
   kernel optimization. It lists **Auto** followed by every supported level. Levels the machine
   does not support are not shown.
4. **R4** — **Auto** is the default and resolves to the highest supported level. The Auto entry
   names the level it resolved to (for example "Auto (AVX2)"), so the user can read the active
   level without changing anything.
5. **R5** — The user may select any listed level. A selection is persisted across restarts and
   takes effect for all kernels.
6. **R6** — If a persisted selection names a level the current machine does not support (the
   settings were carried to a different computer), the application behaves as Auto and the
   combobox shows Auto. It never faults and never silently keeps the unsupported name.
7. **R7** — Every kernel produces bit-identical output at every level for every input,
   including NaN, both infinities, both signed zeros, subnormals, empty spans, spans shorter
   than one vector width, and spans whose length is not a multiple of the vector width. The
   existing documented exception stands: the sign of a min/max result when -0.0 and +0.0
   compare equal.
8. **R8** — Selecting a lower level never changes a displayed value, an exported file, a parsed
   frame or a statistic; only throughput may differ.
9. **R9** — A command-line option pins the level for that run. It overrides the persisted
   preference, is not saved, and is honoured by headless mode, the self-test suites and the
   hotpath benchmark. An unsupported value is reported and the run proceeds as Auto.
10. **R10** — Struck 2026-09-11 by the maintainer: no per-start level line. A rejected
    preference or pin is still logged; the Preferences combobox's Auto label is the report.
11. **R11** — A build that disables SIMD at compile time (the existing scalar-only switch) still
    builds and runs: it lists Scalar only and the combobox shows Auto (Scalar).

## Acceptance Criteria

- [ ] **AC1** (R1, R7) — A unit in the ctest tier runs every shared kernel at every level the
      build supports against the scalar reference over randomized inputs and the edge-case set
      in R7, comparing results bitwise. It passes on the x86-64 and ARM CI hosts.
- [ ] **AC2** (R2, R6) — The same unit forces each level through the public selection and
      confirms an unsupported level is refused and reported as Auto rather than accepted.
- [ ] **AC3** (R3, R4, R5) — Maintainer observation: on an AVX2 machine the combobox lists
      Auto (AVX2), Scalar, SSE4, AVX2; on an Apple Silicon machine it lists Auto (NEON), Scalar,
      NEON. Choosing a level, restarting, and reopening the dialog shows the same choice.
- [ ] **AC4** (R8) — Maintainer observation: with a live project on a plot, an FFT and a
      waterfall, switching between every listed level produces no visible change and an
      exported CSV of the same window is byte-identical across levels.
- [ ] **AC5** (R9) — `--benchmark-hotpath` runs with the level pinned to SSE4 and to Auto both
      clear the existing 256 kHz gate on the CI hosts; a run pinned to Scalar completes and
      reports its number (informational, not gated).
- [ ] **AC6** (R9) — The pinned-level option is exercised in CI so a regression in dispatch
      overhead on the baseline lane is caught, not only on the widest lane the runner happens
      to have.
- [ ] **AC7** (R10) — Struck with R10: a headless run with a bad `--simd` value logs the
      rejection and nothing else about the level.
- [ ] **AC8** (R11) — The scalar-only compile-time configuration builds and its ctest unit
      passes with the single Scalar level.
- [ ] **AC9** — The macOS Intel binary running under Rosetta 2 (how CI already runs it) starts,
      passes AC1 and clears the gate whether or not the translation layer advertises AVX2.

## Constraints & Invariants

- **Bit-exact per lane is the deciding constraint.** A wide body may only use the op classes the
  kernel contract already allows (byte compares and masks, integer ops, IEEE min/max compares,
  per-lane add and mul in the scalar's order, per-lane f64 to f32 converts). No fused
  multiply-add, no horizontal float reductions, no reassociation, no approximate
  transcendentals. AVX2 is selected without FMA for this reason.
- **The wide code must never execute on a machine that lacks it, and must never be visible to
  the compiler as ordinary code.** The dispatch decision is a call boundary the compiler cannot
  hoist across, on every supported toolchain (GCC, Clang, AppleClang, clang-cl, MSVC cl.exe,
  MinGW, IntelLLVM). No translation unit is compiled with a wide instruction set as a whole:
  link-time folding of a shared inline function compiled wide would put wide instructions in
  every caller.
- **No new per-element cost on the hotpath.** The level is read once per kernel call, at span
  granularity, never inside an inner loop, and reading it allocates nothing and takes no lock.
- **The 256 kHz acquisition gate must not regress on the baseline lane.** The dispatch must be
  free enough that SSE4 pinned scores the same as today's fixed SSE4 build.
- **No new dependency.** CPU detection uses what the compilers and operating systems already
  provide.
- **The scalar reference stays the oracle.** Any lane, including SSE4 and NEON as they exist
  today, is measured against the scalar body; the new test makes that check mechanical for
  every level instead of a reviewer's promise.
- **Level detection and selection carry no application singleton and run before the
  composition root**, because the self-test suites and the benchmark root need them too.
- **The combobox follows the existing System-section rows**: same label style, same
  persistence, same explanatory footnote pattern.

## Open Questions

- **Immediate or next-start effect?** The neighbouring System-section rows take effect at the
  next start. Because every level is bit-identical, switching live has no observable seam and
  is more useful for support triage, so this spec assumes the level applies immediately.
  Confirm, or fall back to next-start for symmetry with the existing rows.
- **Naming of the baseline entry.** "SSE4" is accurate and short; "SSE4.1" or "Baseline" are
  alternatives. This spec uses SSE4.
