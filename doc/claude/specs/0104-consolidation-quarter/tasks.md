---
spec: 0104-consolidation-quarter
phase: tasks
status: approved     # approved 2026-10-30; tracker and boxes are the live record
updated: 2026-10-30
---

# Tasks 0104 — 4.3.0 Consolidation Quarter

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md) into units
> that are ordered and *individually verifiable*. This is an umbrella, so a unit is a child
> spec's whole lifecycle (born → approved → implemented → `done`) or a program checkpoint, and
> its **Verify** is the score dimension it moves or the gate it closes. The code-level
> tasks live in each child's own `tasks.md`. `/ss-implement` keeps the tracker and the boxes
> current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- A child is born with `/ss-spec` no earlier than one week before its month slot, so no
  child sits `approved` without commits (R9's 30-day rule binds the children too). Its
  `spec.md` frontmatter carries `umbrella: 0104`.
- Each child runs the full gate sequence on its own; "implemented" below means its
  `spec.md` reads `done`.
- A dimension reading comes from `python scripts/quality-score.py --check` once T3 lands;
  before that, from the census the plan names.
- CI premise runs land on master with `continue-on-error: true` on the probe step and are
  hardened in the next commit (no feature branches in this repo).
- **Deps** lists task IDs that must land first. Months are the plan's schedule.

## Tracker

| Id | Child | Spec | Month | Status | Moves | Reading (origin → now → target) |
|----|-------|------|-------|--------|-------|---------------------------------|
| C1 | Ship-artifact proof | — | Nov wk 1–2 | not born | `ship` | 4 → 4 → 0 |
| C0 | Quality score + five censuses | — | Nov wk 1 | not born | all | lenient 0.0 → 0.1 → 100 |
| C5 | AI-facing budget hard gate + overview | — | Nov wk 2 | not born | AC9 | one-hop 53,503 → 53,850 → below |
| C2 | Archive + issue triage | — | Nov wk 2 | not born | `specs` | 15 → 15 → 0 |
| C3-A | TU excess, the 9 cold files | — | Nov wk 3–4 | not born | `tu` | 2,641 → 2,627 → 1,694 |
| C4 | Suppressions reasoned or removed | — | Nov–Dec | not born | `supp`, `bare` | 171/152 → 171/152 → 86/0 |
| C6 | Dashboard widget dedup | — | Dec | not born | `dup` | 1,730 → 1,728 → ≈ 980 |
| C8 | Ledger + test-tier debt | — | Dec | not born | `ledger`, `skips` | 51/36 → 51/36 → 20/10 |
| C9 | Singleton static-cache conversion | — | Dec–Jan | not born | `singleton` | 803 → 803 → 600 |
| C7 | Second dedup family | — | Jan | not born | `dup` | ≈ 980 → — → 865 |
| C3-B | TU excess, `Dashboard.cpp` + `FrameBuilder.cpp` | — | Jan | not born | `tu` | 1,694 → — → 0 |

## Tasks

### T1 — Land the approved spec and plan

- **Files:** `doc/claude/specs/0104-consolidation-quarter/{spec,plan,tasks}.md`
- **Does:** One commit, `docs(spec): 0104 consolidation quarter (plan, tasks approved)`,
  carrying only this directory: the four amendments in `spec.md`, the approved `plan.md`,
  this file. Sets `spec.md` `status: in-progress`.
- **Verify:** `python scripts/commit-shape.py --range HEAD~1..HEAD` reports no findings.
- **Deps:** none
- [x] done (2026-10-30, this commit)

### T2 — C1: ship-artifact proof, born and probed

- **Files:** `doc/claude/specs/NNNN-ship-artifact-proof/`, `.github/workflows/ci.yml`
- **Does:** `/ss-spec` with the plan's leg table as the design; premise check is one master
  CI run with the four steps as `continue-on-error` probes: Rosetta launches the DMG's app on
  `macos-latest`, the MSI installs silently to a findable path, the `qml` suite passes from
  each package type (offscreen plugin present in all three). The `isolated surface` rule
  binds: the `ci.yml` commit carries only the workflow and its test.
- **Verify:** The probe run's four steps are green in the Actions log (URL recorded in the
  child's spec).
- **Deps:** T1
- [ ] done

### T3 — C0: quality score and the five censuses, implemented

- **Files:** `scripts/quality-score.py`, `scripts/quality-score.json`,
  `scripts/tests/test_quality_score.py`, `scripts/code-verify.py`,
  `scripts/code_verify_rules.py`, `scripts/ledger-verify.py`, `scripts/sanitize-commit.py`,
  `.claude/hooks/claim-check.py`, `.github/workflows/{ci,supply-chain}.yml`,
  `doc/claude/scripts.md`, `CLAUDE.md`
- **Does:** Born in parallel with T2 and implemented through its own tasks: the nine
  dimensions and the lenient/strict pair, the five new censuses under the plan's grammars,
  `--check`/`--accept` (upward-only), `--next`, `--share`, `--lines`, `--issues`,
  `--summary`; the `--accept` guard on every existing census and the ledger; the
  `suppression-bare` advisory rule; the lint-job step and the weekly `--issues` step; the
  `CLAUDE.md` sentence paid for by an equal trim (hard word gate at 5,128).
- **Verify:** `python scripts/quality-score.py --check` prints both readings and the gate
  checklist; `python scripts/quality-score.py --lines --at be6066d59` reads 446,632;
  `python -m pytest scripts/tests/test_quality_score.py -q` passes; the full score runs
  under 10 s.
- **Deps:** T1
- [ ] done

### T4 — C1: the four from-package steps hardened and pinned

- **Files:** `.github/workflows/ci.yml`, `scripts/tests/test_ci_workflow.py`
- **Does:** Drops `continue-on-error` from the four probe steps, adds
  `test_package_jobs_self_test_from_artifact` through `quality-score.ship_legs()`; C1's
  `spec.md` reads `done`.
- **Verify:** `python -m pytest scripts/tests/test_ci_workflow.py -q` passes; `ship` reads
  0; the next master run shows the four steps green and gating.
- **Deps:** T2, T3
- [ ] done

### T5 — AC7b: the published 4.2.1 Intel slice, proven once by hand

- **Files:** `doc/claude/specs/0104-consolidation-quarter/spec.md`
- **Does:** The maintainer downloads the released `Serial-Studio-Pro-4.2.1-macOS.dmg`, runs
  the app's binary under `arch -x86_64` with `--headless --selftest-suite qml`, and records
  date, host and result in `spec.md` under R3.
- **Verify:** The recorded line exists; AC7b's first clause is ticked.
- **Deps:** none (maintainer; Nov wk 1)
- [ ] done

### T6 — C5: AI-facing budget hard gate and the newcomer overview, implemented

- **Files:** `doc/claude/specs/NNNN-ai-facing-budget/`, `scripts/claim-verify.py`,
  `scripts/context-budget.json`, `scripts/documentation-verify.py`, `doc/ARCHITECTURE.md`,
  `CONTRIBUTING.md`, `README.md`
- **Does:** One-hop growth becomes a failure, `--accept` downward-only, the overview entry
  (≤ 3,000 words) joins the budget, claim and tone checks; the one-hop total is trimmed below
  53,503 without cutting a load-bearing rule.
- **Verify:** `python scripts/claim-verify.py --context-budget --check` passes with both
  totals below baseline and the overview under the cap; `python scripts/claim-verify.py`
  stays at 0 errors.
- **Deps:** T1
- [ ] done

### T7 — C2: spec archive and issue triage, implemented

- **Files:** `doc/claude/specs/NNNN-archive-and-issue-triage/`, the 15 non-done
  `doc/claude/specs/*/spec.md`, new `0021-simd-kernels/spec.md` and
  `0056-connect-path-crashes/spec.md`, GitHub issues (21 open)
- **Does:** Each non-done spec ends `done` or `shelved` with a dated note (one commit per
  spec, referencing it); the two bare directories get a one-paragraph `spec.md`; every open
  issue is fixed, declined with a reply, or scheduled in a numbered spec and labelled.
- **Verify:** `specs` reads 0 with 0 stale; `python scripts/quality-score.py --issues`
  reads 0.
- **Deps:** T3 (for the readings; the work may start before)
- [ ] done

### T8 — C3-A: the nine cold TU splits, implemented

- **Files:** `doc/claude/specs/NNNN-tu-excess-to-zero/`, `WorkspacesHandler.cpp`,
  `ExtensionManager.cpp`, `Modbus.cpp`, `Audio.cpp`, `MQTT.cpp`, `Publisher.cpp`,
  `Player.cpp` (CSV), `OpcUa.cpp`, `Terminal.cpp` and their `CMakeLists.txt`
- **Does:** One `tu-cutter.py` manifest and one commit per file, facade-plus-members shape;
  every new TU listed in its layer's target (`layer-verify` `cmake-missing`/`core-unowned`
  catch an omission). Phase B of the same child spec is T15.
- **Verify:** `python scripts/code-verify.py --tu-census --check` reads excess 1,694 over 2
  files; `python scripts/syntax-check.py` clean on every emitted TU; `ctest` green
  (maintainer builds).
- **Deps:** T1
- [ ] done

### T9 — C4: suppressions reasoned or removed

- **Files:** `doc/claude/specs/NNNN-suppressions/`, the ~93 files holding the 171 sites
- **Does:** Premise first (20 sampled bare sites, at least half removable by a root-cause
  fix, else the 86 target is amended). Then batches by subsystem, each under 60 paths: a
  site is removed by fixing the cause or given a one-clause reason; fences in hotpath files
  default to reasoned-and-kept.
- **Verify:** `bare` reads 0 and `supp` ≤ 86; `python scripts/code-verify.py --check` shows
  no new `suppression-bare` advisory.
- **Deps:** T3
- [ ] done

### T10 — November share check

- **Files:** none
- **Does:** On 2026-12-01 run `python scripts/quality-score.py --share 2026-11`; a miss is
  recorded in this file for the January review.
- **Verify:** The printed line reads ≥ 2/3.
- **Deps:** T3
- [ ] done

### T11 — C6: dashboard widget dedup, implemented

- **Files:** `doc/claude/specs/NNNN-dashboard-widget-dedup/`,
  `app/qml/Widgets/Dashboard/{Gauge,Meter,Bar,Clock,Compass,Plot,MultiPlot,FFTPlot}.qml`
  and the new base components beside them
- **Does:** Radial base, plot base, shared alarm-band painter; every widget keeps its file,
  `objectName`s and `Cpp_*` reads. Binding invariants named in the child's tasks: no new
  animation (motion contract), no per-tick `grab()`, the band *predicate* stays in C++
  (`datasetRendersAlarmBands` has one home), a stutter is sampled, never reasoned.
- **Verify:** `--dup-census --check` reads ≈ 980; `--selftest-suite qml` green; one
  maintainer screenshot per widget before and after, identical.
- **Deps:** T1
- [ ] done

### T12 — C8: ledger and test-tier debt, implemented

- **Files:** `doc/claude/specs/NNNN-ledger-and-test-debt/`, `doc/claude/common-mistakes.md`,
  `scripts/code_verify_rules.py`, `scripts/tests/test_code_verify.py`, `app/tests/*.cpp`,
  `tests/**`, `scripts/ledger-baseline.json`
- **Does:** 31 rows codified (a lint rule with its fixture, an anchor, or a ctest) or deleted
  with the why in the commit message; the static skip markers brought to 10 (the spec 0096
  T30 fixtures land, the environmental ones keep their reason). Baselines re-seeded downward
  only.
- **Verify:** `python scripts/ledger-verify.py --check` ≤ 20; `skips` ≤ 10;
  `python -m pytest scripts/tests/ -q` passes (the rule-fixture ratchet included).
- **Deps:** T3
- [ ] done

### T13 — C9: singleton static-cache conversion, implemented

- **Files:** `doc/claude/specs/NNNN-singleton-static-cache/`, the `static-cache` sites
  (422 across the three root-bound module sets' consumers)
- **Does:** Premise first (the top reached class: what share of its sites sits in classes
  that already hold an injected context). Then 203 sites take the dependency from the
  injected context, through `Core::services()`, `DataModel::pipelineModules()` or
  `API::handlerContext()`, never a new set and never a new `instance()`. Binding invariant:
  no function-static reach inside a module ctor closure, and no cross-library reach.
- **Verify:** `python scripts/code-verify.py --singleton-census --check` ≤ 600 with
  cross-library 0; `ctest` green.
- **Deps:** T1
- [ ] done

### T14 — December share check

- **Files:** none
- **Does:** On 2027-01-01 run `--share 2026-12`; a miss is recorded here.
- **Verify:** The printed line reads ≥ 1/2.
- **Deps:** T3
- [ ] done

### T15 — C3-B: `Dashboard.cpp` and `FrameBuilder.cpp`, cut at a clean benchmark point

- **Files:** `core/Ui/UI/Dashboard.cpp`, `core/Pipeline/DataModel/FrameBuilder.cpp`, their
  sibling dirs and `CMakeLists.txt`
- **Does:** `ss-hotpath` invoked and the four hotpath files read in full before the first
  manifest. Verbatim block moves only: no signal, connection type, pool, cached flag or
  timestamp site changes by construction (`tu-cutter` refuses a cut that does not
  reconstruct the original). The maintainer runs `--benchmark-hotpath` before the first cut
  and after each file; a tier loss is a revert, not a tune.
- **Verify:** `--tu-census --check` reads excess 0 over 0 files; nine benchmark tiers hold.
- **Deps:** T8, T11 (a quiet benchmark window after the QML work)
- [ ] done

### T16 — C7: second dedup family, implemented

- **Files:** `doc/claude/specs/NNNN-dedup-family-two/`,
  `app/qml/ProjectEditor/Views/{Group,Table,Workspace}FolderView.qml`,
  `{AlarmBands,FrequencyMarkers}Editor.qml`, `{EipTags,S7Variables}Dialog.qml`,
  `core/Api/API/{CommandCapabilities,SealedPolicy}.cpp`
- **Does:** Shared folder-view and editor bases; one table walker for the two command
  registries (a protected classification surface: the change is named in chat with its
  failure modes first, and `tst_command_capability_registry` / `tst_sealed_policy_registry`
  stay green).
- **Verify:** `--dup-census --check` ≤ 865.
- **Deps:** T11
- [ ] done

### T17 — R13: the delete budget balanced

- **Files:** whatever the `cxx-trivial-function` advisory (1 today) and the feature months'
  growth name
- **Does:** Resolve the remaining dead-code candidate; if `--lines` is above 468,963 after
  the feature share's growth, delete (never split) until it is not.
- **Verify:** `python scripts/quality-score.py --lines` ≤ 468,963.
- **Deps:** T3
- [ ] done

### T18 — The January review

- **Files:** `doc/claude/specs/0104-consolidation-quarter/{spec,tasks}.md`
- **Does:** End of January 2027: `--share 2027-01`; every AC checked against its command in
  the plan's table; the maintainer runs the pytest tiers, `ctest` and `--benchmark-hotpath`
  on the review's master (AC14); any month that missed the share is named here; the review
  verdict is recorded in `spec.md`.
- **Verify:** `python scripts/quality-score.py --check` reads lenient 100.0 and strict
  ≥ 82.5; `git log -p -- scripts/*.json` over the quarter shows no upward re-seed; AC1–AC14
  ticked in `spec.md`.
- **Deps:** T4–T17
- [ ] done

### T19 — 4.3.0, the exit release

- **Files:** version bumps as the release process names them; `spec.md`
- **Does:** Tag 4.3.0 only when T18 holds; its artifacts pass the four from-package steps;
  `spec.md` `status: done`.
- **Verify:** The 4.3.0 run shows the four steps green; every tag between 4.2.0 and 4.3.0 is
  a 4.2.x; AC15 ticked.
- **Deps:** T18
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC7a already).
- [ ] Every child spec in the tracker reads `done` or `shelved` with a dated note.
- [ ] `python scripts/quality-score.py --check` reads lenient 100.0; strict and the gap
      are recorded in `spec.md`.
- [ ] `git log -p -- scripts/*.json` over the quarter shows no upward re-seed.
- [ ] `--benchmark-hotpath` nine tiers hold on the review's master; pytest tiers and `ctest`
      green (maintainer).
- [ ] `python scripts/sanitize-commit.py` run before every commit; working tree clean.
- [ ] Each child's diff was *what was asked, and only that*; no foreign files touched.
- [ ] `spec.md` status set to `done`; 4.3.0 tagged.
