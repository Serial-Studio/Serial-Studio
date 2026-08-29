---
spec: 0032-cpp-unit-tests
title: C++ unit-test tier plus build presets
status: done          # closed 2026-08-20
created: 2026-07-25
author: Claude (drafted with Alex)
---

# Spec 0032 — C++ unit-test tier plus build presets

> **2026-08-20 update:** `CMakePresets.json` was removed from the repo — the maintainer
> prefers hand-written `cmake -B ... -D...` configures. The unit-test tier itself stands;
> the CI `unit` job now inlines the former `unit-ci` flags directly in `ci.yml`, and local
> runs use `cmake -B build/dev -DSS_BUILD_TESTS=ON` + `ctest --test-dir build/dev`. Preset
> references below (R7, AC6) are historical.

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R3. Approved sequencing places this in the
> "start now, independent" group, and spec 0030's R4 (session context) depends on it —
> a safety net has to exist before a context is threaded through the composition root.

## Problem / Motivation

The test pyramid here is missing its middle. At the top, `pytest` drives a *running*
Serial Studio over the TCP API: 23 integration files, 7 security files, a performance
tier. At the bottom, `tests/scripts/` runs JS parser logic under Node.js and
`test_cpp_regressions.py` greps C++ source text for known-fixed bugs. In between —
the pure C++ logic that decides whether a frame is found, whether its checksum passes,
whether a project round-trips, whether a SIMD kernel agrees with the scalar loop it
replaced — there is nothing. `tests/unit/` contains one Python file that validates a
JSON manifest.

The practical cost is feedback latency and coverage shape. A regression in delimiter
scanning or checksum packing is caught only after a full platform build, install, launch,
and API round-trip — on a workflow whose fastest build job is measured in tens of minutes
and whose pytest job runs *after* publishing and is `continue-on-error` today. Some
classes of bug are not reachable from that tier at all: the SIMD kernels in the DSP layer
carry a written bit-exactness contract against their scalar reference, and today nothing
anywhere checks it — the scalar path is a tail loop inside the same function and is never
compared against the vector block on the same input. The source-grep tests in
`tests/scripts/test_cpp_regressions.py` exist precisely because there was no better place
to pin C++ behavior; they assert on the *text* of a function body, which breaks on
refactors that preserve behavior and passes on refactors that break it.

## Goals

- A maintainer can build and run the full C++ unit tier locally in seconds, with one
  command, and see a pass/fail summary per suite.
- A logic regression in frame delimiting, checksums, project serialization, buffer
  semantics, or a DSP kernel turns CI red in the fast pre-packaging tier, before any
  platform build or packaging job has produced an artifact.
- The SIMD kernels' scalar-vs-vector bit-exactness contract is machine-checked on both
  an x86-64 and an aarch64 runner, not asserted in a doc comment.
- Configuring a sanitizer, analysis, or test build is a named one-command operation
  instead of a remembered pile of `-D` flags.
- Adding a new test is a one-file change plus one line of build registration.
- Nothing about the default build, the shipped binary, or the hotpath benchmark changes.

## Non-Goals

- **No test coverage of GUI, QML, or singleton-wired subsystems in v1.** Anything that
  needs the composition root is out of scope here; it is what the phase-2 in-app test
  mode exists for.
- No replacement of the `pytest` tiers. The live-API suites keep owning end-to-end
  behavior; this tier owns pure logic only.
- No performance assertions. Throughput remains the exclusive business of
  `--benchmark-hotpath`; a unit test that measures time would be flaky on shared runners
  and would duplicate an existing gate.
- No coverage-percentage target, no coverage instrumentation in v1.
- No new third-party test framework, no vendored test library, no `FetchContent` at
  configure time for test purposes.
- No mocking framework and no interface extraction to enable mocking. Only code that is
  already testable as-is gets tested; making more code testable is R4's job.
- Presets do not become the required way to build. The existing `cmake -B build -D...`
  invocations in CI and in the maintainer's habits keep working unchanged.

## Requirements

1. **R1 — Opt-in test target.** The C++ test tier compiles only when explicitly
   requested. A default configure, and every configure line currently used by CI or by
   the maintainer, produces exactly the artifacts it produces today, with no new
   dependency lookup and no new target.
2. **R2 — Runs under ctest.** With the tier enabled, `ctest` from the build directory
   discovers and runs every suite, reports per-suite pass/fail, and returns a non-zero
   exit code if any suite fails.
3. **R3 — Seconds, not minutes.** The whole tier — build of the test targets from a warm
   build directory plus the `ctest` run — completes fast enough to be part of an edit
   loop. No suite may depend on the application executable being built.
4. **R4 — Initial coverage.** The tier ships with suites covering: DSP kernel
   scalar-vs-SIMD equivalence; circular-buffer semantics including pattern scans that
   straddle the ring wrap; frame delimiter extraction edge cases (start-delimited,
   end-delimited, start+end, split across chunks, empty and back-to-back frames);
   every supported checksum algorithm against known-answer vectors including its output
   byte order; and project-entity serialization round-trips.
5. **R5 — Bit-exactness is actually checked.** For every DSP kernel, the test compares
   the shipping (vector) result against the scalar reference on the same input, bit for
   bit, on both supported SIMD architectures. A test that can only reach the scalar tail
   loop by choosing short inputs does not satisfy this requirement.
6. **R6 — CI gate in the fast tier.** The tier builds and runs automatically in CI in the
   fast, pre-packaging tier — no dependency on a build, upload, or packaging job, and
   failing it is visible before any artifact exists. CI is the only place tests compile
   and run automatically; no commit hook, sanitize step, or local script compiles C++.
7. **R7 — Named build configurations.** A `CMakePresets.json` provides at minimum `dev`
   (debug + tests), `asan`, `tsan`, and `analysis` configurations, each mapping onto the
   build options that already exist, plus matching build and test presets. Adding the
   file changes no default behavior; a developer who ignores it sees no difference.
8. **R8 — Style contract applies.** Test sources obey the repo's formatting contract and
   are covered by the sanitize pipeline. Where a linter rule is deliberately relaxed for
   test code, the relaxation is explicit and documented, not accidental.
9. **R9 — In-app test mode (phase 2).** A build-flag-gated, CLI-flag-run test mode
   executes inside the real composition root, following the precedent of the in-binary
   hotpath benchmark and the existing self-test flags. v1 of this mode is deliberately
   minimal: the mechanism plus one trivial suite, proving the seam exists for R4's
   session-context work to use later.

## Acceptance Criteria

- [x] **AC1** (R1) — A configure with no new flags produces a build tree containing no
      test target and no `Qt6::Test` lookup; the resulting binary is byte-comparable in
      behavior, and `--benchmark-hotpath` results are within run-to-run noise of the
      pre-change baseline.
- [x] **AC2** (R2, R3) — With the tier enabled, `ctest --output-on-failure` reports every
      suite green, and the maintainer confirms wall-clock time for the run is in seconds.
- [x] **AC3** (R4, R5) — Each shipped suite exists and passes; the DSP suite fails if the
      vector and scalar results of any kernel diverge by one bit, demonstrated by a
      maintainer-run scratch experiment that perturbs one kernel's vector block.
- [x] **AC4** (R5) — The DSP suite runs and passes on both an x86-64 and an aarch64 CI
      runner, and the run log shows which SIMD lane was exercised on each.
- [x] **AC5** (R6) — A seeded regression (one flipped byte comparison in the delimiter
      scan, on a scratch branch) turns the CI unit job red in under five minutes from
      workflow start, with no packaging job having produced an artifact.
- [x] **AC6** (R7) — `cmake --preset dev && cmake --build --preset dev && ctest --preset dev`
      works from a clean checkout; the `asan` and `tsan` presets each configure and build;
      the `analysis` preset produces a `compile_commands.json`.
- [x] **AC7** (R8) — `python scripts/code-verify.py --check` and
      `python scripts/sanitize-commit.py` run clean over the new sources, and the plan
      records exactly which rule set does and does not reach the test directory.
- [x] **AC8** (R9) — The in-app test flag runs inside the real application process,
      prints a pass/fail summary, exits with a meaningful status, and does not start the
      GUI. In a build without the flag's compile-time option, the flag is absent.

## Constraints & Invariants

- **No new dependency.** The test framework must already be present in the Qt
  installation CI and the maintainer already have. Nothing may be vendored into `lib/`,
  fetched at configure time, or added to `tests/requirements.txt` for this tier.
- **The app target and its PGO/LTO setup are not restructured.** Whatever mechanism
  shares production sources with the test binaries must leave the existing
  `qt_add_executable` call, the QML module registration, the two-stage PGO flow, and the
  hardening/signing hooks exactly as they are.
- **Nothing here may regress the `--benchmark-hotpath` gates.** Any change to a hotpath
  header — and one is required to make the DSP scalar reference callable — must be a
  compile-time no-op for the default build, proven by benchmark and by inspection of the
  default build's preprocessed output.
- **Test builds must succeed in the default GPL configuration.** No suite may require
  `BUILD_COMMERCIAL=ON` or any Pro-only header in v1.
- **CI must not grow a second slow job.** The unit job's cost is bounded by not building
  the application target and by disabling optional subsystems it does not exercise.
- The spec-0001 composition-root ordering proof governs anything the phase-2 in-app mode
  does inside the real process.
- Every new file carries the SPDX dual-license header like the rest of the tree.

## Open Questions

- **DSP scalar reference requires a source change.** `DSPSimd.h` derives its
  `SS_SIMD_X86` / `SS_SIMD_NEON` selection from the target architecture with no opt-out,
  and each kernel's scalar path is the tail loop of the same function — so today there is
  no way to call "the scalar version" of a kernel on a full-length buffer. Satisfying R5
  needs a small compile-time-only hook in that header (an opt-out macro plus a
  configurable namespace name so a second translation unit can hold the scalar build
  without an ODR collision). Does the maintainer accept a two-line, default-inert change
  to a hotpath header for this, or should v1 settle for oracle-and-consistency tests that
  do not satisfy R5?
- **FrameReader link footprint.** `FrameReader` itself constructs standalone, but its
  dropped-frame path reaches `NotificationCenter`, which transitively pulls a wide set of
  app headers. If the link set for a `FrameReader`-level suite turns out to be
  disproportionate, is it acceptable to cover delimiter logic at the circular-buffer
  level in v1 and defer end-to-end reader coverage to the phase-2 in-app mode?
- **One CI job or two?** The roadmap's acceptance names "the lint-tier job". The lint job
  today has no Qt, no compiler, and no CMake, and is ~1-3 minutes of pure Python. Should
  the unit tier be a sibling job in the same tier (recommended: independent, parallel, no
  `needs:`), or literally added as steps to the existing `lint` job?
- **Should the unit job be a required check** for merging, given that the existing pytest
  job is currently non-blocking (`REQUIRE_TESTS_TO_PUBLISH: false`)?
