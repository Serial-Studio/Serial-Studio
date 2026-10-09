---
spec: 0095-structural-invariant-enforcement
title: Structural invariant enforcement
status: in-progress    # draft -> approved -> in-progress -> done | shelved (M4 amended 2026-10-07, re-confirmed)
created: 2026-10-07
author: Alex Spataru
---

# Spec 0095 — Structural invariant enforcement

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Several of the repo's most expensive defect classes are guarded only by prose. Each failed
silently in the field before the prose existed, and each still compiles, lints and passes CI
when violated today:

- **Cached hotpath flags.** The acquisition pipeline reads seven cached booleans/modes instead
  of recomputing them per frame. A new input that is not wired to the cache refresh leaves the
  cache stale: frames stop reaching the dashboard, or recordings come out as valid-looking empty
  files. Nothing ever compares a cached value against a fresh derivation, so a stale cache is
  invisible until a user notices missing data. (The 2026-09-12 data-table arm/release skip is
  the dated instance; the generic "frames/exports silently stop" row in the mistakes ledger is
  undated and uncodified.)
- **Republish lanes.** The dashboard lane and the export lane must keep separate "already
  republished" marks. When they shared one (2026-08-18), a 635-dataset field project exported 4
  populated columns, 107 of 109 MDF4 channel groups were empty and the historian recorded
  nothing. The current guard is a single bookkeeping object whose callers pass a boolean saying
  which lane they are; a caller can claim to be the wrong lane and nothing objects.
- **Time rings and plot clocks.** Rings must be sized from a rate and a window, never a sample
  count alone (2026-08-15: 44.1 kHz audio filled a 10 s axis to 5.9 s). The per-source plot
  clocks and the shared display time are one logical state that must reset together (2026-08-18:
  clearing only the clocks blanked QuickPlot audio for seconds after every rebuild). Today
  capacities are plain integers and the two halves of the clock state are separate members that
  happen to be cleared in one place.
- **Constructor ordering.** The project model does real work in its constructor (new-file
  setup, file watching, autosave scheduling, control-script reset) before the modules it may
  reach exist. A reach into a not-yet-built module aborts at startup (2026-07-07). The guard is a
  re-run-on-every-edit proof plus prose, and grounding for this spec found that the doc's
  "gated on an initialized flag" claim is stale: the flag is set but never read.

The cost is paid twice. Contributors (human and AI) are protected only if they read and retain
~5,200 lines of live guidance, and the AI-facing process (per-turn canary, verbalized
invariants) exists largely to keep these facts in context. An invariant held by a type, an
assertion or a test costs nothing per turn; one held in prose costs attention on every edit and
fails exactly when attention lapses.

## Goals

- Each of the four defect classes above fails loudly (compile error, debug abort, or a failing
  test) when violated, instead of producing silent data loss.
- A stale cached flag is detected and repaired in release builds within about one second,
  with a one-time diagnostic, instead of persisting for the rest of the session.
- The live AI/contributor guidance shrinks measurably, because rules now enforced by code
  collapse to a one-line pointer naming their enforcer.
- Every remaining prose rule in the binding sections is explicitly labeled as either enforced
  (and by what), enforceable-but-not-yet (tracked debt), or a judgment call.

## Non-Goals

- No change to user-visible behavior, file formats, the API surface, or recording contents.
- No throughput work: the hotpath gate must hold, not improve.
- No rewrite of the threading model. The intentional auto-queued refresh of pipeline-owned
  flags from GUI emitters (spec 0051 M3) stays as is.
- No removal of the context canary or J-Space discipline in this spec; their future is
  revisited after the doc triage shows how much prose remains.
- No new linter framework. Existing gates (code-verify, claim-verify, ctest, selftest) are
  extended, not replaced.
- No changes to the licensing or API-auth surfaces (M6 touches the API server's enabled flag
  and client count storage only, not auth or consent).

## Requirements

### M1 — Cached flag cross-check

1. **R1** — Every cached hotpath flag the dataflow doc lists has a single, authoritative
   "derive from scratch" definition, and the cached value is only ever written from it.
2. **R2** — At a low fixed rate (about 1 Hz), on the thread that owns each flag, the cached
   value is compared against a fresh derivation. The comparison never runs per frame.
3. **R3** — On mismatch: debug builds abort with a message naming the flag, the cached value
   and the derived value; release builds report once per flag per session and overwrite the
   cache with the derived value, so the pipeline recovers within about one second.
4. **R4** — Adding a new cached hotpath flag without registering it in the cross-check fails a
   check (lint, test, or census), so the mechanism cannot be bypassed by omission.
5. **R5** — The two documents that currently disagree on how flag refreshes must be wired
   (Direct vs. intentionally auto-queued) are reconciled to one statement.

### M2 — Typed republish lanes

6. **R6** — The dashboard lane and the export lane are distinct types. Code holding one cannot
   ask the other's question or record the other's publish; a lane mix-up is a compile error.
7. **R7** — Both lanes still share their underlying bookkeeping, with the existing rule intact:
   any lane seeing a change marks the sinks dirty; only an export publish clears it.
8. **R8** — The existing republish-lane unit tests pass unchanged in intent, and one new test
   proves a masked dashboard refresh cannot leave the export lane believing it is current.

### M3 — Rate-sized rings and unified plot clocks

9. **R9** — A time-ring capacity can only be produced from a rate and a window (with the shared
   ceiling applied). Constructing or resizing a time ring from a bare sample count does not
   compile.
10. **R10** — The per-source plot clocks and the shared display time are one value with one
    reset operation. No code path can clear, save, or restore one half without the other.
11. **R11** — Existing ring-sizing behavior is preserved exactly: the 44.1/48 kHz audio cases
    fill their configured window, and the rebuild and block-continuation rules from the
    2026-08 incidents still hold.

### M4 — Construction-order guard (amended 2026-10-07 during planning)

> Planning found the original M4 premise mostly obsolete. Since spec 0039 every core module is
> adopted after construction, and its instance accessor already fails loudly by name when
> reached before adoption, so the 2026-07-07 recursion abort cannot recur for those modules.
> The project model's constructor reaches no other module today (its one cross-module call is
> a no-op at construction, and autosave is guarded by an empty path). Moving that work to
> "after all modules exist" would also change the startup state the next module reads. M4 is
> therefore reduced to closing the real residual gaps.

12. **R12** — Reaching a core module before it is available fails with a message that says
    which module was reached and why it is unavailable: still under construction, or not yet
    constructed in the pinned order. Today all three cases print one generic message.
13. **R13** — Every composition root that publishes data binds the interfaces, and a check
    fails if a root that publishes omits the binding. A root that does not publish (schema
    dump) is explicitly listed as exempt rather than silently skipped.
14. **R14** — Startup reaches into singletons that are not adopted modules (function-static
    instances) are counted by an existing census, so new ones are visible in review instead of
    being found by a startup abort.
15. **R15** — The stale startup-doc claims are corrected: the dead "initialized" flag is removed
    (or made load-bearing), the wrong line citations are fixed, and the ctor-edge proof's
    re-run trigger is narrowed to the residual surface that the runtime guard does not cover.

### M5 — Doc triage

16. **R16** — Every rule in the binding sections of CLAUDE.md (Threading & Hotpath, Startup &
    Composition Root, Subsystem Contracts) and the architecture docs they point to is classified
    as one of: *enforced* (names its enforcer), *enforceable* (tracked debt), or *judgment*.
17. **R17** — Every *enforced* rule is reduced to a one-line pointer naming its enforcer; the
    full rationale and incident history move to the relevant architecture doc or the mistakes
    ledger, not deleted.
18. **R18** — A named enforcer must exist: claim-verify fails if a rule (or a mistakes-ledger
    `Codified:` tag) names a lint rule, test, anchor, or assertion site that does not exist.
19. **R19** — The *enforceable* list is recorded as follow-up debt, ordered by the severity of
    the silent failure it guards.

### M6 — Cross-thread flag inputs (added 2026-10-07 at the tasks gate)

20. **R20** — Every input the pipeline thread reads to derive a cached hotpath flag is safe to
    read from that thread: either owned by the pipeline thread or published through an atomic.
    No data race remains on the inputs M1 audits (control-script running state, every block
    sink's activity verdict, the change-driven-transforms setting).
21. **R21** — A sink whose activity verdict is derived from a container (e.g. "any recording
    session open") exposes that verdict through an atomic summary instead of reading the
    container from a foreign thread.
22. **R22** — The block-sink interface documents that its activity verdict is called from the
    pipeline thread and must be lock-free and race-free, so a new sink inherits the rule.

## Acceptance Criteria

- [ ] **AC1** (R1–R3) — A ctest unit drives each registered flag's inputs, deliberately
  desynchronizes the cache, and observes: debug-mode abort hook fires; release-mode path
  reports once and the cache matches the derivation after the next check.
- [ ] **AC2** (R4) — Adding a dummy cached flag without registration makes the chosen gate fail
  (demonstrated once during implementation, then reverted).
- [ ] **AC3** (R6) — A compile-fail probe (a lane asked the other lane's question) fails
  `scripts/syntax-check.py`; demonstrated once, not checked in.
- [x] **AC4** (R7–R8) — Republish-lane ctests pass, including the new masked-refresh case
  (CI run 37668278964: 186/186 ctests on Linux x86_64, Linux arm64, TSan and ASan+UBSan).
- [ ] **AC5** (R9) — A compile-fail probe sizing a time ring from a bare integer fails
  `scripts/syntax-check.py`.
- [ ] **AC6** (R10–R11) — Existing dashboard/time-ring tests pass; in the running app, a 48 kHz
  audio QuickPlot fills its 10 s window and survives a rebuild without blanking (maintainer
  observation).
- [ ] **AC7** (R12) — A ctest reaches a module during its own construction and before its
  turn in the pinned order; each produces its distinct named message (debug-abort hook).
- [ ] **AC8** (R13–R14) — Removing the interface binding from one publishing root fails a
  check; adding a new function-static singleton reach in a startup path grows a census.
- [x] **AC9** (R16–R18) — claim-verify passes on the triaged docs and fails on a seeded bogus
  enforcer name; the live guidance line count is reported before and after (tasks.md T30:
  CLAUDE.md 445 → 412, live guidance 5297 → 5269).
- [x] **AC11** (R20–R22) — Read-back: every `sinkActive()` implementation and every checker
  derive input resolves to an atomic load or pipeline-owned state (recorded in tasks.md T6);
  TSan-clean where the maintainer runs a TSan build (CI TSan job: ctest suite clean, no reports).
- [x] **AC10** (all) — `--benchmark-hotpath` passes all nine gates; `sanitize-commit.py` clean
  (tasks.md T15; sanitize ran before `3657d2ca2`).

## Constraints & Invariants

- **Deciding constraint: nothing new on the per-frame path.** Cross-checks run at about 1 Hz on
  the owning thread; type changes must compile to the same per-frame code. The 256 kHz gate
  must hold.
- Cross-checks read flag inputs on the thread that owns the flag; no new GUI→pipeline blocking
  wait, no new mutex on the hotpath, no message-bus traffic on the hotpath.
- The pinned singleton construction order does not change; only the work done inside the
  project model's constructor moves.
- Singleton census, TU census and layering baselines must not grow.
- Prose is moved, not lost: incident history and rationale survive in architecture docs or the
  mistakes ledger.
- Each milestone lands independently and leaves the tree green; M5 runs last (or after each of
  M1–M4) so it classifies the post-enforcement state.

## Open Questions

- **R4 gate shape:** should "unregistered cached flag" be caught by a code-verify rule (naming
  convention / annotation) or by a census baseline like the singleton census? Plan-phase call
  unless the maintainer has a preference.
- ~~**R14 scope**~~ — resolved by the M4 amendment: the guard covers all adopted modules.
- **M5 scope:** include `common-mistakes.md` rows in the triage, or only CLAUDE.md and the
  architecture docs? Recommendation: include the ledger only for R18's `Codified:` validation,
  not a full rewrite.
