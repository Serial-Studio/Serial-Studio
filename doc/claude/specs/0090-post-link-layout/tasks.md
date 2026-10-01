---
spec: 0090-post-link-layout
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-01
---

# Tasks 0090 — Post-Link Binary Layout Optimization

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — `ENABLE_POST_LINK_LAYOUT` option + Linux BOLT-enabling flags

- **Files:** `CMakeLists.txt`, `cmake/Optimization.cmake`
- **Does:** Declares the option (OFF default, beside `ENABLE_PGO`); when ON on Linux-ELF adds
  `-Wl,--emit-relocs` to the link and, on GCC, `-fno-reorder-blocks-and-partition` to the
  compile. Binding invariants, named: flags apply in BOTH PGO stages so the GENERATE/USE
  stage-identity rule stays literal; nothing here may touch the IEEE-math or unwind-table
  flag sets; the Flatpak/sandbox branch must never see these flags (option stays OFF there).
- **Verify:** Read-back against the per-toolchain branch map in the cpp-compiler-flags skill
  reference; confirm by grep that no other branch (APPLE, MinGW, clang-cl, MSVC) inherits
  the ELF flags.
- **Deps:** none
- [x] done

### T2 — macOS hot-function order file at PGO-USE configure

- **Files:** `cmake/Optimization.cmake`
- **Does:** Guarded by `ENABLE_POST_LINK_LAYOUT AND APPLE AND PGO_STAGE=USE`: after the
  existing `llvm-profdata merge`, derives hot function symbols from `merged.profdata`
  (`llvm-profdata show`, `_`-prefixed for Mach-O), writes `hot-order.txt` into the build
  dir, `FATAL_ERROR`s if the file is empty (spec R6 fail-hard), and adds
  `-Wl,-order_file,<path>`. Invariant, named: AppleClang branch only; the DISABLE_LTO
  forcing and the lua54/unwind flag handling nearby must be untouched.
- **Verify:** Read-back; confirm the guard excludes the x86_64 leg (which never sets the
  option) and the GENERATE stage; dry-parse the `llvm-profdata show` output format against
  the installed tool's docs.
- **Deps:** T1
- [x] done

### T3 — Windows `/print-symbol-order` evidence flag

- **Files:** `cmake/Optimization.cmake`
- **Does:** Guarded by `ENABLE_POST_LINK_LAYOUT` in the clang-cl branch: adds
  `/print-symbol-order:${CMAKE_BINARY_DIR}/symbol-order.txt` to the link options. Invariant,
  named: the clang-cl link step takes NATIVE lld-link flags only — no driver-style flag may
  be added here, and the flag goes in both PGO stages per the stage-identity rule.
- **Verify:** Read-back; confirm the flag is link-side native syntax and sits inside the
  clang-cl branch (which precedes cl.exe's).
- **Deps:** T1
- [x] done

### T4 — Defer the spec-0084 split/strip behind `ss_finalize_binary`

- **Files:** `app/CMakeLists.txt`
- **Does:** In the commercial-hardening block's Linux branch: when
  `ENABLE_POST_LINK_LAYOUT` is ON, register the objcopy sidecar-split +
  `strip --strip-unneeded` commands as a custom target `ss_finalize_binary` instead of
  POST_BUILD, so CI can run them *after* the BOLT rewrite; otherwise (and on macOS/Windows)
  the POST_BUILD behavior is byte-identical to today. Invariant, named: the sidecar must be
  produced from the final shipped binary or spec-0084 symbolication silently lies — that is
  the entire reason this task exists.
- **Verify:** Read-back; confirm the OFF path is unchanged (diff shows pure addition plus
  the conditional), and that the target also strips the leftover `--emit-relocs` sections
  (`objcopy --remove-section` or strip behavior confirmed) so the shipped binary does not
  carry them.
- **Deps:** T1
- [x] done

### T5 — `script-unwind` pre-root selftest suite

- **Files:** `app/src/SelfTest/SelfTest.cpp`
- **Does:** New entry in `kSuites`: creates a standalone LuaJIT state, raises a Lua error,
  verifies it unwinds into the host `pcall` guard and the state remains usable, closes it.
  Binding invariants, named: pre-root suites run before the composition root and must never
  touch an application singleton; `SS_ASSERT(cond, action)` with density ≥2; no in-body
  comments; `[[nodiscard]]` on non-void returns.
- **Verify:** `python scripts/code-verify.py --check app/src/SelfTest/SelfTest.cpp`;
  maintainer (or any existing build dir) runs `<app> --selftest` and sees the suite listed
  and passing.
- **Deps:** none
- [x] done

### T6 — `bolt-linux` composite action

- **Files:** `.github/actions/bolt-linux/action.yml` (new)
- **Does:** Mirrors `pgo-train-linux`'s shape: installs the pinned apt.llvm.org BOLT package
  (name confirmed for amd64 AND arm64 as part of this task — plan risk), runs
  `llvm-bolt --instrument` on the PGO-optimized binary, drives the hotpath (`--min-fps 1`)
  and `big_db_test` loads against the instrumented copy, `merge-fdata`s the profiles, runs
  the final `llvm-bolt` rewrite (`-reorder-blocks=ext-tsp -reorder-functions=cdsort
  -split-functions -split-all-cold -update-debug-sections -dyno-stats`) capturing
  `bolt-report.txt`, atomically replaces the binary, then builds `ss_finalize_binary`.
  Every step fail-hard (spec R3): no fallback to the un-rewritten binary.
- **Verify:** `yamllint`-level read-back; cross-check each invocation pattern against
  `pgo-train-linux/action.yml` (env, offscreen platform, LD_LIBRARY_PATH); confirm no
  activation step is added (seat already active from PGO training, released by the job's
  `always()` deactivate).
- **Deps:** T1, T4
- [x] done

### T7 — Wire the Linux x86_64 job

- **Files:** `.github/workflows/ci.yml`
- **Does:** In `build-linux`: adds `-DENABLE_POST_LINK_LAYOUT=ON` to both PGO configures,
  inserts the `bolt-linux` action between '🚧 Build application (PGO optimized)' and
  '🧪 QML instantiation self-test', adds a bare `--selftest` step (runs `smoke` +
  `script-unwind` against the rewritten binary), and adds `bolt-report.txt` to the
  benchmark artifact upload. Invariant, named: every gate, package and signing step must
  remain strictly downstream of the rewrite + finalize (spec R2/AC6).
- **Verify:** Read-back of the step order; grep both configure blocks for the new define.
- **Deps:** T5, T6
- [x] done

### T8 — Wire the Linux arm64 job

- **Files:** `.github/workflows/ci.yml`
- **Does:** Identical wiring in the aarch64 job (configure defines, action insert, bare
  `--selftest`, artifact). Invariant, named: same downstream-ordering rule; arm64 is BOLT's
  least-tested arch, so this job is the canary AC3 exercises first.
- **Verify:** Read-back; diff the two jobs' new steps against each other for symmetry.
- **Deps:** T7
- [x] done

### T9 — Wire the macOS arm64 job

- **Files:** `.github/workflows/ci.yml`
- **Does:** Adds `-DENABLE_POST_LINK_LAYOUT=ON` to both arm64 configures and uploads
  `build/hot-order.txt` with the benchmark artifact (spec R8). The x86_64 leg and merge job
  are untouched.
- **Verify:** Read-back; confirm the x86_64 leg's configure lines show no new define.
- **Deps:** T2
- [x] done

### T10 — Wire the Windows job + ordering assertion

- **Files:** `.github/workflows/ci.yml`
- **Does:** Adds `-DENABLE_POST_LINK_LAYOUT=ON` to both configures; after the optimized
  build, a new step asserts `build/symbol-order.txt` exists and is non-trivially sized
  (call-graph profile sort provably ran — spec R5/AC4) and uploads it (R8). If the first
  real run shows the file absent/empty, that is the R5 *finding*: file a follow-up task for
  the `/call-graph-ordering-file` fallback rather than weakening the assertion.
- **Verify:** Read-back; PowerShell step syntax checked against the job's existing
  Start-Process patterns (common-mistakes "CI & Platform": no new app invocations added).
- **Deps:** T3
- [x] done

### T11 — Document the flag surface

- **Files:** `.claude/skills/cpp-compiler-flags/references/serial-studio-build-flags.md`
- **Does:** Adds `ENABLE_POST_LINK_LAYOUT` to the option matrix and a short section on the
  per-platform mechanisms (BOLT CI flow, macOS order file, Windows evidence flag), including
  the deferred-strip interaction with spec 0084.
- **Verify:** `python scripts/claim-verify.py` (every named path/option must resolve);
  read-back against the shipped cmake diff.
- **Deps:** T1–T4
- [ ] done

### T12 — First-run validation + deliberate-break exercise (maintainer-gated)

- **Files:** none (CI runs + PR notes)
- **Does:** First full CI run with the stage live on all four pipelines; one-off audit of
  the rewritten Linux artifacts (RELRO/CET/BTI notes via `readelf`, debuglink matches the
  sidecar); AC3 exercise — scratch run with the merged `.fdata` removed (both Linux arches)
  and an empty macOS profile, confirming fail-hard; record outcomes in the PR/commit notes
  and check off the spec's acceptance criteria. If the arm64 BOLT package or rewrite proves
  unviable, invoke the plan's named fallback (arm64 ships with the option OFF; spec
  open-question answer flips to "x86_64 first").
- **Verify:** All seven acceptance criteria in `spec.md` checked off; benchmark gates green
  on all release jobs.
- **Deps:** T7, T8, T9, T10, T11
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff (`SelfTest.cpp`); findings addressed or noted.
- [ ] `--benchmark-hotpath` gate green on every release job against the rewritten binaries
  (no local hotpath code touched; the CI gate is the check).
- [ ] No `pytest` targets apply (CI-only feature); the selftest suite is the runtime check.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
