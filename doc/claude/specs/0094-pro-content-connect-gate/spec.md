---
spec: 0094-pro-content-connect-gate
title: Pro-Content Connect Gate
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-10-06
author: Alex Spataru
---

# Spec 0094 — Pro-Content Connect Gate

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Dataset transforms, Variables and custom workspaces are Pro features, but since the spec
0092 feature-list audit their gate guards only the authoring gestures: opening the transform
editor, creating a table, switching a project to custom workspaces. Running a project that
already contains them is free. Three things follow, all observed in the current tree. Work
authored during a 14-day trial keeps running after the trial ends, indefinitely. Every
release grants a fresh trial (intended policy), so the authoring window reopens every few
weeks. And the gate can be skipped without the editor at all: the project file is plain
JSON, and the automation API sets transform code and creates tables with no licensing check.

Sixteen months of orders say what this costs. Payers convert on hard walls: the monthly plan
is used as a project pass (median three payments, a quarter of subscribers gone after one),
and the Pro features that refuse on every use, a Pro driver that will not connect, are the
ones with no workaround. An authoring-only gate gives nobody a lasting reason to pay.

On 2026-10-04 the maintainer ruled that loaded projects keep running these features
unlicensed, because the alternative on the table was switching transforms off, and silently
changing displayed data is worse than the leak. That reasoning still holds, and this spec
keeps it: it replaces the ruling with a mechanism that never changes a displayed value. An
unlicensed project with Pro content does not run degraded. It does not run.

## Goals

- A project that contains transforms or Variables cannot acquire data without a license or
  an active trial, in every build.
- No licensing state ever shows a value from a project whose transforms did not run.
- The refusal explains itself and names both ways forward: unlock Pro, or remove the content.
- The authoring gate can no longer be bypassed through the automation API or by editing the
  project file.
- A session already in progress is never interrupted by a license or trial ending.

## Non-Goals

- **No grandfathering.** The gate applies to every project, whatever version created it
  (maintainer decision, 2026-10-06).
- **No runtime degradation of the math.** Transforms are never skipped, stubbed or bypassed;
  nothing here adds licensing work to a per-frame path.
- **No change to trial cadence.** A fresh trial per release stays intended policy.
- **No change to which features are Pro**, and no change to Quick Plot or Console Only modes,
  which carry no project content.
- **No rewrite of the bundled examples.** Seven of 28 use transforms; they stay as they are
  and offer the trial when run unlicensed (maintainer decision, 2026-10-06).
- **No project schema change.** A refused project is not modified on disk.
- **No separate release is cut first.** The draft deferred this to the release after 4.2;
  the maintainer chose on 2026-10-06 to build it now, so it ships with the next release.
- **Mirror viewing is not gated.** A viewer shows values the publisher already computed and
  never opens a device; the publisher is the one that meets the gate.
- **No telemetry or analytics.**

## Requirements

1. **R1 — What counts as Pro content.** A project contains Pro content when at least one
   dataset carries non-blank transform code, or the project defines at least one user table
   (Variables). User tables are the tables saved with the project: the automatic system
   table does not count, and a computed dataset with no transform code does not count.
   Custom workspaces alone do not count; R6 covers them.
2. **R2 — Connect is refused.** With no license and no active trial, connecting while a
   Pro-content project is the active project is refused before any device is opened. No
   frame from that project reaches a dashboard, an export or a recording. The same holds
   for content that reaches a session already running: when a load, undo, redo or mode
   switch gives a live session without an entitlement more transforms or tables than it
   was admitted with, the session ends with the same explained refusal, and a driver's
   own reconnect after a drop is refused on the same terms.
3. **R3 — Playback stays free.** File playback never runs a project's transforms: recorded
   values are shown as recorded. Playing a CSV through a Pro-content project therefore works
   in every licensing state. MDF4 and Historian playback keep the Pro gates they already
   have. (Amended in planning; the draft refused playback.)
4. **R4 — The refusal is the central prompt.** On a machine that never used a trial, the
   refusal asks the one trial question; accepting starts the trial and the refused connect
   proceeds in the same session with no second gesture. After expiry it offers activation or
   purchase. A GPL build shows its Pro notice instead.
5. **R5 — The refusal explains itself.** The prompt names the kind of content that triggered
   it (transforms, Variables) and how many items, and states that removing them is the free
   alternative.
6. **R6 — Custom workspaces fall back.** Without a license, a project that defines custom
   workspaces shows the automatic layout. The project file is untouched, and the custom
   layout returns without a reload once a license or trial is active.
7. **R7 — Removal is free.** Clearing a dataset's transform and deleting a user table or a
   variable work without a license, each as one explicit and undoable gesture. Creating and
   editing stay Pro. Once the content is gone the project connects in every licensing state.
8. **R8 — Running sessions finish.** A license or trial ending while a session is connected
   does not disconnect it, rebuild its devices, or change what it computes. The next connect
   is refused. This covers sessions on free buses; a Pro-bus session still ends when the
   entitlement does, as under spec 0092. A dial still in flight counts as a running
   session, so accepting the trial never cancels the connect it was asked for.
9. **R9 — The automation surface refuses too.** API commands that set non-blank transform
   code, create or rename a user table, or add or change a variable fail with an explicit
   error when unlicensed. Clearing a transform and deleting a table or variable stay
   allowed, as do commands that load a whole project, because the connect gate covers what
   they bring in. A connect requested over the API fails with an error that names the
   reason. A remote-origin request never raises a dialog.
10. **R10 — Operator and headless runs say why.** In operator runtime mode the refusal
    reaches the operator through the same explained prompt when Connect is pressed
    (automatic connect is already skipped without a license or trial). A headless run
    reports the reason in its log. (Amended in planning; prompts are not suppressed in
    runtime mode today.)
11. **R11 — Unlocking applies live.** Activating a license or starting a trial while a gated
    project is loaded makes connect and playback available at once, with no project reload
    and no restart.
12. **R12 — The rule is published.** User documentation, the unlicensed and GPL welcome
    texts, and the in-app assistant's reference material state the rule, and the release
    notes carry a migration note for projects created in earlier versions.

## Acceptance Criteria

- [ ] **AC1** (R1) — A unit test over the content rule: blank and whitespace-only transform
      code do not count, non-blank code does, one user table does, custom workspaces alone
      do not. `ctest`.
- [ ] **AC2** (R2, R9) — With the running app unentitled, loading a project with one
      transform and requesting connect over the API returns an error naming the reason, and
      no frame is published. `pytest` integration, skipped when the app is entitled.
- [ ] **AC3** (R2) — A project with no Pro content connects on a free bus in every licensing
      state. Extends the existing free-bus pin. `pytest` integration.
- [ ] **AC4** (R3) — With the app unentitled, playing a CSV through a Pro-content project
      works and shows the recorded values. Maintainer observation.
- [ ] **AC5** (R4, R11) — On a machine with no trial history, clicking Connect on a
      Pro-content project raises the trial question; accepting connects without a second
      click. Maintainer observation.
- [ ] **AC6** (R5) — The prompt for a project with three transforms and one table states
      both kinds and both counts. Maintainer observation in English and one right-to-left
      language.
- [ ] **AC7** (R6) — Unlicensed, a custom-workspace project shows the automatic layout and
      the file is byte-identical after the session; licensed, the custom layout returns
      without reopening the project. Maintainer observation plus a file hash check.
- [ ] **AC8** (R7) — Unlicensed, clearing the only transform lets the project connect; undo
      restores the transform and the next connect is refused. Maintainer observation.
- [ ] **AC9** (R8) — With a session connected to a Pro-content project, forcing the trial to
      expire leaves the session running and its values unchanged; disconnecting and
      reconnecting is refused. Maintainer observation against a seeded expiry.
- [ ] **AC10** (R9) — Unlicensed API calls that set transform code or create a table return
      an error and leave the project unchanged; no dialog appears on the host. `pytest`
      integration.
- [ ] **AC11** (R10) — An operator deployment of a Pro-content project on an unlicensed
      machine shows the reason and no prompt. Maintainer observation.
- [ ] **AC12** (R4) — A GPL build refuses the same connect and shows its Pro notice.
      Maintainer observation on a GPL build.
- [ ] **AC13** — `--benchmark-hotpath` gates unchanged.
- [ ] **AC15** (R2) — Unentitled, a free project connected over TCP ends its session when
      `project.loadJson` brings in a transform (source saved without connection settings, so
      no device rebuild runs); entitled, the session stays up. `pytest`
      `test_pro_content_loaded_into_a_live_session`. Maintainer observation for the GUI path:
      Remove Transform, Connect, Undo ends the session with the explained prompt.
- [ ] **AC16** (R4, R8) — On a fresh profile, a TCP project with one transform connects on
      the first click after the trial is accepted (no second click). Maintainer observation.
- [ ] **AC14** (R12) — `documentation-verify.py` and `claim-verify.py` clean on the changed
      docs; the licensing consumer inventory records this spec as superseding the 2026-10-04
      ruling.

## Constraints & Invariants

- **The deciding constraint: a dashboard never shows data from a project whose transforms
  did not run.** The gate refuses; it never degrades. Any design that leaves a Pro-content
  project half-running is out of scope by construction.
- **Command rate only.** The decision is taken when a connect or playback is requested and
  when licensing state changes. Nothing is added to the acquisition pipeline, and the
  throughput gates do not move.
- **Free projects are never hostage.** A project with no Pro content behaves identically in
  every licensing state (spec 0092 R6 holds).
- **License-derived state re-derives on transitions.** Anything that bakes the gate's answer
  must track activation and expiry live (the spec 0042 consumer rule).
- **Remote origin stays silent.** No request arriving over the API may raise a modal dialog
  on the host.
- **Trial parity holds.** An active trial unlocks everything a license does.
- **This supersedes a recorded maintainer ruling.** The licensing consumer inventory changes
  in the same commit, with the reason.
- **Licensing behavior is decision-visible.** The change is named in chat with its failure
  modes before it lands and never rides inside a bulk package.
- **No project schema change, and no new third-party dependency.**
- **New user-facing strings cover all 21 languages** before release.

## Open Questions

- **Existing users.** With no grandfathering, a free user whose project has used transforms
  since before they were gated meets this on upgrade. The plan treats the explained prompt
  (R5) as the notice and puts the migration note in the release notes; whether a separate
  one-time in-app notice is wanted is still the maintainer's call before release.

Resolved in planning (2026-10-06), with code evidence recorded in `plan.md`:

- CSV, MDF4 and Historian playback never re-run transforms, so R3 was rewritten.
- No free importer or generator emits transforms or tables; only the Pro importers do.
- The Variables boundary is the saved table list; the system table and the constants
  library do not count on their own.
- A computed dataset with no transform is a constant zero and needs no rule.
- A mirror viewer shows publisher-computed values and never opens a device, so viewing is a
  non-goal.

## Amendments

Changes made after approval, during `/ss-plan`, confirmed with the plan on 2026-10-06:

- **R3 and AC4** — playback is no longer refused (it never ran transforms).
- **R8** — "does not rebuild its devices" added, because a license transition rebuilds live
  devices today.
- **R9** — narrowed to the commands that author content; clearing, deleting and
  whole-project loads stay allowed.
- **R10** — the "prompts are suppressed in runtime mode" premise was wrong and is removed.

Changes made after the six-reviewer pass of 2026-10-06, approved by the maintainer's "fix all
issues" the same day:

- **R2** — extended past connect time: content reaching a live unentitled session ends it,
  keyed on content growing past the session's admission, so a licence ending (R8) does not.
  Known edge, accepted: a session that started entitled, outlived its licence, and then
  loads a project with no more transforms and tables than it started with keeps running.
- **R8** — scoped to free buses, and in-flight dials count as running.
- **AC15, AC16** — added for the two behaviours above.
