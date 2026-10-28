---
spec: 0104-consolidation-quarter
title: 4.3.0 consolidation quarter
status: approved     # draft -> approved -> in-progress -> done | shelved
approved: 2026-10-28 # tracks, targets, share and exit rule; open questions decided
created: 2026-10-27
author: Alex Spataru
---

# Spec 0104 — 4.3.0 Consolidation Quarter

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Approved 2026-10-28; `/ss-plan` may
> start when the maintainer asks for it.

> **Umbrella spec.** Like spec 0030, this is a program, not a single feature: four tracks
> with numeric exit targets, run in parallel with ordinary feature work at a fixed share of
> each week. Each piece of work that is more than trivial graduates to its own numbered spec
> through `/ss-spec` before any plan or code exists. Approving this spec approves the tracks,
> the targets, the share and the exit rule, not any implementation.

## Problem / Motivation

Between 2025-11-30 and 2026-10-27 the first-party code grew from 59,730 to 446,632 lines
(7.5×), the first-party C++ file count from 115 to 1,270, Qt classes from 52 to 244 and
local include edges from 267 to 3,791, all carried by one maintainer. Per-function
complexity stayed flat (lizard average 3.33 to 3.32), so the complexity moved up into the
module graph and into volume, where one person cannot hold it. The repository's own gates
measure the debt that accumulated on the way:

- 11 first-party C++ files exceed 1,500 lines, 2,641 lines of excess in total; the largest is
  2,911 lines.
- 1,730 duplicated 10-line windows across 21 file pairs; six of the eight heaviest pairs are
  dashboard widget QML files that evolved as copies of one another.
- 171 lint suppression sites in 93 files, 152 of them with no stated reason.
- 802 singleton reach sites across 200 files (down from 1,596 in July; none cross a library).
- 51 of the 57 mistakes-ledger rows are prose only: no check enforces them, and the repo's
  own audit found that prose rules are the ones that keep failing.
- 13 of the 104 numbered specs are not marked done (10 in-progress, 2 draft, 1 approved),
  so the archive no longer says what is true.
- 36 tests are skipped or expected to fail (18 in the pytest tiers, 18 in the C++ tier).
- 22 issues are open, the oldest from 2021, with no triage state.
- No CI leg ever launches a shipped artifact. Every 4.x macOS installer was unusable on Intel
  Macs for four and a half months (issue #392) while build, signing, notarization and the
  arm64 test job passed, because nothing exercised the Intel half.

The October 2026 history audit traced both user-facing failures of the year to the same
shape: a change inside a package too large to read, on a surface nothing exercised. The
commit-shape gate, the ledger ratchet and the premise check now stop new debt of that shape
from landing. This spec is about the debt already in the tree.

## Goals

- At the end of the quarter every growth ratchet reads lower than on 2026-10-27, and none was
  re-seeded upward on the way.
- Every artifact a user can download has been launched by CI from the exact file that was
  uploaded, on every platform and slice it ships for.
- The mistakes ledger, the spec archive and the open-issue list each say what is true: a row
  is enforced or gone, a spec is done or shelved, an issue is fixed, declined or scheduled.
- The maintenance surface stops growing for one quarter: first-party line count at the end is
  within 5% of the start, while feature work continues at a fixed share.
- The quarter ends on measured criteria, reviewed at the end of January 2027, never on a
  calendar date alone; 4.3.0 is the release that marks the exit, and 4.2.x point releases
  keep shipping fixes on cadence until then.

## Non-Goals

- New data sources, widgets, Pro features or protocol stacks as consolidation work. Feature
  work continues on its own share and its own specs; it does not count toward this spec.
- Rewriting the pipeline, changing the library layering or the composition root. The
  structure is sound; the quarter reduces volume inside it.
- Performance work beyond keeping the throughput gate intact.
- A full-code rewrite of the dashboard widgets. Deduplication keeps every widget's observable
  behavior and look.
- Translating or restructuring the user manual.

## Requirements

Numbers are measured on 2026-10-27 at master `be6066d59`; a target is a value the existing
gate reads at the end of the quarter.

1. **R1 — Fixed share.** Consolidation receives two-thirds of each week's engineering time
   in November 2026, while the artifact proof and 4.2.1 land, and half from December on. The
   share is auditable from the commit log: in November at least two-thirds, and in every
   later month at least half, of the non-merge commits carry a consolidation spec reference
   or a consolidation scope. A month that misses the share is named in the January review.
2. **R2 — Ship-artifact proof.** Every CI package leg launches the artifact it uploads, from
   the uploaded file and not from the build tree, and runs the in-app self-test from it: the
   Linux x64 and arm64 AppImages, the universal macOS disk image on both slices, and the
   installed Windows MSI. A self-test failure fails the job before the artifact is published.
3. **R3 — 4.2.1.** A 4.2.1 release ships with the Intel fix through the proven path of R2,
   and issue #392 is answered and closed by the reporter's confirmation or after 30 days.
4. **R4 — Translation-unit excess to zero.** No first-party C++ file exceeds 1,500 lines
   (excess 2,641 → 0, files over the limit 11 → 0). Each split keeps the facade-plus-members
   shape the repo already uses and keeps every test passing.
5. **R5 — Duplicated windows halved.** Shared 10-line windows 1,730 → at most 865, with the
   dashboard widget QML family addressed first (the six heaviest pairs). A deduplicated widget
   renders identically before and after, checked by the existing QML self-test suite plus a
   screenshot comparison the maintainer runs once per widget.
6. **R6 — Suppressions reasoned or removed.** Lint suppression sites 171 → at most 86, and
   every remaining site carries a one-clause reason on the suppression line; bare
   suppressions (152 today) reach zero.
7. **R7 — Singleton reach down.** Singleton reach sites 802 → at most 600, with
   cross-library reaches staying at zero.
8. **R8 — Ledger closed down.** Open mistakes-ledger rows 51 → at most 20. Each row that
   leaves the open set is either codified by a check that exists in the tree, or deleted in a
   commit whose message says why the class no longer applies. The ledger ratchet is re-seeded
   only downward.
9. **R9 — Spec archive reconciled.** Every spec not marked done on 2026-10-27 ends the
   quarter as done or shelved with a dated note; no spec sits in-progress for more than 30
   days without a commit referencing it.
10. **R10 — AI-facing material does not grow.** The always-loaded word budget and the one-hop
    document total both end the quarter lower than on 2026-10-27, the one-hop total becomes a
    hard gate that fails on growth and is re-seeded downward only (as the always-loaded budget
    already is), and a newcomer can read the system's shape from one architecture overview of
    at most 3,000 words without opening the AI-facing tree.
11. **R11 — Test-tier debt.** Skipped and expected-to-fail tests 36 → at most 10; each
    survivor names an upstream bug or an environmental reason, as the test policy already
    requires.
12. **R12 — Issue triage.** Every open issue is fixed, declined with a reply, or scheduled in
    a numbered spec by the January review; no issue waits more than 90 days without a
    maintainer reply.
13. **R13 — Delete budget.** The dead-code candidate list from the 2026 comment cleanup is
    resolved (each candidate removed or kept with a reason), and first-party C++ plus QML
    lines at the January review are at most 468,963, five percent above the 446,632 of
    2026-10-27, because splits move lines and only deletion removes them while features keep
    landing at half share.
14. **R14 — Behavior preserved.** A consolidation change never alters user-visible behavior;
    the pytest integration, security and performance suites and the C++ unit tier pass before
    and after each one, and the hotpath gate's nine tiers hold their thresholds.
15. **R15 — Exit release.** 4.3.0 ships when AC1 through AC14 hold on master, and not before;
    4.2.x point releases continue on their own cadence for fixes in the meantime, 4.2.1 first
    (R3).

## Acceptance Criteria

- [ ] **AC1** — The translation-unit census check passes with a baseline of zero excess and
      zero files over the limit (R4).
- [ ] **AC2** — The duplicate-window census check passes with a baseline at or below 865
      shared windows (R5).
- [ ] **AC3** — A suppression count over the first-party tree reports at most 86 sites and
      zero without a reason, and the number is held by a checked-in baseline the lint job
      reads (R6).
- [ ] **AC4** — The singleton census check passes with a baseline at or below 600 sites and
      zero cross-library reaches (R7).
- [ ] **AC5** — The ledger ratchet check passes with a baseline at or below 20 open rows, and
      `git log` over the quarter shows no upward re-seed of any ratchet baseline (R8).
- [ ] **AC6** — Each of the four package jobs in CI contains a step that runs the uploaded
      artifact's self-test, pinned by the workflow test suite, and the macOS step runs both
      slices (R2).
- [ ] **AC7** — Release 4.2.1 is tagged and its macOS artifact passed AC6; issue #392 is
      closed (R3).
- [ ] **AC8** — No spec under the archive carries `in-progress`, `draft` or `approved` older
      than 30 days without a referencing commit; the count of non-done specs from 2026-10-27
      is zero (R9).
- [ ] **AC9** — The context-budget check reports both word totals lower than the 2026-10-27
      baseline, fails on growth of either, and the architecture overview exists at or under
      3,000 words (R10).
- [ ] **AC10** — The skip and xfail marker count across all test tiers is at most 10 and the
      test-policy check passes (R11).
- [ ] **AC11** — The issue tracker shows zero open issues without a maintainer reply in the
      last 90 days (R12).
- [ ] **AC12** — The first-party line count, measured the way the 2026-10-27 history audit
      measured it, is at or below 468,963 at the January review (R13).
- [ ] **AC13** — In November 2026 at least two-thirds, and in every later month at least half,
      of the non-merge commits reference a consolidation spec or scope (R1).
- [ ] **AC14** — The throughput gate and every pytest and ctest tier pass on the January
      review's master (R14).
- [ ] **AC15** — The 4.3.0 tag points at a master where AC1 through AC14 hold, and every tag
      between 4.2.0 and 4.3.0 is a 4.2.x point release (R15).

## Constraints & Invariants

- The 256 kHz hotpath gate and its nine tiers are never lowered; a split or a dedup that
  costs throughput is reverted, not tuned around.
- No ratchet baseline is ever re-seeded upward during the quarter. Downward re-seeds are
  expected and are made in the commit that earned them.
- Every piece of consolidation work beyond a trivial edit is its own numbered spec with its
  own premise check; this umbrella approves direction and targets only.
- The commit-shape gate applies as it stands: protected surfaces land alone, bulk needs a
  stated generator, a spec is born before its code.
- Security-sensitive surfaces (licensing, machine identity, API auth and consent) are out of
  scope for mechanical splitting or deduplication; a change there needs its own spec and is
  named in chat with its failure modes first.
- Feature work on its own share is unaffected by this spec except for R13: the five percent
  allowance is the whole quarter's, so a feature-heavy month is balanced by deletion before
  the review, not ignored.
- Trust Contract: foreign working-tree files are never touched; nothing is committed without
  per-turn permission.

## Premise Check

- **Question:** Would pausing half of feature intake for a quarter cost revenue, and is the
  debt real rather than a baseline artifact?
- **Measured:** Sales were flat at roughly $845 a month, about nine paying customers a month
  and a 0.16% download-to-paid rate across the entire 7.5× growth period (Lemon Squeezy
  export of 2026-10-06), so feature velocity did not move revenue. The debt is not noise:
  2,641 lines of translation-unit excess, 1,730 duplicated windows concentrated in six widget
  pairs, 152 bare suppressions and 51 prose-only ledger rows are each read by a gate that
  already exists, and the Intel-Mac window showed what an unexercised artifact costs
  (4.5 months, every 4.x macOS user on Intel).
- **Verdict:** Proceed. Half of the feature share bought nothing measurable in sales this
  year; every target is a number an existing gate reads today, so the quarter can end on
  evidence.

## Open Questions

None open. The draft's five questions were decided by the maintainer on 2026-10-28 and are
folded into the requirements above: the share is two-thirds in November and half after (R1);
the singleton target stays at 600 because halving took three months last time (R7); the
line budget allows five percent net growth by the review rather than net zero (R13); 4.3.0
marks the exit and 4.2.x point releases continue (R15); the one-hop document total becomes
a downward-only hard gate (R10).
