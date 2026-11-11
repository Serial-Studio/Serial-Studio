---
spec: 0119-resync-decision-seam
title: Testable re-sync decisions for the frame builder and the dashboard
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-11-10
author: Alex Spataru
---

# Spec 0119 — Testable re-sync decisions for the frame builder and the dashboard

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

On 2026-11-09 an operator-mode session with an audio source and a CAN source rebuilt its
dashboard on every display tick. A licence-state update re-synced the project mid-session; the
frame builder dropped its per-source layouts without announcing them again, the next audio
refresh published the combined 920-dataset frame as if it were source 0's layout, every CAN
block (916 columns) mismatched it, and each healthy audio block reset the dashboard's single
rebuild-retry flag, so the one-retry-then-quarantine rule never engaged. The fix (commit
`9e202eecf`) changed three decisions: the combined frame stands in for single-source projects
only, a live re-sync re-announces every connect-time layout, and the rebuild retry is tracked
per (source, structure generation).

None of the three is pinned by a test. The frame builder and the dashboard are the two largest
first-party units on the acquisition pipeline and neither has a unit harness, so the only
evidence that the fix holds is the absence of a field report. The decisions themselves are small
and read only plain data, but they live inline in the two classes, where a test cannot reach
them without a running pipeline, a project model and a widget registry.

## Goals

- The three re-sync decisions can be exercised by a unit test with no running pipeline,
  project model, dashboard or GUI.
- A ctest pins each of the three decisions and fails if any one is reverted; the re-sync trigger
  and the rebuild-keeps-quarantine property, which live at the call sites, are pinned by an
  integration test (AC8) and by review (AC9).
- The acquisition pipeline behaves exactly as it does today; the change is invisible to users.

## Non-Goals

- A general unit harness for the frame builder or the dashboard. This spec carves out the
  three re-sync decisions only; other behaviour stays where it is.
- The spec 0104 C3-B translation-unit cut of the same two files, planned for January. This spec
  moves decisions out; C3-B moves code between files. Neither depends on the other.
- Changing what a re-sync does. Any behaviour a test exposes as wrong is reported, not fixed
  inside this spec.
- New user-facing behaviour, settings or API commands.
- Removing the duplicate template rebuild a multi-source re-sync makes for a stream-lane source
  (it is rebuilt once as a project source and again as a stream source; only one layout reaches
  the dashboard). Recorded as an observation, not fixed here.

## Requirements

1. **R1 — Which layouts a session announces is decidable from plain data.** Given a project's
   source list, its groups' widget kinds, the set of sources that publish through the stream
   lane, the operation mode and the connection state, one decision answers which layouts the
   connect edge announces and which a live re-sync announces again. A multi-source project
   announces one layout per source that has at least one group; a single-source project whose
   every group is an image widget announces the whole frame; any other single-source project
   announces nothing ahead of its first block. A re-sync announces every one of those layouts
   again, even one that already went out since the edit, and announces every stream-lane
   source's layout at least once. A re-sync announces nothing, and forgets nothing, when
   disconnected, outside project mode, or with a project that has no groups or no title.
2. **R2 — Whether the combined frame may stand in is decidable from plain data.** A synthetic
   refresh publishes the combined frame in place of a source's own frame only when the project
   has at most one source and no per-source frame (other than a stream-lane source's) took part
   in that refresh. In a multi-source project the combined frame never publishes under any
   source's identity.
3. **R3 — The rebuild retry is a per-source, per-generation decision.** When a block does not
   match the dashboard's widget model, the first mismatch for a (source, structure generation)
   pair triggers one rebuild; the second quarantines that pair and its blocks are dropped; a
   quarantined pair stays dropped until that source's structure generation changes, which earns
   a fresh retry. Traffic from any other source, healthy or not, never changes a source's
   retry or quarantine state. A rebuild never forgets any source's retry or quarantine, whether
   it came from a retry or from another source's layout arriving. Only a hard dashboard reset
   forgets them all: a disconnect that leaves no stream, a replay player opening or closing, a
   project-file swap, or leaving project mode.
4. **R4 — The live components use those decisions and nothing else.** The frame builder and
   the dashboard make each of the three choices through the extracted decision, so a test of
   the decision is a test of the shipped behaviour.
5. **R5 — Each decision has a failing-when-reverted test.** For each of the three decisions
   there is a ctest case that fails against that decision's pre-`9e202eecf` behaviour.
6. **R6 — A live edit keeps every source's widgets.** An edit made through the API while a
   multi-source project is connected leaves every source's widgets on the dashboard, including
   when a synthetic refresh runs before any source's next frame.

## Acceptance Criteria

- [x] **AC1** — A new ctest suite constructs no application singleton and no QML, and covers
  R1: multi-source connect announces one layout per source with groups and none for a group-less
  source; image-only single source announces the whole frame; other single source announces
  nothing; a re-sync forgets before it announces, re-announces every connect-time layout, and
  announces every stream-lane source at least once; disconnected, non-project mode, no groups
  or no title announce and forget nothing.
- [x] **AC2** — The same suite covers R2: a two-source project never yields a combined-frame
  stand-in; a one-source project yields it only when no per-source frame other than a
  stream-lane source's took part in the refresh.
- [x] **AC3** — The same suite covers R3 with the 2026-11-09 shape: source A mismatches
  (rebuild); source B then mismatches, rebuilds, mismatches again and is quarantined; source A
  mismatches again and is quarantined, not rebuilt a second time (a single global flag fails
  this). A new generation for A rebuilds once more, and a hard reset clears both states.
- [ ] **AC4** — For each of the three decisions, `tasks.md` names the pre-fix line whose
  revert makes its cases fail. The maintainer reverts each decision locally, rebuilds only this
  suite, and records the failure; the assistant does not build.
- [ ] **AC5** — `--benchmark-hotpath` holds its CI gate on every leg, and the per-tier numbers
  from the maintainer's local run before and after stay within run-to-run noise.
- [ ] **AC6** — The existing integration suite passes unchanged on all four CI legs, now that a
  pytest failure reddens the run.
- [x] **AC7** — Neither of the two god translation units grows: the TU census is not re-seeded
  upward for them.
- [ ] **AC8** — An integration test (commercial build, which CI's test legs run) connects a
  two-source session, makes a layout-neutral project edit through the API, and forces a
  synthetic refresh before either source's next frame. The dashboard's widget count settles
  back to its pre-edit value within a deadline and then stays constant over an observation
  window, and both sources keep receiving values.
- [x] **AC9** — Review confirms the only place that forgets retry and quarantine state is the
  hard-reset path, never the rebuild path. The plan decides whether an anchor enforces it.

## Constraints & Invariants

- **The deciding constraint: no allocation and no new thread hop on the publish path.** The
  combined-frame decision runs on the pipeline thread at display-tick rate (only when stream
  values changed) or at command rate, never once per frame; the extracted form must still
  allocate nothing per call and must not move work off that thread.
- The 256 kHz `--benchmark-hotpath` gate holds.
- Behaviour-preserving: the published blocks, their order, and the dashboard's rebuild and
  quarantine outcomes are identical for every input, before and after. The one exception is a
  source's structure generation returning to an earlier value, which only a mirror peer can
  produce: it earns a fresh rebuild, as R3 says, where the old code could drop.
- Unit-tier rules: the new ctest suite constructs no application singleton and no QML.
- The integration suite now gates the run, so the AC8 check observes by polling to a deadline;
  it never asserts a timing floor.
- One class per header/source pair; decomposition through real member sub-objects, not a class
  spread across translation units.
- Layering: a decision used by the frame builder lives at or below the pipeline library; one
  used only by the dashboard may live in the UI library.

## Premise Check

- **Question:** Do the three decisions read only plain data, so a unit test can drive them
  without a pipeline, a project model or a dashboard, and would such a test have caught the
  2026-11-09 defects at all? If the inputs drag in singletons or live widget state, or the
  defects were wiring rather than decisions, the seam buys no coverage and the spec dies.
- **Measured:** (2026-11-10, read of the current code and of `9e202eecf`.) Layout announcement
  reads four builder members (the frame's source list, group widget kinds and title, the
  stream-lane source set, the connection state) plus the operation mode, and zero singleton
  reaches. The combined-frame stand-in reads the source count and whether any per-source frame
  published in the same refresh: zero singletons. The rebuild retry reads the source id, its
  structure generation and two per-source maps, and is written in exactly two places (the
  mismatch handler and the hard reset): zero singletons. Of the three defects the fix closed,
  two (the combined frame claiming source 0's layout, and the global retry flag re-armed by
  another source's healthy block) are pure decision logic. The third (no re-announce after a
  re-sync) is half decision, half trigger wiring. A decision-level test would have caught two
  of three outright and the content of the third.
- **Verdict:** Proceed. The seam is plain-data and singleton-free. The re-sync *trigger* stays
  outside unit reach and is pinned end to end by AC8; the rebuild-keeps-quarantine property
  lives at the dashboard's call site and is pinned by AC9.

## Open Questions

- **Q1 — resolved 2026-11-10.** The trigger half of R1 gets an integration test (R6, AC8). The
  suite already connects a two-source session on loopback TCP, and any mutating API command
  made while connected schedules the live re-sync, so a CI runner can reach it.
