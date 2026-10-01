---
spec: 0090-post-link-layout
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-01
---

# Plan 0090 — Post-Link Binary Layout Optimization

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A new opt-in build option, `ENABLE_POST_LINK_LAYOUT`, makes the three release pipelines
layout-aware without touching any default or local build. On Linux (both arches) it adds the
two flags BOLT needs (`-Wl,--emit-relocs` link-side; `-fno-reorder-blocks-and-partition` on
GCC) and defers the spec-0084 symbol-split/strip from a POST_BUILD command to an on-demand
target; ci.yml then drives BOLT exactly the way it already drives PGO — a new composite
action instruments the PGO-optimized binary, re-runs the two existing training workloads,
rewrites the binary with `llvm-bolt` (ext-tsp block reordering, cdsort function reordering,
function splitting, debug-info update), and only then invokes the deferred split/strip — so
every existing gate, package, and signature downstream consumes the rewritten binary. On
macOS the same option makes the AppleClang PGO-USE branch derive a hot-function order file
from the already-merged `merged.profdata` at configure time and pass it via
`-Wl,-order_file`. On Windows it adds `/print-symbol-order:` to the link so a new CI step can
assert lld-link's call-graph profile sort is actually active (enabling it explicitly, or
falling back to a generated `/call-graph-ordering-file`, only if the assertion shows it
is not). All sequencing lives in CI (the "CI-scripted" design), mirroring the PGO staging CI
already owns.

## Affected subsystems & files

| File | Change |
|------|--------|
| `cmake/Optimization.cmake` | New `ENABLE_POST_LINK_LAYOUT` option block: Linux-ELF BOLT-enabling flags (`--emit-relocs`, GCC `-fno-reorder-blocks-and-partition`), macOS order-file generation + `-Wl,-order_file` in the AppleClang PGO-USE path (FATAL_ERROR on empty file, R6), clang-cl `/print-symbol-order:` link flag. Flags apply in both PGO stages so the stage-identity rule stays literal. |
| `app/CMakeLists.txt` | Spec-0084 block (lines ~1266–1309): on Linux with `ENABLE_POST_LINK_LAYOUT`, attach the objcopy-sidecar + `strip --strip-unneeded` commands to a new custom target `ss_finalize_binary` instead of POST_BUILD; macOS/Windows branches unchanged. |
| `.github/actions/bolt-linux/action.yml` | **New** composite action (mirrors `pgo-train-linux`): install pinned `llvm-bolt`/`merge-fdata`, `llvm-bolt --instrument`, run the hotpath (`--min-fps 1`) and `big_db_test` training loads against the instrumented copy, `merge-fdata`, final `llvm-bolt` rewrite with `-dyno-stats` captured to `bolt-report.txt`, replace the binary, build `ss_finalize_binary`. Fail-hard at every step (R3). |
| `.github/workflows/ci.yml` | Linux x64 + arm64 jobs: pass `-DENABLE_POST_LINK_LAYOUT=ON` at both configures, insert the bolt-linux action between '🚧 Build application (PGO optimized)' and '🧪 QML instantiation self-test', add a bare `--selftest` step (R4), add `bolt-report.txt` to the benchmark artifact (R8). macOS arm64 job: `-DENABLE_POST_LINK_LAYOUT=ON` on both configures, upload the generated order file (R8). Windows job: `-DENABLE_POST_LINK_LAYOUT=ON`, new assertion step on the `/print-symbol-order` output (AC4), upload it (R8). The macOS x86_64 leg and all non-release jobs are untouched (option stays OFF). |
| `app/src/SelfTest/SelfTest.cpp` | New pre-root suite `script-unwind` in `kSuites`: creates a standalone LuaJIT state, raises a Lua error, verifies the error unwinds into the host `pcall` guard and the state survives — singleton-free per the pre-root contract (AC2's unwind canary). |
| `.claude/skills/cpp-compiler-flags/references/serial-studio-build-flags.md` | Document `ENABLE_POST_LINK_LAYOUT`, the BOLT CI flow, and the per-platform mechanisms (the skill names this file the authoritative flag map). |

## Architecture & data flow

No application architecture changes — this is a build/CI feature. The Linux release job's
step order becomes:

```
PGO-GENERATE configure/build → training runs → PGO-USE configure/build (unstripped,
--emit-relocs) → [NEW] BOLT instrument → re-train (same two loads) → merge-fdata →
llvm-bolt rewrite → ss_finalize_binary (sidecar split + strip, now from the rewritten
binary) → [NEW] --selftest → QML selftest → 256 kHz gate → big-project verify →
package (AppImage/deb/rpm) → GPG sign
```

Licensing: the PGO training already activates the seat and the job's `always()` step
deactivates it, so the BOLT training runs between them execute activated with no new
activation (the machine config persists across the rebuild).

macOS: order-file generation runs inside the PGO-USE configure, immediately after the
existing `llvm-profdata merge` — `llvm-profdata show --all-functions` output is reduced to
hot symbols, `_`-prefixed for Mach-O, written to `${CMAKE_BINARY_DIR}/hot-order.txt`, and
passed with `-Wl,-order_file,...`. Unmatched entries are ignored by ld64 by design; an empty
file is a configure-time FATAL_ERROR. arm64 slice only — the x86_64 leg builds without PGO
and never sets the option (per spec decision).

Windows: no pipeline reshape. `/print-symbol-order:${CMAKE_BINARY_DIR}/symbol-order.txt` on
the lld-link line; the CI step after the optimized build asserts the file exists and is
non-trivially sized (the file is only produced when call-graph profile sorting actually
runs, which makes it the direct evidence R5 wants).

## Hotpath & threading impact

- **Touches the hotpath?** No source file on the Driver → FrameReader → FrameBuilder →
  Dashboard path changes. The generated *code layout* of the whole binary changes, which is
  the point — and the existing 256 kHz gate re-runs against the rewritten binary on every
  push (AC1), so a layout-induced regression cannot land silently.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unaffected; no code change on any stamping path.
- The one C++ addition (`script-unwind` selftest suite) is pre-root, self-contained, and
  never touches an application singleton, per the pre-root registry contract in
  `SelfTest.cpp`.

## Data model & persistence

None. No project JSON, schema, or Sessions DB change.

## API / SDK surface

None.

## QML / UI

None (headless/CI only).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where BOLT sequencing lives | CI-scripted (ci.yml + action) / CMake custom-target chain / `PGO_STAGE=BOLT` third stage | **CI-scripted** — training needs license activation, Qt env and multi-process loads that don't belong in cmake; CI already owns the identical PGO staging. A third PGO_STAGE misrepresents a post-link tool as a compiler stage and invites stage-identity drift. |
| Spec-0084 strip vs BOLT ordering | Deferred strip via `ss_finalize_binary` / BOLT the stripped binary / duplicate objcopy+strip commands in CI | **Deferred strip target** — BOLT needs the symbol table, and the sidecar must be split from the *rewritten* binary or crash symbolication silently lies; a target keeps the logic in one place for all callers. |
| BOLT profiling mode | Instrumentation / perf sampling (LBR) | **Instrumentation** — hosted runners have no PMU/LBR; this is the spec's deciding constraint, not a preference. |
| Enabling-flag staging | Flags in USE stage only / both PGO stages | **Both stages** — keeps "identical configure values per stage" literal; `--emit-relocs` on the instrumented binary is harmless (file-size only), `-fno-reorder-blocks-and-partition` is inert without profile data. |
| Debug info under BOLT | `-update-debug-sections` / drop sidecar fidelity for speed | **Update** — spec 0084 exists precisely so shipped-build stacks symbolicate; a few extra seconds of BOLT time is cheap against that. |
| macOS order-file generation site | PGO-USE configure (cmake) / separate CI script step | **Configure-time** — the profdata merge already happens there, the FATAL_ERROR gives R6 its fail-hard for free, and flag logic stays in the flags module. |
| Windows R5 posture | Assert-only first / enable-and-assert blind | **Assert-only first** — the spec demands the empirical answer before sizing the work; if the assertion shows sorting inactive, the fallback (generated `/call-graph-ordering-file` from profdata) becomes a follow-up task, not a speculative one now. |

## Risks & mitigations

- **`--emit-relocs` together with `--gc-sections` on GNU ld** is the least-exercised flag
  pair here; if the Ubuntu 24.04 binutils mishandles it, the fallback is linking the USE
  stage with `-fuse-ld=` unchanged but dropping gc-sections *only* when
  `ENABLE_POST_LINK_LAYOUT` is ON (BOLT's own dead-code handling recovers most of it).
  Verified empirically in the first CI run; fail-hard means a breakage is loud.
- **Unwind correctness (the Lua invariant).** BOLT regenerates `.eh_frame` for moved code;
  the new `script-unwind` suite plus the existing big-project teardown run make a corrupted
  unwind path fail CI rather than ship (AC2). This is the diff's nearest-risk rule — see the
  counterfactual check at handoff.
- **AArch64 BOLT with BTI landing pads** (`-mbranch-protection=standard`) is BOLT's
  least-battle-tested path. Mitigation: identical gates run on the arm64 job; AC3's
  deliberate-break exercise is done on both arches before approval of the rollout.
- **CET (`-fcf-protection=full`) on x86_64**: BOLT understands `endbr64`; the selftest +
  benchmark + big-project trio exercises the rewritten binary enough to catch a landing-pad
  break. Hardening properties of the ELF (RELRO, GNU_PROPERTY notes) are checked once,
  manually, on the first rewritten artifact.
- **BOLT package name/availability** on apt.llvm.org for both amd64 and arm64 must be
  confirmed in the task phase (expected: the `bolt-<N>`/`llvm-bolt-<N>` package providing
  `llvm-bolt` + `merge-fdata`, pinned major). If the arm64 package is missing, the arm64 job
  ships the stage disabled and the spec's open-question staging answer flips to "x86_64
  first" — a named fallback, not a silent one.
- **`/print-symbol-order` producing nothing on Windows** is a *finding*, not a failure of
  this plan — it answers R5's empirical question and triggers the documented fallback.
- **CI wall-clock**: the stage re-runs both training loads once more plus two BOLT passes —
  estimated +4–6 min per Linux job, inside the spec's "repeat training once" budget.
- **Known CI gotchas that apply** (`common-mistakes.md` "CI & Platform"): none of the
  Windows console-subsystem or `-platform` traps are touched — the new Linux runs reuse the
  exact invocation patterns of the existing training steps; the Windows change adds no new
  app invocation at all.

## Test & verification plan

- **AC1 (gates pass on rewritten binary):** existing '🚦 Hotpath throughput gate' steps,
  unchanged, now downstream of the BOLT step on both Linux jobs. Pass/fail only — CI noise
  (±45–56%) means no delta claims from CI numbers.
- **AC2 (selftest incl. Lua unwind):** new bare `--selftest` CI step (runs `smoke` +
  `script-unwind`) plus the existing `--selftest-suite qml` step, both after the rewrite.
- **AC3 (fail-hard):** one-off exercise on a throwaway branch — delete the merged `.fdata`
  (Linux) / point the order file at an empty profile (macOS) and confirm the job fails;
  recorded in the PR, not checked in.
- **AC4 (Windows ordering proven):** new CI assertion step on `symbol-order.txt`; the file
  is also uploaded for audit (R8).
- **AC5 (macOS order file):** configure-time FATAL_ERROR covers the empty case; the
  existing macOS 256 kHz gate covers the rest; order file uploaded (R8).
- **AC6 (artifacts from optimized binaries):** step-order review of ci.yml in the PR; the
  packaging steps are untouched and sit strictly after the rewrite + finalize.
- **AC7 (report artifact):** `bolt-report.txt` (dyno-stats) / `symbol-order.txt` /
  `hot-order.txt` added to the existing per-job upload lists.
- **Static:** `python scripts/code-verify.py --check` on `SelfTest.cpp`; `qt-cpp-review` on
  the C++ before handoff; `python scripts/sanitize-commit.py` before commit.
- **Unit:** none applicable (no `tests/scripts/` surface); the selftest suite *is* the test.
- **Maintainer:** local before/after `--benchmark-hotpath` runs on one machine if a real
  delta number is wanted (optional; not a gate).
