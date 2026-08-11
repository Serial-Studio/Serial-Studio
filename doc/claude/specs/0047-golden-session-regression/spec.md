---
spec: 0047-golden-session-regression
title: Golden-Session Parser Regression
status: done         # draft -> approved -> in-progress -> done | shelved
# Implementation complete 2026-08-06; acceptance criteria await the maintainer's pytest runs
# (AC checkboxes below stay open until then).
created: 2026-08-06
author: Alex Spataru
---

# Spec 0047 — Golden-Session Parser Regression

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Spec 0044 proves an archived session reproduces under its *own* archived project
configuration. It deliberately answers only one question: "does this archive still decode
the way it was recorded?" The question users actually face day to day is the inverse: **"I
just edited my frame parser / transform / project — does my old telemetry still decode the
same, and if not, what exactly changed?"** Today the only tool for that is a single-frame
parser dry-run; there is no session-scale answer. A user who reworks a delimiter, tightens a
checksum, or rescales a transform ships the change blind against every byte they have ever
captured, and discovers breakage when an old session replays wrong — or never.

The gap is mechanical, and the wrong obvious design is demonstrably broken: a scratchpad
simulation (10 000 archived frames, three typical parser edits) showed that comparing
regenerated readings to recorded readings *by ordinal position* turns a parser change that
drops 3 % of frames into **9 696 false "value changed" reports when the true drift is 0
values changed and 304 frames lost**. Pairing readings by their capture timestamp instead
reports the truth exactly, and cleanly separates "values moved" from "frames appeared or
vanished" from "a dataset was added or removed". That alignment rule is the deciding
constraint of this feature.

For the same regulated/industrial audience 0044 serves (test benches, calibration labs,
flight-test telemetry), this closes the loop: pin known-good sessions as *golden*, and gate
every interpretation change — interactive or CI — on "all golden sessions still decode
identically, or here is the exact drift". It turns the Session Database from an archive into
a regression harness.

## Goals

- A user can run a **regression pass**: replay an archived session's raw bytes through a
  *candidate* interpretation — the project currently open in the editor, or an explicitly
  supplied project file — and compare the regenerated readings against the recorded ones.
- The pass produces a **drift verdict** from a fixed taxonomy: *identical*, *value drift*
  (same readings exist, some values differ), *coverage drift* (readings missing or extra —
  frame extraction changed), *structural drift* (datasets added or removed), or *not
  mechanically verifiable* (inherited from the session's 0044 classification).
- A **drift report** localizes every difference per dataset: how many values changed, the
  first divergence (capture time, recorded value, regenerated value), the largest numeric
  delta, and how many readings exist only on one side — precise enough that the user can
  tell an intended change from an accident without replaying anything by hand.
- The pass is **scriptable end to end**: invocable from the existing external automation
  surface with the full report returned, so CI can gate a parser change on a set of golden
  sessions.
- **Golden pinning rides existing session tags**: a regression run can target every session
  carrying a chosen tag and return an aggregated pass/drift summary alongside the per-session
  reports.
- The **session browsing UI** can launch a regression pass against the currently open
  project and display the drift report, so the interactive "did my edit break old data"
  loop needs no scripting.

## Non-Goals

- **No tolerance knob.** The verdict is bit-exact on the stored representation, same as
  0044; the report carries deltas so a human or script can judge intended drift. A "close
  enough" parameter would silently weaken the product's bit-stability claim.
- **No change to 0044 reproduction semantics.** Verification against the archived project,
  its verdicts, its stored records, and its classification rules stay exactly as specified;
  regression is a sibling pass, not a replacement or a mode switch of it.
- **Not a project migration or auto-fix tool.** The pass reports drift; it never edits the
  candidate project, the archive, or the session to make them agree.
- **No cross-archive comparison.** Both sides of the diff come from one session in one
  database; comparing two different sessions or two databases is out of scope.
- **No capture-path changes.** Everything this feature needs is already archived by 0044;
  nothing new is recorded at capture time and nothing touches the dashboard hotpath.
- **No scheduling or watch mode.** CI and macros drive recurring runs; the product ships
  the single pass.

## Requirements

1. **R1 — On-demand regression pass.** The user can invoke a regression pass on any
   completed archived session. The pass runs offline (no device connection) and must not
   disturb a live capture, an open replay, or a running 0044 verification.
2. **R2 — Candidate selection.** The candidate interpretation is either (a) the project
   currently open in the editor — the default — or (b) an explicitly supplied project file.
   The report records which candidate was used, identified well enough (name plus a content
   fingerprint) that a stored or logged report is attributable to an exact configuration.
3. **R3 — Real pipeline.** Regeneration runs the archived raw bytes through the same frame
   extraction and interpretation engine the live path uses — never a parallel
   reimplementation — configured by the candidate project.
4. **R4 — Provenance alignment.** The baseline side of the comparison is the session as
   reproduced under its **archived** configuration on the current build — bit-identical to
   the recorded readings whenever the session's reproducibility verdict (spec 0044) is
   *reproduced*; the report states that reproduction status alongside the drift verdict.
   Baseline and candidate readings are paired per dataset by capture provenance — the
   archived chunk of raw bytes that produced them, ties within a chunk paired in production
   order. Readings present on only one side are reported as *missing* or *added* — never as
   value changes. Ordinal (position-based) pairing across the stream is explicitly
   non-conforming. *(Amended at plan time, 2026-08-06: baseline redefined from "recorded
   readings" to "archived-configuration replay" — deterministic alignment by construction;
   maintainer approved.)*
5. **R5 — Verdict taxonomy.** The session verdict is the most severe drift class present,
   in the order: *structural drift* > *coverage drift* > *value drift* > *identical*. The
   report always carries all four dimensions regardless of which one names the verdict.
   Comparison is bit-exact on the stored numeric representation; text values compare
   exactly.
6. **R6 — Drift report.** Per dataset, the report states: readings compared, values
   changed, first divergence (capture time of the producing chunk, baseline value,
   candidate value), largest numeric delta, readings only in the baseline, readings only in
   the candidate. Datasets present on only one side are listed as removed or added.
   Identical datasets report zero on every drift figure.
7. **R7 — Honest classification.** Datasets classified by 0044 as not mechanically
   verifiable (table-fed, virtual, script-fed values) regress to that classification, per
   dataset, never to a drift verdict the pass could not actually establish. Legacy
   (pre-0044) sessions run best-effort with the verdict qualified as legacy, mirroring
   0044's rule. *(Amended 2026-08-07, maintainer-requested: control-script sessions are NOT
   refused wholesale — unlike 0044 verification, both regression replays run without the
   script under identical conditions, so the comparison of parsed values is deterministic
   by construction; the report notes that script effects are excluded. The pass must
   guarantee the script cannot execute during either replay.)*
11. **R11 — Configuration comparison** *(added 2026-08-07)*. The report states whether the
    control script, the frame parser (any source), or any per-dataset value transform
    differs textually between the archived project and the candidate, so the user sees
    *what kind* of change produced the drift without reading either project.
8. **R8 — Scriptable.** The regression pass is invocable through the existing external
   automation surface with the verdict and full drift report returned, in a form a script
   can gate on (distinguishing *identical*, each drift class, *not verifiable*, and
   *error*).
9. **R9 — Golden-tag sweep.** A single automation call can run the pass over every session
   carrying a caller-named tag and return an aggregated summary (sessions passed / drifted /
   not verifiable / failed) plus the individual reports. No new pinning concept is
   introduced; existing session tags are the mechanism.
10. **R10 — Session UI integration.** From the session browsing UI the user can run a
    regression pass for a session against the currently open project and view the drift
    report (verdict, per-dataset figures, first divergences). The UI makes visually clear
    that this is drift against a *candidate*, distinct from the 0044 reproducibility
    verdict shown for the archived configuration.

## Acceptance Criteria

- [ ] **AC1 — Identity.** Capture a session, leave the project untouched, run regression:
  verdict *identical*, every dataset reports zero drift on all figures. (pytest via the
  automation surface, R1/R3/R5.)
- [ ] **AC2 — Value drift.** Change one transform constant in the candidate, rerun: verdict
  *value drift*; the affected dataset reports the exact changed count, correct first
  divergence pair, and a max delta consistent with the edit; all other datasets report zero
  drift. (pytest, R2/R5/R6.)
- [ ] **AC3 — Coverage drift, no false value diffs.** Use a candidate parser variant that
  rejects a known subset of archived frames: verdict *coverage drift*; the report shows
  exactly the rejected readings as missing and **zero values changed** — the ordinal-pairing
  failure mode from the motivating simulation must not occur. (pytest, R4/R5/R6.)
- [ ] **AC4 — Structural drift.** Candidate adds one dataset and removes another: verdict
  *structural drift*; report lists the added and removed datasets; surviving datasets
  report zero drift. (pytest, R5/R6.)
- [ ] **AC5 — Explicit candidate file.** Run the same pass with a supplied project file
  differing from the open project: results reflect the file, and the report's candidate
  identification names it. (pytest, R2.)
- [ ] **AC6 — Classification honored.** A session with a data-table-fed transform returns
  the not-mechanically-verifiable classification for the affected datasets, not a drift
  verdict; a pre-0044 fixture session runs best-effort with a legacy-qualified verdict.
  (pytest, R7.)
- [ ] **AC7 — Golden-tag sweep.** Tag three sessions, run the tag sweep with a drifting
  candidate: aggregated summary counts match the individual verdicts. (pytest, R8/R9.)
- [ ] **AC8 — Non-interference.** Regression during a live capture disturbs neither the
  capture nor the dashboard; concurrent-use limits with 0044 verification are enforced
  cleanly rather than by corruption. (destructive-marked pytest + maintainer observation,
  R1.)
- [ ] **AC9 — UI loop.** Maintainer runs the interactive loop: open project, edit a
  transform, run regression from the session browser, read the drift report, revert, rerun,
  see *identical*. (maintainer observation, R10.)
- [ ] **AC10 — Hotpath unchanged.** `--benchmark-hotpath` gate unchanged; no capture-path
  or dashboard-path cost is added. (CI gate, Constraints.)

## Constraints & Invariants

- **The deciding constraint: alignment is by capture time, never by position.** Grounded,
  not hypothetical: the motivating simulation showed ordinal pairing converts a 3 % frame
  loss into thousands of false value diffs. Any implementation whose report fails AC3 is
  wrong regardless of how it is built.
- **Reuse the real interpretation pipeline** (inherited from 0044): drift detected by the
  pass must be drift a live user would see with the candidate project.
- **Archives are read-only evidence.** The pass never mutates recorded session data; the
  recorded side of the diff is exactly what 0044 protects.
- **0044 behavior is untouched.** Reproducibility verification, its verdicts, its stored
  records, and existing replay semantics must be bit-for-bit unaffected by this feature's
  presence.
- **No hotpath regression.** Nothing per-frame on the dashboard path; the 256 kHz CI gate
  stands.
- **Honesty over green checkmarks.** Bit-exact verdicts, no numeric fuzz, classification
  instead of approximation — same posture as 0044.
- **Old databases keep opening.** Any stored artifact this feature adds must be additive
  and backward compatible; a pre-0047 archive must never be rendered unreadable.
- **Pro feature.** Ships under the same gate as the Session Database, trial-equals-Pro.

## Open Questions

- **Q1 — Dataset identity across projects.** Matching datasets between recording and
  candidate requires a stable identity; a renamed or re-created dataset will surface as
  removed + added (structural drift). Is that acceptable UX, or does the report need a
  "possibly renamed" hint when a removed and an added dataset pair up plausibly?
  Recommendation: accept removed + added for this spec; hinting is heuristic polish.
- **Q2 — Persistence of regression results.** *Resolved 2026-08-06: ephemeral — results
  are returned/displayed and never written to the archive; durable regression history is a
  follow-up if real usage wants it.*
- **Q3 — Candidate changes the source topology.** *Resolved 2026-08-06 (maintainer):
  archived streams match candidate sources by device id only; an archived device with no
  candidate source is whole-source structural drift (reported as source-removed), never
  fuzzily remapped and never silently parsed by a fallback configuration.*
- **Q4 — Equal-timestamp ties.** *Resolved by the R4 amendment: provenance pairing makes
  tie order deterministic by construction on both sides.*
- **Q5 — UI report depth.** Inline drift table only, or also an export of the full report
  from the UI? Recommendation: inline table plus "copy report" for this spec; formatted
  export can ride existing report tooling later.
