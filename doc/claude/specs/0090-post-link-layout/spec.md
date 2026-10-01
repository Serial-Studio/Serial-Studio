---
spec: 0090-post-link-layout
title: Post-Link Binary Layout Optimization
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-10-01
author: Alex Spataru
---

# Spec 0090 — Post-Link Binary Layout Optimization

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio ships a single large executable: all seven first-party libraries and the
vendored third-party stacks (OPC UA, mbedTLS, LuaJIT, S7comm, IEC-104, ...) are statically
linked. PGO shapes the layout of basic blocks *within* each function, but the cross-function
placement of code in the final image is essentially link order — hot parse/publish/dashboard
code ends up interleaved with cold driver, dialog, and error-path code across the whole text
segment. That is exactly the problem Microsoft's BBT tool ("Lego") solved for Windows in the
90s and that post-link optimizers solve today: regroup hot code onto few contiguous pages so
the i-cache, iTLB, and startup paging touch a fraction of the image. The modern tool for
this, LLVM BOLT, is designed to run on top of a PGO+LTO binary and is reported by its
authors to add 2–8% on large applications beyond what PGO alone achieves — and our release
pipeline already has everything it needs: a two-stage PGO flow with representative training
workloads and a hard throughput gate at the end.

Today none of our three release platforms gets deliberate whole-binary layout: Linux gets
none at all, Windows may get profile-driven function ordering from its linker but nobody has
verified it is actually active, and macOS (where LTO is forced off for toolchain reasons)
gets the least of the three. The deciding constraint for any fix: hosted CI runners are VMs
with no PMU/LBR access, so a post-link optimizer cannot profile by sampling — it must
self-instrument and reuse the training workloads the PGO flow already runs. BOLT supports
that mode, but only for ELF; Windows and macOS therefore need the equivalent effect at link
time rather than post-link.

## Goals

- Linux release binaries (x86_64 and arm64) are layout-optimized post-link by BOLT, trained
  on the same workloads that train PGO, with no change to what the release artifacts are or
  how they are signed and packaged.
- Windows release builds demonstrably apply profile-driven function ordering at link time —
  "demonstrably" meaning CI proves it is active, not that we assume the linker does it.
- macOS release builds apply a hot-function order file, derived from the PGO profile the
  pipeline already produces, at link time.
- Every existing release gate (the 256 kHz hotpath gate, in-app selftests, packaging,
  signing) runs against the final optimized binary — the thing we ship is the thing we gate.
- The maintainer can see, per CI run, that the layout stage ran and what it did (a stats
  artifact), without any expectation of reading a performance delta from CI numbers.

## Non-Goals

- **Shared libraries.** The executable already contains all first-party code; the shared
  mimalloc on Linux and the bundled Qt libraries are explicitly out of scope (prebuilt Qt
  binaries lack relocations and are third-party risk; mimalloc is too small to matter).
  Revisit only if later profiling shows otherwise.
- **Swapping the macOS linker.** lld's Mach-O port could do automatic call-graph sorting,
  but we will not introduce a new linker on the platform where we already carry Apple
  toolchain unwind workarounds. Order file with Apple's own linker only.
- **Sampling-based profiling** (perf/LBR) in CI, now or as a fallback — runners cannot do it.
- **Recompile-based layout schemes** (Propeller / basic-block-sections) — they would double
  the already-long PGO pipeline for less benefit than BOLT on ELF.
- **Proving the speedup in CI.** Run-to-run benchmark spread on hosted runners is ±45–56%;
  the throughput gate stays pass/fail. Any performance claim is made from local measurement
  or the historical-band method, never from a single CI run.
- **No user-facing behavior change of any kind.** The optimized binary is functionally
  identical; this is invisible to users except as speed.

## Requirements

1. **R1** — On both Linux release jobs, after the PGO-optimized build completes, the binary
   is instrumented by BOLT, re-run through the existing training workloads (the hotpath
   benchmark run and the big-project run), and rewritten by BOLT using the collected
   profile. The rewritten binary replaces the original for everything downstream.
2. **R2** — The 256 kHz hotpath gate, the packaging step, and GPG signing all consume the
   BOLT-rewritten binary. There is no code path by which a non-optimized binary reaches a
   release artifact on Linux.
3. **R3** — The Linux post-link stage is fail-hard: if BOLT, its instrumentation run, or the
   profile collection fails, the CI job fails. We never silently fall back to shipping the
   un-rewritten binary, so every shipped build has the same layout treatment
   (predictability over availability).
4. **R4** — The rewritten binary passes a functional smoke beyond the benchmark: the in-app
   selftest suite runs against it in CI and must pass, explicitly covering a script-engine
   error path (exception unwind across the Lua VM is the behavior most sensitive to a
   binary rewriter corrupting unwind metadata).
5. **R5** — Windows release links order functions by PGO call-graph hotness, and the CI job
   contains a check that fails if that ordering silently stops happening (e.g. the profile
   data stops reaching the linker). If verification shows ordering is not currently active,
   enabling it is in scope for this spec.
6. **R6** — The macOS release link consumes a hot-function order file derived from the PGO
   profile the pipeline already produces. Generation of an empty or unusable order file
   fails the job rather than silently linking without it.
7. **R7** — The BOLT tool version is pinned like every other CI toolchain component; a tool
   update is a reviewable diff, not a floating latest.
8. **R8** — Each release job uploads a layout-stage report artifact (BOLT's dyno-stats on
   Linux; the ordering evidence on Windows/macOS) so a maintainer can audit what the stage
   did for any given release.

## Acceptance Criteria

- [ ] **AC1** — Both Linux release jobs pass end-to-end with the BOLT stage inserted, and
  the `--benchmark-hotpath --min-fps 256000` gate passes against the rewritten binary.
- [ ] **AC2** — The in-app selftest tier passes against the rewritten binary on both Linux
  arches, including a case that raises and recovers from a Lua script error.
- [ ] **AC3** — Deliberately breaking the stage in a test branch (e.g. removing the profile)
  fails the job — verifying R3/R6 fail-hard behavior on Linux and macOS.
- [ ] **AC4** — The Windows job's link log (or an equivalent explicit check) proves
  call-graph profile ordering was applied, and that check is a permanent CI assertion.
- [ ] **AC5** — The macOS release job generates a non-empty order file from the PGO profile,
  the linker consumes it without error, and the arm64 slice's 256 kHz gate still passes.
- [ ] **AC6** — Release artifacts (AppImage, installer, DMG) are built from the optimized
  binaries, confirmed by the stage ordering in the CI logs; signing succeeds on all three
  platforms afterward.
- [ ] **AC7** — The layout report artifact (R8) is present on every release-job run.

## Constraints & Invariants

- **Unwind correctness is the hard safety constraint.** Lua errors throw across the VM
  stack; the rewritten/reordered binary must preserve working exception unwind everywhere.
  This is why AC2 names a Lua error path explicitly.
- **Hardening must survive the rewrite.** Linux x86_64 ships with CET (`endbr64` landing
  pads), arm64 with branch protection (BTI); the post-link stage must not strip or
  invalidate either, nor weaken RELRO/ASLR properties of the artifact.
- **No codegen semantics change.** Layout only — floating-point behavior, the IEEE-stable
  math guarantee, and bit-stable telemetry output are untouched by construction; any tool
  option that would alter instruction semantics is out of bounds.
- **The PGO stage-identity rule extends to the new stage.** The flags both PGO stages must
  agree on remain identical, and any link-side additions needed by the post-link stage must
  not reintroduce GENERATE/USE drift.
- **No new runtime dependency** in any shipped artifact; the optimizer is a build-time tool
  only.
- **No reproducibility regression beyond PGO's existing one.** The stage must be
  deterministic given the same input binary and profile; profile variance across runs is
  accepted exactly as it already is for PGO.
- **CI wall-clock stays sane.** The added training re-runs roughly repeat the existing
  training time once more on Linux; the stage must not add more than that order of cost.

## Open Questions

- **arm64 BOLT maturity.** BOLT on AArch64 (with BTI landing pads) is its least
  battle-tested path. Ship both arches in one pass, or land x86_64 first and let arm64
  follow after a canary period? Recommendation: one pass — R4's selftest coverage plus AC3
  is the canary — but the maintainer may prefer staging.
- **macOS universal-binary asymmetry.** Only the arm64 slice is PGO-trained; the x86_64
  slice builds without PGO. Does the order file apply to the arm64 slice only, or do we
  apply the arm64-derived hot order to the x86_64 slice too on the theory that hotness is
  arch-independent? Recommendation: arm64-only first; the x86_64 slice is the legacy path.
- **Windows current state.** R5 is written as "verify, then enable if absent" because we do
  not yet know whether function-granularity sections and the profile actually reach the
  linker today. The plan phase must answer this empirically before committing to the size
  of the Windows work.
- **Selftest-in-CI placement.** The Linux jobs currently gate on the benchmark only; R4
  adds a selftest run. Confirm the maintainer is happy adding that (small) wall-clock cost
  to the release jobs rather than only to the test jobs.
