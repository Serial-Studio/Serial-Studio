---
spec: 0107-ship-artifact-proof
phase: tasks
status: approved     # approved 2026-10-30
updated: 2026-10-30
---

# Tasks 0107 — Ship-Artifact Proof

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
- The CI workflow is an isolated surface (`scripts/commit-shape.json`): T2 and T5 each land as
  one commit carrying only `ci.yml`, the shape test and `doc/`. The repo has no feature
  branches, so the probe (R9) is a master run.

## Tasks

### T1 — Land the approved plan and tasks

- **Files:** `doc/claude/specs/0107-ship-artifact-proof/{spec,plan,tasks}.md`,
  `doc/claude/specs/0104-consolidation-quarter/tasks.md`
- **Does:** One docs commit: `spec.md` to `in-progress`, this plan and task list approved, the
  umbrella's tracker row C1 pointing at 0107.
- **Verify:** `python scripts/commit-shape.py --range HEAD~1..HEAD` reports no findings.
- **Deps:** none
- [ ] done

### T2 — Probe landing: the flags and the four steps, non-blocking

- **Files:** `.github/workflows/ci.yml`
- **Does:** `-DSS_INAPP_TESTS=ON` on the shipping configures of `build-macos-arm64` (PGO
  stage 2), `build-macos-intel` and `build-windows` (PGO stage 2). Four proof steps, each with
  `continue-on-error: true` and `timeout-minutes`: both Linux jobs between the packaging action
  and the AppImage upload; `build-macos` between Staple and the DMG upload; `build-windows`
  between the MSI creation and the portable TGZ. Step bodies as in `plan.md` "Architecture".
- **Verify:** `python -c "import yaml; yaml.safe_load(open('.github/workflows/ci.yml'))"`;
  `python -m pytest scripts/tests/test_ci_workflow.py -q` (the existing gates still hold);
  `python scripts/commit-shape.py --range HEAD~1..HEAD` on the commit.
- **Deps:** T1
- [ ] done

### T3 — Read the probe run

- **Files:** `doc/claude/specs/0107-ship-artifact-proof/spec.md`
- **Does:** The master run of T2: for each of the four steps, conclusion and duration from the
  job page; the Rosetta log group shows `x86_64`; the Windows log shows the installed path and
  the "is gone" line. Record the run URL at R9, the per-leg seconds at AC5, and tick AC3 and
  AC4. A red step gets a `fix(ci)` commit citing the run (the commit-shape rule) and a re-read.
- **Verify:** every proof step green in one run; added seconds per leg within R8's five
  minutes.
- **Deps:** T2
- [ ] done

### T4 — Shape test

- **Files:** `scripts/tests/test_ci_workflow.py`
- **Does:** A `PROOF_LEGS` table (job, package suffix, the step name the proof follows, the
  upload artifact name it precedes) and four tests: each leg has exactly one step whose `run`
  carries `--selftest-suite qml` and the suffix, ordered after its package step and before its
  upload, with no `continue-on-error` and a `timeout-minutes`; the macOS step contains
  `arch -x86_64` and two suite runs; the Windows step contains `msiexec`, `/i`, `/x` and
  `Test-Path`; the three shipping configures contain `-DSS_INAPP_TESTS=ON`. Lands with T5.
- **Verify:** `python -m pytest scripts/tests/test_ci_workflow.py -q` passes on the hardened
  workflow and fails on each of the three mutations (a dropped step, a `continue-on-error`,
  a step moved after its upload), tried by hand before the commit.
- **Deps:** T3
- [ ] done

### T5 — Hardening commit

- **Files:** `.github/workflows/ci.yml`, `scripts/tests/test_ci_workflow.py`,
  `doc/claude/specs/0107-ship-artifact-proof/spec.md`
- **Does:** Drop `continue-on-error` from the four steps; land T4's test in the same commit;
  record the hardening run URL at AC6 once it is green.
- **Verify:** the next master run shows the four steps green and gating; AC1, AC2, AC6 ticked.
- **Deps:** T3, T4
- [ ] done

### T6 — Umbrella bookkeeping

- **Files:** `doc/claude/specs/0107-ship-artifact-proof/spec.md`,
  `doc/claude/specs/0104-consolidation-quarter/{spec,tasks}.md`
- **Does:** `ship` counted by hand (four legs, four steps) and AC7 ticked; 0104's AC6 ticked
  with `0107`, tracker row C1 `done` with the reading `4 → 0`, T2 and T4's C1 half ticked;
  this spec's status `done`.
- **Verify:** `python scripts/claim-verify.py` passes; the 0104 tracker reads 0107 as done.
- **Deps:** T5
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python -m pytest scripts/tests/test_ci_workflow.py -q` passes.
- [ ] Each workflow commit carried only `ci.yml`, the shape test and `doc/`
      (`python scripts/commit-shape.py --range` clean on each).
- [ ] No C++ or CMake file touched; `app/CMakeLists.txt` and `CLI.*` unchanged.
- [ ] `python scripts/sanitize-commit.py` run before each commit; working tree clean.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
