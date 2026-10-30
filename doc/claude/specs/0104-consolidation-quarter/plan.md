---
spec: 0104-consolidation-quarter
phase: plan
status: approved     # approved 2026-10-30 with the four spec amendments folded in
updated: 2026-10-30
---

# Plan 0104 — 4.3.0 Consolidation Quarter

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Grounded in the gate inventory, the CI package legs and the debt
> lists as measured on 2026-10-30 at master `aa02aa313`. Gate: do not start `/ss-tasks` until
> a human marks this `approved`.

> **Program plan.** The umbrella approves direction; this plan fixes three things the child
> specs then inherit: the **measurement instrument** (one quality score over every target),
> the **ship-artifact proof** (the CI design for R2), and the **child-spec roster** with its
> order, month and premise check. Each child still goes through `/ss-spec` → `/ss-plan` →
> `/ss-tasks` on its own; this plan is what makes those short.

## Approach (one paragraph)

Build the instrument first: `scripts/quality-score.py` reads every target the spec names
from the gate that already measures it (four existing censuses) plus five new censuses it
measures itself (suppressions, bare suppressions, non-done specs, static skip markers,
package legs without a from-artifact self-test), and folds them into one 0–100 number with
two readings, adapted from desloppify's lenient/strict pair: **lenient** reaches 100 exactly
when AC1–AC10 hold (the number 4.3.0 ships at), **strict** reads every dimension against
zero (the number that says how much debt the quarter chose to keep). The score is a ratchet
like every other gate: the lint job fails when it drops, `--accept` only moves it up, and it
is computed live so a stale baseline cannot inflate it. Then land the ship-artifact proof as
one step per package job that launches the packaged file and runs the `qml` self-test suite
from it, before the upload step, pinned by the workflow test. The remaining tracks run as
nine child specs in a fixed order (cheap, visible work in November to bank the two-thirds
share; the hotpath TU splits and the singleton conversion in December and January when the
benchmark has a quiet window), each with its own premise check and each moving one or more
score dimensions.

## Affected subsystems & files

Tooling and CI are this plan's lane. Code surfaces are listed per child spec in the roster
below and become that child's lane when it is planned.

| File | Change |
|------|--------|
| `scripts/quality-score.py` | **New.** The score: nine dimensions, lenient/strict, `--check`/`--accept`/`--json`/`--next`/`--share`/`--lines`/`--issues`/`--summary`. Imports the census collectors from `code-verify.py` and `ledger-verify.py` through `importlib` (hyphenated names), never re-implements them. |
| `scripts/quality-score.json` | **New.** The origin values, targets, weights, the last-accepted `now` per dimension and the two accepted scores. The one baseline for the five new measures. |
| `scripts/tests/test_quality_score.py` | **New.** Fixture tree per dimension, the formula at origin (0.0) and at target (100.0), the upward-only accept, the per-dimension ratchet, `--share` on a canned log, `ship_legs()` on a canned workflow. |
| `scripts/code-verify.py` | `--accept` on `--tu-census`, `--dup-census`, `--singleton-census`, `--clock-census` refuses a baseline higher than the checked-in one unless `--force` (spec constraint "never re-seeded upward", made mechanical). New advisory rule `suppression-bare` in `code_verify_rules.py` (grammar below). |
| `scripts/code_verify_rules.py` | The `suppression-bare` rule: `// code-verify off` with no reason clause on the line. |
| `scripts/ledger-verify.py` | `--accept` refuses an upward re-seed unless `--force`. |
| `scripts/claim-verify.py` | `--context-budget --check` fails on one-hop growth (today advisory); `--accept` downward-only; new `overview` entry: `doc/ARCHITECTURE.md` exists and is at most `overview_words_max` words. `doc/ARCHITECTURE.md` joins the claim-checked set. |
| `scripts/context-budget.json` | Adds `overview_words_max: 3000`, `overview_words`. |
| `scripts/documentation-verify.py` | `doc/ARCHITECTURE.md` joins the tone-checked set. |
| `scripts/sanitize-commit.py` | Runs `quality-score.py --check` after the ledger gate (blocking). |
| `.claude/hooks/claim-check.py` | `quality-score.py` joins the credited verifier list. |
| `.github/workflows/ci.yml` | Lint job: `🔒 Ratchet the quality score` (`--check --summary "$GITHUB_STEP_SUMMARY"`). Package jobs: one `🚀 Self-test from the package` step each (design below). |
| `.github/workflows/supply-chain.yml` | Weekly `quality-score.py --issues` (advisory; needs the network and `gh`). |
| `scripts/tests/test_ci_workflow.py` | Pins the four from-package steps through `quality-score.ship_legs()`: present, after the package step, before that package's upload, no `continue-on-error`, macOS runs both slices. |
| `doc/claude/scripts.md` | Row for `quality-score.py`; the ratchet paragraph names the score and the `--force` rule. |
| `CLAUDE.md` | The "Four gates ratchet growth" sentence names the score. CLAUDE.md is under a hard word gate (5,128): the addition is paid for by an equal trim in the same commit. |
| `doc/ARCHITECTURE.md` | **New.** The newcomer overview R10 asks for, ≤ 3,000 words, outside the AI-facing tree, linked from `CONTRIBUTING.md` and `README.md`. |
| `doc/claude/specs/0104-consolidation-quarter/spec.md` | The amendments listed at the end of this plan, once the maintainer confirms them. |

## Architecture & data flow

### The quality score

Every dimension is a count the tree already yields; the score only normalizes and sums.

| id | Measure | Source | Origin (2026-10-27) | Lenient target | Weight |
|----|---------|--------|--------------------:|---------------:|-------:|
| `tu` | TU excess lines over 1,500 | `code-verify --tu-census` | 2,641 | 0 | 2 |
| `dup` | shared 10-line windows over pairs ≥ 40 | `code-verify --dup-census` | 1,730 | 865 | 2 |
| `ship` | package legs with no from-artifact self-test step | `ship_legs(ci.yml)` | 4 | 0 | 2 |
| `supp` | suppression sites in the first-party tree | new census | 171 | 86 | 1 |
| `bare` | suppression sites with no reason clause | new census | 152 † | 0 | 1 |
| `singleton` | singleton reach sites | `code-verify --singleton-census` | 803 ‡ | 600 | 1 |
| `ledger` | open mistakes-ledger rows | `ledger-verify` | 51 | 20 | 1 |
| `specs` | live specs not `done`/`shelved` | new census | 15 † | 0 | 1 |
| `skips` | static skip/xfail markers | new census | 36 † | 10 | 1 |

† Re-measured by the new census on the commit that creates it, under the grammar pinned
below; the spec's number was the audit's own count. ‡ The census reads 803; the spec's 802 is
an off-by-one from the audit, the census is authoritative.

Formula, per dimension, with `now` the live count:

```
lenient_d = clamp(100 × (origin_d − now_d) / (origin_d − target_d), 0, 100)
strict_d  = clamp(100 × (origin_d − now_d) / origin_d,              0, 100)
score     = Σ weight_d × score_d / Σ weight_d
```

- Both readings share the one anchor (`origin`, never changed by this cycle); lenient reads
  against the spec's target, strict against zero. The next consolidation cycle changes
  targets only, so strict stays comparable across cycles.
- Overshoot holds at 100; a regression past the target shows as a drop below 100 at once
  (the formula is linear, only clamped at the ends).
- On the 2026-10-27 tree both readings are 0.0. At the spec's targets lenient is **100.0**
  and strict is **82.5**: the 17.5-point gap is the debt the spec accepted (half the dup
  windows, 86 reasoned suppressions, 600 reaches, 20 rows, 10 reasoned skips), named as one
  number. Today (2026-10-30) lenient reads 0.1.
- Weights 2/2/2/1…: the spec's motivation names unread packages and unexercised surfaces, so
  the TU, duplication and ship dimensions carry double weight. `--flat` prints the unweighted
  pair for comparison.

Hold-type criteria are **gates, not dimensions**, printed as a checklist under the score:
AC9 (both word totals below baseline, overview ≤ 3,000 words), AC12 (`--lines` ≤ 468,963),
AC13 (`--share` per month), AC14 (maintainer-run suites), AC15 (tags). A gate is pass/fail
and never moves the number; a dimension is a burn-down and never blocks on its own.

**Anti-gaming, adapted from desloppify.** (1) Suppressing a finding to move another
dimension raises `supp` and `bare` at equal weight, so it nets zero or worse. (2) The score
is computed live on every run; baselines record, they never feed the number. (3)
`quality-score.json` ratchets upward only: `--check` fails when either reading drops below
the accepted one or any of the five new measures grows past its accepted `now`; `--accept`
refuses to write a lower score or a higher measure (`--force` exists for a deliberate
decision and is a review trigger). (4) The four pre-existing censuses keep their own
baselines and gates; the score's file is not a second copy of them, only of the five new
measures and the two readings. (5) What the score cannot see: a deleted test or a spec marked
`shelved` without a dated note both lower a dimension honestly by the count and dishonestly
by intent; the monthly share review and the commit-shape gate are the defence, named here so
nobody mistakes the number for the whole truth.

**`--next`, desloppify's queue without a state file.** Prints the single highest-leverage
item per dimension from the per-item maps the censuses already return: the largest TU, the
heaviest pair, the file with the most bare suppressions, the most-reached singleton class,
the oldest open ledger row, the oldest non-done spec, the first static skip, the first
package leg without a step. Nine lines; the dispositions live in the tree (a reason clause, a
`Codified:` cell, a `reason=`, a `shelved` note), never in a dotfile.

**`--share [YYYY-MM]`** counts the month's non-merge commits and the ones that are
consolidation work: the message references `0104` or a spec whose `spec.md` frontmatter
carries `umbrella: 0104`, or the conventional type is `refactor`, `chore`, `ci` or `test`,
or the scope is `claude`, `spec` or `ledger`. Prints `share 2026-11: 41/58 (71%), rule 2/3:
pass`. Report-only; AC13 is judged at the review.

**`--lines`** reproduces the audit's method (newlines of every `.cpp`/`.h`/`.mm` blob under
`core/` and `app/src`, every `.qml` under `app/qml`, `ThirdParty` excluded, read through
`git ls-tree -r -l` + `git cat-file --batch` so no checkout is needed) and checks the
468,963 ceiling. Its premise check is that it reads 446,632 at `be6066d59`.

**`--issues`** asks `gh` for open issues with no maintainer comment in 90 days (AC11).
Network, so never in the lint job; the weekly supply-chain workflow runs it as advisory.

**New census grammars**, pinned so the numbers are reproducible:

- *Suppression site*: a line matching `// code-verify off`, `<!-- doc-verify off -->` or
  `<!-- claim-verify off -->` under `core/`, `app/`, `doc/`, `README.md`, excluding
  `lib/`, `ThirdParty/`, `scripts/` and `.claude/`. *Reasoned*: the same line carries, after
  `off`, a separator (`:`, `--`, `—`, `(`) and at least three words. The two forms already in
  the tree (`(cold argv setup …)` and `-- canonical lua_next walk …`) pass.
- *Non-done spec*: a directory under `doc/claude/specs/` matching `NNNN-*` whose `spec.md`
  `status:` is not `done` or `shelved`, or which has no `spec.md` (two today: `0021`, `0056`;
  each gets a one-paragraph `spec.md` with a terminal status). `templates/` is skipped.
  *Stale*: non-done, directory last touched over 30 days ago, and no commit in the last 30
  days mentions its number; printed separately, counted by `--check`.
- *Static skip marker*: `@pytest.mark.skip(`, `@pytest.mark.xfail(`, `pytest.xfail(` under
  `tests/` and `scripts/tests/`; `QSKIP(`, `QEXPECT_FAIL(` under `app/tests/`. `skipif` and
  runtime `pytest.skip(` env probes are the policy's sanctioned form and are not counted.
- *Package leg without a step*: for each of `build-linux`, `build-linux-arm64`,
  `build-macos`, `build-windows`, no step whose `run` contains `--selftest-suite qml` and the
  package file suffix (`.AppImage`, `.dmg`, `.msi`), ordered after the package step and
  before that package's `upload-artifact`, without `continue-on-error`; `build-macos` also
  needs `arch -x86_64` in the same step.

### The ship-artifact proof (R2)

One step per package job, after the package exists and before its upload, no
`continue-on-error`. A failure fails the build job, and the `upload` job needs every build
job, so nothing reaches the release. The downstream `test-suite.yml` callers stay as they
are (non-gating by design, and the Windows caller runs the portable ZIP, not the MSI).

| Leg | Launch | Notes |
|-----|--------|-------|
| `build-linux`, `build-linux-arm64` | `APPIMAGE_EXTRACT_AND_RUN=1 QT_QPA_PLATFORM=offscreen ./<pkg>.AppImage --headless --selftest-suite qml` | The file itself, not `squashfs-root/AppRun`. The composite action's glibc-2.28 smoke (`AppRun --selftest`, pre-root `smoke` suite) stays; this step is the pinned one and runs the post-root suite. |
| `build-macos` | `hdiutil attach -nobrowse -readonly -mountpoint "$RUNNER_TEMP/dmg" <pkg>.dmg`; run `<App>.app/Contents/MacOS/<exe> --headless --selftest-suite qml` natively, then under `arch -x86_64`; `hdiutil detach` | After `staple`, before upload. `arch -x86_64 /usr/bin/true` is the Rosetta probe; the step installs Rosetta (`softwareupdate --install-rosetta --agree-to-license`) when the probe fails. Does not disturb the pinned lipo → verify → codesign order. |
| `build-windows` | `msiexec /i <pkg>.msi /qn /norestart /l*v msi.log`; run the installed exe with `QT_QPA_PLATFORM=offscreen --headless --selftest-suite qml`; `msiexec /x <pkg>.msi /qn`; assert the install dir is gone | Proves install and uninstall. The installed path comes from the MSI's property (`INSTALL_ROOT` under CPack WiX); the child's premise check confirms it. |

The `qml` suite instantiates every compiled QML file against the real `Cpp_*` globals under
`--headless` + offscreen, so it needs the offscreen platform plugin inside each package; the
macOS rpath is already pinned, Linux and Windows are checked at the child's premise.

### The child-spec roster

Numbers are assigned by `/ss-spec` at birth; C-ids are this plan's handles. Each child's
`spec.md` carries `umbrella: 0104` so `--share` can count it. "Moves" names the dimensions
it moves.

| Id | Child | Month | Moves | Premise check (the number that kills it) | Mechanical? |
|----|-------|-------|-------|------------------------------------------|-------------|
| C0 | Quality score and the five censuses | Nov wk 1 | all | `--lines` reads 446,632 at `be6066d59` and the full score runs under 10 s (censuses measured 0.6/2.1/2.4/0.1 s). | no |
| C1 | Ship-artifact proof | Nov wk 1–2 | `ship` | One CI run: `arch -x86_64` launches the DMG's app on `macos-latest`, the MSI installs silently to a findable path, the `qml` suite passes from all three package types. | no (workflow) |
| C2 | Archive and issue triage (R9, R12) | Nov wk 2 | `specs` | None needed: 15 specs and 21 issues, each a reply or a status line. | no |
| C3 | TU excess to zero (R4): phase A the 9 cold files, phase B `Dashboard.cpp` and `FrameBuilder.cpp` | A: Nov wk 3–4, B: Jan | `tu` | B: `--benchmark-hotpath` before and after the first hot cut, nine tiers hold (an LTO build should see no loss; the gate decides, a loss is a revert). | yes: `tu-cutter.py` manifests, exact reconstruction |
| C4 | Suppressions reasoned or removed (R6) | Nov–Dec | `supp`, `bare` | Sample 20 bare sites: at least half removable by a root-cause fix, else the 86 target is amended. | no (judgment per site) |
| C5 | AI-facing budget hard gate and the newcomer overview (R10) | Nov wk 2 | gate AC9 | One-hop is already +347 over baseline; the trim that brings it under must not cut a load-bearing rule (claim-verify stays green). | no |
| C6 | Dashboard widget dedup (R5, family 1) | Dec | `dup` | The six pairs plus Bar\|Gauge, FFTPlot\|Plot, Compass\|Meter sum to 939 windows; the family alone cannot reach 865 (≈ 980 left), so C7 is scheduled, not optional. | no |
| C7 | Second dedup family (Project Editor folder views, `CommandCapabilities`\|`SealedPolicy`) | Jan | `dup` | 310 + 94 windows; needed to cross 865. | no |
| C8 | Ledger and test-tier debt (R8, R11) | Dec | `ledger`, `skips` | 31 rows to codify or delete; each codified row needs a lint fixture (`test_code_verify.py` ratchets on a rule without one). | partly |
| C9 | Singleton static-cache conversion (R7) | Dec–Jan | `singleton` | 203 sites from the `static-cache` bucket (422); pick the top reached class: the share of its sites inside classes that already hold an injected context decides whether this is a sweep or a redesign. | partly |

Share math: at October's volume (64 non-merge commits), November's two-thirds is about 43
consolidation commits. C0 (~8), C1 (~4), C2 (~20 one-per-spec/issue), C3-A (9), C4's first
batches (~8) and C5 (~3) bank it by week three.

## Hotpath & threading impact

- **Touches the hotpath?** The tooling and CI work: no. C3 phase B cuts `FrameBuilder.cpp`
  (2,911 lines, the span fast lane and the block stager live there) and `Dashboard.cpp`
  (1,783, `onDisplayTick` drains the SPSC ring); C6 edits `Widgets/Dashboard/` delegates on
  the draw path. For C3-B the cut is a verbatim block move (`tu-cutter.py` refuses a cut that
  does not reconstruct the original), so SPSC, `Qt::DirectConnection`, the no-alloc publish
  path, the slot pool and the cached flags are untouched by construction; what can change is
  cross-TU inlining, which the LTO release build mostly recovers and `--benchmark-hotpath`
  measures. That child's plan invokes `ss-hotpath` and reads the four files in full. For C6
  the rule is behavior-identical rendering: no new animation (motion contract), no per-tick
  `grab()`, the alarm-band predicate stays in `SerialStudio::datasetRendersAlarmBands` (only
  drawing is shared), and a stutter is sampled, never reasoned.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; `--clock-census` stays gated.

## Data model & persistence

`scripts/quality-score.json`:

```json
{
  "spec": "0104-consolidation-quarter",
  "regenerate": "python scripts/quality-score.py --accept",
  "origin": {"commit": "be6066d59", "date": "2026-10-27"},
  "dimensions": {
    "tu": {"origin": 2641, "target": 0, "weight": 2, "now": 2627, "gate": "tu-census"},
    "supp": {"origin": 171, "target": 86, "weight": 1, "now": 171, "gate": null}
  },
  "accepted": {"lenient": 0.1, "strict": 0.1, "commit": "…", "date": "2026-11-02"}
}
```

`gate` names the pre-existing baseline that ratchets the measure; `null` means this file is
its baseline. No project-file, session, or on-disk user format changes anywhere in the
quarter: R14 forbids it.

## API / SDK surface

None. C9 removes reaches; it adds no handler, slug or generated surface.

## QML / UI

C6 extracts the shared scaffolding of the seven dashboard widgets (6,650 lines) into base
components under `app/qml/Widgets/Dashboard/`: one radial base (Gauge, Meter, Compass,
Clock: the value overlay, min/max/unit labels, theme bindings, the `Cpp_UI_Dashboard` model
wiring), one plot base (Plot, MultiPlot, FFTPlot), and one shared alarm-band painter (Bar,
Gauge, Meter). Each widget keeps its file, its `objectName`s (spec 0101: addressed by name
and nothing else) and its `Cpp_*` reads; the base owns the chrome. Verification is the `qml`
self-test suite plus one maintainer screenshot per widget before and after, as the spec
requires. The overview `doc/ARCHITECTURE.md` is prose, no UI.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Score shape | (a) desloppify's absolute `(potential − failures)/potential`; (b) burn-down from origin to target; (c) no composite, just the gates | **(b)**. (a) reads 98–99 on this tree today (11 of 1,270 files over the limit) and names no target; (b) reads 0 at the audit and 100 exactly when 4.3.0 may ship, and still shows a regression after. |
| Strict reading | count "wontfix" dispositions as failures (desloppify) vs target zero for every dimension | **Target zero.** The repo's wontfix forms (a reasoned suppression, a reasoned skip, a kept row) are exactly the accepted debt; one rule, no new disposition state. |
| Weights | flat vs 2/2/2/1… | **Weighted**, from the spec's motivation; `--flat` prints the other pair, and flattening is one JSON edit if the maintainer prefers. |
| Subjective (LLM) dimension | in v1 (desloppify gives it 75%) vs deferred | **Deferred.** The review skills are non-deterministic and unledgered; a checked-in review ledger with dispositions is its own spec once the mechanical score is trusted. Named, not forgotten. |
| Score state | in-tree JSON vs a gitignored `.desloppify/`-style dir | **In-tree.** The archive says what is true; every disposition already has an in-tree form. |
| New censuses' baselines | five new JSON files vs inside `quality-score.json` | **Inside.** One file, one `--accept`, one ratchet; the four old censuses keep theirs. |
| Upward re-seed | prose rule vs `--accept` refusing it | **Mechanical** on every census and the ledger (`--force` for a deliberate decision, a review trigger). |
| Where the from-artifact self-test runs | package job before upload / `upload` job / downstream `test-suite.yml` | **Package job.** `upload` runs on Ubuntu only; downstream is non-gating by design and runs the ZIP, not the MSI. |
| macOS Intel slice | `arch -x86_64` on the arm64 runner vs an Intel runner | **Rosetta.** One job, the exact DMG, no second runner family; C1's premise check is the one run that proves it. |
| Windows | install the MSI vs run the portable ZIP | **Install.** R2 says "installed"; it proves uninstall for free. |
| TU splitting | `tu-cutter.py` manifests vs hand splits | **tu-cutter.** Exact reconstruction, facade-plus-members shape by manifest, `cmake-missing`/`core-unowned` catch a forgotten target entry. |
| Dedup shape | shared QML base components vs moving chrome into C++ | **QML bases.** Render-identical, no new C++ surface, no change to the `Cpp_*` contract. |
| R3 and 4.2.1 | re-tag vs amend AC7 | **Amend** (below). 4.2.1 shipped 2026-10-28 and #392 closed 2026-10-29, before R2 existed. |
| Badge | README badge (desloppify writes one) vs none | **None** for the quarter; the job summary and `--check` output are the visible number. Revisit at 4.3.0. |
| Child granularity | one spec per split/pair (~20) vs ten by track | **Ten.** Each is one premise check and one lane; one-per-file is ceremony. |

## Risks & mitigations

- **A hot TU cut costs throughput.** Benchmark before and after the first cut of each hot
  file; the spec's rule is revert, not tune. Phase B is scheduled last for that reason.
- **A dedup changes a pixel or a name.** `qml` suite, per-widget screenshot by the
  maintainer, `objectName`s unchanged (`tst_ui_automation` pins the roles), no animation
  added.
- **Rosetta absent, MSI property wrong, offscreen plugin missing from a package.** All three
  are C1's premise check, run once in CI before the gate step is made blocking.
- **Gaming by deletion** (a test, a spec note). Not mechanical; the share review and the
  commit-shape gate are the defence, and the plan says so where the score is defined.
- **CLAUDE.md word gate.** The score sentence is paid for by a trim in the same commit; the
  one-hop total is already +347 and C5 brings it under before the gate hardens.
- **Double failure.** The score fails beside the census it wraps; same cause, two lines.
- **A removed suppression un-fences a real hotpath why.** Fences in hotpath files default to
  "reasoned, kept"; removal needs the root cause fixed, per site, never in bulk.
- **Child ceremony eats the share.** A child's spec commits are consolidation commits; the
  ceremony counts for, not against.
- **Silent-breakage classes exposed** (`common-mistakes.md`): bundled scope creep (each child
  is one lane), trusting a subagent report (every number above was re-measured in the main
  session), hand-editing a generated TU (`tu-cutter` never touches `Generated/`).

## Test & verification plan

| AC | Check | Who |
|----|-------|-----|
| AC1 | `code-verify.py --tu-census --check` with baseline 0 excess / 0 files | CI |
| AC2 | `--dup-census --check` baseline ≤ 865 | CI |
| AC3 | `quality-score.py --check`: `supp` ≤ 86, `bare` = 0 | CI |
| AC4 | `--singleton-census --check` ≤ 600, cross-library 0 | CI |
| AC5 | `ledger-verify.py --check` ≤ 20; `git log -p scripts/*.json` shows no upward re-seed (the `--accept` guard makes one impossible without `--force`) | CI + review |
| AC6 | `test_ci_workflow.py::test_package_jobs_self_test_from_artifact` via `ship_legs()` | CI |
| AC7 | Amended (below) | maintainer |
| AC8 | `quality-score.py --check`: `specs` = 0, stale = 0 | CI |
| AC9 | `claim-verify.py --context-budget --check` hard on both totals, overview ≤ 3,000 words | CI |
| AC10 | `quality-score.py --check`: `skips` ≤ 10; `test_ci_workflow.py` xfail `reason=` rule | CI |
| AC11 | `quality-score.py --issues` = 0 | weekly workflow, maintainer |
| AC12 | `quality-score.py --lines` ≤ 468,963 | CI |
| AC13 | `quality-score.py --share` per month | review |
| AC14 | `pytest tests/ -m "not destructive"`, `ctest`, `--benchmark-hotpath` on the review's master | maintainer |
| AC15 | `git tag` between 4.2.0 and 4.3.0 | maintainer |

- **Unit (I can run):** `pytest scripts/tests/ -q` (new `test_quality_score.py`, the extended
  `test_ci_workflow.py`); `python scripts/quality-score.py --check`; every census.
- **Integration / perf (maintainer runs):** the pytest tiers before and after each child
  (R14); `--benchmark-hotpath` around C3-B and C6; the per-widget screenshots for C6.
- **Static:** `code-verify.py --check` on every edited file; `syntax-check.py` on every C++
  edit; `sanitize-commit.py` before every commit.

## Spec amendments surfaced by planning

To be confirmed by the maintainer, then folded into `spec.md`:

1. **R3 / AC7.** 4.2.1 was tagged and released on 2026-10-28 with all nine assets, and #392
   was closed on 2026-10-29, both before R2 existed. Proposed AC7: "4.2.1 is tagged and #392
   closed (done); the maintainer runs the published 4.2.1 DMG's x86_64 slice once under
   `arch -x86_64 … --selftest` and records the result here; the first tag after R2 lands is
   the first release whose artifacts passed AC6."
2. **Origin values measured by new censuses.** `bare`, `specs` and `skips` take the value the
   new census reads on the commit that creates it (152 / 15 / 36 were the audit's counts
   under its own grammar); `singleton` takes the census's 803.
3. **R5 scope.** The dashboard family alone leaves about 980 windows; a second family (Project
   Editor folder views, the one C++ pair) is in scope, not optional.
4. **R9 count.** 15 live non-done specs today (12 in-progress, 1 draft, 2 approved), plus two
   directories with no `spec.md`; the target stays zero.
